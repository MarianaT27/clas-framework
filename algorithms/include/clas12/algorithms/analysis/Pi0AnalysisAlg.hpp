#pragma once

/**
 * @file Pi0AnalysisAlg.hpp
 * @brief Combined pi0 multiplicity-ratio and event-mixing analysis.
 *
 * Merges the per-event work of the former Pi0MultiplicityRatioAlg and
 * Pi0EventMixingAlg into a single pass:
 *   - DIS-binned multiplicity histograms (Q2, xB, z, pT2, phi_h, 4D moments),
 *     filled from Keys::Pi0Candidates (mass-window pi0s).
 *   - Same-event and mixed-event m_gg histograms, filled from raw
 *     Keys::SelectedPhotons_Pi0 with a per-bin photon pool.
 *   - A flat per-pair TTree ("pi0_pairs") containing the same-event
 *     signal rows (mixed pairs stay in histograms only — see process()).
 */

#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <Math/Vector4D.h>
#include <Math/Vector3D.h>
#include <Math/GenVector/VectorUtil.h>

#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/QuantileBinner.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/CompositeParticle.hpp>

namespace algorithms {

using framework::Log;

struct Pi0AnalysisConfig {
    int max_pool_events = 50;

    std::vector<double> Q2_edges  = {1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 7.0, 11.0};
    std::vector<double> xB_edges  = {0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5, 0.7};

    // Multiplicity binning (was Pi0MultiplicityRatioConfig).
    std::vector<double> z_edges   = {0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
    std::vector<double> pT2_edges = {0.0, 0.05, 0.1, 0.2, 0.3, 0.5, 0.7, 1.0, 1.5};

    // Mixing binning (was Pi0EventMixingConfig).
    std::vector<double> z_mix_edges  = {0.0, 0.15, 0.25, 0.35, 0.45, 0.55, 0.65, 0.8, 1.0};
    std::vector<double> pT_mix_edges = {0.0, 0.15, 0.30, 0.50};
    std::vector<double> W_edges      = {2.0, 2.5, 3.0, 3.5, 4.0, 5.0};
    std::vector<double> nu_edges     = {2.0, 4.0, 5.5, 7.0, 9.0, 11.0};

    bool   enable_ntuple = true;
    double beam_energy   = 10.53;

    // Equal-statistics binning scan. When enabled, the algorithm samples the
    // real Q^2 / x_B (per DIS event) and z / p_T^2 (per pi0 candidate)
    // distributions during the run and, at finalize, prints + writes
    // quantile edges so that each bin holds ~the same number of entries —
    // replacing the hand-chosen *_edges above. The emitted edges keep the
    // same bin *count* as the current *_edges, so they are a drop-in. The
    // scan is purely additive (it doesn't change the histograms produced by
    // this run); adopt the edges by pasting the written TOML into the
    // [Pi0Analysis] section, then re-run. Best run single-process over a
    // representative subset (quantiles converge with ~1e5-1e6 samples), not
    // sharded via local_batch.
    bool        scan_quantile_binning  = false;
    int         quantile_sample_cap    = 1'000'000;  // reservoir size per variable
    std::string quantile_binning_output = "pi0_equal_stat_binning.toml";

    static constexpr auto config_fields() {
        using T = Pi0AnalysisConfig;
        return std::make_tuple(
            framework::field("max_pool_events", &T::max_pool_events),
            framework::field("Q2_edges",        &T::Q2_edges),
            framework::field("xB_edges",        &T::xB_edges),
            framework::field("z_edges",         &T::z_edges),
            framework::field("pT2_edges",       &T::pT2_edges),
            framework::field("z_mix_edges",     &T::z_mix_edges),
            framework::field("pT_mix_edges",    &T::pT_mix_edges),
            framework::field("W_edges",         &T::W_edges),
            framework::field("nu_edges",        &T::nu_edges),
            framework::field("enable_ntuple",   &T::enable_ntuple),
            framework::field("beam_energy",     &T::beam_energy),
            framework::field("scan_quantile_binning",   &T::scan_quantile_binning),
            framework::field("quantile_sample_cap",     &T::quantile_sample_cap),
            framework::field("quantile_binning_output", &T::quantile_binning_output));
    }
};

struct Pi0PairNTupleData {
    // 0 = same-event. Mixed-event pairs are intentionally not written to
    // the TTree (kept in /Pi0EventMixing histograms only) — see the
    // process_photon_pairs comment. The field stays for forward
    // compatibility in case mixed rows are reinstated.
    int     pair_type = 0;
    long    event_id  = 0;
    int     n_photons = 0;
    int     helicity  = 0;

    double  Q2 = 0.0;
    double  xB = 0.0;
    double  nu = 0.0;
    double  W  = 0.0;
    double  y  = 0.0;

    double  mgg   = 0.0;
    double  z     = 0.0;
    double  pT    = 0.0;
    double  pT2   = 0.0;
    double  phi_h = 0.0;

    double  pi0_px = 0.0;
    double  pi0_py = 0.0;
    double  pi0_pz = 0.0;
    double  pi0_E  = 0.0;

    int iQ2      = -1;
    int ixB      = -1;
    int iz_mult  = -1;
    int ipT2_mult = -1;
    int iz_mix   = -1;
    int ipT_mix  = -1;

    static constexpr auto ntuple_fields() {
        using T = Pi0PairNTupleData;
        return std::make_tuple(
            framework::field("pair_type", &T::pair_type),
            framework::field("event_id",  &T::event_id),
            framework::field("n_photons", &T::n_photons),
            framework::field("helicity",  &T::helicity),
            framework::field("Q2",        &T::Q2),
            framework::field("xB",        &T::xB),
            framework::field("nu",        &T::nu),
            framework::field("W",         &T::W),
            framework::field("y",         &T::y),
            framework::field("mgg",       &T::mgg),
            framework::field("z",         &T::z),
            framework::field("pT",        &T::pT),
            framework::field("pT2",       &T::pT2),
            framework::field("phi_h",     &T::phi_h),
            framework::field("pi0_px",    &T::pi0_px),
            framework::field("pi0_py",    &T::pi0_py),
            framework::field("pi0_pz",    &T::pi0_pz),
            framework::field("pi0_E",     &T::pi0_E),
            framework::field("iQ2",       &T::iQ2),
            framework::field("ixB",       &T::ixB),
            framework::field("iz_mult",   &T::iz_mult),
            framework::field("ipT2_mult", &T::ipT2_mult),
            framework::field("iz_mix",    &T::iz_mix),
            framework::field("ipT_mix",   &T::ipT_mix));
    }
};

class Pi0AnalysisAlg : public framework::Algorithm {
   public:
    using Config = Pi0AnalysisConfig;
    using StatusCode = framework::StatusCode;

