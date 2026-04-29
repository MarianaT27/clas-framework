#pragma once

#include <array>
#include <cmath>

#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace algorithms {

using framework::Log;

/**
 * @brief N-Tuple data structure for DIS kinematics
 */
struct DISNTupleData {
    double e_p = 0.0;
    double e_theta = 0.0;
    double e_phi = 0.0;
    double e_vz = 0.0;
    double e_chi2pid = 0.0;
    double Q2 = 0.0;
    double W = 0.0;
    double nu = 0.0;
    double xB = 0.0;
    double y = 0.0;
    double epsilon = 0.0;
    double gamma_flux = 0.0;

    static constexpr auto ntuple_fields() {
        using T = DISNTupleData;
        return std::make_tuple(framework::field("e_p", &T::e_p), framework::field("e_theta", &T::e_theta), framework::field("e_phi", &T::e_phi), framework::field("e_vz", &T::e_vz), framework::field("e_chi2pid", &T::e_chi2pid), framework::field("Q2", &T::Q2), framework::field("W", &T::W), framework::field("nu", &T::nu), framework::field("xB", &T::xB), framework::field("y", &T::y), framework::field("epsilon", &T::epsilon), framework::field("gamma_flux", &T::gamma_flux));
    }
};

/// Configuration for DISSelectionAlg.
struct DISSelectionConfig {
    physics::DISAnalysis::KinematicRegion dis_region = physics::DISAnalysis::full_phase_space();

    double q2_min = 1.0;
    double q2_max = 100.0;
    double w_min = 2.0;
    double w_max = 100.0;
    double y_min = 0.0;
    double y_max = 0.85;
    bool enable_ntuple = true;
    bool enable_q2_cut = true;
    bool enable_w_cut = true;
    bool enable_y_cut = true;

    static constexpr auto config_fields() {
        using T = DISSelectionConfig;
        return std::make_tuple(
            framework::field("q2_min", &T::q2_min),
            framework::field("q2_max", &T::q2_max),
            framework::field("w_min", &T::w_min),
            framework::field("w_max", &T::w_max),
            framework::field("y_min", &T::y_min),
            framework::field("y_max", &T::y_max),
            framework::field("enable_ntuple", &T::enable_ntuple),
            framework::field("enable_q2_cut", &T::enable_q2_cut),
            framework::field("enable_w_cut", &T::enable_w_cut),
            framework::field("enable_y_cut", &T::enable_y_cut));
    }
};

/**
 * @brief Algorithm for DIS event selection
 */
class DISSelectionAlg : public framework::Algorithm {
   public:
    using Config = DISSelectionConfig;
    using StatusCode = framework::StatusCode;

    explicit DISSelectionAlg(std::string name = "DISSelectionAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("DIS Selection") {
        required_services(m_hist_svc, m_ntuple_svc);
        declare_config(m_config);
        declare_histograms(m_h_Q2, m_h_W, m_h_nu, m_h_xB, m_h_y, m_h_Q2_precut, m_h_W_precut, m_h_nu_precut, m_h_xB_precut, m_h_y_precut,
             m_h_e_p_precut, m_h_e_theta_precut, m_h_e_phi_precut, m_h_Q2_vs_W_precut, m_h_Q2_vs_xB_precut, m_h_W_vs_y_precut,
              m_h_e_theta_vs_p_precut, m_h_Q2_vs_W, m_h_Q2_vs_xB, m_h_W_vs_y, m_h_Q2_nminus1, m_h_W_nminus1, m_h_y_nminus1);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing DIS selection algorithm");

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Good Electron");
        m_cutflow.add_cut("Q2 Cut");
        m_cutflow.add_cut("W Cut");
        m_cutflow.add_cut("y Cut");

        if (m_config.enable_ntuple) {
            m_ntuple.init(m_ntuple_svc.get(), "dis_kinematics");
            Log::info(name(), "N-tuple branches registered for DIS kinematics");
        }

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());
        Log::info(name(), "DIS region: Q2=[{}, {}], W=[{}, {}], y=[{}, {}]", m_config.q2_min, m_config.q2_max, m_config.w_min, m_config.w_max, m_config.y_min, m_config.y_max);

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        auto electron_opt = framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::FILTERED();
        const auto& electron = *electron_opt;
        m_cutflow.pass("Has Good Electron");

