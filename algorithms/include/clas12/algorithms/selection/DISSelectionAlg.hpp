#pragma once

#include <cmath>

#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

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

    NTUPLE_FIELDS(e_p, e_theta, e_phi, e_vz, e_chi2pid, Q2, W, nu, xB, y, epsilon, gamma_flux)
};

/**
 * @brief Algorithm for DIS event selection
 *
 * Modernized using Algorithm CRTP pattern with:
 * - Automatic service injection via svc<T>()
 * - Property system for configuration
 * - CutBuilder for fluent cut definitions
 * - NTupleWriter for simplified N-tuple management
 */
class DISSelectionAlg : public clas12::framework::Algorithm {
   public:
    explicit DISSelectionAlg(std::string name = "DISSelectionAlg",
                              clas12::physics::DISAnalysis::KinematicRegion dis_region = clas12::physics::DISAnalysis::full_phase_space())
        : Algorithm(std::move(name))
        , m_cutflow("DIS Selection")
        , m_dis_region(std::move(dis_region)) {
        // Declare properties
        declare_property(m_q2_min);
        declare_property(m_q2_max);
        declare_property(m_w_min);
        declare_property(m_w_max);
        declare_property(m_y_min);
        declare_property(m_y_max);
        declare_property(m_enable_ntuple);
        declare_property(m_enable_q2_cut);
        declare_property(m_enable_w_cut);
        declare_property(m_enable_y_cut);
    }