    explicit Pi0AnalysisAlg(std::string name = "Pi0Analysis", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Pi0 Analysis") {
        required_services(m_hist, m_ntuple);
        declare_banks("REC::Event");
        declare_config(m_config);
        declare_ntuple(m_ntuple_writer, "pi0_pairs", m_config.enable_ntuple);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Beam energy: {} GeV", m_config.beam_energy);
        Log::info(name(), "Max pool events: {}", m_config.max_pool_events);
        Log::info(name(), "Multiplicity 4D bins (Q2 x xB x z x pT2): {} x {} x {} x {} = {}",
                  m_config.Q2_edges.size() - 1, m_config.xB_edges.size() - 1,
                  m_config.z_edges.size() - 1, m_config.pT2_edges.size() - 1, total_mult_4D_bins());
        Log::info(name(), "Mixing 4D bins (Q2 x xB x z_mix x pT_mix): {} x {} x {} x {} = {}",
                  m_config.Q2_edges.size() - 1, m_config.xB_edges.size() - 1,
                  m_config.z_mix_edges.size() - 1, m_config.pT_mix_edges.size() - 1, total_mix_4D_bins());
        Log::info(name(), "Pool bins (Q2 x xB x W x mult x nu): {} x {} x {} x {} x {} = {}",
                  m_config.Q2_edges.size() - 1, m_config.xB_edges.size() - 1,
                  m_config.W_edges.size() - 1, N_MULT_BINS, m_config.nu_edges.size() - 1,
                  (m_config.Q2_edges.size() - 1) * (m_config.xB_edges.size() - 1) *
                      (m_config.W_edges.size() - 1) * N_MULT_BINS * (m_config.nu_edges.size() - 1));

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has DIS Electron");
        m_cutflow.add_cut("Has Pi0");
        m_cutflow.add_cut("z in range");
        m_cutflow.add_cut("Has Photons");
        m_cutflow.add_cut("Same-event pairs");
        m_cutflow.add_cut("Mixed-event pairs");

        book_histograms();

        if (m_config.scan_quantile_binning) {
            const auto cap = static_cast<std::size_t>(
                std::max(1, m_config.quantile_sample_cap));
            m_qb_Q2  = std::make_unique<framework::QuantileBinner>(cap);
            m_qb_xB  = std::make_unique<framework::QuantileBinner>(cap);
            m_qb_z   = std::make_unique<framework::QuantileBinner>(cap);
            m_qb_pT2 = std::make_unique<framework::QuantileBinner>(cap);
            Log::info(name(),
                      "Equal-statistics binning scan ENABLED "
                      "(reservoir cap {} / variable -> '{}')",
                      cap, m_config.quantile_binning_output);
        }

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        if (!event.passes_decision_typed(framework::Decisions::HasGoodDISElectron)) return StatusCode::FILTERED();
        m_cutflow.pass("Has DIS Electron");

        auto dis = framework::EventHelpers::get_dis_kinematics(event);
        if (!dis) return StatusCode::FILTERED();
        const double Q2 = dis->Q2;
        const double xB = dis->xB;
        const double nu = dis->nu;
        const double W  = dis->W;
        const double y  = dis->y;

        auto electron_opt = framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::SUCCESS();

        auto& rec_event = event.bank<bankdefs::REC_Event>();
        const int helicity = rec_event.rows() > 0 ? rec_event.helicity(0) : 0;

        ROOT::Math::PxPyPzEVector beam_4v(0.0, 0.0, m_config.beam_energy, m_config.beam_energy);
        ROOT::Math::PxPyPzEVector electron_4v = electron_opt->PxPyPzEVector();
        ROOT::Math::PxPyPzEVector q_4v = beam_4v - electron_4v;
        auto q_3v = q_4v.Vect();
        double q_mag = std::sqrt(q_3v.Mag2());
        ROOT::Math::XYZVector q_hat = q_3v / q_mag;

        ROOT::Math::XYZVector beam_3v(0.0, 0.0, m_config.beam_energy);
        ROOT::Math::XYZVector y_hat = q_3v.Cross(beam_3v);
        double y_mag = std::sqrt(y_hat.Mag2());
        if (y_mag > 0) y_hat = y_hat / y_mag;
        ROOT::Math::XYZVector x_hat = y_hat.Cross(q_hat);

        const int iQ2 = find_1d_bin(Q2, m_config.Q2_edges);
        const int ixB = find_1d_bin(xB, m_config.xB_edges);

        fill_dis_normalization(Q2, xB, iQ2, ixB);

        fill_multiplicity_histograms(event, Q2, nu, q_hat, x_hat, y_hat, iQ2, ixB);

        const long event_id = event.event_number();
        // Reuse a thread-local row buffer instead of constructing a fresh
        // vector each event — keeps the allocator warm so a multi-photon
        // event doesn't re-grow it. Cleared, not destroyed, so the
        // underlying capacity sticks around for the next pi0-producing
        // event on this worker thread.
        thread_local std::vector<Pi0PairNTupleData> local_rows;
        local_rows.clear();
        process_photon_pairs(event, Q2, xB, nu, W, y, q_hat, x_hat, y_hat,
                             electron_4v, iQ2, ixB, event_id, helicity, local_rows);

        flush_rows_to_ntuple(local_rows);

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        size_t total_stored = 0;
        size_t bins_used = 0;
        size_t max_occupancy = 0;
        size_t total_photons = 0;
        size_t max_photons = 0;
        for (auto& shard : m_pool_shards) {
            std::shared_lock lock(shard.mutex);
            for (const auto& [bin, pool] : shard.pools) {
                if (pool.event_sizes.empty()) continue;
                bins_used++;
                total_stored += pool.event_sizes.size();
                max_occupancy = std::max(max_occupancy, pool.event_sizes.size());
                total_photons += pool.photons.size();
                max_photons = std::max(max_photons, pool.photons.size());
            }
        }
        double avg_occupancy = bins_used > 0 ? static_cast<double>(total_stored) / bins_used : 0.0;
        Log::info(name(), "Pool bins used: {}", bins_used);
        Log::info(name(), "Total events in pools: {}", total_stored);
        Log::info(name(), "Total photons in pools: {}", total_photons);
        Log::info(name(), "Avg event occupancy: {:.1f}", avg_occupancy);
        Log::info(name(), "Max event occupancy: {}", max_occupancy);
        Log::info(name(), "Max photon pool size: {}", max_photons);
        Log::info(name(), "Offline normalization: scale mixed to same in sideband [{:.2f}, {:.2f}] GeV", SIDEBAND_LOW, SIDEBAND_HIGH);
        Log::info(name(), "Offline fit region: [{:.2f}, {:.2f}] GeV", FIT_RANGE_LOW, FIT_RANGE_HIGH);
        if (m_ntuple) Log::info(name(), "N-tuple 'pi0_pairs' has {} entries", m_ntuple->get_entries("pi0_pairs"));

        report_equal_stat_binning();

        Log::blank();
        m_cutflow.print();
        Log::success(name(), "Finalization complete");

        return StatusCode::SUCCESS();
    }

