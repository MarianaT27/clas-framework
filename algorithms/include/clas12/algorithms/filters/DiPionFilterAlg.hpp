#pragma once

#include <cmath>
#include <clas12/PIDs.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for DiPionFilterAlg.
struct DiPionFilterConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.
};

/**
 * @brief Algorithm for filtering events with DIS electron + pi+ pi- and
 *        writing them to HIPO output.
 */
class DiPionFilterAlg : public framework::Algorithm {
   public:
    using Config = DiPionFilterConfig;
    using StatusCode = framework::StatusCode;

    explicit DiPionFilterAlg(std::string name = "DiPionFilterAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Di-Pion Event Filter") {
        required_services(m_hipo_svc, m_hist_svc);
        declare_properties(m_q2_min, m_w_min, m_pion_p_min, m_pion_chi2pid_max, m_enable_histograms, m_enable_q2_cut, m_enable_w_cut, m_enable_piplus_momentum_cut, m_enable_piplus_chi2pid_cut, m_enable_piminus_momentum_cut, m_enable_piminus_chi2pid_cut);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "DIS cuts: Q2 > {} GeV^2, W > {} GeV", m_q2_min.get(), m_w_min.get());
        Log::info(name(), "Pion cuts: p > {} GeV, |chi2pid| < {}", m_pion_p_min.get(), m_pion_chi2pid_max.get());

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

        if (m_enable_histograms.get()) book_histograms();

        if (!m_hipo_svc->is_output_enabled()) {
            Log::warning(name(), "HIPO output file not configured");
        }

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.particles(PIDs::ELECTRON);
        if (electrons.empty()) return StatusCode::FILTERED();
        m_cutflow.pass("Has Electron");

        const auto& electron = *std::max_element(electrons.begin(), electrons.end(), [](const Particle& a, const Particle& b) { return a.p() < b.p(); });

        physics::DISAnalysis::DISKinematics kin = physics::DISAnalysis::calculate_kinematics(electron);

        if (m_enable_q2_cut.get() && kin.q2 < m_q2_min.get()) return StatusCode::FILTERED();
        m_cutflow.pass("Q2 Cut");

        if (m_enable_w_cut.get() && kin.w < m_w_min.get()) return StatusCode::FILTERED();
        m_cutflow.pass("W Cut");

        const auto& pions_plus = event.particles(PIDs::PIPLUS);
        if (pions_plus.empty()) return StatusCode::FILTERED();
        m_cutflow.pass("Has pi+");

