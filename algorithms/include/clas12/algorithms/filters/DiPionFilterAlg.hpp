#pragma once

#include <cmath>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Pipeline.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

/**
 * @brief Algorithm for filtering events with DIS electron + pi+ pi- and writing to HIPO
 *
 * This algorithm selects events containing:
 * - An electron with Q2 > 0.7 GeV^2 and W > 1.7 GeV
 * - At least one pi+ with p > 0.3 GeV and |chi2pid| < 10
 * - At least one pi- with p > 0.3 GeV and |chi2pid| < 10
 *
 * Events passing all cuts are written to an output HIPO file.
 *
 * Uses SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via svc<T>()
 * - Property system for configuration
 */
class DiPionFilterAlg : public clas12::framework::SimpleAlgorithm {
   public:
    explicit DiPionFilterAlg(std::string name = "DiPionFilterAlg")
        : SimpleAlgorithm(std::move(name))
        , m_cutflow("Di-Pion Event Filter") {
        // Declare configurable properties
        declare_property(m_q2_min);
        declare_property(m_w_min);
        declare_property(m_pion_p_min);
        declare_property(m_pion_chi2pid_max);
        declare_property(m_enable_histograms);
        declare_property(m_enable_q2_cut);
        declare_property(m_enable_w_cut);
        declare_property(m_enable_piplus_momentum_cut);
        declare_property(m_enable_piplus_chi2pid_cut);
        declare_property(m_enable_piminus_momentum_cut);
        declare_property(m_enable_piminus_chi2pid_cut);
    }

    // ========================================================================
    // Configuration setters (call before initialize)
    // ========================================================================

    /**
     * @brief Set minimum Q2 cut
     */
    auto set_q2_min(double value) -> DiPionFilterAlg& {
        m_q2_min.set(value);
        return *this;
    }

    /**
     * @brief Set minimum W cut
     */
    auto set_w_min(double value) -> DiPionFilterAlg& {
        m_w_min.set(value);
        return *this;
    }

    /**
     * @brief Set minimum pion momentum cut
     */
    auto set_pion_p_min(double value) -> DiPionFilterAlg& {
        m_pion_p_min.set(value);
        return *this;
    }

    /**
     * @brief Set maximum |chi2pid| cut for pions
     */
    auto set_pion_chi2pid_max(double value) -> DiPionFilterAlg& {
        m_pion_chi2pid_max.set(value);
        return *this;
    }

