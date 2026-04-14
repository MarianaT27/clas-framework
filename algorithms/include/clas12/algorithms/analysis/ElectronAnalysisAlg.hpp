#pragma once

#include <array>
#include <vector>
#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/Pipeline.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/ConditionsService.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/physics/Kinematics.hpp>

namespace clas12::algorithms {

// Import Log singleton for convenient logging
using clas12::framework::Log;

/**
 * @brief Full electron analysis algorithm (refactored from study1::Reader)
 *
 * This demonstrates a fully service-based electron analysis using the framework:
 * - ElectronCutsService for all electron selection criteria
 * - SamplingFractionService for energy-momentum correlation cuts
 * - HistogramService for histogram management
 * - Cutflow for automatic efficiency tracking
 * - Per-sector track analysis histograms
 * - DC fiducial studies (momentum and angular binning)
 * - Cut-stage monitoring for optimization
 * - Clean separation of concerns with no direct cut implementations
 */
/// Configuration structure for ElectronAnalysisAlg parameters
struct ElectronAnalysisConfig {
    clas12::Target target = clas12::Target::LD2;
    clas12::Polarity polarity = clas12::Polarity::Outbending;
    bool enable_ntuple = true;
    bool enable_cut_stage_histograms = true;
    // Momentum binning for DC histograms
    double p_bin_min = 1.0;
    double p_bin_max = 10.0;
    double p_bin_width = 1.0;
    // Theta binning for DC histograms
    double theta_bin_min = 5.0;
    double theta_bin_max = 40.0;
    double theta_bin_width = 5.0;
};

class ElectronAnalysisAlg : public clas12::framework::SimpleAlgorithm {
   public:
    using Config = ElectronAnalysisConfig;

    explicit ElectronAnalysisAlg(std::string name = "ElectronAnalysis", const Config& config = Config{})
        : SimpleAlgorithm(std::move(name)),
          m_config(config),
          m_target(m_config.target),
          m_polarity(m_config.polarity),
          m_cutflow("Electron Selection") {
        declare_property(m_enable_chi2pid_cut);
        declare_property(m_enable_momentum_cut);
        declare_property(m_enable_vz_cut);
        declare_property(m_enable_sf_cut);
        declare_property(m_enable_pcal_fiducial_cut);
        declare_property(m_enable_dc_chi2ndf_cut);

        declare_bank("REC::Event");
        declare_bank("REC::Particle");
        declare_bank("REC::Calorimeter");
        declare_bank("REC::Cherenkov");
        declare_bank("REC::Scintillator");
        declare_bank("REC::Traj");
        declare_bank("REC::Track");
        declare_optional_bank("FMT::Tracks");
    }


    /// Access configuration (const)
    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_log = svc<clas12::framework::LoggingService>();
        m_hist = svc<clas12::framework::HistogramService>();
        m_ntuple = svc<clas12::framework::NTupleService>();
        m_particle = svc<clas12::framework::ParticleSelectorService>();
        m_sf = svc<clas12::framework::SamplingFractionService>();
        m_electron_cuts = svc<clas12::framework::ElectronCutsService>();

        if (Log::is_available()) {
            Log::info(name(), "Initializing algorithm");
            Log::info(name(), "Target: {}, Polarity: {}",
                      m_target == clas12::Target::LD2 ? "LD2" : "Other",
                      m_polarity == clas12::Polarity::Outbending ? "Outbending" : "Inbending");
        }

        if (!m_hist) {
            Log::error(name(), "Could not get HistogramService");
            return clas12::framework::StatusCode::FAILURE();
        }

        if (!m_sf) {
            Log::error(name(), "Could not get SamplingFractionService");
            return clas12::framework::StatusCode::FAILURE();
        }

        if (!m_electron_cuts) {
            Log::error(name(), "Could not get ElectronCutsService");
            return clas12::framework::StatusCode::FAILURE();
        }

        // Configure services with target and polarity
        m_electron_cuts->set_target(m_target);
        m_electron_cuts->set_polarity(m_polarity);

        // Connect the sampling fraction service to the electron cuts service
        m_electron_cuts->set_sf_service(m_sf);

        setup_cutflow();
        book_histograms();