    /// Format a list of edges as a TOML / C++ array literal, e.g. "[1, 1.5, 2]".
    static auto format_edges(const std::vector<double>& edges) -> std::string {
        std::string s = "[";
        for (std::size_t i = 0; i < edges.size(); ++i) {
            s += fmt::format("{}{:.4g}", i ? ", " : "", edges[i]);
        }
        s += "]";
        return s;
    }

    /// Compute equal-statistics edges for Q2, xB, z and pT2 from the sampled
    /// distributions, log them, and write a TOML snippet that can be pasted
    /// straight into the [Pi0Analysis] config section. The bin *count* for
    /// each variable matches the current *_edges, so the result is a drop-in
    /// replacement. No-op unless scan_quantile_binning was enabled.
    auto report_equal_stat_binning() -> void {
        if (!m_qb_Q2) return;

        struct Var {
            const char*                name;
            framework::QuantileBinner* binner;
            int                        n_bins;
            std::vector<double>        edges;  // filled below
        };
        std::array<Var, 4> vars{{
            {"Q2_edges",  m_qb_Q2.get(),  static_cast<int>(m_config.Q2_edges.size())  - 1, {}},
            {"xB_edges",  m_qb_xB.get(),  static_cast<int>(m_config.xB_edges.size())  - 1, {}},
            {"z_edges",   m_qb_z.get(),   static_cast<int>(m_config.z_edges.size())   - 1, {}},
            {"pT2_edges", m_qb_pT2.get(), static_cast<int>(m_config.pT2_edges.size()) - 1, {}},
        }};

        Log::blank();
        Log::info(name(), "Equal-statistics binning scan results:");
        bool all_ok = true;
        for (auto& v : vars) {
            v.edges = v.binner->equal_stat_edges(v.n_bins);
            if (v.edges.empty()) {
                all_ok = false;
                Log::warning(name(),
                             "  {}: too few samples ({}) for {} bins — keeping current edges",
                             v.name, v.binner->count(), v.n_bins);
                continue;
            }
            Log::info(name(), "  {} ({} bins, {} sampled / {} seen): {}",
                      v.name, v.n_bins, v.binner->sample_size(), v.binner->count(),
                      format_edges(v.edges));
        }

        // Write the TOML snippet (only the variables that produced edges).
        const std::string& path = m_config.quantile_binning_output;
        std::ofstream out(path);
        if (!out) {
            Log::warning(name(), "Could not open '{}' to write equal-stat binning", path);
            return;
        }
        out << "# Equal-statistics (quantile) binning for Pi0AnalysisAlg,\n"
               "# generated by the scan_quantile_binning pass. Each bin holds\n"
               "# ~the same number of entries. Paste into the [Pi0Analysis]\n"
               "# section of your analysis TOML (and set scan_quantile_binning\n"
               "# = false) to adopt it on the next run.\n"
               "[Pi0Analysis]\n";
        for (const auto& v : vars) {
            if (v.edges.empty()) {
                out << fmt::format("# {} = (insufficient samples; left at default)\n", v.name);
            } else {
                out << fmt::format("{} = {}\n", v.name, format_edges(v.edges));
            }
        }
        Log::info(name(), "Wrote equal-statistics binning to '{}'{}",
                  path, all_ok ? "" : " (some variables fell back to defaults)");
    }

   private:
    /// Per-photon (or per-electron) cached kinematic snapshot used by the
    /// pair validators. Storing px/py/pz/E plus 1/|p| lets the inner loops
    /// compare angles by cosine (dot * inv_p1 * inv_p2 vs cos(θ_min))
    /// instead of going through `acos` + `sqrt` for every pair tested.
    /// Defined here (ahead of `PhotonPool`) so `std::vector<PhotonCache>`
    /// in the pool member is complete-typed at member-declaration time.
    struct PhotonCache {
        double px = 0.0, py = 0.0, pz = 0.0, E = 0.0, inv_p = 0.0;
        [[nodiscard]] auto PxPyPzEVector() const noexcept -> ROOT::Math::PxPyPzEVector {
            return ROOT::Math::PxPyPzEVector{px, py, pz, E};
        }
    };

    Config m_config;
    framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService>    m_ntuple;

    // Equal-statistics binning samplers. Allocated in on_initialize only
    // when m_config.scan_quantile_binning is set, so a normal run pays
    // nothing. Q2/xB sample one value per DIS event; z/pT2 one per pi0
    // candidate that fills the multiplicity histograms.
    std::unique_ptr<framework::QuantileBinner> m_qb_Q2;
    std::unique_ptr<framework::QuantileBinner> m_qb_xB;
    std::unique_ptr<framework::QuantileBinner> m_qb_z;
    std::unique_ptr<framework::QuantileBinner> m_qb_pT2;
    framework::NTupleWriter<Pi0PairNTupleData>         m_ntuple_writer;

    // -------- Pool ------------------------------------------------------------
    /// Per-(Q^2, x_B, W, mult, ν)-bin photon pool. Stores the cached
    /// kinematic snapshot rather than the heavier `Particle` so the
    /// mixed-event loop sees a contiguous array of 5-double records.
    struct PhotonPool {
        std::vector<PhotonCache> photons;
        std::deque<size_t>       event_sizes;
    };

    /// Sharded pool. Each shard owns an independent `shared_mutex` +
    /// `unordered_map<bin, PhotonPool>`. Different pool bins map to
    /// different shards via the bin modulus, so two worker threads
    /// updating different (Q^2, x_B, …) bins never block each other.
    /// 32 shards covers the ~hundreds of populated bins observed in
    /// the LD2 / CuSn analyses with comfortable headroom.
    static constexpr int N_POOL_SHARDS = 32;
    struct PoolShard {
        mutable std::shared_mutex          mutex;
        std::unordered_map<int, PhotonPool> pools;
    };
    std::array<PoolShard, N_POOL_SHARDS> m_pool_shards;

    [[nodiscard]] auto pool_shard_for(int bin) noexcept -> PoolShard& {
        // Bins are non-negative once they're emitted by find_pool_bin;
        // a plain modulus is enough for the distribution we see.
        return m_pool_shards[static_cast<unsigned>(bin) % static_cast<unsigned>(N_POOL_SHARDS)];
    }
    [[nodiscard]] auto pool_shard_for(int bin) const noexcept -> const PoolShard& {
        return m_pool_shards[static_cast<unsigned>(bin) % static_cast<unsigned>(N_POOL_SHARDS)];
    }

    static constexpr int N_MULT_BINS = 4;

    // -------- Constants -------------------------------------------------------
    static constexpr double SIDEBAND_LOW   = 0.19;
    static constexpr double SIDEBAND_HIGH  = 0.24;
    static constexpr double FIT_RANGE_LOW  = 0.10;
    static constexpr double FIT_RANGE_HIGH = 0.16;