        physics::DISAnalysis::DISKinematics kin = physics::DISAnalysis::calculate_kinematics(electron);

        m_h_Q2_precut.fill(kin.q2);
        m_h_W_precut.fill(kin.w);
        m_h_nu_precut.fill(kin.nu);
        m_h_xB_precut.fill(kin.xb);
        m_h_y_precut.fill(kin.y);
        m_h_e_p_precut.fill(electron.p());
        m_h_e_theta_precut.fill(electron.theta());
        m_h_e_phi_precut.fill(electron.phi());
        m_h_Q2_vs_W_precut.fill(kin.w, kin.q2);
        m_h_Q2_vs_xB_precut.fill(kin.xb, kin.q2);
        m_h_W_vs_y_precut.fill(kin.y, kin.w);
        m_h_e_theta_vs_p_precut.fill(electron.p(), electron.theta());

        const bool pass_q2 = m_config.enable_q2_cut && (m_config.q2_min < kin.q2 && kin.q2 < m_config.q2_max);
        const bool pass_w = m_config.enable_w_cut && (m_config.w_min < kin.w && kin.w < m_config.w_max);
        const bool pass_y = m_config.enable_y_cut && (m_config.y_min < kin.y && kin.y < m_config.y_max);

        // N-1 histograms: fill if all-other cuts pass.
        if (pass_w && pass_y) m_h_Q2_nminus1.fill(kin.q2);
        if (pass_q2 && pass_y) m_h_W_nminus1.fill(kin.w);
        if (pass_q2 && pass_w) m_h_y_nminus1.fill(kin.y);

        if (!pass_q2) return StatusCode::FILTERED();
        m_cutflow.pass("Q2 Cut");

        if (!pass_w) return StatusCode::FILTERED();
        m_cutflow.pass("W Cut");

        if (!pass_y) return StatusCode::FILTERED();
        m_cutflow.pass("y Cut");

        event.set_decision_typed(framework::Decisions::HasGoodDISElectron, true);
        event.store_typed(framework::Keys::SelectedDISElectron, std::vector<Particle>{electron});
        event.store_typed(framework::Keys::DIS_Q2, kin.q2);
        event.store_typed(framework::Keys::DIS_W, kin.w);
        event.store_typed(framework::Keys::DIS_xB, kin.xb);
        event.store_typed(framework::Keys::DIS_y, kin.y);
        event.store_typed(framework::Keys::DIS_nu, kin.nu);

        m_h_Q2.fill(kin.q2);
        m_h_W.fill(kin.w);
        m_h_nu.fill(kin.nu);
        m_h_xB.fill(kin.xb);
        m_h_y.fill(kin.y);
        m_h_Q2_vs_W.fill(kin.w, kin.q2);
        m_h_Q2_vs_xB.fill(kin.xb, kin.q2);
        m_h_W_vs_y.fill(kin.y, kin.w);

        if (m_ntuple && m_config.enable_ntuple) {
            m_ntuple.data().e_p = electron.p();
            m_ntuple.data().e_theta = electron.theta();
            m_ntuple.data().e_phi = electron.phi();
            m_ntuple.data().e_vz = electron.vz();
            m_ntuple.data().e_chi2pid = electron.chi2pid();
            m_ntuple.data().Q2 = kin.q2;
            m_ntuple.data().W = kin.w;
            m_ntuple.data().nu = kin.nu;
            m_ntuple.data().xB = kin.xb;
            m_ntuple.data().y = kin.y;
            m_ntuple.data().epsilon = kin.epsilon;
            m_ntuple.data().gamma_flux = kin.gamma;
            m_ntuple.fill();
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Finalizing DIS selection algorithm");

        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("dis_kinematics");
            Log::info(name(), "N-tuple has {} entries", entries);
        }

        m_cutflow.print();
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    framework::Cutflow m_cutflow;

framework::ServiceRef<framework::HistogramService> m_hist_svc;
    framework::ServiceRef<framework::NTupleService> m_ntuple_svc;

