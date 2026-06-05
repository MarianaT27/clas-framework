#pragma once

#include <array>
#include <cmath>
#include <type_traits>

#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/HistogramReflect.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/ServiceReflect.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace algorithms {

using namespace framework;

/**
 * @brief N-Tuple data structure for DIS kinematics
 *
 * Plain aggregate; branches reflected via Boost.PFR (no ntuple_fields() tuple).
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
};

static_assert(std::is_aggregate_v<DISNTupleData> && std::is_standard_layout_v<DISNTupleData>, "DISNTupleData must be a standard-layout aggregate for PFR reflection");

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
    bool enable_histograms = true;
    bool enable_q2_cut = true;
    bool enable_w_cut = true;
    bool enable_y_cut = true;
};

static_assert(std::is_aggregate_v<DISSelectionConfig>, "DISSelectionConfig must stay a simple aggregate for PFR reflection");

/**
 * @brief Algorithm for DIS event selection
 */
class DISSelectionAlg : public ConfigurableAlgorithm<DISSelectionConfig> {
   public:
    explicit DISSelectionAlg(std::string name = "DISSelectionAlg", Config config = {})
        : ConfigurableAlgorithm(std::move(name), std::move(config)), m_cutflow("DIS Selection") {
        wire_services(*this);  // reflects ServiceRef members (refl-cpp)
        declare_ntuple(m_ntuple, [this] { return m_config.enable_ntuple; });
    }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        log_info("Initializing DIS selection algorithm");
        log_info("NTuple output: {}", m_config.enable_ntuple ? "enabled" : "disabled");
        log_info("Histograms: {}", m_config.enable_histograms ? "enabled" : "disabled");
        log_info("DIS region: Q2=[{}, {}], W=[{}, {}], y=[{}, {}]", m_config.q2_min, m_config.q2_max, m_config.w_min, m_config.w_max, m_config.y_min, m_config.y_max);

        if (m_config.enable_histograms) {
            register_fields(*m_hist_svc, "/DIS", h_dis);
            register_fields(*m_hist_svc, "/DIS/PreCut", h_precut);
            register_fields(*m_hist_svc, "/DIS/NMinus1", h_nminus1);
        }

        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        auto electron_opt = EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::FILTERED();
        const auto& electron = *electron_opt;
        m_cutflow.pass("Has Good Electron");

        physics::DISAnalysis::DISKinematics kin = physics::DISAnalysis::calculate_kinematics(electron);

        h_precut.Q2.fill(kin.q2);
        h_precut.W.fill(kin.w);
        h_precut.nu.fill(kin.nu);
        h_precut.xB.fill(kin.xb);
        h_precut.y.fill(kin.y);
        h_precut.e_p.fill(electron.p());
        h_precut.e_theta.fill(electron.theta());
        h_precut.e_phi.fill(electron.phi());
        h_precut.Q2_vs_W.fill(kin.w, kin.q2);
        h_precut.Q2_vs_xB.fill(kin.xb, kin.q2);
        h_precut.W_vs_y.fill(kin.y, kin.w);
        h_precut.e_theta_vs_p.fill(electron.p(), electron.theta());

        const bool pass_q2 = m_config.enable_q2_cut && (m_config.q2_min < kin.q2 && kin.q2 < m_config.q2_max);
        const bool pass_w = m_config.enable_w_cut && (m_config.w_min < kin.w && kin.w < m_config.w_max);
        const bool pass_y = m_config.enable_y_cut && (m_config.y_min < kin.y && kin.y < m_config.y_max);

        // N-1 histograms: fill if all-other cuts pass.
        if (pass_w && pass_y) h_nminus1.Q2.fill(kin.q2);
        if (pass_q2 && pass_y) h_nminus1.W.fill(kin.w);
        if (pass_q2 && pass_w) h_nminus1.y.fill(kin.y);

        // The enable flag is already folded into pass_q2/pass_w/pass_y (a
        // disabled cut yields false here, filtering the event — and zeroes the
        // matching N-1 fill above), so gate on the precomputed booleans with
        // enabled=true to reproduce the original `if (!pass_x) return FILTERED()`
        // semantics exactly while still recording each stage via the cutflow.
        if (!m_cutflow.check("Q2 Cut", true, [&] { return pass_q2; })) return StatusCode::FILTERED();
        if (!m_cutflow.check("W Cut", true, [&] { return pass_w; })) return StatusCode::FILTERED();
        if (!m_cutflow.check("y Cut", true, [&] { return pass_y; })) return StatusCode::FILTERED();

