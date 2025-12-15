#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include "CLAS/ElectronCuts.hpp"
#include "CLAS/Particle.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"
#include "Framework/SystematicVariation.hpp"
#include "Physics/Kinematics.hpp"
#include "TH1D.h"
#include "TH2D.h"

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
/// Configuration structure for ElectronAnalysisAlg parameters
struct ElectronAnalysisConfig {
    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;
    bool enable_systematics = false;
    bool enable_ntuple = true;
    bool enable_cut_stage_histograms = true;
    // Trigger electron momentum range
    double trigger_p_min = 2000.0;
    double trigger_p_max = 4000.0;
    // Momentum binning for DC histograms
    double p_bin_min = 1.0;
    double p_bin_max = 10.0;
    double p_bin_width = 1.0;
    // Theta binning for DC histograms
    double theta_bin_min = 5.0;
    double theta_bin_max = 40.0;
    double theta_bin_width = 5.0;
};

class ElectronAnalysisAlg : public Framework::Algorithm {
   public:
    using Config = ElectronAnalysisConfig;

    explicit ElectronAnalysisAlg(std::string name = "ElectronAnalysis", const Config& config = Config{})
        : Algorithm(std::move(name)),
          m_config(config),
          m_target(m_config.target),
          m_polarity(m_config.polarity),
          m_enable_systematics(m_config.enable_systematics),
          m_cutflow("Electron Selection") {}

    // Legacy constructor for backward compatibility
    explicit ElectronAnalysisAlg(std::string name, CLAS::Target target, CLAS::Polarity polarity)
        : Algorithm(std::move(name)), m_target(target), m_polarity(polarity), m_cutflow("Electron Selection") {
        m_config.target = target;
        m_config.polarity = polarity;
    }

    /// Access configuration (const)
    [[nodiscard]] auto config() const -> const Config& { return m_config; }

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

        setup_cutflow();
        book_histograms();

        // Initialize systematic variations if enabled
        if (m_enable_systematics) {
            setup_systematics();
            Log::info(name(), "Registered {} systematic variations", m_sys_mgr.variations().size());
        }

        if (m_ntuple_svc && m_config.enable_ntuple) {
            setup_ntuple();
        } else if (!m_ntuple_svc) {
            Log::warning(name(), "NTupleService not available - N-tuple output disabled");
        } else {
            Log::info(name(), "N-tuple output disabled by configuration");
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_cutflow.pass("All Events");

        // Get all needed banks using structured bindings (new hipo interface)
        auto& hipo_event = event.hipo_event();
        auto [REC_Event, REC_Particle, REC_Calorimeter, REC_Cherenkov, REC_Scintillator, REC_Traj, REC_Track] =
            hipo_event.banks("REC::Event", "REC::Particle", "REC::Calorimeter",
                             "REC::Cherenkov", "REC::Scintillator", "REC::Traj", "REC::Track");

        // Early exit if no event data
        if (REC_Event->rows() == 0) return Framework::StatusCode::SUCCESS();

        const double event_start_time = REC_Event->data().get<float>("startTime", 0);

        // Extract particles
        Framework::ParticleMap particles = m_particle_svc->extract_particles(*REC_Particle, {CLAS::PIDs::ELECTRON});

        std::map<int, std::vector<CLAS::Particle>> charged_tracks = m_particle_svc->get_particles_by_charge(*REC_Particle);
        std::vector<CLAS::Particle> neg_tracks = charged_tracks[-1];

        // Process tracks and fill histograms
        process_tracks(neg_tracks, h_neg_tracks, *REC_Calorimeter, *REC_Cherenkov,
                       *REC_Scintillator, *REC_Traj, *REC_Track, event_start_time);

        const auto& electrons = m_particle_svc->get_electrons(particles);
        if (electrons.empty()) return Framework::StatusCode::SUCCESS();

        // Select trigger electron using configurable momentum range
        std::optional<CLAS::Particle> trigger_e = Physics::ParticleUtils::find_trigger_electron(
            electrons, {m_config.trigger_p_min, m_config.trigger_p_max});
        if (!trigger_e.has_value()) return Framework::StatusCode::SUCCESS();
        m_cutflow.pass("Trigger Electron");

        // Get detector info for electron using BankView
        CLAS::CalorimeterBank calorimeter = CLAS::read_Calorimeter_bank(*REC_Calorimeter, trigger_e->index());
        CLAS::DCTrajBank traj = CLAS::read_Traj_bank(*REC_Traj, trigger_e->index());
        CLAS::TrackBank track = CLAS::read_Track_bank(*REC_Track, trigger_e->index());

        // Apply cuts sequentially and fill histograms after each cut
        auto electron = trigger_e.value();

        process_tracks({electron}, h_electron_tracks, *REC_Calorimeter, *REC_Cherenkov,
                       *REC_Scintillator, *REC_Traj, *REC_Track, event_start_time);

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
        if (m_ntuple_svc && m_config.enable_ntuple) {
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

            if (m_ntuple_svc && m_config.enable_ntuple) {
                auto entries = m_ntuple_svc->get_entries("electron_analysis");
                Log::info(name(), "N-tuple has {} entries", entries);
            }
        }

        fmt::print("\n");
        m_cutflow.print();

        return Framework::StatusCode::SUCCESS();
    }

