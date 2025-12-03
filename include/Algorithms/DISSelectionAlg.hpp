#pragma once

#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Physics/DISAnalysis.hpp"

namespace Algorithms {
/**
 * @brief Algorithm for DIS event selection
 */

class DISSelectionAlg : public Framework::Algorithm {
   public:
    explicit DISSelectionAlg(std::string name = "DISSelectionAlg", Physics::DISAnalysis::KinematicRegion dis_region = Physics::DISAnalysis::full_phase_space()) : Algorithm(std::move(name)), m_cutflow("DIS Selection"), m_dis_region(std::move(dis_region)) {}

    auto set_dis_region(const Physics::DISAnalysis::KinematicRegion& region) -> void { m_dis_region = region; }

    auto initialize() -> Framework::StatusCode override {
        if (m_log_svc) LOG_INFO(m_log_svc.get(), "Initializing DIS selection algorithm");

        auto log_sc = initialize_handle(m_log_svc);
        if (log_sc.is_failure()) fmt::print(stderr, "WARNING: LoggingService not available\n");
        if (m_log_svc) LOG_INFO(m_log_svc.get(), "Initializing algorithm");

        auto hist_sc = initialize_handle(m_hist_svc);
        if (hist_sc.is_failure()) {
            if (m_log_svc) LOG_ERROR(m_log_svc.get(), "Could not get HistogramService: {}", hist_sc.message());
            fmt::print(stderr, "ERROR: {}\n", hist_sc.message());
            return hist_sc;
        }

        auto ntuple_sc = initialize_handle(m_ntuple_svc);
        if (ntuple_sc.is_failure()) {
            if (m_log_svc) LOG_WARNING(m_log_svc.get(), "Could not get NTupleService - N-tuple output disabled");
        }

        auto cond_sc = initialize_handle(m_cond_svc);
        if (cond_sc.is_failure()) {
            if (m_log_svc) LOG_WARNING(m_log_svc.get(), "Could not get ConditionsService");
        }

        book_histograms();
        if (m_ntuple_svc) setup_ntuple();

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Q2 Cut");
        m_cutflow.add_cut("W Cut");
        m_cutflow.add_cut("y Cut");

        if (m_log_svc) {
            m_cutflow.set_logger(m_log_svc.get());
            LOG_INFO(m_log_svc.get(), "Initialized with 8 cuts in cutflow");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Check for good electron decision from upstream algorithm
        if (!event.passes_decision("HasGoodElectron")) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Has Electron");

        Physics::DISAnalysis::DISKinematics kin = Physics::DISAnalysis::calculate_kinematics(event.get_particles<Core::Particle>("SelectedElectrons")->front());
        
        // Apply DIS region cuts one by one for cutflow
        if (kin.q2 < m_dis_region.q2_min || kin.q2 > m_dis_region.q2_max) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Q2 Cut");
        if (kin.w < m_dis_region.w_min || kin.w > m_dis_region.w_max) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("W Cut");
        if (kin.y < m_dis_region.y_min || kin.y > m_dis_region.y_max) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("y Cut");

        // Electron passed all cuts - set decision and store for downstream algorithms
        event.set_decision("HasGoodDISElectron", true);
        event.store_particles<Core::Particle>("SelectedDISElectron", {event.get_particles<Core::Particle>("SelectedElectrons")->front()});
        event.store("dis_Q2", kin.q2);
        event.store("dis_W", kin.w);
        event.store("dis_xB", kin.xb);
        event.store("dis_y", kin.y);
        event.store("dis_nu", kin.nu);

        // Fill histograms
        if (h_Q2) h_Q2->Fill(kin.q2);
        if (h_W) h_W->Fill(kin.w);
        if (h_nu) h_nu->Fill(kin.nu);
        if (h_xB) h_xB->Fill(kin.xb);
        if (h_y) h_y->Fill(kin.y);

        // Fill N-tuple if service is available
        if (m_ntuple_svc) {
            // Get electron from event store (stored by ElectronSelectionAlg)
            auto* electrons = event.get_particles<Core::Particle>("SelectedElectrons");
            if (electrons && !electrons->empty()) {
                const auto& electron = (*electrons)[0];
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
            m_ntuple_svc->fill("dis_kinematics");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {

        if (m_log_svc) {
            LOG_INFO(m_log_svc.get(), "Finalizing DIS selection algorithm");
            if (m_ntuple_svc) {
                auto entries = m_ntuple_svc->get_entries("dis_kinematics");
                LOG_INFO(m_log_svc.get(), "N-tuple has {} entries", entries);
            }
        }

        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

   private:
    Framework::Cutflow m_cutflow;
    Physics::DISAnalysis::KinematicRegion m_dis_region;

    // Services - using typed ServiceHandle pattern for validation at initialization
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};
    Framework::ServiceHandle<Framework::ConditionsService> m_cond_svc{"ConditionsService"};

    // Histograms (non-owning pointers managed by HistogramService)
    TH1D* h_Q2 = nullptr;
    TH1D* h_W = nullptr;
    TH1D* h_nu = nullptr;
    TH1D* h_xB = nullptr;
    TH1D* h_y = nullptr;

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
        h_Q2 = m_hist_svc->book_hist1D("/DIS/Q2", "Q^{2};Q^{2} [GeV^{2}/c^{2}];Events", 120, 0, 12);
        h_W = m_hist_svc->book_hist1D("/DIS/W", "W;W [GeV];Events", 180, 0, 10);
        h_nu = m_hist_svc->book_hist1D("/DIS/nu", "#nu;#nu;Events", 180, 0, 11);
        h_xB = m_hist_svc->book_hist1D("/DIS/xB", "x_{B};x_{B} [GeV];Events", 200, 0, 1.2);
        h_y = m_hist_svc->book_hist1D("/DIS/y", "y;y;Events", 200, 0, 1);
    }

    auto setup_ntuple() -> void {
        m_ntuple_svc->register_branch("dis_kinematics", "e_p", &m_e_p);
        m_ntuple_svc->register_branch("dis_kinematics", "e_theta", &m_e_theta);
        m_ntuple_svc->register_branch("dis_kinematics", "e_phi", &m_e_phi);
        m_ntuple_svc->register_branch("dis_kinematics", "e_vz", &m_e_vz);
        m_ntuple_svc->register_branch("dis_kinematics", "e_chi2pid", &m_e_chi2pid);
        m_ntuple_svc->register_branch("dis_kinematics", "Q2", &m_Q2);
        m_ntuple_svc->register_branch("dis_kinematics", "W", &m_W);
        m_ntuple_svc->register_branch("dis_kinematics", "nu", &m_nu);
        m_ntuple_svc->register_branch("dis_kinematics", "xB", &m_xB);
        m_ntuple_svc->register_branch("dis_kinematics", "y", &m_y);
        if (m_log_svc) LOG_INFO(m_log_svc.get(), "N-tuple branches registered for electron kinematics");
    }
};

}  // namespace Algorithms