    /**
     * @brief Enable or disable histogram output
     */
    auto set_enable_histograms(bool value) -> DiPionFilterAlg& {
        m_enable_histograms.set(value);
        return *this;
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hipo_svc = svc<clas12::framework::HipoFileService>();
        m_hist_svc = svc<clas12::framework::HistogramService>();

        Log::info(name(), "Initializing Di-Pion filter algorithm");
        Log::info(name(), "  DIS cuts: Q2 > {} GeV^2, W > {} GeV",
                  m_q2_min.get(), m_w_min.get());
        Log::info(name(), "  Pion cuts: p > {} GeV, |chi2pid| < {}",
                  m_pion_p_min.get(), m_pion_chi2pid_max.get());

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Q2 Cut");
        m_cutflow.add_cut("W Cut");
        m_cutflow.add_cut("Has pi+");
        m_cutflow.add_cut("pi+ Momentum");
        m_cutflow.add_cut("pi+ Chi2PID");
        m_cutflow.add_cut("Has pi-");
        m_cutflow.add_cut("pi- Momentum");
        m_cutflow.add_cut("pi- Chi2PID");

        // Book histograms
        if (m_enable_histograms.get()) {
            book_histograms();
        }

        // Check HIPO output service
        if (!m_hipo_svc) {
            Log::warning(name(), "HipoFileService not available - output disabled");
        } else if (!m_hipo_svc->is_output_enabled()) {
            Log::warning(name(), "HIPO output file not configured");
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        using namespace clas12::framework;

        // Pick the trigger electron and compute DIS kinematics up front.
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        clas12::Particle electron{};
        clas12::physics::DISAnalysis::DISKinematics kin{};
        if (!electrons.empty()) {
            electron = *std::max_element(electrons.begin(), electrons.end(),
                [](const clas12::Particle& a, const clas12::Particle& b) { return a.p() < b.p(); });
            kin = clas12::physics::DISAnalysis::calculate_kinematics(electron);
        }

        // Per-charge ParticlePipeline runs both cuts and collects the
        // survivors. The momentum cut runs before chi2pid so the
        // cutflow counts match the old body's "momentum then chi2pid"
        // bookkeeping.
        const auto& pions_plus  = event.particles(clas12::PIDs::PIPLUS);
        const auto& pions_minus = event.particles(clas12::PIDs::PIMINUS);

        auto good_piplus = particles(pions_plus, m_cutflow)
            .filter("pi+ Momentum", [this](const clas12::Particle& pi) {
                return !m_enable_piplus_momentum_cut.get() || pi.p() >= m_pion_p_min.get();
            })
            .filter("pi+ Chi2PID", [this](const clas12::Particle& pi) {
                return !m_enable_piplus_chi2pid_cut.get()
                    || std::abs(pi.chi2pid()) < m_pion_chi2pid_max.get();
            })
            .collect();

        auto good_piminus = particles(pions_minus, m_cutflow)
            .filter("pi- Momentum", [this](const clas12::Particle& pi) {
                return !m_enable_piminus_momentum_cut.get() || pi.p() >= m_pion_p_min.get();
            })
            .filter("pi- Chi2PID", [this](const clas12::Particle& pi) {
                return !m_enable_piminus_chi2pid_cut.get()
                    || std::abs(pi.chi2pid()) < m_pion_chi2pid_max.get();
            })
            .collect();

        // Enumerate the (pi+, pi-) Cartesian product through PairPipeline.
        // The pair terminal only runs if there's at least one survivor
        // from each per-charge pipeline above, so the Cartesian product
        // is empty whenever either side was filtered out.
        auto good_pairs = particle_pairs(good_piplus, good_piminus, m_cutflow)
            .count("di-pion pairs")
            .collect_pairs();

        return pipeline(event, m_cutflow)
            .count("All Events")
            .filter("Has Electron", !electrons.empty())
            .filter("Q2 Cut", !m_enable_q2_cut.get() || kin.q2 >= m_q2_min.get())
            .filter("W Cut",  !m_enable_w_cut.get()  || kin.w  >= m_w_min.get())
            .filter("Has pi+", !pions_plus.empty())
            .filter("Has pi-", !pions_minus.empty())
            .filter("passes pi+ cuts", !good_piplus.empty())
            .filter("passes pi- cuts", !good_piminus.empty())
            .filter("di-pion pair", !good_pairs.empty())
            .tap([&] {
                ++m_events_passed;
                if (m_enable_histograms.get()) {
                    fill_histograms(electron, kin, good_pairs.front().first, good_pairs.front().second);
                }
                // TODO: event writing needs migration to new hipo4 API.
            })
            .set_decision(Decisions::HasDiPionEvent, true)
            .publish(Keys::DiPionQ2, kin.q2)
            .publish(Keys::DiPionW,  kin.w)
            .status();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing Di-Pion filter algorithm");
        Log::info(name(), "Events passing all cuts: {}", m_events_passed);

        if (m_hipo_svc) {
            Log::info(name(), "Events written to HIPO: {}",
                      m_hipo_svc->events_written());
        }

        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::framework::Cutflow m_cutflow;

    // Configurable properties
    clas12::framework::Property<double> m_q2_min{"q2_min", 0.7, "Minimum Q2 [GeV^2]"};
    clas12::framework::Property<double> m_w_min{"w_min", 1.7, "Minimum W [GeV]"};
    clas12::framework::Property<double> m_pion_p_min{"pion_p_min", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_pion_chi2pid_max{"pion_chi2pid_max", 10.0, "Maximum |chi2pid| for pions"};
    clas12::framework::Property<bool> m_enable_histograms{"enable_histograms", true, "Enable histogram output"};
    clas12::framework::Property<bool> m_enable_q2_cut{"enable_q2_cut", true, "Enable Q2 cut"};
    clas12::framework::Property<bool> m_enable_w_cut{"enable_w_cut", true, "Enable W cut"};
    clas12::framework::Property<bool> m_enable_piplus_momentum_cut{"enable_piplus_momentum_cut", true, "Enable pi+ momentum cut"};
    clas12::framework::Property<bool> m_enable_piplus_chi2pid_cut{"enable_piplus_chi2pid_cut", true, "Enable pi+ chi2PID cut"};
    clas12::framework::Property<bool> m_enable_piminus_momentum_cut{"enable_piminus_momentum_cut", true, "Enable pi- momentum cut"};
    clas12::framework::Property<bool> m_enable_piminus_chi2pid_cut{"enable_piminus_chi2pid_cut", true, "Enable pi- chi2PID cut"};

    // Service pointers (cached in on_initialize via svc<T>())
    clas12::framework::HipoFileService* m_hipo_svc = nullptr;
    clas12::framework::HistogramService* m_hist_svc = nullptr;

    // Statistics
    size_t m_events_passed = 0;

    // Lock-free atomic histograms
    clas12::framework::ThreadLocalHistogram1D* h_Q2 = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_W = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_piplus_p = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_piminus_p = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_piplus_chi2pid = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_piminus_chi2pid = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_Q2_vs_W = nullptr;

    auto book_histograms() -> void {
        if (!m_hist_svc) return;
        auto* svc = m_hist_svc;

        // Use lock-free atomic histogram templates for DIS kinematics
        h_Q2 = svc->book_q2_hist("/DiPion/Q2", "Q^{2}");
        h_W = svc->book_w_hist("/DiPion/W", "W");

        // Use atomic momentum templates for pion histograms
        h_piplus_p = svc->book_momentum_hist("/DiPion/piplus_p", "#pi^{+} Momentum", 100, 0, 6);
        h_piminus_p = svc->book_momentum_hist("/DiPion/piminus_p", "#pi^{-} Momentum", 100, 0, 6);

        // Chi2PID histograms (lock-free atomic)
        h_piplus_chi2pid = svc->book_hist1D("/DiPion/piplus_chi2pid", "#pi^{+} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);
        h_piminus_chi2pid = svc->book_hist1D("/DiPion/piminus_chi2pid", "#pi^{-} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);

        // Q2 vs W (lock-free atomic 2D)
        h_Q2_vs_W = svc->book_hist2D("/DiPion/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 100, 1, 5, 100, 0, 10);
    }

    auto fill_histograms([[maybe_unused]] const clas12::Particle& electron,
                         const clas12::physics::DISAnalysis::DISKinematics& kin,
                         const clas12::Particle& piplus,
                         const clas12::Particle& piminus) -> void {
        if (h_Q2) h_Q2->fill(kin.q2);
        if (h_W) h_W->fill(kin.w);
        if (h_piplus_p) h_piplus_p->fill(piplus.p());
        if (h_piminus_p) h_piminus_p->fill(piminus.p());
        if (h_piplus_chi2pid) h_piplus_chi2pid->fill(piplus.chi2pid());
        if (h_piminus_chi2pid) h_piminus_chi2pid->fill(piminus.chi2pid());
        if (h_Q2_vs_W) h_Q2_vs_W->fill(kin.w, kin.q2);
    }
};

}  // namespace clas12::algorithms
