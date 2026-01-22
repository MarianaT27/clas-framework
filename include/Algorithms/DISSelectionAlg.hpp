#pragma once

#include "Framework/CutBuilder.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Decisions.hpp"
#include "Framework/Event.hpp"
#include "Framework/Property.hpp"
#include "Framework/SimpleAlgorithm.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/ThreadSafeHistogram.hpp"
#include "Physics/DISAnalysis.hpp"

namespace Algorithms {

using Framework::Log;

/**
 * @brief Algorithm for DIS event selection
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>()
 * - Property system for configuration
 * - CutBuilder for fluent cut definitions
 */
class DISSelectionAlg : public Framework::SimpleAlgorithm<DISSelectionAlg> {
   public:
    explicit DISSelectionAlg(std::string name = "DISSelectionAlg",
                              Physics::DISAnalysis::KinematicRegion dis_region = Physics::DISAnalysis::full_phase_space())
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

    auto set_dis_region(const Physics::DISAnalysis::KinematicRegion& region) -> void {
        m_dis_region = region;
    }

    /**
     * @brief Configure required services
     */
    auto configure() -> void override {
        m_hist_svc = &require_service<Framework::HistogramService>("HistogramService");
        m_ntuple_svc = &require_service<Framework::NTupleService>("NTupleService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> Framework::StatusCode override {
        Log::info(name(), "Initializing DIS selection algorithm");

        book_histograms();
        if (m_ntuple_svc && m_ntuple_svc->is_valid() && m_enable_ntuple.get()) {
            setup_ntuple();
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

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Check for good electron decision from upstream algorithm
        if (!event.passes_decision_typed(Framework::Decisions::HasGoodElectron)) {
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Get selected electrons using typed retrieval
        auto electrons_opt = event.retrieve_typed(Framework::Keys::SelectedElectronsCapital);
        if (!electrons_opt || electrons_opt->empty()) {
            return Framework::StatusCode::FILTERED();
        }

        Physics::DISAnalysis::DISKinematics kin =
            Physics::DISAnalysis::calculate_kinematics(electrons_opt->front());

        // Apply DIS region cuts using CutBuilder
        bool passed = Framework::CutBuilder(m_cutflow)
            .require_in_range("Q2 Cut", kin.q2, m_q2_min.get(), m_q2_max.get())
            .require_in_range("W Cut", kin.w, m_w_min.get(), m_w_max.get())
            .require_in_range("y Cut", kin.y, m_y_min.get(), m_y_max.get())
            .evaluate();

        if (!passed) {
            return Framework::StatusCode::FILTERED();
        }

        // Electron passed all cuts - set decision and store for downstream algorithms
        event.set_decision_typed(Framework::Decisions::HasGoodDISElectron, true);
        event.store_typed(Framework::Keys::SelectedDISElectron, std::vector<CLAS::Particle>{electrons_opt->front()});
        event.store_typed(Framework::Keys::DIS_Q2, kin.q2);
        event.store_typed(Framework::Keys::DIS_W, kin.w);
        event.store_typed(Framework::Keys::DIS_xB, kin.xb);
        event.store_typed(Framework::Keys::DIS_y, kin.y);
        event.store_typed(Framework::Keys::DIS_nu, kin.nu);

        // Fill histograms (thread-safe)
        if (h_Q2) h_Q2->fill(kin.q2);
        if (h_W) h_W->fill(kin.w);
        if (h_nu) h_nu->fill(kin.nu);
        if (h_xB) h_xB->fill(kin.xb);
        if (h_y) h_y->fill(kin.y);

        // Fill N-tuple if service is available
        if (m_ntuple_svc && m_ntuple_svc->is_valid() && m_enable_ntuple.get()) {
            // electrons_opt is already retrieved above
            if (electrons_opt && !electrons_opt->empty()) {
                const auto& electron = electrons_opt->front();
                m_e_p = electron.p();
                m_e_theta = electron.theta();
                m_e_phi = electron.phi();
                m_e_vz = electron.vz();
                m_e_chi2pid = electron.chi2pid();
            }
            m_Q2 = kin.q2;
            m_W = kin.w;
            m_nu = kin.nu;
            m_xB = kin.xb;
            m_y = kin.y;
            m_ntuple_svc->get()->fill("dis_kinematics");
        }

        return Framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> Framework::StatusCode override {
        Log::info(name(), "Finalizing DIS selection algorithm");
        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
            auto entries = m_ntuple_svc->get()->get_entries("dis_kinematics");
            Log::info(name(), "N-tuple has {} entries", entries);
        }

        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

   private:
    Framework::Cutflow m_cutflow;
    Physics::DISAnalysis::KinematicRegion m_dis_region;

    // Properties for DIS cuts
    Framework::Property<double> m_q2_min{"q2_min", 1.0, "Minimum Q2 [GeV^2]"};
    Framework::Property<double> m_q2_max{"q2_max", 100.0, "Maximum Q2 [GeV^2]"};
    Framework::Property<double> m_w_min{"w_min", 2.0, "Minimum W [GeV]"};
    Framework::Property<double> m_w_max{"w_max", 100.0, "Maximum W [GeV]"};
    Framework::Property<double> m_y_min{"y_min", 0.0, "Minimum y"};
    Framework::Property<double> m_y_max{"y_max", 0.85, "Maximum y"};
    Framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles (auto-injected via require_service)
    Framework::ServiceHandle<Framework::HistogramService>* m_hist_svc = nullptr;
    Framework::ServiceHandle<Framework::NTupleService>* m_ntuple_svc = nullptr;

    // Thread-safe histograms (managed by HistogramService)
    Framework::ThreadSafeHist1D* h_Q2 = nullptr;
    Framework::ThreadSafeHist1D* h_W = nullptr;
    Framework::ThreadSafeHist1D* h_nu = nullptr;
    Framework::ThreadSafeHist1D* h_xB = nullptr;
    Framework::ThreadSafeHist1D* h_y = nullptr;

    // N-tuple variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_e_chi2pid = 0.0;
    double m_Q2 = 0.0;
    double m_W = 0.0;
    double m_nu = 0.0;
    double m_xB = 0.0;
    double m_y = 0.0;

    auto book_histograms() -> void {
        if (!m_hist_svc || !m_hist_svc->is_valid()) return;
        auto* svc = m_hist_svc->get();
        h_Q2 = svc->book_safe_hist1D("/DIS/Q2", "Q^{2};Q^{2} [GeV^{2}/c^{2}];Events", 120, 0, 12);
        h_W = svc->book_safe_hist1D("/DIS/W", "W;W [GeV];Events", 180, 0, 10);
        h_nu = svc->book_safe_hist1D("/DIS/nu", "#nu;#nu;Events", 180, 0, 11);
        h_xB = svc->book_safe_hist1D("/DIS/xB", "x_{B};x_{B} [GeV];Events", 200, 0, 1.2);
        h_y = svc->book_safe_hist1D("/DIS/y", "y;y;Events", 200, 0, 1);
    }

    auto setup_ntuple() -> void {
        if (!m_ntuple_svc || !m_ntuple_svc->is_valid()) return;
        auto* svc = m_ntuple_svc->get();
        svc->register_branch("dis_kinematics", "e_p", &m_e_p);
        svc->register_branch("dis_kinematics", "e_theta", &m_e_theta);
        svc->register_branch("dis_kinematics", "e_phi", &m_e_phi);
        svc->register_branch("dis_kinematics", "e_vz", &m_e_vz);
        svc->register_branch("dis_kinematics", "e_chi2pid", &m_e_chi2pid);
        svc->register_branch("dis_kinematics", "Q2", &m_Q2);
        svc->register_branch("dis_kinematics", "W", &m_W);
        svc->register_branch("dis_kinematics", "nu", &m_nu);
        svc->register_branch("dis_kinematics", "xB", &m_xB);
        svc->register_branch("dis_kinematics", "y", &m_y);
        Log::info(name(), "N-tuple branches registered for DIS kinematics");
    }
};

}  // namespace Algorithms