    static constexpr double MIN_ELECTRON_PHOTON_ANGLE = 8.0;
    static constexpr double MIN_INVARIANT_MASS        = 0.001;
    static constexpr double MIN_PHOTON_ANGLE_A        = 17.561;
    static constexpr double MIN_PHOTON_ANGLE_B        = 0.756;
    static constexpr double MIN_PHOTON_ANGLE_OFFSET   = 1.0;

    // -------- Multiplicity histograms (Pi0Multiplicity directory) -------------
    framework::H1<> h_n_DIS_events{mh::regular(0, 1, 1)};
    framework::H1<> h_n_DIS_vs_Q2{mh::regular(0, 12, 100)};
    framework::H1<> h_n_DIS_vs_xB{mh::regular(0, 0.8, 100)};
    std::vector<std::unique_ptr<framework::H1<>>> h_n_DIS_Q2bin;
    std::vector<std::unique_ptr<framework::H1<>>> h_n_DIS_xBbin;

    // DIS-electron counts on the analysis (Q^2 x x_B) binning, used offline
    // (Python) as the denominator of the multiplicity ratio R_A. The variable
    // axes carry the physics bin edges directly into the ROOT file so the
    // downstream code does not need to redo the binning logic.
    using H2VarVar = mh::Histogram<mh::CountAccumulator,
                                   mh::AtomicStorage<mh::CountAccumulator>,
                                   mh::VariableAxis, mh::VariableAxis>;
    std::unique_ptr<H2VarVar> h_n_DIS_Q2_vs_xB;

    framework::H1<> h_z{mh::regular(0, 1, 100)};
    framework::H1<> h_pT2{mh::regular(0, 2, 100)};
    framework::H1<> h_phi_h{mh::regular(-180, 180, 72)};
    framework::H1<> h_pi0_mass{mh::regular(0, 0.5, 200)};

    framework::H2<> h_z_vs_Q2{mh::regular(0, 12, 100), mh::regular(0, 1, 100)};
    framework::H2<> h_z_vs_pT2{mh::regular(0, 2, 100), mh::regular(0, 1, 100)};
    framework::H2<> h_pT2_vs_Q2{mh::regular(0, 12, 100), mh::regular(0, 2, 100)};

    std::vector<std::unique_ptr<framework::H1<>>> h_z_Q2bin;
    std::vector<std::unique_ptr<framework::H1<>>> h_pT2_Q2bin;
    std::vector<std::unique_ptr<framework::H1<>>> h_z_xBbin;
    std::vector<std::unique_ptr<framework::H1<>>> h_pT2_zbin;

    std::unique_ptr<framework::H1<>> h_counts_4D;
    std::unique_ptr<framework::H1<mh::WeightedAccumulator>> h_sum_pT2_4D;
    std::unique_ptr<framework::H1<mh::WeightedAccumulator>> h_sum_cosphi_4D;
    std::unique_ptr<framework::H1<mh::WeightedAccumulator>> h_sum_sinphi_4D;

    // -------- Mixing histograms (Pi0EventMixing directory) --------------------
    framework::H1<> h_mgg_same_all{mh::regular(0.0, 0.3, 200)};
    framework::H1<> h_mgg_mixed_all{mh::regular(0.0, 0.3, 200)};
    std::map<int, std::unique_ptr<framework::H1<>>> h_mgg_same;
    std::map<int, std::unique_ptr<framework::H1<>>> h_mgg_mixed;

    // -------- Helpers ---------------------------------------------------------
    static auto min_photon_opening_angle(double momentum) -> double {
        return MIN_PHOTON_ANGLE_A * std::exp(-MIN_PHOTON_ANGLE_B * momentum) + MIN_PHOTON_ANGLE_OFFSET;
    }

    static auto make_cache(const ROOT::Math::PxPyPzEVector& v) noexcept -> PhotonCache {
        const double p3 = std::sqrt(v.Px() * v.Px() + v.Py() * v.Py() + v.Pz() * v.Pz());
        return {v.Px(), v.Py(), v.Pz(), v.E(), p3 > 0.0 ? 1.0 / p3 : 0.0};
    }
    static auto make_cache(const Particle& p) noexcept -> PhotonCache {
        return make_cache(p.PxPyPzEVector());
    }

    /// True if the angle between `a` and `b` exceeds `theta_min_deg`.
    /// Implements the inequality `cos(θ_ab) < cos(θ_min)` using cached
    /// inverse momenta to avoid `acos` and the per-call `sqrt` of |a||b|.
    static auto angle_exceeds(const PhotonCache& a, const PhotonCache& b,
                              double cos_theta_min) noexcept -> bool {
        const double dot = a.px * b.px + a.py * b.py + a.pz * b.pz;
        return (dot * a.inv_p * b.inv_p) < cos_theta_min;
    }

    /// Squared γγ invariant mass — `(E1+E2)² − |p1+p2|²` using the
    /// cached 4-momenta directly (avoids the temporary
    /// `PxPyPzEVector::operator+` and `M()` call).
    static auto mgg_squared(const PhotonCache& g1, const PhotonCache& g2) noexcept -> double {
        const double dE  = g1.E  + g2.E;
        const double dpx = g1.px + g2.px;
        const double dpy = g1.py + g2.py;
        const double dpz = g1.pz + g2.pz;
        return dE * dE - (dpx * dpx + dpy * dpy + dpz * dpz);
    }

    /// Magnitude of the pair's 3-momentum (used by the opening-angle
    /// threshold that scales with pi0 momentum).
    static auto pair_momentum(const PhotonCache& g1, const PhotonCache& g2) noexcept -> double {
        const double dpx = g1.px + g2.px;
        const double dpy = g1.py + g2.py;
        const double dpz = g1.pz + g2.pz;
        return std::sqrt(dpx * dpx + dpy * dpy + dpz * dpz);
    }

    // Pre-computed cosine of the fixed e-γ angle threshold; used as the
    // inequality target in `angle_exceeds`. Computed at compile time
    // so the inner loop never re-evaluates std::cos.
    static inline const double COS_MIN_ELECTRON_PHOTON_ANGLE =
        std::cos(MIN_ELECTRON_PHOTON_ANGLE * M_PI / 180.0);
    static constexpr double MIN_INVARIANT_MASS_SQUARED =
        MIN_INVARIANT_MASS * MIN_INVARIANT_MASS;

    auto find_1d_bin(double value, const std::vector<double>& edges) const -> int {
        for (size_t i = 0; i + 1 < edges.size(); ++i) {
            if (value >= edges[i] && value < edges[i + 1]) return static_cast<int>(i);
        }
        return -1;
    }

    static auto find_multiplicity_bin(size_t n_photons) -> int {
        if (n_photons == 0) return -1;
        if (n_photons >= 4) return 3;
        return static_cast<int>(n_photons) - 1;
    }

