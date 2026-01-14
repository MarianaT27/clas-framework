#pragma once

#include <cmath>
#include "Framework/CutBuilder.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/Property.hpp"
#include "Framework/SimpleAlgorithm.hpp"
#include "Framework/Services/HipoFileService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "CLAS/PIDs.hpp"
#include "Physics/DISAnalysis.hpp"
#include "TH1D.h"
#include "TH2D.h"

namespace Algorithms {

using Framework::Log;

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
 * - Automatic service injection via require_service<T>()
 * - Property system for configuration
 * - CutBuilder for fluent cut definitions
 */
class DiPionFilterAlg : public Framework::SimpleAlgorithm<DiPionFilterAlg> {
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
    }

    // ========================================================================
    // Configuration setters (call before initialize)
    // ========================================================================

    /**
     * @brief Set minimum Q2 cut
     */
    auto set_q2_min(double value) -> DiPionFilterAlg& {
        (void)m_q2_min.set(value);
        return *this;
    }

    /**
     * @brief Set minimum W cut
     */
    auto set_w_min(double value) -> DiPionFilterAlg& {
        (void)m_w_min.set(value);
        return *this;
    }

    /**
     * @brief Set minimum pion momentum cut
     */
    auto set_pion_p_min(double value) -> DiPionFilterAlg& {
        (void)m_pion_p_min.set(value);
        return *this;
    }

    /**
     * @brief Set maximum |chi2pid| cut for pions
     */
    auto set_pion_chi2pid_max(double value) -> DiPionFilterAlg& {
        (void)m_pion_chi2pid_max.set(value);
        return *this;
    }

    /**
     * @brief Enable or disable histogram output
     */
    auto set_enable_histograms(bool value) -> DiPionFilterAlg& {
        (void)m_enable_histograms.set(value);
        return *this;
    }