        event.set_decision_typed(Decisions::HasGoodDISElectron, true);
        event.store_typed(Keys::SelectedDISElectron, std::vector<Particle>{electron});
        event.store_typed(Keys::DIS_Q2, kin.q2);
        event.store_typed(Keys::DIS_W, kin.w);
        event.store_typed(Keys::DIS_xB, kin.xb);
        event.store_typed(Keys::DIS_y, kin.y);
        event.store_typed(Keys::DIS_nu, kin.nu);

        h_dis.Q2.fill(kin.q2);
        h_dis.W.fill(kin.w);
        h_dis.nu.fill(kin.nu);
        h_dis.xB.fill(kin.xb);
        h_dis.y.fill(kin.y);
        h_dis.Q2_vs_W.fill(kin.w, kin.q2);
        h_dis.Q2_vs_xB.fill(kin.xb, kin.q2);
        h_dis.W_vs_y.fill(kin.y, kin.w);

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

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));
        if (m_config.enable_ntuple) log_stat("N-tuple entries", m_ntuple.entries());
        m_cutflow.print();
        log_success("Finalization complete");
        return StatusCode::SUCCESS();
    }

   private:
    FRAMEWORK_REFLECT_FRIEND;  // let refl-cpp reflect the private ServiceRef members

    Cutflow m_cutflow;

    ServiceRef<HistogramService> m_hist_svc;
    ServiceRef<NTupleService> m_ntuple_svc;

    // Selected-electron DIS histograms — registered under "/DIS".
    struct DISHistograms {
        H1Auto Q2{"Q^{2};Q^{2} [GeV^{2}];Counts", 0.0, 12.0, 100};
        H1Auto W{"W;W [GeV];Counts", 0.0, 5.0, 100};
        H1Auto nu{"#nu;#nu;Events", 0, 11, 180};
        H1Auto xB{"x_{B};x_{B};Counts", 0.0, 1.0, 100};
        H1Auto y{"y;y;Events", 0, 1, 200};
        H2Auto Q2_vs_W{"Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 0, 5, 200, 0, 12, 200};
        H2Auto Q2_vs_xB{"Q^{2} vs x_{B};x_{B};Q^{2} [GeV^{2}]", 0, 1, 200, 0, 12, 200};
        H2Auto W_vs_y{"W vs y;y;W [GeV]", 0, 1, 200, 0, 5, 200};
    };

    // Pre-cut histograms — registered under "/DIS/PreCut".
    struct PreCutHistograms {
        H1Auto Q2{"Q^{2} (pre-cut);Q^{2} [GeV^{2}];Counts", 0.0, 12.0, 100};
        H1Auto W{"W (pre-cut);W [GeV];Counts", 0.0, 5.0, 100};
        H1Auto nu{"#nu (pre-cut);#nu;Events", 0, 11, 180};
        H1Auto xB{"x_{B} (pre-cut);x_{B};Counts", 0.0, 1.0, 100};
        H1Auto y{"y (pre-cut);y;Events", 0, 1, 200};
        H1Auto e_p{"Electron p (pre-cut);Momentum [GeV/c];Counts", 0.0, 11.0, 100};
        H1Auto e_theta{"Electron #theta (pre-cut);#theta [deg];Counts", 0.0, 180.0, 180};
        H1Auto e_phi{"Electron #phi (pre-cut);#phi [deg];Events", -180, 180, 360};
        H2Auto Q2_vs_W{"Q^{2} vs W (pre-cut);W [GeV];Q^{2} [GeV^{2}]", 0, 5, 200, 0, 12, 200};
        H2Auto Q2_vs_xB{"Q^{2} vs x_{B} (pre-cut);x_{B};Q^{2} [GeV^{2}]", 0, 1, 200, 0, 12, 200};
        H2Auto W_vs_y{"W vs y (pre-cut);y;W [GeV]", 0, 1, 200, 0, 5, 200};
        H2Auto e_theta_vs_p{"#theta vs p (pre-cut);p [GeV/c];#theta [deg]", 0, 11, 200, 0, 45, 200};
    };

    // N-1 histograms — registered under "/DIS/NMinus1".
    struct NMinus1Histograms {
        H1Auto Q2{"Q^{2} (N-1);Q^{2} [GeV^{2}];Counts", 0.0, 12.0, 100};
        H1Auto W{"W (N-1);W [GeV];Counts", 0.0, 5.0, 100};
        H1Auto y{"y (N-1);y;Events", 0, 1, 200};
    };

    DISHistograms h_dis;
    PreCutHistograms h_precut;
    NMinus1Histograms h_nminus1;

    NTupleWriter<DISNTupleData> m_ntuple{"dis_kinematics"};
};

}  // namespace algorithms

REFL_AUTO(type(algorithms::DISSelectionAlg), field(m_hist_svc), field(m_ntuple_svc))
