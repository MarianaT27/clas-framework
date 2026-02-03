#pragma once

/**
 * @file PionAnalysisAlg.hpp
 * @brief Pion analysis algorithm with calorimeter and DC quality histograms
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>()
 * - Property system for configuration
 * - Reduced boilerplate through CRTP
 *
 * Produces histograms for:
 * - lv vs Sampling Fraction
 * - lw vs Sampling Fraction
 * - chi2/ndf vs DC edge (all 3 regions)
 *
 * Requires a selected electron in the event (HasGoodElectron decision).
 */

#include <clas12/Constants.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

#include <cmath>
#include <array>
#include <string>

namespace clas12::algorithms {

using clas12::framework::Log;

/**
 * @brief Pion analysis with calorimeter SF and DC quality histograms
 */
class PionAnalysisAlg : public clas12::framework::SimpleAlgorithm<PionAnalysisAlg> {
   public:
    explicit PionAnalysisAlg(std::string name = "PionAnalysis",
                              clas12::Target target = clas12::Target::LD2,
                              clas12::Polarity polarity = clas12::Polarity::Outbending)
        : SimpleAlgorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow_piplus("π⁺ Analysis")
        , m_cutflow_piminus("π⁻ Analysis") {
        // Declare properties
        declare_property(m_min_momentum);
        declare_property(m_min_calo_energy);
    }

    /**
     * @brief Configure required services
     */
    auto configure() -> void override {
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Initializing pion analysis");
        Log::info(name(), "Target: {}, Polarity: {}", clas12::to_string(m_target), clas12::to_string(m_polarity));
        Log::info(name(), "Min momentum: {} GeV, Min calo energy: {} GeV",
                  m_min_momentum.get(), m_min_calo_energy.get());

        setup_cutflows();
        book_histograms();

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Require selected electron
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodElectron)) {
            return clas12::framework::StatusCode::FILTERED();
        }

        auto& hipo_event = event.hipo_event();
        auto [REC_Event, REC_Particle, REC_Calorimeter, REC_Traj, REC_Track, REC_Scintillator] =
            hipo_event.banks("REC::Event", "REC::Particle", "REC::Calorimeter", "REC::Traj", "REC::Track", "REC::Scintillator");

        if (REC_Event->rows() == 0) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Get start time for timing calculations
        double start_time = REC_Event->get<double>("startTime", 0);

        // Get electron vertex for delta_vz calculation
        double electron_vz = 0.0;
        auto electrons_opt = event.retrieve_typed(clas12::framework::Keys::SelectedElectronsCapital);
        if (electrons_opt && !electrons_opt->empty()) {
            electron_vz = electrons_opt->at(0).vz();
        }

        // Extract pions
        auto particles = m_particle_svc->get()->extract_particles(
            *REC_Particle, {clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS});

        // Process π⁺
        for (const auto& pion : m_particle_svc->get()->get_pions_plus(particles)) {
            process_pion(pion, *REC_Calorimeter, *REC_Traj, *REC_Track, *REC_Scintillator, start_time, electron_vz, true);
        }