    /**
     * @brief Configure required services
     */
    auto configure() -> void override {
        m_hipo_svc = &require_service<Framework::HipoFileService>("HipoFileService");
        m_hist_svc = &require_service<Framework::HistogramService>("HistogramService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> Framework::StatusCode override {
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
        if (!m_hipo_svc || !m_hipo_svc->is_valid()) {
            Log::warning(name(), "HipoFileService not available - output disabled");
        } else if (!m_hipo_svc->get()->is_output_enabled()) {
            Log::warning(name(), "HIPO output file not configured");
        }

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // ================================================================
        // Step 1: Find trigger electron
        // ================================================================
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);
        if (electrons.empty()) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Use the highest momentum electron as trigger
        const auto& electron = *std::max_element(electrons.begin(), electrons.end(),
            [](const CLAS::Particle& a, const CLAS::Particle& b) {
                return a.p() < b.p();
            });

        // ================================================================
        // Step 2: Apply DIS kinematic cuts (Q2, W)
        // ================================================================
        Physics::DISAnalysis::DISKinematics kin =
            Physics::DISAnalysis::calculate_kinematics(electron);

        if (kin.q2 < m_q2_min.get()) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Q2 Cut");

        if (kin.w < m_w_min.get()) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("W Cut");

        // ================================================================
        // Step 3: Find pi+ passing cuts
        // ================================================================
        const auto& pions_plus = event.particles(CLAS::PIDs::PIPLUS);
        if (pions_plus.empty()) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has pi+");

        // Find pi+ candidates passing momentum and chi2pid cuts
        std::vector<const CLAS::Particle*> good_piplus;
        for (const auto& pi : pions_plus) {
            if (pi.p() < m_pion_p_min.get()) continue;
            if (std::abs(pi.chi2pid()) >= m_pion_chi2pid_max.get()) continue;
            good_piplus.push_back(&pi);
        }

        if (good_piplus.empty()) {
            // Check which cut failed for cutflow
            bool any_pass_momentum = false;
            for (const auto& pi : pions_plus) {
                if (pi.p() >= m_pion_p_min.get()) {
                    any_pass_momentum = true;
                    break;
                }
            }
            if (!any_pass_momentum) {
                return Framework::StatusCode::FILTERED();
            }
            m_cutflow.pass("pi+ Momentum");
            return Framework::StatusCode::FILTERED();  // Failed chi2pid
        }
        m_cutflow.pass("pi+ Momentum");
        m_cutflow.pass("pi+ Chi2PID");

        // ================================================================
        // Step 4: Find pi- passing cuts
        // ================================================================
        const auto& pions_minus = event.particles(CLAS::PIDs::PIMINUS);
        if (pions_minus.empty()) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has pi-");

        // Find pi- candidates passing momentum and chi2pid cuts
        std::vector<const CLAS::Particle*> good_piminus;
        for (const auto& pi : pions_minus) {
            if (pi.p() < m_pion_p_min.get()) continue;
            if (std::abs(pi.chi2pid()) >= m_pion_chi2pid_max.get()) continue;
            good_piminus.push_back(&pi);
        }

        if (good_piminus.empty()) {
            // Check which cut failed for cutflow
            bool any_pass_momentum = false;
            for (const auto& pi : pions_minus) {
                if (pi.p() >= m_pion_p_min.get()) {
                    any_pass_momentum = true;
                    break;
                }
            }
            if (!any_pass_momentum) {
                return Framework::StatusCode::FILTERED();
            }
            m_cutflow.pass("pi- Momentum");
            return Framework::StatusCode::FILTERED();  // Failed chi2pid
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
        if (m_hipo_svc && m_hipo_svc->is_valid() && m_hipo_svc->get()->is_output_enabled()) {
            auto& raw_event = event.hipo_event().raw_event();
            m_hipo_svc->get()->write_event(raw_event);
        }

        // Store selected particles for downstream algorithms
        event.set_decision("HasDiPionEvent", true);
        event.store("dipion_Q2", kin.q2);
        event.store("dipion_W", kin.w);

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> Framework::StatusCode override {
        Log::info(name(), "Finalizing Di-Pion filter algorithm");
        Log::info(name(), "Events passing all cuts: {}", m_events_passed);

        if (m_hipo_svc && m_hipo_svc->is_valid()) {
            Log::info(name(), "Events written to HIPO: {}",
                      m_hipo_svc->get()->events_written());
        }

        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

   private:
    Framework::Cutflow m_cutflow;

    // Configurable properties
    Framework::Property<double> m_q2_min{"q2_min", 0.7, "Minimum Q2 [GeV^2]"};
    Framework::Property<double> m_w_min{"w_min", 1.7, "Minimum W [GeV]"};
    Framework::Property<double> m_pion_p_min{"pion_p_min", 0.3, "Minimum pion momentum [GeV]"};
    Framework::Property<double> m_pion_chi2pid_max{"pion_chi2pid_max", 10.0, "Maximum |chi2pid| for pions"};
    Framework::Property<bool> m_enable_histograms{"enable_histograms", true, "Enable histogram output"};

    // Service handles (auto-injected via require_service)
    Framework::ServiceHandle<Framework::HipoFileService>* m_hipo_svc = nullptr;
    Framework::ServiceHandle<Framework::HistogramService>* m_hist_svc = nullptr;

    // Statistics
    size_t m_events_passed = 0;

    // Histograms
    TH1D* h_Q2 = nullptr;
    TH1D* h_W = nullptr;
    TH1D* h_piplus_p = nullptr;
    TH1D* h_piminus_p = nullptr;
    TH1D* h_piplus_chi2pid = nullptr;
    TH1D* h_piminus_chi2pid = nullptr;
    TH2D* h_Q2_vs_W = nullptr;

    auto book_histograms() -> void {
        if (!m_hist_svc || !m_hist_svc->is_valid()) return;
        auto* svc = m_hist_svc->get();

        h_Q2 = svc->book_hist1D("/DiPion/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        h_W = svc->book_hist1D("/DiPion/W", "W;W [GeV];Events", 100, 1, 5);
        h_piplus_p = svc->book_hist1D("/DiPion/piplus_p", "#pi^{+} Momentum;p [GeV];Events", 100, 0, 6);
        h_piminus_p = svc->book_hist1D("/DiPion/piminus_p", "#pi^{-} Momentum;p [GeV];Events", 100, 0, 6);
        h_piplus_chi2pid = svc->book_hist1D("/DiPion/piplus_chi2pid", "#pi^{+} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);
        h_piminus_chi2pid = svc->book_hist1D("/DiPion/piminus_chi2pid", "#pi^{-} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);
        h_Q2_vs_W = svc->book_hist2D("/DiPion/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 100, 1, 5, 100, 0, 10);
    }

    auto fill_histograms([[maybe_unused]] const CLAS::Particle& electron,
                         const Physics::DISAnalysis::DISKinematics& kin,
                         const CLAS::Particle& piplus,
                         const CLAS::Particle& piminus) -> void {
        if (h_Q2) h_Q2->Fill(kin.q2);
        if (h_W) h_W->Fill(kin.w);
        if (h_piplus_p) h_piplus_p->Fill(piplus.p());
        if (h_piminus_p) h_piminus_p->Fill(piminus.p());
        if (h_piplus_chi2pid) h_piplus_chi2pid->Fill(piplus.chi2pid());
        if (h_piminus_chi2pid) h_piminus_chi2pid->Fill(piminus.chi2pid());
        if (h_Q2_vs_W) h_Q2_vs_W->Fill(kin.w, kin.q2);
    }
};

}  // namespace Algorithms