        if (m_ntuple && m_config.enable_ntuple) {
            setup_ntuple();
        } else if (!m_ntuple) {
            Log::warning(name(), "NTupleService not available - N-tuple output disabled");
        } else {
            Log::info(name(), "N-tuple output disabled by configuration");
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        using namespace clas12::framework;

        // Bind REC::Event up front for the per-event start time.
        clas12::bankdefs::REC_Event rec_event;
        rec_event.bind(event.get_bank("REC::Event"));
        const auto& REC_Particle = event.get_bank("REC::Particle");
        const bool have_event  = rec_event.rows() > 0;
        const double event_start_time = have_event ? rec_event.startTime(0) : 0.0;

        // Particle extraction (may be empty; pipeline filters gate the rest).
        ParticleMap particles;
        std::vector<clas12::Particle> neg_tracks;
        if (have_event) {
            particles = m_particle->extract_particles(REC_Particle, {clas12::PIDs::ELECTRON});
            auto charged_tracks = m_particle->get_particles_by_charge(REC_Particle);
            neg_tracks = std::move(charged_tracks[-1]);
        }

        static const std::vector<clas12::Particle> kEmptyParticles;
        const auto& electrons = have_event
            ? m_particle->get_electrons(particles)
            : kEmptyParticles;
        auto trigger_e = (!electrons.empty())
            ? clas12::physics::ParticleUtils::find_trigger_electron(electrons, {clas12::FD, clas12::CD})
            : std::optional<clas12::Particle>{};

        // Build the candidate only when we have a trigger electron.
        ElectronCandidate candidate{};
        if (trigger_e.has_value()) {
            candidate = build_candidate(*trigger_e, event);
        }

        // Cut decisions (preserving the old "disabled => pass" semantics).
        const bool pass_chi2 = trigger_e && (!m_enable_chi2pid_cut.get()
            || m_electron_cuts->pass_chi2pid_cut(candidate.particle.chi2pid()));
        const bool pass_mom  = trigger_e && (!m_enable_momentum_cut.get()
            || m_electron_cuts->pass_momentum_cut(candidate.particle.p()));
        const bool pass_vz   = trigger_e && (!m_enable_vz_cut.get()
            || m_electron_cuts->pass_vz_cut(candidate.particle.vz()));
        const bool pass_sf   = trigger_e && (!m_enable_sf_cut.get()
            || m_sf->passes_cut(candidate.sampling_fraction,
                                candidate.particle.p(),
                                candidate.sector, m_polarity));
        const bool pass_pcal = trigger_e && (!m_enable_pcal_fiducial_cut.get()
            || m_electron_cuts->pass_pcal_fiducial_cut(
                   candidate.calorimeter.pcal_lv,
                   candidate.calorimeter.pcal_lw));
        const bool pass_dc   = trigger_e && (!m_enable_dc_chi2ndf_cut.get()
            || m_electron_cuts->pass_dc_edge_cut(
                   candidate.trajectory.region1_edge,
                   candidate.trajectory.region2_edge,
                   candidate.trajectory.region3_edge));

        return pipeline(event, m_cutflow)
            .count("All Events")
            // Fill negative-track histograms regardless of whether a
            // trigger electron is later found, matching the original
            // ordering.
            .tap([&] {
                if (have_event) {
                    process_tracks(neg_tracks, h_neg_tracks, event, event_start_time);
                }
            })
            .filter("has event data", have_event)
            .filter("has electrons",  !electrons.empty())
            .filter("Trigger Electron", trigger_e.has_value())
            .tap([&] {
                process_tracks({candidate.particle}, h_electron_tracks, event, event_start_time);
            })
            .filter("Chi2 PID",             pass_chi2)
            .tap   ([&] { fill_after_cut("Chi2PID",
                                         candidate.particle, candidate.calorimeter,
                                         candidate.trajectory, candidate.track); })
            .filter("Momentum > 0.8 GeV",   pass_mom)
            .tap   ([&] { fill_after_cut("Momentum",
                                         candidate.particle, candidate.calorimeter,
                                         candidate.trajectory, candidate.track); })
            .filter("Vertex Z",             pass_vz)
            .tap   ([&] { fill_after_cut("VertexZ",
                                         candidate.particle, candidate.calorimeter,
                                         candidate.trajectory, candidate.track); })
            .filter("Sampling Fraction",    pass_sf)
            .tap   ([&] { fill_after_cut("SamplingFraction",
                                         candidate.particle, candidate.calorimeter,
                                         candidate.trajectory, candidate.track); })
            .filter("PCAL Fiducial",        pass_pcal)
            .tap   ([&] { fill_after_cut("PCALFiducial",
                                         candidate.particle, candidate.calorimeter,
                                         candidate.trajectory, candidate.track); })
            .filter("DC Chi2/NDF",          pass_dc)
            .tap   ([&] { fill_after_cut("DCChi2",
                                         candidate.particle, candidate.calorimeter,
                                         candidate.trajectory, candidate.track); })
            .tap([&] {
                fill_final_histograms(event, candidate.particle, candidate.calorimeter,
                                      candidate.trajectory, candidate.track);
                if (m_ntuple && m_config.enable_ntuple) {
                    m_nt_e_p               = candidate.particle.p();
                    m_nt_e_theta           = candidate.particle.theta();
                    m_nt_e_phi             = candidate.particle.phi();
                    m_nt_e_vz              = candidate.particle.vz();
                    m_nt_e_chi2pid         = candidate.particle.chi2pid();
                    m_nt_sampling_fraction = candidate.sampling_fraction;
                    m_nt_sector            = candidate.sector;
                    m_ntuple->fill("electron_analysis");
                }
            })
            .set_decision(Decisions::HasGoodElectron, true)
            .status();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        if (Log::is_available()) {
            Log::info(name(), "Finalizing algorithm");
            auto final_count = m_cutflow.get_count("DC Chi2/NDF");
            Log::info(name(), "Events passing all cuts: {}", final_count);

            if (m_ntuple && m_config.enable_ntuple) {
                auto entries = m_ntuple->get_entries("electron_analysis");
                Log::info(name(), "N-tuple has {} entries", entries);
            }
        }

        fmt::print("\n");
        m_cutflow.print();

        return clas12::framework::StatusCode::SUCCESS();
    }

    /// Structure to hold electron candidate with all associated detector info
    struct ElectronCandidate {
        clas12::Particle particle;
        clas12::CalorimeterBank calorimeter;
        clas12::DCTrajBank trajectory;
        clas12::TrackBank track;
        double sampling_fraction = 0.0;
        double total_energy = 0.0;
        int sector = 0;

        [[nodiscard]] auto is_valid() const -> bool {
            return sector >= 1 && sector <= 6 && particle.p() > 0;
        }
    };

    /// Build electron candidate with all detector information. Every
    /// read_*_bank call below reuses the per-event generated-wrapper
    /// cache owned by Event, so binding and linker construction happen
    /// exactly once per event, not per call.
    [[nodiscard]] auto build_candidate(const clas12::Particle& electron,
                                       clas12::framework::Event& event) const -> ElectronCandidate {
        ElectronCandidate candidate;
        candidate.particle    = electron;
        candidate.calorimeter = clas12::read_Calorimeter_bank(event, electron.index());
        candidate.trajectory  = clas12::read_Traj_bank(event, electron.index());
        candidate.track       = clas12::read_Track_bank(event, electron.index());
        candidate.total_energy = candidate.calorimeter.pcal_energy +
                                 candidate.calorimeter.inner_energy +
                                 candidate.calorimeter.outer_energy;
        candidate.sampling_fraction = candidate.total_energy / electron.p();
        candidate.sector = candidate.calorimeter.pcal_sector;
        return candidate;
    }