    auto set_dis_region(const clas12::physics::DISAnalysis::KinematicRegion& region) -> void {
        m_dis_region = region;
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hist_svc = svc<clas12::framework::HistogramService>();
        m_ntuple_svc = svc<clas12::framework::NTupleService>();

        Log::info(name(), "Initializing DIS selection algorithm");

        m_cutflow.define_always("All Events");
        m_cutflow.define("Has Electron", [](const auto&) { return true; });  // pre-checked
        m_cutflow.define_toggleable("Q2 Cut",
            [this](const auto& k) { return k.q2 >= m_q2_min.get() && k.q2 <= m_q2_max.get(); },
            m_enable_q2_cut,
            [](const auto& k) { return k.q2; });
        m_cutflow.define_toggleable("W Cut",
            [this](const auto& k) { return k.w >= m_w_min.get() && k.w <= m_w_max.get(); },
            m_enable_w_cut,
            [](const auto& k) { return k.w; });
        m_cutflow.define_toggleable("y Cut",
            [this](const auto& k) { return k.y >= m_y_min.get() && k.y <= m_y_max.get(); },
            m_enable_y_cut,
            [](const auto& k) { return k.y; });

        book_histograms();

        // Setup N-tuple using NTupleWriter
        if (m_enable_ntuple.get() && m_ntuple_svc) {
            m_ntuple.init(m_ntuple_svc, "dis_kinematics");
            Log::info(name(), "N-tuple branches registered for DIS kinematics");
        }

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());
        Log::info(name(), "DIS region: Q2=[{}, {}], W=[{}, {}], y=[{}, {}]",
                  m_q2_min.get(), m_q2_max.get(),
                  m_w_min.get(), m_w_max.get(),
                  m_y_min.get(), m_y_max.get());

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Check for good electron decision from upstream algorithm
        auto electron_opt = clas12::framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) {
            return clas12::framework::StatusCode::FILTERED();
        }
        const auto& electron = *electron_opt;

        clas12::physics::DISAnalysis::DISKinematics kin =
            clas12::physics::DISAnalysis::calculate_kinematics(electron);

        // Fill pre-cut histograms (after electron selection, before DIS cuts)
        if (h_Q2_precut) h_Q2_precut->fill(kin.q2);
        if (h_W_precut) h_W_precut->fill(kin.w);
        if (h_nu_precut) h_nu_precut->fill(kin.nu);
        if (h_xB_precut) h_xB_precut->fill(kin.xb);
        if (h_y_precut) h_y_precut->fill(kin.y);
        if (h_e_p_precut) h_e_p_precut->fill(electron.p());
        if (h_e_theta_precut) h_e_theta_precut->fill(electron.theta());
        if (h_e_phi_precut) h_e_phi_precut->fill(electron.phi());
        if (h_Q2_vs_W_precut) h_Q2_vs_W_precut->fill(kin.w, kin.q2);
        if (h_Q2_vs_xB_precut) h_Q2_vs_xB_precut->fill(kin.xb, kin.q2);
        if (h_W_vs_y_precut) h_W_vs_y_precut->fill(kin.y, kin.w);
        if (h_e_theta_vs_p_precut) h_e_theta_vs_p_precut->fill(electron.p(), electron.theta());

        // Evaluate all cuts (no short-circuit) for N-1 histograms
        auto results = m_cutflow.evaluate_all(kin);

        // Fill N-1 histograms using framework support
        for (std::size_t i = 0; i < m_cutflow.num_cuts(); ++i) {
            if (results.passes_all_except(i)) {
                double val = m_cutflow.variable(i, kin);
                if (!std::isnan(val)) {
                    if (i < m_nminus1_hists.size() && m_nminus1_hists[i]) {
                        m_nminus1_hists[i]->fill(val);
                    }
                }
            }
        }

        if (!results.all_passed) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Electron passed all cuts - set decision and store for downstream algorithms
        event.set_decision_typed(clas12::framework::Decisions::HasGoodDISElectron, true);
        event.store_typed(clas12::framework::Keys::SelectedDISElectron, std::vector<clas12::Particle>{electron});
        event.store_typed(clas12::framework::Keys::DIS_Q2, kin.q2);
        event.store_typed(clas12::framework::Keys::DIS_W, kin.w);
        event.store_typed(clas12::framework::Keys::DIS_xB, kin.xb);
        event.store_typed(clas12::framework::Keys::DIS_y, kin.y);
        event.store_typed(clas12::framework::Keys::DIS_nu, kin.nu);

        // Fill post-cut 1D histograms
        if (h_Q2) h_Q2->fill(kin.q2);
        if (h_W) h_W->fill(kin.w);
        if (h_nu) h_nu->fill(kin.nu);
        if (h_xB) h_xB->fill(kin.xb);
        if (h_y) h_y->fill(kin.y);

        // Fill post-cut 2D correlation histograms
        if (h_Q2_vs_W) h_Q2_vs_W->fill(kin.w, kin.q2);
        if (h_Q2_vs_xB) h_Q2_vs_xB->fill(kin.xb, kin.q2);
        if (h_W_vs_y) h_W_vs_y->fill(kin.y, kin.w);

        // Fill N-tuple using NTupleWriter
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

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing DIS selection algorithm");

        if (m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("dis_kinematics");
            Log::info(name(), "N-tuple has {} entries", entries);
        }

        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::framework::ContextCutflow<clas12::physics::DISAnalysis::DISKinematics> m_cutflow;
    clas12::physics::DISAnalysis::KinematicRegion m_dis_region;

    // Properties for DIS cuts
    clas12::framework::Property<double> m_q2_min{"q2_min", 1.0, "Minimum Q2 [GeV^2]"};
    clas12::framework::Property<double> m_q2_max{"q2_max", 100.0, "Maximum Q2 [GeV^2]"};
    clas12::framework::Property<double> m_w_min{"w_min", 2.0, "Minimum W [GeV]"};
    clas12::framework::Property<double> m_w_max{"w_max", 100.0, "Maximum W [GeV]"};
    clas12::framework::Property<double> m_y_min{"y_min", 0.0, "Minimum y"};
    clas12::framework::Property<double> m_y_max{"y_max", 0.85, "Maximum y"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};
    clas12::framework::Property<bool> m_enable_q2_cut{"enable_q2_cut", true, "Enable Q2 cut"};
    clas12::framework::Property<bool> m_enable_w_cut{"enable_w_cut", true, "Enable W cut"};
    clas12::framework::Property<bool> m_enable_y_cut{"enable_y_cut", true, "Enable y cut"};

    // Service pointers (cached in on_initialize via svc<T>())
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    clas12::framework::NTupleService* m_ntuple_svc = nullptr;

    // Lock-free atomic histograms (managed by HistogramService)
    // Post-cut 1D
    clas12::framework::ThreadLocalHistogram1D* h_Q2 = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_W = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_nu = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_xB = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_y = nullptr;

    // Pre-cut 1D
    clas12::framework::ThreadLocalHistogram1D* h_Q2_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_W_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_nu_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_xB_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_y_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_e_p_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_e_theta_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_e_phi_precut = nullptr;

    // Pre-cut 2D correlations
    clas12::framework::ThreadLocalHistogram2D* h_Q2_vs_W_precut = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_Q2_vs_xB_precut = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_W_vs_y_precut = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_e_theta_vs_p_precut = nullptr;

    // Post-cut 2D correlations
    clas12::framework::ThreadLocalHistogram2D* h_Q2_vs_W = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_Q2_vs_xB = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_W_vs_y = nullptr;

    // N-1 histograms (indexed by cut, populated in book_histograms)
    std::vector<clas12::framework::ThreadLocalHistogram1D*> m_nminus1_hists;

    // N-tuple writer (replaces individual variables + manual registration)
    clas12::framework::NTupleWriter<DISNTupleData> m_ntuple;

    auto book_histograms() -> void {
        if (m_hist_svc) { auto* svc = m_hist_svc;
            // Post-cut 1D histograms
            h_Q2 = svc->book_q2_hist("/DIS/Q2", "Q^{2}");
            h_W = svc->book_w_hist("/DIS/W", "W");
            h_nu = svc->book_hist1D("/DIS/nu", "#nu;#nu;Events", 180, 0, 11);
            h_xB = svc->book_xb_hist("/DIS/xB", "x_{B}");
            h_y = svc->book_hist1D("/DIS/y", "y;y;Events", 200, 0, 1);

            // Pre-cut 1D histograms
            h_Q2_precut = svc->book_q2_hist("/DIS/PreCut/Q2", "Q^{2} (pre-cut)");
            h_W_precut = svc->book_w_hist("/DIS/PreCut/W", "W (pre-cut)");
            h_nu_precut = svc->book_hist1D("/DIS/PreCut/nu", "#nu (pre-cut);#nu;Events", 180, 0, 11);
            h_xB_precut = svc->book_xb_hist("/DIS/PreCut/xB", "x_{B} (pre-cut)");
            h_y_precut = svc->book_hist1D("/DIS/PreCut/y", "y (pre-cut);y;Events", 200, 0, 1);
            h_e_p_precut = svc->book_momentum_hist("/DIS/PreCut/e_p", "Electron p (pre-cut)");
            h_e_theta_precut = svc->book_theta_hist("/DIS/PreCut/e_theta", "Electron #theta (pre-cut)");
            h_e_phi_precut = svc->book_hist1D("/DIS/PreCut/e_phi", "Electron #phi (pre-cut);#phi [deg];Events", 360, -180, 180);

            // Pre-cut 2D correlation histograms
            h_Q2_vs_W_precut = svc->book_hist2D("/DIS/PreCut/Q2_vs_W", "Q^{2} vs W (pre-cut);W [GeV];Q^{2} [GeV^{2}]",
                200, 0, 5, 200, 0, 12);
            h_Q2_vs_xB_precut = svc->book_hist2D("/DIS/PreCut/Q2_vs_xB", "Q^{2} vs x_{B} (pre-cut);x_{B};Q^{2} [GeV^{2}]",
                200, 0, 1, 200, 0, 12);
            h_W_vs_y_precut = svc->book_hist2D("/DIS/PreCut/W_vs_y", "W vs y (pre-cut);y;W [GeV]",
                200, 0, 1, 200, 0, 5);
            h_e_theta_vs_p_precut = svc->book_hist2D("/DIS/PreCut/e_theta_vs_p", "#theta vs p (pre-cut);p [GeV/c];#theta [deg]",
                200, 0, 11, 200, 0, 45);

            // 2D correlation histograms (post-cut)
            h_Q2_vs_W = svc->book_hist2D("/DIS/Q2_vs_W", "Q^{2} vs W;W [GeV];Q^{2} [GeV^{2}]",
                200, 0, 5, 200, 0, 12);
            h_Q2_vs_xB = svc->book_hist2D("/DIS/Q2_vs_xB", "Q^{2} vs x_{B};x_{B};Q^{2} [GeV^{2}]",
                200, 0, 1, 200, 0, 12);
            h_W_vs_y = svc->book_hist2D("/DIS/W_vs_y", "W vs y;y;W [GeV]",
                200, 0, 1, 200, 0, 5);

            // N-1 histograms (one per cut, indexed by cut position)
            // Cuts: 0=All Events, 1=Has Electron, 2=Q2 Cut, 3=W Cut, 4=y Cut
            m_nminus1_hists.resize(m_cutflow.num_cuts(), nullptr);
            m_nminus1_hists[2] = svc->book_q2_hist("/DIS/NMinus1/Q2", "Q^{2} (N-1)");
            m_nminus1_hists[3] = svc->book_w_hist("/DIS/NMinus1/W", "W (N-1)");
            m_nminus1_hists[4] = svc->book_hist1D("/DIS/NMinus1/y", "y (N-1);y;Events", 200, 0, 1);
        }
    }
};

}  // namespace clas12::algorithms