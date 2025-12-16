#pragma once

#include <optional>
#include "CLAS/Detectors.hpp"
#include "CLAS/ElectronCuts.hpp"
#include "CLAS/Particle.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/HistogramTemplates.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"

namespace Algorithms {

// Import Log singleton for convenient logging
using Framework::Log;

/**
 * @brief Algorithm for electron selection and kinematics
 *
 * This algorithm demonstrates the new framework by:
 * - Using services for histogram management and conditions
 * - Implementing a proper cutflow
 * - Separating concerns (selection logic vs. framework)
 */
class ElectronSelectionAlg : public Framework::Algorithm {
   public:
    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg", CLAS::Target target = CLAS::Target::LD2, CLAS::Polarity polarity = CLAS::Polarity::Outbending)
        : Algorithm(std::move(name)), m_target(target), m_polarity(polarity), m_cutflow("Electron Selection") {}

    auto initialize() -> Framework::StatusCode override {
        // Initialize service handles using the typed ServiceHandle pattern
        // LoggingService is optional - we continue if it's not available
        auto log_sc = initialize_handle(m_log_svc);
        if (log_sc.is_failure()) Log::error(name(), "WARNING: LoggingService not available");

        Log::info(name(), "Initializing algorithm");

        // HistogramService is required
        auto hist_sc = initialize_handle(m_hist_svc);
        if (hist_sc.is_failure()) {
            Log::error(name(), "Could not get HistogramService: {}", hist_sc.message());
            return hist_sc;
        }

        // NTupleService is optional
        auto ntuple_sc = initialize_handle(m_ntuple_svc);
        if (ntuple_sc.is_failure()) Log::warning(name(), "Could not get NTupleService - N-tuple output disabled");

        // ConditionsService is optional
        auto cond_sc = initialize_handle(m_cond_svc);
        if (cond_sc.is_failure()) Log::warning(name(), "Could not get ConditionsService");

        // SamplingFractionService is required for electron selection
        auto sf_sc = initialize_handle(m_sf_svc);
        if (sf_sc.is_failure()) {
            Log::error(name(), "Could not get SamplingFractionService: {}", sf_sc.message());
            return sf_sc;
        }

        book_histograms();
        if (m_ntuple_svc) setup_ntuple();

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");

        {  // Inject logger into cutflow for structured output
            m_cutflow.set_logger(m_log_svc.get());
            Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get electrons from event
        const auto& electrons = event.particles(CLAS::PIDs::ELECTRON);

        auto& hipo_event = event.hipo_event();
        auto [REC_Calorimeter, REC_Traj] = hipo_event.banks("REC::Calorimeter", "REC::Traj");

        if (electrons.empty()) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Select first electron (could be improved with trigger selection)
        const std::optional<CLAS::Particle>& possible_electron = Physics::ParticleUtils::find_trigger_electron(electrons, {CLAS::FD, CLAS::BD});
        if (!possible_electron.has_value()) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        const CLAS::Particle& electron = possible_electron.value();

        // Apply cuts sequentially
        if (!CLAS::ElectronCuts::pass_chi2pid_cut(electron.chi2pid())) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Chi2 PID");

        double p = electron.p();
        if (!CLAS::ElectronCuts::pass_electron_momentum_cut(p)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Momentum > 0.8 GeV");

        if (!CLAS::ElectronCuts::pass_electron_vz_cut(electron.vz(), m_target)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Vertex Z");

        // Get detector info for electron using BankView
        CLAS::CalorimeterBank calorimeter = CLAS::read_Calorimeter_bank(*REC_Calorimeter, electron.index());
        CLAS::DCTrajBank traj = CLAS::read_Traj_bank(*REC_Traj, electron.index());

        // Calculate derived quantities
        const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_tot / electron.p();
        const int sector = calorimeter.pcal.sector;

        if (!m_sf_svc->passes_cut(SF, p, sector, m_target, m_polarity)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Sampling Fraction");

        if (!CLAS::ElectronCuts::pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("PCAL Fiducial");

        if (!CLAS::ElectronCuts::pass_chi2ndf_cut(traj.region1.edge, traj.region2.edge, traj.region3.edge)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("DC Chi2/NDF");

        // Electron passed all cuts - set decision and store for downstream algorithms
        event.set_decision("HasGoodElectron", true);
        event.store_particles<CLAS::Particle>("SelectedElectrons", {electron});

        // Fill histograms after selection using templates
        (*m_kinematics_hists)["p"].fill(p);
        (*m_kinematics_hists)["theta"].fill(electron.theta());
        (*m_kinematics_hists)["phi"].fill(electron.phi());
        m_kinematics_hists->hist2d("p_vs_theta").fill(electron.theta(), p);
        m_kinematics_hists->hist2d("phi_vs_theta").fill(electron.theta(), electron.phi());

        // Fill electron ID histograms
        (*m_electron_id_hists)["chi2pid"].fill(electron.chi2pid());
        (*m_electron_id_hists)["vz"].fill(electron.vz());
        (*m_electron_id_hists)["sampling_fraction"].fill(SF);
        m_electron_id_hists->hist2d("sf_vs_p").fill(p, SF);

        // Fill N-tuple if service is available
        if (m_ntuple_svc) {
            m_e_p = p;
            m_e_theta = electron.theta();
            m_e_phi = electron.phi();
            m_e_vz = electron.vz();
            m_e_chi2pid = electron.chi2pid();
            m_ntuple_svc->fill("electron_kinematics");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        {
            Log::info(name(), "Finalizing algorithm");
            if (m_ntuple_svc) {
                auto entries = m_ntuple_svc->get_entries("electron_kinematics");
                Log::info(name(), "N-tuple has {} entries", entries);
            }
        }

        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

   private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    Framework::Cutflow m_cutflow;

    // Services - using typed ServiceHandle pattern for validation at initialization
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};
    Framework::ServiceHandle<Framework::ConditionsService> m_cond_svc{"ConditionsService"};
    Framework::ServiceHandle<Framework::SamplingFractionService> m_sf_svc{"SamplingFractionService"};

    // Histogram groups using templates
    std::unique_ptr<Framework::HistogramGroup> m_kinematics_hists;
    std::unique_ptr<Framework::HistogramGroup> m_electron_id_hists;

    // N-tuple variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_e_chi2pid = 0.0;

    auto book_histograms() -> void {
        // Use histogram templates for standard electron histograms
        m_kinematics_hists = Framework::HistogramTemplates::ParticleKinematics::create(
            m_hist_svc.get(), "/Electron/Kinematics");
        m_electron_id_hists = Framework::HistogramTemplates::ElectronID::create(
            m_hist_svc.get(), "/Electron/ID");
    }

    auto setup_ntuple() -> void {
        m_ntuple_svc->register_branch("electron_kinematics", "e_p", &m_e_p);
        m_ntuple_svc->register_branch("electron_kinematics", "e_theta", &m_e_theta);
        m_ntuple_svc->register_branch("electron_kinematics", "e_phi", &m_e_phi);
        m_ntuple_svc->register_branch("electron_kinematics", "e_vz", &m_e_vz);
        m_ntuple_svc->register_branch("electron_kinematics", "e_chi2pid", &m_e_chi2pid);
        Log::info(name(), "N-tuple branches registered for electron kinematics");
    }
};

}  // namespace Algorithms