    /// Apply all electron selection cuts and return result
    /// Returns true if electron passes all cuts, fills cutflow along the way
    [[nodiscard]] auto apply_electron_cuts(const ElectronCandidate& candidate) -> bool {
        const auto& e = candidate.particle;
        const auto& calo = candidate.calorimeter;
        const auto& traj = candidate.trajectory;

        // Chi2 PID cut
        if (m_enable_chi2pid_cut.get() && !m_electron_cuts->pass_chi2pid_cut(e.chi2pid())) return false;
        m_cutflow.pass("Chi2 PID");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("Chi2PID", e, calo, traj, candidate.track);
        }

        // Momentum cut
        if (m_enable_momentum_cut.get() && !m_electron_cuts->pass_momentum_cut(e.p())) return false;
        m_cutflow.pass("Momentum > 0.8 GeV");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("Momentum", e, calo, traj, candidate.track);
        }

        // Vertex Z cut
        if (m_enable_vz_cut.get() && !m_electron_cuts->pass_vz_cut(e.vz())) return false;
        m_cutflow.pass("Vertex Z");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("VertexZ", e, calo, traj, candidate.track);
        }

        // Sampling fraction cut
        if (m_enable_sf_cut.get() && !m_sf->passes_cut(candidate.sampling_fraction, e.p(), candidate.sector, m_polarity)) return false;
        m_cutflow.pass("Sampling Fraction");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("SamplingFraction", e, calo, traj, candidate.track);
        }

        // PCAL fiducial cut
        if (m_enable_pcal_fiducial_cut.get() && !m_electron_cuts->pass_pcal_fiducial_cut(calo.pcal_lv, calo.pcal_lw)) return false;
        m_cutflow.pass("PCAL Fiducial");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("PCALFiducial", e, calo, traj, candidate.track);
        }

        // DC edge cut
        if (m_enable_dc_chi2ndf_cut.get() && !m_electron_cuts->pass_dc_edge_cut(traj.region1_edge, traj.region2_edge, traj.region3_edge)) return false;
        m_cutflow.pass("DC Chi2/NDF");
        if (m_config.enable_cut_stage_histograms) {
            fill_after_cut("DCChi2", e, calo, traj, candidate.track);
        }

        return true;
    }

   private:
    Config m_config;
    clas12::Target m_target;
    clas12::Polarity m_polarity;
    clas12::framework::Cutflow m_cutflow;

    // Cut toggle properties
    clas12::framework::Property<bool> m_enable_chi2pid_cut{"enable_chi2pid_cut", true, "Enable Chi2 PID cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
    clas12::framework::Property<bool> m_enable_sf_cut{"enable_sf_cut", true, "Enable sampling fraction cut"};
    clas12::framework::Property<bool> m_enable_pcal_fiducial_cut{"enable_pcal_fiducial_cut", true, "Enable PCAL fiducial cut"};
    clas12::framework::Property<bool> m_enable_dc_chi2ndf_cut{"enable_dc_chi2ndf_cut", true, "Enable DC Chi2/NDF cut"};

    // Services (cached pointers from svc<T>())
    clas12::framework::LoggingService* m_log = nullptr;
    clas12::framework::HistogramService* m_hist = nullptr;
    clas12::framework::NTupleService* m_ntuple = nullptr;
    clas12::framework::ParticleSelectorService* m_particle = nullptr;
    clas12::framework::SamplingFractionService* m_sf = nullptr;
    clas12::framework::ElectronCutsService* m_electron_cuts = nullptr;

    // N-tuple variables
    double m_nt_e_p = 0.0;
    double m_nt_e_theta = 0.0;
    double m_nt_e_phi = 0.0;
    double m_nt_e_vz = 0.0;
    double m_nt_e_chi2pid = 0.0;
    double m_nt_sampling_fraction = 0.0;
    int m_nt_sector = 0;

    // Histograms - organized by category (lock-free atomic)
    struct {
        clas12::framework::ThreadLocalHistogram1D* momentum = nullptr;
        clas12::framework::ThreadLocalHistogram1D* theta = nullptr;
        clas12::framework::ThreadLocalHistogram1D* phi = nullptr;
        clas12::framework::ThreadLocalHistogram1D* vz = nullptr;
        clas12::framework::ThreadLocalHistogram1D* chi2pid = nullptr;
        clas12::framework::ThreadLocalHistogram2D* p_vs_theta = nullptr;
        clas12::framework::ThreadLocalHistogram2D* phi_vs_theta = nullptr;
    } h_electron;

    struct {
        clas12::framework::ThreadLocalHistogram2D* lu_vs_lv_pcal = nullptr;
        clas12::framework::ThreadLocalHistogram2D* lu_vs_lw_pcal = nullptr;
        clas12::framework::ThreadLocalHistogram2D* lu_vs_lv_inner = nullptr;
        clas12::framework::ThreadLocalHistogram2D* lu_vs_lw_inner = nullptr;
        clas12::framework::ThreadLocalHistogram2D* lu_vs_lv_outer = nullptr;
        clas12::framework::ThreadLocalHistogram2D* lu_vs_lw_outer = nullptr;
    } h_calorimeter;

    struct {
        std::map<std::string, clas12::framework::ThreadLocalHistogram2D*> chi2ndf_vs_edge_r1;
        std::map<std::string, clas12::framework::ThreadLocalHistogram2D*> chi2ndf_vs_edge_r2;
        std::map<std::string, clas12::framework::ThreadLocalHistogram2D*> chi2ndf_vs_edge_r3;
    } h_dc;

    static constexpr int N_VZ_P_BINS = 20;
    static constexpr std::array<double, N_VZ_P_BINS + 1> vz_p_bin_edges = {
        0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0,
        5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0
    };

    static constexpr int N_SECTORS = 6;

    struct VzHistograms {
        clas12::framework::ThreadLocalHistogram2D* vz_vs_theta = nullptr;
        clas12::framework::ThreadLocalHistogram2D* vz_vs_p = nullptr;
        std::array<clas12::framework::ThreadLocalHistogram2D*, N_VZ_P_BINS> vz_vs_theta_pbin = {};
        std::array<std::array<clas12::framework::ThreadLocalHistogram2D*, N_VZ_P_BINS>, N_SECTORS> vz_vs_theta_pbin_sector = {};
    };

    VzHistograms h_electron_vz;
    VzHistograms h_electron_vz_fmt;

    // Track histograms structure (for neg/pos tracks, pions, etc.)
    // Each histogram is stored per sector (1-6) - using lock-free atomic histograms
    struct TrackHistograms {
        bool booked = false;
        // Calorimeter (per sector)
        std::map<int, clas12::framework::ThreadLocalHistogram1D*> nphe;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> EPCal_vs_ECal;
        std::map<int, clas12::framework::ThreadLocalHistogram1D*> EPCal;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> EPCal_vs_ECal_cut;
        std::map<int, clas12::framework::ThreadLocalHistogram1D*> EPCal_cut;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> Etot_vs_SF;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> Etot_vs_SF_cut;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> p_vs_SF;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> lv_vs_SF;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> lw_vs_SF;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> lu_vs_SF_low;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> lu_vs_SF_high;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> lu_vs_SF_low_with_cut;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> lu_vs_SF_high_with_cut;
        // DC fiducial (per sector)
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> edge_vs_theta_r1;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> edge_vs_theta_r2;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> edge_vs_theta_r3;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> edge_vs_chi2ndf_r1;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> edge_vs_chi2ndf_r2;
        std::map<int, clas12::framework::ThreadLocalHistogram2D*> edge_vs_chi2ndf_r3;
    };

    TrackHistograms h_neg_tracks;
    TrackHistograms h_electron_tracks;

    // Histograms for each cut stage (lock-free atomic)
    struct CutStageHistograms {
        bool booked = false;
        clas12::framework::ThreadLocalHistogram1D* momentum = nullptr;
        clas12::framework::ThreadLocalHistogram1D* theta = nullptr;
        clas12::framework::ThreadLocalHistogram1D* phi = nullptr;
        clas12::framework::ThreadLocalHistogram1D* vz = nullptr;
        clas12::framework::ThreadLocalHistogram2D* sampling_fraction = nullptr;
        clas12::framework::ThreadLocalHistogram1D* dc_edge_r1 = nullptr;
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
        // Electron kinematics (lock-free atomic)
        h_electron.momentum = m_hist->book_hist1D(
            "/Electron/momentum", "Electron Momentum;p [GeV/c];Events", 120, 0, 12);
        h_electron.theta = m_hist->book_hist1D(
            "/Electron/theta", "Electron #theta;#theta [deg];Events", 180, 0, 180);
        h_electron.phi = m_hist->book_hist1D(
            "/Electron/phi", "Electron #phi;#phi [deg];Events", 180, -180, 180);
        h_electron.vz = m_hist->book_hist1D(
            "/Electron/vz", "Electron v_{z};v_{z} [cm];Events", 200, -20, 20);
        h_electron.chi2pid = m_hist->book_hist1D(
            "/Electron/chi2pid", "Electron #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);

        h_electron.p_vs_theta = m_hist->book_hist2D(
            "/Electron/p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
            180, 0, 180, 120, 0, 12);
        h_electron.phi_vs_theta = m_hist->book_hist2D(
            "/Electron/phi_vs_theta", "#phi vs #theta;#theta [deg];#phi [deg]",
            180, 0, 180, 180, -180, 180);

        book_vz_histograms(h_electron_vz, "/Electron", "Electron");
        book_vz_histograms(h_electron_vz_fmt, "/Electron/FMT", "Electron (FMT)");

        // Calorimeter - PCAL (lock-free atomic)
        h_calorimeter.lu_vs_lv_pcal = m_hist->book_hist2D(
            "/Calorimeter/lu_vs_lv_pcal", "PCAL lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_pcal = m_hist->book_hist2D(
            "/Calorimeter/lu_vs_lw_pcal", "PCAL lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        // Calorimeter - Inner EC (lock-free atomic)
        h_calorimeter.lu_vs_lv_inner = m_hist->book_hist2D(
            "/Calorimeter/lu_vs_lv_inner", "Inner EC lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_inner = m_hist->book_hist2D(
            "/Calorimeter/lu_vs_lw_inner", "Inner EC lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        // Calorimeter - Outer EC (lock-free atomic)
        h_calorimeter.lu_vs_lv_outer = m_hist->book_hist2D(
            "/Calorimeter/lu_vs_lv_outer", "Outer EC lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_outer = m_hist->book_hist2D(
            "/Calorimeter/lu_vs_lw_outer", "Outer EC lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        // DC Chi2/ndf vs edge for momentum bins (lock-free atomic)
        std::vector<std::string> p_bins = {"1_2", "2_3", "3_4", "4_5", "5_6", "6_7", "7_8", "8_9", "9_10"};
        for (const auto& bin : p_bins) {
            h_dc.chi2ndf_vs_edge_r1[bin] = m_hist->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r1_{}", bin),
                fmt::format("Region 1: p={} GeV;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : p_bins) {
            h_dc.chi2ndf_vs_edge_r2[bin] = m_hist->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r2_{}", bin),
                fmt::format("Region 2: p={} GeV;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : p_bins) {
            h_dc.chi2ndf_vs_edge_r3[bin] = m_hist->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r3_{}", bin),
                fmt::format("Region 3: p={} GeV;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }

        // DC Chi2/ndf vs edge for theta bins (lock-free atomic)
        std::vector<std::string> theta_bins = {"5_10", "10_15", "15_20", "20_25", "25_30", "30_35", "35_40"};
        for (const auto& bin : theta_bins) {
            h_dc.chi2ndf_vs_edge_r1[bin] = m_hist->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r1_theta_{}", bin),
                fmt::format("Region 1: #theta={} deg;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : theta_bins) {
            h_dc.chi2ndf_vs_edge_r2[bin] = m_hist->book_hist2D(
                fmt::format("/DC/chi2ndf_vs_edge_r2_theta_{}", bin),
                fmt::format("Region 2: #theta={} deg;edge;#chi^{{2}}/ndf", bin),
                200, 0, 50, 200, 0, 10);
        }
        for (const auto& bin : theta_bins) {
            h_dc.chi2ndf_vs_edge_r3[bin] = m_hist->book_hist2D(
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

    auto setup_ntuple() -> void {
        // Create and register branches for electron analysis
        m_ntuple->register_branch("electron_analysis", "e_p", &m_nt_e_p);
        m_ntuple->register_branch("electron_analysis", "e_theta", &m_nt_e_theta);
        m_ntuple->register_branch("electron_analysis", "e_phi", &m_nt_e_phi);
        m_ntuple->register_branch("electron_analysis", "e_vz", &m_nt_e_vz);
        m_ntuple->register_branch("electron_analysis", "e_chi2pid", &m_nt_e_chi2pid);
        m_ntuple->register_branch("electron_analysis", "sampling_fraction", &m_nt_sampling_fraction);
        m_ntuple->register_branch("electron_analysis", "sector", &m_nt_sector);

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

        cut_hists.momentum = m_hist->book_hist1D(
            base + "/p", fmt::format("Electron p after {};p [GeV/c];Events", cut_name),
            120, 0, 12);
        cut_hists.theta = m_hist->book_hist1D(
            base + "/theta", fmt::format("Electron #theta after {};#theta [deg];Events", cut_name),
            180, 0, 180);
        cut_hists.phi = m_hist->book_hist1D(
            base + "/phi", fmt::format("Electron #phi after {};#phi [deg];Events", cut_name),
            180, -180, 180);
        cut_hists.vz = m_hist->book_hist1D(
            base + "/vz", fmt::format("Electron v_{{z}} after {};v_{{z}} [cm];Events", cut_name),
            200, -20, 20);
        cut_hists.sampling_fraction = m_hist->book_hist2D(
            base + "/sf_vs_p", fmt::format("SF vs p after {};p [GeV/c];E/p", cut_name),
            120, 0, 12, 100, 0, 0.5);
        cut_hists.dc_edge_r1 = m_hist->book_hist1D(
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
                        const clas12::Particle& electron,
                        const clas12::CalorimeterBank& calo,
                        const clas12::DCTrajBank& traj,
                        [[maybe_unused]] const clas12::TrackBank& track) -> void {
        // Get pre-booked histograms for this cut stage
        auto& cut_hists = m_cut_stage_histograms.at(cut_name);

        // Fill thread-safe histograms
        cut_hists.momentum->fill(electron.p());
        cut_hists.theta->fill(electron.theta());
        cut_hists.phi->fill(electron.phi());
        cut_hists.vz->fill(electron.vz());

        const double E_tot = calo.pcal_energy + calo.inner_energy + calo.outer_energy;
        const double SF = E_tot / electron.p();
        cut_hists.sampling_fraction->fill(electron.p(), SF);
        cut_hists.dc_edge_r1->fill(traj.region1_edge);
    }

    /**
     * @brief Books histograms for a track category with per-sector granularity
     */
    auto book_track_histograms(TrackHistograms& h, const std::string& category) -> void {
        std::string base = fmt::format("/Tracks/{}", category);

        // Book lock-free atomic histograms for each sector (1-6)
        for (int sector = 1; sector <= 6; ++sector) {
            std::string sec_suffix = fmt::format("_S{}", sector);

            h.nphe[sector] = m_hist->book_hist1D(
                base + "/nphe" + sec_suffix,
                fmt::format("Cherenkov nphe (Sector {});nphe;Tracks", sector),
                51, 0, 50);

            h.EPCal_vs_ECal[sector] = m_hist->book_hist2D(
                base + "/EPCal_vs_ECal" + sec_suffix,
                fmt::format("E_{{PCal}} vs E_{{Cal}} (Sector {});E_{{PCal}} [GeV];E_{{Cal}} [GeV]", sector),
                200, 0, 0.6, 200, 0, 0.6);

            h.EPCal[sector] = m_hist->book_hist1D(
                base + "/EPCal" + sec_suffix,
                fmt::format("E_{{PCal}} (Sector {});E_{{PCal}} [GeV];Tracks", sector),
                200, 0, 1);

            h.EPCal_vs_ECal_cut[sector] = m_hist->book_hist2D(
                base + "/EPCal_vs_ECal_cut" + sec_suffix,
                fmt::format("E_{{PCal}} vs E_{{Cal}} (cut, Sector {});E_{{PCal}} [GeV];E_{{Cal}} [GeV]", sector),
                200, 0, 0.6, 200, 0, 0.6);

            h.EPCal_cut[sector] = m_hist->book_hist1D(
                base + "/EPCal_cut" + sec_suffix,
                fmt::format("E_{{PCal}} (cut, Sector {});E_{{PCal}} [GeV];Tracks", sector),
                200, 0, 1);

            h.Etot_vs_SF[sector] = m_hist->book_hist2D(
                base + "/Etot_vs_SF" + sec_suffix,
                fmt::format("E_{{tot}} vs SF (Sector {});E_{{tot}} [GeV];E/p", sector),
                200, 0, 2, 200, 0, 0.5);

            h.Etot_vs_SF_cut[sector] = m_hist->book_hist2D(
                base + "/Etot_vs_SF_cut" + sec_suffix,
                fmt::format("E_{{tot}} vs SF (cut, Sector {});E_{{tot}} [GeV];E/p", sector),
                200, 0, 2, 200, 0, 0.5);

            h.p_vs_SF[sector] = m_hist->book_hist2D(
                base + "/p_vs_SF" + sec_suffix,
                fmt::format("p vs SF (Sector {});p [GeV];E/p", sector),
                200, 0, 11, 200, 0, 0.4);

            h.edge_vs_theta_r1[sector] = m_hist->book_hist2D(
                base + "/edge_vs_theta_r1" + sec_suffix,
                fmt::format("Edge vs #theta (R1, Sector {});edge [cm];#theta [deg]", sector),
                200, 0, 50, 200, 0, 40);

            h.edge_vs_theta_r2[sector] = m_hist->book_hist2D(
                base + "/edge_vs_theta_r2" + sec_suffix,
                fmt::format("Edge vs #theta (R2, Sector {});edge [cm];#theta [deg]", sector),
                200, 0, 50, 200, 0, 40);

            h.edge_vs_theta_r3[sector] = m_hist->book_hist2D(
                base + "/edge_vs_theta_r3" + sec_suffix,
                fmt::format("Edge vs #theta (R3, Sector {});edge [cm];#theta [deg]", sector),
                200, 0, 50, 200, 0, 40);

            h.edge_vs_chi2ndf_r1[sector] = m_hist->book_hist2D(
                base + "/edge_vs_chi2ndf_r1" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R1, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.edge_vs_chi2ndf_r2[sector] = m_hist->book_hist2D(
                base + "/edge_vs_chi2ndf_r2" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R2, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.edge_vs_chi2ndf_r3[sector] = m_hist->book_hist2D(
                base + "/edge_vs_chi2ndf_r3" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R3, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.lv_vs_SF[sector] = m_hist->book_hist2D(
                base + "/lv_vs_SF" + sec_suffix,
                fmt::format("lv vs SF (Sector {});lv;E/p", sector),
                100, 0, 70, 200, 0, 0.5);
            h.lw_vs_SF[sector] = m_hist->book_hist2D(
                base + "/lw_vs_SF" + sec_suffix,
                fmt::format("lw vs SF (Sector {});lw;E/p", sector),
                100, 0, 70, 200, 0, 0.5);
            h.lu_vs_SF_low[sector] = m_hist->book_hist2D(
                base + "/lu_vs_SF_low" + sec_suffix,
                fmt::format("lu vs SF (low, Sector {});lu;E/p", sector),
                100, 0, 170, 200, 0, 0.5);
            h.lu_vs_SF_high[sector] = m_hist->book_hist2D(
                base + "/lu_vs_SF_high" + sec_suffix,
                fmt::format("lu vs SF (high, Sector {});lu;E/p", sector),
                100, 350, 450, 200, 0, 0.5);
            h.lu_vs_SF_low_with_cut[sector] = m_hist->book_hist2D(
                base + "/lu_vs_SF_low_with_cut" + sec_suffix,
                fmt::format("lu vs SF with cut (low, Sector {});lu;E/p", sector),
                100, 0, 170, 200, 0, 0.5);
            h.lu_vs_SF_high_with_cut[sector] = m_hist->book_hist2D(
                base + "/lu_vs_SF_high_with_cut" + sec_suffix,
                fmt::format("lu vs SF with cut (high, Sector {});lu;E/p", sector),
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
    auto process_tracks(const std::vector<clas12::Particle>& tracks, TrackHistograms& histograms,
                        clas12::framework::Event& event, [[maybe_unused]] double event_start_time) -> void {
        for (const auto& track : tracks) {
            clas12::CalorimeterBank calo      = clas12::read_Calorimeter_bank(event, track.index());
            clas12::CherenkovBank   cherenkov = clas12::read_Cherenkov_bank(event, track.index());
            [[maybe_unused]] clas12::ScintillatorBank scint = clas12::read_Scintillator_bank(event, track.index());
            clas12::DCTrajBank      traj      = clas12::read_Traj_bank(event, track.index());
            clas12::TrackBank       trk_r1    = clas12::read_Track_bank(event, traj.region1_index);
            clas12::TrackBank       trk_r2    = clas12::read_Track_bank(event, traj.region2_index);
            clas12::TrackBank       trk_r3    = clas12::read_Track_bank(event, traj.region3_index);

            const int sector = calo.pcal_sector;
            if (sector < 1 || sector > 6) continue;

            // Calorimeter quantities
            const double E_tot = calo.pcal_energy + calo.inner_energy + calo.outer_energy;
            const double SF = E_tot / track.p();
            const double E_cal = calo.inner_energy + calo.outer_energy;
            const double E_pcal = calo.pcal_energy;
            const int nphe = cherenkov.nphe;

            if (E_pcal == 0) continue; // Should not happen for electrons because of EB cut EPCal > 0.06 GeV

            // Fill thread-safe calorimeter histograms for this sector
            histograms.nphe[sector]->fill(nphe);
            histograms.EPCal_vs_ECal[sector]->fill(E_pcal, E_cal);
            histograms.EPCal[sector]->fill(E_pcal);
            histograms.Etot_vs_SF[sector]->fill(E_tot, SF);
            histograms.p_vs_SF[sector]->fill(track.p(), SF);

            // Apply EB-like cuts for cut histograms
            constexpr double EB_MIN_CHERENKOV_NPHE = 2.0;
            constexpr double EB_MIN_PCAL_ENERGY = 0.06;
            const bool passes_nphe = nphe > EB_MIN_CHERENKOV_NPHE;
            const bool passes_pcal = E_pcal > EB_MIN_PCAL_ENERGY;

            if (passes_nphe) {
                histograms.EPCal_vs_ECal_cut[sector]->fill(E_pcal, E_cal);
                histograms.EPCal_cut[sector]->fill(E_pcal);
            }
            if (passes_pcal && passes_nphe) {
                histograms.Etot_vs_SF_cut[sector]->fill(E_tot, SF);
            }

            // DC fiducial: edge vs theta for each region
            auto compute_local_theta = [](double x, double y, double z, int sec) {
                constexpr double SECTOR_ANGLE = M_PI / 3.0;
                double angle = (sec - 1) * SECTOR_ANGLE;
                double s = std::sin(angle);
                double c = std::cos(angle);
                return std::atan2(std::hypot(c * x - s * y, s * x + c * y), z) * 180.0 / M_PI;
            };

            const double theta_r1 = compute_local_theta(traj.region1_x, traj.region1_y, traj.region1_z, trk_r1.sector);
            const double theta_r2 = compute_local_theta(traj.region2_x, traj.region2_y, traj.region2_z, trk_r2.sector);
            const double theta_r3 = compute_local_theta(traj.region3_x, traj.region3_y, traj.region3_z, trk_r3.sector);

            histograms.edge_vs_theta_r1[sector]->fill(traj.region1_edge, theta_r1);
            histograms.edge_vs_theta_r2[sector]->fill(traj.region2_edge, theta_r2);
            histograms.edge_vs_theta_r3[sector]->fill(traj.region3_edge, theta_r3);

            // Edge vs chi2/ndf
            const double chi2ndf_r1 = trk_r1.NDF > 0 ? trk_r1.chi2 / trk_r1.NDF : std::numeric_limits<double>::quiet_NaN();
            const double chi2ndf_r2 = trk_r2.NDF > 0 ? trk_r2.chi2 / trk_r2.NDF : std::numeric_limits<double>::quiet_NaN();
            const double chi2ndf_r3 = trk_r3.NDF > 0 ? trk_r3.chi2 / trk_r3.NDF : std::numeric_limits<double>::quiet_NaN();

            histograms.edge_vs_chi2ndf_r1[sector]->fill(traj.region1_edge, chi2ndf_r1);
            histograms.edge_vs_chi2ndf_r2[sector]->fill(traj.region2_edge, chi2ndf_r2);
            histograms.edge_vs_chi2ndf_r3[sector]->fill(traj.region3_edge, chi2ndf_r3);

            // SF vs lv,w
            histograms.lv_vs_SF[sector]->fill(calo.pcal_lv, SF);
            histograms.lw_vs_SF[sector]->fill(calo.pcal_lw, SF);

            // Low end and high end of lu
            histograms.lu_vs_SF_low[sector]->fill(calo.pcal_lu, SF);
            histograms.lu_vs_SF_high[sector]->fill(calo.pcal_lu, SF);
            if (calo.pcal_lv > 9 && calo.pcal_lw > 9) {
                histograms.lu_vs_SF_low_with_cut[sector]->fill(calo.pcal_lu, SF);
                histograms.lu_vs_SF_high_with_cut[sector]->fill(calo.pcal_lu, SF);
            }
        }
    }

    auto fill_final_histograms(clas12::framework::Event& event, const clas12::Particle& electron, const clas12::CalorimeterBank& calo, const clas12::DCTrajBank& traj, const clas12::TrackBank& track) -> void {
        // Fill thread-safe electron kinematics histograms
        h_electron.momentum->fill(electron.p());
        h_electron.theta->fill(electron.theta());
        h_electron.phi->fill(electron.phi());
        h_electron.vz->fill(electron.vz());
        h_electron.chi2pid->fill(electron.chi2pid());
        h_electron.p_vs_theta->fill(electron.theta(), electron.p());
        h_electron.phi_vs_theta->fill(electron.theta(), electron.phi());

        fill_vz_histograms(h_electron_vz, electron.theta(), electron.p(), electron.vz(), calo.pcal_sector);

        int rec_track_row = -1;
        for (auto r : event.rows_for_pindex<clas12::bankdefs::REC_Track>(electron.index())) {
            if (r.detector() != clas12::DC) continue;
            rec_track_row = r.row_index();
            break;
        }
        if (rec_track_row >= 0) {
            for (auto fmt : event.rows_for_index<clas12::bankdefs::FMT_Tracks>(rec_track_row)) {
                const double fmt_px = fmt.p0_x();
                const double fmt_py = fmt.p0_y();
                const double fmt_pz = fmt.p0_z();
                const double fmt_p  = std::sqrt(fmt_px * fmt_px + fmt_py * fmt_py + fmt_pz * fmt_pz);
                if (fmt_p <= 0.0) break;
                const double fmt_theta = std::acos(fmt_pz / fmt_p) * 180.0 / M_PI;
                const double fmt_vz    = fmt.Vtx0_z();
                fill_vz_histograms(h_electron_vz_fmt, fmt_theta, fmt_p, fmt_vz, fmt.sector());
                break;
            }
        }

        // Fill calorimeter - PCAL
        h_calorimeter.lu_vs_lv_pcal->fill(calo.pcal_lv, calo.pcal_lu);
        h_calorimeter.lu_vs_lw_pcal->fill(calo.pcal_lw, calo.pcal_lu);

        // Fill calorimeter - Inner EC
        h_calorimeter.lu_vs_lv_inner->fill(calo.inner_lv, calo.inner_lu);
        h_calorimeter.lu_vs_lw_inner->fill(calo.inner_lw, calo.inner_lu);

        // Fill calorimeter - Outer EC
        h_calorimeter.lu_vs_lv_outer->fill(calo.outer_lv, calo.outer_lu);
        h_calorimeter.lu_vs_lw_outer->fill(calo.outer_lw, calo.outer_lu);

        // Fill DC histograms by momentum bin
        fill_dc_momentum_binned(electron.p(), traj, track);
        // Fill DC histograms by theta bin
        fill_dc_theta_binned(electron.theta(), traj, track);
    }

    auto book_vz_histograms(VzHistograms& h, const std::string& path_prefix, const std::string& title_prefix) -> void {
        h.vz_vs_theta = m_hist->book_hist2D(
            path_prefix + "/vz_vs_theta",
            title_prefix + " v_{z} vs #theta;#theta [deg];v_{z} [cm]",
            100, 0, 40, 100, -13, 0);
        h.vz_vs_p = m_hist->book_hist2D(
            path_prefix + "/vz_vs_p",
            title_prefix + " v_{z} vs p;p [GeV/c];v_{z} [cm]",
            100, 0, 10, 100, -13, 0);
        for (int i = 0; i < N_VZ_P_BINS; ++i) {
            const auto plo = std::to_string(static_cast<int>(vz_p_bin_edges[i]));
            const auto phi = std::to_string(static_cast<int>(vz_p_bin_edges[i + 1]));
            const auto title = title_prefix + " v_{z} vs #theta (" + plo + "<p<" + phi
                             + " GeV/c);#theta [deg];v_{z} [cm]";
            h.vz_vs_theta_pbin[i] = m_hist->book_hist2D(
                path_prefix + "/vz_vs_theta_p" + plo + "_" + phi,
                title, 100, 0, 40, 100, -13, 0);
            for (int s = 0; s < N_SECTORS; ++s) {
                const auto sec = std::to_string(s + 1);
                const auto sec_title = title_prefix + " v_{z} vs #theta (sector " + sec
                                     + ", " + plo + "<p<" + phi
                                     + " GeV/c);#theta [deg];v_{z} [cm]";
                h.vz_vs_theta_pbin_sector[s][i] = m_hist->book_hist2D(
                    path_prefix + "/Sector" + sec + "/vz_vs_theta_p" + plo + "_" + phi,
                    sec_title, 100, 0, 40, 100, -13, 0);
            }
        }
    }

    auto fill_vz_histograms(VzHistograms& h, double e_theta, double e_p, double e_vz, int e_sector) -> void {
        h.vz_vs_theta->fill(e_theta, e_vz);
        h.vz_vs_p->fill(e_p, e_vz);
        for (int i = 0; i < N_VZ_P_BINS; ++i) {
            if (e_p >= vz_p_bin_edges[i] && e_p < vz_p_bin_edges[i + 1]) {
                h.vz_vs_theta_pbin[i]->fill(e_theta, e_vz);
                if (e_sector >= 1 && e_sector <= N_SECTORS) {
                    h.vz_vs_theta_pbin_sector[e_sector - 1][i]->fill(e_theta, e_vz);
                }
                break;
            }
        }
    }

    /// Fill DC histograms in the appropriate momentum bin
    auto fill_dc_momentum_binned(double p, const clas12::DCTrajBank& traj, const clas12::TrackBank& track) -> void {
        // Compute chi2/ndf safely
        const double chi2ndf = (track.NDF > 0) ? track.chi2 / track.NDF : 0.0;

        // Determine momentum bin using configurable range
        const int bin_idx = static_cast<int>((p - m_config.p_bin_min) / m_config.p_bin_width);
        const int p_low = static_cast<int>(m_config.p_bin_min) + bin_idx;
        const int p_high = p_low + static_cast<int>(m_config.p_bin_width);

        // Check bounds
        if (p < m_config.p_bin_min || p >= m_config.p_bin_max) return;

        const std::string bin_key = fmt::format("{}_{}", p_low, p_high);

        // Fill thread-safe histogram if it exists for this bin
        if (auto it = h_dc.chi2ndf_vs_edge_r1.find(bin_key); it != h_dc.chi2ndf_vs_edge_r1.end()) {
            it->second->fill(traj.region1_edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r2.find(bin_key); it != h_dc.chi2ndf_vs_edge_r2.end()) {
            it->second->fill(traj.region2_edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r3.find(bin_key); it != h_dc.chi2ndf_vs_edge_r3.end()) {
            it->second->fill(traj.region3_edge, chi2ndf);
        }
    }

    auto fill_dc_theta_binned(double theta, const clas12::DCTrajBank& traj, const clas12::TrackBank& track) -> void {
        // Compute chi2/ndf safely
        const double chi2ndf = (track.NDF > 0) ? track.chi2 / track.NDF : 0.0;

        // Determine theta bin using configurable range
        const int bin_idx = static_cast<int>((theta - m_config.theta_bin_min) / m_config.theta_bin_width);
        const int theta_low = static_cast<int>(m_config.theta_bin_min) + bin_idx * static_cast<int>(m_config.theta_bin_width);
        const int theta_high = theta_low + static_cast<int>(m_config.theta_bin_width);

        // Check bounds
        if (theta < m_config.theta_bin_min || theta >= m_config.theta_bin_max) return;

        const std::string bin_key = fmt::format("{}_{}", theta_low, theta_high);

        // Fill thread-safe histogram if it exists for this bin
        if (auto it = h_dc.chi2ndf_vs_edge_r1.find(bin_key); it != h_dc.chi2ndf_vs_edge_r1.end()) {
            it->second->fill(traj.region1_edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r2.find(bin_key); it != h_dc.chi2ndf_vs_edge_r2.end()) {
            it->second->fill(traj.region2_edge, chi2ndf);
        }
        if (auto it = h_dc.chi2ndf_vs_edge_r3.find(bin_key); it != h_dc.chi2ndf_vs_edge_r3.end()) {
            it->second->fill(traj.region3_edge, chi2ndf);
        }
    }
};

}  // namespace clas12::algorithms
