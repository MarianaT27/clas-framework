#pragma once

/**
 * @file ElectronSelectionAlg.hpp
 * @brief Algorithm for electron selection and kinematics analysis
 *
 * This algorithm selects good electrons from CLAS12 data using:
 * - Particle identification cuts (chi2pid)
 * - Kinematic cuts (momentum, vertex z)
 * - Sampling fraction cuts (calorimeter response)
 * - Fiducial cuts (PCAL and drift chamber edge cuts via FiducialService)
 *
 * Uses modern framework patterns:
 * - ContextCutflow for combined cut definition and tracking
 * - NTupleWriter for struct-based N-tuple management
 * - FiducialService for unified fiducial cuts
 *
 * @author CLAS12 Analysis Framework
 */

#include <optional>
#include <vector>

#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Histogram.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

// ============================================================================
// N-Tuple Data Structure
// ============================================================================

/**
 * @brief N-Tuple data structure for electron kinematics
 *
 * Branches are listed in `ntuple_fields()`; NTupleWriter wires each one
 * to the matching member at init() time.
 */
struct ElectronNTupleData {
    double e_p       = 0.0;  ///< Electron momentum [GeV/c]
    double e_theta   = 0.0;  ///< Electron polar angle [rad]
    double e_phi     = 0.0;  ///< Electron azimuthal angle [rad]
    double e_vz      = 0.0;  ///< Electron vertex z [cm]
    double e_chi2pid = 0.0;  ///< Electron chi2pid
    double e_sf      = 0.0;  ///< Sampling fraction
    int e_sector     = 0;    ///< PCAL sector (1-6)

    static constexpr auto ntuple_fields() {
        using T = ElectronNTupleData;
        return std::make_tuple(
            clas12::framework::field("e_p", &T::e_p),
            clas12::framework::field("e_theta", &T::e_theta),
            clas12::framework::field("e_phi", &T::e_phi),
            clas12::framework::field("e_vz", &T::e_vz),
            clas12::framework::field("e_chi2pid", &T::e_chi2pid),
            clas12::framework::field("e_sf", &T::e_sf),
            clas12::framework::field("e_sector", &T::e_sector)
        );
    }
};

// ============================================================================
// Cut Context
// ============================================================================

/**
 * @brief Context for electron cut evaluation
 *
 * Holds all per-event data needed for cut decisions, enabling
 * the ContextCutflow to evaluate cuts with full context access.
 */
struct ElectronCutContext {
    const clas12::Particle& electron;       ///< The electron candidate
    const clas12::CalorimeterBank& calo;    ///< Calorimeter data
    const clas12::DCTrajBank& traj;         ///< Drift chamber trajectory
    double SF;                               ///< Sampling fraction (E_total/p)
    int sector;                              ///< PCAL sector (1-6)
    clas12::framework::FiducialResult fiducial_result;  ///< Pre-computed fiducial result
};

// ============================================================================
// Algorithm Implementation
// ============================================================================

/**
 * @brief Algorithm for electron selection and kinematics
 *
 * This algorithm implements a complete electron selection pipeline for
 * CLAS12 forward detector data. It combines:
 *
 * **Particle ID Cuts:**
 * - Chi2PID within configurable range (default: [-5, 5])
 *
 * **Kinematic Cuts:**
 * - Minimum momentum (default: 0.8 GeV)
 * - Vertex z within target-specific range
 *
 * **Calorimeter Cuts:**
 * - Sampling fraction within n-sigma band (default: 3.5 sigma)
 *
 * **Fiducial Cuts (via FiducialService):**
 * - PCAL lv/lw coordinates (default: > 9 cm)
 * - DC edge distances for regions 1, 2, 3
 *
 * The algorithm outputs:
 * - Histograms of electron kinematics (before/after cuts)
 * - Optional N-tuple with per-electron data
 * - Cutflow summary showing cut efficiencies
 *
 * @see FiducialService for fiducial cut configuration
 * @see ElectronCutsService for PID and kinematic cut configuration
 */
class ElectronSelectionAlg : public clas12::framework::Algorithm {
public:
    // ========================================================================
    // Construction
    // ========================================================================

