#pragma once

#include <array>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string>
#include <clas12/Constants.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_CovMat.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/CandidateUtils.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SectorHistograms.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for PionAnalysisAlg.
struct PionAnalysisConfig {
    Target target = Target::LD2;
    Polarity polarity = Polarity::Outbending;

    double min_momentum = 0.3;
    double min_calo_energy = 0.01;
    bool enable_forward_detector_cut = true;
    bool enable_momentum_cut = true;
    bool enable_calorimeter_cut = true;
    bool enable_dc_track_cut = true;

    static constexpr auto config_fields() {
        using T = PionAnalysisConfig;
        return std::make_tuple(
            framework::field("min_momentum", &T::min_momentum),
            framework::field("min_calo_energy", &T::min_calo_energy),
            framework::field("enable_forward_detector_cut", &T::enable_forward_detector_cut),
            framework::field("enable_momentum_cut", &T::enable_momentum_cut),
            framework::field("enable_calorimeter_cut", &T::enable_calorimeter_cut),
            framework::field("enable_dc_track_cut", &T::enable_dc_track_cut));
    }
};

/**
 * @brief Pion analysis with calorimeter SF and DC quality histograms
 */
class PionAnalysisAlg : public framework::Algorithm {
   public:
    using Config = PionAnalysisConfig;
    using StatusCode = framework::StatusCode;

