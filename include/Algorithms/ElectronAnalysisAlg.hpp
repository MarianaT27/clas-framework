#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/Event.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/SystematicVariation.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"
#include "CLAS/ElectronCuts.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Physics/Kinematics.hpp"
#include "TH1D.h"
#include "TH2D.h"
#include <memory>

namespace Algorithms {

// Import Log singleton for convenient logging
using Framework::Log;

/**
 * @brief Full electron analysis algorithm (refactored from study1::Reader)
 *
 * This demonstrates how to refactor the original Reader.cpp to use
 * the new framework with all Phase 3 improvements:
 * - HistogramService for histogram management
 * - Cutflow for automatic efficiency tracking
 * - Systematic variations
 * - Physics calculators
 * - Clean separation of concerns
 */
class ElectronAnalysisAlg : public Framework::Algorithm {
public:
    explicit ElectronAnalysisAlg(std::string name = "ElectronAnalysis",
                                CLAS::Target target = CLAS::Target::LD2,
                                CLAS::Polarity polarity = CLAS::Polarity::Outbending,
                                bool enable_systematics = false)
        : Algorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_enable_systematics(enable_systematics)
        , m_cutflow("Electron Selection") {}

    auto initialize() -> Framework::StatusCode override {
        // Get services
        m_log_svc = service<Framework::LoggingService>("LoggingService");
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        m_ntuple_svc = service<Framework::NTupleService>("NTupleService");
        m_cond_svc = service<Framework::ConditionsService>("ConditionsService");
        m_particle_svc = service<Framework::ParticleSelectorService>("ParticleSelectorService");
        m_sf_svc = service<Framework::SamplingFractionService>("SamplingFractionService");

        if (Log::is_available()) {
            Log::info(name(), "Initializing algorithm");
            Log::info(name(), "Target: {}, Polarity: {}, Systematics: {}",
                     m_target == CLAS::Target::LD2 ? "LD2" : "Other",
                     m_polarity == CLAS::Polarity::Outbending ? "Outbending" : "Inbending",
                     m_enable_systematics ? "enabled" : "disabled");
        }

        if (!m_hist_svc) {
            Log::error(name(), "Could not get HistogramService");
            return Framework::StatusCode::FAILURE();
        }

        if (!m_sf_svc) {
            Log::error(name(), "Could not get SamplingFractionService");
            return Framework::StatusCode::FAILURE();
        }

        // Setup cutflow
        setup_cutflow();

        // Book histograms
        book_histograms();

        // Setup N-tuple branches
        if (m_ntuple_svc) {
            setup_ntuple();
        } else {
            Log::warning(name(), "NTupleService not available - N-tuple output disabled");
        }

        // Setup systematic variations if enabled
        if (m_enable_systematics) {
            setup_systematics();
            if (Log::is_available()) {
                Log::info(name(), "Systematic variations enabled");
            }
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get REC::Event for start time
        auto& hipo_event = event.hipo_event();
        hipo::bank_map REC_Event = hipo_event.get_bank_map("REC::Event");
        if (REC_Event.get_rows() == 0) return Framework::StatusCode::SUCCESS();

        hipo::bank_map REC_Particle = hipo_event.get_bank_map("REC::Particle");
        hipo::bank_map REC_Scintillator = hipo_event.get_bank_map("REC::Scintillator");
        hipo::bank_map REC_Traj = hipo_event.get_bank_map("REC::Traj");
        hipo::bank_map REC_Track = hipo_event.get_bank_map("REC::Track");
        hipo::bank_map REC_Calorimeter = hipo_event.get_bank_map("REC::Calorimeter");

        // Extract electrons using ParticleSelectorService
        Framework::ParticleMap particles = m_particle_svc->extract_particles(
            REC_Particle,
            {CLAS::PIDs::ELECTRON, CLAS::PIDs::PIMINUS, CLAS::PIDs::PIPLUS,
             CLAS::PIDs::KMINUS, CLAS::PIDs::KPLUS, CLAS::PIDs::PHOTON}
        );

        const auto& electrons = m_particle_svc->get_electrons(particles);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();

        // Select trigger electron
        std::optional<CLAS::Particle> trigger_e = Physics::ParticleUtils::find_trigger_electron(electrons, {2000, 4000});
        if (!trigger_e.has_value()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Trigger Electron");

        // Get detector info for electron
        CLAS::CalorimeterBank calorimeter = CLAS::read_Calorimeter_bank(
            REC_Calorimeter.get_bank(), trigger_e->index());
        CLAS::DCTrajBank traj = CLAS::read_Traj_bank(
            REC_Traj.get_bank(), trigger_e->index());
        CLAS::TrackBank track = CLAS::read_Track_bank(
            REC_Track.get_bank(), trigger_e->index());

        // Apply cuts sequentially and fill histograms after each cut
        auto electron = trigger_e.value();

        // Calculate derived quantities
        const double E_tot = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_tot / electron.p();
        const double p_e = electron.p();
        const int sector = calorimeter.pcal.sector;

        // Chi2 PID cut
        if (!CLAS::ElectronCuts::pass_chi2pid_cut(electron.chi2pid())) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Chi2 PID");
        fill_after_cut("Chi2PID", electron, calorimeter, traj, track);

        // Momentum cut
        if (!CLAS::ElectronCuts::pass_electron_momentum_cut(p_e)) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Momentum > 0.8 GeV");
        fill_after_cut("Momentum", electron, calorimeter, traj, track);

        // Vertex Z cut
        if (!CLAS::ElectronCuts::pass_electron_vz_cut(electron.vz(), m_target)) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Vertex Z");
        fill_after_cut("VertexZ", electron, calorimeter, traj, track);

        // Sampling fraction cut
        if (!m_sf_svc->passes_cut(SF, p_e, sector, m_target, m_polarity)) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("Sampling Fraction");
        fill_after_cut("SamplingFraction", electron, calorimeter, traj, track);

        // PCAL fiducial cut
        if (!CLAS::ElectronCuts::pass_pcal_fiducial_cut(calorimeter.pcal.lv, calorimeter.pcal.lw)) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("PCAL Fiducial");
        fill_after_cut("PCALFiducial", electron, calorimeter, traj, track);

        // DC chi2/ndf cut
        if (!CLAS::ElectronCuts::pass_chi2ndf_cut(traj.region1.edge, traj.region2.edge, traj.region3.edge)) {
            return Framework::StatusCode::SUCCESS();
        }
        m_cutflow.pass("DC Chi2/NDF");
        fill_after_cut("DCChi2", electron, calorimeter, traj, track);

        // All cuts passed - fill final histograms
        fill_final_histograms(electron, calorimeter, traj, track);

        // Fill N-tuple if service is available
        if (m_ntuple_svc) {
            m_nt_e_p = electron.p();
            m_nt_e_theta = electron.theta();
            m_nt_e_phi = electron.phi();
            m_nt_e_vz = electron.vz();
            m_nt_e_chi2pid = electron.chi2pid();
            m_nt_sampling_fraction = SF;
            m_nt_sector = calorimeter.pcal.sector;

            m_ntuple_svc->fill("electron_analysis");
        }

        // Set decision flag for downstream algorithms
        event.set_decision("HasGoodElectron", true);

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        if (Log::is_available()) {
            Log::info(name(), "Finalizing algorithm");
            auto final_count = m_cutflow.get_count("DC Chi2/NDF");
            Log::info(name(), "Events passing all cuts: {}", final_count);

            if (m_ntuple_svc) {
                auto entries = m_ntuple_svc->get_entries("electron_analysis");
                Log::info(name(), "N-tuple has {} entries", entries);
            }
        }

        fmt::print("\n");
        m_cutflow.print();

        fmt::print("\nElectron Analysis Statistics:\n");
        fmt::print("  Events passing all cuts: {}\n", m_cutflow.get_count("DC Chi2/NDF"));

        return Framework::StatusCode::SUCCESS();
    }

private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    bool m_enable_systematics;
    Framework::Cutflow m_cutflow;
    Framework::SystematicManager m_sys_mgr;

    // Services
    Framework::LoggingService* m_log_svc = nullptr;
    Framework::HistogramService* m_hist_svc = nullptr;
    Framework::NTupleService* m_ntuple_svc = nullptr;
    Framework::ConditionsService* m_cond_svc = nullptr;
    Framework::ParticleSelectorService* m_particle_svc = nullptr;
    Framework::SamplingFractionService* m_sf_svc = nullptr;

    // N-tuple variables
    double m_nt_e_p = 0.0;
    double m_nt_e_theta = 0.0;
    double m_nt_e_phi = 0.0;
    double m_nt_e_vz = 0.0;
    double m_nt_e_chi2pid = 0.0;
    double m_nt_sampling_fraction = 0.0;
    int m_nt_sector = 0;

    // Histograms - organized by category
    struct {
        TH1D* momentum = nullptr;
        TH1D* theta = nullptr;
        TH1D* phi = nullptr;
        TH1D* vz = nullptr;
        TH1D* chi2pid = nullptr;
        TH2D* p_vs_theta = nullptr;
        TH2D* phi_vs_theta = nullptr;
    } h_electron;

    struct {
        TH2D* lu_vs_lv_pcal = nullptr;
        TH2D* lu_vs_lw_pcal = nullptr;
        TH2D* lu_vs_lv_inner = nullptr;
        TH2D* lu_vs_lw_inner = nullptr;
        TH2D* lu_vs_lv_outer = nullptr;
        TH2D* lu_vs_lw_outer = nullptr;
    } h_calorimeter;

    struct {
        std::map<std::string, TH2D*> chi2ndf_vs_edge_r1;
        std::map<std::string, TH2D*> chi2ndf_vs_edge_r2;
        std::map<std::string, TH2D*> chi2ndf_vs_edge_r3;
    } h_dc;

    auto setup_cutflow() -> void {
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Trigger Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");
    }

    auto book_histograms() -> void {
        // Electron kinematics
        h_electron.momentum = m_hist_svc->book_hist1D(
            "/Electron/momentum", "Electron Momentum;p [GeV/c];Events", 120, 0, 12);
        h_electron.theta = m_hist_svc->book_hist1D(
            "/Electron/theta", "Electron #theta;#theta [deg];Events", 180, 0, 180);
        h_electron.phi = m_hist_svc->book_hist1D(
            "/Electron/phi", "Electron #phi;#phi [deg];Events", 180, -180, 180);
        h_electron.vz = m_hist_svc->book_hist1D(
            "/Electron/vz", "Electron v_{z};v_{z} [cm];Events", 200, -20, 20);
        h_electron.chi2pid = m_hist_svc->book_hist1D(
            "/Electron/chi2pid", "Electron #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);

        h_electron.p_vs_theta = m_hist_svc->book_hist2D(
            "/Electron/p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
            180, 0, 180, 120, 0, 12);
        h_electron.phi_vs_theta = m_hist_svc->book_hist2D(
            "/Electron/phi_vs_theta", "#phi vs #theta;#theta [deg];#phi [deg]",
            180, 0, 180, 180, -180, 180);

        // Calorimeter
        h_calorimeter.lu_vs_lv_pcal = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lv_pcal", "PCAL lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_pcal = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lw_pcal", "PCAL lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        // DC Chi2/ndf vs edge for momentum bins
        std::vector<std::string> p_bins = {"1_2", "2_3", "3_4", "4_5", "5_6", "6_7", "7_8", "8_9", "9_10"};
        for (const auto& bin : p_bins) {
            h_dc.chi2ndf_vs_edge_r1[bin] = m_hist_svc->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r1_{}", bin),
                fmt::format("Region 1: p={} GeV;edge;#chi^{{2}}/ndf", bin),
                100, 0, 400, 100, 0, 10);
        }
    }

    auto setup_systematics() -> void {
        m_sys_mgr.add_variation("EnergyScale_Up", Framework::VariationType::EnergyScale, +1.0);
        m_sys_mgr.add_variation("EnergyScale_Down", Framework::VariationType::EnergyScale, -1.0);
        m_sys_mgr.add_variation("MomentumScale_Up", Framework::VariationType::MomentumScale, +1.0);
        m_sys_mgr.add_variation("MomentumScale_Down", Framework::VariationType::MomentumScale, -1.0);
    }

    auto setup_ntuple() -> void {
        // Create and register branches for electron analysis
        m_ntuple_svc->register_branch("electron_analysis", "e_p", &m_nt_e_p);
        m_ntuple_svc->register_branch("electron_analysis", "e_theta", &m_nt_e_theta);
        m_ntuple_svc->register_branch("electron_analysis", "e_phi", &m_nt_e_phi);
        m_ntuple_svc->register_branch("electron_analysis", "e_vz", &m_nt_e_vz);
        m_ntuple_svc->register_branch("electron_analysis", "e_chi2pid", &m_nt_e_chi2pid);
        m_ntuple_svc->register_branch("electron_analysis", "sampling_fraction", &m_nt_sampling_fraction);
        m_ntuple_svc->register_branch("electron_analysis", "sector", &m_nt_sector);

        if (Log::is_available()) {
            Log::info(name(), "N-tuple branches registered for electron analysis");
        }
    }

    auto fill_after_cut([[maybe_unused]] const std::string& cut_name,
                       [[maybe_unused]] const CLAS::Particle& electron,
                       [[maybe_unused]] const CLAS::CalorimeterBank& calo,
                       [[maybe_unused]] const CLAS::DCTrajBank& traj,
                       [[maybe_unused]] const CLAS::TrackBank& track) -> void {
        // Could fill histograms specific to this cut stage
        // For now, just placeholder
    }

    auto fill_final_histograms(const CLAS::Particle& electron,
                              const CLAS::CalorimeterBank& calo,
                              const CLAS::DCTrajBank& traj,
                              const CLAS::TrackBank& track) -> void {
        // Fill electron kinematics
        h_electron.momentum->Fill(electron.p());
        h_electron.theta->Fill(electron.theta());
        h_electron.phi->Fill(electron.phi());
        h_electron.vz->Fill(electron.vz());
        h_electron.chi2pid->Fill(electron.chi2pid());
        h_electron.p_vs_theta->Fill(electron.theta(), electron.p());
        h_electron.phi_vs_theta->Fill(electron.theta(), electron.phi());

        // Fill calorimeter
        h_calorimeter.lu_vs_lv_pcal->Fill(calo.pcal.lv, calo.pcal.lu);
        h_calorimeter.lu_vs_lw_pcal->Fill(calo.pcal.lw, calo.pcal.lu);

        // Fill DC histograms by momentum bin
        double p = electron.p();
        double chi2ndf = track.chi2 / track.ndf;

        auto fill_dc_bin = [&](const std::string& bin) {
            if (h_dc.chi2ndf_vs_edge_r1.count(bin)) {
                h_dc.chi2ndf_vs_edge_r1[bin]->Fill(traj.region1.edge, chi2ndf);
            }
        };

        if (1.0 <= p && p < 2.0) fill_dc_bin("1_2");
        else if (2.0 <= p && p < 3.0) fill_dc_bin("2_3");
        else if (3.0 <= p && p < 4.0) fill_dc_bin("3_4");
        else if (4.0 <= p && p < 5.0) fill_dc_bin("4_5");
        else if (5.0 <= p && p < 6.0) fill_dc_bin("5_6");
        else if (6.0 <= p && p < 7.0) fill_dc_bin("6_7");
        else if (7.0 <= p && p < 8.0) fill_dc_bin("7_8");
        else if (8.0 <= p && p < 9.0) fill_dc_bin("8_9");
        else if (9.0 <= p && p < 10.0) fill_dc_bin("9_10");
    }
};

}  // namespace Algorithms