    /**
     * @brief Construct electron selection algorithm
     *
     * @param name Algorithm name (default: "ElectronSelectionAlg")
     * @param target Target material (affects vertex z cuts)
     * @param polarity Torus polarity (affects sampling fraction)
     */
    explicit ElectronSelectionAlg(
        std::string name = "ElectronSelectionAlg",
        clas12::Target target = clas12::Target::LD2,
        clas12::Polarity polarity = clas12::Polarity::Outbending)
        : Algorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow("Electron Selection") {
        declare_property(m_enable_ntuple);
        declare_property(m_enable_fiducial_histograms);
        declare_property(m_enable_chi2pid_cut);
        declare_property(m_enable_momentum_cut);
        declare_property(m_enable_vz_cut);
        declare_property(m_enable_sf_cut);
        declare_property(m_enable_pcal_fiducial_cut);
        declare_property(m_enable_dc_r1_cut);
        declare_property(m_enable_dc_r2_cut);
        declare_property(m_enable_dc_r3_cut);
        declare_bank("REC::Calorimeter");
        declare_bank("REC::Traj");
        declare_bank("REC::Track");
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    /**
     * @brief Initialize algorithm after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        // Cache service pointers
        m_hist = svc<clas12::framework::HistogramService>();
        m_ntuple_svc = svc<clas12::framework::NTupleService>();
        m_sf = svc<clas12::framework::SamplingFractionService>();
        m_electron_cuts = svc<clas12::framework::ElectronCutsService>();
        m_fiducial = svc<clas12::framework::FiducialService>();

        Log::subsection(fmt::format("{} Configuration", name()));
        Log::config(name(), "Target", clas12::to_string(m_target));
        Log::config(name(), "Polarity", clas12::to_string(m_polarity));
        Log::config(name(), "NTuple output", m_enable_ntuple.get() ? "enabled" : "disabled");
        Log::config(name(), "Fiducial histograms", m_enable_fiducial_histograms.get() ? "enabled" : "disabled");

        // Configure ElectronCutsService
        configure_electron_cuts_service();

        // Book histograms
        book_histograms();

        // Setup N-Tuple
        if (m_enable_ntuple.get() && m_ntuple_svc) {
            m_ntuple.init(m_ntuple_svc, "electron_kinematics");
        }

        // Define cutflow with predicates
        define_cutflow();

        // Log configuration summary
        auto vz_range = m_electron_cuts->get_vz_range(clas12::RunGroup::RG_D, m_target);
        Log::config(name(), "Vz range", fmt::format("[{:.2f}, {:.2f}] cm", vz_range.min, vz_range.max));
        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return clas12::framework::StatusCode::SUCCESS();
    }

    // ========================================================================
    // Event Processing
    // ========================================================================

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        if (event.electrons().empty()) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }

        // Select trigger electron (highest momentum in FD)
        const auto possible_electron =
            event.find_trigger_electron({clas12::FD, clas12::BD});

        if (!possible_electron.has_value()) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        const clas12::Particle& electron = possible_electron.value();

        // Read detector bank data. Each accessor reuses the per-event
        // generated-wrapper cache on Event, so the three read_Track_bank
        // calls below share a single binding + linker.
        const auto calorimeter = electron.calo();
        const auto traj = electron.traj();
        const auto trk_r1 = clas12::read_Track_bank(event, traj.region1_index);
        const auto trk_r2 = clas12::read_Track_bank(event, traj.region2_index);
        const auto trk_r3 = clas12::read_Track_bank(event, traj.region3_index);

        // Calculate derived quantities
        const double E_total = calorimeter.pcal_energy + calorimeter.inner_energy + calorimeter.outer_energy;
        const double SF = E_total / electron.p();
        const int sector = calorimeter.pcal_sector;

        // Pre-compute fiducial result using FiducialService
        auto fiducial_result = m_fiducial->evaluate(electron, calorimeter, traj, sector);

        // Fill histograms before cuts
        h_before.fill(electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

        // Fill fiducial diagnostic histograms
        if (m_enable_fiducial_histograms.get()) {
            fill_fiducial_histograms(calorimeter, traj, fiducial_result, "before_cuts");
        }

        // Create context for cut evaluation
        ElectronCutContext ctx{electron, calorimeter, traj, SF, sector, fiducial_result};

        // Evaluate all cuts via cutflow
        if (!m_cutflow.evaluate(ctx)) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }

        // Electron passed all cuts. Publish both the vector form (legacy
        // callers) and the single-particle TriggerElectron key so
        // downstream algorithms can pick whichever shape fits.
        event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, true);
        event.store_typed(clas12::framework::Keys::SelectedElectrons,
                          std::vector<clas12::Particle>{electron});
        event.store_typed(clas12::framework::Keys::TriggerElectron, electron);

        // Fill histograms after cuts
        h_after.fill(electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

        if (m_enable_fiducial_histograms.get()) {
            fill_fiducial_histograms(calorimeter, traj, fiducial_result, "after_cuts");
        }

        // Fill N-tuple
        fill_ntuple(electron, SF, sector);

        return clas12::framework::StatusCode::SUCCESS();
    }

    // ========================================================================
    // Finalization
    // ========================================================================

    /**
     * @brief Finalization - print summaries
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::subsection(fmt::format("{} Summary", name()));

        // N-tuple statistics
        if (m_enable_ntuple.get() && m_ntuple_svc) {
            auto entries = m_ntuple_svc->get_entries("electron_kinematics");
            Log::stat(name(), "N-tuple entries", entries);
        }

        // Fiducial service statistics
        if (m_fiducial) {
            Log::stat(name(), "Fiducial evaluations", m_fiducial->total_evaluations());
            Log::stat(name(), "Fiducial efficiency", fmt::format("{:.2f}%", m_fiducial->efficiency() * 100));
        }

        // Print cutflow
        Log::blank();
        m_cutflow.print();

        Log::success(name(), "Finalization complete");
        return clas12::framework::StatusCode::SUCCESS();
    }

private:
    // ========================================================================
    // Configuration
    // ========================================================================

    clas12::Target m_target;
    clas12::Polarity m_polarity;

    // Properties
    clas12::framework::Property<bool> m_enable_ntuple{
        "enable_ntuple", true, "Enable N-tuple output"};
    clas12::framework::Property<bool> m_enable_fiducial_histograms{
        "enable_fiducial_histograms", false, "Enable fiducial diagnostic histograms"};

    // Cut toggle properties
    clas12::framework::Property<bool> m_enable_chi2pid_cut{"enable_chi2pid_cut", true, "Enable chi2 PID cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
    clas12::framework::Property<bool> m_enable_sf_cut{"enable_sf_cut", true, "Enable sampling fraction cut"};
    clas12::framework::Property<bool> m_enable_pcal_fiducial_cut{"enable_pcal_fiducial_cut", true, "Enable PCAL fiducial cut"};
    clas12::framework::Property<bool> m_enable_dc_r1_cut{"enable_dc_r1_cut", true, "Enable DC R1 edge cut"};
    clas12::framework::Property<bool> m_enable_dc_r2_cut{"enable_dc_r2_cut", true, "Enable DC R2 edge cut"};
    clas12::framework::Property<bool> m_enable_dc_r3_cut{"enable_dc_r3_cut", true, "Enable DC R3 edge cut"};

    // ========================================================================
    // Services (cached in on_initialize via svc<T>())
    // ========================================================================

    clas12::framework::HistogramService* m_hist = nullptr;
    clas12::framework::NTupleService* m_ntuple_svc = nullptr;
    clas12::framework::SamplingFractionService* m_sf = nullptr;
    clas12::framework::ElectronCutsService* m_electron_cuts = nullptr;
    clas12::framework::FiducialService* m_fiducial = nullptr;

    // ========================================================================
    // Cutflow
    // ========================================================================

    clas12::framework::ContextCutflow<ElectronCutContext> m_cutflow;

    // ========================================================================
    // Histograms
    // ========================================================================

    /// One stage's worth of electron-kinematics histograms (before/after
    /// cuts). `book()` registers all branches under @p dir, `fill()`
    /// pushes one electron through the full set.
    struct StageHistograms {
        // Kinematics
        clas12::framework::Hist1D* p           = nullptr;
        clas12::framework::Hist1D* theta       = nullptr;
        clas12::framework::Hist1D* phi         = nullptr;
        clas12::framework::Hist1D* pt          = nullptr;
        clas12::framework::Hist1D* sector      = nullptr;
        // Vertex
        clas12::framework::Hist1D* vz          = nullptr;
        clas12::framework::Hist1D* vx          = nullptr;
        clas12::framework::Hist1D* vy          = nullptr;
        // PID
        clas12::framework::Hist1D* chi2pid     = nullptr;
        clas12::framework::Hist1D* beta        = nullptr;
        clas12::framework::Hist1D* nphe        = nullptr;
        // Calorimeter
        clas12::framework::Hist1D* pcal_E      = nullptr;
        clas12::framework::Hist1D* ecin_E      = nullptr;
        clas12::framework::Hist1D* ecout_E     = nullptr;
        clas12::framework::Hist1D* ecal_E      = nullptr;
        clas12::framework::Hist1D* sf          = nullptr;
        // DC
        clas12::framework::Hist1D* dc_chi2_R1  = nullptr;
        clas12::framework::Hist1D* dc_chi2_R2  = nullptr;
        clas12::framework::Hist1D* dc_chi2_R3  = nullptr;
        // PCAL fiducial
        clas12::framework::Hist1D* pcal_lu     = nullptr;
        clas12::framework::Hist1D* pcal_lv     = nullptr;
        clas12::framework::Hist1D* pcal_lw     = nullptr;
        // 2D correlations
        clas12::framework::Hist2D* sf_vs_p       = nullptr;
        clas12::framework::Hist2D* chi2pid_vs_p  = nullptr;
        clas12::framework::Hist2D* vz_vs_phi     = nullptr;
        clas12::framework::Hist2D* vz_vs_theta   = nullptr;
        clas12::framework::Hist2D* theta_vs_phi  = nullptr;
        clas12::framework::Hist2D* p_vs_theta    = nullptr;
        clas12::framework::Hist2D* pcal_lv_vs_lu = nullptr;
        clas12::framework::Hist2D* ecin_vs_pcal  = nullptr;

        auto book(clas12::framework::HistogramService* svc, const std::string& dir) -> void {
            // The group owns the underlying Hist1D/Hist2D objects; we hold
            // pointers back to each so fill() stays handle-only with no
            // string lookups on the hot path.
            m_group = std::make_unique<clas12::framework::HistogramGroup>(svc, dir);
            auto* g = m_group.get();
            p          = &g->add_hist1D("p",       "Electron Momentum;p [GeV/c];Events", 100, 0.0, 12.0);
            theta      = &g->add_hist1D("theta",   "Electron #theta;#theta [deg];Events", 90, 0.0, 45.0);
            phi        = &g->add_hist1D("phi",     "Electron #phi;#phi [deg];Events", 360, -180.0, 180.0);
            pt         = &g->add_hist1D("pt",      "Electron p_{T};p_{T} [GeV/c];Events", 100, 0.0, 4.0);
            sector     = &g->add_hist1D("sector",  "Electron Sector;Sector;Events", 7, 0.5, 7.5);
            vz         = &g->add_hist1D("vz",      "Vertex Z;v_{z} [cm];Events", 200, -40.0, 40.0);
            vx         = &g->add_hist1D("vx",      "Vertex X;v_{x} [cm];Events", 100, -2.0, 2.0);
            vy         = &g->add_hist1D("vy",      "Vertex Y;v_{y} [cm];Events", 100, -2.0, 2.0);
            chi2pid    = &g->add_hist1D("chi2pid", "Electron #chi^{2}_{PID};#chi^{2}_{PID};Events", 200, -10.0, 10.0);
            beta       = &g->add_hist1D("beta",    "Electron #beta;#beta;Events", 100, 0.9, 1.1);
            nphe       = &g->add_hist1D("nphe",    "Number of Photoelectrons;N_{phe};Events", 100, 0.0, 50.0);
            pcal_E     = &g->add_hist1D("pcal_E",  "PCAL Energy;E_{PCAL} [GeV];Events", 100, 0.0, 2.0);
            ecin_E     = &g->add_hist1D("ecin_E",  "ECIN Energy;E_{ECIN} [GeV];Events", 100, 0.0, 2.0);
            ecout_E    = &g->add_hist1D("ecout_E", "ECOUT Energy;E_{ECOUT} [GeV];Events", 100, 0.0, 2.0);
            ecal_E     = &g->add_hist1D("ecal_E",  "Total ECal Energy;E_{ECal} [GeV];Events", 100, 0.0, 4.0);
            sf         = &g->add_hist1D("sampling_fraction", "Sampling Fraction;SF;Events", 100, 0.0, 0.5);
            dc_chi2_R1 = &g->add_hist1D("dc_chi2_R1", "DC #chi^{2}/NDF R1;#chi^{2}/NDF;Events", 100, 0.0, 10.0);
            dc_chi2_R2 = &g->add_hist1D("dc_chi2_R2", "DC #chi^{2}/NDF R2;#chi^{2}/NDF;Events", 100, 0.0, 10.0);
            dc_chi2_R3 = &g->add_hist1D("dc_chi2_R3", "DC #chi^{2}/NDF R3;#chi^{2}/NDF;Events", 100, 0.0, 10.0);
            pcal_lu    = &g->add_hist1D("pcal_lu", "PCAL Local U;PCAL_{lu} [cm];Events", 100, 0.0, 450.0);
            pcal_lv    = &g->add_hist1D("pcal_lv", "PCAL Local V;PCAL_{lv} [cm];Events", 100, 0.0, 450.0);
            pcal_lw    = &g->add_hist1D("pcal_lw", "PCAL Local W;PCAL_{lw} [cm];Events", 100, 0.0, 450.0);

            sf_vs_p       = &g->add_hist2D("sf_vs_p",       "SF vs p;p [GeV/c];SF",                                    100, 0.0, 12.0, 100, 0.0, 0.5);
            chi2pid_vs_p  = &g->add_hist2D("chi2pid_vs_p",  "#chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}",            100, 0.0, 12.0, 200, -10.0, 10.0);
            vz_vs_phi     = &g->add_hist2D("vz_vs_phi",     "v_{z} vs #phi;#phi [deg];v_{z} [cm]",                     180, -180.0, 180.0, 100, -40.0, 40.0);
            vz_vs_theta   = &g->add_hist2D("vz_vs_theta",   "v_{z} vs #theta;#theta [deg];v_{z} [cm]",                 90, 0.0, 45.0, 100, -40.0, 40.0);
            theta_vs_phi  = &g->add_hist2D("theta_vs_phi",  "#theta vs #phi;#phi [deg];#theta [deg]",                  180, -180.0, 180.0, 90, 0.0, 45.0);
            p_vs_theta    = &g->add_hist2D("p_vs_theta",    "p vs #theta;#theta [deg];p [GeV/c]",                      90, 0.0, 45.0, 100, 0.0, 12.0);
            pcal_lv_vs_lu = &g->add_hist2D("pcal_lv_vs_lu", "PCAL lv vs lu;PCAL_{lu} [cm];PCAL_{lv} [cm]",             100, 0.0, 450.0, 100, 0.0, 450.0);
            ecin_vs_pcal  = &g->add_hist2D("ecin_vs_pcal",  "E_{ECIN} vs E_{PCAL};E_{PCAL} [GeV];E_{ECIN} [GeV]",      100, 0.0, 2.0, 100, 0.0, 2.0);
        }

        auto fill(const clas12::Particle& e,
                  const clas12::CalorimeterBank& calo,
                  const clas12::TrackBank& trk_r1,
                  const clas12::TrackBank& trk_r2,
                  const clas12::TrackBank& trk_r3,
                  double SF) const -> void {
            const double pmag = e.p();
            const double E_tot = calo.pcal_energy + calo.inner_energy + calo.outer_energy;

            p->fill(pmag);
            theta->fill(e.theta());
            phi->fill(e.phi());
            pt->fill(e.pt());
            sector->fill(static_cast<double>(calo.pcal_sector));
            vz->fill(e.vz());
            vx->fill(e.vx());
            vy->fill(e.vy());
            chi2pid->fill(e.chi2pid());
            beta->fill(e.beta());

            pcal_E->fill(calo.pcal_energy);
            ecin_E->fill(calo.inner_energy);
            ecout_E->fill(calo.outer_energy);
            ecal_E->fill(E_tot);
            sf->fill(SF);

            // Guard division by zero when track bank is empty.
            if (trk_r1.NDF > 0) dc_chi2_R1->fill(trk_r1.chi2 / trk_r1.NDF);
            if (trk_r2.NDF > 0) dc_chi2_R2->fill(trk_r2.chi2 / trk_r2.NDF);
            if (trk_r3.NDF > 0) dc_chi2_R3->fill(trk_r3.chi2 / trk_r3.NDF);

            pcal_lu->fill(calo.pcal_lu);
            pcal_lv->fill(calo.pcal_lv);
            pcal_lw->fill(calo.pcal_lw);

            sf_vs_p->fill(pmag, SF);
            chi2pid_vs_p->fill(pmag, e.chi2pid());
            vz_vs_phi->fill(e.phi(), e.vz());
            vz_vs_theta->fill(e.theta(), e.vz());
            theta_vs_phi->fill(e.phi(), e.theta());
            p_vs_theta->fill(e.theta(), pmag);
            pcal_lv_vs_lu->fill(calo.pcal_lu, calo.pcal_lv);
            ecin_vs_pcal->fill(calo.pcal_energy, calo.inner_energy);
        }

       private:
        // Owns the underlying Hist1D/Hist2D handles bound above.
        std::unique_ptr<clas12::framework::HistogramGroup> m_group;
    };

    StageHistograms h_before;
    StageHistograms h_after;

    // Fiducial diagnostic histograms
    clas12::framework::ThreadLocalHistogram2D* h_pcal_lv_lw_before = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_pcal_lv_lw_after = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_dc_r1_edge_before = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_dc_r1_edge_after = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_dc_r2_edge_before = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_dc_r2_edge_after = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_dc_r3_edge_before = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_dc_r3_edge_after = nullptr;

    // ========================================================================
    // N-Tuple
    // ========================================================================

    clas12::framework::NTupleWriter<ElectronNTupleData> m_ntuple;

    // ========================================================================
    // Helper Methods
    // ========================================================================

    /**
     * @brief Configure the ElectronCutsService with target/polarity
     */
    void configure_electron_cuts_service() {
        m_electron_cuts->set_target(m_target);
        m_electron_cuts->set_polarity(m_polarity);
    }

    /**
     * @brief Book all histograms
     */
    void book_histograms() {
        // Kinematics histograms (before/after cuts)
        h_before.book(m_hist, "/Electron/Kinematics/before_cuts");
        h_after .book(m_hist, "/Electron/Kinematics/after_cuts");

        // Fiducial diagnostic histograms
        if (m_enable_fiducial_histograms.get()) {
            h_pcal_lv_lw_before = m_hist->book_hist2D(
                "/Electron/Fiducial/before_cuts/pcal_lv_lw",
                "PCAL lv vs lw (before cuts);lv [cm];lw [cm]",
                100, 0, 450, 100, 0, 450);
            h_pcal_lv_lw_after = m_hist->book_hist2D(
                "/Electron/Fiducial/after_cuts/pcal_lv_lw",
                "PCAL lv vs lw (after cuts);lv [cm];lw [cm]",
                100, 0, 450, 100, 0, 450);

            h_dc_r1_edge_before = m_hist->book_hist1D(
                "/Electron/Fiducial/before_cuts/dc_r1_edge",
                "DC R1 Edge (before cuts);Edge [cm];Events", 100, 0, 50);
            h_dc_r1_edge_after = m_hist->book_hist1D(
                "/Electron/Fiducial/after_cuts/dc_r1_edge",
                "DC R1 Edge (after cuts);Edge [cm];Events", 100, 0, 50);

            h_dc_r2_edge_before = m_hist->book_hist1D(
                "/Electron/Fiducial/before_cuts/dc_r2_edge",
                "DC R2 Edge (before cuts);Edge [cm];Events", 100, 0, 50);
            h_dc_r2_edge_after = m_hist->book_hist1D(
                "/Electron/Fiducial/after_cuts/dc_r2_edge",
                "DC R2 Edge (after cuts);Edge [cm];Events", 100, 0, 50);

            h_dc_r3_edge_before = m_hist->book_hist1D(
                "/Electron/Fiducial/before_cuts/dc_r3_edge",
                "DC R3 Edge (before cuts);Edge [cm];Events", 100, 0, 50);
            h_dc_r3_edge_after = m_hist->book_hist1D(
                "/Electron/Fiducial/after_cuts/dc_r3_edge",
                "DC R3 Edge (after cuts);Edge [cm];Events", 100, 0, 50);
        }
    }

    /**
     * @brief Define cutflow with all electron selection cuts
     */
    void define_cutflow() {
        m_cutflow
            .define_always("All Events")
            .define("Has Electron", [](const ElectronCutContext&) {
                return true;  // Already filtered before context creation
            })
            .define("Chi2 PID", [this](const ElectronCutContext& ctx) {
                return !m_enable_chi2pid_cut.get() || m_electron_cuts->pass_chi2pid_cut(ctx.electron.chi2pid());
            })
            .define("Momentum > 0.8 GeV", [this](const ElectronCutContext& ctx) {
                return !m_enable_momentum_cut.get() || m_electron_cuts->pass_momentum_cut(ctx.electron.p());
            })
            .define("Vertex Z", [this](const ElectronCutContext& ctx) {
                return !m_enable_vz_cut.get() || m_electron_cuts->pass_vz_cut(ctx.electron.vz());
            })
            .define("Sampling Fraction", [this](const ElectronCutContext& ctx) {
                return !m_enable_sf_cut.get() || m_electron_cuts->pass_sampling_fraction_cut(ctx.SF, ctx.electron.p(), ctx.sector);
            })
            .define("PCAL Fiducial", [this](const ElectronCutContext& ctx) {
                return !m_enable_pcal_fiducial_cut.get() || ctx.fiducial_result.passed_region(clas12::framework::DetectorRegion::PCAL);
            })
            .define("DC R1 Edge", [this](const ElectronCutContext& ctx) {
                return !m_enable_dc_r1_cut.get() || ctx.fiducial_result.passed_region(clas12::framework::DetectorRegion::DC_R1);
            })
            .define("DC R2 Edge", [this](const ElectronCutContext& ctx) {
                return !m_enable_dc_r2_cut.get() || ctx.fiducial_result.passed_region(clas12::framework::DetectorRegion::DC_R2);
            })
            .define("DC R3 Edge", [this](const ElectronCutContext& ctx) {
                return !m_enable_dc_r3_cut.get() || ctx.fiducial_result.passed_region(clas12::framework::DetectorRegion::DC_R3);
            });
    }

    /**
     * @brief Fill fiducial diagnostic histograms
     */
    void fill_fiducial_histograms(
        const clas12::CalorimeterBank& calo,
        const clas12::DCTrajBank& traj,
        [[maybe_unused]] const clas12::framework::FiducialResult& result,
        const std::string& stage) {

        if (stage == "before_cuts") {
            if (h_pcal_lv_lw_before) h_pcal_lv_lw_before->fill(calo.pcal_lv, calo.pcal_lw);
            if (h_dc_r1_edge_before) h_dc_r1_edge_before->fill(traj.region1_edge);
            if (h_dc_r2_edge_before) h_dc_r2_edge_before->fill(traj.region2_edge);
            if (h_dc_r3_edge_before) h_dc_r3_edge_before->fill(traj.region3_edge);
        } else {
            if (h_pcal_lv_lw_after) h_pcal_lv_lw_after->fill(calo.pcal_lv, calo.pcal_lw);
            if (h_dc_r1_edge_after) h_dc_r1_edge_after->fill(traj.region1_edge);
            if (h_dc_r2_edge_after) h_dc_r2_edge_after->fill(traj.region2_edge);
            if (h_dc_r3_edge_after) h_dc_r3_edge_after->fill(traj.region3_edge);
        }
    }

    /**
     * @brief Fill N-tuple with electron data
     */
    void fill_ntuple(const clas12::Particle& electron, double SF, int sector) {
        if (!m_ntuple) return;

        auto& nt = m_ntuple.data();
        nt.e_p       = electron.p();
        nt.e_theta   = electron.theta();
        nt.e_phi     = electron.phi();
        nt.e_vz      = electron.vz();
        nt.e_chi2pid = electron.chi2pid();
        nt.e_sf      = SF;
        nt.e_sector  = sector;
        m_ntuple.fill();
    }
};

}  // namespace clas12::algorithms