    explicit PionAnalysisAlg(std::string name = "PionAnalysis", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow_piplus("π⁺ Analysis"), m_cutflow_piminus("π⁻ Analysis") {
        required_services(m_hist, m_ntuple);
        declare_banks("REC::Event", "REC::Particle", "REC::Calorimeter", "REC::Traj", "REC::Track", "REC::Scintillator");
        // REC::CovMat (5x5 DC Kalman state cov: x, y, tx, ty, q/p) feeds the
        // slope/angle resolution histograms; optional so files without it
        // (e.g. some MC skims) just leave those plots empty.
        declare_optional_bank("REC::CovMat");
        declare_config(m_config);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing pion analysis");
        Log::info(name(), "Target: {}, Polarity: {}", to_string(m_config.target), to_string(m_config.polarity));
        Log::info(name(), "Min momentum: {} GeV, Min calo energy: {} GeV", m_config.min_momentum, m_config.min_calo_energy);

        for (auto* cf : {&m_cutflow_piplus, &m_cutflow_piminus}) {
            cf->add_cut("All Pions");
            cf->add_cut("In the forward detector");
            cf->add_cut("Momentum > 0.3 GeV");
            cf->add_cut("Has Calorimeter");
            cf->add_cut("Has DC Track");
        }

        book_histograms();

        m_ntuple_piplus.init(m_ntuple.get(), "pion_plus_analysis");
        m_ntuple_piminus.init(m_ntuple.get(), "pion_minus_analysis");
        Log::info(name(), "N-tuple branches registered for pi+ and pi-");

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(framework::Decisions::HasGoodElectron)) return StatusCode::FILTERED();
        auto electron_opt = event.retrieve_typed(framework::Keys::TriggerElectron);
        if (!electron_opt) return StatusCode::FILTERED();
        const Particle& electron = *electron_opt;

        bankdefs::REC_Event rec_event;
        rec_event.bind(event.get_bank("REC::Event"));
        if (rec_event.rows() == 0) return StatusCode::FILTERED();

        const double start_time = rec_event.startTime(0);

        for (const auto& pion : event.pions_plus()) process_pion(event, pion, electron, start_time, true);
        for (const auto& pion : event.pions_minus()) process_pion(event, pion, electron, start_time, false);

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Finalizing pion analysis");

        m_cutflow_piplus.print();
        m_cutflow_piminus.print();

        Log::info(name(), "Pion Analysis Summary:");
        Log::info(name(), "     - π⁺ with calorimeter info: {}", m_cutflow_piplus.get_count("Has Calorimeter"));
        Log::info(name(), "     - π⁻ with calorimeter info: {}", m_cutflow_piminus.get_count("Has Calorimeter"));

        if (m_ntuple) {
            Log::info(name(), "     - π⁺ N-tuple entries: {}", m_ntuple->get_entries("pion_plus_analysis"));
            Log::info(name(), "     - π⁻ N-tuple entries: {}", m_ntuple->get_entries("pion_minus_analysis"));
        }

        report_momentum_scan("π⁺", m_pscan_piplus);
        report_momentum_scan("π⁻", m_pscan_piminus);

        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;

    framework::Cutflow m_cutflow_piplus;
    framework::Cutflow m_cutflow_piminus;

    // Momentum-cut impact scan. For each candidate p_min threshold, count
    // how many forward-detector pions would be removed (p < threshold) so
    // the analyst can read off the cost of each momentum cut from the
    // finalize summary without re-running. The denominator is every
    // forward-detector pion, counted before any momentum filtering, so the
    // numbers are independent of the active min_momentum cut.
    static constexpr std::array<double, 4> kMomentumThresholds{0.3, 0.5, 0.7, 1.0};

    struct MomentumScan {
        std::atomic<long>                total{0};   // forward-detector pions seen
        std::array<std::atomic<long>, 4> lost{};     // count with p < threshold[i]

        void record(double p) noexcept {
            total.fetch_add(1, std::memory_order_relaxed);
            for (std::size_t i = 0; i < kMomentumThresholds.size(); ++i) {
                if (p < kMomentumThresholds[i]) {
                    lost[i].fetch_add(1, std::memory_order_relaxed);
                }
            }
        }
    };
    MomentumScan m_pscan_piplus;
    MomentumScan m_pscan_piminus;

    // Guards the one-time REC::CovMat schema (5x5 vs 6x6) warning.
    mutable std::atomic<bool> m_covmat_schema_checked{false};

    /// Print, per momentum threshold, how many forward-detector pions a cut
    /// at that p_min would remove (and retain), as an absolute count and a
    /// fraction of the forward-detector sample.
    auto report_momentum_scan(const char* label, const MomentumScan& scan) -> void {
        const long total = scan.total.load(std::memory_order_relaxed);
        Log::info(name(),
                  "     - {} momentum-cut impact (forward-detector pions = {}):",
                  label, total);
        for (std::size_t i = 0; i < kMomentumThresholds.size(); ++i) {
            const long lost = scan.lost[i].load(std::memory_order_relaxed);
            const long kept = total - lost;
            const double pct = total > 0 ? 100.0 * static_cast<double>(lost) / total : 0.0;
            Log::info(name(),
                      "         p < {:.1f} GeV -> lost {} ({:.2f}%), kept {}",
                      kMomentumThresholds[i], lost, pct, kept);
        }
    }

framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple;

    struct PionCandidate {
        Particle pion;
        CalorimeterBank calorimeter;
        DCTrajBank traj;
        TrackBank track;
        double E_tot = 0.0;
        double sampling_fraction = 0.0;
        double chi2ndf = 0.0;
        double delta_t = 0.0;
        double tof_path = 0.0;
        int sector = 0;
    };

    struct PionNTupleData {
        double pi_p = 0.0;
        double pi_theta = 0.0;
        double pi_phi = 0.0;
        double pi_vz = 0.0;
        double pi_chi2pid = 0.0;
        double pi_beta = 0.0;
        double pi_sampling_fraction = 0.0;
        int pi_sector = 0;
        double pi_chi2ndf = 0.0;
        double pi_edge_r1 = 0.0;
        double pi_edge_r2 = 0.0;
        double pi_edge_r3 = 0.0;
        double pi_x_r1 = 0.0;
        double pi_y_r1 = 0.0;
        double pi_z_r1 = 0.0;
        double pi_x_r2 = 0.0;
        double pi_y_r2 = 0.0;
        double pi_z_r2 = 0.0;
        double pi_delta_t = 0.0;
        double pi_tof_path = 0.0;

        auto from(const PionCandidate& c) -> void {
            pi_p = c.pion.p();
            pi_theta = c.pion.theta();
            pi_phi = c.pion.phi();
            pi_vz = c.pion.vz();
            pi_chi2pid = c.pion.chi2pid();
            pi_beta = c.pion.beta();
            pi_sampling_fraction = c.sampling_fraction;
            pi_sector = c.sector;
            pi_chi2ndf = c.chi2ndf;
            pi_edge_r1 = c.traj.region1_edge;
            pi_edge_r2 = c.traj.region2_edge;
            pi_edge_r3 = c.traj.region3_edge;
            pi_delta_t = c.delta_t;
            pi_tof_path = c.tof_path;
        }

        static constexpr auto ntuple_fields() {
            using T = PionNTupleData;
            return std::make_tuple(framework::field("pi_p", &T::pi_p), framework::field("pi_theta", &T::pi_theta), framework::field("pi_phi", &T::pi_phi), framework::field("pi_vz", &T::pi_vz), framework::field("pi_chi2pid", &T::pi_chi2pid), framework::field("pi_beta", &T::pi_beta), framework::field("pi_sampling_fraction", &T::pi_sampling_fraction), framework::field("pi_sector", &T::pi_sector), framework::field("pi_chi2ndf", &T::pi_chi2ndf), framework::field("pi_edge_r1", &T::pi_edge_r1), framework::field("pi_edge_r2", &T::pi_edge_r2), framework::field("pi_edge_r3", &T::pi_edge_r3), framework::field("pi_x_r1", &T::pi_x_r1), framework::field("pi_y_r1", &T::pi_y_r1), framework::field("pi_z_r1", &T::pi_z_r1), framework::field("pi_x_r2", &T::pi_x_r2), framework::field("pi_y_r2", &T::pi_y_r2), framework::field("pi_z_r2", &T::pi_z_r2), framework::field("pi_delta_t", &T::pi_delta_t), framework::field("pi_tof_path", &T::pi_tof_path));
        }
    };

    framework::NTupleWriter<PionNTupleData> m_ntuple_piplus;
    framework::NTupleWriter<PionNTupleData> m_ntuple_piminus;

    struct PionHistograms {
        framework::H1<> p{mh::regular(0.0, 10.0, 100)};
        framework::H1<> theta{mh::regular(0.0, 40.0, 180)};
        framework::H1<> phi{mh::regular(-180.0, 180.0, 360)};
        framework::H1<> vz{mh::regular(-40.0, 40.0, 200)};
        framework::H1<> chi2pid{mh::regular(-10.0, 10.0, 200)};
        framework::H1<> beta{mh::regular(0.5, 1.1, 100)};

        // Covariance-derived track resolutions from REC::CovMat (5x5 DC
        // Kalman state x, y, tx, ty, q/p): sigma(tx)=sqrt(C33),
        // sigma(ty)=sqrt(C44), sigma(theta) from (C33,C44,C34) propagated
        // through the (theta) Jacobian at the lab direction [mrad]. The vs-p
        // 2D views are the useful ones — they show how the slope/angle
        // resolution degrades at low momentum.
        framework::H1<> sigma_tx{mh::regular(0.0, 0.02, 200)};
        framework::H1<> sigma_ty{mh::regular(0.0, 0.02, 200)};
        framework::H1<> sigma_theta{mh::regular(0.0, 10.0, 200)};  // mrad
        framework::H2<> sigma_tx_vs_p{mh::regular(0, 10, 100), mh::regular(0.0, 0.02, 200)};
        framework::H2<> sigma_ty_vs_p{mh::regular(0, 10, 100), mh::regular(0.0, 0.02, 200)};
        framework::H2<> sigma_theta_vs_p{mh::regular(0, 10, 100), mh::regular(0.0, 10.0, 200)};

        framework::H2<> vz_vs_phi{mh::regular(-180, 180, 180), mh::regular(-40, 40, 100)};
        framework::H2<> vz_vs_sector{mh::regular(0.5, 6.5, 6), mh::regular(-40, 40, 100)};
        framework::H2<> vz_vs_p_2d{mh::regular(0, 10, 100), mh::regular(-40, 40, 100)};
        framework::H2<> vx_vs_vy{mh::regular(-2, 2, 100), mh::regular(-2, 2, 100)};
        framework::H1<> delta_vz{mh::regular(-20, 20, 200)};
        framework::H2<> delta_vz_vs_p{mh::regular(0, 10, 100), mh::regular(-20, 20, 200)};

        framework::H1<> delta_t{mh::regular(-2, 2, 200)};
        framework::H2<> delta_t_vs_p_diag{mh::regular(0, 10, 100), mh::regular(-2, 2, 200)};
        framework::H1<> tof_mass2{mh::regular(-0.2, 0.5, 200)};
        framework::H2<> tof_mass2_vs_p{mh::regular(0, 10, 100), mh::regular(-0.2, 0.5, 200)};
        framework::H1<> tof_path{mh::regular(0, 800, 100)};
        framework::H2<> tof_path_vs_p{mh::regular(0, 10, 100), mh::regular(0, 800, 100)};
        framework::H2<> beta_vs_p{mh::regular(0, 10, 100), mh::regular(0.5, 1.1, 100)};
        framework::H1<> beta_diff{mh::regular(-0.1, 0.1, 200)};
        framework::H2<> beta_diff_vs_p{mh::regular(0, 10, 100), mh::regular(-0.1, 0.1, 200)};

        framework::H2<> chi2pid_vs_p{mh::regular(0, 10, 100), mh::regular(-10, 10, 100)};
        framework::H2<> chi2pid_vs_theta{mh::regular(0, 45, 90), mh::regular(-10, 10, 100)};
        framework::H2<> chi2pid_vs_sector{mh::regular(0.5, 6.5, 6), mh::regular(-10, 10, 100)};

        framework::H2<> phi_vs_theta{mh::regular(0, 45, 90), mh::regular(-180, 180, 180)};
        framework::H2<> p_vs_phi{mh::regular(-180, 180, 180), mh::regular(0, 10, 100)};
        framework::H2<> p_vs_theta_diag{mh::regular(0, 45, 90), mh::regular(0, 10, 100)};

        framework::H1<> sf{mh::regular(0, 0.4, 100)};
        framework::H1<> lv{mh::regular(0, 500, 100)};
        framework::H1<> lw{mh::regular(0, 500, 100)};
        framework::H2<> sf_vs_lv{mh::regular(0, 70, 200), mh::regular(0, 0.4, 200)};
        framework::H2<> sf_vs_lw{mh::regular(0, 70, 200), mh::regular(0, 0.4, 200)};

        framework::H1<> chi2ndf{mh::regular(0, 20, 100)};
        framework::H1<> edge_r1{mh::regular(-10, 50, 100)};
        framework::H1<> edge_r2{mh::regular(-10, 50, 100)};
        framework::H1<> edge_r3{mh::regular(-10, 50, 100)};
        framework::H2<> chi2ndf_vs_edge_r1{mh::regular(0, 50, 200), mh::regular(0, 100, 200)};
        framework::H2<> chi2ndf_vs_edge_r2{mh::regular(0, 50, 200), mh::regular(0, 100, 200)};
        framework::H2<> chi2ndf_vs_edge_r3{mh::regular(0, 50, 200), mh::regular(0, 100, 200)};

        framework::H1<> edge_r1_precut{mh::regular(-10, 50, 100)};
        framework::H1<> edge_r2_precut{mh::regular(-10, 50, 100)};
        framework::H1<> edge_r3_precut{mh::regular(-10, 50, 100)};
        framework::H2<> chi2ndf_vs_edge_r1_precut{mh::regular(0, 50, 200), mh::regular(0, 100, 200)};
        framework::H2<> chi2ndf_vs_edge_r2_precut{mh::regular(0, 50, 200), mh::regular(0, 100, 200)};
        framework::H2<> chi2ndf_vs_edge_r3_precut{mh::regular(0, 50, 200), mh::regular(0, 100, 200)};

        // delta_t vs p in the forward detector, before the momentum /
        // calorimeter / DC cuts — inclusive + per sector (Forward/ subdir).
        framework::H2<> delta_t_vs_p_precut{mh::regular(0, 10, 100), mh::regular(-2, 2, 200)};
        framework::PerSectorHist2D<6> delta_t_vs_p_precut_sector;

        framework::PerSectorHist2D<6> delta_t_vs_p_sector;
        framework::PerSectorHist2D<6> p_vs_theta_sector;

        framework::PerSectorHist1D<6> nphe;
        framework::PerSectorHist2D<6> EPCal_vs_ECal;
        framework::PerSectorHist1D<6> EPCal;
        framework::PerSectorHist2D<6> p_vs_SF;
        framework::PerSectorHist2D<6> lv_vs_SF;
        framework::PerSectorHist2D<6> lw_vs_SF;
        framework::PerSectorHist2D<6> edge_vs_chi2ndf_r1;
        framework::PerSectorHist2D<6> edge_vs_chi2ndf_r2;
        framework::PerSectorHist2D<6> edge_vs_chi2ndf_r3;
        framework::PerSectorHist2D<6> p_vs_theta;
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    auto evaluate_cutflow(bool in_forward_detector, double p, double E_tot, bool has_dc, framework::Cutflow& cf) const -> bool {
        cf.pass("All Pions");
        if (m_config.enable_forward_detector_cut && !in_forward_detector) return false;
        cf.pass("In the forward detector");
        if (m_config.enable_momentum_cut && p < m_config.min_momentum) return false;
        cf.pass("Momentum > 0.3 GeV");
        if (m_config.enable_calorimeter_cut && E_tot <= m_config.min_calo_energy) return false;
        cf.pass("Has Calorimeter");
        if (m_config.enable_dc_track_cut && !has_dc) return false;
        cf.pass("Has DC Track");
        return true;
    }

    auto book_histograms() -> void {
        book_pion_histograms("/PionAnalysis/PiPlus", h_piplus, "#pi^{+}");
        book_pion_histograms("/PionAnalysis/PiMinus", h_piminus, "#pi^{-}");
    }

    auto book_pion_histograms(const std::string& dir, PionHistograms& h, const std::string& label) -> void {
        auto* svc = m_hist.get();
        svc->register_hist(dir + "/p", fmt::format("{} Momentum;p [GeV/c];Events", label), h.p);        svc->register_hist(dir + "/theta", fmt::format("{} #theta;#theta [deg];Events", label), h.theta);        svc->register_hist(dir + "/phi", fmt::format("{} #phi;#phi [deg];Events", label), h.phi);        svc->register_hist(dir + "/vz", fmt::format("{} v_{{z}};v_{{z}} [cm];Events", label), h.vz);        svc->register_hist(dir + "/chi2pid", fmt::format("{} #chi^{{2}}_{{PID}};#chi^{{2}}_{{PID}};Events", label), h.chi2pid);        svc->register_hist(dir + "/beta", fmt::format("{} #beta;#beta;Events", label), h.beta);
        svc->register_hist(dir + "/sigma_tx", fmt::format("{} #sigma(t_{{x}});#sigma(t_{{x}});Tracks", label), h.sigma_tx);        svc->register_hist(dir + "/sigma_ty", fmt::format("{} #sigma(t_{{y}});#sigma(t_{{y}});Tracks", label), h.sigma_ty);        svc->register_hist(dir + "/sigma_theta", fmt::format("{} #sigma(#theta);#sigma(#theta) [mrad];Tracks", label), h.sigma_theta);        svc->register_hist(dir + "/sigma_tx_vs_p", fmt::format("{} #sigma(t_{{x}}) vs p;p [GeV/c];#sigma(t_{{x}})", label), h.sigma_tx_vs_p);        svc->register_hist(dir + "/sigma_ty_vs_p", fmt::format("{} #sigma(t_{{y}}) vs p;p [GeV/c];#sigma(t_{{y}})", label), h.sigma_ty_vs_p);        svc->register_hist(dir + "/sigma_theta_vs_p", fmt::format("{} #sigma(#theta) vs p;p [GeV/c];#sigma(#theta) [mrad]", label), h.sigma_theta_vs_p);
        svc->register_hist(dir + "/vz_vs_phi", fmt::format("{} v_{{z}} vs #phi;#phi [deg];v_{{z}} [cm]", label), h.vz_vs_phi);        svc->register_hist(dir + "/vz_vs_sector", fmt::format("{} v_{{z}} vs Sector;Sector;v_{{z}} [cm]", label), h.vz_vs_sector);        svc->register_hist(dir + "/vz_vs_p", fmt::format("{} v_{{z}} vs p;p [GeV/c];v_{{z}} [cm]", label), h.vz_vs_p_2d);        svc->register_hist(dir + "/vx_vs_vy", fmt::format("{} v_{{x}} vs v_{{y}};v_{{x}} [cm];v_{{y}} [cm]", label), h.vx_vs_vy);        svc->register_hist(dir + "/delta_vz", fmt::format("{} #Deltav_{{z}} (#pi - e);#Deltav_{{z}} [cm];Events", label), h.delta_vz);        svc->register_hist(dir + "/delta_vz_vs_p", fmt::format("{} #Deltav_{{z}} vs p;p [GeV/c];#Deltav_{{z}} [cm]", label), h.delta_vz_vs_p);
        svc->register_hist(dir + "/delta_t", fmt::format("{} #Deltat;#Deltat [ns];Events", label), h.delta_t);        svc->register_hist(dir + "/delta_t_vs_p_diag", fmt::format("{} #Deltat vs p;p [GeV/c];#Deltat [ns]", label), h.delta_t_vs_p_diag);        svc->register_hist(dir + "/tof_mass2", fmt::format("{} m^{{2}}_{{TOF}};m^{{2}} [GeV^{{2}}];Events", label), h.tof_mass2);        svc->register_hist(dir + "/tof_mass2_vs_p", fmt::format("{} m^{{2}}_{{TOF}} vs p;p [GeV/c];m^{{2}} [GeV^{{2}}]", label), h.tof_mass2_vs_p);        svc->register_hist(dir + "/tof_path", fmt::format("{} TOF Path Length;Path [cm];Events", label), h.tof_path);        svc->register_hist(dir + "/tof_path_vs_p", fmt::format("{} TOF Path vs p;p [GeV/c];Path [cm]", label), h.tof_path_vs_p);        svc->register_hist(dir + "/beta_vs_p", fmt::format("{} #beta vs p;p [GeV/c];#beta", label), h.beta_vs_p);        svc->register_hist(dir + "/beta_diff", fmt::format("{} #Delta#beta;#Delta#beta;Events", label), h.beta_diff);        svc->register_hist(dir + "/beta_diff_vs_p", fmt::format("{} #Delta#beta vs p;p [GeV/c];#Delta#beta", label), h.beta_diff_vs_p);
        svc->register_hist(dir + "/chi2pid_vs_p", fmt::format("{} #chi^{{2}}_{{PID}} vs p;p [GeV/c];#chi^{{2}}_{{PID}}", label), h.chi2pid_vs_p);        svc->register_hist(dir + "/chi2pid_vs_theta", fmt::format("{} #chi^{{2}}_{{PID}} vs #theta;#theta [deg];#chi^{{2}}_{{PID}}", label), h.chi2pid_vs_theta);        svc->register_hist(dir + "/chi2pid_vs_sector", fmt::format("{} #chi^{{2}}_{{PID}} vs Sector;Sector;#chi^{{2}}_{{PID}}", label), h.chi2pid_vs_sector);
        svc->register_hist(dir + "/phi_vs_theta", fmt::format("{} #phi vs #theta;#theta [deg];#phi [deg]", label), h.phi_vs_theta);        svc->register_hist(dir + "/p_vs_phi", fmt::format("{} p vs #phi;#phi [deg];p [GeV/c]", label), h.p_vs_phi);        svc->register_hist(dir + "/p_vs_theta_diag", fmt::format("{} p vs #theta;#theta [deg];p [GeV/c]", label), h.p_vs_theta_diag);
        svc->register_hist(dir + "/sf", fmt::format("{} Sampling Fraction;SF;Events", label), h.sf);        svc->register_hist(dir + "/lv", fmt::format("{} PCAL lv;lv [cm];Events", label), h.lv);        svc->register_hist(dir + "/lw", fmt::format("{} PCAL lw;lw [cm];Events", label), h.lw);        svc->register_hist(dir + "/sf_vs_lv", fmt::format("{} Sf vs lv;lv [cm];SF", label), h.sf_vs_lv);        svc->register_hist(dir + "/sf_vs_lw", fmt::format("{} Sf vs lw;lw [cm];SF", label), h.sf_vs_lw);
        svc->register_hist(dir + "/chi2ndf", fmt::format("{} DC #chi^{{2}}/NDF;#chi^{{2}}/NDF;Events", label), h.chi2ndf);        svc->register_hist(dir + "/edge_r1", fmt::format("{} DC Edge R1;Edge [cm];Events", label), h.edge_r1);        svc->register_hist(dir + "/edge_r2", fmt::format("{} DC Edge R2;Edge [cm];Events", label), h.edge_r2);        svc->register_hist(dir + "/edge_r3", fmt::format("{} DC Edge R3;Edge [cm];Events", label), h.edge_r3);        svc->register_hist(dir + "/chi2ndf_vs_edge_r1", fmt::format("{} #chi^{{2}}/NDF vs Edge R1;Edge R1 [cm];#chi^{{2}}/NDF", label), h.chi2ndf_vs_edge_r1);        svc->register_hist(dir + "/chi2ndf_vs_edge_r2", fmt::format("{} #chi^{{2}}/NDF vs Edge R2;Edge R2 [cm];#chi^{{2}}/NDF", label), h.chi2ndf_vs_edge_r2);        svc->register_hist(dir + "/chi2ndf_vs_edge_r3", fmt::format("{} #chi^{{2}}/NDF vs Edge R3;Edge R3 [cm];#chi^{{2}}/NDF", label), h.chi2ndf_vs_edge_r3);
        svc->register_hist(dir + "/edge_r1_precut", fmt::format("{} DC Edge R1 (pre-cut);Edge [cm];Events", label), h.edge_r1_precut);        svc->register_hist(dir + "/edge_r2_precut", fmt::format("{} DC Edge R2 (pre-cut);Edge [cm];Events", label), h.edge_r2_precut);        svc->register_hist(dir + "/edge_r3_precut", fmt::format("{} DC Edge R3 (pre-cut);Edge [cm];Events", label), h.edge_r3_precut);        svc->register_hist(dir + "/chi2ndf_vs_edge_r1_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R1 (pre-cut);Edge R1 [cm];#chi^{{2}}/NDF", label), h.chi2ndf_vs_edge_r1_precut);        svc->register_hist(dir + "/chi2ndf_vs_edge_r2_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R2 (pre-cut);Edge R2 [cm];#chi^{{2}}/NDF", label), h.chi2ndf_vs_edge_r2_precut);        svc->register_hist(dir + "/chi2ndf_vs_edge_r3_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R3 (pre-cut);Edge R3 [cm];#chi^{{2}}/NDF", label), h.chi2ndf_vs_edge_r3_precut);

