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
#include <clas12/framework/core/Pipeline.hpp>
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
    const clas12::TrackBank& trk_r1;        ///< DC track region-1 info
    const clas12::TrackBank& trk_r2;        ///< DC track region-2 info
    const clas12::TrackBank& trk_r3;        ///< DC track region-3 info
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
class ElectronSelectionAlg : public clas12::framework::SimpleAlgorithm {
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
        using namespace clas12::framework;

        // Default decision: filtered out. Flipped to true on success.
        event.set_decision_typed(Decisions::HasGoodElectron, false);

        const auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        auto trigger_e = !electrons.empty()
            ? clas12::physics::ParticleUtils::find_trigger_electron(
                  electrons, {clas12::FD, clas12::BD})
            : std::optional<clas12::Particle>{};

        // Pre-compute detector reads + derived quantities. These are
        // safe to run on a default-constructed Particle (read_*_bank
        // returns a NaN-filled struct) so the pipeline below can still
        // short-circuit on the missing-electron filter.
        const clas12::Particle electron = trigger_e.value_or(clas12::Particle{});
        const auto calorimeter = trigger_e
            ? clas12::read_Calorimeter_bank(event, electron.index())
            : clas12::CalorimeterBank{};
        const auto traj = trigger_e
            ? clas12::read_Traj_bank(event, electron.index())
            : clas12::DCTrajBank{};
        const auto trk_r1 = trigger_e ? clas12::read_Track_bank(event, traj.region1_index) : clas12::TrackBank{};
        const auto trk_r2 = trigger_e ? clas12::read_Track_bank(event, traj.region2_index) : clas12::TrackBank{};
        const auto trk_r3 = trigger_e ? clas12::read_Track_bank(event, traj.region3_index) : clas12::TrackBank{};

        const double E_total = calorimeter.pcal_energy + calorimeter.inner_energy + calorimeter.outer_energy;
        const double SF      = trigger_e ? (E_total / electron.p()) : 0.0;
        const int    sector  = calorimeter.pcal_sector;

        auto fiducial_result = trigger_e
            ? m_fiducial->evaluate(electron, calorimeter, traj, sector)
            : decltype(m_fiducial->evaluate(electron, calorimeter, traj, sector)){};

        ElectronCutContext ctx{electron, calorimeter, traj,
                                trk_r1, trk_r2, trk_r3,
                                SF, sector, fiducial_result};
        const bool passes_cuts = trigger_e && m_cutflow.evaluate(ctx);

        return pipeline(event)
            .filter("has electrons",    !electrons.empty())
            .filter("trigger electron", trigger_e.has_value())
            .fill(&ElectronSelectionAlg::fill_stage, this, "before_cuts", ctx)
            .filter("passes cuts", passes_cuts)
            .set_decision(Decisions::HasGoodElectron, true)
            .publish(Keys::SelectedElectrons, std::vector<clas12::Particle>{electron})
            .fill(&ElectronSelectionAlg::fill_stage, this, "after_cuts",  ctx)
            .fill(&ElectronSelectionAlg::fill_ntuple, this, electron, SF, sector)
            .status();
    }

    /// Per-stage histogram fill. Called once for "before_cuts" (before
    /// the cutflow gate) and once for "after_cuts" (after it passes).
    /// Bundles the 7-arg HistogramFiller::fill_electron call + the
    /// optional fiducial-histogram fill so the pipeline body stays a
    /// one-liner per stage.
    auto fill_stage(std::string_view stage, const ElectronCutContext& c) -> void {
        clas12::framework::HistogramFiller::fill_electron(
            kinematics_groups[std::string(stage)].get(),
            c.electron, c.calo, c.trk_r1, c.trk_r2, c.trk_r3, c.SF);
        if (m_enable_fiducial_histograms.get()) {
            fill_fiducial_histograms(c.calo, c.traj, c.fiducial_result, stage);
        }
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
        std::string_view stage) {

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