    auto find_pool_bin(double Q2, double xB, double W, size_t n_photons, double nu) const -> int {
        int iq  = find_1d_bin(Q2, m_config.Q2_edges);
        int ix  = find_1d_bin(xB, m_config.xB_edges);
        int iw  = find_1d_bin(W,  m_config.W_edges);
        int im  = find_multiplicity_bin(n_photons);
        int inu = find_1d_bin(nu, m_config.nu_edges);
        if (iq < 0 || ix < 0 || iw < 0 || im < 0 || inu < 0) return -1;
        int n_xB = static_cast<int>(m_config.xB_edges.size() - 1);
        int n_W  = static_cast<int>(m_config.W_edges.size() - 1);
        int n_nu = static_cast<int>(m_config.nu_edges.size() - 1);
        return (((iq * n_xB + ix) * n_W + iw) * N_MULT_BINS + im) * n_nu + inu;
    }

    auto find_mix_4d_bin(double Q2, double xB, double z, double pT) const -> int {
        int iq = find_1d_bin(Q2, m_config.Q2_edges);
        int ix = find_1d_bin(xB, m_config.xB_edges);
        int iz = find_1d_bin(z,  m_config.z_mix_edges);
        int ip = find_1d_bin(pT, m_config.pT_mix_edges);
        if (iq < 0 || ix < 0 || iz < 0 || ip < 0) return -1;
        int n_xB = static_cast<int>(m_config.xB_edges.size() - 1);
        int n_z  = static_cast<int>(m_config.z_mix_edges.size() - 1);
        int n_pT = static_cast<int>(m_config.pT_mix_edges.size() - 1);
        return ((iq * n_xB + ix) * n_z + iz) * n_pT + ip;
    }

    [[nodiscard]] auto flat_mult_4D_index(int iQ2, int ixB, int iz, int ipT2) const -> int {
        int n_xB = static_cast<int>(m_config.xB_edges.size())  - 1;
        int n_z  = static_cast<int>(m_config.z_edges.size())   - 1;
        int n_pT2 = static_cast<int>(m_config.pT2_edges.size())- 1;
        return ((iQ2 * n_xB + ixB) * n_z + iz) * n_pT2 + ipT2;
    }

    [[nodiscard]] auto total_mult_4D_bins() const -> int {
        return static_cast<int>((m_config.Q2_edges.size() - 1) * (m_config.xB_edges.size() - 1) *
                                (m_config.z_edges.size() - 1)  * (m_config.pT2_edges.size() - 1));
    }

    [[nodiscard]] auto total_mix_4D_bins() const -> int {
        return static_cast<int>((m_config.Q2_edges.size() - 1) * (m_config.xB_edges.size() - 1) *
                                (m_config.z_mix_edges.size() - 1) * (m_config.pT_mix_edges.size() - 1));
    }

    struct MixBinIndices { int iq, ix, iz, ip; };

    auto decompose_mix_4d_bin(int flat_idx) const -> MixBinIndices {
        int n_xB = static_cast<int>(m_config.xB_edges.size() - 1);
        int n_z  = static_cast<int>(m_config.z_mix_edges.size() - 1);
        int n_pT = static_cast<int>(m_config.pT_mix_edges.size() - 1);
        int ip = flat_idx % n_pT;
        flat_idx /= n_pT;
        int iz = flat_idx % n_z;
        flat_idx /= n_z;
        int ix = flat_idx % n_xB;
        int iq = flat_idx / n_xB;
        return {iq, ix, iz, ip};
    }

    // -------- DIS normalization ----------------------------------------------
    auto fill_dis_normalization(double Q2, double xB, int iQ2, int ixB) -> void {
        // Equal-stat binning sample: one (Q2, xB) per DIS event — the same
        // population the Q2/xB binning has to balance.
        if (m_qb_Q2) { m_qb_Q2->add(Q2); m_qb_xB->add(xB); }

        h_n_DIS_events.fill(0.5);
        h_n_DIS_vs_Q2.fill(Q2);
        h_n_DIS_vs_xB.fill(xB);
        if (iQ2 >= 0 && iQ2 < static_cast<int>(h_n_DIS_Q2bin.size())) h_n_DIS_Q2bin[iQ2]->fill(0.5);
        if (ixB >= 0 && ixB < static_cast<int>(h_n_DIS_xBbin.size())) h_n_DIS_xBbin[ixB]->fill(0.5);
        // (Q^2, x_B) 2D count on the analysis binning — flow bins capture
        // events outside the kinematic grid so they remain accountable
        // offline.
        if (h_n_DIS_Q2_vs_xB) h_n_DIS_Q2_vs_xB->fill(Q2, xB);
    }

    // -------- Multiplicity histograms ----------------------------------------
    auto fill_multiplicity_histograms(framework::Event& event,
                                      double Q2, double nu,
                                      const ROOT::Math::XYZVector& q_hat,
                                      const ROOT::Math::XYZVector& x_hat,
                                      const ROOT::Math::XYZVector& y_hat,
                                      int iQ2, int ixB) -> void {
        if (!event.passes_decision_typed(framework::Decisions::HasPi0)) return;
        auto pi0_opt = event.retrieve_typed(framework::Keys::Pi0Candidates);
        if (!pi0_opt || pi0_opt->empty()) return;
        m_cutflow.pass("Has Pi0");

        for (const auto& pi0 : *pi0_opt) {
            double E_pi0 = pi0.energy();
            double z = E_pi0 / nu;

            ROOT::Math::XYZVector pi0_3v(pi0.px(), pi0.py(), pi0.pz());
            double p_parallel = pi0_3v.Dot(q_hat);
            ROOT::Math::XYZVector p_perp = pi0_3v - p_parallel * q_hat;
            double pT2 = p_perp.Mag2();
            double phi_h = std::atan2(p_perp.Dot(y_hat), p_perp.Dot(x_hat)) * 180.0 / M_PI;

            if (z <= 0.0 || z >= 1.0) continue;
            m_cutflow.pass("z in range");

            // Equal-stat binning sample: one (z, pT2) per pi0 candidate that
            // enters the multiplicity histograms — same population as h_z/h_pT2.
            if (m_qb_z) { m_qb_z->add(z); m_qb_pT2->add(pT2); }

            double pi0_mass = pi0.mass();

            h_z.fill(z);
            h_pT2.fill(pT2);
            h_phi_h.fill(phi_h);
            h_pi0_mass.fill(pi0_mass);
            h_z_vs_Q2.fill(Q2, z);
            h_z_vs_pT2.fill(pT2, z);
            h_pT2_vs_Q2.fill(Q2, pT2);

            int iz   = find_1d_bin(z,   m_config.z_edges);
            int ipT2 = find_1d_bin(pT2, m_config.pT2_edges);

            if (iQ2 >= 0) {
                h_z_Q2bin[iQ2]->fill(z);
                h_pT2_Q2bin[iQ2]->fill(pT2);
            }
            if (ixB >= 0) h_z_xBbin[ixB]->fill(z);
            if (iz  >= 0) h_pT2_zbin[iz]->fill(pT2);

            if (iQ2 >= 0 && ixB >= 0 && iz >= 0 && ipT2 >= 0) {
                double phi_h_rad = phi_h * M_PI / 180.0;
                double idx = static_cast<double>(flat_mult_4D_index(iQ2, ixB, iz, ipT2)) + 0.5;
                h_counts_4D->fill(idx);
                h_sum_pT2_4D->fill_weighted(pT2, idx);
                h_sum_cosphi_4D->fill_weighted(std::cos(phi_h_rad), idx);
                h_sum_sinphi_4D->fill_weighted(std::sin(phi_h_rad), idx);
            }
        }
    }

