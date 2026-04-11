#pragma once

/**
 * @file Pi0EventMixingAlg.hpp
 * @brief Event-mixing background estimation for π⁰ → γγ reconstruction
 *
 * Implements the event-mixing technique to estimate combinatorial background
 * in the γγ invariant mass spectrum. Photons from different events in the
 * same kinematic bin (Q², x_B, W, photon multiplicity) are paired to construct a background
 * shape that can be subtracted from the same-event distribution.
 *
 * Also implements the rotational background technique: one photon's azimuthal
 * angle is rotated by 180° around the beam axis within the same event, preserving
 * event-level correlations while breaking the real γγ pairing.
 *
 * Produces same-event, mixed-event, and rotated-background mgg histograms in 4D
 * kinematic bins (Q², x_B, z, p_T). Background subtraction and signal fitting
 * are done offline.
 *
 * Thread-safe: uses std::shared_mutex (readers-writer lock) so multiple threads
 * can read the photon pool concurrently while writes are brief and exclusive.
 */

#include <cmath>
#include <deque>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include <Math/Vector4D.h>
#include <Math/Vector3D.h>
#include <Math/GenVector/VectorUtil.h>

#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Pipeline.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

class Pi0EventMixingAlg : public clas12::framework::SimpleAlgorithm {
   public:
    explicit Pi0EventMixingAlg(std::string name = "Pi0EventMixing",
                                int max_pool_events = 50)
        : SimpleAlgorithm(std::move(name))
        , m_max_pool_events{"max_pool_events", max_pool_events, "Max events per DIS bin in mixing pool"}
        , m_cutflow("Pi0 Event Mixing") {
        declare_property(m_max_pool_events);
        declare_property(m_beam_energy);
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hist = svc<clas12::framework::HistogramService>();

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Max pool events", fmt::format("{}", m_max_pool_events.get()));
        Log::config(name(), "Beam energy", fmt::format("{} GeV", m_beam_energy.get()));
        Log::config(name(), "Pool bins (Q2 x xB x W x mult)", fmt::format("{} x {} x {} x {} = {}",
            m_Q2_edges.size() - 1, m_xB_edges.size() - 1, m_W_edges.size() - 1, N_MULT_BINS,
            (m_Q2_edges.size() - 1) * (m_xB_edges.size() - 1) * (m_W_edges.size() - 1) * N_MULT_BINS));
        Log::config(name(), "4D bins total", fmt::format("{}",
            (m_Q2_edges.size() - 1) * (m_xB_edges.size() - 1) *
            (m_z_edges.size() - 1) * (m_pT_edges.size() - 1)));

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has DIS Electron");
        m_cutflow.add_cut("Has Photons");
        m_cutflow.add_cut("Same-event pairs");
        m_cutflow.add_cut("Rotated-bg pairs");
        m_cutflow.add_cut("Mixed-event pairs");

        book_histograms();

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        using namespace clas12::framework;

        auto dis          = EventHelpers::get_dis_kinematics(event);
        auto electron_opt = EventHelpers::get_selected_electron(event);
        auto photons_opt  = event.retrieve_typed(Keys::SelectedPhotons_Pi0);

        const bool have_photons = photons_opt && !photons_opt->empty();

        // Pre-compute virtual photon direction + pool bin. These are
        // cheap but depend on dis + electron, so only do it when both
        // are present. The pipeline below filters on the same.
        ROOT::Math::PxPyPzEVector beam_4v(0.0, 0.0, m_beam_energy.get(), m_beam_energy.get());
        ROOT::Math::PxPyPzEVector electron_4v = electron_opt
            ? electron_opt->PxPyPzEVector()
            : ROOT::Math::PxPyPzEVector{};
        ROOT::Math::PxPyPzEVector q_4v = beam_4v - electron_4v;
        auto q_3v = q_4v.Vect();
        const double q_mag = std::sqrt(q_3v.Mag2());
        ROOT::Math::XYZVector q_hat = q_mag > 0
            ? ROOT::Math::XYZVector{q_3v / q_mag}
            : ROOT::Math::XYZVector{};

        const double Q2 = dis ? dis->Q2 : 0.0;
        const double xB = dis ? dis->xB : 0.0;
        const double nu = dis ? dis->nu : 0.0;
        const double W  = dis ? dis->W  : 0.0;
        const int pool_bin = (dis && have_photons)
            ? find_pool_bin(Q2, xB, W, photons_opt->size(), nu)
            : -1;

        return pipeline(event, m_cutflow)
            .count("All Events")
            .require(Decisions::HasGoodDISElectron)
            .count("Has DIS Electron")
            .filter("has DIS kinematics",  dis.has_value())
            .filter("Has Photons",          have_photons)
            .count("Has Photons")
            .filter("has electron",         electron_opt.has_value())
            // Same-event pairs: PairPipeline over unique photon pairs
            // from this event. No pool access, fully parallel.
            .tap([&] {
                if (photons_opt->size() < 2) return;
                unique_pairs(*photons_opt, m_cutflow)
                    .filter("Same-event pairs", [&](const clas12::Particle& g1, const clas12::Particle& g2) {
                        return validate_photon_pair(
                            electron_4v, g1.PxPyPzEVector(), g2.PxPyPzEVector());
                    })
                    .tap([&](const clas12::Particle& g1, const clas12::Particle& g2) {
                        auto pi0_4v = g1.PxPyPzEVector() + g2.PxPyPzEVector();
                        const double mgg = pi0_4v.M();
                        const double z   = pi0_4v.E() / nu;
                        ROOT::Math::XYZVector pi0_3v(pi0_4v.Px(), pi0_4v.Py(), pi0_4v.Pz());
                        const double p_parallel = pi0_3v.Dot(q_hat);
                        ROOT::Math::XYZVector p_perp = pi0_3v - p_parallel * q_hat;
                        const double pT = std::sqrt(p_perp.Mag2());
                        if (h_mgg_same_all) h_mgg_same_all->fill(mgg);
                        const int bin_4d = find_4d_bin(Q2, xB, z, pT);
                        if (bin_4d >= 0 && h_mgg_same.count(bin_4d)) {
                            h_mgg_same[bin_4d]->fill(mgg);
                        }
                    })
                    .run();
            })
            // Rotated-background pairs: rotate one photon's phi by 180
            // degrees and pair with the unrotated other. PairPipeline
            // filter rejects pairs that fail the validation cut.
            .tap([&] {
                if (photons_opt->size() < 2) return;
                unique_pairs(*photons_opt, m_cutflow)
                    .filter("Rotated-bg pairs", [&](const clas12::Particle& g1, const clas12::Particle& g2) {
                        auto g1_4v = g1.PxPyPzEVector();
                        auto g2_rot = rotate_phi_180(g2.PxPyPzEVector());
                        return validate_photon_pair(electron_4v, g1_4v, g2_rot);
                    })
                    .tap([&](const clas12::Particle& g1, const clas12::Particle& g2) {
                        auto g1_4v = g1.PxPyPzEVector();
                        auto g2_rot = rotate_phi_180(g2.PxPyPzEVector());
                        auto pi0_4v = g1_4v + g2_rot;
                        const double mgg = pi0_4v.M();
                        const double z   = pi0_4v.E() / nu;
                        ROOT::Math::XYZVector pi0_3v(pi0_4v.Px(), pi0_4v.Py(), pi0_4v.Pz());
                        const double p_parallel = pi0_3v.Dot(q_hat);
                        ROOT::Math::XYZVector p_perp = pi0_3v - p_parallel * q_hat;
                        const double pT = std::sqrt(p_perp.Mag2());
                        if (h_mgg_rotated_all) h_mgg_rotated_all->fill(mgg);
                        const int bin_4d = find_4d_bin(Q2, xB, z, pT);
                        if (bin_4d >= 0 && h_mgg_rotated.count(bin_4d)) {
                            h_mgg_rotated[bin_4d]->fill(mgg);
                        }
                    })
                    .run();
            })
            // Mixed-event pairs: current photons x stored pool photons.
            // PairPipeline walks the full Cartesian product; the pool
            // snapshot is taken once under a shared_lock and fed to
            // particle_pairs as the rhs collection.
            .tap([&] {
                if (pool_bin < 0) return;
                std::vector<clas12::Particle> pool_photons_snapshot;
                {
                    std::shared_lock lock(m_pool_mutex);
                    auto pool_it = m_photon_pools.find(pool_bin);
                    if (pool_it == m_photon_pools.end() || pool_it->second.photons.empty()) return;
                    pool_photons_snapshot = pool_it->second.photons;
                }
                const std::size_t n_before = m_cutflow.count("Mixed-event pairs");
                particle_pairs(*photons_opt, pool_photons_snapshot, m_cutflow)
                    .filter("Mixed-event pairs", [&](const clas12::Particle& g1, const clas12::Particle& g2) {
                        return validate_photon_pair(
                            electron_4v, g1.PxPyPzEVector(), g2.PxPyPzEVector(), true);
                    })
                    .tap([&](const clas12::Particle& g1, const clas12::Particle& g2) {
                        auto pi0_4v = g1.PxPyPzEVector() + g2.PxPyPzEVector();
                        const double mgg = pi0_4v.M();
                        const double z   = pi0_4v.E() / nu;
                        ROOT::Math::XYZVector pi0_3v(pi0_4v.Px(), pi0_4v.Py(), pi0_4v.Pz());
                        const double p_parallel = pi0_3v.Dot(q_hat);
                        ROOT::Math::XYZVector p_perp = pi0_3v - p_parallel * q_hat;
                        const double pT = std::sqrt(p_perp.Mag2());
                        if (h_mgg_mixed_all) h_mgg_mixed_all->fill(mgg);
                        const int bin_4d = find_4d_bin(Q2, xB, z, pT);
                        if (bin_4d >= 0 && h_mgg_mixed.count(bin_4d)) {
                            h_mgg_mixed[bin_4d]->fill(mgg);
                        }
                    })
                    .run();
                // The old body incremented "Mixed-event pairs" once per
                // event that had a non-empty pool; the PairPipeline
                // filter above increments once per surviving pair.
                // Post-process to match the original one-per-event
                // semantics: if no pair survived but the pool was
                // non-empty, bump once for the event marker.
                if (m_cutflow.count("Mixed-event pairs") == n_before) {
                    m_cutflow.pass("Mixed-event pairs");
                }
            })
            // Pool update: brief exclusive lock, O(1) per event.
            .tap([&] {
                if (pool_bin < 0) return;
                std::unique_lock lock(m_pool_mutex);
                auto& pool = m_photon_pools[pool_bin];
                pool.photons.insert(pool.photons.end(), photons_opt->begin(), photons_opt->end());
                pool.event_sizes.push_back(photons_opt->size());
                while (static_cast<int>(pool.event_sizes.size()) > m_max_pool_events.get()) {
                    const size_t n = pool.event_sizes.front();
                    pool.photons.erase(pool.photons.begin(),
                                       pool.photons.begin() + static_cast<ptrdiff_t>(n));
                    pool.event_sizes.pop_front();
                }
            })
            .status();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        // Report pool statistics
        size_t total_stored = 0;
        size_t bins_used = 0;
        size_t max_occupancy = 0;
        size_t total_photons = 0;
        size_t max_photons = 0;
        for (const auto& [bin, pool] : m_photon_pools) {
            if (!pool.event_sizes.empty()) {
                bins_used++;
                total_stored += pool.event_sizes.size();
                max_occupancy = std::max(max_occupancy, pool.event_sizes.size());
                total_photons += pool.photons.size();
                max_photons = std::max(max_photons, pool.photons.size());
            }
        }
        double avg_occupancy = bins_used > 0 ? static_cast<double>(total_stored) / bins_used : 0.0;
        Log::stat(name(), "Pool bins used", bins_used);
        Log::stat(name(), "Total events in pools", total_stored);
        Log::stat(name(), "Total photons in pools", total_photons);
        Log::stat(name(), "Avg event occupancy", fmt::format("{:.1f}", avg_occupancy));
        Log::stat(name(), "Max event occupancy", max_occupancy);
        Log::stat(name(), "Max photon pool size", max_photons);
        Log::info(name(), "Offline normalization: scale mixed to same in sideband [{:.2f}, {:.2f}] GeV",
                  SIDEBAND_LOW, SIDEBAND_HIGH);
        Log::info(name(), "Offline fit region: [{:.2f}, {:.2f}] GeV", FIT_RANGE_LOW, FIT_RANGE_HIGH);

        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");

        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    // Properties
    clas12::framework::Property<int> m_max_pool_events{"max_pool_events", 100, "Max events per DIS bin"};
    clas12::framework::Property<double> m_beam_energy{"beam_energy", 10.53, "Beam energy [GeV]"};

    // Service pointers (cached in on_initialize via svc<T>())
    clas12::framework::HistogramService* m_hist = nullptr;

    // Cutflow
    clas12::framework::Cutflow m_cutflow;

    // Photon pool: flat collection of all photons from the last N events per DIS bin
    struct PhotonPool {
        std::vector<clas12::Particle> photons;   // flat pool of all stored photons
        std::deque<size_t> event_sizes;           // number of photons per event (for FIFO eviction)
    };
    std::map<int, PhotonPool> m_photon_pools;
    mutable std::shared_mutex m_pool_mutex;

    // Binning edges
    std::vector<double> m_Q2_edges = {1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 7.0, 11.0};
    std::vector<double> m_xB_edges = {0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.7};
    std::vector<double> m_z_edges = {0.0, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.8, 1.0};
    std::vector<double> m_pT_edges = {0.0, 0.15, 0.30, 0.50};
    std::vector<double> m_W_edges = {2.0, 2.5, 3.0, 3.5, 4.0, 5.0};
    std::vector<double> m_nu_edges = {2.0, 4.0, 5.5, 7.0, 9.0, 11.0};  // virtual photon energy bins
    static constexpr int N_MULT_BINS = 4;  // photon multiplicity bins: 1, 2, 3, 4+

    // Histograms: per 4D bin
    std::map<int, clas12::framework::ThreadLocalHistogram1D*> h_mgg_same;
    std::map<int, clas12::framework::ThreadLocalHistogram1D*> h_mgg_mixed;
    std::map<int, clas12::framework::ThreadLocalHistogram1D*> h_mgg_rotated;

    // Histograms: inclusive
    clas12::framework::ThreadLocalHistogram1D* h_mgg_same_all = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_mgg_mixed_all = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_mgg_rotated_all = nullptr;

    // =========================================================================
    // Photon pair validation cuts (matching reference implementation)
    // =========================================================================

    // Sideband normalization region for offline background subtraction (GeV)
    // To extract pi0 signal: scale mixed histogram so that
    //   integral_same(sideband) == integral_mixed(sideband)
    // then subtract: h_signal = h_same - scale * h_mixed
    static constexpr double SIDEBAND_LOW = 0.19;
    static constexpr double SIDEBAND_HIGH = 0.24;
    // Pi0 fit region (GeV)
    static constexpr double FIT_RANGE_LOW = 0.10;
    static constexpr double FIT_RANGE_HIGH = 0.16;

    // Minimum angle between electron and each photon (degrees)
    static constexpr double MIN_ELECTRON_PHOTON_ANGLE = 8.0;
    // Minimum invariant mass to reject collinear/fake pairs (GeV)
    static constexpr double MIN_INVARIANT_MASS = 0.001;
    // Momentum-dependent minimum opening angle: A * exp(-B * p) + offset (degrees)
    static constexpr double MIN_PHOTON_ANGLE_A = 17.561;
    static constexpr double MIN_PHOTON_ANGLE_B = 0.756;
    static constexpr double MIN_PHOTON_ANGLE_OFFSET = 1.0;

    /// Rotate a 4-vector's azimuthal angle by 180° around the beam axis (z)
    /// Preserves pT, pz, and energy; flips px → -px, py → -py
    static auto rotate_phi_180(const ROOT::Math::PxPyPzEVector& v) -> ROOT::Math::PxPyPzEVector {
        return ROOT::Math::PxPyPzEVector(-v.Px(), -v.Py(), v.Pz(), v.E());
    }

    /// Momentum-dependent minimum photon opening angle (degrees)
    static auto min_photon_opening_angle(double momentum) -> double {
        return MIN_PHOTON_ANGLE_A * std::exp(-MIN_PHOTON_ANGLE_B * momentum) + MIN_PHOTON_ANGLE_OFFSET;
    }

    /// Validate a photon pair against quality cuts
    /// Checks: electron-photon angles, invariant mass, momentum-dependent opening angle
    /// For mixed events, only check g1 (current event photon) against the electron;
    /// g2 (stored photon) already passed this cut with its own electron.
    static auto validate_photon_pair(
        const ROOT::Math::PxPyPzEVector& electron_4v,
        const ROOT::Math::PxPyPzEVector& g1_4v,
        const ROOT::Math::PxPyPzEVector& g2_4v,
        bool is_mixed = false) -> bool {

        // Check electron-photon angle (> 8 degrees)
        // For mixed events, only check g1 (current event photon) — g2 is from a different event
        double angle1 = ROOT::Math::VectorUtil::Angle(electron_4v, g1_4v) * 180.0 / M_PI;
        if (angle1 < MIN_ELECTRON_PHOTON_ANGLE) return false;

        if (!is_mixed) {
            double angle2 = ROOT::Math::VectorUtil::Angle(electron_4v, g2_4v) * 180.0 / M_PI;
            if (angle2 < MIN_ELECTRON_PHOTON_ANGLE) return false;
        }

        // Check invariant mass above minimum
        auto total = g1_4v + g2_4v;
        double mass = total.M();
        if (mass < MIN_INVARIANT_MASS) return false;

        // Check momentum-dependent photon opening angle
        double momentum = std::sqrt(total.Px() * total.Px() + total.Py() * total.Py() + total.Pz() * total.Pz());
        double opening_angle = ROOT::Math::VectorUtil::Angle(g1_4v, g2_4v) * 180.0 / M_PI;
        if (opening_angle < min_photon_opening_angle(momentum)) return false;

        return true;
    }

    // =========================================================================
    // Binning helpers
    // =========================================================================

    auto find_1d_bin(double value, const std::vector<double>& edges) const -> int {
        for (size_t i = 0; i < edges.size() - 1; ++i) {
            if (value >= edges[i] && value < edges[i + 1]) return static_cast<int>(i);
        }
        return -1;
    }

    /// Map photon multiplicity to bin index: 0=1γ, 1=2γ, 2=3γ, 3=4+γ
    static auto find_multiplicity_bin(size_t n_photons) -> int {
        if (n_photons == 0) return -1;
        if (n_photons >= 4) return 3;
        return static_cast<int>(n_photons) - 1;
    }

    /// Compute pool bin from (Q2, xB, W, photon multiplicity, nu)
    auto find_pool_bin(double Q2, double xB, double W, size_t n_photons, double nu) const -> int {
        int iq = find_1d_bin(Q2, m_Q2_edges);
        int ix = find_1d_bin(xB, m_xB_edges);
        int iw = find_1d_bin(W, m_W_edges);
        int im = find_multiplicity_bin(n_photons);
        int inu = find_1d_bin(nu, m_nu_edges);
        if (iq < 0 || ix < 0 || iw < 0 || im < 0 || inu < 0) return -1;

        int n_xB = static_cast<int>(m_xB_edges.size() - 1);
        int n_W  = static_cast<int>(m_W_edges.size() - 1);
        int n_nu = static_cast<int>(m_nu_edges.size() - 1);
        return (((iq * n_xB + ix) * n_W + iw) * N_MULT_BINS + im) * n_nu + inu;
    }

    auto find_4d_bin(double Q2, double xB, double z, double pT) const -> int {
        int iq = find_1d_bin(Q2, m_Q2_edges);
        int ix = find_1d_bin(xB, m_xB_edges);
        int iz = find_1d_bin(z, m_z_edges);
        int ip = find_1d_bin(pT, m_pT_edges);
        if (iq < 0 || ix < 0 || iz < 0 || ip < 0) return -1;

        int n_xB = static_cast<int>(m_xB_edges.size() - 1);
        int n_z = static_cast<int>(m_z_edges.size() - 1);
        int n_pT = static_cast<int>(m_pT_edges.size() - 1);
        return ((iq * n_xB + ix) * n_z + iz) * n_pT + ip;
    }

    struct BinIndices { int iq, ix, iz, ip; };

    auto decompose_4d_bin(int flat_idx) const -> BinIndices {
        int n_xB = static_cast<int>(m_xB_edges.size() - 1);
        int n_z = static_cast<int>(m_z_edges.size() - 1);
        int n_pT = static_cast<int>(m_pT_edges.size() - 1);

        int ip = flat_idx % n_pT;
        flat_idx /= n_pT;
        int iz = flat_idx % n_z;
        flat_idx /= n_z;
        int ix = flat_idx % n_xB;
        int iq = flat_idx / n_xB;

        return {iq, ix, iz, ip};
    }

    auto bin_4d_label(int flat_idx) const -> std::string {
        auto [iq, ix, iz, ip] = decompose_4d_bin(flat_idx);
        return fmt::format("Q2[{:.1f},{:.1f}]_xB[{:.2f},{:.2f}]_z[{:.2f},{:.2f}]_pT[{:.2f},{:.2f}]",
            m_Q2_edges[iq], m_Q2_edges[iq + 1],
            m_xB_edges[ix], m_xB_edges[ix + 1],
            m_z_edges[iz], m_z_edges[iz + 1],
            m_pT_edges[ip], m_pT_edges[ip + 1]);
    }

    // =========================================================================
    // Histogram booking
    // =========================================================================

    auto book_histograms() -> void {
        if (m_hist) { auto* svc = m_hist;
            const std::string dir = "/Pi0EventMixing";

            // Inclusive histograms
            h_mgg_same_all = svc->book_hist1D(
                dir + "/mgg_same_all",
                "Same-event M_{#gamma#gamma};M_{#gamma#gamma} [GeV/c^{2}];Counts",
                200, 0.0, 0.3);
            h_mgg_mixed_all = svc->book_hist1D(
                dir + "/mgg_mixed_all",
                "Mixed-event M_{#gamma#gamma};M_{#gamma#gamma} [GeV/c^{2}];Counts",
                200, 0.0, 0.3);
            h_mgg_rotated_all = svc->book_hist1D(
                dir + "/mgg_rotated_all",
                "Rotated-bg M_{#gamma#gamma};M_{#gamma#gamma} [GeV/c^{2}];Counts",
                200, 0.0, 0.3);

            // Per 4D bin histograms
            int n_Q2 = static_cast<int>(m_Q2_edges.size() - 1);
            int n_xB = static_cast<int>(m_xB_edges.size() - 1);
            int n_z = static_cast<int>(m_z_edges.size() - 1);
            int n_pT = static_cast<int>(m_pT_edges.size() - 1);
            int total_bins = n_Q2 * n_xB * n_z * n_pT;

            for (int idx = 0; idx < total_bins; ++idx) {
                auto [iq, ix, iz, ip] = decompose_4d_bin(idx);

                std::string title_same = fmt::format(
                    "Same M_{{#gamma#gamma}} "
                    "Q^{{2}}#in[{:.1f},{:.1f}] "
                    "x_{{B}}#in[{:.2f},{:.2f}] "
                    "z#in[{:.2f},{:.2f}] "
                    "p_{{T}}#in[{:.2f},{:.2f}]"
                    ";M_{{#gamma#gamma}} [GeV/c^{{2}}];Counts",
                    m_Q2_edges[iq], m_Q2_edges[iq + 1],
                    m_xB_edges[ix], m_xB_edges[ix + 1],
                    m_z_edges[iz], m_z_edges[iz + 1],
                    m_pT_edges[ip], m_pT_edges[ip + 1]);

                std::string title_mixed = fmt::format(
                    "Mixed M_{{#gamma#gamma}} "
                    "Q^{{2}}#in[{:.1f},{:.1f}] "
                    "x_{{B}}#in[{:.2f},{:.2f}] "
                    "z#in[{:.2f},{:.2f}] "
                    "p_{{T}}#in[{:.2f},{:.2f}]"
                    ";M_{{#gamma#gamma}} [GeV/c^{{2}}];Counts",
                    m_Q2_edges[iq], m_Q2_edges[iq + 1],
                    m_xB_edges[ix], m_xB_edges[ix + 1],
                    m_z_edges[iz], m_z_edges[iz + 1],
                    m_pT_edges[ip], m_pT_edges[ip + 1]);

                h_mgg_same[idx] = svc->book_hist1D(
                    fmt::format("{}/same/mgg_same_bin{}", dir, idx),
                    title_same, 200, 0.0, 0.3);

                h_mgg_mixed[idx] = svc->book_hist1D(
                    fmt::format("{}/mixed/mgg_mixed_bin{}", dir, idx),
                    title_mixed, 200, 0.0, 0.3);

                std::string title_rotated = fmt::format(
                    "Rotated M_{{#gamma#gamma}} "
                    "Q^{{2}}#in[{:.1f},{:.1f}] "
                    "x_{{B}}#in[{:.2f},{:.2f}] "
                    "z#in[{:.2f},{:.2f}] "
                    "p_{{T}}#in[{:.2f},{:.2f}]"
                    ";M_{{#gamma#gamma}} [GeV/c^{{2}}];Counts",
                    m_Q2_edges[iq], m_Q2_edges[iq + 1],
                    m_xB_edges[ix], m_xB_edges[ix + 1],
                    m_z_edges[iz], m_z_edges[iz + 1],
                    m_pT_edges[ip], m_pT_edges[ip + 1]);

                h_mgg_rotated[idx] = svc->book_hist1D(
                    fmt::format("{}/rotated/mgg_rotated_bin{}", dir, idx),
                    title_rotated, 200, 0.0, 0.3);
            }

            Log::info("Pi0EventMixing", "Booked {} same + {} mixed + {} rotated histograms",
                       total_bins, total_bins, total_bins);
        }
    }
};

}  // namespace clas12::algorithms