    /// Structure to hold electron candidate with all associated detector info
    struct ElectronCandidate {
        CLAS::Particle particle;
        CLAS::CalorimeterBank calorimeter;
        CLAS::DCTrajBank trajectory;
        CLAS::TrackBank track;
        double sampling_fraction = 0.0;
        double total_energy = 0.0;
        int sector = 0;

        [[nodiscard]] auto is_valid() const -> bool {
            return sector >= 1 && sector <= 6 && particle.p() > 0;
        }
    };

    /// Build electron candidate with all detector information
    [[nodiscard]] auto build_candidate(const CLAS::Particle& electron,
                                        hipo::BankView& REC_Calorimeter,
                                        hipo::BankView& REC_Traj,
                                        hipo::BankView& REC_Track) const -> ElectronCandidate {
        ElectronCandidate candidate;
        candidate.particle = electron;
        candidate.calorimeter = CLAS::read_Calorimeter_bank(REC_Calorimeter, electron.index());
        candidate.trajectory = CLAS::read_Traj_bank(REC_Traj, electron.index());
        candidate.track = CLAS::read_Track_bank(REC_Track, electron.index());
        candidate.total_energy = candidate.calorimeter.pcal.energy +
                                  candidate.calorimeter.inner.energy +
                                  candidate.calorimeter.outer.energy;
        candidate.sampling_fraction = candidate.total_energy / electron.p();
        candidate.sector = candidate.calorimeter.pcal.sector;
        return candidate;
    }

    /// Apply all electron selection cuts and return result
    /// Returns true if electron passes all cuts, fills cutflow along the way
    [[nodiscard]] auto apply_electron_cuts(const ElectronCandidate& candidate) -> bool {
        const auto& e = candidate.particle;
        const auto& calo = candidate.calorimeter;
        const auto& traj = candidate.trajectory;

        // Chi2 PID cut
        if (!CLAS::ElectronCuts::pass_chi2pid_cut(e.chi2pid())) return false;
        m_cutflow.pass("Chi2 PID");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("Chi2PID", e, calo, traj, candidate.track);
        }