    // -------- Pair iteration (same + mixed) ---------------------------------
    auto process_photon_pairs(framework::Event& event,
                              double Q2, double xB, double nu, double W, double y,
                              const ROOT::Math::XYZVector& q_hat,
                              const ROOT::Math::XYZVector& x_hat,
                              const ROOT::Math::XYZVector& y_hat,
                              const ROOT::Math::PxPyPzEVector& electron_4v,
                              int iQ2, int ixB,
                              long event_id,
                              int helicity,
                              std::vector<Pi0PairNTupleData>& local_rows) -> void {
        auto photons_opt = event.retrieve_typed(framework::Keys::SelectedPhotons_Pi0);
        if (!photons_opt || photons_opt->empty()) return;
        m_cutflow.pass("Has Photons");

        const auto& photons = *photons_opt;
        const int n_photons = static_cast<int>(photons.size());
        const int pool_bin = find_pool_bin(Q2, xB, W, photons.size(), nu);

        // Cache the kinematic snapshots once per event, plus the e-γ
        // angle precheck for each current-event photon. Same-event pairs
        // only iterate photon indices whose e-γ check already passed;
        // mixed-event pairs reuse the same precheck for the "g1" side.
        // The caches and the gate vector are thread-local so successive
        // events on the same worker thread keep the allocator warm.
        thread_local std::vector<PhotonCache> current_cache;
        thread_local std::vector<char>        current_e_g_ok;
        const PhotonCache electron_cache = make_cache(electron_4v);
        current_cache.resize(photons.size());
        current_e_g_ok.resize(photons.size());
        for (size_t i = 0; i < photons.size(); ++i) {
            current_cache[i]   = make_cache(photons[i]);
            current_e_g_ok[i]  = angle_exceeds(electron_cache, current_cache[i],
                                               COS_MIN_ELECTRON_PHOTON_ANGLE) ? 1 : 0;
        }

        // Same-event pairs
        if (photons.size() >= 2) {
            for (size_t i = 0; i < photons.size(); ++i) {
                if (!current_e_g_ok[i]) continue;
                const auto& g1 = current_cache[i];
                for (size_t j = i + 1; j < photons.size(); ++j) {
                    if (!current_e_g_ok[j]) continue;
                    const auto& g2 = current_cache[j];

                    const double mass_sq = mgg_squared(g1, g2);
                    if (mass_sq < MIN_INVARIANT_MASS_SQUARED) continue;
                    const double momentum = pair_momentum(g1, g2);
                    const double cos_min_open =
                        std::cos(min_photon_opening_angle(momentum) * M_PI / 180.0);
                    if (!angle_exceeds(g1, g2, cos_min_open)) continue;

                    auto pi0_4v = g1.PxPyPzEVector() + g2.PxPyPzEVector();
                    PairKinematics k = compute_pair_kinematics(pi0_4v, q_hat, x_hat, y_hat, nu);

                    h_mgg_same_all.fill(k.mgg);
                    int bin_4d = find_mix_4d_bin(Q2, xB, k.z, k.pT);
                    if (bin_4d >= 0) {
                        auto it = h_mgg_same.find(bin_4d);
                        if (it != h_mgg_same.end()) it->second->fill(k.mgg);
                    }
                    m_cutflow.pass("Same-event pairs");

                    local_rows.push_back(make_row(/*pair_type=*/0, event_id, n_photons, helicity,
                                                  Q2, xB, nu, W, y, pi0_4v, k,
                                                  iQ2, ixB));
                }
            }
        }

        // Mixed-event pairs: current event photons against the entire
        // photon pool. Histogram-only (no TTree row): the per-pair fill
        // rate here dwarfs every other path. The pool stores photons as
        // pre-computed `PhotonCache`s so the j-loop never reconstructs
        // a 4-vector that ends up rejected.
        if (pool_bin >= 0 && !photons.empty()) {
            auto& shard = pool_shard_for(pool_bin);
            std::shared_lock lock(shard.mutex);
            auto pool_it = shard.pools.find(pool_bin);
            if (pool_it != shard.pools.end() && !pool_it->second.photons.empty()) {
                const auto& pool_photons = pool_it->second.photons;
                for (size_t i = 0; i < photons.size(); ++i) {
                    if (!current_e_g_ok[i]) continue;
                    const auto& g1 = current_cache[i];
                    for (size_t j = 0; j < pool_photons.size(); ++j) {
                        const auto& g2 = pool_photons[j];

                        const double mass_sq = mgg_squared(g1, g2);
                        if (mass_sq < MIN_INVARIANT_MASS_SQUARED) continue;
                        const double momentum = pair_momentum(g1, g2);
                        const double cos_min_open =
                            std::cos(min_photon_opening_angle(momentum) * M_PI / 180.0);
                        if (!angle_exceeds(g1, g2, cos_min_open)) continue;

                        auto pi0_4v = g1.PxPyPzEVector() + g2.PxPyPzEVector();
                        PairKinematics k = compute_pair_kinematics(pi0_4v, q_hat, x_hat, y_hat, nu);

                        h_mgg_mixed_all.fill(k.mgg);
                        int bin_4d = find_mix_4d_bin(Q2, xB, k.z, k.pT);
                        if (bin_4d >= 0) {
                            auto it = h_mgg_mixed.find(bin_4d);
                            if (it != h_mgg_mixed.end()) it->second->fill(k.mgg);
                        }
                    }
                }
                m_cutflow.pass("Mixed-event pairs");
            }
        }

        // Pool update: append current event's PhotonCache snapshots,
        // evict oldest FIFO if full. Storing the cache (not the heavier
        // Particle) makes the mixed-pair iteration purely arithmetic.
        // The shard lock is held only for the duration of the update on
        // this bin's shard; other shards stay independently writable.
        if (pool_bin >= 0 && !photons.empty()) {
            auto& shard = pool_shard_for(pool_bin);
            std::unique_lock lock(shard.mutex);
            auto& pool = shard.pools[pool_bin];
            pool.photons.insert(pool.photons.end(), current_cache.begin(), current_cache.end());
            pool.event_sizes.push_back(photons.size());
            while (static_cast<int>(pool.event_sizes.size()) > m_config.max_pool_events) {
                size_t n = pool.event_sizes.front();
                pool.photons.erase(pool.photons.begin(), pool.photons.begin() + static_cast<ptrdiff_t>(n));
                pool.event_sizes.pop_front();
            }
        }
    }

