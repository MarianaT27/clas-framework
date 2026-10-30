#pragma once

#include <memory>
#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SectorHistograms.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/services/ConditionsService.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/physics/Kinematics.hpp>
#include <vector>

namespace algorithms {

// Import Log singleton for convenient logging
using framework::Log;

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
    Target target = Target::LD2;
    Polarity polarity = Polarity::Outbending;
    bool enable_cut_stage_histograms = true;
    // Momentum binning for DC histograms
    double p_bin_min = 1.0;
    double p_bin_max = 10.0;
    double p_bin_width = 1.0;
    // Theta binning for DC histograms
    double theta_bin_min = 5.0;
    double theta_bin_max = 40.0;
    double theta_bin_width = 5.0;

    bool enable_chi2pid_cut = true;
    bool enable_momentum_cut = true;
    bool enable_vz_cut = true;
    bool enable_sf_cut = true;
    bool enable_pcal_fiducial_cut = true;
    bool enable_dc_chi2ndf_cut = true;

    static constexpr auto config_fields() {
        using T = ElectronAnalysisConfig;
        return std::make_tuple(
            framework::field("target",                   &T::target),
            framework::field("polarity",                 &T::polarity),
            framework::field("enable_chi2pid_cut",       &T::enable_chi2pid_cut),
            framework::field("enable_momentum_cut",      &T::enable_momentum_cut),
            framework::field("enable_vz_cut",            &T::enable_vz_cut),
            framework::field("enable_sf_cut",            &T::enable_sf_cut),
            framework::field("enable_pcal_fiducial_cut", &T::enable_pcal_fiducial_cut),
            framework::field("enable_dc_chi2ndf_cut",    &T::enable_dc_chi2ndf_cut));
    }
};

class ElectronAnalysisAlg : public framework::Algorithm {
   public:
    using Config = ElectronAnalysisConfig;
    using StatusCode = framework::StatusCode;

    explicit ElectronAnalysisAlg(std::string name = "ElectronAnalysis", const Config& config = Config{}) : Algorithm(std::move(name)), m_config(config), m_target(m_config.target), m_polarity(m_config.polarity), m_cutflow("Electron Selection") {
        required_services(m_hist, m_ntuple, m_electron_cuts);
        declare_banks("REC::Event", "REC::Particle", "REC::Calorimeter", "REC::Cherenkov", "REC::Scintillator", "REC::Traj", "REC::Track");
        declare_config(m_config);
        declare_histograms(
            h_electron.momentum, h_electron.theta, h_electron.phi, h_electron.vz,
            h_electron.chi2pid, h_electron.p_vs_theta, h_electron.phi_vs_theta,
            h_calorimeter.lu_vs_lv_pcal, h_calorimeter.lu_vs_lw_pcal,
            h_calorimeter.lu_vs_lv_inner, h_calorimeter.lu_vs_lw_inner,
            h_calorimeter.lu_vs_lv_outer, h_calorimeter.lu_vs_lw_outer,
            h_electron_vz.vz_vs_theta, h_electron_vz.vz_vs_p, h_electron_vz.vz_vs_theta_vs_p);
    }

    /// Access configuration (const)
    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing algorithm");
        m_target = m_config.target; m_polarity = m_config.polarity;
        m_target = m_config.target; m_polarity = m_config.polarity;
        Log::info(name(), "Target: {}, Polarity: {}", m_target, m_polarity);