        // Momentum cut
        if (!CLAS::ElectronCuts::pass_electron_momentum_cut(e.p())) return false;
        m_cutflow.pass("Momentum > 0.8 GeV");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("Momentum", e, calo, traj, candidate.track);
        }

        // Vertex Z cut
        if (!CLAS::ElectronCuts::pass_electron_vz_cut(e.vz(), m_target)) return false;
        m_cutflow.pass("Vertex Z");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("VertexZ", e, calo, traj, candidate.track);
        }

        // Sampling fraction cut
        if (!m_sf_svc->passes_cut(candidate.sampling_fraction, e.p(), candidate.sector, m_target, m_polarity)) return false;
        m_cutflow.pass("Sampling Fraction");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("SamplingFraction", e, calo, traj, candidate.track);
        }

        // PCAL fiducial cut
        if (!CLAS::ElectronCuts::pass_pcal_fiducial_cut(calo.pcal.lv, calo.pcal.lw)) return false;
        m_cutflow.pass("PCAL Fiducial");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("PCALFiducial", e, calo, traj, candidate.track);
        }

        // DC chi2/ndf cut
        if (!CLAS::ElectronCuts::pass_chi2ndf_cut(traj.region1.edge, traj.region2.edge, traj.region3.edge)) return false;
        m_cutflow.pass("DC Chi2/NDF");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("DCChi2", e, calo, traj, candidate.track);
        }

        return true;
    }

   private:
    Config m_config;
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    bool m_enable_systematics = false;
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

    // Track histograms structure (for neg/pos tracks, pions, etc.)
    // Each histogram is stored per sector (1-6)
    struct TrackHistograms {
        bool booked = false;
        // Calorimeter (per sector)
        std::map<int, TH1D*> nphe;
        std::map<int, TH2D*> EPCal_vs_ECal;
        std::map<int, TH1D*> EPCal;
        std::map<int, TH2D*> EPCal_vs_ECal_cut;
        std::map<int, TH1D*> EPCal_cut;
        std::map<int, TH2D*> Etot_vs_SF;
        std::map<int, TH2D*> Etot_vs_SF_cut;
        std::map<int, TH2D*> p_vs_SF;
        std::map<int, TH2D*> lv_vs_SF;
        std::map<int, TH2D*> lw_vs_SF;
        std::map<int, TH2D*> lu_vs_SF_low;
        std::map<int, TH2D*> lu_vs_SF_high;
        // DC fiducial (per sector)
        std::map<int, TH2D*> edge_vs_theta_r1;
        std::map<int, TH2D*> edge_vs_theta_r2;
        std::map<int, TH2D*> edge_vs_theta_r3;
        std::map<int, TH2D*> edge_vs_chi2ndf_r1;
        std::map<int, TH2D*> edge_vs_chi2ndf_r2;
        std::map<int, TH2D*> edge_vs_chi2ndf_r3;
    };

    TrackHistograms h_neg_tracks;
    TrackHistograms h_electron_tracks;

    // Histograms for each cut stage (dynamically created)
    struct CutStageHistograms {
        bool booked = false;
        TH1D* momentum = nullptr;
        TH1D* theta = nullptr;
        TH1D* phi = nullptr;
        TH1D* vz = nullptr;
        TH2D* sampling_fraction = nullptr;
        TH1D* dc_edge_r1 = nullptr;
    };

    std::map<std::string, CutStageHistograms> m_cut_stage_histograms;

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

        // Calorimeter - PCAL
        h_calorimeter.lu_vs_lv_pcal = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lv_pcal", "PCAL lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_pcal = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lw_pcal", "PCAL lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        // Calorimeter - Inner EC
        h_calorimeter.lu_vs_lv_inner = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lv_inner", "Inner EC lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_inner = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lw_inner", "Inner EC lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        // Calorimeter - Outer EC
        h_calorimeter.lu_vs_lv_outer = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lv_outer", "Outer EC lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_outer = m_hist_svc->book_hist2D(
            "/Calorimeter/lu_vs_lw_outer", "Outer EC lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        // DC Chi2/ndf vs edge for momentum bins
        std::vector<std::string> p_bins = {"1_2", "2_3", "3_4", "4_5", "5_6", "6_7", "7_8", "8_9", "9_10"};
        for (const auto& bin : p_bins) {
            h_dc.chi2ndf_vs_edge_r1[bin] = m_hist_svc->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r1_{}", bin),
                fmt::format("Region 1: p={} GeV;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : p_bins) {
            h_dc.chi2ndf_vs_edge_r2[bin] = m_hist_svc->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r2_{}", bin),
                fmt::format("Region 2: p={} GeV;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : p_bins) {
            h_dc.chi2ndf_vs_edge_r3[bin] = m_hist_svc->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r3_{}", bin),
                fmt::format("Region 3: p={} GeV;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }

        // DC Chi2/ndf vs edge for theta bins 
        std::vector<std::string> theta_bins = {"5_10", "10_15", "15_20", "20_25", "25_30", "30_35", "35_40"};
        for (const auto& bin : theta_bins) {
            h_dc.chi2ndf_vs_edge_r1[bin] = m_hist_svc->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r1_theta_{}", bin),
                fmt::format("Region 1: #theta={} deg;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : theta_bins) {
            h_dc.chi2ndf_vs_edge_r2[bin] = m_hist_svc->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r2_theta_{}", bin),
                fmt::format("Region 2: #theta={} deg;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : theta_bins) {
            h_dc.chi2ndf_vs_edge_r3[bin] = m_hist_svc->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r3_theta_{}", bin),
                fmt::format("Region 3: #theta={} deg;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }


        // Book track histograms for all track categories
        book_track_histograms(h_neg_tracks, "NegativeTracks");
        book_track_histograms(h_electron_tracks, "ElectronTracks");

        // Book cut-stage histograms (must be done at init time for thread safety)
        if (m_config.enable_cut_stage_histograms) {
            book_cut_stage_histograms("Chi2PID");
            book_cut_stage_histograms("Momentum");
            book_cut_stage_histograms("VertexZ");
            book_cut_stage_histograms("SamplingFraction");
            book_cut_stage_histograms("PCALFiducial");
            book_cut_stage_histograms("DCChi2");
            Log::info(name(), "Cut-stage histograms enabled");
        }
    }

    auto setup_systematics() -> void {
        // Energy scale variations (±1 sigma)
        m_sys_mgr.add_variation("EnergyScale_Up", Framework::VariationType::EnergyScale, +1.0);
        m_sys_mgr.add_variation("EnergyScale_Down", Framework::VariationType::EnergyScale, -1.0);

        // Momentum scale variations (±1 sigma)
        m_sys_mgr.add_variation("MomentumScale_Up", Framework::VariationType::MomentumScale, +1.0);
        m_sys_mgr.add_variation("MomentumScale_Down", Framework::VariationType::MomentumScale, -1.0);

        // Resolution variations
        m_sys_mgr.add_variation("EnergyResolution_Up", Framework::VariationType::EnergyResolution, +1.0);
        m_sys_mgr.add_variation("EnergyResolution_Down", Framework::VariationType::EnergyResolution, -1.0);

        if (Log::is_available()) {
            Log::debug(name(), "Systematic variations configured:");
            for (const auto& var : m_sys_mgr.variations()) {
                Log::debug(name(), "  - {}", var.name);
            }
        }
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

    /**
     * @brief Books histograms for a specific cut stage
     * Must be called during initialize() to ensure thread safety
     */
    auto book_cut_stage_histograms(const std::string& cut_name) -> void {
        auto& cut_hists = m_cut_stage_histograms[cut_name];
        std::string base = fmt::format("/Electron/CutStages/{}", cut_name);

        cut_hists.momentum = m_hist_svc->book_hist1D(
            base + "/p", fmt::format("Electron p after {};p [GeV/c];Events", cut_name),
            120, 0, 12);
        cut_hists.theta = m_hist_svc->book_hist1D(
            base + "/theta", fmt::format("Electron #theta after {};#theta [deg];Events", cut_name),
            180, 0, 180);
        cut_hists.phi = m_hist_svc->book_hist1D(
            base + "/phi", fmt::format("Electron #phi after {};#phi [deg];Events", cut_name),
            180, -180, 180);
        cut_hists.vz = m_hist_svc->book_hist1D(
            base + "/vz", fmt::format("Electron v_{{z}} after {};v_{{z}} [cm];Events", cut_name),
            200, -20, 20);
        cut_hists.sampling_fraction = m_hist_svc->book_hist2D(
            base + "/sf_vs_p", fmt::format("SF vs p after {};p [GeV/c];E/p", cut_name),
            120, 0, 12, 100, 0, 0.5);
        cut_hists.dc_edge_r1 = m_hist_svc->book_hist1D(
            base + "/dc_edge_r1", fmt::format("DC Edge R1 after {};edge [cm];Events", cut_name),
            100, 0, 50);

        cut_hists.booked = true;
    }

    /**
     * @brief Fills histograms after each cut stage to monitor cut effects
     *
     * This function fills a set of histograms for each cut stage, allowing
     * visualization of how each cut affects the electron distributions.
     * Histograms must be pre-booked during initialize() via book_cut_stage_histograms().
     *
     * @param cut_name Name of the cut that was just passed
     * @param electron The electron particle passing the cut
     * @param calo Calorimeter bank data for the electron
     * @param traj DC trajectory bank data for the electron
     * @param track Track bank data for the electron
     */
    auto fill_after_cut(const std::string& cut_name,
                        const CLAS::Particle& electron,
                        const CLAS::CalorimeterBank& calo,
                        const CLAS::DCTrajBank& traj,
                        [[maybe_unused]] const CLAS::TrackBank& track) -> void {
        // Get pre-booked histograms for this cut stage
        auto& cut_hists = m_cut_stage_histograms.at(cut_name);

        // Fill histograms
        cut_hists.momentum->Fill(electron.p());
        cut_hists.theta->Fill(electron.theta());
        cut_hists.phi->Fill(electron.phi());
        cut_hists.vz->Fill(electron.vz());

        const double E_tot = calo.pcal.energy + calo.inner.energy + calo.outer.energy;
        const double SF = E_tot / electron.p();
        cut_hists.sampling_fraction->Fill(electron.p(), SF);
        cut_hists.dc_edge_r1->Fill(traj.region1.edge);
    }

    /**
     * @brief Books histograms for a track category with per-sector granularity
     */
    auto book_track_histograms(TrackHistograms& h, const std::string& category) -> void {
        std::string base = fmt::format("/Tracks/{}", category);

        // Book histograms for each sector (1-6)
        for (int sector = 1; sector <= 6; ++sector) {
            std::string sec_suffix = fmt::format("_S{}", sector);

            h.nphe[sector] = m_hist_svc->book_hist1D(
                base + "/nphe" + sec_suffix,
                fmt::format("Cherenkov nphe (Sector {});nphe;Tracks", sector),
                51, 0, 50);

            h.EPCal_vs_ECal[sector] = m_hist_svc->book_hist2D(
                base + "/EPCal_vs_ECal" + sec_suffix,
                fmt::format("E_{{PCal}} vs E_{{Cal}} (Sector {});E_{{PCal}} [GeV];E_{{Cal}} [GeV]", sector),
                200, 0, 0.6, 200, 0, 0.6);

            h.EPCal[sector] = m_hist_svc->book_hist1D(
                base + "/EPCal" + sec_suffix,
                fmt::format("E_{{PCal}} (Sector {});E_{{PCal}} [GeV];Tracks", sector),
                200, 0, 1);

            h.EPCal_vs_ECal_cut[sector] = m_hist_svc->book_hist2D(
                base + "/EPCal_vs_ECal_cut" + sec_suffix,
                fmt::format("E_{{PCal}} vs E_{{Cal}} (cut, Sector {});E_{{PCal}} [GeV];E_{{Cal}} [GeV]", sector),
                200, 0, 0.6, 200, 0, 0.6);

            h.EPCal_cut[sector] = m_hist_svc->book_hist1D(
                base + "/EPCal_cut" + sec_suffix,
                fmt::format("E_{{PCal}} (cut, Sector {});E_{{PCal}} [GeV];Tracks", sector),
                200, 0, 1);

            h.Etot_vs_SF[sector] = m_hist_svc->book_hist2D(
                base + "/Etot_vs_SF" + sec_suffix,
                fmt::format("E_{{tot}} vs SF (Sector {});E_{{tot}} [GeV];E/p", sector),
                200, 0, 2, 200, 0, 0.5);

            h.Etot_vs_SF_cut[sector] = m_hist_svc->book_hist2D(
                base + "/Etot_vs_SF_cut" + sec_suffix,
                fmt::format("E_{{tot}} vs SF (cut, Sector {});E_{{tot}} [GeV];E/p", sector),
                200, 0, 2, 200, 0, 0.5);
            
            h.p_vs_SF[sector] = m_hist_svc->book_hist2D(
                base + "/p_vs_SF" + sec_suffix,
                fmt::format("p vs SF (Sector {});p [GeV];E/p", sector),
                200, 0, 11, 200, 0, 0.4);

            h.edge_vs_theta_r1[sector] = m_hist_svc->book_hist2D(
                base + "/edge_vs_theta_r1" + sec_suffix,
                fmt::format("Edge vs #theta (R1, Sector {});edge [cm];#theta [deg]", sector),
                200, 0, 50, 200, 0, 40);

            h.edge_vs_theta_r2[sector] = m_hist_svc->book_hist2D(
                base + "/edge_vs_theta_r2" + sec_suffix,
                fmt::format("Edge vs #theta (R2, Sector {});edge [cm];#theta [deg]", sector),
                200, 0, 50, 200, 0, 40);

            h.edge_vs_theta_r3[sector] = m_hist_svc->book_hist2D(
                base + "/edge_vs_theta_r3" + sec_suffix,
                fmt::format("Edge vs #theta (R3, Sector {});edge [cm];#theta [deg]", sector),
                200, 0, 50, 200, 0, 40);

            h.edge_vs_chi2ndf_r1[sector] = m_hist_svc->book_hist2D(
                base + "/edge_vs_chi2ndf_r1" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R1, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.edge_vs_chi2ndf_r2[sector] = m_hist_svc->book_hist2D(
                base + "/edge_vs_chi2ndf_r2" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R2, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.edge_vs_chi2ndf_r3[sector] = m_hist_svc->book_hist2D(
                base + "/edge_vs_chi2ndf_r3" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R3, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.lv_vs_SF[sector] = m_hist_svc->book_hist2D(
                base + "/lv_vs_SF" + sec_suffix,
                fmt::format("lv vs SF (Sector {});lv;E/p", sector),
                100, 0, 70, 200, 0, 0.5);
            h.lw_vs_SF[sector] = m_hist_svc->book_hist2D(
                base + "/lw_vs_SF" + sec_suffix,
                fmt::format("lw vs SF (Sector {});lw;E/p", sector),
                100, 0, 70, 200, 0, 0.5);
            h.lu_vs_SF_low[sector] = m_hist_svc->book_hist2D(
                base + "/lu_vs_SF_low" + sec_suffix,
                fmt::format("lu vs SF (low, Sector {});lu;E/p", sector),
                100, 0, 170, 200, 0, 0.5);
            h.lu_vs_SF_high[sector] = m_hist_svc->book_hist2D(
                base + "/lu_vs_SF_high" + sec_suffix,
                fmt::format("lu vs SF (high, Sector {});lu;E/p", sector),
                100, 350, 450, 200, 0, 0.5);
            
        }

        h.booked = true;
    }

    /**
     * @brief Process tracks and fill per-sector histograms
     *
     * For each track, reads calorimeter, Cherenkov, scintillator, and DC trajectory
     * information and fills the corresponding per-sector histograms.
     */
    auto process_tracks(const std::vector<CLAS::Particle>& tracks, TrackHistograms& histograms,
                        hipo::BankView& REC_Calorimeter, hipo::BankView& REC_Cherenkov,
                        hipo::BankView& REC_Scintillator, hipo::BankView& REC_Traj,
                        hipo::BankView& REC_Track, [[maybe_unused]] double event_start_time) -> void {
        for (const auto& track : tracks) {
            CLAS::CalorimeterBank calo = CLAS::read_Calorimeter_bank(REC_Calorimeter, track.index());
            CLAS::CherenkovBank cherenkov = CLAS::read_Cherenkov_bank(REC_Cherenkov, track.index());
            [[maybe_unused]] CLAS::ScintillatorBank scint = CLAS::read_Scintillator_bank(REC_Scintillator, track.index());
            CLAS::DCTrajBank traj = CLAS::read_Traj_bank(REC_Traj, track.index());
            CLAS::TrackBank trk_r1 = CLAS::read_Track_bank(REC_Track, traj.region1.index);
            CLAS::TrackBank trk_r2 = CLAS::read_Track_bank(REC_Track, traj.region2.index);
            CLAS::TrackBank trk_r3 = CLAS::read_Track_bank(REC_Track, traj.region3.index);

            const int sector = calo.pcal.sector;
            if (sector < 1 || sector > 6) continue;

            // Calorimeter quantities
            const double E_tot = calo.pcal.energy + calo.inner.energy + calo.outer.energy;
            const double SF = E_tot / track.p();
            const double E_cal = calo.inner.energy + calo.outer.energy;
            const double E_pcal = calo.pcal.energy;
            const int nphe = cherenkov.nphe;

            if (E_cal == 0 || E_pcal == 0) continue;

            // Fill calorimeter histograms for this sector
            histograms.nphe[sector]->Fill(nphe);
            histograms.EPCal_vs_ECal[sector]->Fill(E_pcal, E_cal);
            histograms.EPCal[sector]->Fill(E_pcal);
            histograms.Etot_vs_SF[sector]->Fill(E_tot, SF);
            histograms.p_vs_SF[sector]->Fill(track.p(), SF);

            // Apply EB-like cuts for cut histograms
            constexpr double EB_MIN_CHERENKOV_NPHE = 2.0;
            constexpr double EB_MIN_PCAL_ENERGY = 0.06;
            const bool passes_nphe = nphe > EB_MIN_CHERENKOV_NPHE;
            const bool passes_pcal = E_pcal > EB_MIN_PCAL_ENERGY;

            if (passes_nphe) {
                histograms.EPCal_vs_ECal_cut[sector]->Fill(E_pcal, E_cal);
                histograms.EPCal_cut[sector]->Fill(E_pcal);
            }
            if (passes_pcal && passes_nphe) {
                histograms.Etot_vs_SF_cut[sector]->Fill(E_tot, SF);
            }

            // DC fiducial: edge vs theta for each region
            auto compute_local_theta = [](double x, double y, double z, int sec) {
                constexpr double SECTOR_ANGLE = M_PI / 3.0;
                double angle = (sec - 1) * SECTOR_ANGLE;
                double s = std::sin(angle);
                double c = std::cos(angle);
                return std::atan2(std::hypot(c * x - s * y, s * x + c * y), z) * 180.0 / M_PI;
            };

            const double theta_r1 = compute_local_theta(traj.region1.x, traj.region1.y, traj.region1.z, trk_r1.sector);
            const double theta_r2 = compute_local_theta(traj.region2.x, traj.region2.y, traj.region2.z, trk_r2.sector);
            const double theta_r3 = compute_local_theta(traj.region3.x, traj.region3.y, traj.region3.z, trk_r3.sector);

            histograms.edge_vs_theta_r1[sector]->Fill(traj.region1.edge, theta_r1);
            histograms.edge_vs_theta_r2[sector]->Fill(traj.region2.edge, theta_r2);
            histograms.edge_vs_theta_r3[sector]->Fill(traj.region3.edge, theta_r3);

            // Edge vs chi2/ndf
            const double chi2ndf_r1 = trk_r1.ndf > 0 ? trk_r1.chi2 / trk_r1.ndf : std::numeric_limits<double>::quiet_NaN();
            const double chi2ndf_r2 = trk_r2.ndf > 0 ? trk_r2.chi2 / trk_r2.ndf : std::numeric_limits<double>::quiet_NaN();
            const double chi2ndf_r3 = trk_r3.ndf > 0 ? trk_r3.chi2 / trk_r3.ndf : std::numeric_limits<double>::quiet_NaN();

            histograms.edge_vs_chi2ndf_r1[sector]->Fill(traj.region1.edge, chi2ndf_r1);
            histograms.edge_vs_chi2ndf_r2[sector]->Fill(traj.region2.edge, chi2ndf_r2);
            histograms.edge_vs_chi2ndf_r3[sector]->Fill(traj.region3.edge, chi2ndf_r3);

            // SF vs lv,w
            histograms.lv_vs_SF[sector]->Fill(calo.pcal.lv, SF);
            histograms.lw_vs_SF[sector]->Fill(calo.pcal.lw, SF);

            // Low end and high end of lu
            histograms.lu_vs_SF_low[sector]->Fill(calo.pcal.lu, SF);
            histograms.lu_vs_SF_high[sector]->Fill(calo.pcal.lu, SF);
        }
    }

    auto fill_final_histograms(const CLAS::Particle& electron, const CLAS::CalorimeterBank& calo, const CLAS::DCTrajBank& traj, const CLAS::TrackBank& track) -> void {
        // Fill electron kinematics
        h_electron.momentum->Fill(electron.p());
        h_electron.theta->Fill(electron.theta());
        h_electron.phi->Fill(electron.phi());
        h_electron.vz->Fill(electron.vz());
        h_electron.chi2pid->Fill(electron.chi2pid());
        h_electron.p_vs_theta->Fill(electron.theta(), electron.p());
        h_electron.phi_vs_theta->Fill(electron.theta(), electron.phi());

        // Fill calorimeter - PCAL
        h_calorimeter.lu_vs_lv_pcal->Fill(calo.pcal.lv, calo.pcal.lu);
        h_calorimeter.lu_vs_lw_pcal->Fill(calo.pcal.lw, calo.pcal.lu);

        // Fill calorimeter - Inner EC
        h_calorimeter.lu_vs_lv_inner->Fill(calo.inner.lv, calo.inner.lu);
        h_calorimeter.lu_vs_lw_inner->Fill(calo.inner.lw, calo.inner.lu);

        // Fill calorimeter - Outer EC
        h_calorimeter.lu_vs_lv_outer->Fill(calo.outer.lv, calo.outer.lu);
        h_calorimeter.lu_vs_lw_outer->Fill(calo.outer.lw, calo.outer.lu);

        // Fill DC histograms by momentum bin
        fill_dc_momentum_binned(electron.p(), traj, track);
        // Fill DC histograms by theta bin
        fill_dc_theta_binned(electron.theta(), traj, track);
    }

    /// Fill DC histograms in the appropriate momentum bin 
    auto fill_dc_momentum_binned(double p, const CLAS::DCTrajBank& traj, const CLAS::TrackBank& track) -> void {
        // Compute chi2/ndf safely
        const double chi2ndf = (track.ndf > 0) ? track.chi2 / track.ndf : 0.0;

        // Determine momentum bin using configurable range
        const int bin_idx = static_cast<int>((p - m_config.p_bin_min) / m_config.p_bin_width);
        const int p_low = static_cast<int>(m_config.p_bin_min) + bin_idx;
        const int p_high = p_low + static_cast<int>(m_config.p_bin_width);

        // Check bounds
        if (p < m_config.p_bin_min || p >= m_config.p_bin_max) return;

        const std::string bin_key = fmt::format("{}_{}", p_low, p_high);

        // Fill histogram if it exists for this bin
        if (auto it = h_dc.chi2ndf_vs_edge_r1.find(bin_key); it != h_dc.chi2ndf_vs_edge_r1.end()) {
            it->second->Fill(traj.region1.edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r2.find(bin_key); it != h_dc.chi2ndf_vs_edge_r2.end()) {
            it->second->Fill(traj.region2.edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r3.find(bin_key); it != h_dc.chi2ndf_vs_edge_r3.end()) {
            it->second->Fill(traj.region3.edge, chi2ndf);
        }
    }

    auto fill_dc_theta_binned(double theta, const CLAS::DCTrajBank& traj, const CLAS::TrackBank& track) -> void {
        // Compute chi2/ndf safely
        const double chi2ndf = (track.ndf > 0) ? track.chi2 / track.ndf : 0.0;

        // Determine theta bin using configurable range
        const int bin_idx = static_cast<int>((theta - m_config.theta_bin_min) / m_config.theta_bin_width);
        const int theta_low = static_cast<int>(m_config.theta_bin_min) + bin_idx;
        const int theta_high = theta_low + static_cast<int>(m_config.theta_bin_width);

        // Check bounds
        if (theta < m_config.theta_bin_min || theta >= m_config.theta_bin_max) return;

        const std::string bin_key = fmt::format("{}_{}", theta_low, theta_high);

        // Fill histogram if it exists for this bin
        if (auto it = h_dc.chi2ndf_vs_edge_r1.find(bin_key); it != h_dc.chi2ndf_vs_edge_r1.end()) {
            it->second->Fill(traj.region1.edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r2.find(bin_key); it != h_dc.chi2ndf_vs_edge_r2.end()) {
            it->second->Fill(traj.region2.edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r3.find(bin_key); it != h_dc.chi2ndf_vs_edge_r3.end()) {
            it->second->Fill(traj.region3.edge, chi2ndf);
        }
};

}  // namespace Algorithms