    struct PairKinematics {
        double mgg;
        double z;
        double pT;
        double pT2;
        double phi_h;
    };

    auto compute_pair_kinematics(const ROOT::Math::PxPyPzEVector& pi0_4v,
                                 const ROOT::Math::XYZVector& q_hat,
                                 const ROOT::Math::XYZVector& x_hat,
                                 const ROOT::Math::XYZVector& y_hat,
                                 double nu) const -> PairKinematics {
        PairKinematics k{};
        k.mgg = pi0_4v.M();
        k.z   = pi0_4v.E() / nu;
        ROOT::Math::XYZVector pi0_3v(pi0_4v.Px(), pi0_4v.Py(), pi0_4v.Pz());
        double p_parallel = pi0_3v.Dot(q_hat);
        ROOT::Math::XYZVector p_perp = pi0_3v - p_parallel * q_hat;
        k.pT2 = p_perp.Mag2();
        k.pT  = std::sqrt(k.pT2);
        k.phi_h = std::atan2(p_perp.Dot(y_hat), p_perp.Dot(x_hat)) * 180.0 / M_PI;
        return k;
    }

    auto make_row(int pair_type, long event_id, int n_photons, int helicity,
                  double Q2, double xB, double nu, double W, double y,
                  const ROOT::Math::PxPyPzEVector& pi0_4v,
                  const PairKinematics& k,
                  int iQ2, int ixB) const -> Pi0PairNTupleData {
        Pi0PairNTupleData row{};
        row.pair_type = pair_type;
        row.event_id  = event_id;
        row.n_photons = n_photons;
        row.helicity  = helicity;
        row.Q2 = Q2; row.xB = xB; row.nu = nu; row.W = W; row.y = y;
        row.mgg = k.mgg; row.z = k.z; row.pT = k.pT; row.pT2 = k.pT2; row.phi_h = k.phi_h;
        row.pi0_px = pi0_4v.Px(); row.pi0_py = pi0_4v.Py(); row.pi0_pz = pi0_4v.Pz(); row.pi0_E = pi0_4v.E();
        row.iQ2 = iQ2;
        row.ixB = ixB;
        row.iz_mult   = find_1d_bin(k.z,   m_config.z_edges);
        row.ipT2_mult = find_1d_bin(k.pT2, m_config.pT2_edges);
        row.iz_mix    = find_1d_bin(k.z,   m_config.z_mix_edges);
        row.ipT_mix   = find_1d_bin(k.pT,  m_config.pT_mix_edges);
        return row;
    }

    auto flush_rows_to_ntuple(const std::vector<Pi0PairNTupleData>& rows) -> void {
        if (rows.empty() || !m_ntuple_writer) return;
        // No explicit lock: NTupleService now hands each worker thread
        // its own RNTupleFillContext via NTupleWriter<T>::slot(), so the
        // fills below are independent across threads.
        for (const auto& r : rows) {
            m_ntuple_writer.data() = r;
            m_ntuple_writer.fill();
        }
    }