    framework::Hist1<> m_h_Q2{"/DIS/Q2", "Q^{2};Q^{2} [GeV^{2}];Counts", 0.0, 12.0, 100};
    framework::Hist1<> m_h_W{"/DIS/W", "W;W [GeV];Counts", 0.0, 5.0, 100};
    framework::Hist1<> m_h_nu{"/DIS/nu", "#nu;#nu;Events", 0, 11, 180};
    framework::Hist1<> m_h_xB{"/DIS/xB", "x_{B};x_{B};Counts", 0.0, 1.0, 100};
    framework::Hist1<> m_h_y{"/DIS/y", "y;y;Events", 0, 1, 200};
    framework::Hist1<> m_h_Q2_precut{"/DIS/PreCut/Q2", "Q^{2} (pre-cut);Q^{2} [GeV^{2}];Counts", 0.0, 12.0, 100};
    framework::Hist1<> m_h_W_precut{"/DIS/PreCut/W", "W (pre-cut);W [GeV];Counts", 0.0, 5.0, 100};
    framework::Hist1<> m_h_nu_precut{"/DIS/PreCut/nu", "#nu (pre-cut);#nu;Events", 0, 11, 180};
    framework::Hist1<> m_h_xB_precut{"/DIS/PreCut/xB", "x_{B} (pre-cut);x_{B};Counts", 0.0, 1.0, 100};
    framework::Hist1<> m_h_y_precut{"/DIS/PreCut/y", "y (pre-cut);y;Events", 0, 1, 200};
    framework::Hist1<> m_h_e_p_precut{"/DIS/PreCut/e_p", "Electron p (pre-cut);Momentum [GeV/c];Counts", 0.0, 11.0, 100};
    framework::Hist1<> m_h_e_theta_precut{"/DIS/PreCut/e_theta", "Electron #theta (pre-cut);#theta [deg];Counts", 0.0, 180.0, 180};
    framework::Hist1<> m_h_e_phi_precut{"/DIS/PreCut/e_phi", "Electron #phi (pre-cut);#phi [deg];Events", -180, 180, 360};
    framework::Hist2<> m_h_Q2_vs_W_precut{"/DIS/PreCut/Q2_vs_W", "Q^{2} vs W (pre-cut);W [GeV];Q^{2} [GeV^{2}]", 0, 5, 200, 0, 12, 200};
    framework::Hist2<> m_h_Q2_vs_xB_precut{"/DIS/PreCut/Q2_vs_xB", "Q^{2} vs x_{B} (pre-cut);x_{B};Q^{2} [GeV^{2}]", 0, 1, 200, 0, 12, 200};
    framework::Hist2<> m_h_W_vs_y_precut{"/DIS/PreCut/W_vs_y", "W vs y (pre-cut);y;W [GeV]", 0, 1, 200, 0, 5, 200};
    framework::Hist2<> m_h_e_theta_vs_p_precut{"/DIS/PreCut/e_theta_vs_p", "#theta vs p (pre-cut);p [GeV/c];#theta [deg]", 0, 11, 200, 0, 45, 200};
    framework::Hist2<> m_h_Q2_vs_W{"/DIS/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 0, 5, 200, 0, 12, 200};
    framework::Hist2<> m_h_Q2_vs_xB{"/DIS/Q2_vs_xB", "Q^{2} vs x_{B};x_{B};Q^{2} [GeV^{2}]", 0, 1, 200, 0, 12, 200};
    framework::Hist2<> m_h_W_vs_y{"/DIS/W_vs_y", "W vs y;y;W [GeV]", 0, 1, 200, 0, 5, 200};
    framework::Hist1<> m_h_Q2_nminus1{"/DIS/NMinus1/Q2", "Q^{2} (N-1);Q^{2} [GeV^{2}];Counts", 0.0, 12.0, 100};
    framework::Hist1<> m_h_W_nminus1{"/DIS/NMinus1/W", "W (N-1);W [GeV];Counts", 0.0, 5.0, 100};
    framework::Hist1<> m_h_y_nminus1{"/DIS/NMinus1/y", "y (N-1);y;Events", 0, 1, 200};
    
    framework::NTupleWriter<DISNTupleData> m_ntuple;

};

}  // namespace algorithms
