#pragma once

#include <cmath>
#include <clas12/PIDs.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for DiPionFilterAlg.
struct DiPionFilterConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.

    double q2_min = 0.7;
    double w_min = 1.7;
    double pion_p_min = 0.3;
    double pion_chi2pid_max = 10.0;
    bool enable_histograms = true;
    bool enable_q2_cut = true;
    bool enable_w_cut = true;
    bool enable_piplus_momentum_cut = true;
    bool enable_piplus_chi2pid_cut = true;
    bool enable_piminus_momentum_cut = true;
    bool enable_piminus_chi2pid_cut = true;

    static constexpr auto config_fields() {
        using T = DiPionFilterConfig;
        return std::make_tuple(
            framework::field("q2_min", &T::q2_min),
            framework::field("w_min", &T::w_min),
            framework::field("pion_p_min", &T::pion_p_min),
            framework::field("pion_chi2pid_max", &T::pion_chi2pid_max),
            framework::field("enable_histograms", &T::enable_histograms),
            framework::field("enable_q2_cut", &T::enable_q2_cut),
            framework::field("enable_w_cut", &T::enable_w_cut),
            framework::field("enable_piplus_momentum_cut", &T::enable_piplus_momentum_cut),
            framework::field("enable_piplus_chi2pid_cut", &T::enable_piplus_chi2pid_cut),
            framework::field("enable_piminus_momentum_cut", &T::enable_piminus_momentum_cut),
            framework::field("enable_piminus_chi2pid_cut", &T::enable_piminus_chi2pid_cut));
    }
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
        declare_config(m_config);
        declare_histograms(h_Q2, h_W, h_piplus_p, h_piminus_p, h_piplus_chi2pid, h_piminus_chi2pid, h_Q2_vs_W);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "DIS cuts: Q2 > {} GeV^2, W > {} GeV", m_config.q2_min, m_config.w_min);
        Log::info(name(), "Pion cuts: p > {} GeV, |chi2pid| < {}", m_config.pion_p_min, m_config.pion_chi2pid_max);

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

        if (m_config.enable_histograms) book_histograms();

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

        if (m_config.enable_q2_cut && kin.q2 < m_config.q2_min) return StatusCode::FILTERED();
        m_cutflow.pass("Q2 Cut");

        if (m_config.enable_w_cut && kin.w < m_config.w_min) return StatusCode::FILTERED();
        m_cutflow.pass("W Cut");

        const auto& pions_plus = event.particles(PIDs::PIPLUS);
        if (pions_plus.empty()) return StatusCode::FILTERED();
        m_cutflow.pass("Has pi+");

        std::vector<const Particle*> good_piplus;
        for (const auto& pi : pions_plus) {
            if (m_config.enable_piplus_momentum_cut && pi.p() < m_config.pion_p_min) continue;
            if (m_config.enable_piplus_chi2pid_cut && std::abs(pi.chi2pid()) >= m_config.pion_chi2pid_max) continue;
            good_piplus.push_back(&pi);
        }
        if (good_piplus.empty()) {
            bool any_pass_momentum = !m_config.enable_piplus_momentum_cut;
            if (!any_pass_momentum) {
                for (const auto& pi : pions_plus) {
                    if (pi.p() >= m_config.pion_p_min) {
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
            if (m_config.enable_piminus_momentum_cut && pi.p() < m_config.pion_p_min) continue;
            if (m_config.enable_piminus_chi2pid_cut && std::abs(pi.chi2pid()) >= m_config.pion_chi2pid_max) continue;
            good_piminus.push_back(&pi);
        }
        if (good_piminus.empty()) {
            bool any_pass_momentum = !m_config.enable_piminus_momentum_cut;
            if (!any_pass_momentum) {
                for (const auto& pi : pions_minus) {
                    if (pi.p() >= m_config.pion_p_min) {
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

        if (m_config.enable_histograms) fill_histograms(electron, kin, *good_piplus.front(), *good_piminus.front());

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

framework::ServiceRef<framework::HipoFileService> m_hipo_svc;
    framework::ServiceRef<framework::HistogramService> m_hist_svc;

    framework::Hist1<> h_Q2{"/DiPion/Q2", "Q^{2};Q^{2} [GeV^{2}];Counts", 0.0, 12.0, 100};
    framework::Hist1<> h_W{"/DiPion/W", "W;W [GeV];Counts", 0.0, 5.0, 100};
    framework::Hist1<> h_piplus_p{"/DiPion/piplus_p", "#pi^{+} Momentum;Momentum [GeV/c];Counts", 0.0, 6.0, 100};
    framework::Hist1<> h_piminus_p{"/DiPion/piminus_p", "#pi^{-} Momentum;Momentum [GeV/c];Counts", 0.0, 6.0, 100};
    framework::Hist1<> h_piplus_chi2pid{"/DiPion/piplus_chi2pid", "#pi^{+} #chi^{2}_{PID};#chi^{2}_{PID};Events", -15.0, 15.0, 100};
    framework::Hist1<> h_piminus_chi2pid{"/DiPion/piminus_chi2pid", "#pi^{-} #chi^{2}_{PID};#chi^{2}_{PID};Events", -15.0, 15.0, 100};
    framework::Hist2<> h_Q2_vs_W{"/DiPion/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 1.0, 5.0, 100, 0.0, 10.0, 100};
    auto fill_histograms([[maybe_unused]] const Particle& electron, const physics::DISAnalysis::DISKinematics& kin, const Particle& piplus, const Particle& piminus) -> void {
        h_Q2.fill(kin.q2);
        h_W.fill(kin.w);
        h_piplus_p.fill(piplus.p());
        h_piminus_p.fill(piminus.p());
        h_piplus_chi2pid.fill(piplus.chi2pid());
        h_piminus_chi2pid.fill(piminus.chi2pid());
        h_Q2_vs_W.fill(kin.w, kin.q2);
    }
};

}  // namespace algorithms
