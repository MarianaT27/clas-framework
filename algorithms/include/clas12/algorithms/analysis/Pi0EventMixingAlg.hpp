#pragma once

/**
 * @file Pi0EventMixingAlg.hpp
 * @brief Event-mixing background estimation for π⁰ → γγ reconstruction
 *
 * Implements the event-mixing technique to estimate combinatorial background
 * in the γγ invariant mass spectrum. Photons from different events in the
 * same DIS kinematic bin (Q², x_B) are paired to construct a background
 * shape that can be subtracted from the same-event distribution.
 *
 * Produces same-event and mixed-event mgg histograms in 4D kinematic bins
 * (Q², x_B, z, p_T). Background subtraction and signal fitting are done offline.
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
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

class Pi0EventMixingAlg : public clas12::framework::SimpleAlgorithm<Pi0EventMixingAlg> {
   public:
    explicit Pi0EventMixingAlg(std::string name = "Pi0EventMixing",
                                int max_pool_events = 10)
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
        Log::config(name(), "DIS bins (Q2 x xB)", fmt::format("{} x {} = {}",
            m_Q2_edges.size() - 1, m_xB_edges.size() - 1,
            (m_Q2_edges.size() - 1) * (m_xB_edges.size() - 1)));
        Log::config(name(), "4D bins total", fmt::format("{}",
            (m_Q2_edges.size() - 1) * (m_xB_edges.size() - 1) *
            (m_z_edges.size() - 1) * (m_pT_edges.size() - 1)));

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has DIS Electron");
        m_cutflow.add_cut("Has Photons");
        m_cutflow.add_cut("Same-event pairs");
        m_cutflow.add_cut("Mixed-event pairs");

        book_histograms();

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Retrieve DIS kinematics
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodDISElectron)) return clas12::framework::StatusCode::FILTERED();
        m_cutflow.pass("Has DIS Electron");

        auto Q2_opt = event.retrieve_typed(clas12::framework::Keys::DIS_Q2);
        auto xB_opt = event.retrieve_typed(clas12::framework::Keys::DIS_xB);
        auto nu_opt = event.retrieve_typed(clas12::framework::Keys::DIS_nu);
        if (!Q2_opt || !xB_opt || !nu_opt) return clas12::framework::StatusCode::FILTERED();

        double Q2 = *Q2_opt;
        double xB = *xB_opt;
        double nu = *nu_opt;

        // Retrieve selected photons (need >= 1 for pool, >= 2 for same-event pairs)
        auto photons_opt = event.retrieve_typed(clas12::framework::Keys::SelectedPhotons_Pi0);
        if (!photons_opt || photons_opt->empty()) return clas12::framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Photons");

        const auto& photons = *photons_opt;

        // Get electron for virtual photon direction and pair validation
        auto electron_opt = clas12::framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return clas12::framework::StatusCode::SUCCESS();

        // Compute virtual photon
        ROOT::Math::PxPyPzEVector beam_4v(0.0, 0.0, m_beam_energy.get(), m_beam_energy.get());
        ROOT::Math::PxPyPzEVector electron_4v = electron_opt->PxPyPzEVector();
        ROOT::Math::PxPyPzEVector q_4v = beam_4v - electron_4v;
        auto q_3v = q_4v.Vect();
        double q_mag = std::sqrt(q_3v.Mag2());
        ROOT::Math::XYZVector q_hat = q_3v / q_mag;

        // Find DIS bin for pool organization
        int dis_bin = find_dis_bin(Q2, xB);

        // =====================================================================
        // 1. Fill same-event histograms and collect 4D bins for mixed-event use
        //    (no pool access, fully parallel)
        // =====================================================================
        // same_event_bins: the 4D bins where this event has same-event pi0 candidates.
        // Mixed-event masses will be filled into these same bins (not independently binned).
        std::vector<int> same_event_bins;

        if (photons.size() >= 2) {
            for (size_t i = 0; i < photons.size(); ++i) {
                for (size_t j = i + 1; j < photons.size(); ++j) {
                    auto g1_4v = photons[i].PxPyPzEVector();
                    auto g2_4v = photons[j].PxPyPzEVector();

                    // Apply photon pair validation cuts
                    if (!validate_photon_pair(electron_4v, g1_4v, g2_4v)) continue;

                    // Compute pi0 kinematics for this same-event pair
                    auto pi0_4v = g1_4v + g2_4v;
                    double mgg = pi0_4v.M();
                    double z = pi0_4v.E() / nu;
                    ROOT::Math::XYZVector pi0_3v(pi0_4v.Px(), pi0_4v.Py(), pi0_4v.Pz());
                    double p_parallel = pi0_3v.Dot(q_hat);
                    ROOT::Math::XYZVector p_perp = pi0_3v - p_parallel * q_hat;
                    double pT = std::sqrt(p_perp.Mag2());

                    // Fill inclusive same-event histogram
                    if (h_mgg_same_all) h_mgg_same_all->fill(mgg);

                    // Find 4D bin and fill binned histogram
                    int bin_4d = find_4d_bin(Q2, xB, z, pT);
                    if (bin_4d >= 0 && h_mgg_same.count(bin_4d)) {
                        h_mgg_same[bin_4d]->fill(mgg);
                        same_event_bins.push_back(bin_4d);
                    }

                    m_cutflow.pass("Same-event pairs");
                }
            }
        }

        // =====================================================================
        // 2. Fill mixed-event histograms (shared lock — concurrent reads)
        //    Mixed masses are assigned to the SAME 4D bins as same-event pairs,
        //    not independently binned. This ensures the background shape matches
        //    the same-event combinatorial background in each kinematic bin.
        // =====================================================================
        if (dis_bin >= 0 && !same_event_bins.empty()) {
            std::shared_lock lock(m_pool_mutex);

            auto pool_it = m_photon_pools.find(dis_bin);
            if (pool_it != m_photon_pools.end() && !pool_it->second.empty()) {
                for (const auto& stored_photons : pool_it->second) {
                    // Compute ALL valid mixed invariant masses:
                    // current event photons × stored event photons
                    std::vector<double> mixed_masses;
                    mixed_masses.reserve(photons.size() * stored_photons.size());

                    for (size_t i = 0; i < photons.size(); ++i) {
                        for (size_t j = 0; j < stored_photons.size(); ++j) {
                            auto g1_4v = photons[i].PxPyPzEVector();
                            auto g2_4v = stored_photons[j].PxPyPzEVector();

                            // Apply same pair validation cuts as same-event
                            if (!validate_photon_pair(electron_4v, g1_4v, g2_4v)) continue;

                            mixed_masses.push_back((g1_4v + g2_4v).M());
                        }
                    }

                    if (mixed_masses.empty()) continue;

                    // Fill all mixed masses into each same-event 4D bin
                    // (matching reference: event_mixing.cpp:162-169)
                    for (int bin_4d : same_event_bins) {
                        if (h_mgg_mixed.count(bin_4d)) {
                            for (double mgg : mixed_masses) {
                                h_mgg_mixed[bin_4d]->fill(mgg);
                            }
                        }
                    }

                    // Fill inclusive mixed histogram (once per stored event, not per bin)
                    for (double mgg : mixed_masses) {
                        if (h_mgg_mixed_all) h_mgg_mixed_all->fill(mgg);
                    }

                    m_cutflow.pass("Mixed-event pairs");
                }
            }
        }

        // =====================================================================
        // 3. Add current photons to pool (>= 1 photon sufficient)
        //    Exclusive lock — brief O(1) operation
        // =====================================================================
        if (dis_bin >= 0 && !photons.empty()) {
            std::unique_lock lock(m_pool_mutex);

            auto& pool = m_photon_pools[dis_bin];
            pool.push_back(photons);
            while (static_cast<int>(pool.size()) > m_max_pool_events.get()) {
                pool.pop_front();
            }
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        // Report pool statistics
        size_t total_stored = 0;
        size_t bins_used = 0;
        size_t max_occupancy = 0;
        for (const auto& [bin, pool] : m_photon_pools) {
            if (!pool.empty()) {
                bins_used++;
                total_stored += pool.size();
                max_occupancy = std::max(max_occupancy, pool.size());
            }
        }
        double avg_occupancy = bins_used > 0 ? static_cast<double>(total_stored) / bins_used : 0.0;
        Log::stat(name(), "DIS bins used", bins_used);
        Log::stat(name(), "Total events in pools", total_stored);
        Log::stat(name(), "Avg pool occupancy", fmt::format("{:.1f}", avg_occupancy));
        Log::stat(name(), "Max pool occupancy", max_occupancy);
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
    clas12::framework::Property<int> m_max_pool_events{"max_pool_events", 10, "Max events per DIS bin"};
    clas12::framework::Property<double> m_beam_energy{"beam_energy", 10.53, "Beam energy [GeV]"};

    // Service pointers (cached in on_initialize via svc<T>())
    clas12::framework::HistogramService* m_hist = nullptr;

    // Cutflow
    clas12::framework::Cutflow m_cutflow;

    // Photon pool: DIS bin index → FIFO deque of photon collections
    std::map<int, std::deque<std::vector<clas12::Particle>>> m_photon_pools;
    mutable std::shared_mutex m_pool_mutex;

    // Binning edges
    std::vector<double> m_Q2_edges = {1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 7.0, 11.0};
    std::vector<double> m_xB_edges = {0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.7};
    std::vector<double> m_z_edges = {0.0, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.8, 1.0};
    std::vector<double> m_pT_edges = {0.0, 0.15, 0.30, 0.50};

    // Histograms: per 4D bin
    std::map<int, clas12::framework::AtomicHistogram1D*> h_mgg_same;
    std::map<int, clas12::framework::AtomicHistogram1D*> h_mgg_mixed;

    // Histograms: inclusive
    clas12::framework::AtomicHistogram1D* h_mgg_same_all = nullptr;
    clas12::framework::AtomicHistogram1D* h_mgg_mixed_all = nullptr;

    // =========================================================================
    // Photon pair validation cuts (matching reference implementation)
    // =========================================================================

    // Sideband normalization region for offline background subtraction (GeV)
    // To extract pi0 signal: scale mixed histogram so that
    //   integral_same(sideband) == integral_mixed(sideband)
    // then subtract: h_signal = h_same - scale * h_mixed
    static constexpr double SIDEBAND_LOW = 0.17;
    static constexpr double SIDEBAND_HIGH = 0.30;
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

    /// Momentum-dependent minimum photon opening angle (degrees)
    static auto min_photon_opening_angle(double momentum) -> double {
        return MIN_PHOTON_ANGLE_A * std::exp(-MIN_PHOTON_ANGLE_B * momentum) + MIN_PHOTON_ANGLE_OFFSET;
    }

    /// Validate a photon pair against quality cuts
    /// Checks: electron-photon angles, invariant mass, momentum-dependent opening angle
    static auto validate_photon_pair(
        const ROOT::Math::PxPyPzEVector& electron_4v,
        const ROOT::Math::PxPyPzEVector& g1_4v,
        const ROOT::Math::PxPyPzEVector& g2_4v) -> bool {

        // Check electron-photon angle for each photon (> 8 degrees)
        double angle1 = ROOT::Math::VectorUtil::Angle(electron_4v, g1_4v) * 180.0 / M_PI;
        if (angle1 < MIN_ELECTRON_PHOTON_ANGLE) return false;

        double angle2 = ROOT::Math::VectorUtil::Angle(electron_4v, g2_4v) * 180.0 / M_PI;
        if (angle2 < MIN_ELECTRON_PHOTON_ANGLE) return false;

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

    auto find_dis_bin(double Q2, double xB) const -> int {
        int iq = find_1d_bin(Q2, m_Q2_edges);
        int ix = find_1d_bin(xB, m_xB_edges);
        if (iq < 0 || ix < 0) return -1;
        return iq * static_cast<int>(m_xB_edges.size() - 1) + ix;
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
            h_mgg_same_all = svc->book_atomic_hist1D(
                dir + "/mgg_same_all",
                "Same-event M_{#gamma#gamma};M_{#gamma#gamma} [GeV/c^{2}];Counts",
                200, 0.0, 0.3);
            h_mgg_mixed_all = svc->book_atomic_hist1D(
                dir + "/mgg_mixed_all",
                "Mixed-event M_{#gamma#gamma};M_{#gamma#gamma} [GeV/c^{2}];Counts",
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

                h_mgg_same[idx] = svc->book_atomic_hist1D(
                    fmt::format("{}/same/mgg_same_bin{}", dir, idx),
                    title_same, 200, 0.0, 0.3);

                h_mgg_mixed[idx] = svc->book_atomic_hist1D(
                    fmt::format("{}/mixed/mgg_mixed_bin{}", dir, idx),
                    title_mixed, 200, 0.0, 0.3);
            }

            Log::info("Pi0EventMixing", "Booked {} same + {} mixed histograms", total_bins, total_bins);
        }
    }
};

}  // namespace clas12::algorithms