        std::vector<const Particle*> good_piplus;
        for (const auto& pi : pions_plus) {
            if (m_enable_piplus_momentum_cut.get() && pi.p() < m_pion_p_min.get()) continue;
            if (m_enable_piplus_chi2pid_cut.get() && std::abs(pi.chi2pid()) >= m_pion_chi2pid_max.get()) continue;
            good_piplus.push_back(&pi);
        }
        if (good_piplus.empty()) {
            bool any_pass_momentum = !m_enable_piplus_momentum_cut.get();
            if (!any_pass_momentum) {
                for (const auto& pi : pions_plus) {
                    if (pi.p() >= m_pion_p_min.get()) {
                        any_pass_momentum = true;
                        break;
                    }
                }
            }
            if (!any_pass_momentum) return StatusCode::FILTERED();
            m_cutflow.pass("pi+ Momentum");
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("pi+ Momentum");
        m_cutflow.pass("pi+ Chi2PID");

        const auto& pions_minus = event.particles(PIDs::PIMINUS);
        if (pions_minus.empty()) return StatusCode::FILTERED();
        m_cutflow.pass("Has pi-");

        std::vector<const Particle*> good_piminus;
        for (const auto& pi : pions_minus) {
            if (m_enable_piminus_momentum_cut.get() && pi.p() < m_pion_p_min.get()) continue;
            if (m_enable_piminus_chi2pid_cut.get() && std::abs(pi.chi2pid()) >= m_pion_chi2pid_max.get()) continue;
            good_piminus.push_back(&pi);
        }
        if (good_piminus.empty()) {
            bool any_pass_momentum = !m_enable_piminus_momentum_cut.get();
            if (!any_pass_momentum) {
                for (const auto& pi : pions_minus) {
                    if (pi.p() >= m_pion_p_min.get()) {
                        any_pass_momentum = true;
                        break;
                    }
                }
            }
            if (!any_pass_momentum) return StatusCode::FILTERED();
            m_cutflow.pass("pi- Momentum");
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("pi- Momentum");
        m_cutflow.pass("pi- Chi2PID");

        if (m_enable_histograms.get()) fill_histograms(electron, kin, *good_piplus.front(), *good_piminus.front());

        event.set_decision_typed(framework::Decisions::HasDiPionEvent, true);
        event.store_typed(framework::Keys::DiPionQ2, kin.q2);
        event.store_typed(framework::Keys::DiPionW, kin.w);

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Finalizing");
        Log::info(name(), "Events passing all cuts: {}", m_cutflow.get_count("pi- Chi2PID"));
        if (m_hipo_svc) Log::info(name(), "Events written to HIPO: {}", m_hipo_svc->events_written());
        m_cutflow.print();
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    framework::Cutflow m_cutflow;

    framework::Property<double> m_q2_min{"q2_min", 0.7, "Minimum Q2 [GeV^2]"};
    framework::Property<double> m_w_min{"w_min", 1.7, "Minimum W [GeV]"};
    framework::Property<double> m_pion_p_min{"pion_p_min", 0.3, "Minimum pion momentum [GeV]"};
    framework::Property<double> m_pion_chi2pid_max{"pion_chi2pid_max", 10.0, "Maximum |chi2pid| for pions"};
    framework::Property<bool> m_enable_histograms{"enable_histograms", true, "Enable histogram output"};
    framework::Property<bool> m_enable_q2_cut{"enable_q2_cut", true, "Enable Q2 cut"};
    framework::Property<bool> m_enable_w_cut{"enable_w_cut", true, "Enable W cut"};
    framework::Property<bool> m_enable_piplus_momentum_cut{"enable_piplus_momentum_cut", true, "Enable pi+ momentum cut"};
    framework::Property<bool> m_enable_piplus_chi2pid_cut{"enable_piplus_chi2pid_cut", true, "Enable pi+ chi2PID cut"};
    framework::Property<bool> m_enable_piminus_momentum_cut{"enable_piminus_momentum_cut", true, "Enable pi- momentum cut"};
    framework::Property<bool> m_enable_piminus_chi2pid_cut{"enable_piminus_chi2pid_cut", true, "Enable pi- chi2PID cut"};

    framework::ServiceRef<framework::HipoFileService> m_hipo_svc;
    framework::ServiceRef<framework::HistogramService> m_hist_svc;

    framework::ThreadLocalHistogram1D* h_Q2 = nullptr;
    framework::ThreadLocalHistogram1D* h_W = nullptr;
    framework::ThreadLocalHistogram1D* h_piplus_p = nullptr;
    framework::ThreadLocalHistogram1D* h_piminus_p = nullptr;
    framework::ThreadLocalHistogram1D* h_piplus_chi2pid = nullptr;
    framework::ThreadLocalHistogram1D* h_piminus_chi2pid = nullptr;
    framework::ThreadLocalHistogram2D* h_Q2_vs_W = nullptr;

    auto book_histograms() -> void {
        h_Q2 = m_hist_svc->book_hist1D("/DiPion/Q2", "Q^{2};Q^{2} [GeV^{2}];Counts", 100, 0.0, 12.0);
        h_W = m_hist_svc->book_hist1D("/DiPion/W", "W;W [GeV];Counts", 100, 0.0, 5.0);
        h_piplus_p = m_hist_svc->book_hist1D("/DiPion/piplus_p", "#pi^{+} Momentum;Momentum [GeV/c];Counts", 100, 0, 6);
        h_piminus_p = m_hist_svc->book_hist1D("/DiPion/piminus_p", "#pi^{-} Momentum;Momentum [GeV/c];Counts", 100, 0, 6);
        h_piplus_chi2pid = m_hist_svc->book_hist1D("/DiPion/piplus_chi2pid", "#pi^{+} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);
        h_piminus_chi2pid = m_hist_svc->book_hist1D("/DiPion/piminus_chi2pid", "#pi^{-} #chi^{2}_{PID};#chi^{2}_{PID};Events", 100, -15, 15);
        h_Q2_vs_W = m_hist_svc->book_hist2D("/DiPion/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 100, 1, 5, 100, 0, 10);
    }

    auto fill_histograms([[maybe_unused]] const Particle& electron, const physics::DISAnalysis::DISKinematics& kin, const Particle& piplus, const Particle& piminus) -> void {
        h_Q2->fill(kin.q2);
        h_W->fill(kin.w);
        h_piplus_p->fill(piplus.p());
        h_piminus_p->fill(piminus.p());
        h_piplus_chi2pid->fill(piplus.chi2pid());
        h_piminus_chi2pid->fill(piminus.chi2pid());
        h_Q2_vs_W->fill(kin.w, kin.q2);
    }
};

}  // namespace algorithms
