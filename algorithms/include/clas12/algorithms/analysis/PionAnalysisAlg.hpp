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
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/CandidateUtils.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/HistogramFiller.hpp>
#include <clas12/framework/core/HistogramTemplates.hpp>
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
        declare_property(m_enable_forward_detector_cut);
        declare_property(m_enable_momentum_cut);
        declare_property(m_enable_calorimeter_cut);
        declare_property(m_enable_dc_track_cut);
        // Declare required banks
        declare_bank("REC::Event");
        declare_bank("REC::Particle");
        declare_bank("REC::Calorimeter");
        declare_bank("REC::Traj");
        declare_bank("REC::Track");
        declare_bank("REC::Scintillator");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hist = svc<clas12::framework::HistogramService>();
        m_particle = svc<clas12::framework::ParticleSelectorService>();

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
        auto electron_opt = clas12::framework::EventHelpers::get_selected_electron(event);
        if (!electron_opt) return clas12::framework::StatusCode::FILTERED();
        double electron_vz = electron_opt->vz();

        const auto& REC_Event = event.get_bank("REC::Event");
        const auto& REC_Particle = event.get_bank("REC::Particle");
        const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");
        const auto& REC_Traj = event.get_bank("REC::Traj");
        const auto& REC_Track = event.get_bank("REC::Track");
        const auto& REC_Scintillator = event.get_bank("REC::Scintillator");

        if (REC_Event.rows() == 0) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Get start time for timing calculations
        double start_time = static_cast<double>(REC_Event.get<float>("startTime", 0));

        // Extract pions
        auto particles = m_particle->extract_particles(
            REC_Particle, {clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS});

        // Process π⁺
        for (const auto& pion : m_particle->get_pions_plus(particles)) {
            process_pion(pion, REC_Calorimeter, REC_Traj, REC_Track, REC_Scintillator, start_time, electron_vz, true);
        }

        // Process π⁻
        for (const auto& pion : m_particle->get_pions_minus(particles)) {
            process_pion(pion, REC_Calorimeter, REC_Traj, REC_Track, REC_Scintillator, start_time, electron_vz, false);
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
        Log::info(name(), "     - π⁺ with calorimeter info: {}", m_cutflow_piplus.count("Has Calorimeter"));
        Log::info(name(), "     - π⁻ with calorimeter info: {}", m_cutflow_piminus.count("Has Calorimeter"));

        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::Target m_target;
    clas12::Polarity m_polarity;

    struct PionCutContext {
        bool in_forward_detector = false;
        double p = 0.0;
        double E_tot = 0.0;
        bool has_dc = false;
    };

    clas12::framework::ContextCutflow<PionCutContext> m_cutflow_piplus;
    clas12::framework::ContextCutflow<PionCutContext> m_cutflow_piminus;

    // Properties
    clas12::framework::Property<double> m_min_momentum{"min_momentum", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_min_calo_energy{"min_calo_energy", 0.01, "Minimum calorimeter energy [GeV]"};

    // Cut toggle properties
    clas12::framework::Property<bool> m_enable_forward_detector_cut{"enable_forward_detector_cut", true, "Enable forward detector cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_calorimeter_cut{"enable_calorimeter_cut", true, "Enable calorimeter cut"};
    clas12::framework::Property<bool> m_enable_dc_track_cut{"enable_dc_track_cut", true, "Enable DC track cut"};

    // Services (cached pointers from svc<T>())
    clas12::framework::HistogramService* m_hist = nullptr;
    clas12::framework::ParticleSelectorService* m_particle = nullptr;

    // Lock-free atomic histogram set for each pion charge
    struct PionHistograms {
        // Base diagnostics: kinematics + vertex + timing + PID + phase space (from PionDiagnostics template)
        std::unique_ptr<clas12::framework::AtomicHistogramGroup> base;

        // Calorimeter-specific (post-cut)
        clas12::framework::AtomicHistogram1D* sf = nullptr;
        clas12::framework::AtomicHistogram1D* lv = nullptr;
        clas12::framework::AtomicHistogram1D* lw = nullptr;
        clas12::framework::AtomicHistogram2D* sf_vs_lv = nullptr;
        clas12::framework::AtomicHistogram2D* sf_vs_lw = nullptr;

        // DC post-cut
        clas12::framework::AtomicHistogram1D* chi2ndf = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r1 = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r2 = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r3 = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r1 = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r2 = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r3 = nullptr;

        // DC pre-cut (for efficiency studies)
        clas12::framework::AtomicHistogram1D* edge_r1_precut = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r2_precut = nullptr;
        clas12::framework::AtomicHistogram1D* edge_r3_precut = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r1_precut = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r2_precut = nullptr;
        clas12::framework::AtomicHistogram2D* chi2ndf_vs_edge_r3_precut = nullptr;

        // Per-sector arrays
        std::array<clas12::framework::AtomicHistogram2D*, 6> delta_t_vs_p_sector = {};
        std::array<clas12::framework::AtomicHistogram2D*, 6> p_vs_theta_sector = {};

        // Sector-level histograms
        struct SectorHistograms {
            bool booked = false;
            std::map<int, clas12::framework::AtomicHistogram1D*> nphe;
            std::map<int, clas12::framework::AtomicHistogram2D*> EPCal_vs_ECal;
            std::map<int, clas12::framework::AtomicHistogram1D*> EPCal;
            std::map<int, clas12::framework::AtomicHistogram2D*> p_vs_SF;
            std::map<int, clas12::framework::AtomicHistogram2D*> lv_vs_SF;
            std::map<int, clas12::framework::AtomicHistogram2D*> lw_vs_SF;
            std::map<int, clas12::framework::AtomicHistogram2D*> edge_vs_chi2ndf_r1;
            std::map<int, clas12::framework::AtomicHistogram2D*> edge_vs_chi2ndf_r2;
            std::map<int, clas12::framework::AtomicHistogram2D*> edge_vs_chi2ndf_r3;
            std::map<int, clas12::framework::AtomicHistogram2D*> p_vs_theta;
        };
        SectorHistograms sector_histograms;
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    void setup_cutflows() {
        auto define = [this](clas12::framework::ContextCutflow<PionCutContext>& cf) {
            cf.define_always("All Pions")
              .define("In the forward detector", [this](const PionCutContext& c) {
                  return !m_enable_forward_detector_cut.get() || c.in_forward_detector;
              })
              .define("Momentum > 0.3 GeV", [this](const PionCutContext& c) {
                  return !m_enable_momentum_cut.get() || c.p >= m_min_momentum.get();
              })
              .define("Has Calorimeter", [this](const PionCutContext& c) {
                  return !m_enable_calorimeter_cut.get() || c.E_tot > m_min_calo_energy.get();
              })
              .define("Has DC Track", [this](const PionCutContext& c) {
                  return !m_enable_dc_track_cut.get() || c.has_dc;
              });
        };
        define(m_cutflow_piplus);
        define(m_cutflow_piminus);
    }

    void book_histograms() {
        if (!m_hist) return;
        book_pion_histograms("/PionAnalysis/PiPlus", h_piplus, "#pi^{+}");
        book_pion_histograms("/PionAnalysis/PiMinus", h_piminus, "#pi^{-}");
    }

    void book_pion_histograms(const std::string& dir, PionHistograms& h, const std::string& label) {
        auto* svc = m_hist;

        // Base diagnostics: kinematics + vertex + timing + PID + phase space (27 histograms)
        h.base = clas12::framework::AtomicHistogramTemplates::PionDiagnostics::create(svc, dir, label);

        // Calorimeter-specific (post-cut)
        h.sf = svc->book_atomic_hist1D(dir + "/sf", fmt::format("{} Sampling Fraction;SF;Events", label), 100, 0, 0.4);
        h.lv = svc->book_atomic_hist1D(dir + "/lv", fmt::format("{} PCAL lv;lv [cm];Events", label), 100, 0, 500);
        h.lw = svc->book_atomic_hist1D(dir + "/lw", fmt::format("{} PCAL lw;lw [cm];Events", label), 100, 0, 500);
        h.sf_vs_lv = svc->book_atomic_hist2D(dir + "/sf_vs_lv", fmt::format("{} Sf vs lv;lv [cm];SF", label), 200, 0, 70, 200, 0, 0.4);
        h.sf_vs_lw = svc->book_atomic_hist2D(dir + "/sf_vs_lw", fmt::format("{} Sf vs lw;lw [cm];SF", label), 200, 0, 70, 200, 0, 0.4);

        // DC post-cut
        h.chi2ndf = svc->book_atomic_hist1D(dir + "/chi2ndf", fmt::format("{} DC #chi^{{2}}/NDF;#chi^{{2}}/NDF;Events", label), 100, 0, 20);
        h.edge_r1 = svc->book_atomic_hist1D(dir + "/edge_r1", fmt::format("{} DC Edge R1;Edge [cm];Events", label), 100, -10, 50);
        h.edge_r2 = svc->book_atomic_hist1D(dir + "/edge_r2", fmt::format("{} DC Edge R2;Edge [cm];Events", label), 100, -10, 50);
        h.edge_r3 = svc->book_atomic_hist1D(dir + "/edge_r3", fmt::format("{} DC Edge R3;Edge [cm];Events", label), 100, -10, 50);
        h.chi2ndf_vs_edge_r1 = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r1", fmt::format("{} #chi^{{2}}/NDF vs Edge R1;Edge R1 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r2 = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r2", fmt::format("{} #chi^{{2}}/NDF vs Edge R2;Edge R2 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r3 = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r3", fmt::format("{} #chi^{{2}}/NDF vs Edge R3;Edge R3 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);

        // DC pre-cut (for efficiency studies)
        h.edge_r1_precut = svc->book_atomic_hist1D(dir + "/edge_r1_precut", fmt::format("{} DC Edge R1 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.edge_r2_precut = svc->book_atomic_hist1D(dir + "/edge_r2_precut", fmt::format("{} DC Edge R2 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.edge_r3_precut = svc->book_atomic_hist1D(dir + "/edge_r3_precut", fmt::format("{} DC Edge R3 (pre-cut);Edge [cm];Events", label), 100, -10, 50);
        h.chi2ndf_vs_edge_r1_precut = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r1_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R1 (pre-cut);Edge R1 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r2_precut = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r2_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R2 (pre-cut);Edge R2 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);
        h.chi2ndf_vs_edge_r3_precut = svc->book_atomic_hist2D(dir + "/chi2ndf_vs_edge_r3_precut", fmt::format("{} #chi^{{2}}/NDF vs Edge R3 (pre-cut);Edge R3 [cm];#chi^{{2}}/NDF", label), 200, 0, 50, 200, 0, 100);

        // Per-sector histograms
        for (int sector = 1; sector <= 6; ++sector) {
            std::string sec_suffix = fmt::format("_S{}", sector);

            h.sector_histograms.nphe[sector] = svc->book_atomic_hist1D(
                dir + "/nphe" + sec_suffix,
                fmt::format("Cherenkov nphe (Sector {});nphe;Tracks", sector), 51, 0, 50);
            h.sector_histograms.EPCal_vs_ECal[sector] = svc->book_atomic_hist2D(
                dir + "/EPCal_vs_ECal" + sec_suffix,
                fmt::format("E_{{PCal}} vs E_{{Cal}} (Sector {});E_{{PCal}} [GeV];E_{{Cal}} [GeV]", sector), 200, 0, 0.6, 200, 0, 0.6);
            h.sector_histograms.EPCal[sector] = svc->book_atomic_hist1D(
                dir + "/EPCal" + sec_suffix,
                fmt::format("E_{{PCal}} (Sector {});E_{{PCal}} [GeV];Tracks", sector), 200, 0, 1);
            h.sector_histograms.p_vs_SF[sector] = svc->book_atomic_hist2D(
                dir + "/p_vs_SF" + sec_suffix,
                fmt::format("p vs SF (Sector {});p [GeV];E/p", sector), 200, 0, 11, 200, 0, 0.4);
            h.sector_histograms.edge_vs_chi2ndf_r1[sector] = svc->book_atomic_hist2D(
                dir + "/edge_vs_chi2ndf_r1" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R1, Sector {});edge [cm];#chi^{{2}}/ndf", sector), 200, 0, 50, 200, 0, 10);
            h.sector_histograms.edge_vs_chi2ndf_r2[sector] = svc->book_atomic_hist2D(
                dir + "/edge_vs_chi2ndf_r2" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R2, Sector {});edge [cm];#chi^{{2}}/ndf", sector), 200, 0, 50, 200, 0, 10);
            h.sector_histograms.edge_vs_chi2ndf_r3[sector] = svc->book_atomic_hist2D(
                dir + "/edge_vs_chi2ndf_r3" + sec_suffix,
                fmt::format("Edge vs #chi^{{2}}/ndf (R3, Sector {});edge [cm];#chi^{{2}}/ndf", sector), 200, 0, 50, 200, 0, 10);
            h.sector_histograms.lv_vs_SF[sector] = svc->book_atomic_hist2D(
                dir + "/lv_vs_SF" + sec_suffix,
                fmt::format("lv vs SF (Sector {});lv;E/p", sector), 200, 0, 70, 200, 0, 0.5);
            h.sector_histograms.lw_vs_SF[sector] = svc->book_atomic_hist2D(
                dir + "/lw_vs_SF" + sec_suffix,
                fmt::format("lw vs SF (Sector {});lw;E/p", sector), 200, 0, 70, 200, 0, 0.5);
            h.sector_histograms.p_vs_theta[sector] = svc->book_atomic_hist2D(
                dir + "/p_vs_theta" + sec_suffix,
                fmt::format("{} p vs #theta (Sector {});#theta [deg];p [GeV/c]", label, sector), 90, 0, 45, 100, 0, 10);
            h.delta_t_vs_p_sector[sector - 1] = svc->book_atomic_hist2D(
                dir + "/delta_t_vs_p" + sec_suffix,
                fmt::format("{} #Deltat vs p (Sector {});p [GeV/c];#Deltat [ns]", label, sector), 100, 0, 10, 200, -2, 2);
            h.p_vs_theta_sector[sector - 1] = svc->book_atomic_hist2D(
                dir + "/p_vs_theta_acceptance" + sec_suffix,
                fmt::format("{} p vs #theta Acceptance (Sector {});#theta [deg];p [GeV/c]", label, sector), 90, 0, 45, 100, 0, 10);
        }

        h.sector_histograms.booked = true;
    }

    void process_pion(const clas12::Particle& pion, const hipo::bank_view& REC_Calorimeter, const hipo::bank_view& REC_Traj, const hipo::bank_view& REC_Track,
                      const hipo::bank_view& REC_Scintillator, double start_time, double electron_vz, bool is_piplus) {
        auto& cutflow = is_piplus ? m_cutflow_piplus : m_cutflow_piminus;
        auto& h = is_piplus ? h_piplus : h_piminus;

        // Read bank data needed for cuts and pre-cut histograms
        auto track = clas12::read_Track_bank(REC_Track, pion.index());
        auto traj = clas12::read_Traj_bank(REC_Traj, pion.index());
        auto calo = clas12::read_Calorimeter_bank(REC_Calorimeter, pion.index());

        double chi2ndf = (track.ndf > 0) ? track.chi2 / track.ndf : 0.0;
        double edge_r1 = traj.region1.edge;
        double edge_r2 = traj.region2.edge;
        double edge_r3 = traj.region3.edge;
        double E_tot = calo.pcal.energy + calo.inner.energy + calo.outer.energy;
        bool in_forward_detector = clas12::framework::CandidateUtils::is_forward_detector(pion.status());

        // Fill PRE-CUT DC edge histograms (for candidates in forward detector)
        if (in_forward_detector) {
            if (h.edge_r1_precut) h.edge_r1_precut->fill(edge_r1);
            if (h.edge_r2_precut) h.edge_r2_precut->fill(edge_r2);
            if (h.edge_r3_precut) h.edge_r3_precut->fill(edge_r3);
            if (h.chi2ndf_vs_edge_r1_precut) h.chi2ndf_vs_edge_r1_precut->fill(edge_r1, chi2ndf);
            if (h.chi2ndf_vs_edge_r2_precut) h.chi2ndf_vs_edge_r2_precut->fill(edge_r2, chi2ndf);
            if (h.chi2ndf_vs_edge_r3_precut) h.chi2ndf_vs_edge_r3_precut->fill(edge_r3, chi2ndf);
        }

        // Evaluate all cuts declaratively
        PionCutContext ctx{in_forward_detector, pion.p(), E_tot, track.ndf > 0};
        if (!cutflow.evaluate(ctx)) return;

        // Calculate quantities
        double p = pion.p();
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
        auto timing = clas12::framework::CandidateUtils::compute_timing(
            p, clas12::Constants::PIMASS, pion.beta(),
            tof_time, tof_path, start_time);

        // Fill base diagnostics (kinematics + vertex + timing + PID + phase space) in one call
        clas12::framework::AtomicHistogramFiller::fill_pion_diagnostics(
            h.base.get(), pion, timing, electron_vz, tof_path, sector);

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

        // Per-sector histograms
        if (sector >= 1 && sector <= 6) {
            int idx = sector - 1;
            if (h.delta_t_vs_p_sector[idx]) h.delta_t_vs_p_sector[idx]->fill(p, timing.delta_t);
            if (h.p_vs_theta_sector[idx]) h.p_vs_theta_sector[idx]->fill(pion.theta(), p);
        }

        // Sector-level calorimeter histograms
        if (h.sector_histograms.booked) {
            int calo_sector = calo.pcal.sector;
            if (calo_sector < 1 || calo_sector > 6) return;
            h.sector_histograms.edge_vs_chi2ndf_r1[calo_sector]->fill(edge_r1, chi2ndf);
            h.sector_histograms.edge_vs_chi2ndf_r2[calo_sector]->fill(edge_r2, chi2ndf);
            h.sector_histograms.edge_vs_chi2ndf_r3[calo_sector]->fill(edge_r3, chi2ndf);

            double E_cal = calo.inner.energy + calo.outer.energy;
            double E_pcal = calo.pcal.energy;
            if (E_pcal == 0) return;
            h.sector_histograms.lv_vs_SF[calo_sector]->fill(lv, SF);
            h.sector_histograms.lw_vs_SF[calo_sector]->fill(lw, SF);
            h.sector_histograms.nphe[calo_sector]->fill(nphe);
            h.sector_histograms.EPCal_vs_ECal[calo_sector]->fill(E_pcal, E_cal);
            h.sector_histograms.EPCal[calo_sector]->fill(E_pcal);
            h.sector_histograms.p_vs_SF[calo_sector]->fill(p, SF);

            if (h.sector_histograms.p_vs_theta.count(calo_sector)) {
                h.sector_histograms.p_vs_theta[calo_sector]->fill(pion.theta(), p);
            }
        }
    }
};

}  // namespace clas12::algorithms
