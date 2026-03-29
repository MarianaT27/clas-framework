#pragma once

#include <cmath>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
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
class DiPionFilterAlg : public clas12::framework::SimpleAlgorithm<DiPionFilterAlg> {
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
        m_cutflow.pass("All Events");

        // ================================================================
        // Step 1: Find trigger electron
        // ================================================================
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        if (electrons.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Use the highest momentum electron as trigger
        const auto& electron = *std::max_element(electrons.begin(), electrons.end(),
            [](const clas12::Particle& a, const clas12::Particle& b) {
                return a.p() < b.p();
            });

        // ================================================================
        // Step 2: Apply DIS kinematic cuts (Q2, W)
        // ================================================================
        clas12::physics::DISAnalysis::DISKinematics kin =
            clas12::physics::DISAnalysis::calculate_kinematics(electron);

        if (m_enable_q2_cut.get() && kin.q2 < m_q2_min.get()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Q2 Cut");

        if (m_enable_w_cut.get() && kin.w < m_w_min.get()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("W Cut");

        // ================================================================
        // Step 3: Find pi+ passing cuts
        // ================================================================
        const auto& pions_plus = event.particles(clas12::PIDs::PIPLUS);
        if (pions_plus.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has pi+");

        // Find pi+ candidates passing momentum and chi2pid cuts
        std::vector<const clas12::Particle*> good_piplus;
        for (const auto& pi : pions_plus) {
            if (m_enable_piplus_momentum_cut.get() && pi.p() < m_pion_p_min.get()) continue;
            if (m_enable_piplus_chi2pid_cut.get() && std::abs(pi.chi2pid()) >= m_pion_chi2pid_max.get()) continue;
            good_piplus.push_back(&pi);
        }

        if (good_piplus.empty()) {
            // Check which cut failed for cutflow
            bool any_pass_momentum = !m_enable_piplus_momentum_cut.get();
            if (!any_pass_momentum) {
                for (const auto& pi : pions_plus) {
                    if (pi.p() >= m_pion_p_min.get()) {
                        any_pass_momentum = true;
                        break;
                    }
                }
            }
            if (!any_pass_momentum) {
                return clas12::framework::StatusCode::FILTERED();
            }
            m_cutflow.pass("pi+ Momentum");
            return clas12::framework::StatusCode::FILTERED();  // Failed chi2pid
        }
        m_cutflow.pass("pi+ Momentum");
        m_cutflow.pass("pi+ Chi2PID");

        // ================================================================
        // Step 4: Find pi- passing cuts
        // ================================================================
        const auto& pions_minus = event.particles(clas12::PIDs::PIMINUS);
        if (pions_minus.empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has pi-");

        // Find pi- candidates passing momentum and chi2pid cuts
        std::vector<const clas12::Particle*> good_piminus;
        for (const auto& pi : pions_minus) {
            if (m_enable_piminus_momentum_cut.get() && pi.p() < m_pion_p_min.get()) continue;
            if (m_enable_piminus_chi2pid_cut.get() && std::abs(pi.chi2pid()) >= m_pion_chi2pid_max.get()) continue;
            good_piminus.push_back(&pi);
        }

        if (good_piminus.empty()) {
            // Check which cut failed for cutflow
            bool any_pass_momentum = !m_enable_piminus_momentum_cut.get();
            if (!any_pass_momentum) {
                for (const auto& pi : pions_minus) {
                    if (pi.p() >= m_pion_p_min.get()) {
                        any_pass_momentum = true;
                        break;
                    }
                }
            }
            if (!any_pass_momentum) {
                return clas12::framework::StatusCode::FILTERED();
            }
            m_cutflow.pass("pi- Momentum");
            return clas12::framework::StatusCode::FILTERED();  // Failed chi2pid
        }
        m_cutflow.pass("pi- Momentum");
        m_cutflow.pass("pi- Chi2PID");

        // ================================================================
        // Event passed all cuts - fill histograms and write to output
        // ================================================================
        m_events_passed++;

        // Fill histograms
        if (m_enable_histograms.get()) {
            fill_histograms(electron, kin, *good_piplus.front(), *good_piminus.front());
        }

        // Write event to output HIPO file
        if (m_hipo_svc && m_hipo_svc->is_output_enabled()) {
            // TODO: event writing needs migration to new hipo4 API
            // The old API used: event.hipo_event().raw_event()
            // m_hipo_svc->get()->write_event(raw_event);
        }

        // Store selected particles for downstream algorithms
        event.set_decision_typed(clas12::framework::Decisions::HasDiPionEvent, true);
        event.store_typed(clas12::framework::Keys::DiPionQ2, kin.q2);
        event.store_typed(clas12::framework::Keys::DiPionW, kin.w);

        return clas12::framework::StatusCode::SUCCESS();
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
    clas12::framework::AtomicHistogram1D* h_Q2 = nullptr;
    clas12::framework::AtomicHistogram1D* h_W = nullptr;
    clas12::framework::AtomicHistogram1D* h_piplus_p = nullptr;
    clas12::framework::AtomicHistogram1D* h_piminus_p = nullptr;
    clas12::framework::AtomicHistogram1D* h_piplus_chi2pid = nullptr;
    clas12::framework::AtomicHistogram1D* h_piminus_chi2pid = nullptr;
    clas12::framework::AtomicHistogram2D* h_Q2_vs_W = nullptr;

    auto book_histograms() -> void {
        if (!m_hist_svc) return;
        auto* svc = m_hist_svc;

        // Use lock-free atomic histogram templates for DIS kinematics
        h_Q2 = svc->book_atomic_q2_hist("/DiPion/Q2", "Q^{2}");
        h_W = svc->book_atomic_w_hist("/DiPion/W", "W");

        // Use atomic momentum templates for pion histograms
        h_piplus_p = svc->book_atomic_momentum_hist("/DiPion/piplus_p", "#pi^{+} Momentum", 100, 0, 6);
        h_piminus_p = svc->book_atomic_momentum_hist("/DiPion/piminus_p", "#pi^{-} Momentum", 100, 0, 6);

        // Chi2PID histograms (lock-free atomic)
        h_piplus_chi2pid = svc->book_atomic_hist1D("/DiPion/piplus_chi2pid", "#pi^{+} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);
        h_piminus_chi2pid = svc->book_atomic_hist1D("/DiPion/piminus_chi2pid", "#pi^{-} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);

        // Q2 vs W (lock-free atomic 2D)
        h_Q2_vs_W = svc->book_atomic_hist2D("/DiPion/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 100, 1, 5, 100, 0, 10);
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