        framework::SectorHistogramGroup<6> g(svc, dir);
        h.nphe = g.add_hist1D("nphe", "Cherenkov nphe;nphe;Tracks", 51, 0, 50);
        h.EPCal_vs_ECal = g.add_hist2D("EPCal_vs_ECal", "E_{PCal} vs E_{Cal};E_{PCal} [GeV];E_{Cal} [GeV]", 200, 0, 0.6, 200, 0, 0.6);
        h.EPCal = g.add_hist1D("EPCal", "E_{PCal};E_{PCal} [GeV];Tracks", 200, 0, 1);
        h.p_vs_SF = g.add_hist2D("p_vs_SF", "p vs SF;p [GeV];E/p", 200, 0, 11, 200, 0, 0.4);
        h.edge_vs_chi2ndf_r1 = g.add_hist2D("edge_vs_chi2ndf_r1", "Edge vs #chi^{2}/ndf (R1);edge [cm];#chi^{2}/ndf", 200, 0, 50, 200, 0, 10);
        h.edge_vs_chi2ndf_r2 = g.add_hist2D("edge_vs_chi2ndf_r2", "Edge vs #chi^{2}/ndf (R2);edge [cm];#chi^{2}/ndf", 200, 0, 50, 200, 0, 10);
        h.edge_vs_chi2ndf_r3 = g.add_hist2D("edge_vs_chi2ndf_r3", "Edge vs #chi^{2}/ndf (R3);edge [cm];#chi^{2}/ndf", 200, 0, 50, 200, 0, 10);
        h.lv_vs_SF = g.add_hist2D("lv_vs_SF", "lv vs SF;lv;E/p", 200, 0, 70, 200, 0, 0.5);
        h.lw_vs_SF = g.add_hist2D("lw_vs_SF", "lw vs SF;lw;E/p", 200, 0, 70, 200, 0, 0.5);
        h.p_vs_theta = g.add_hist2D("p_vs_theta", fmt::format("{} p vs #theta;#theta [deg];p [GeV/c]", label), 90, 0, 45, 100, 0, 10);
        h.delta_t_vs_p_sector = g.add_hist2D("delta_t_vs_p", fmt::format("{} #Deltat vs p;p [GeV/c];#Deltat [ns]", label), 100, 0, 10, 200, -2, 2);
        h.p_vs_theta_sector = g.add_hist2D("p_vs_theta_acceptance", fmt::format("{} p vs #theta Acceptance;#theta [deg];p [GeV/c]", label), 90, 0, 45, 100, 0, 10);