        // Process π⁻
        for (const auto& pion : m_particle_svc->get()->get_pions_minus(particles)) {
            process_pion(pion, *REC_Calorimeter, *REC_Traj, *REC_Track, *REC_Scintillator, start_time, electron_vz, false);
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing pion analysis");

        m_cutflow_piplus.print();
        m_cutflow_piminus.print();

        Log::info(name(), "Pion Analysis Summary:");
        Log::info(name(), "     - π⁺ with calorimeter info: {}", m_cutflow_piplus.get_count("Has Calorimeter"));
        Log::info(name(), "     - π⁻ with calorimeter info: {}", m_cutflow_piminus.get_count("Has Calorimeter"));

        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::Target m_target;
    clas12::Polarity m_polarity;
    clas12::framework::Cutflow m_cutflow_piplus;
    clas12::framework::Cutflow m_cutflow_piminus;

    // Properties
    clas12::framework::Property<double> m_min_momentum{"min_momentum", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_min_calo_energy{"min_calo_energy", 0.01, "Minimum calorimeter energy [GeV]"};

    // Service handles (auto-injected via require_service)
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;

    // Lock-free atomic histogram set for each pion charge
    struct PionHistograms {
        // Calorimeter: lv, lw vs SF (post-cut)
        clas12::framework::AtomicHistogram2D* sf_vs_lv = nullptr;
        clas12::framework::AtomicHistogram2D* sf_vs_lw = nullptr;
        clas12::framework::AtomicHistogram1D* sf = nullptr;
        clas12::framework::AtomicHistogram1D* lv = nullptr;
        clas12::framework::AtomicHistogram1D* lw = nullptr;

        // DC: chi2/ndf vs edge (post-cut)
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r1 = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r2 = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r3 = nullptr;
        clas12::framework::AtomicHistogram1D* chi2ndf = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r1 = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r2 = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r3 = nullptr;

        // Pre-cut DC edge histograms (for efficiency studies)
        clas12::framework::AtomicHistogram1D* edge_r1_precut = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r2_precut = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r3_precut = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r1_precut = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r2_precut = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r3_precut = nullptr;

        struct SectorHistograms {
            bool booked = false;
            // Calorimeter (per sector)
            std::map<int, clas12::framework::AtomicHistogram1D*> nphe;
            std::map<int, clas12::framework::AtomicHistogram2D*> EPCal_vs_ECal;
            std::map<int, clas12::framework::AtomicHistogram1D*> EPCal;
            std::map<int, clas12::framework::AtomicHistogram2D*> p_vs_SF;
            std::map<int, clas12::framework::AtomicHistogram2D*> lv_vs_SF;
            std::map<int, clas12::framework::AtomicHistogram2D*> lw_vs_SF;
            // DC fiducial (per sector)
            std::map<int, clas12::framework::AtomicHistogram2D*> edge_vs_chi2ndf_r1;
            std::map<int, clas12::framework::AtomicHistogram2D*> edge_vs_chi2ndf_r2;
            std::map<int, clas12::framework::AtomicHistogram2D*> edge_vs_chi2ndf_r3;
            // Sector-level p vs theta
            std::map<int, clas12::framework::AtomicHistogram2D*> p_vs_theta;
        };
        SectorHistograms sector_histograms;

        // Kinematics
        clas12::framework::AtomicHistogram1D* momentum = nullptr;
        clas12::framework::AtomicHistogram1D* theta = nullptr;
        clas12::framework::AtomicHistogram1D* phi = nullptr;
        clas12::framework::AtomicHistogram1D* vz = nullptr;

        // ========== ENHANCED DIAGNOSTIC HISTOGRAMS ==========

        // Vertex diagnostics
        clas12::framework::AtomicHistogram2D* vz_vs_phi = nullptr;
        clas12::framework::AtomicHistogram2D* vz_vs_sector = nullptr;
        clas12::framework::AtomicHistogram2D* vz_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* vx_vs_vy = nullptr;
        clas12::framework::AtomicHistogram1D* delta_vz = nullptr;
        clas12::framework::AtomicHistogram2D* delta_vz_vs_p = nullptr;

        // Timing diagnostics (delta_t vs p)
        clas12::framework::AtomicHistogram2D* delta_t_vs_p = nullptr;
        std::array<clas12::framework::AtomicHistogram2D*, 6> delta_t_vs_p_sector = {};
        clas12::framework::AtomicHistogram1D* tof_mass2 = nullptr;
        clas12::framework::AtomicHistogram2D* tof_mass2_vs_p = nullptr;
        clas12::framework::AtomicHistogram1D* tof_path = nullptr;
        clas12::framework::AtomicHistogram2D* tof_path_vs_p = nullptr;
        clas12::framework::AtomicHistogram1D* beta = nullptr;
        clas12::framework::AtomicHistogram2D* beta_vs_p = nullptr;
        clas12::framework::AtomicHistogram1D* beta_diff = nullptr;
        clas12::framework::AtomicHistogram2D* beta_diff_vs_p = nullptr;
        clas12::framework::AtomicHistogram1D* delta_t = nullptr;

        // PID diagnostics
        clas12::framework::AtomicHistogram1D* chi2pid = nullptr;
        clas12::framework::AtomicHistogram2D* chi2pid_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* chi2pid_vs_theta = nullptr;
        clas12::framework::AtomicHistogram2D* chi2pid_vs_sector = nullptr;

        // Phase space and acceptance
        clas12::framework::AtomicHistogram2D* phi_vs_theta = nullptr;
        clas12::framework::AtomicHistogram2D* p_vs_phi = nullptr;
        clas12::framework::AtomicHistogram2D* p_vs_theta = nullptr;
        std::array<clas12::framework::AtomicHistogram2D*, 6> p_vs_theta_sector = {};
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    void setup_cutflows() {
        auto setup = [](clas12::framework::Cutflow& cf) {
            cf.add_cut("All Pions");
            cf.add_cut("In the forward detector");
            cf.add_cut("Momentum > 0.3 GeV");
            cf.add_cut("Has Calorimeter");
            cf.add_cut("Has DC Track");
        };
        setup(m_cutflow_piplus);
        setup(m_cutflow_piminus);
    }

    void book_histograms() {
        if (!m_hist_svc || !m_hist_svc->is_valid()) return;
        book_pion_histograms("/PionAnalysis/PiPlus", h_piplus, "#pi^{+}");
        book_pion_histograms("/PionAnalysis/PiMinus", h_piminus, "#pi^{-}");
    }

    void book_pion_histograms(const std::string& dir, PionHistograms& h, const std::string& label) {
        auto* svc = m_hist_svc->get();
        // Sampling fraction histograms (lock-free atomic)
        h.sf = svc->book_atomic_hist1D(dir + "/sf", fmt::format("{} Sampling Fraction;SF;Events", label), 100, 0, 0.4);
        h.lv = svc->book_atomic_hist1D(dir + "/lv", fmt::format("{} PCAL lv;lv [cm];Events", label), 100, 0, 500);
        h.lw = svc->book_atomic_hist1D(dir + "/lw", fmt::format("{} PCAL lw;lw [cm];Events", label), 100, 0, 500);
        h.sf_vs_lv = svc->book_atomic_hist2D(dir + "/sf_vs_lv", fmt::format("{} Sf vs lv;lv [cm];SF", label), 200, 0, 70, 200, 0, 0.4);
        h.sf_vs_lw = svc->book_atomic_hist2D(dir + "/sf_vs_lw", fmt::format("{} Sf vs lw;lw [cm];SF", label), 200, 0, 70, 200, 0, 0.4);

        // DC chi2/ndf vs edge histograms (lock-free atomic)
        h.chi2ndf = svc->book_atomic_hist1D(dir + "/chi2ndf", fmt::format("{} DC #chi^{{2}}/NDF;#chi^{{2}}/NDF;Events", label), 100, 0, 20);
        h.edge_r1 = svc->book_atomic_hist1D(dir + "/edge_r1", fmt::format("{} DC Edge R1;Edge [cm];Events", label), 100, -10, 50);
        h.edge_r2 = svc->book_atomic_hist1D(dir + "/edge_r2", fmt::format("{} DC Edge R2;Edge [cm];Events", label), 100, -10, 50);
        h.edge_r3 = svc->book_atomic_hist1D(dir + "/edge_r3", fmt::format("{} DC Edge R3;Edge [cm];Events", label), 100, -10, 50);

        h.chi2ndf_vs_edge_r1 = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r1", fmt::format("{} #chi^{{2}}/NDF vs Edge R1;Edge R1 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r2 = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r2", fmt::format("{} #chi^{{2}}/NDF vs Edge R2;Edge R2 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r3 = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r3", fmt::format("{} #chi^{{2}}/NDF vs Edge R3;Edge R3 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);

        // Pre-cut DC edge histograms (for efficiency studies)
        h.edge_r1_precut = svc->book_atomic_hist1D(dir + "/edge_r1_precut", fmt::format("{} DC Edge R1 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.edge_r2_precut = svc->book_atomic_hist1D(dir + "/edge_r2_precut", fmt::format("{} DC Edge R2 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.edge_r3_precut = svc->book_atomic_hist1D(dir + "/edge_r3_precut", fmt::format("{} DC Edge R3 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.chi2ndf_vs_edge_r1_precut = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r1_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R1 (pre-cut);Edge R1 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r2_precut = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r2_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R2 (pre-cut);Edge R2 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r3_precut = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r3_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R3 (pre-cut);Edge R3 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);

        // Basic kinematics (lock-free atomic)
        h.momentum = svc->book_atomic_momentum_hist(dir + "/momentum", fmt::format("{} Momentum", label));
        h.theta = svc->book_atomic_hist1D(dir + "/theta", fmt::format("{} #theta;#theta [deg];Events", label), 180, 0, 40);
        h.phi = svc->book_atomic_hist1D(dir + "/phi", fmt::format("{} #phi;#phi [deg];Events", label), 360, -180, 180);
        h.vz = svc->book_atomic_hist1D(dir + "/vz", fmt::format("{} v_{{z}};v_{{z}} [cm];Events", label), 200, -40, 40);

        // ========== ENHANCED DIAGNOSTIC HISTOGRAMS ==========

        // Vertex diagnostics
        h.vz_vs_phi = svc->book_atomic_hist2D(dir + "/vz_vs_phi",
            fmt::format("{} v_{{z}} vs #phi;#phi [deg];v_{{z}} [cm]", label), 180, -180, 180, 100, -40, 40);
        h.vz_vs_sector = svc->book_atomic_hist2D(dir + "/vz_vs_sector",
            fmt::format("{} v_{{z}} vs Sector;Sector;v_{{z}} [cm]", label), 6, 0.5, 6.5, 100, -40, 40);
        h.vz_vs_p = svc->book_atomic_hist2D(dir + "/vz_vs_p",
            fmt::format("{} v_{{z}} vs p;p [GeV/c];v_{{z}} [cm]", label), 100, 0, 10, 100, -40, 40);
        h.vx_vs_vy = svc->book_atomic_hist2D(dir + "/vx_vs_vy",
            fmt::format("{} v_{{x}} vs v_{{y}};v_{{x}} [cm];v_{{y}} [cm]", label), 100, -2, 2, 100, -2, 2);
        h.delta_vz = svc->book_atomic_hist1D(dir + "/delta_vz",
            fmt::format("{} #Deltav_{{z}} (#pi - e);#Deltav_{{z}} [cm];Events", label), 200, -20, 20);
        h.delta_vz_vs_p = svc->book_atomic_hist2D(dir + "/delta_vz_vs_p",
            fmt::format("{} #Deltav_{{z}} vs p;p [GeV/c];#Deltav_{{z}} [cm]", label), 100, 0, 10, 200, -20, 20);

        // Timing diagnostics (delta_t vs p)
        h.delta_t_vs_p = svc->book_atomic_hist2D(dir + "/delta_t_vs_p",
            fmt::format("{} #Deltat vs p;p [GeV/c];#Deltat [ns]", label), 100, 0, 10, 200, -2, 2);
        h.delta_t = svc->book_atomic_hist1D(dir + "/delta_t",
            fmt::format("{} #Deltat;#Deltat [ns];Events", label), 200, -2, 2);
        h.tof_mass2 = svc->book_atomic_hist1D(dir + "/tof_mass2",
            fmt::format("{} m^{{2}}_{{TOF}};m^{{2}} [GeV^{{2}}];Events", label), 200, -0.2, 0.5);
        h.tof_mass2_vs_p = svc->book_atomic_hist2D(dir + "/tof_mass2_vs_p",
            fmt::format("{} m^{{2}}_{{TOF}} vs p;p [GeV/c];m^{{2}} [GeV^{{2}}]", label), 100, 0, 10, 200, -0.2, 0.5);
        h.tof_path = svc->book_atomic_hist1D(dir + "/tof_path",
            fmt::format("{} TOF Path Length;Path [cm];Events", label), 100, 0, 800);
        h.tof_path_vs_p = svc->book_atomic_hist2D(dir + "/tof_path_vs_p",
            fmt::format("{} TOF Path vs p;p [GeV/c];Path [cm]", label), 100, 0, 10, 100, 0, 800);
        h.beta = svc->book_atomic_hist1D(dir + "/beta",
            fmt::format("{} #beta;#beta;Events", label), 100, 0.5, 1.1);
        h.beta_vs_p = svc->book_atomic_hist2D(dir + "/beta_vs_p",
            fmt::format("{} #beta vs p;p [GeV/c];#beta", label), 100, 0, 10, 100, 0.5, 1.1);
        h.beta_diff = svc->book_atomic_hist1D(dir + "/beta_diff",
            fmt::format("{} #Delta#beta (#beta_{{meas}} - #beta_{{exp}});#Delta#beta;Events", label), 200, -0.1, 0.1);
        h.beta_diff_vs_p = svc->book_atomic_hist2D(dir + "/beta_diff_vs_p",
            fmt::format("{} #Delta#beta vs p;p [GeV/c];#Delta#beta", label), 100, 0, 10, 200, -0.1, 0.1);

        // PID diagnostics
        h.chi2pid = svc->book_atomic_hist1D(dir + "/chi2pid",
            fmt::format("{} #chi^{{2}}_{{PID}};#chi^{{2}}_{{PID}};Events", label), 200, -10, 10);
        h.chi2pid_vs_p = svc->book_atomic_hist2D(dir + "/chi2pid_vs_p",
            fmt::format("{} #chi^{{2}}_{{PID}} vs p;p [GeV/c];#chi^{{2}}_{{PID}}", label), 100, 0, 10, 100, -10, 10);
        h.chi2pid_vs_theta = svc->book_atomic_hist2D(dir + "/chi2pid_vs_theta",
            fmt::format("{} #chi^{{2}}_{{PID}} vs #theta;#theta [deg];#chi^{{2}}_{{PID}}", label), 90, 0, 45, 100, -10, 10);
        h.chi2pid_vs_sector = svc->book_atomic_hist2D(dir + "/chi2pid_vs_sector",
            fmt::format("{} #chi^{{2}}_{{PID}} vs Sector;Sector;#chi^{{2}}_{{PID}}", label), 6, 0.5, 6.5, 100, -10, 10);

        // Phase space and acceptance
        h.phi_vs_theta = svc->book_atomic_hist2D(dir + "/phi_vs_theta",
            fmt::format("{} #phi vs #theta;#theta [deg];#phi [deg]", label), 90, 0, 45, 180, -180, 180);
        h.p_vs_phi = svc->book_atomic_hist2D(dir + "/p_vs_phi",
            fmt::format("{} p vs #phi;#phi [deg];p [GeV/c]", label), 180, -180, 180, 100, 0, 10);
        h.p_vs_theta = svc->book_atomic_hist2D(dir + "/p_vs_theta",
            fmt::format("{} p vs #theta;#theta [deg];p [GeV/c]", label), 90, 0, 45, 100, 0, 10);

        // Book histograms for each sector (1-6)
        for (int sector = 1; sector <= 6; ++sector) {
            std::string sec_suffix = fmt::format("_S{}", sector);

            h.sector_histograms.nphe[sector] = svc->book_atomic_hist1D(
                dir + "/nphe" + sec_suffix,
                fmt::format("Cherenkov nphe (Sector {});nphe;Tracks", sector),
                51, 0, 50);

            h.sector_histograms.EPCal_vs_ECal[sector] = svc->book_atomic_hist2D(
                dir + "/EPCal_vs_ECal" + sec_suffix,
                fmt::format("E_{{PCal}} vs E_{{Cal}} (Sector {});E_{{PCal}} [GeV];E_{{Cal}} [GeV]", sector),
                200, 0, 0.6, 200, 0, 0.6);

            h.sector_histograms.EPCal[sector] = svc->book_atomic_hist1D(
                dir + "/EPCal" + sec_suffix,
                fmt::format("E_{{PCal}} (Sector {});E_{{PCal}} [GeV];Tracks", sector),
                200, 0, 1);

            h.sector_histograms.p_vs_SF[sector] = svc->book_atomic_hist2D(
                dir + "/p_vs_SF" + sec_suffix,
                fmt::format("p vs SF (Sector {});p [GeV];E/p", sector),
                200, 0, 11, 200, 0, 0.4);

            h.sector_histograms.edge_vs_chi2ndf_r1[sector] = svc->book_atomic_hist2D(
                dir + "/edge_vs_chi2ndf_r1" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R1, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.sector_histograms.edge_vs_chi2ndf_r2[sector] = svc->book_atomic_hist2D(
                dir + "/edge_vs_chi2ndf_r2" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R2, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.sector_histograms.edge_vs_chi2ndf_r3[sector] = svc->book_atomic_hist2D(
                dir + "/edge_vs_chi2ndf_r3" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R3, Sector {});edge [cm];#chi^{{2}}/ndf", sector),
                200, 0, 50, 200, 0, 10);

            h.sector_histograms.lv_vs_SF[sector] = svc->book_atomic_hist2D(
                dir + "/lv_vs_SF" + sec_suffix,
                fmt::format("lv vs SF (Sector {});lv;E/p", sector),
                200, 0, 70, 200, 0, 0.5);
            h.sector_histograms.lw_vs_SF[sector] = svc->book_atomic_hist2D(
                dir + "/lw_vs_SF" + sec_suffix,
                fmt::format("lw vs SF (Sector {});lw;E/p", sector),
                200, 0, 70, 200, 0, 0.5);

            // Sector-level p vs theta
            h.sector_histograms.p_vs_theta[sector] = svc->book_atomic_hist2D(
                dir + "/p_vs_theta" + sec_suffix,
                fmt::format("{} p vs #theta (Sector {});#theta [deg];p [GeV/c]", label, sector),
                90, 0, 45, 100, 0, 10);

            // Per-sector delta_t vs p
            h.delta_t_vs_p_sector[sector - 1] = svc->book_atomic_hist2D(
                dir + "/delta_t_vs_p" + sec_suffix,
                fmt::format("{} #Deltat vs p (Sector {});p [GeV/c];#Deltat [ns]", label, sector),
                100, 0, 10, 200, -2, 2);

            // Per-sector p vs theta (acceptance maps)
            h.p_vs_theta_sector[sector - 1] = svc->book_atomic_hist2D(
                dir + "/p_vs_theta_acceptance" + sec_suffix,
                fmt::format("{} p vs #theta Acceptance (Sector {});#theta [deg];p [GeV/c]", label, sector),
                90, 0, 45, 100, 0, 10);
        }

        h.sector_histograms.booked = true;

    }

    void process_pion(const clas12::Particle& pion, hipo::bank& REC_Calorimeter, hipo::bank& REC_Traj, hipo::bank& REC_Track,
                      hipo::bank& REC_Scintillator, double start_time, double electron_vz, bool is_piplus) {
        auto& cutflow = is_piplus ? m_cutflow_piplus : m_cutflow_piminus;
        auto& h = is_piplus ? h_piplus : h_piminus;

        cutflow.pass("All Pions");

        // Pion in the forward detector (2000<=|status| < 4000)
        int status = pion.status();
        bool in_forward_detector = (std::abs(status) >= 2000 && std::abs(status) < 4000);

        // Read DC track and trajectory info BEFORE cuts for pre-cut histograms
        auto track = clas12::read_Track_bank(REC_Track, pion.index());
        auto traj = clas12::read_Traj_bank(REC_Traj, pion.index());

        double chi2ndf = (track.ndf > 0) ? track.chi2 / track.ndf : 0.0;
        double edge_r1 = traj.region1.edge;
        double edge_r2 = traj.region2.edge;
        double edge_r3 = traj.region3.edge;

        // Fill PRE-CUT DC edge histograms (for candidates in forward detector)
        if (in_forward_detector) {
            if (h.edge_r1_precut) h.edge_r1_precut->fill(edge_r1);
            if (h.edge_r2_precut) h.edge_r2_precut->fill(edge_r2);
            if (h.edge_r3_precut) h.edge_r3_precut->fill(edge_r3);
            if (h.chi2ndf_vs_edge_r1_precut) h.chi2ndf_vs_edge_r1_precut->fill(edge_r1, chi2ndf);
            if (h.chi2ndf_vs_edge_r2_precut) h.chi2ndf_vs_edge_r2_precut->fill(edge_r2, chi2ndf);
            if (h.chi2ndf_vs_edge_r3_precut) h.chi2ndf_vs_edge_r3_precut->fill(edge_r3, chi2ndf);
        }

        // Apply forward detector cut
        if (!in_forward_detector) return;
        cutflow.pass("In the forward detector");

        // Basic momentum cut (using property)
        double p = pion.p();
        if (p < m_min_momentum.get()) return;
        cutflow.pass("Momentum > 0.3 GeV");

        // Read calorimeter info
        auto calo = clas12::read_Calorimeter_bank(REC_Calorimeter, pion.index());
        double E_tot = calo.pcal.energy + calo.inner.energy + calo.outer.energy;

        bool has_calo = (E_tot > m_min_calo_energy.get());
        if (!has_calo) return;
        cutflow.pass("Has Calorimeter");

        bool has_dc = (track.ndf > 0);
        if (!has_dc) return;
        cutflow.pass("Has DC Track");

        // Calculate quantities
        double SF = E_tot / p;
        double lv = calo.pcal.lv;
        double lw = calo.pcal.lw;

        // Cherenkov nphe (if available)
        auto cherenkov = clas12::read_Cherenkov_bank(REC_Calorimeter, pion.index());
        double nphe = cherenkov.nphe;

        // Read scintillator info for timing
        auto scintillator = clas12::read_Scintillator_bank(REC_Scintillator, pion.index());
        int sector = scintillator.sector;
        double tof_time = scintillator.time;
        double tof_path = scintillator.path;

        // Calculate timing quantities
        double pion_mass = clas12::Constants::PIMASS;
        double beta_expected = p / std::sqrt(p * p + pion_mass * pion_mass);
        double t_expected = tof_path / (beta_expected * clas12::Constants::SPEED_OF_LIGHT);
        double measured_time = tof_time - start_time;
        double delta_t = measured_time - t_expected;
        double beta_measured = pion.beta();

        // Calculate TOF mass^2: m^2 = p^2 * (1/beta^2 - 1)
        double tof_mass2 = 0.0;
        if (beta_measured > 0) {
            tof_mass2 = p * p * (1.0 / (beta_measured * beta_measured) - 1.0);
        }

        // Calculate beta difference
        double beta_diff = beta_measured - beta_expected;

        // Calculate delta Vz (pion - electron)
        double delta_vz = pion.vz() - electron_vz;

        // Fill histograms (lock-free atomic)
        h.momentum->fill(p);
        h.theta->fill(pion.theta());
        if (h.phi) h.phi->fill(pion.phi());
        if (h.vz) h.vz->fill(pion.vz());

        // Calorimeter histograms
        h.sf->fill(SF);
        h.lv->fill(lv);
        h.lw->fill(lw);
        h.sf_vs_lv->fill(lv, SF);
        h.sf_vs_lw->fill(lw, SF);

        // DC histograms
        h.chi2ndf->fill(chi2ndf);
        h.edge_r1->fill(edge_r1);
        h.edge_r2->fill(edge_r2);
        h.edge_r3->fill(edge_r3);
        h.chi2ndf_vs_edge_r1->fill(edge_r1, chi2ndf);
        h.chi2ndf_vs_edge_r2->fill(edge_r2, chi2ndf);
        h.chi2ndf_vs_edge_r3->fill(edge_r3, chi2ndf);

        // ========== ENHANCED DIAGNOSTIC HISTOGRAMS ==========

        // Vertex diagnostics
        if (h.vz_vs_phi) h.vz_vs_phi->fill(pion.phi(), pion.vz());
        if (h.vz_vs_p) h.vz_vs_p->fill(p, pion.vz());
        if (h.vx_vs_vy) h.vx_vs_vy->fill(pion.vx(), pion.vy());
        if (h.delta_vz) h.delta_vz->fill(delta_vz);
        if (h.delta_vz_vs_p) h.delta_vz_vs_p->fill(p, delta_vz);

        // Timing diagnostics (delta_t vs p)
        if (h.delta_t_vs_p) h.delta_t_vs_p->fill(p, delta_t);
        if (h.delta_t) h.delta_t->fill(delta_t);
        if (h.tof_mass2) h.tof_mass2->fill(tof_mass2);
        if (h.tof_mass2_vs_p) h.tof_mass2_vs_p->fill(p, tof_mass2);
        if (h.tof_path) h.tof_path->fill(tof_path);
        if (h.tof_path_vs_p) h.tof_path_vs_p->fill(p, tof_path);
        if (h.beta) h.beta->fill(beta_measured);
        if (h.beta_vs_p) h.beta_vs_p->fill(p, beta_measured);
        if (h.beta_diff) h.beta_diff->fill(beta_diff);
        if (h.beta_diff_vs_p) h.beta_diff_vs_p->fill(p, beta_diff);

        // PID diagnostics
        if (h.chi2pid) h.chi2pid->fill(pion.chi2pid());
        if (h.chi2pid_vs_p) h.chi2pid_vs_p->fill(p, pion.chi2pid());
        if (h.chi2pid_vs_theta) h.chi2pid_vs_theta->fill(pion.theta(), pion.chi2pid());

        // Phase space and acceptance
        if (h.phi_vs_theta) h.phi_vs_theta->fill(pion.theta(), pion.phi());
        if (h.p_vs_phi) h.p_vs_phi->fill(pion.phi(), p);
        if (h.p_vs_theta) h.p_vs_theta->fill(pion.theta(), p);

        // Fill sector-specific histograms
        if (sector >= 1 && sector <= 6) {
            int idx = sector - 1;

            // Vertex vs sector
            if (h.vz_vs_sector) h.vz_vs_sector->fill(sector, pion.vz());

            // PID vs sector
            if (h.chi2pid_vs_sector) h.chi2pid_vs_sector->fill(sector, pion.chi2pid());

            // Per-sector delta_t vs p
            if (h.delta_t_vs_p_sector[idx]) h.delta_t_vs_p_sector[idx]->fill(p, delta_t);

            // Per-sector p vs theta (acceptance)
            if (h.p_vs_theta_sector[idx]) h.p_vs_theta_sector[idx]->fill(pion.theta(), p);
        }

        // Fill existing sector-specific histograms (calorimeter)
        if (h.sector_histograms.booked) {
            int calo_sector = calo.pcal.sector;
            if (calo_sector < 1 || calo_sector > 6) return;
            // Edge vs chi2/ndf
            h.sector_histograms.edge_vs_chi2ndf_r1[calo_sector]->fill(edge_r1, chi2ndf);
            h.sector_histograms.edge_vs_chi2ndf_r2[calo_sector]->fill(edge_r2, chi2ndf);
            h.sector_histograms.edge_vs_chi2ndf_r3[calo_sector]->fill(edge_r3, chi2ndf);

            // Calorimeter quantities
            double E_cal = calo.inner.energy + calo.outer.energy;
            double E_pcal = calo.pcal.energy;
            if (E_pcal == 0) return; // Should not happen for pions with calorimeter info
            h.sector_histograms.lv_vs_SF[calo_sector]->fill(lv, SF);
            h.sector_histograms.lw_vs_SF[calo_sector]->fill(lw, SF);
            h.sector_histograms.nphe[calo_sector]->fill(nphe);
            h.sector_histograms.EPCal_vs_ECal[calo_sector]->fill(E_pcal, E_cal);
            h.sector_histograms.EPCal[calo_sector]->fill(E_pcal);
            h.sector_histograms.p_vs_SF[calo_sector]->fill(p, SF);

            // Sector-level p vs theta
            if (h.sector_histograms.p_vs_theta.count(calo_sector)) {
                h.sector_histograms.p_vs_theta[calo_sector]->fill(pion.theta(), p);
            }
        }
    }
};

}  // namespace clas12::algorithms
