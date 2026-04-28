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
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
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
        declare_properties(m_q2_min, m_q2_max, m_w_min, m_w_max, m_y_min, m_y_max, m_enable_ntuple, m_enable_q2_cut, m_enable_w_cut, m_enable_y_cut);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing DIS selection algorithm");

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Q2 Cut");
        m_cutflow.add_cut("W Cut");
        m_cutflow.add_cut("y Cut");

        book_histograms();

        if (m_enable_ntuple.get()) {
            m_ntuple.init(m_ntuple_svc.get(), "dis_kinematics");
            Log::info(name(), "N-tuple branches registered for DIS kinematics");
        }

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());
        Log::info(name(), "DIS region: Q2=[{}, {}], W=[{}, {}], y=[{}, {}]", m_q2_min.get(), m_q2_max.get(), m_w_min.get(), m_w_max.get(), m_y_min.get(), m_y_max.get());

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        auto electron_opt = framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return StatusCode::FILTERED();
        const auto& electron = *electron_opt;
        m_cutflow.pass("Has Electron");

        physics::DISAnalysis::DISKinematics kin = physics::DISAnalysis::calculate_kinematics(electron);

        h_Q2_precut->fill(kin.q2);
        h_W_precut->fill(kin.w);
        h_nu_precut->fill(kin.nu);
        h_xB_precut->fill(kin.xb);
        h_y_precut->fill(kin.y);
        h_e_p_precut->fill(electron.p());
        h_e_theta_precut->fill(electron.theta());
        h_e_phi_precut->fill(electron.phi());
        h_Q2_vs_W_precut->fill(kin.w, kin.q2);
        h_Q2_vs_xB_precut->fill(kin.xb, kin.q2);
        h_W_vs_y_precut->fill(kin.y, kin.w);
        h_e_theta_vs_p_precut->fill(electron.p(), electron.theta());
        const bool pass_q2 = !m_enable_q2_cut.get() || (kin.q2 >= m_q2_min.get() && kin.q2 <= m_q2_max.get());
        const bool pass_w = !m_enable_w_cut.get() || (kin.w >= m_w_min.get() && kin.w <= m_w_max.get());
        const bool pass_y = !m_enable_y_cut.get() || (kin.y >= m_y_min.get() && kin.y <= m_y_max.get());

        // N-1 histograms: fill if all-other cuts pass.
        if (pass_w && pass_y) h_Q2_nminus1->fill(kin.q2);
        if (pass_q2 && pass_y) h_W_nminus1->fill(kin.w);
        if (pass_q2 && pass_w) h_y_nminus1->fill(kin.y);

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

        h_Q2->fill(kin.q2);
        h_W->fill(kin.w);
        h_nu->fill(kin.nu);
        h_xB->fill(kin.xb);
        h_y->fill(kin.y);
        h_Q2_vs_W->fill(kin.w, kin.q2);
        h_Q2_vs_xB->fill(kin.xb, kin.q2);
        h_W_vs_y->fill(kin.y, kin.w);
        if (m_ntuple && m_enable_ntuple.get()) {
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

    framework::Property<double> m_q2_min{"q2_min", 1.0, "Minimum Q2 [GeV^2]"};
    framework::Property<double> m_q2_max{"q2_max", 100.0, "Maximum Q2 [GeV^2]"};
    framework::Property<double> m_w_min{"w_min", 2.0, "Minimum W [GeV]"};
    framework::Property<double> m_w_max{"w_max", 100.0, "Maximum W [GeV]"};
    framework::Property<double> m_y_min{"y_min", 0.0, "Minimum y"};
    framework::Property<double> m_y_max{"y_max", 0.85, "Maximum y"};
    framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};
    framework::Property<bool> m_enable_q2_cut{"enable_q2_cut", true, "Enable Q2 cut"};
    framework::Property<bool> m_enable_w_cut{"enable_w_cut", true, "Enable W cut"};
    framework::Property<bool> m_enable_y_cut{"enable_y_cut", true, "Enable y cut"};

    framework::ServiceRef<framework::HistogramService> m_hist_svc;
    framework::ServiceRef<framework::NTupleService> m_ntuple_svc;

    framework::ThreadLocalHistogram1D* h_Q2 = nullptr;
    framework::ThreadLocalHistogram1D* h_W = nullptr;
    framework::ThreadLocalHistogram1D* h_nu = nullptr;
    framework::ThreadLocalHistogram1D* h_xB = nullptr;
    framework::ThreadLocalHistogram1D* h_y = nullptr;

    framework::ThreadLocalHistogram1D* h_Q2_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_W_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_nu_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_xB_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_y_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_e_p_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_e_theta_precut = nullptr;
    framework::ThreadLocalHistogram1D* h_e_phi_precut = nullptr;

    framework::ThreadLocalHistogram2D* h_Q2_vs_W_precut = nullptr;
    framework::ThreadLocalHistogram2D* h_Q2_vs_xB_precut = nullptr;
    framework::ThreadLocalHistogram2D* h_W_vs_y_precut = nullptr;
    framework::ThreadLocalHistogram2D* h_e_theta_vs_p_precut = nullptr;

    framework::ThreadLocalHistogram2D* h_Q2_vs_W = nullptr;
    framework::ThreadLocalHistogram2D* h_Q2_vs_xB = nullptr;
    framework::ThreadLocalHistogram2D* h_W_vs_y = nullptr;

    framework::ThreadLocalHistogram1D* h_Q2_nminus1 = nullptr;
    framework::ThreadLocalHistogram1D* h_W_nminus1 = nullptr;
    framework::ThreadLocalHistogram1D* h_y_nminus1 = nullptr;

    framework::NTupleWriter<DISNTupleData> m_ntuple;

    auto book_histograms() -> void {
        auto* svc = m_hist_svc.get();

        h_Q2 = svc->book_hist1D("/DIS/Q2", "Q^{2};Q^{2} [GeV^{2}];Counts", 100, 0.0, 12.0);
        h_W = svc->book_hist1D("/DIS/W", "W;W [GeV];Counts", 100, 0.0, 5.0);
        h_nu = svc->book_hist1D("/DIS/nu", "#nu;#nu;Events", 180, 0, 11);
        h_xB = svc->book_hist1D("/DIS/xB", "x_{B};x_{B};Counts", 100, 0.0, 1.0);
        h_y = svc->book_hist1D("/DIS/y", "y;y;Events", 200, 0, 1);

        h_Q2_precut = svc->book_hist1D("/DIS/PreCut/Q2", "Q^{2} (pre-cut);Q^{2} [GeV^{2}];Counts", 100, 0.0, 12.0);
        h_W_precut = svc->book_hist1D("/DIS/PreCut/W", "W (pre-cut);W [GeV];Counts", 100, 0.0, 5.0);
        h_nu_precut = svc->book_hist1D("/DIS/PreCut/nu", "#nu (pre-cut);#nu;Events", 180, 0, 11);
        h_xB_precut = svc->book_hist1D("/DIS/PreCut/xB", "x_{B} (pre-cut);x_{B};Counts", 100, 0.0, 1.0);
        h_y_precut = svc->book_hist1D("/DIS/PreCut/y", "y (pre-cut);y;Events", 200, 0, 1);
        h_e_p_precut = svc->book_hist1D("/DIS/PreCut/e_p", "Electron p (pre-cut);Momentum [GeV/c];Counts", 100, 0.0, 11.0);
        h_e_theta_precut = svc->book_hist1D("/DIS/PreCut/e_theta", "Electron #theta (pre-cut);#theta [deg];Counts", 180, 0.0, 180.0);
        h_e_phi_precut = svc->book_hist1D("/DIS/PreCut/e_phi", "Electron #phi (pre-cut);#phi [deg];Events", 360, -180, 180);

        h_Q2_vs_W_precut = svc->book_hist2D("/DIS/PreCut/Q2_vs_W", "Q^{2} vs W (pre-cut);W [GeV];Q^{2} [GeV^{2}]", 200, 0, 5, 200, 0, 12);
        h_Q2_vs_xB_precut = svc->book_hist2D("/DIS/PreCut/Q2_vs_xB", "Q^{2} vs x_{B} (pre-cut);x_{B};Q^{2} [GeV^{2}]", 200, 0, 1, 200, 0, 12);
        h_W_vs_y_precut = svc->book_hist2D("/DIS/PreCut/W_vs_y", "W vs y (pre-cut);y;W [GeV]", 200, 0, 1, 200, 0, 5);
        h_e_theta_vs_p_precut = svc->book_hist2D("/DIS/PreCut/e_theta_vs_p", "#theta vs p (pre-cut);p [GeV/c];#theta [deg]", 200, 0, 11, 200, 0, 45);

        h_Q2_vs_W = svc->book_hist2D("/DIS/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]", 200, 0, 5, 200, 0, 12);
        h_Q2_vs_xB = svc->book_hist2D("/DIS/Q2_vs_xB", "Q^{2} vs x_{B};x_{B};Q^{2} [GeV^{2}]", 200, 0, 1, 200, 0, 12);
        h_W_vs_y = svc->book_hist2D("/DIS/W_vs_y", "W vs y;y;W [GeV]", 200, 0, 1, 200, 0, 5);

        h_Q2_nminus1 = svc->book_hist1D("/DIS/NMinus1/Q2", "Q^{2} (N-1);Q^{2} [GeV^{2}];Counts", 100, 0.0, 12.0);
        h_W_nminus1 = svc->book_hist1D("/DIS/NMinus1/W", "W (N-1);W [GeV];Counts", 100, 0.0, 5.0);
        h_y_nminus1 = svc->book_hist1D("/DIS/NMinus1/y", "y (N-1);y;Events", 200, 0, 1);
    }
};

}  // namespace algorithms
