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
 * - HistogramFiller for efficient histogram filling
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
#include <clas12/framework/core/HistogramFiller.hpp>
#include <clas12/framework/core/HistogramTemplates.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
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
 * Using NTUPLE_FIELDS macro for automatic branch registration.
 */
struct ElectronNTupleData {
    double e_p       = 0.0;  ///< Electron momentum [GeV/c]
    double e_theta   = 0.0;  ///< Electron polar angle [rad]
    double e_phi     = 0.0;  ///< Electron azimuthal angle [rad]
    double e_vz      = 0.0;  ///< Electron vertex z [cm]
    double e_chi2pid = 0.0;  ///< Electron chi2pid
    double e_sf      = 0.0;  ///< Sampling fraction
    int e_sector     = 0;    ///< PCAL sector (1-6)

    NTUPLE_FIELDS(e_p, e_theta, e_phi, e_vz, e_chi2pid, e_sf, e_sector)
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
class ElectronSelectionAlg : public clas12::framework::SimpleAlgorithm<ElectronSelectionAlg> {
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
        : SimpleAlgorithm(std::move(name))
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
        // Get electrons from event
        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);

        if (electrons.empty()) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }

        // Get HIPO banks
        const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");
        const auto& REC_Traj = event.get_bank("REC::Traj");
        const auto& REC_Track = event.get_bank("REC::Track");

        // Select trigger electron (highest momentum in FD)
        const auto possible_electron =
            clas12::physics::ParticleUtils::find_trigger_electron(electrons, {clas12::FD, clas12::BD});

        if (!possible_electron.has_value()) {
            event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        const clas12::Particle& electron = possible_electron.value();

        // Read detector bank data
        const auto calorimeter = clas12::read_Calorimeter_bank(REC_Calorimeter, electron.index());
        const auto traj = clas12::read_Traj_bank(REC_Traj, electron.index());
        const auto trk_r1 = clas12::read_Track_bank(REC_Track, traj.region1.index);
        const auto trk_r2 = clas12::read_Track_bank(REC_Track, traj.region2.index);
        const auto trk_r3 = clas12::read_Track_bank(REC_Track, traj.region3.index);

        // Calculate derived quantities
        const double E_total = calorimeter.pcal.energy + calorimeter.inner.energy + calorimeter.outer.energy;
        const double SF = E_total / electron.p();
        const int sector = calorimeter.pcal.sector;

        // Pre-compute fiducial result using FiducialService
        auto fiducial_result = m_fiducial->evaluate(electron, calorimeter, traj, sector);

        // Fill histograms before cuts
        clas12::framework::HistogramFiller::fill_electron(
            kinematics_groups["before_cuts"].get(), electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

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

        // Electron passed all cuts
        event.set_decision_typed(clas12::framework::Decisions::HasGoodElectron, true);
        event.store_typed(clas12::framework::Keys::SelectedElectrons,
                          std::vector<clas12::Particle>{electron});

        // Fill histograms after cuts
        clas12::framework::HistogramFiller::fill_electron(
            kinematics_groups["after_cuts"].get(), electron, calorimeter, trk_r1, trk_r2, trk_r3, SF);

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

    std::map<std::string, std::unique_ptr<clas12::framework::HistogramGroup>> kinematics_groups;

    // Fiducial diagnostic histograms
    clas12::framework::AtomicHistogram2D* h_pcal_lv_lw_before = nullptr;
    clas12::framework::AtomicHistogram2D* h_pcal_lv_lw_after = nullptr;
    clas12::framework::AtomicHistogram1D* h_dc_r1_edge_before = nullptr;
    clas12::framework::AtomicHistogram1D* h_dc_r1_edge_after = nullptr;
    clas12::framework::AtomicHistogram1D* h_dc_r2_edge_before = nullptr;
    clas12::framework::AtomicHistogram1D* h_dc_r2_edge_after = nullptr;
    clas12::framework::AtomicHistogram1D* h_dc_r3_edge_before = nullptr;
    clas12::framework::AtomicHistogram1D* h_dc_r3_edge_after = nullptr;

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
        if (m_sf) {
            m_electron_cuts->set_sf_service(m_sf);
        }
    }

    /**
     * @brief Book all histograms
     */
    void book_histograms() {
        // Kinematics histograms (before/after cuts)
        kinematics_groups = clas12::framework::HistogramTemplates::ElectronHistograms::create_with_stages(
            m_hist, "/Electron/Kinematics", {"before_cuts", "after_cuts"});

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
            if (h_pcal_lv_lw_before) h_pcal_lv_lw_before->fill(calo.pcal.lv, calo.pcal.lw);
            if (h_dc_r1_edge_before) h_dc_r1_edge_before->fill(traj.region1.edge);
            if (h_dc_r2_edge_before) h_dc_r2_edge_before->fill(traj.region2.edge);
            if (h_dc_r3_edge_before) h_dc_r3_edge_before->fill(traj.region3.edge);
        } else {
            if (h_pcal_lv_lw_after) h_pcal_lv_lw_after->fill(calo.pcal.lv, calo.pcal.lw);
            if (h_dc_r1_edge_after) h_dc_r1_edge_after->fill(traj.region1.edge);
            if (h_dc_r2_edge_after) h_dc_r2_edge_after->fill(traj.region2.edge);
            if (h_dc_r3_edge_after) h_dc_r3_edge_after->fill(traj.region3.edge);
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