    // -------- Histogram booking ----------------------------------------------
    auto book_histograms() -> void {
        auto* svc = m_hist.get();

        // -------- Pi0Multiplicity directory ----------------------------------
        const std::string mult_dir = "/Pi0Multiplicity";

        svc->register_hist(mult_dir + "/n_DIS_events", "DIS Event Count;count;Events", h_n_DIS_events);
        svc->register_hist(mult_dir + "/n_DIS_vs_Q2",  "DIS Events vs Q^{2};Q^{2} [GeV^{2}];Events", h_n_DIS_vs_Q2);
        svc->register_hist(mult_dir + "/n_DIS_vs_xB",  "DIS Events vs x_{B};x_{B};Events", h_n_DIS_vs_xB);

        h_n_DIS_Q2bin.resize(m_config.Q2_edges.size() - 1);
        for (size_t i = 0; i + 1 < m_config.Q2_edges.size(); ++i) {
            h_n_DIS_Q2bin[i] = std::make_unique<framework::H1<>>(mh::regular(0, 1, 1));
            svc->register_hist(fmt::format("{}/norm/n_DIS_Q2bin{}", mult_dir, i),
                               fmt::format("DIS Events Q^{{2}} [{:.1f}, {:.1f}];count;Events",
                                           m_config.Q2_edges[i], m_config.Q2_edges[i + 1]),
                               *h_n_DIS_Q2bin[i]);
        }

        h_n_DIS_xBbin.resize(m_config.xB_edges.size() - 1);
        for (size_t i = 0; i + 1 < m_config.xB_edges.size(); ++i) {
            h_n_DIS_xBbin[i] = std::make_unique<framework::H1<>>(mh::regular(0, 1, 1));
            svc->register_hist(fmt::format("{}/norm/n_DIS_xBbin{}", mult_dir, i),
                               fmt::format("DIS Events x_{{B}} [{:.2f}, {:.2f}];count;Events",
                                           m_config.xB_edges[i], m_config.xB_edges[i + 1]),
                               *h_n_DIS_xBbin[i]);
        }

        // 2D DIS-electron count on the analysis (Q^2 x x_B) variable-edge
        // binning. Consumed by the Python multiplicity-ratio extraction as the
        // denominator N_A^{DIS}(Q^2, x_B). Stored in the /norm subdirectory
        // alongside the per-axis projections so the offline tooling can grab
        // everything DIS-norm-related from one place.
        h_n_DIS_Q2_vs_xB = std::make_unique<H2VarVar>(
            mh::VariableAxis(m_config.Q2_edges.begin(), m_config.Q2_edges.end()),
            mh::VariableAxis(m_config.xB_edges.begin(), m_config.xB_edges.end()));
        svc->register_hist(mult_dir + "/norm/n_DIS_Q2_vs_xB",
                           "DIS Events vs Q^{2} and x_{B};Q^{2} [GeV^{2}];x_{B}",
                           *h_n_DIS_Q2_vs_xB);

        svc->register_hist(mult_dir + "/z",        "#pi^{0} z = E_{#pi^{0}}/#nu;z;Counts", h_z);
        svc->register_hist(mult_dir + "/pT2",      "#pi^{0} p_{T}^{2};p_{T}^{2} [GeV^{2}];Counts", h_pT2);
        svc->register_hist(mult_dir + "/phi_h",    "#pi^{0} #phi_{h};#phi_{h} [deg];Counts", h_phi_h);
        svc->register_hist(mult_dir + "/pi0_mass", "#pi^{0} Invariant Mass;M_{#gamma#gamma} [GeV/c^{2}];Counts", h_pi0_mass);
        svc->register_hist(mult_dir + "/z_vs_Q2",  "z vs Q^{2};Q^{2} [GeV^{2}];z", h_z_vs_Q2);
        svc->register_hist(mult_dir + "/z_vs_pT2", "z vs p_{T}^{2};p_{T}^{2} [GeV^{2}];z", h_z_vs_pT2);
        svc->register_hist(mult_dir + "/pT2_vs_Q2","p_{T}^{2} vs Q^{2};Q^{2} [GeV^{2}];p_{T}^{2} [GeV^{2}]", h_pT2_vs_Q2);

        h_z_Q2bin.resize(m_config.Q2_edges.size() - 1);
        h_pT2_Q2bin.resize(m_config.Q2_edges.size() - 1);
        for (size_t i = 0; i + 1 < m_config.Q2_edges.size(); ++i) {
            h_z_Q2bin[i]   = std::make_unique<framework::H1<>>(mh::regular(0, 1, 100));
            h_pT2_Q2bin[i] = std::make_unique<framework::H1<>>(mh::regular(0, 2, 100));
            svc->register_hist(fmt::format("{}/z_Q2bin/z_Q2bin{}", mult_dir, i),
                               fmt::format("#pi^{{0}} z, Q^{{2}} #in [{:.1f}, {:.1f}] GeV^{{2}};z;Counts",
                                           m_config.Q2_edges[i], m_config.Q2_edges[i + 1]),
                               *h_z_Q2bin[i]);
            svc->register_hist(fmt::format("{}/pT2_Q2bin/pT2_Q2bin{}", mult_dir, i),
                               fmt::format("#pi^{{0}} p_{{T}}^{{2}}, Q^{{2}} #in [{:.1f}, {:.1f}] GeV^{{2}};p_{{T}}^{{2}} [GeV^{{2}}];Counts",
                                           m_config.Q2_edges[i], m_config.Q2_edges[i + 1]),
                               *h_pT2_Q2bin[i]);
        }

        h_z_xBbin.resize(m_config.xB_edges.size() - 1);
        for (size_t i = 0; i + 1 < m_config.xB_edges.size(); ++i) {
            h_z_xBbin[i] = std::make_unique<framework::H1<>>(mh::regular(0, 1, 100));
            svc->register_hist(fmt::format("{}/z_xBbin/z_xBbin{}", mult_dir, i),
                               fmt::format("#pi^{{0}} z, x_{{B}} #in [{:.2f}, {:.2f}];z;Counts",
                                           m_config.xB_edges[i], m_config.xB_edges[i + 1]),
                               *h_z_xBbin[i]);
        }

        h_pT2_zbin.resize(m_config.z_edges.size() - 1);
        for (size_t i = 0; i + 1 < m_config.z_edges.size(); ++i) {
            h_pT2_zbin[i] = std::make_unique<framework::H1<>>(mh::regular(0, 2, 100));
            svc->register_hist(fmt::format("{}/pT2_zbin/pT2_zbin{}", mult_dir, i),
                               fmt::format("#pi^{{0}} p_{{T}}^{{2}}, z #in [{:.1f}, {:.1f}];p_{{T}}^{{2}} [GeV^{{2}}];Counts",
                                           m_config.z_edges[i], m_config.z_edges[i + 1]),
                               *h_pT2_zbin[i]);
        }

        int n4D_mult = total_mult_4D_bins();
        const auto axis_mult_4d = mh::regular(0.0, static_cast<double>(n4D_mult), n4D_mult);
        h_counts_4D     = std::make_unique<framework::H1<>>(axis_mult_4d);
        h_sum_pT2_4D    = std::make_unique<framework::H1<mh::WeightedAccumulator>>(axis_mult_4d);
        h_sum_cosphi_4D = std::make_unique<framework::H1<mh::WeightedAccumulator>>(axis_mult_4d);
        h_sum_sinphi_4D = std::make_unique<framework::H1<mh::WeightedAccumulator>>(axis_mult_4d);
        svc->register_hist(mult_dir + "/4D/counts",     "4D bin counts (Q^{2}, x_{B}, z, p_{T}^{2});flat bin;Counts", *h_counts_4D);
        svc->register_hist(mult_dir + "/4D/sum_pT2",    "#Sigma p_{T}^{2} per 4D bin;flat bin;#Sigma p_{T}^{2}", *h_sum_pT2_4D);
        svc->register_hist(mult_dir + "/4D/sum_cosphi", "#Sigma cos(#phi_{h}) per 4D bin;flat bin;#Sigma cos#phi_{h}", *h_sum_cosphi_4D);
        svc->register_hist(mult_dir + "/4D/sum_sinphi", "#Sigma sin(#phi_{h}) per 4D bin;flat bin;#Sigma sin#phi_{h}", *h_sum_sinphi_4D);

        // -------- Pi0EventMixing directory -----------------------------------
        const std::string mix_dir = "/Pi0EventMixing";
        svc->register_hist(mix_dir + "/mgg_same_all",  "Same-event M_{#gamma#gamma};M_{#gamma#gamma} [GeV/c^{2}];Counts",  h_mgg_same_all);
        svc->register_hist(mix_dir + "/mgg_mixed_all", "Mixed-event M_{#gamma#gamma};M_{#gamma#gamma} [GeV/c^{2}];Counts", h_mgg_mixed_all);

        int n_mix = total_mix_4D_bins();
        for (int idx = 0; idx < n_mix; ++idx) {
            auto [iq, ix, iz, ip] = decompose_mix_4d_bin(idx);
            const std::string range = fmt::format(
                "Q^{{2}}#in[{:.1f},{:.1f}] x_{{B}}#in[{:.2f},{:.2f}] z#in[{:.2f},{:.2f}] p_{{T}}#in[{:.2f},{:.2f}]",
                m_config.Q2_edges[iq], m_config.Q2_edges[iq + 1],
                m_config.xB_edges[ix], m_config.xB_edges[ix + 1],
                m_config.z_mix_edges[iz], m_config.z_mix_edges[iz + 1],
                m_config.pT_mix_edges[ip], m_config.pT_mix_edges[ip + 1]);

            h_mgg_same[idx]  = std::make_unique<framework::H1<>>(mh::regular(0.0, 0.3, 200));
            h_mgg_mixed[idx] = std::make_unique<framework::H1<>>(mh::regular(0.0, 0.3, 200));

            svc->register_hist(fmt::format("{}/same/mgg_same_bin{}",   mix_dir, idx),
                               fmt::format("Same M_{{#gamma#gamma}} {};M_{{#gamma#gamma}} [GeV/c^{{2}}];Counts",  range), *h_mgg_same[idx]);
            svc->register_hist(fmt::format("{}/mixed/mgg_mixed_bin{}", mix_dir, idx),
                               fmt::format("Mixed M_{{#gamma#gamma}} {};M_{{#gamma#gamma}} [GeV/c^{{2}}];Counts", range), *h_mgg_mixed[idx]);
        }

        Log::info(name(), "Booked {} multiplicity 4D bins + {} mixing 4D bins (same/mixed)",
                  n4D_mult, n_mix);
    }
};

}  // namespace algorithms
