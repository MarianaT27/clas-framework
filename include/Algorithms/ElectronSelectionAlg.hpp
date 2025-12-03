#pragma once

#include <optional>
#include "Core/Cuts.hpp"
#include "Core/Detectors.hpp"
#include "Core/Helpers.hpp"
#include "Core/Particle.hpp"
#include "Core/ReadBank.hpp"
#include "Core/Target.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"

namespace Algorithms {

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
    explicit ElectronSelectionAlg(std::string name = "ElectronSelectionAlg", Core::Target target = Core::Target::LD2, Core::Polarity polarity = Core::Polarity::Outbending)
        : Algorithm(std::move(name)), m_target(target), m_polarity(polarity), m_cutflow("Electron Selection") {}

    auto initialize() -> Framework::StatusCode override {
        // Initialize service handles using the typed ServiceHandle pattern
        // LoggingService is optional - we continue if it's not available
        auto log_sc = initialize_handle(m_log_svc);
        if (log_sc.is_failure()) {
            // LoggingService is optional, just print a warning
            fmt::print(stderr, "WARNING: LoggingService not available\n");
        }

        if (m_log_svc) {
            LOG_INFO(m_log_svc.get(), "Initializing algorithm");
        }

        // HistogramService is required
        auto hist_sc = initialize_handle(m_hist_svc);
        if (hist_sc.is_failure()) {
            if (m_log_svc) {
                LOG_ERROR(m_log_svc.get(), "Could not get HistogramService: {}", hist_sc.message());
            }
            fmt::print(stderr, "ERROR: {}\n", hist_sc.message());
            return hist_sc;
        }

        // NTupleService is optional
        auto ntuple_sc = initialize_handle(m_ntuple_svc);
        if (ntuple_sc.is_failure()) {
            if (m_log_svc) {
                LOG_WARNING(m_log_svc.get(), "Could not get NTupleService - N-tuple output disabled");
            }
        }

        // ConditionsService is optional
        auto cond_sc = initialize_handle(m_cond_svc);
        if (cond_sc.is_failure()) {
            if (m_log_svc) {
                LOG_WARNING(m_log_svc.get(), "Could not get ConditionsService");
            }
        }

        // Book histograms
        book_histograms();

        // Setup N-tuple branches
        if (m_ntuple_svc) {
            setup_ntuple();
        }

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");

        // Inject logger into cutflow for structured output
        if (m_log_svc) {
            m_cutflow.set_logger(m_log_svc.get());
            LOG_INFO(m_log_svc.get(), "Initialized with 8 cuts in cutflow");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get electrons from event
        const auto& electrons = event.particles(Core::PIDs::ELECTRON);

        auto& hipo_event = event.hipo_event();
        hipo::bank_map REC_Particle = hipo_event.get_bank_map("REC::Particle");
        hipo::bank_map REC_Scintillator = hipo_event.get_bank_map("REC::Scintillator");
        hipo::bank_map REC_Traj = hipo_event.get_bank_map("REC::Traj");
        hipo::bank_map REC_Track = hipo_event.get_bank_map("REC::Track");
        hipo::bank_map REC_Calorimeter = hipo_event.get_bank_map("REC::Calorimeter");

        if (electrons.empty()) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Has Electron");

        // Select first electron (could be improved with trigger selection)
        const std::optional<Core::Particle>& possible_electron = Core::find_trigger_electron(electrons, {Core::FD, Core::BD});
        if (!possible_electron.has_value()) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        const Core::Particle& electron = possible_electron.value();

        // Apply cuts sequentially
        if (!Core::ElectronCuts::pass_chi2pid_cut(electron.chi2pid())) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Chi2 PID");

        double p = electron.p();
        if (!Core::ElectronCuts::pass_electron_momentum_cut(p)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Momentum > 0.8 GeV");

        if (!Core::ElectronCuts::pass_electron_vz_cut(electron.vz(), m_target)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Vertex Z");

        // Get detector info for electron
        Core::CalorimeterBank calorimeter = Core::read_Calorimeter_bank(REC_Calorimeter.get_bank(), electron.index());
        Core::DCTrajBank traj = Core::read_Traj_bank(REC_Traj.get_bank(), electron.index());

        // Calculate derived quantities
        const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_tot / electron.p();
        const int sector = calorimeter.pcal.sector;

        if (!Core::ElectronCuts::pass_sampling_fraction_cut(SF, p, sector, m_target, m_polarity, sf)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Sampling Fraction");

        if (!Core::ElectronCuts::pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("PCAL Fiducial");

        if (!Core::ElectronCuts::pass_chi2ndf_cut(traj.region1.edge, traj.region2.edge, traj.region3.edge)) {
            event.set_decision("HasGoodElectron", false);
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("DC Chi2/NDF");

        // Electron passed all cuts - set decision and store for downstream algorithms
        event.set_decision("HasGoodElectron", true);
        event.store_particles<Core::Particle>("SelectedElectrons", {electron});

        // Fill histograms after selection
        h_momentum->Fill(p);
        h_theta->Fill(electron.theta());
        h_phi->Fill(electron.phi());
        h_vz->Fill(electron.vz());

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
        if (m_log_svc) {
            LOG_INFO(m_log_svc.get(), "Finalizing algorithm");
            if (m_ntuple_svc) {
                auto entries = m_ntuple_svc->get_entries("electron_kinematics");
                LOG_INFO(m_log_svc.get(), "N-tuple has {} entries", entries);
            }
        }

        m_cutflow.print();
        return Framework::StatusCode::SUCCESS();
    }

   private:
    Core::Target m_target;
    Core::Polarity m_polarity;
    Framework::Cutflow m_cutflow;
    Core::SamplingFractionCut sf;

    // Services - using typed ServiceHandle pattern for validation at initialization
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};
    Framework::ServiceHandle<Framework::ConditionsService> m_cond_svc{"ConditionsService"};

    // Histograms (non-owning pointers managed by HistogramService)
    TH1D* h_momentum = nullptr;
    TH1D* h_theta = nullptr;
    TH1D* h_phi = nullptr;
    TH1D* h_vz = nullptr;

    // N-tuple variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_e_chi2pid = 0.0;

    auto book_histograms() -> void {
        h_momentum = m_hist_svc->book_hist1D("/ElectronKinematics/momentum", "Electron Momentum;p [GeV/c];Events", 120, 0, 12);
        h_theta = m_hist_svc->book_hist1D("/ElectronKinematics/theta", "Electron Theta;#theta [deg];Events", 180, 0, 180);
        h_phi = m_hist_svc->book_hist1D("/ElectronKinematics/phi", "Electron Phi;#phi [deg];Events", 180, -180, 180);
        h_vz = m_hist_svc->book_hist1D("/ElectronKinematics/vz", "Electron Vertex Z;v_{z} [cm];Events", 200, -20, 20);
    }

    auto setup_ntuple() -> void {
        m_ntuple_svc->register_branch("electron_kinematics", "e_p", &m_e_p);
        m_ntuple_svc->register_branch("electron_kinematics", "e_theta", &m_e_theta);
        m_ntuple_svc->register_branch("electron_kinematics", "e_phi", &m_e_phi);
        m_ntuple_svc->register_branch("electron_kinematics", "e_vz", &m_e_vz);
        m_ntuple_svc->register_branch("electron_kinematics", "e_chi2pid", &m_e_chi2pid);
        if (m_log_svc) LOG_INFO(m_log_svc.get(), "N-tuple branches registered for electron kinematics");
    }
};

}  // namespace Algorithms
