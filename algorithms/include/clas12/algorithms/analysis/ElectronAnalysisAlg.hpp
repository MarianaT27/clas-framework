#pragma once

#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SectorHistograms.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/ConditionsService.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>
#include <clas12/physics/Kinematics.hpp>
#include <vector>

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
    using StatusCode = clas12::framework::StatusCode;

    explicit ElectronAnalysisAlg(std::string name = "ElectronAnalysis", const Config& config = Config{}) : SimpleAlgorithm(std::move(name)), m_config(config), m_target(m_config.target), m_polarity(m_config.polarity), m_cutflow("Electron Selection") {
        required_services(m_hist, m_ntuple, m_particle, m_electron_cuts);
        declare_banks("REC::Event", "REC::Particle", "REC::Calorimeter", "REC::Cherenkov", "REC::Scintillator", "REC::Traj", "REC::Track");
        declare_properties(m_enable_chi2pid_cut, m_enable_momentum_cut, m_enable_vz_cut, m_enable_sf_cut, m_enable_pcal_fiducial_cut, m_enable_dc_chi2ndf_cut);
    }

    /// Access configuration (const)
    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::info(name(), "Initializing algorithm");
        Log::info(name(), "Target: {}, Polarity: {}", m_target, m_polarity);

        m_electron_cuts->set_target(m_target);
        m_electron_cuts->set_polarity(m_polarity);

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Trigger Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 0.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");

        m_ntuple_electron.init(m_ntuple.get(), "electron_analysis");
        Log::info(name(), "N-tuple branches registered for electron analysis");

        book_histograms();

        return StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        bankdefs::REC_Particle rec_particle = event.bank<clas12::bankdefs::REC_Particle>();
        framework::EventParticles particles = m_particle->extract_particles(rec_particle);

        // Process negative tracks
        process_tracks(particles.by_charge(-1), h_neg_tracks, event);

        // Select trigger electron
        if (particles.electrons().empty()) return StatusCode::FILTERED();
        std::optional<clas12::Particle> trigger_e = particles.find_trigger_electron();
        if (!trigger_e.has_value()) return StatusCode::FILTERED();
        const Particle electron = trigger_e.value();
        m_cutflow.pass("Trigger Electron");

        // Build electron candidate with all detector information
        ElectronCandidate can;
        can.particle = electron;
        can.calorimeter = clas12::read_Calorimeter_bank(event, electron.index());
        can.traj = clas12::read_Traj_bank(event, electron.index());
        can.track = clas12::read_Track_bank(event, electron.index());
        can.total_energy = can.calorimeter.pcal_energy + can.calorimeter.inner_energy + can.calorimeter.outer_energy;
        can.sampling_fraction = can.total_energy / electron.p();
        can.sector = can.calorimeter.pcal_sector;

        process_tracks({can.particle}, h_electron_tracks, event);

        if (m_enable_chi2pid_cut.get() && !m_electron_cuts->pass_chi2pid_cut(electron.chi2pid())) return StatusCode::FILTERED();
        m_cutflow.pass("Chi2 PID");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("Chi2PID", can);

        if (m_enable_momentum_cut.get() && m_electron_cuts->pass_momentum_cut(electron.p())) return StatusCode::FILTERED();
        m_cutflow.pass("Momentum > 0.8 GeV");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("Momentum", can);

        if (m_enable_vz_cut.get() && m_electron_cuts->pass_vz_cut(electron.vz())) return StatusCode::FILTERED();
        m_cutflow.pass("Vertex Z");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("VertexZ", can);

        if (m_enable_sf_cut.get() && m_electron_cuts->pass_sampling_fraction_cut(can.sampling_fraction, electron.p(), can.sector)) return StatusCode::FILTERED();
        m_cutflow.pass("Sampling Fraction");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("SamplingFraction", can);

        if (m_enable_pcal_fiducial_cut.get() && m_electron_cuts->pass_pcal_fiducial_cut(can.calorimeter.pcal_lv, can.calorimeter.pcal_lw)) return StatusCode::FILTERED();
        m_cutflow.pass("PCAL Fiducial");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("PCALFiducial", can);

        if (m_enable_dc_chi2ndf_cut.get() && m_electron_cuts->pass_dc_edge_cut(can.traj.region1_edge, can.traj.region2_edge, can.traj.region3_edge)) return StatusCode::FILTERED();
        m_cutflow.pass("DC Chi2/NDF");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("DCChi2", can);

        // All cuts passed - fill final histograms
        fill_final_histograms(electron, can.calorimeter, can.traj, can.track);

        // Fill N-tuple if service is available
        if (m_ntuple) {
            m_ntuple_electron.data().from(can);
            m_ntuple_electron.fill();
        }

        // Set decision flag for downstream algorithms
        event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, true);

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        if (Log::is_available()) {
            Log::info(name(), "Finalizing algorithm");
            auto final_count = m_cutflow.get_count("DC Chi2/NDF");
            Log::info(name(), "Events passing all cuts: {}", final_count);

            if (m_ntuple) {
                auto entries = m_ntuple->get_entries("electron_analysis");
                Log::info(name(), "N-tuple has {} entries", entries);
            }
        }

        fmt::print("\n");
        m_cutflow.print();

        return clas12::framework::StatusCode::SUCCESS();
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

    // Services (auto-resolved by required_services(...) in the constructor)
    clas12::framework::ServiceRef<clas12::framework::HistogramService> m_hist;
    clas12::framework::ServiceRef<clas12::framework::NTupleService> m_ntuple;
    clas12::framework::ServiceRef<clas12::framework::ParticleSelectorService> m_particle;
    clas12::framework::ServiceRef<clas12::framework::ElectronCutsService> m_electron_cuts;

    struct ElectronCandidate {
        clas12::Particle particle;
        clas12::CalorimeterBank calorimeter;
        clas12::DCTrajBank traj;
        clas12::TrackBank track;
        double sampling_fraction = 0.0;
        double total_energy = 0.0;
        int sector = 0;

        [[nodiscard]] auto is_valid() const -> bool { return sector >= 1 && sector <= 6 && particle.p() > 0; }
    };

    /// N-Tuple data structure for electron analysis. Branch names and
    /// ordering match the original output schema so downstream readers
    /// see no change.
    struct ElectronNTupleData {
        double e_p = 0.0;
        double e_theta = 0.0;
        double e_phi = 0.0;
        double e_vz = 0.0;
        double e_chi2pid = 0.0;
        double sampling_fraction = 0.0;
        int sector = 0;
        double x_r1 = 0.0;
        double y_r1 = 0.0;
        double z_r1 = 0.0;
        double x_r2 = 0.0;
        double y_r2 = 0.0;
        double z_r2 = 0.0;

        auto from(const ElectronCandidate& can) -> void {
            e_p = can.particle.p();
            e_theta = can.particle.theta();
            e_phi = can.particle.phi();
            e_vz = can.particle.vz();
            e_chi2pid = can.particle.chi2pid();
            sampling_fraction = can.sampling_fraction;
            sector = can.sector;
            x_r1 = can.traj.region1_x;
            y_r1 = can.traj.region1_y;
            z_r1 = can.traj.region1_z;
            x_r2 = can.traj.region2_x;
            y_r2 = can.traj.region2_y;
            z_r2 = can.traj.region2_z;
        }

        static constexpr auto ntuple_fields() {
            using T = ElectronNTupleData;
            return std::make_tuple(
                clas12::framework::field("e_p", &T::e_p),
                clas12::framework::field("e_theta", &T::e_theta),
                clas12::framework::field("e_phi", &T::e_phi),
                clas12::framework::field("e_vz", &T::e_vz),
                clas12::framework::field("e_chi2pid", &T::e_chi2pid),
                clas12::framework::field("sampling_fraction", &T::sampling_fraction),
                clas12::framework::field("sector", &T::sector),
                clas12::framework::field("x_r1", &T::x_r1),
                clas12::framework::field("y_r1", &T::y_r1),
                clas12::framework::field("z_r1", &T::z_r1),
                clas12::framework::field("x_r2", &T::x_r2),
                clas12::framework::field("y_r2", &T::y_r2),
                clas12::framework::field("z_r2", &T::z_r2)
            );
        }
    };

    clas12::framework::NTupleWriter<ElectronNTupleData> m_ntuple_electron;

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

    // DC chi2/ndf vs edge, sliced on momentum and on theta.
    // Each TH3D replaces a stack of per-bin 2D histograms; project at
    // analysis time via TH3::ProjectionYZ(binlo, binhi).
    struct {
        clas12::framework::ThreadLocalHistogram3D* chi2ndf_vs_edge_vs_p_r1 = nullptr;
        clas12::framework::ThreadLocalHistogram3D* chi2ndf_vs_edge_vs_p_r2 = nullptr;
        clas12::framework::ThreadLocalHistogram3D* chi2ndf_vs_edge_vs_p_r3 = nullptr;
        clas12::framework::ThreadLocalHistogram3D* chi2ndf_vs_edge_vs_theta_r1 = nullptr;
        clas12::framework::ThreadLocalHistogram3D* chi2ndf_vs_edge_vs_theta_r2 = nullptr;
        clas12::framework::ThreadLocalHistogram3D* chi2ndf_vs_edge_vs_theta_r3 = nullptr;
    } h_dc;

    struct {
        clas12::framework::ThreadLocalHistogram2D* vz_vs_theta = nullptr;
        clas12::framework::ThreadLocalHistogram2D* vz_vs_p = nullptr;
        clas12::framework::ThreadLocalHistogram3D* vz_vs_theta_vs_p = nullptr;
    } h_electron_vz;

    // Track histograms structure (for neg/pos tracks, pions, etc.)
    // Each histogram is stored per sector (1-6) via PerSectorHist handles.
    struct TrackHistograms {
        // Calorimeter (per sector)
        clas12::framework::PerSectorHist1D<6> nphe;
        clas12::framework::PerSectorHist2D<6> EPCal_vs_ECal;
        clas12::framework::PerSectorHist1D<6> EPCal;
        clas12::framework::PerSectorHist2D<6> EPCal_vs_ECal_cut;
        clas12::framework::PerSectorHist1D<6> EPCal_cut;
        clas12::framework::PerSectorHist2D<6> Etot_vs_SF;
        clas12::framework::PerSectorHist2D<6> Etot_vs_SF_cut;
        clas12::framework::PerSectorHist2D<6> p_vs_SF;
        clas12::framework::PerSectorHist2D<6> lv_vs_SF;
        clas12::framework::PerSectorHist2D<6> lw_vs_SF;
        clas12::framework::PerSectorHist2D<6> lu_vs_SF_low;
        clas12::framework::PerSectorHist2D<6> lu_vs_SF_high;
        clas12::framework::PerSectorHist2D<6> lu_vs_SF_low_with_cut;
        clas12::framework::PerSectorHist2D<6> lu_vs_SF_high_with_cut;
        // DC fiducial (per sector)
        clas12::framework::PerSectorHist2D<6> edge_vs_theta_r1;
        clas12::framework::PerSectorHist2D<6> edge_vs_theta_r2;
        clas12::framework::PerSectorHist2D<6> edge_vs_theta_r3;
        clas12::framework::PerSectorHist2D<6> edge_vs_chi2ndf_r1;
        clas12::framework::PerSectorHist2D<6> edge_vs_chi2ndf_r2;
        clas12::framework::PerSectorHist2D<6> edge_vs_chi2ndf_r3;
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

    auto book_histograms() -> void {
        h_electron.momentum = m_hist->book_hist1D("/Electron/momentum", "Electron Momentum;p [GeV/c];Events", 120, 0, 12);
        h_electron.theta = m_hist->book_hist1D("/Electron/theta", "Electron #theta;#theta [deg];Events", 180, 0, 180);
        h_electron.phi = m_hist->book_hist1D("/Electron/phi", "Electron #phi;#phi [deg];Events", 180, -180, 180);
        h_electron.vz = m_hist->book_hist1D("/Electron/vz", "Electron v_{z};v_{z} [cm];Events", 200, -20, 20);
        h_electron.chi2pid = m_hist->book_hist1D("/Electron/chi2pid", "Electron #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);

        h_electron.p_vs_theta = m_hist->book_hist2D("/Electron/p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]", 180, 0, 180, 120, 0, 12);
        h_electron.phi_vs_theta = m_hist->book_hist2D("/Electron/phi_vs_theta", "#phi vs #theta;#theta [deg];#phi [deg]", 180, 0, 180, 180, -180, 180);

        h_electron_vz.vz_vs_theta = m_hist->book_hist2D("/Electron/vz_vs_theta", "Electron v_{z} vs #theta;#theta [deg];v_{z} [cm]", 100, 0, 40, 100, -13, 0);
        h_electron_vz.vz_vs_p = m_hist->book_hist2D("/Electron/vz_vs_p", "Electron v_{z} vs p;p [GeV/c];v_{z} [cm]", 100, 0, 10, 100, -13, 0);
        h_electron_vz.vz_vs_theta_vs_p = m_hist->book_hist3D("/Electron/vz_vs_theta_vs_p", "Electron v_{z} vs #theta vs p;p [GeV/c];#theta [deg];v_{z} [cm]", 20, 0, 10, 100, 0, 40, 100, -13, 0);

        h_calorimeter.lu_vs_lv_pcal = m_hist->book_hist2D("/Calorimeter/lu_vs_lv_pcal", "PCAL lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_pcal = m_hist->book_hist2D("/Calorimeter/lu_vs_lw_pcal", "PCAL lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        h_calorimeter.lu_vs_lv_inner = m_hist->book_hist2D("/Calorimeter/lu_vs_lv_inner", "Inner EC lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_inner = m_hist->book_hist2D("/Calorimeter/lu_vs_lw_inner", "Inner EC lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        h_calorimeter.lu_vs_lv_outer = m_hist->book_hist2D("/Calorimeter/lu_vs_lv_outer", "Outer EC lu vs lv;lv;lu", 100, 0, 500, 100, 0, 500);
        h_calorimeter.lu_vs_lw_outer = m_hist->book_hist2D("/Calorimeter/lu_vs_lw_outer", "Outer EC lu vs lw;lw;lu", 100, 0, 500, 100, 0, 500);

        const int p_nbins = static_cast<int>((m_config.p_bin_max - m_config.p_bin_min) / m_config.p_bin_width);
        const int theta_nbins = static_cast<int>((m_config.theta_bin_max - m_config.theta_bin_min) / m_config.theta_bin_width);

        auto book_dc_3d = [&](const std::string& path, const std::string& title, int xn, double xlo, double xhi) {
            return m_hist->book_hist3D(path, title, xn, xlo, xhi, 
                                200, 0, 50,    // edge
                                200, 0, 10);   // chi2/ndf
        };

        h_dc.chi2ndf_vs_edge_vs_p_r1 = book_dc_3d("/DC/chi2ndf_vs_edge_vs_p_r1", "Region 1;p [GeV/c];edge [cm];#chi^{2}/ndf", p_nbins, m_config.p_bin_min, m_config.p_bin_max);
        h_dc.chi2ndf_vs_edge_vs_p_r2 = book_dc_3d("/DC/chi2ndf_vs_edge_vs_p_r2", "Region 2;p [GeV/c];edge [cm];#chi^{2}/ndf", p_nbins, m_config.p_bin_min, m_config.p_bin_max);
        h_dc.chi2ndf_vs_edge_vs_p_r3 = book_dc_3d("/DC/chi2ndf_vs_edge_vs_p_r3", "Region 3;p [GeV/c];edge [cm];#chi^{2}/ndf", p_nbins, m_config.p_bin_min, m_config.p_bin_max);

        h_dc.chi2ndf_vs_edge_vs_theta_r1 = book_dc_3d("/DC/chi2ndf_vs_edge_vs_theta_r1", "Region 1;#theta [deg];edge [cm];#chi^{2}/ndf", theta_nbins, m_config.theta_bin_min, m_config.theta_bin_max);
        h_dc.chi2ndf_vs_edge_vs_theta_r2 = book_dc_3d("/DC/chi2ndf_vs_edge_vs_theta_r2", "Region 2;#theta [deg];edge [cm];#chi^{2}/ndf", theta_nbins, m_config.theta_bin_min, m_config.theta_bin_max);
        h_dc.chi2ndf_vs_edge_vs_theta_r3 = book_dc_3d("/DC/chi2ndf_vs_edge_vs_theta_r3", "Region 3;#theta [deg];edge [cm];#chi^{2}/ndf", theta_nbins, m_config.theta_bin_min, m_config.theta_bin_max);

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
        Log::info(name(), "Book all histograms");
    }

    /**
     * @brief Books histograms for a specific cut stage
     * Must be called during initialize() to ensure thread safety
     */
    auto book_cut_stage_histograms(const std::string& cut_name) -> void {
        auto& cut_hists = m_cut_stage_histograms[cut_name];
        std::string base = fmt::format("/Electron/CutStages/{}", cut_name);

        cut_hists.momentum = m_hist->book_hist1D(base + "/p", fmt::format("Electron p after {};p [GeV/c];Events", cut_name), 120, 0, 12);
        cut_hists.theta = m_hist->book_hist1D(base + "/theta", fmt::format("Electron #theta after {};#theta [deg];Events", cut_name), 180, 0, 180);
        cut_hists.phi = m_hist->book_hist1D(base + "/phi", fmt::format("Electron #phi after {};#phi [deg];Events", cut_name), 180, -180, 180);
        cut_hists.vz = m_hist->book_hist1D(base + "/vz", fmt::format("Electron v_{{z}} after {};v_{{z}} [cm];Events", cut_name), 200, -20, 20);
        cut_hists.sampling_fraction = m_hist->book_hist2D(base + "/sf_vs_p", fmt::format("SF vs p after {};p [GeV/c];E/p", cut_name), 120, 0, 12, 100, 0, 0.5);
        cut_hists.dc_edge_r1 = m_hist->book_hist1D(base + "/dc_edge_r1", fmt::format("DC Edge R1 after {};edge [cm];Events", cut_name), 100, 0, 50);

        cut_hists.booked = true;
    }

    /**
     * @brief Fills histograms after each cut stage to monitor cut effects
     *
     * Pre-booked histograms per cut name are populated from the
     * ElectronCandidate bundle, which already holds the particle, the
     * cached bank reads, and the derived sampling fraction.
     *
     * @param cut_name Name of the cut that was just passed
     * @param can     Electron candidate carrying particle + bank + SF
     */
    auto fill_after_cut(const std::string& cut_name, const ElectronCandidate& can) -> void {
        auto& cut_hists = m_cut_stage_histograms.at(cut_name);

        cut_hists.momentum->fill(can.particle.p());
        cut_hists.theta->fill(can.particle.theta());
        cut_hists.phi->fill(can.particle.phi());
        cut_hists.vz->fill(can.particle.vz());
        cut_hists.sampling_fraction->fill(can.particle.p(), can.sampling_fraction);
        cut_hists.dc_edge_r1->fill(can.traj.region1_edge);
    }

    /**
     * @brief Books histograms for a track category with per-sector granularity
     */
    auto book_track_histograms(TrackHistograms& h, const std::string& category) -> void {
        clas12::framework::SectorHistogramGroup<6> g(m_hist.get(), fmt::format("/Tracks/{}", category));

        h.nphe = g.add_hist1D("nphe", "Cherenkov nphe;nphe;Tracks", 51, 0, 50);
        h.EPCal_vs_ECal = g.add_hist2D("EPCal_vs_ECal", "E_{PCal} vs E_{Cal};E_{PCal} [GeV];E_{Cal} [GeV]", 200, 0, 0.6, 200, 0, 0.6);
        h.EPCal = g.add_hist1D("EPCal", "E_{PCal};E_{PCal} [GeV];Tracks", 200, 0, 1);
        h.EPCal_vs_ECal_cut = g.add_hist2D("EPCal_vs_ECal_cut", "E_{PCal} vs E_{Cal} (cut);E_{PCal} [GeV];E_{Cal} [GeV]", 200, 0, 0.6, 200, 0, 0.6);
        h.EPCal_cut = g.add_hist1D("EPCal_cut", "E_{PCal} (cut);E_{PCal} [GeV];Tracks", 200, 0, 1);
        h.Etot_vs_SF = g.add_hist2D("Etot_vs_SF", "E_{tot} vs SF;E_{tot} [GeV];E/p", 200, 0, 2, 200, 0, 0.5);
        h.Etot_vs_SF_cut = g.add_hist2D("Etot_vs_SF_cut", "E_{tot} vs SF (cut);E_{tot} [GeV];E/p", 200, 0, 2, 200, 0, 0.5);
        h.p_vs_SF = g.add_hist2D("p_vs_SF", "p vs SF;p [GeV];E/p", 200, 0, 11, 200, 0, 0.4);

        h.edge_vs_theta_r1 = g.add_hist2D("edge_vs_theta_r1", "Edge vs #theta (R1);edge [cm];#theta [deg]", 200, 0, 50, 200, 0, 40);
        h.edge_vs_theta_r2 = g.add_hist2D("edge_vs_theta_r2", "Edge vs #theta (R2);edge [cm];#theta [deg]", 200, 0, 50, 200, 0, 40);
        h.edge_vs_theta_r3 = g.add_hist2D("edge_vs_theta_r3", "Edge vs #theta (R3);edge [cm];#theta [deg]", 200, 0, 50, 200, 0, 40);
        h.edge_vs_chi2ndf_r1 = g.add_hist2D("edge_vs_chi2ndf_r1", "Edge vs #chi^{2}/ndf (R1);edge [cm];#chi^{2}/ndf", 200, 0, 50, 200, 0, 10);
        h.edge_vs_chi2ndf_r2 = g.add_hist2D("edge_vs_chi2ndf_r2", "Edge vs #chi^{2}/ndf (R2);edge [cm];#chi^{2}/ndf", 200, 0, 50, 200, 0, 10);
        h.edge_vs_chi2ndf_r3 = g.add_hist2D("edge_vs_chi2ndf_r3", "Edge vs #chi^{2}/ndf (R3);edge [cm];#chi^{2}/ndf", 200, 0, 50, 200, 0, 10);

        h.lv_vs_SF = g.add_hist2D("lv_vs_SF", "lv vs SF;lv;E/p", 100, 0, 70, 200, 0, 0.5);
        h.lw_vs_SF = g.add_hist2D("lw_vs_SF", "lw vs SF;lw;E/p", 100, 0, 70, 200, 0, 0.5);
        h.lu_vs_SF_low = g.add_hist2D("lu_vs_SF_low", "lu vs SF (low);lu;E/p", 100, 0, 170, 200, 0, 0.5);
        h.lu_vs_SF_high = g.add_hist2D("lu_vs_SF_high", "lu vs SF (high);lu;E/p", 100, 350, 450, 200, 0, 0.5);
        h.lu_vs_SF_low_with_cut = g.add_hist2D("lu_vs_SF_low_with_cut", "lu vs SF with cut (low);lu;E/p", 100, 0, 170, 200, 0, 0.5);
        h.lu_vs_SF_high_with_cut = g.add_hist2D("lu_vs_SF_high_with_cut", "lu vs SF with cut (high);lu;E/p", 100, 350, 450, 200, 0, 0.5);
    }

    /**
     * @brief Process tracks and fill per-sector histograms
     *
     * For each track, reads calorimeter, Cherenkov, and DC trajectory
     * information and fills the corresponding per-sector histograms.
     */
    auto process_tracks(const std::vector<clas12::Particle>& tracks, TrackHistograms& histograms, clas12::framework::Event& event) -> void {
        for (const auto& track : tracks) {
            clas12::CalorimeterBank calo = clas12::read_Calorimeter_bank(event, track.index());
            clas12::CherenkovBank cherenkov = clas12::read_Cherenkov_bank(event, track.index());
            clas12::DCTrajBank traj = clas12::read_Traj_bank(event, track.index());
            clas12::TrackBank trk_r1 = clas12::read_Track_bank(event, traj.region1_index);
            clas12::TrackBank trk_r2 = clas12::read_Track_bank(event, traj.region2_index);
            clas12::TrackBank trk_r3 = clas12::read_Track_bank(event, traj.region3_index);

            const int sector = calo.pcal_sector;
            if (sector < 1 || sector > 6) continue;

            // Calorimeter quantities
            const double E_tot = calo.pcal_energy + calo.inner_energy + calo.outer_energy;
            const double SF = E_tot / track.p();
            const double E_cal = calo.inner_energy + calo.outer_energy;
            const double E_pcal = calo.pcal_energy;
            const int nphe = cherenkov.nphe;

            if (E_pcal == 0) continue;  // Should not happen for electrons because of EB cut EPCal > 0.06 GeV

            // Fill thread-safe calorimeter histograms for this sector
            histograms.nphe.fill(sector, nphe);
            histograms.EPCal_vs_ECal.fill(sector, E_pcal, E_cal);
            histograms.EPCal.fill(sector, E_pcal);
            histograms.Etot_vs_SF.fill(sector, E_tot, SF);
            histograms.p_vs_SF.fill(sector, track.p(), SF);

            // Apply EB-like cuts for cut histograms
            constexpr double EB_MIN_CHERENKOV_NPHE = 2.0;
            constexpr double EB_MIN_PCAL_ENERGY = 0.06;

            if (nphe > EB_MIN_CHERENKOV_NPHE) {
                histograms.EPCal_vs_ECal_cut.fill(sector, E_pcal, E_cal);
                histograms.EPCal_cut.fill(sector, E_pcal);
            }
            if (nphe > EB_MIN_CHERENKOV_NPHE && E_pcal > EB_MIN_PCAL_ENERGY) {
                histograms.Etot_vs_SF_cut.fill(sector, E_tot, SF);
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

            histograms.edge_vs_theta_r1.fill(sector, traj.region1_edge, theta_r1);
            histograms.edge_vs_theta_r2.fill(sector, traj.region2_edge, theta_r2);
            histograms.edge_vs_theta_r3.fill(sector, traj.region3_edge, theta_r3);

            // Edge vs chi2/ndf
            const double chi2ndf_r1 = trk_r1.NDF > 0 ? trk_r1.chi2 / trk_r1.NDF : std::numeric_limits<double>::quiet_NaN();
            const double chi2ndf_r2 = trk_r2.NDF > 0 ? trk_r2.chi2 / trk_r2.NDF : std::numeric_limits<double>::quiet_NaN();
            const double chi2ndf_r3 = trk_r3.NDF > 0 ? trk_r3.chi2 / trk_r3.NDF : std::numeric_limits<double>::quiet_NaN();

            histograms.edge_vs_chi2ndf_r1.fill(sector, traj.region1_edge, chi2ndf_r1);
            histograms.edge_vs_chi2ndf_r2.fill(sector, traj.region2_edge, chi2ndf_r2);
            histograms.edge_vs_chi2ndf_r3.fill(sector, traj.region3_edge, chi2ndf_r3);

            // SF vs lv,w
            histograms.lv_vs_SF.fill(sector, calo.pcal_lv, SF);
            histograms.lw_vs_SF.fill(sector, calo.pcal_lw, SF);

            // Low end and high end of lu
            histograms.lu_vs_SF_low.fill(sector, calo.pcal_lu, SF);
            histograms.lu_vs_SF_high.fill(sector, calo.pcal_lu, SF);
            if (calo.pcal_lv > 9 && calo.pcal_lw > 9) {
                histograms.lu_vs_SF_low_with_cut.fill(sector, calo.pcal_lu, SF);
                histograms.lu_vs_SF_high_with_cut.fill(sector, calo.pcal_lu, SF);
            }
        }
    }

    auto fill_final_histograms(const clas12::Particle& electron, const clas12::CalorimeterBank& calo, const clas12::DCTrajBank& traj, const clas12::TrackBank& track) -> void {
        // Fill thread-safe electron kinematics histograms
        h_electron.momentum->fill(electron.p());
        h_electron.theta->fill(electron.theta());
        h_electron.phi->fill(electron.phi());
        h_electron.vz->fill(electron.vz());
        h_electron.chi2pid->fill(electron.chi2pid());
        h_electron.p_vs_theta->fill(electron.theta(), electron.p());
        h_electron.phi_vs_theta->fill(electron.theta(), electron.phi());

        const double e_vz = electron.vz();
        const double e_theta = electron.theta();
        const double e_p = electron.p();
        h_electron_vz.vz_vs_theta->fill(e_theta, e_vz);
        h_electron_vz.vz_vs_p->fill(e_p, e_vz);
        h_electron_vz.vz_vs_theta_vs_p->fill(e_p, e_theta, e_vz);

        // Fill calorimeter - PCAL
        h_calorimeter.lu_vs_lv_pcal->fill(calo.pcal_lv, calo.pcal_lu);
        h_calorimeter.lu_vs_lw_pcal->fill(calo.pcal_lw, calo.pcal_lu);

        // Fill calorimeter - Inner EC
        h_calorimeter.lu_vs_lv_inner->fill(calo.inner_lv, calo.inner_lu);
        h_calorimeter.lu_vs_lw_inner->fill(calo.inner_lw, calo.inner_lu);

        // Fill calorimeter - Outer EC
        h_calorimeter.lu_vs_lv_outer->fill(calo.outer_lv, calo.outer_lu);
        h_calorimeter.lu_vs_lw_outer->fill(calo.outer_lw, calo.outer_lu);

        const double chi2ndf = (track.NDF > 0) ? track.chi2 / track.NDF : 0.0;
        // Fill DC histograms by momentum bin
        h_dc.chi2ndf_vs_edge_vs_p_r1->fill(e_p, traj.region1_edge, chi2ndf);
        h_dc.chi2ndf_vs_edge_vs_p_r2->fill(e_p, traj.region2_edge, chi2ndf);
        h_dc.chi2ndf_vs_edge_vs_p_r3->fill(e_p, traj.region3_edge, chi2ndf);
        // Fill DC histograms by theta bin
        h_dc.chi2ndf_vs_edge_vs_theta_r1->fill(e_theta, traj.region1_edge, chi2ndf);
        h_dc.chi2ndf_vs_edge_vs_theta_r2->fill(e_theta, traj.region2_edge, chi2ndf);
        h_dc.chi2ndf_vs_edge_vs_theta_r3->fill(e_theta, traj.region3_edge, chi2ndf);
    }
};

}  // namespace clas12::algorithms