        m_electron_cuts->set_target(m_target);
        m_electron_cuts->set_polarity(m_polarity);

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Trigger Electron");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum > 1.8 GeV");
        m_cutflow.add_cut("Vertex Z");
        m_cutflow.add_cut("Sampling Fraction");
        m_cutflow.add_cut("PCAL Fiducial");
        m_cutflow.add_cut("DC Chi2/NDF");

        m_ntuple_electron.init(m_ntuple.get(), "electron_analysis");
        Log::info(name(), "N-tuple branches registered for electron analysis");

        book_histograms();

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        // Process negative tracks
        process_tracks(event.by_charge(-1), h_neg_tracks);
        process_tracks(event.by_charge(1), h_pos_tracks);

        // Select trigger electron. find_trigger_electron() returns nullopt when
        // none qualifies, so no separate electrons().empty() guard is needed
        // (that guard forced a full particle load before the lazy finder).
        std::optional<Particle> trigger_e = event.find_trigger_electron();
        if (!trigger_e.has_value()) return StatusCode::FILTERED();
        Particle electron = trigger_e.value();
        m_cutflow.pass("Trigger Electron");

        // Build electron candidate with all detector information
        ElectronCandidate can;
        can.particle = electron;
        can.calorimeter = electron.calo();
        can.traj = electron.traj();
        can.track = electron.track();
        can.total_energy = can.calorimeter.pcal_energy + can.calorimeter.inner_energy + can.calorimeter.outer_energy;
        can.sampling_fraction = can.total_energy / electron.p();
        can.sector = can.calorimeter.pcal_sector;

        process_tracks({can.particle}, h_electron_tracks);

        if (m_config.enable_chi2pid_cut && !m_electron_cuts->pass_chi2pid_cut(electron.chi2pid())) return StatusCode::FILTERED();
        m_cutflow.pass("Chi2 PID");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("Chi2PID", can);

        if (m_config.enable_momentum_cut && !m_electron_cuts->pass_momentum_cut(electron.p())) return StatusCode::FILTERED();
        m_cutflow.pass("Momentum > 0.8 GeV");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("Momentum", can);

        if (m_config.enable_vz_cut && !m_electron_cuts->pass_vz_cut(electron.vz())) return StatusCode::FILTERED();
        m_cutflow.pass("Vertex Z");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("VertexZ", can);

        if (m_config.enable_sf_cut && !m_electron_cuts->pass_sampling_fraction_cut(can.sampling_fraction, electron.p(), can.sector)) return StatusCode::FILTERED();
        m_cutflow.pass("Sampling Fraction");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("SamplingFraction", can);

        if (m_config.enable_pcal_fiducial_cut && !m_electron_cuts->pass_pcal_fiducial_cut(can.calorimeter.pcal_lv, can.calorimeter.pcal_lw)) return StatusCode::FILTERED();
        m_cutflow.pass("PCAL Fiducial");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("PCALFiducial", can);

        if (m_config.enable_dc_chi2ndf_cut && !m_electron_cuts->pass_dc_edge_cut(can.traj.region1_edge, can.traj.region2_edge, can.traj.region3_edge)) return StatusCode::FILTERED();
        m_cutflow.pass("DC Chi2/NDF");
        if (m_config.enable_cut_stage_histograms) fill_after_cut("DCChi2", can);

        // All cuts passed - fill final histograms
        fill_final_histograms(electron, can.calorimeter, can.traj, can.track);

        // Fill N-tuple if service is available
        if (m_ntuple) {
            m_ntuple_electron.data().from(can);
            m_ntuple_electron.fill();
        }

        // Publish the trigger electron + decision so downstream algorithms
        // (e.g. PionAnalysis) can gate on `HasGoodElectron` and pick the
        // electron up via `Keys::TriggerElectron`.
        event.store_typed(framework::Keys::TriggerElectron, electron);
        event.set_decision_typed(framework::Decisions::HasGoodElectron, true);

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> framework::StatusCode override {
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

        return framework::StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    Target m_target;
    Polarity m_polarity;
    framework::Cutflow m_cutflow;

    // Cut toggle properties
// Services (auto-resolved by required_services(...) in the constructor)
    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple;
    framework::ServiceRef<framework::ElectronCutsService> m_electron_cuts;

    struct ElectronCandidate {
        Particle particle;
        CalorimeterBank calorimeter;
        DCTrajBank traj;
        TrackBank track;
        double sampling_fraction = 0.0;
        double total_energy = 0.0;
        int sector = 0;

        [[nodiscard]] auto is_valid() const -> bool { return sector >= 1 && sector <= 6 && particle.p() > 0; }
    };

    struct ElectronNTupleData {
        double e_p = 0.0;
        double e_theta = 0.0;
        double e_phi = 0.0;
        double e_vz = 0.0;
        double e_chi2pid = 0.0;
        double sampling_fraction = 0.0;
        int sector = 0;
        double edge_r1 = 0.0;
        double edge_r2 = 0.0;
        double edge_r3 = 0.0;
        double x_r1 = 0.0;
        double y_r1 = 0.0;
        double z_r1 = 0.0;
        double x_r2 = 0.0;
        double y_r2 = 0.0;
        double z_r2 = 0.0;
        double x_r3 = 0.0;
        double y_r3 = 0.0;
        double z_r3 = 0.0;

        auto from(const ElectronCandidate& can) -> void {
            e_p = can.particle.p();
            e_theta = can.particle.theta();
            e_phi = can.particle.phi();
            e_vz = can.particle.vz();
            e_chi2pid = can.particle.chi2pid();
            sampling_fraction = can.sampling_fraction;
            sector = can.sector;
            edge_r1 = can.traj.region1_edge;
            edge_r2 = can.traj.region2_edge;
            edge_r3 = can.traj.region3_edge;
            x_r1 = can.traj.region1_x;
            y_r1 = can.traj.region1_y;
            z_r1 = can.traj.region1_z;
            x_r2 = can.traj.region2_x;
            y_r2 = can.traj.region2_y;
            z_r2 = can.traj.region2_z;
            x_r3 = can.traj.region3_x;
            y_r3 = can.traj.region3_y;
            z_r3 = can.traj.region3_z;
        }

        static constexpr auto ntuple_fields() {
            using T = ElectronNTupleData;
            return std::make_tuple(framework::field("e_p", &T::e_p), framework::field("e_theta", &T::e_theta), framework::field("e_phi", &T::e_phi), framework::field("e_vz", &T::e_vz), framework::field("e_chi2pid", &T::e_chi2pid), framework::field("sampling_fraction", &T::sampling_fraction), framework::field("sector", &T::sector),
                                   framework::field("edge_r1", &T::edge_r1), framework::field("edge_r2", &T::edge_r2), framework::field("edge_r3", &T::edge_r3), framework::field("x_r1", &T::x_r1), framework::field("y_r1", &T::y_r1), framework::field("z_r1", &T::z_r1), framework::field("x_r2", &T::x_r2), framework::field("y_r2", &T::y_r2), framework::field("z_r2", &T::z_r2), framework::field("x_r3", &T::x_r3), framework::field("y_r3", &T::y_r3), framework::field("z_r3", &T::z_r3));
        }
    };

    framework::NTupleWriter<ElectronNTupleData> m_ntuple_electron;

    // Histograms - organized by category (lock-free atomic, self-registering).
    struct {
        framework::Hist1<> momentum{"/Electron/momentum",
            "Electron Momentum;p [GeV/c];Events", 0, 12, 120};
        framework::Hist1<> theta{"/Electron/theta",
            "Electron #theta;#theta [deg];Events", 0, 180, 180};
        framework::Hist1<> phi{"/Electron/phi",
            "Electron #phi;#phi [deg];Events", -180, 180, 180};
        framework::Hist1<> vz{"/Electron/vz",
            "Electron v_{z};v_{z} [cm];Events", -20, 20, 200};
        // VzCorrector-corrected vertex z. Same binning as the raw vz so
        // the two histograms can be overlaid 1:1 offline. Falls back to
        // raw vz when no corrector variant is loaded (LD2 path).
        framework::Hist1<> vz_corr{"/Electron/vz_corr",
            "Electron corrected v_{z};v_{z}^{corr} [cm];Events", -20, 20, 200};
        framework::Hist1<> chi2pid{"/Electron/chi2pid",
            "Electron #chi^{2}_{PID};#chi^{2};Events", -10, 10, 200};
        framework::Hist2<> p_vs_theta{"/Electron/p_vs_theta",
            "p vs #theta;#theta [deg];p [GeV/c]", 0, 180, 180, 0, 12, 120};
        framework::Hist2<> phi_vs_theta{"/Electron/phi_vs_theta",
            "#phi vs #theta;#theta [deg];#phi [deg]", 0, 180, 180, -180, 180, 180};
    } h_electron;

    struct {
        framework::Hist2<> lu_vs_lv_pcal{"/Calorimeter/lu_vs_lv_pcal",
            "PCAL lu vs lv;lv;lu", 0, 500, 100, 0, 500, 100};
        framework::Hist2<> lu_vs_lw_pcal{"/Calorimeter/lu_vs_lw_pcal",
            "PCAL lu vs lw;lw;lu", 0, 500, 100, 0, 500, 100};
        framework::Hist2<> lu_vs_lv_inner{"/Calorimeter/lu_vs_lv_inner",
            "Inner EC lu vs lv;lv;lu", 0, 500, 100, 0, 500, 100};
        framework::Hist2<> lu_vs_lw_inner{"/Calorimeter/lu_vs_lw_inner",
            "Inner EC lu vs lw;lw;lu", 0, 500, 100, 0, 500, 100};
        framework::Hist2<> lu_vs_lv_outer{"/Calorimeter/lu_vs_lv_outer",
            "Outer EC lu vs lv;lv;lu", 0, 500, 100, 0, 500, 100};
        framework::Hist2<> lu_vs_lw_outer{"/Calorimeter/lu_vs_lw_outer",
            "Outer EC lu vs lw;lw;lu", 0, 500, 100, 0, 500, 100};
    } h_calorimeter;

    // DC chi2/ndf vs edge, sliced on momentum and on theta.
    // Each TH3D replaces a stack of per-bin 2D histograms; project at
    // analysis time via TH3::ProjectionYZ(binlo, binhi).
    // DC chi2/ndf vs edge, sliced on momentum and on theta. The first axis
    // (momentum or theta) is binned with the algorithm's runtime config; we
    // use a wide default range here and the booking step rebuilds each
    // histogram with the correct edges.
    struct DcHistograms {
        std::unique_ptr<framework::H3<>> chi2ndf_vs_edge_vs_p_r1;
        std::unique_ptr<framework::H3<>> chi2ndf_vs_edge_vs_p_r2;
        std::unique_ptr<framework::H3<>> chi2ndf_vs_edge_vs_p_r3;
        std::unique_ptr<framework::H3<>> chi2ndf_vs_edge_vs_theta_r1;
        std::unique_ptr<framework::H3<>> chi2ndf_vs_edge_vs_theta_r2;
        std::unique_ptr<framework::H3<>> chi2ndf_vs_edge_vs_theta_r3;
    };
    DcHistograms h_dc;

    struct {
        framework::Hist2<> vz_vs_theta{"/Electron/vz_vs_theta",
            "Electron v_{z} vs #theta;#theta [deg];v_{z} [cm]", 0, 40, 100, -13, 0, 100};
        framework::Hist2<> vz_vs_p{"/Electron/vz_vs_p",
            "Electron v_{z} vs p;p [GeV/c];v_{z} [cm]", 0, 10, 100, -13, 0, 100};
        framework::Hist3<> vz_vs_theta_vs_p{"/Electron/vz_vs_theta_vs_p",
            "Electron v_{z} vs #theta vs p;p [GeV/c];#theta [deg];v_{z} [cm]",
            0, 10, 20, 0, 40, 100, -13, 0, 100};
        // Same set of 2D / 3D projections but on the corrected vz axis.
        framework::Hist2<> vz_corr_vs_theta{"/Electron/vz_corr_vs_theta",
            "Electron corrected v_{z} vs #theta;#theta [deg];v_{z}^{corr} [cm]",
            0, 40, 100, -13, 0, 100};
        framework::Hist2<> vz_corr_vs_p{"/Electron/vz_corr_vs_p",
            "Electron corrected v_{z} vs p;p [GeV/c];v_{z}^{corr} [cm]",
            0, 10, 100, -13, 0, 100};
        framework::Hist3<> vz_corr_vs_theta_vs_p{"/Electron/vz_corr_vs_theta_vs_p",
            "Electron corrected v_{z} vs #theta vs p;p [GeV/c];#theta [deg];v_{z}^{corr} [cm]",
            0, 10, 20, 0, 40, 100, -13, 0, 100};
    } h_electron_vz;

    // Track histograms structure (for neg/pos tracks, pions, etc.)
    // Each histogram is stored per sector (1-6) via PerSectorHist handles.
    struct TrackHistograms {
        // Calorimeter (per sector)
        framework::PerSectorHist1D<6> nphe;
        framework::PerSectorHist2D<6> EPCal_vs_ECal;
        framework::PerSectorHist1D<6> EPCal;
        framework::PerSectorHist2D<6> EPCal_vs_ECal_cut;
        framework::PerSectorHist1D<6> EPCal_cut;
        framework::PerSectorHist2D<6> Etot_vs_SF;
        framework::PerSectorHist2D<6> Etot_vs_SF_cut;
        framework::PerSectorHist2D<6> p_vs_SF;
        framework::PerSectorHist2D<6> lv_vs_SF;
        framework::PerSectorHist2D<6> lw_vs_SF;
        framework::PerSectorHist2D<6> lu_vs_SF_low;
        framework::PerSectorHist2D<6> lu_vs_SF_high;
        framework::PerSectorHist2D<6> lu_vs_SF_low_with_cut;
        framework::PerSectorHist2D<6> lu_vs_SF_high_with_cut;
        // DC fiducial (per sector)
        framework::PerSectorHist2D<6> edge_vs_theta_r1;
        framework::PerSectorHist2D<6> edge_vs_theta_r2;
        framework::PerSectorHist2D<6> edge_vs_theta_r3;
        framework::PerSectorHist2D<6> edge_vs_chi2ndf_r1;
        framework::PerSectorHist2D<6> edge_vs_chi2ndf_r2;
        framework::PerSectorHist2D<6> edge_vs_chi2ndf_r3;
    };

    TrackHistograms h_neg_tracks;
    TrackHistograms h_pos_tracks;
    TrackHistograms h_electron_tracks;

    // Histograms for each cut stage (lock-free atomic)
    struct CutStageHistograms {
        bool booked = false;
        framework::H1<> momentum{mh::regular(0, 12, 120)};
        framework::H1<> theta{mh::regular(0, 180, 180)};
        framework::H1<> phi{mh::regular(-180, 180, 180)};
        framework::H1<> vz{mh::regular(-20, 20, 200)};
        // VzCorrector-corrected vz, same binning as raw. Falls back to
        // raw vz when no corrector variant is loaded so the histogram
        // is always populated.
        framework::H1<> vz_corr{mh::regular(-20, 20, 200)};
        framework::H2<> sampling_fraction{mh::regular(0, 12, 120), mh::regular(0, 0.5, 100)};
        framework::H1<> dc_edge_r1{mh::regular(0, 50, 100)};
    };

    std::map<std::string, CutStageHistograms> m_cut_stage_histograms;

    auto book_histograms() -> void {
        const int p_nbins = static_cast<int>((m_config.p_bin_max - m_config.p_bin_min) / m_config.p_bin_width);
        const int theta_nbins = static_cast<int>((m_config.theta_bin_max - m_config.theta_bin_min) / m_config.theta_bin_width);

        auto book_dc_3d = [&](std::unique_ptr<framework::H3<>>& slot,
                              const std::string& path, const std::string& title,
                              int xn, double xlo, double xhi) {
            slot = std::make_unique<framework::H3<>>(
                mh::regular(xlo, xhi, xn),
                mh::regular(0.0, 50.0, 200),  // edge
                mh::regular(0.0, 10.0, 200)   // chi2/ndf
            );
            m_hist->register_hist(path, title, *slot);
        };

        book_dc_3d(h_dc.chi2ndf_vs_edge_vs_p_r1, "/DC/chi2ndf_vs_edge_vs_p_r1", "Region 1;p [GeV/c];edge [cm];#chi^{2}/ndf", p_nbins, m_config.p_bin_min, m_config.p_bin_max);
        book_dc_3d(h_dc.chi2ndf_vs_edge_vs_p_r2, "/DC/chi2ndf_vs_edge_vs_p_r2", "Region 2;p [GeV/c];edge [cm];#chi^{2}/ndf", p_nbins, m_config.p_bin_min, m_config.p_bin_max);
        book_dc_3d(h_dc.chi2ndf_vs_edge_vs_p_r3, "/DC/chi2ndf_vs_edge_vs_p_r3", "Region 3;p [GeV/c];edge [cm];#chi^{2}/ndf", p_nbins, m_config.p_bin_min, m_config.p_bin_max);

        book_dc_3d(h_dc.chi2ndf_vs_edge_vs_theta_r1, "/DC/chi2ndf_vs_edge_vs_theta_r1", "Region 1;#theta [deg];edge [cm];#chi^{2}/ndf", theta_nbins, m_config.theta_bin_min, m_config.theta_bin_max);
        book_dc_3d(h_dc.chi2ndf_vs_edge_vs_theta_r2, "/DC/chi2ndf_vs_edge_vs_theta_r2", "Region 2;#theta [deg];edge [cm];#chi^{2}/ndf", theta_nbins, m_config.theta_bin_min, m_config.theta_bin_max);
        book_dc_3d(h_dc.chi2ndf_vs_edge_vs_theta_r3, "/DC/chi2ndf_vs_edge_vs_theta_r3", "Region 3;#theta [deg];edge [cm];#chi^{2}/ndf", theta_nbins, m_config.theta_bin_min, m_config.theta_bin_max);

        // Book track histograms for all track categories
        book_track_histograms(h_neg_tracks, "NegativeTracks");
        book_track_histograms(h_pos_tracks, "PositiveTracks");
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

        m_hist->register_hist(base + "/p", fmt::format("Electron p after {};p [GeV/c];Events", cut_name), cut_hists.momentum);
        m_hist->register_hist(base + "/theta", fmt::format("Electron #theta after {};#theta [deg];Events", cut_name), cut_hists.theta);
        m_hist->register_hist(base + "/phi", fmt::format("Electron #phi after {};#phi [deg];Events", cut_name), cut_hists.phi);
        m_hist->register_hist(base + "/vz", fmt::format("Electron v_{{z}} after {};v_{{z}} [cm];Events", cut_name), cut_hists.vz);
        m_hist->register_hist(base + "/vz_corr",
                              fmt::format("Electron corrected v_{{z}} after {};v_{{z}}^{{corr}} [cm];Events", cut_name),
                              cut_hists.vz_corr);
        m_hist->register_hist(base + "/sf_vs_p", fmt::format("SF vs p after {};p [GeV/c];E/p", cut_name), cut_hists.sampling_fraction);
        m_hist->register_hist(base + "/dc_edge_r1", fmt::format("DC Edge R1 after {};edge [cm];Events", cut_name), cut_hists.dc_edge_r1);
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

        cut_hists.momentum.fill(can.particle.p());
        cut_hists.theta.fill(can.particle.theta());
        cut_hists.phi.fill(can.particle.phi());
        cut_hists.vz.fill(can.particle.vz());
        const double vz_corrected = m_electron_cuts->corrected_vz(
            can.particle.vz(), can.particle.p(),
            can.particle.theta(), can.particle.phi(), can.sector);
        cut_hists.vz_corr.fill(vz_corrected);
        cut_hists.sampling_fraction.fill(can.particle.p(), can.sampling_fraction);
        cut_hists.dc_edge_r1.fill(can.traj.region1_edge);
    }

    /**
     * @brief Books histograms for a track category with per-sector granularity
     */
    auto book_track_histograms(TrackHistograms& h, const std::string& category) -> void {
        framework::SectorHistogramGroup<6> g(m_hist.get(), fmt::format("/Tracks/{}", category));

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
    auto process_tracks(const std::vector<Particle>& tracks, TrackHistograms& histograms) -> void {
        for (const auto& track : tracks) {
            CalorimeterBank calo = track.calo();
            CherenkovBank cherenkov = track.cherenkov();
            DCTrajBank traj = track.traj();
            const auto [trk_r1, trk_r2, trk_r3] = track.region_tracks();

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

    auto fill_final_histograms(const Particle& electron, const CalorimeterBank& calo, const DCTrajBank& traj, const TrackBank& track) -> void {
        // Fill thread-safe electron kinematics histograms
        h_electron.momentum.fill(electron.p());
        h_electron.theta.fill(electron.theta());
        h_electron.phi.fill(electron.phi());
        h_electron.vz.fill(electron.vz());
        // Cached corrected vz used by both the 1D plot here and the
        // 2D/3D projections below. Returns the raw vz when no corrector
        // variant is loaded (LD2 path).
        const double vz_corrected = m_electron_cuts->corrected_vz(
            electron.vz(), electron.p(), electron.theta(),
            electron.phi(), calo.pcal_sector);
        h_electron.vz_corr.fill(vz_corrected);
        h_electron.chi2pid.fill(electron.chi2pid());
        h_electron.p_vs_theta.fill(electron.theta(), electron.p());
        h_electron.phi_vs_theta.fill(electron.theta(), electron.phi());

        const double e_vz = electron.vz();
        const double e_theta = electron.theta();
        const double e_p = electron.p();
        h_electron_vz.vz_vs_theta.fill(e_theta, e_vz);
        h_electron_vz.vz_vs_p.fill(e_p, e_vz);
        h_electron_vz.vz_vs_theta_vs_p.fill(e_p, e_theta, e_vz);
        h_electron_vz.vz_corr_vs_theta.fill(e_theta, vz_corrected);
        h_electron_vz.vz_corr_vs_p.fill(e_p, vz_corrected);
        h_electron_vz.vz_corr_vs_theta_vs_p.fill(e_p, e_theta, vz_corrected);

        // Fill calorimeter - PCAL
        h_calorimeter.lu_vs_lv_pcal.fill(calo.pcal_lv, calo.pcal_lu);
        h_calorimeter.lu_vs_lw_pcal.fill(calo.pcal_lw, calo.pcal_lu);

        // Fill calorimeter - Inner EC
        h_calorimeter.lu_vs_lv_inner.fill(calo.inner_lv, calo.inner_lu);
        h_calorimeter.lu_vs_lw_inner.fill(calo.inner_lw, calo.inner_lu);

        // Fill calorimeter - Outer EC
        h_calorimeter.lu_vs_lv_outer.fill(calo.outer_lv, calo.outer_lu);
        h_calorimeter.lu_vs_lw_outer.fill(calo.outer_lw, calo.outer_lu);

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

}  // namespace algorithms
