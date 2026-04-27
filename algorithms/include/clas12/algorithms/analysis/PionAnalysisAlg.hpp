#pragma once

#include <array>
#include <cmath>
#include <string>
#include <clas12/Constants.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
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
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

/// Configuration for PionAnalysisAlg.
struct PionAnalysisConfig {
    clas12::Target target = clas12::Target::LD2;
    clas12::Polarity polarity = clas12::Polarity::Outbending;
};

/**
 * @brief Pion analysis with calorimeter SF and DC quality histograms
 */
class PionAnalysisAlg : public clas12::framework::Algorithm {
   public:
    using Config = PionAnalysisConfig;
    using StatusCode = clas12::framework::StatusCode;

    explicit PionAnalysisAlg(std::string name = "PionAnalysis", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow_piplus("π⁺ Analysis"), m_cutflow_piminus("π⁻ Analysis") {
        required_services(m_hist, m_ntuple);
        declare_banks("REC::Event", "REC::Particle", "REC::Calorimeter", "REC::Traj", "REC::Track", "REC::Scintillator");
        declare_properties(m_min_momentum, m_min_calo_energy, m_enable_forward_detector_cut, m_enable_momentum_cut, m_enable_calorimeter_cut, m_enable_dc_track_cut);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing pion analysis");
        Log::info(name(), "Target: {}, Polarity: {}", clas12::to_string(m_config.target), clas12::to_string(m_config.polarity));
        Log::info(name(), "Min momentum: {} GeV, Min calo energy: {} GeV", m_min_momentum.get(), m_min_calo_energy.get());

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

    auto process(clas12::framework::Event& event) -> StatusCode override {
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodElectron)) return StatusCode::FILTERED();
        auto electron_opt = event.retrieve_typed(clas12::framework::Keys::TriggerElectron);
        if (!electron_opt) return StatusCode::FILTERED();
        const clas12::Particle& electron = *electron_opt;

        clas12::bankdefs::REC_Event rec_event;
        rec_event.bind(event.get_bank("REC::Event"));
        if (rec_event.rows() == 0) return StatusCode::FILTERED();

        const double start_time = static_cast<double>(rec_event.startTime(0));

        for (const auto& pion : event.pions_plus()) process_pion(pion, electron, start_time, true);
        for (const auto& pion : event.pions_minus()) process_pion(pion, electron, start_time, false);

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

        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;

    clas12::framework::Cutflow m_cutflow_piplus;
    clas12::framework::Cutflow m_cutflow_piminus;

    clas12::framework::Property<double> m_min_momentum{"min_momentum", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_min_calo_energy{"min_calo_energy", 0.01, "Minimum calorimeter energy [GeV]"};

    clas12::framework::Property<bool> m_enable_forward_detector_cut{"enable_forward_detector_cut", true, "Enable forward detector cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_calorimeter_cut{"enable_calorimeter_cut", true, "Enable calorimeter cut"};
    clas12::framework::Property<bool> m_enable_dc_track_cut{"enable_dc_track_cut", true, "Enable DC track cut"};

    clas12::framework::ServiceRef<clas12::framework::HistogramService> m_hist;
    clas12::framework::ServiceRef<clas12::framework::NTupleService> m_ntuple;

    struct PionCandidate {
        clas12::Particle pion;
        clas12::CalorimeterBank calorimeter;
        clas12::DCTrajBank traj;
        clas12::TrackBank track;
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
            return std::make_tuple(clas12::framework::field("pi_p", &T::pi_p), clas12::framework::field("pi_theta", &T::pi_theta), clas12::framework::field("pi_phi", &T::pi_phi), clas12::framework::field("pi_vz", &T::pi_vz), clas12::framework::field("pi_chi2pid", &T::pi_chi2pid), clas12::framework::field("pi_beta", &T::pi_beta), clas12::framework::field("pi_sampling_fraction", &T::pi_sampling_fraction), clas12::framework::field("pi_sector", &T::pi_sector), clas12::framework::field("pi_chi2ndf", &T::pi_chi2ndf), clas12::framework::field("pi_edge_r1", &T::pi_edge_r1), clas12::framework::field("pi_edge_r2", &T::pi_edge_r2), clas12::framework::field("pi_edge_r3", &T::pi_edge_r3), clas12::framework::field("pi_x_r1", &T::pi_x_r1), clas12::framework::field("pi_y_r1", &T::pi_y_r1), clas12::framework::field("pi_z_r1", &T::pi_z_r1), clas12::framework::field("pi_x_r2", &T::pi_x_r2), clas12::framework::field("pi_y_r2", &T::pi_y_r2), clas12::framework::field("pi_z_r2", &T::pi_z_r2), clas12::framework::field("pi_delta_t", &T::pi_delta_t), clas12::framework::field("pi_tof_path", &T::pi_tof_path));
        }
    };

    clas12::framework::NTupleWriter<PionNTupleData> m_ntuple_piplus;
    clas12::framework::NTupleWriter<PionNTupleData> m_ntuple_piminus;

    struct PionHistograms {
        clas12::framework::ThreadLocalHistogram1D* p = nullptr;
        clas12::framework::ThreadLocalHistogram1D* theta = nullptr;
        clas12::framework::ThreadLocalHistogram1D* phi = nullptr;
        clas12::framework::ThreadLocalHistogram1D* vz = nullptr;
        clas12::framework::ThreadLocalHistogram1D* chi2pid = nullptr;
        clas12::framework::ThreadLocalHistogram1D* beta = nullptr;

        clas12::framework::ThreadLocalHistogram2D* vz_vs_phi = nullptr;
        clas12::framework::ThreadLocalHistogram2D* vz_vs_sector = nullptr;
        clas12::framework::ThreadLocalHistogram2D* vz_vs_p_2d = nullptr;
        clas12::framework::ThreadLocalHistogram2D* vx_vs_vy = nullptr;
        clas12::framework::ThreadLocalHistogram1D* delta_vz = nullptr;
        clas12::framework::ThreadLocalHistogram2D* delta_vz_vs_p = nullptr;

        clas12::framework::ThreadLocalHistogram1D* delta_t = nullptr;
        clas12::framework::ThreadLocalHistogram2D* delta_t_vs_p_diag = nullptr;
        clas12::framework::ThreadLocalHistogram1D* tof_mass2 = nullptr;
        clas12::framework::ThreadLocalHistogram2D* tof_mass2_vs_p = nullptr;
        clas12::framework::ThreadLocalHistogram1D* tof_path = nullptr;
        clas12::framework::ThreadLocalHistogram2D* tof_path_vs_p = nullptr;
        clas12::framework::ThreadLocalHistogram2D* beta_vs_p = nullptr;
        clas12::framework::ThreadLocalHistogram1D* beta_diff = nullptr;
        clas12::framework::ThreadLocalHistogram2D* beta_diff_vs_p = nullptr;

        clas12::framework::ThreadLocalHistogram2D* chi2pid_vs_p = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2pid_vs_theta = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2pid_vs_sector = nullptr;

        clas12::framework::ThreadLocalHistogram2D* phi_vs_theta = nullptr;
        clas12::framework::ThreadLocalHistogram2D* p_vs_phi = nullptr;
        clas12::framework::ThreadLocalHistogram2D* p_vs_theta_diag = nullptr;

        clas12::framework::ThreadLocalHistogram1D* sf = nullptr;
        clas12::framework::ThreadLocalHistogram1D* lv = nullptr;
        clas12::framework::ThreadLocalHistogram1D* lw = nullptr;
        clas12::framework::ThreadLocalHistogram2D* sf_vs_lv = nullptr;
        clas12::framework::ThreadLocalHistogram2D* sf_vs_lw = nullptr;

        clas12::framework::ThreadLocalHistogram1D* chi2ndf = nullptr;
        clas12::framework::ThreadLocalHistogram1D* edge_r1 = nullptr;
        clas12::framework::ThreadLocalHistogram1D* edge_r2 = nullptr;
        clas12::framework::ThreadLocalHistogram1D* edge_r3 = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2ndf_vs_edge_r1 = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2ndf_vs_edge_r2 = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2ndf_vs_edge_r3 = nullptr;

        clas12::framework::ThreadLocalHistogram1D* edge_r1_precut = nullptr;
        clas12::framework::ThreadLocalHistogram1D* edge_r2_precut = nullptr;
        clas12::framework::ThreadLocalHistogram1D* edge_r3_precut = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2ndf_vs_edge_r1_precut = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2ndf_vs_edge_r2_precut = nullptr;
        clas12::framework::ThreadLocalHistogram2D* chi2ndf_vs_edge_r3_precut = nullptr;

        clas12::framework::PerSectorHist2D<6> delta_t_vs_p_sector;
        clas12::framework::PerSectorHist2D<6> p_vs_theta_sector;

        clas12::framework::PerSectorHist1D<6> nphe;
        clas12::framework::PerSectorHist2D<6> EPCal_vs_ECal;
        clas12::framework::PerSectorHist1D<6> EPCal;
        clas12::framework::PerSectorHist2D<6> p_vs_SF;
        clas12::framework::PerSectorHist2D<6> lv_vs_SF;
        clas12::framework::PerSectorHist2D<6> lw_vs_SF;
        clas12::framework::PerSectorHist2D<6> edge_vs_chi2ndf_r1;
        clas12::framework::PerSectorHist2D<6> edge_vs_chi2ndf_r2;
        clas12::framework::PerSectorHist2D<6> edge_vs_chi2ndf_r3;
        clas12::framework::PerSectorHist2D<6> p_vs_theta;
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    auto evaluate_cutflow(bool in_forward_detector, double p, double E_tot, bool has_dc, clas12::framework::Cutflow& cf) const -> bool {
        cf.pass("All Pions");
        if (m_enable_forward_detector_cut.get() && !in_forward_detector) return false;
        cf.pass("In the forward detector");
        if (m_enable_momentum_cut.get() && p < m_min_momentum.get()) return false;
        cf.pass("Momentum > 0.3 GeV");
        if (m_enable_calorimeter_cut.get() && E_tot <= m_min_calo_energy.get()) return false;
        cf.pass("Has Calorimeter");
        if (m_enable_dc_track_cut.get() && !has_dc) return false;
        cf.pass("Has DC Track");
        return true;
    }

    auto book_histograms() -> void {
        book_pion_histograms("/PionAnalysis/PiPlus", h_piplus, "#pi^{+}");
        book_pion_histograms("/PionAnalysis/PiMinus", h_piminus, "#pi^{-}");
    }

    auto book_pion_histograms(const std::string& dir, PionHistograms& h, const std::string& label) -> void {
        auto* svc = m_hist.get();

        h.p = svc->book_hist1D(dir + "/p", fmt::format("{} Momentum;p [GeV/c];Events", label), 100, 0.0, 10.0);
        h.theta = svc->book_hist1D(dir + "/theta", fmt::format("{} #theta;#theta [deg];Events", label), 180, 0.0, 40.0);
        h.phi = svc->book_hist1D(dir + "/phi", fmt::format("{} #phi;#phi [deg];Events", label), 360, -180.0, 180.0);
        h.vz = svc->book_hist1D(dir + "/vz", fmt::format("{} v_{{z}};v_{{z}} [cm];Events", label), 200, -40.0, 40.0);
        h.chi2pid = svc->book_hist1D(dir + "/chi2pid", fmt::format("{} #chi^{{2}}_{{PID}};#chi^{{2}}_{{PID}};Events", label), 200, -10.0, 10.0);
        h.beta = svc->book_hist1D(dir + "/beta", fmt::format("{} #beta;#beta;Events", label), 100, 0.5, 1.1);

        h.vz_vs_phi = svc->book_hist2D(dir + "/vz_vs_phi", fmt::format("{} v_{{z}} vs #phi;#phi [deg];v_{{z}} [cm]", label), 180, -180, 180, 100, -40, 40);
        h.vz_vs_sector = svc->book_hist2D(dir + "/vz_vs_sector", fmt::format("{} v_{{z}} vs Sector;Sector;v_{{z}} [cm]", label), 6, 0.5, 6.5, 100, -40, 40);
        h.vz_vs_p_2d = svc->book_hist2D(dir + "/vz_vs_p", fmt::format("{} v_{{z}} vs p;p [GeV/c];v_{{z}} [cm]", label), 100, 0, 10, 100, -40, 40);
        h.vx_vs_vy = svc->book_hist2D(dir + "/vx_vs_vy", fmt::format("{} v_{{x}} vs v_{{y}};v_{{x}} [cm];v_{{y}} [cm]", label), 100, -2, 2, 100, -2, 2);
        h.delta_vz = svc->book_hist1D(dir + "/delta_vz", fmt::format("{} #Deltav_{{z}} (#pi - e);#Deltav_{{z}} [cm];Events", label), 200, -20, 20);
        h.delta_vz_vs_p = svc->book_hist2D(dir + "/delta_vz_vs_p", fmt::format("{} #Deltav_{{z}} vs p;p [GeV/c];#Deltav_{{z}} [cm]", label), 100, 0, 10, 200, -20, 20);

        h.delta_t = svc->book_hist1D(dir + "/delta_t", fmt::format("{} #Deltat;#Deltat [ns];Events", label), 200, -2, 2);
        h.delta_t_vs_p_diag = svc->book_hist2D(dir + "/delta_t_vs_p_diag", fmt::format("{} #Deltat vs p;p [GeV/c];#Deltat [ns]", label), 100, 0, 10, 200, -2, 2);
        h.tof_mass2 = svc->book_hist1D(dir + "/tof_mass2", fmt::format("{} m^{{2}}_{{TOF}};m^{{2}} [GeV^{{2}}];Events", label), 200, -0.2, 0.5);
        h.tof_mass2_vs_p = svc->book_hist2D(dir + "/tof_mass2_vs_p", fmt::format("{} m^{{2}}_{{TOF}} vs p;p [GeV/c];m^{{2}} [GeV^{{2}}]", label), 100, 0, 10, 200, -0.2, 0.5);
        h.tof_path = svc->book_hist1D(dir + "/tof_path", fmt::format("{} TOF Path Length;Path [cm];Events", label), 100, 0, 800);
        h.tof_path_vs_p = svc->book_hist2D(dir + "/tof_path_vs_p", fmt::format("{} TOF Path vs p;p [GeV/c];Path [cm]", label), 100, 0, 10, 100, 0, 800);
        h.beta_vs_p = svc->book_hist2D(dir + "/beta_vs_p", fmt::format("{} #beta vs p;p [GeV/c];#beta", label), 100, 0, 10, 100, 0.5, 1.1);
        h.beta_diff = svc->book_hist1D(dir + "/beta_diff", fmt::format("{} #Delta#beta;#Delta#beta;Events", label), 200, -0.1, 0.1);
        h.beta_diff_vs_p = svc->book_hist2D(dir + "/beta_diff_vs_p", fmt::format("{} #Delta#beta vs p;p [GeV/c];#Delta#beta", label), 100, 0, 10, 200, -0.1, 0.1);

        h.chi2pid_vs_p = svc->book_hist2D(dir + "/chi2pid_vs_p", fmt::format("{} #chi^{{2}}_{{PID}} vs p;p [GeV/c];#chi^{{2}}_{{PID}}", label), 100, 0, 10, 100, -10, 10);
        h.chi2pid_vs_theta = svc->book_hist2D(dir + "/chi2pid_vs_theta", fmt::format("{} #chi^{{2}}_{{PID}} vs #theta;#theta [deg];#chi^{{2}}_{{PID}}", label), 90, 0, 45, 100, -10, 10);
        h.chi2pid_vs_sector = svc->book_hist2D(dir + "/chi2pid_vs_sector", fmt::format("{} #chi^{{2}}_{{PID}} vs Sector;Sector;#chi^{{2}}_{{PID}}", label), 6, 0.5, 6.5, 100, -10, 10);

        h.phi_vs_theta = svc->book_hist2D(dir + "/phi_vs_theta", fmt::format("{} #phi vs #theta;#theta [deg];#phi [deg]", label), 90, 0, 45, 180, -180, 180);
        h.p_vs_phi = svc->book_hist2D(dir + "/p_vs_phi", fmt::format("{} p vs #phi;#phi [deg];p [GeV/c]", label), 180, -180, 180, 100, 0, 10);
        h.p_vs_theta_diag = svc->book_hist2D(dir + "/p_vs_theta_diag", fmt::format("{} p vs #theta;#theta [deg];p [GeV/c]", label), 90, 0, 45, 100, 0, 10);

        h.sf = svc->book_hist1D(dir + "/sf", fmt::format("{} Sampling Fraction;SF;Events", label), 100, 0, 0.4);
        h.lv = svc->book_hist1D(dir + "/lv", fmt::format("{} PCAL lv;lv [cm];Events", label), 100, 0, 500);
        h.lw = svc->book_hist1D(dir + "/lw", fmt::format("{} PCAL lw;lw [cm];Events", label), 100, 0, 500);
        h.sf_vs_lv = svc->book_hist2D(dir + "/sf_vs_lv", fmt::format("{} Sf vs lv;lv [cm];SF", label), 200, 0, 70, 200, 0, 0.4);
        h.sf_vs_lw = svc->book_hist2D(dir + "/sf_vs_lw", fmt::format("{} Sf vs lw;lw [cm];SF", label), 200, 0, 70, 200, 0, 0.4);

        h.chi2ndf = svc->book_hist1D(dir + "/chi2ndf", fmt::format("{} DC #chi^{{2}}/NDF;#chi^{{2}}/NDF;Events", label), 100, 0, 20);
        h.edge_r1 = svc->book_hist1D(dir + "/edge_r1", fmt::format("{} DC Edge R1;Edge [cm];Events", label), 100, -10, 50);
        h.edge_r2 = svc->book_hist1D(dir + "/edge_r2", fmt::format("{} DC Edge R2;Edge [cm];Events", label), 100, -10, 50);
        h.edge_r3 = svc->book_hist1D(dir + "/edge_r3", fmt::format("{} DC Edge R3;Edge [cm];Events", label), 100, -10, 50);
        h.chi2ndf_vs_edge_r1 = svc->book_hist2D(dir + "/chi2ndf_vs_edge_r1", fmt::format("{} #chi^{{2}}/NDF vs Edge R1;Edge R1 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r2 = svc->book_hist2D(dir + "/chi2ndf_vs_edge_r2", fmt::format("{} #chi^{{2}}/NDF vs Edge R2;Edge R2 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r3 = svc->book_hist2D(dir + "/chi2ndf_vs_edge_r3", fmt::format("{} #chi^{{2}}/NDF vs Edge R3;Edge R3 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);

        h.edge_r1_precut = svc->book_hist1D(dir + "/edge_r1_precut", fmt::format("{} DC Edge R1 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.edge_r2_precut = svc->book_hist1D(dir + "/edge_r2_precut", fmt::format("{} DC Edge R2 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.edge_r3_precut = svc->book_hist1D(dir + "/edge_r3_precut", fmt::format("{} DC Edge R3 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.chi2ndf_vs_edge_r1_precut = svc->book_hist2D(dir + "/chi2ndf_vs_edge_r1_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R1 (pre-cut);Edge R1 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r2_precut = svc->book_hist2D(dir + "/chi2ndf_vs_edge_r2_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R2 (pre-cut);Edge R2 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r3_precut = svc->book_hist2D(dir + "/chi2ndf_vs_edge_r3_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R3 (pre-cut);Edge R3 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);

        clas12::framework::SectorHistogramGroup<6> g(svc, dir);
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
    }

    auto process_pion(const clas12::Particle& pion, const clas12::Particle& electron, double start_time, bool is_piplus) -> void {
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
        bool in_forward_detector = clas12::framework::CandidateUtils::is_forward_detector(pion.status());

        if (in_forward_detector) {
            if (h.edge_r1_precut) h.edge_r1_precut->fill(edge_r1);
            if (h.edge_r2_precut) h.edge_r2_precut->fill(edge_r2);
            if (h.edge_r3_precut) h.edge_r3_precut->fill(edge_r3);
            if (h.chi2ndf_vs_edge_r1_precut) h.chi2ndf_vs_edge_r1_precut->fill(edge_r1, chi2ndf);
            if (h.chi2ndf_vs_edge_r2_precut) h.chi2ndf_vs_edge_r2_precut->fill(edge_r2, chi2ndf);
            if (h.chi2ndf_vs_edge_r3_precut) h.chi2ndf_vs_edge_r3_precut->fill(edge_r3, chi2ndf);
        }

        if (!evaluate_cutflow(in_forward_detector, pion.p(), E_tot, track.NDF > 0, cutflow)) return;

        double p = pion.p();
        double SF = E_tot / p;
        double lv = calo.pcal_lv;
        double lw = calo.pcal_lw;

        auto cherenkov = pion.cherenkov();
        double nphe = cherenkov.nphe;

        auto scintillator = pion.scintillator();
        int sector = scintillator.sector;
        double tof_time = scintillator.time;
        double tof_path = scintillator.path;

        auto timing = clas12::framework::CandidateUtils::compute_timing(p, clas12::Constants::PIMASS, pion.beta(), tof_time, tof_path, start_time);

        const double dvz = pion.vz() - electron.vz();
        h.p->fill(p);
        h.theta->fill(pion.theta());
        h.phi->fill(pion.phi());
        h.vz->fill(pion.vz());
        h.chi2pid->fill(pion.chi2pid());
        h.beta->fill(pion.beta());

        h.vz_vs_phi->fill(pion.phi(), pion.vz());
        h.vz_vs_p_2d->fill(p, pion.vz());
        h.vx_vs_vy->fill(pion.vx(), pion.vy());
        h.delta_vz->fill(dvz);
        h.delta_vz_vs_p->fill(p, dvz);

        h.delta_t->fill(timing.delta_t);
        h.delta_t_vs_p_diag->fill(p, timing.delta_t);
        h.tof_mass2->fill(timing.tof_mass2);
        h.tof_mass2_vs_p->fill(p, timing.tof_mass2);
        h.tof_path->fill(tof_path);
        h.tof_path_vs_p->fill(p, tof_path);
        h.beta_vs_p->fill(p, pion.beta());
        h.beta_diff->fill(timing.beta_diff);
        h.beta_diff_vs_p->fill(p, timing.beta_diff);

        h.chi2pid_vs_p->fill(p, pion.chi2pid());
        h.chi2pid_vs_theta->fill(pion.theta(), pion.chi2pid());

        h.phi_vs_theta->fill(pion.theta(), pion.phi());
        h.p_vs_phi->fill(pion.phi(), p);
        h.p_vs_theta_diag->fill(pion.theta(), p);

        if (sector >= 1 && sector <= 6) {
            h.vz_vs_sector->fill(static_cast<double>(sector), pion.vz());
            h.chi2pid_vs_sector->fill(static_cast<double>(sector), pion.chi2pid());
        }

        if (m_ntuple) {
            PionCandidate cand{pion, calo, traj, track, E_tot, SF, chi2ndf, timing.delta_t, tof_path, sector};
            ntuple.data().from(cand);
            ntuple.fill();
        }

        h.sf->fill(SF);
        h.lv->fill(lv);
        h.lw->fill(lw);
        h.sf_vs_lv->fill(lv, SF);
        h.sf_vs_lw->fill(lw, SF);

        h.chi2ndf->fill(chi2ndf);
        h.edge_r1->fill(edge_r1);
        h.edge_r2->fill(edge_r2);
        h.edge_r3->fill(edge_r3);
        h.chi2ndf_vs_edge_r1->fill(edge_r1, chi2ndf);
        h.chi2ndf_vs_edge_r2->fill(edge_r2, chi2ndf);
        h.chi2ndf_vs_edge_r3->fill(edge_r3, chi2ndf);

        h.delta_t_vs_p_sector.fill(sector, p, timing.delta_t);
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

}  // namespace clas12::algorithms
