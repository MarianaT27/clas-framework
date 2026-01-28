#pragma once

#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/CutBuilder.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
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

    NTUPLE_FIELDS(e_p, e_theta, e_phi, e_vz, e_chi2pid, Q2, W, nu, xB, y)
};

/**
 * @brief Algorithm for DIS event selection
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>()
 * - Property system for configuration
 * - CutBuilder for fluent cut definitions
 * - NTupleWriter for simplified N-tuple management
 */
class DISSelectionAlg : public clas12::framework::SimpleAlgorithm<DISSelectionAlg> {
   public:
    explicit DISSelectionAlg(std::string name = "DISSelectionAlg",
                              clas12::physics::DISAnalysis::KinematicRegion dis_region = clas12::physics::DISAnalysis::full_phase_space())
        : SimpleAlgorithm(std::move(name))
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
    }

    auto set_dis_region(const clas12::physics::DISAnalysis::KinematicRegion& region) -> void {
        m_dis_region = region;
    }

    /**
     * @brief Configure required services
     */
    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_ntuple_svc = &require_service<clas12::framework::NTupleService>("NTupleService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Initializing DIS selection algorithm");

        book_histograms();

        // Setup N-tuple using NTupleWriter
        if (m_enable_ntuple.get()) {
            clas12::framework::safe(m_ntuple_svc).with([this](auto* svc) {
                m_ntuple.init(svc, "dis_kinematics");
                Log::info(name(), "N-tuple branches registered for DIS kinematics");
            });
        }

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Q2 Cut");
        m_cutflow.add_cut("W Cut");
        m_cutflow.add_cut("y Cut");

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
        m_cutflow.pass("All Events");

        // Check for good electron decision from upstream algorithm
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodElectron)) {
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Get selected electrons using typed retrieval
        auto electrons_opt = event.retrieve_typed(clas12::framework::Keys::SelectedElectronsCapital);
        if (!electrons_opt || electrons_opt->empty()) {
            return clas12::framework::StatusCode::FILTERED();
        }

        clas12::physics::DISAnalysis::DISKinematics kin =
            clas12::physics::DISAnalysis::calculate_kinematics(electrons_opt->front());

        // Apply DIS region cuts using CutBuilder
        bool passed = clas12::framework::CutBuilder(m_cutflow)
            .require_in_range("Q2 Cut", kin.q2, m_q2_min.get(), m_q2_max.get())
            .require_in_range("W Cut", kin.w, m_w_min.get(), m_w_max.get())
            .require_in_range("y Cut", kin.y, m_y_min.get(), m_y_max.get())
            .evaluate();

        if (!passed) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Electron passed all cuts - set decision and store for downstream algorithms
        event.set_decision_typed(clas12::framework::Decisions::HasGoodDISElectron, true);
        event.store_typed(clas12::framework::Keys::SelectedDISElectron, std::vector<clas12::Particle>{electrons_opt->front()});
        event.store_typed(clas12::framework::Keys::DIS_Q2, kin.q2);
        event.store_typed(clas12::framework::Keys::DIS_W, kin.w);
        event.store_typed(clas12::framework::Keys::DIS_xB, kin.xb);
        event.store_typed(clas12::framework::Keys::DIS_y, kin.y);
        event.store_typed(clas12::framework::Keys::DIS_nu, kin.nu);

        // Fill histograms (thread-safe)
        if (h_Q2) h_Q2->fill(kin.q2);
        if (h_W) h_W->fill(kin.w);
        if (h_nu) h_nu->fill(kin.nu);
        if (h_xB) h_xB->fill(kin.xb);
        if (h_y) h_y->fill(kin.y);

        // Fill N-tuple using NTupleWriter
        if (m_ntuple && m_enable_ntuple.get()) {
            const auto& electron = electrons_opt->front();
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
            m_ntuple.fill();
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing DIS selection algorithm");

        clas12::framework::safe(m_ntuple_svc).with([this](auto* svc) {
            auto entries = svc->get_entries("dis_kinematics");
            Log::info(name(), "N-tuple has {} entries", entries);
        });

        m_cutflow.print();
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::framework::Cutflow m_cutflow;
    clas12::physics::DISAnalysis::KinematicRegion m_dis_region;

    // Properties for DIS cuts
    clas12::framework::Property<double> m_q2_min{"q2_min", 1.0, "Minimum Q2 [GeV^2]"};
    clas12::framework::Property<double> m_q2_max{"q2_max", 100.0, "Maximum Q2 [GeV^2]"};
    clas12::framework::Property<double> m_w_min{"w_min", 2.0, "Minimum W [GeV]"};
    clas12::framework::Property<double> m_w_max{"w_max", 100.0, "Maximum W [GeV]"};
    clas12::framework::Property<double> m_y_min{"y_min", 0.0, "Minimum y"};
    clas12::framework::Property<double> m_y_max{"y_max", 0.85, "Maximum y"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles (auto-injected via require_service)
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;

    // Lock-free atomic histograms (managed by HistogramService)
    clas12::framework::AtomicHistogram1D* h_Q2 = nullptr;
    clas12::framework::AtomicHistogram1D* h_W = nullptr;
    clas12::framework::AtomicHistogram1D* h_nu = nullptr;
    clas12::framework::AtomicHistogram1D* h_xB = nullptr;
    clas12::framework::AtomicHistogram1D* h_y = nullptr;

    // N-tuple writer (replaces individual variables + manual registration)
    clas12::framework::NTupleWriter<DISNTupleData> m_ntuple;

    auto book_histograms() -> void {
        clas12::framework::safe(m_hist_svc).with([this](auto* svc) {
            // All physics histogram methods now use atomic histograms by default
            h_Q2 = svc->book_q2_hist("/DIS/Q2", "Q^{2}");
            h_W = svc->book_w_hist("/DIS/W", "W");
            h_nu = svc->book_hist("/DIS/nu", "#nu;#nu;Events", 180, 0, 11);
            h_xB = svc->book_xb_hist("/DIS/xB", "x_{B}");
            h_y = svc->book_hist("/DIS/y", "y;y;Events", 200, 0, 1);
        });
    }
};

}  // namespace clas12::algorithms