        // Pre-cut delta_t vs p, in the Forward/ subdirectory: inclusive +
        // per sector (delta_t_vs_p_precut + delta_t_vs_p_precut_S1..S6).
        const std::string fwd = dir + "/Forward";
        svc->register_hist(fwd + "/delta_t_vs_p_precut",
                           fmt::format("{} #Deltat vs p (pre-cut, forward);p [GeV/c];#Deltat [ns]", label),
                           h.delta_t_vs_p_precut);
        framework::SectorHistogramGroup<6> g_fwd(svc, fwd);
        h.delta_t_vs_p_precut_sector = g_fwd.add_hist2D(
            "delta_t_vs_p_precut",
            fmt::format("{} #Deltat vs p (pre-cut, forward);p [GeV/c];#Deltat [ns]", label),
            100, 0, 10, 200, -2, 2);
    }

    // Covariance-derived resolutions from REC::CovMat (5x5 DC Kalman state
    // x, y, tx, ty, q/p at the last superlayer used in the fit). sigma_theta
    // is in milliradians; all fields are NaN when the bank or the matching
    // row is absent.
    struct CovResolutions {
        double sigma_tx    = std::numeric_limits<double>::quiet_NaN();
        double sigma_ty    = std::numeric_limits<double>::quiet_NaN();
        double sigma_theta = std::numeric_limits<double>::quiet_NaN();  // mrad
    };

    /// Port of the referenced Rust `covariance_resolutions`: sigma(tx) =
    /// sqrt(C33), sigma(ty) = sqrt(C44), and sigma(theta) by propagating the
    /// slope covariance (C33, C44, C34) through the theta Jacobian at the lab
    /// direction. Approximate — it ignores the sector-tilt frame rotation, so
    /// it is a *relative* low-p vs high-p resolution measure, not an absolute
    /// calibration.
    auto compute_cov_resolutions(framework::Event& event, const Particle& pion) const
        -> CovResolutions {
        CovResolutions r;

        // One-time schema sanity check. We read C33/C44/C34 by name, which is
        // correct for the 5x5 (x, y, tx, ty, q/p) state where C33=var(tx) etc.
        // Some older files instead carry a 6x6 (x, y, z, theta, phi, p)
        // REC::CovMat (it has a C66 column), where C33=var(z) — there these
        // plots are NOT the slope/angle resolutions. Warn once if we see it.
        if (!m_covmat_schema_checked.exchange(true)) {
            const auto& cov = event.get_bank("REC::CovMat");
            if (cov.getRows() > 0) {
                if (const_cast<hipo::bank&>(cov).getSchema().exists("C66")) {
                    Log::warning(name(),
                                 "REC::CovMat is the 6x6 (x,y,z,theta,phi,p) layout; "
                                 "sigma_tx/ty/theta assume the 5x5 (x,y,tx,ty,q/p) DC "
                                 "state and are NOT physically meaningful on this file");
                }
            } else {
                m_covmat_schema_checked.store(false);  // retry once the bank is populated
            }
        }

        bool found = false;
        double c33 = 0.0, c44 = 0.0, c34 = 0.0;
        for (auto row : event.rows_for_pindex<bankdefs::REC_CovMat>(pion.index())) {
            c33 = row.C33();
            c44 = row.C44();
            c34 = row.C34();
            found = true;
            break;  // pindex is unique
        }
        if (!found) return r;

        r.sigma_tx = std::sqrt(std::max(0.0, c33));
        r.sigma_ty = std::sqrt(std::max(0.0, c44));

        const double px = pion.px(), py = pion.py(), pz = pion.pz();
        if (pz > 0.0 && std::isfinite(c33) && std::isfinite(c44) && std::isfinite(c34)) {
            const double a = px / pz;  // tx
            const double b = py / pz;  // ty
            const double rho2 = a * a + b * b;
            if (rho2 > 1e-9) {
                const double rho = std::sqrt(rho2);
                // theta = atan(rho); d(theta)/d(tx), d(theta)/d(ty).
                const double dth_da = a / (rho * (1.0 + rho2));
                const double dth_db = b / (rho * (1.0 + rho2));
                const double var_th = dth_da * dth_da * c33 +
                                      dth_db * dth_db * c44 +
                                      2.0 * dth_da * dth_db * c34;
                r.sigma_theta = std::sqrt(std::max(0.0, var_th)) * 1000.0;  // mrad
            }
        }
        return r;
    }

    auto process_pion(framework::Event& event, const Particle& pion, const Particle& electron, double start_time, bool is_piplus) -> void {
        auto& cutflow = is_piplus ? m_cutflow_piplus : m_cutflow_piminus;
        auto& h = is_piplus ? h_piplus : h_piminus;
        auto& ntuple = is_piplus ? m_ntuple_piplus : m_ntuple_piminus;

        auto track = pion.track();
        auto traj = pion.traj();
        auto calo = pion.calo();

        double chi2ndf = (track.NDF > 0) ? track.chi2 / track.NDF : 0.0;
        double edge_r1 = traj.region1_edge;
        double edge_r2 = traj.region2_edge;
        double edge_r3 = traj.region3_edge;
        double E_tot = calo.pcal_energy + calo.inner_energy + calo.outer_energy;
        bool in_forward_detector = framework::CandidateUtils::is_forward_detector(pion.status());

        // Timing is computed up front so the pre-cut delta_t-vs-p plots (in
        // the forward detector, before the momentum / calorimeter / DC cuts)
        // and the post-cut ones share the same value.
        const double p = pion.p();
        auto scintillator = pion.scintillator();
        int sector = scintillator.sector;
        double tof_time = scintillator.time;
        double tof_path = scintillator.path;
        auto timing = framework::CandidateUtils::compute_timing(
            p, Constants::PIMASS, pion.beta(), tof_time, tof_path, start_time);

        if (in_forward_detector) {
            h.edge_r1_precut.fill(edge_r1);
            h.edge_r2_precut.fill(edge_r2);
            h.edge_r3_precut.fill(edge_r3);
            h.chi2ndf_vs_edge_r1_precut.fill(edge_r1, chi2ndf);
            h.chi2ndf_vs_edge_r2_precut.fill(edge_r2, chi2ndf);
            h.chi2ndf_vs_edge_r3_precut.fill(edge_r3, chi2ndf);

            // delta_t vs p before any cut (inclusive + per sector). Skip
            // pions whose delta_t is exactly 0.0 — those are tracks with no
            // valid TOF hit (scintillator time/path = 0), which otherwise
            // pile up as a spurious spike at delta_t = 0.
            if (timing.delta_t != 0.0) {
                h.delta_t_vs_p_precut.fill(p, timing.delta_t);
                h.delta_t_vs_p_precut_sector.fill(sector, p, timing.delta_t);
            }

            // Tally momentum-cut impact before any momentum filtering.
            (is_piplus ? m_pscan_piplus : m_pscan_piminus).record(pion.p());
        }

        if (!evaluate_cutflow(in_forward_detector, pion.p(), E_tot, track.NDF > 0, cutflow)) return;

        double SF = E_tot / p;
        double lv = calo.pcal_lv;
        double lw = calo.pcal_lw;

        auto cherenkov = pion.cherenkov();
        double nphe = cherenkov.nphe;

        const double dvz = pion.vz() - electron.vz();
        h.p.fill(p);
        h.theta.fill(pion.theta());
        h.phi.fill(pion.phi());
        h.vz.fill(pion.vz());
        h.chi2pid.fill(pion.chi2pid());
        h.beta.fill(pion.beta());

        // Covariance-derived track resolutions from REC::CovMat (NaN when
        // the bank is absent or has no row for this track — those fills are
        // skipped so the histograms stay clean).
        const CovResolutions res = compute_cov_resolutions(event, pion);
        if (std::isfinite(res.sigma_tx))    { h.sigma_tx.fill(res.sigma_tx);    h.sigma_tx_vs_p.fill(p, res.sigma_tx); }
        if (std::isfinite(res.sigma_ty))    { h.sigma_ty.fill(res.sigma_ty);    h.sigma_ty_vs_p.fill(p, res.sigma_ty); }
        if (std::isfinite(res.sigma_theta)) { h.sigma_theta.fill(res.sigma_theta); h.sigma_theta_vs_p.fill(p, res.sigma_theta); }

        h.vz_vs_phi.fill(pion.phi(), pion.vz());
        h.vz_vs_p_2d.fill(p, pion.vz());
        h.vx_vs_vy.fill(pion.vx(), pion.vy());
        h.delta_vz.fill(dvz);
        h.delta_vz_vs_p.fill(p, dvz);

        h.delta_t.fill(timing.delta_t);
        // Exclude TOF-less tracks (delta_t exactly 0.0) from the delta_t-vs-p
        // plot — they otherwise form a spurious spike at delta_t = 0.
        if (timing.delta_t != 0.0) h.delta_t_vs_p_diag.fill(p, timing.delta_t);
        h.tof_mass2.fill(timing.tof_mass2);
        h.tof_mass2_vs_p.fill(p, timing.tof_mass2);
        h.tof_path.fill(tof_path);
        h.tof_path_vs_p.fill(p, tof_path);
        h.beta_vs_p.fill(p, pion.beta());
        h.beta_diff.fill(timing.beta_diff);
        h.beta_diff_vs_p.fill(p, timing.beta_diff);

        h.chi2pid_vs_p.fill(p, pion.chi2pid());
        h.chi2pid_vs_theta.fill(pion.theta(), pion.chi2pid());

        h.phi_vs_theta.fill(pion.theta(), pion.phi());
        h.p_vs_phi.fill(pion.phi(), p);
        h.p_vs_theta_diag.fill(pion.theta(), p);

        if (sector >= 1 && sector <= 6) {
            h.vz_vs_sector.fill(static_cast<double>(sector), pion.vz());
            h.chi2pid_vs_sector.fill(static_cast<double>(sector), pion.chi2pid());
        }

        if (m_ntuple) {
            PionCandidate cand{pion, calo, traj, track, E_tot, SF, chi2ndf, timing.delta_t, tof_path, sector};
            ntuple.data().from(cand);
            ntuple.fill();
        }

        h.sf.fill(SF);
        h.lv.fill(lv);
        h.lw.fill(lw);
        h.sf_vs_lv.fill(lv, SF);
        h.sf_vs_lw.fill(lw, SF);

        h.chi2ndf.fill(chi2ndf);
        h.edge_r1.fill(edge_r1);
        h.edge_r2.fill(edge_r2);
        h.edge_r3.fill(edge_r3);
        h.chi2ndf_vs_edge_r1.fill(edge_r1, chi2ndf);
        h.chi2ndf_vs_edge_r2.fill(edge_r2, chi2ndf);
        h.chi2ndf_vs_edge_r3.fill(edge_r3, chi2ndf);

        if (timing.delta_t != 0.0) h.delta_t_vs_p_sector.fill(sector, p, timing.delta_t);
        h.p_vs_theta_sector.fill(sector, pion.theta(), p);

        const int calo_sector = calo.pcal_sector;
        h.edge_vs_chi2ndf_r1.fill(calo_sector, edge_r1, chi2ndf);
        h.edge_vs_chi2ndf_r2.fill(calo_sector, edge_r2, chi2ndf);
        h.edge_vs_chi2ndf_r3.fill(calo_sector, edge_r3, chi2ndf);

        const double E_cal = calo.inner_energy + calo.outer_energy;
        const double E_pcal = calo.pcal_energy;
        if (E_pcal == 0) return;
        h.lv_vs_SF.fill(calo_sector, lv, SF);
        h.lw_vs_SF.fill(calo_sector, lw, SF);
        h.nphe.fill(calo_sector, nphe);
        h.EPCal_vs_ECal.fill(calo_sector, E_pcal, E_cal);
        h.EPCal.fill(calo_sector, E_pcal);
        h.p_vs_SF.fill(calo_sector, p, SF);
        h.p_vs_theta.fill(calo_sector, pion.theta(), p);
    }
};

}  // namespace algorithms
