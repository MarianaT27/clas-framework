#pragma once

#include <array>
#include <atomic>
#include <cmath>
#include <clas12/Constants.hpp>
#include <clas12/ElectronCuts.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/CandidateUtils.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>

namespace clas12::algorithms {

// Import Log singleton for convenient logging
using clas12::framework::Log;

/**
 * @brief N-Tuple data structure for pion kinematics
 */
struct PionNTupleData {
    double p = 0.0;
    double theta = 0.0;
    double phi = 0.0;
    double vz = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double chi2pid = 0.0;
    double beta = 0.0;
    double delta_t = 0.0;
    double tof_mass2 = 0.0;
    double tof_path = 0.0;
    int sector = 0;
    double beta_expected = 0.0;
    double beta_diff = 0.0;
    int is_forward = 0;
    double pcal_lv = 0.0;
    double pcal_lw = 0.0;

    /// Populate from any source with matching field names
    template <typename T>
    auto from(const T& c) -> void {
        p = c.p; theta = c.theta; phi = c.phi;
        vz = c.vz; vx = c.vx; vy = c.vy;
        chi2pid = c.chi2pid; beta = c.beta; delta_t = c.delta_t;
        tof_mass2 = c.tof_mass2; tof_path = c.tof_path; sector = c.sector;
        beta_expected = c.beta_expected; beta_diff = c.beta_diff;
        is_forward = c.is_forward_detector ? 1 : 0;
        pcal_lv = c.pcal_lv; pcal_lw = c.pcal_lw;
    }

    NTUPLE_FIELDS(p, theta, phi, vz, vx, vy, chi2pid, beta, delta_t, tof_mass2, tof_path, sector,
                  beta_expected, beta_diff, is_forward, pcal_lv, pcal_lw)
};

/**
 * @brief Reconstructed pion candidate with full kinematics and cut decisions
 *
 * This struct consolidates:
 * - Particle reference and basic kinematics (p, theta, phi, vz)
 * - PID quality (chi2pid, beta)
 * - Timing quantities (delta_t, expected beta)
 * - Sector information for detector studies
 * - Cached cut decisions for efficient filtering
 *
 * The candidate-based approach allows:
 * 1. Computing kinematic values once during candidate building
 * 2. Filling pre-cut histograms before applying selections
 * 3. Efficiently collecting passing candidates
 * 4. Simplified N-tuple filling with all variables in one place
 */
struct PionCandidate {
    // Particle reference
    clas12::Particle pion;

    // Basic kinematics
    double p = 0.0;           // Momentum [GeV]
    double theta = 0.0;       // Polar angle [deg]
    double phi = 0.0;         // Azimuthal angle [deg]
    double vz = 0.0;          // Vertex z [cm]
    double chi2pid = 0.0;     // Chi2 PID
    double beta = 0.0;        // Measured beta

    // Timing quantities
    double delta_t = 0.0;     // Time-of-flight difference [ns]
    double beta_expected = 0.0; // Expected beta from momentum

    // Scintillator info
    int sector = 0;           // FTOF sector (1-6)
    double tof_time = 0.0;    // Raw TOF time [ns]
    double tof_path = 0.0;    // Path length [cm]

    // Additional vertex info
    double vx = 0.0;          // Vertex x [cm]
    double vy = 0.0;          // Vertex y [cm]

    // TOF mass (calculated from beta)
    double tof_mass2 = 0.0;   // m^2 from TOF [GeV^2]

    // Beta difference
    double beta_diff = 0.0;   // beta_measured - beta_expected

    // PCAL fiducial variables
    double pcal_lv = 0.0;
    double pcal_lw = 0.0;

    // Detector region
    bool is_forward_detector = false;
    bool is_central_detector = false;

    // Cached cut decisions
    bool passes_dc_fiducial = false;
    bool passes_chi2pid = false;
    bool passes_momentum = false;
    bool passes_vz = false;
    bool passes_delta_t = false;
    bool passes_all_cuts = false;
};

/**
 * @brief Pion (π⁺/π⁻) selection and analysis algorithm
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>() and optional_service<T>()
 * - Property system for configuration
 * - New histogram templates for physics quantities
 * - Reduced boilerplate through CRTP
 *
 * This algorithm demonstrates pion identification using:
 * - Chi2 PID cuts
 * - Momentum cuts
 * - Vertex Z cuts
 * - Time-of-flight (delta_t) cuts
 * - Beta cuts
 * - Cutflow tracking for efficiency
 */
class PionSelectionAlg : public clas12::framework::SimpleAlgorithm<PionSelectionAlg> {
   public:
    explicit PionSelectionAlg(std::string name = "PionSelection",
                              clas12::Target target = clas12::Target::LD2,
                              bool separate_charges = true,
                              std::vector<double> sigma_scan = {})
        : SimpleAlgorithm(std::move(name))
        , m_target(target)
        , m_separate_charges(separate_charges)
        , m_cutflow_piplus("π⁺ Selection")
        , m_cutflow_piminus("π⁻ Selection") {
        // Declare configurable properties
        declare_property(m_chi2pid_min);
        declare_property(m_chi2pid_max);
        declare_property(m_momentum_min);
        declare_property(m_momentum_max);
        declare_property(m_enable_ntuple);
        declare_property(m_enable_detector_region_cut);
        declare_property(m_enable_fiducial_cuts);
        declare_property(m_enable_chi2pid_cut);
        declare_property(m_enable_momentum_cut);
        declare_property(m_enable_vz_cut);
        declare_property(m_enable_delta_t_cut);
        declare_property(m_enable_central_pions);
        declare_property(m_delta_t_sigma);
        declare_property(m_sigma_scan_values);
        // Override scan values if provided via constructor
        if (!sigma_scan.empty()) {
            m_sigma_scan_values.set(std::move(sigma_scan));
        }
        // Declare required banks
        declare_bank("REC::Event");
        declare_bank("REC::Particle");
        declare_bank("REC::Scintillator");
        declare_bank("REC::Calorimeter");
        declare_bank("REC::Traj");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        // Cache service pointers
        m_hist = svc<clas12::framework::HistogramService>();
        m_particle = svc<clas12::framework::ParticleSelectorService>();
        m_log = svc<clas12::framework::LoggingService>();
        m_ntuple = svc<clas12::framework::NTupleService>();

        Log::info(name(), "Initializing pion selection algorithm");
        Log::info(name(), "Target: {}, Separate charges: {}",
                  m_target == clas12::Target::LD2 ? "LD2" : "Other",
                  m_separate_charges ? "yes" : "no");
        Log::info(name(), "Chi2PID range: [{}, {}], Momentum range: [{}, {}] GeV",
                  m_chi2pid_min.get(), m_chi2pid_max.get(),
                  m_momentum_min.get(), m_momentum_max.get());
        Log::info(name(), "Cut toggles: detector_region={}, fiducial={}, chi2pid={}, momentum={}, vz={}, delta_t={}",
                  m_enable_detector_region_cut.get(), m_enable_fiducial_cuts.get(),
                  m_enable_chi2pid_cut.get(), m_enable_momentum_cut.get(),
                  m_enable_vz_cut.get(), m_enable_delta_t_cut.get());
        Log::info(name(), "Central pions: {}", m_enable_central_pions.get() ? "enabled" : "disabled");
        Log::info(name(), "Delta-t sigma: {}", m_delta_t_sigma.get());
        if (!m_sigma_scan_values.get().empty()) {
            Log::info(name(), "Sigma scan enabled with values: [{}]",
                      fmt::join(m_sigma_scan_values.get(), ", "));
        }

        // Setup cutflows
        define_cutflows();

        // Book histograms using new histogram templates
        book_histograms();

        // Setup N-tuple branches if service is available
        if (m_ntuple) {
            setup_ntuples();
        } else {
            Log::warning(name(), "NTupleService not available - N-tuple output disabled");
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        // Only process pions if a good electron was found
        if (!event.passes_decision_typed(clas12::framework::Decisions::HasGoodDISElectron)) {
            return clas12::framework::StatusCode::FILTERED();
        }

        // Get banks individually
        const auto& REC_Event = event.get_bank("REC::Event");
        const auto& REC_Particle = event.get_bank("REC::Particle");
        const auto& REC_Scintillator = event.get_bank("REC::Scintillator");
        const auto& REC_Calorimeter = event.get_bank("REC::Calorimeter");
        const auto& REC_Traj = event.get_bank("REC::Traj");

        if (REC_Event.rows() == 0) return clas12::framework::StatusCode::SUCCESS();

        double start_time = static_cast<double>(REC_Event.get<float>("startTime", 0));

        // Extract pions using ParticleSelectorService
        clas12::framework::ParticleMap particles = m_particle->extract_particles(
            REC_Particle,
            {clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS});

        // Build π⁺ candidates with all kinematics computed once
        const auto& pions_plus = clas12::framework::ParticleSelectorService::get_pions_plus(particles);
        auto piplus_candidates = build_pion_candidates(pions_plus, REC_Scintillator, REC_Calorimeter, REC_Traj, start_time, true);

        // Evaluate cutflow per-candidate (sets passes_all_cuts), then fill histograms
        for (auto& cand : piplus_candidates) {
            cand.passes_all_cuts = m_cutflow_piplus.evaluate(cand);
        }
        fill_pre_cut_histograms(piplus_candidates, true);
        fill_nminus1_histograms(piplus_candidates, true);
        fill_post_cut_histograms(piplus_candidates, true);
        fill_ntuple(piplus_candidates, true);

        // Collect passing particles
        auto selected_piplus = collect_passing_particles(piplus_candidates);

        // Build π⁻ candidates with all kinematics computed once
        const auto& pions_minus = clas12::framework::ParticleSelectorService::get_pions_minus(particles);
        auto piminus_candidates = build_pion_candidates(pions_minus, REC_Scintillator, REC_Calorimeter, REC_Traj, start_time, false);

        // Evaluate cutflow per-candidate (sets passes_all_cuts), then fill histograms
        for (auto& cand : piminus_candidates) {
            cand.passes_all_cuts = m_cutflow_piminus.evaluate(cand);
        }
        fill_pre_cut_histograms(piminus_candidates, false);
        fill_nminus1_histograms(piminus_candidates, false);
        fill_post_cut_histograms(piminus_candidates, false);
        fill_ntuple(piminus_candidates, false);

        // Collect passing particles
        auto selected_piminus = collect_passing_particles(piminus_candidates);

        // Store selected pions in event for downstream algorithms
        event.store_typed(clas12::framework::Keys::SelectedPiPlus, selected_piplus);
        event.store_typed(clas12::framework::Keys::SelectedPiMinus, selected_piminus);

        // Set decision based on whether we have good pions
        event.set_decision_typed(clas12::framework::Decisions::HasGoodPiMinus, !selected_piminus.empty());
        event.set_decision_typed(clas12::framework::Decisions::HasGoodPiPlus, !selected_piplus.empty());

        // Sigma scan: for each scan sigma, re-evaluate delta_t cut and store under string keys
        if (!m_sigma_scan_values.get().empty()) {
            for (double scan_sigma : m_sigma_scan_values.get()) {
                auto scan_piplus = collect_passing_with_sigma(piplus_candidates, true, scan_sigma);
                event.store_typed(clas12::framework::TypedKey<std::vector<clas12::Particle>>(format_sigma_key("SelectedPiPlus", scan_sigma)), std::move(scan_piplus));

                auto scan_piminus = collect_passing_with_sigma(piminus_candidates, false, scan_sigma);
                event.store_typed(clas12::framework::TypedKey<std::vector<clas12::Particle>>(format_sigma_key("SelectedPiMinus", scan_sigma)), std::move(scan_piminus));
            }
        }

        // Cumulative pion cut variations: store pion collections at each cut step
        // Steps 0-4 stored here; step 5 (all cuts) = existing SelectedPiPlus/Minus
        static constexpr std::array<std::pair<int, const char*>, 5> CUMULATIVE_STEPS = {{
            {0, "noPionCuts"}, {1, "plusDCFiducial"}, {2, "plusChi2PID"},
            {3, "plusMomentum"}, {4, "plusVertexZ"}
        }};
        for (const auto& [step, label] : CUMULATIVE_STEPS) {
            event.store_typed(clas12::framework::TypedKey<std::vector<clas12::Particle>>(fmt::format("PiPlus_{}", label)),
                collect_passing_cumulative(piplus_candidates, step));
            event.store_typed(clas12::framework::TypedKey<std::vector<clas12::Particle>>(fmt::format("PiMinus_{}", label)),
                collect_passing_cumulative(piminus_candidates, step));
        }

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing pion selection algorithm");

        auto piplus_count = m_cutflow_piplus.count("Delta-t");
        auto piminus_count = m_cutflow_piminus.count("Delta-t");

        Log::info(name(), "π⁺ passing all cuts: {}", piplus_count);
        Log::info(name(), "π⁻ passing all cuts: {}", piminus_count);

        if (m_ntuple) {
            auto piplus_entries = m_ntuple->get_entries("pion_plus");
            auto piminus_entries = m_ntuple->get_entries("pion_minus");
            Log::info(name(), "N-tuple entries: π⁺={}, π⁻={}", piplus_entries, piminus_entries);
        }

        m_cutflow_piplus.print();
        m_cutflow_piminus.print();

        // Pion analysis statistics table
        Log::print_yield_summary_table(
            "Pion Analysis Statistics",
            {"Particle", "Passing All Cuts"},
            {
                {"π⁺", fmt::format("{}", m_cutflow_piplus.count("Delta-t"))},
                {"π⁻", fmt::format("{}", m_cutflow_piminus.count("Delta-t"))}
            }
        );

        // Detector region statistics (pre-cut)
        Log::print_yield_summary_table(
            "Detector Region (pre-cut)",
            {"Region", "π⁺", "π⁻"},
            {
                {"Forward", fmt::format("{}", m_piplus_forward.load()), fmt::format("{}", m_piminus_forward.load())},
                {"Central", fmt::format("{}", m_piplus_central.load()), fmt::format("{}", m_piminus_central.load())},
                {"Other",   fmt::format("{}", m_piplus_other.load()),   fmt::format("{}", m_piminus_other.load())}
            }
        );

        // PCAL lv/lw NaN statistics (forward pions with undefined PCAL data)
        auto pip_fwd = m_piplus_forward.load();
        auto pim_fwd = m_piminus_forward.load();
        auto pip_nan = m_piplus_pcal_nan.load();
        auto pim_nan = m_piminus_pcal_nan.load();
        Log::print_yield_summary_table(
            "PCAL lv/lw NaN (forward pions)",
            {"", "π⁺", "π⁻"},
            {
                {"Forward total", fmt::format("{}", pip_fwd), fmt::format("{}", pim_fwd)},
                {"PCAL NaN",      fmt::format("{}", pip_nan), fmt::format("{}", pim_nan)},
                {"NaN fraction",
                 pip_fwd > 0 ? fmt::format("{:.1f}%", 100.0 * pip_nan / pip_fwd) : "N/A",
                 pim_fwd > 0 ? fmt::format("{:.1f}%", 100.0 * pim_nan / pim_fwd) : "N/A"}
            }
        );

        // Per-sector statistics
        std::vector<std::vector<std::string>> sector_rows;
        for (int sector = 1; sector <= 6; ++sector) {
            int idx = sector - 1;
            std::size_t pip = h_piplus.momentum_sector[idx] ? h_piplus.momentum_sector[idx]->entries() : 0;
            std::size_t pim = h_piminus.momentum_sector[idx] ? h_piminus.momentum_sector[idx]->entries() : 0;
            sector_rows.push_back({fmt::format("{}", sector), fmt::format("{}", pip), fmt::format("{}", pim)});
        }
        Log::print_yield_summary_table(
            "Per-Sector Counts (post-cut)",
            {"Sector", "π⁺", "π⁻"},
            sector_rows
        );

        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::Target m_target;
    bool m_separate_charges;
    clas12::framework::ContextCutflow<PionCandidate> m_cutflow_piplus;
    clas12::framework::ContextCutflow<PionCandidate> m_cutflow_piminus;

    // PCAL NaN counters (forward pions with undefined lv/lw)
    std::atomic<size_t> m_piplus_pcal_nan{0};
    std::atomic<size_t> m_piminus_pcal_nan{0};

    // Pre-cut detector region counters
    std::atomic<size_t> m_piplus_forward{0};
    std::atomic<size_t> m_piplus_central{0};
    std::atomic<size_t> m_piplus_other{0};
    std::atomic<size_t> m_piminus_forward{0};
    std::atomic<size_t> m_piminus_central{0};
    std::atomic<size_t> m_piminus_other{0};

    // Configurable properties
    clas12::framework::Property<double> m_chi2pid_min{"chi2pid_min", -10.0, "Minimum chi2PID"};
    clas12::framework::Property<double> m_chi2pid_max{"chi2pid_max", 10.0, "Maximum chi2PID"};
    clas12::framework::Property<double> m_momentum_min{"momentum_min", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_momentum_max{"momentum_max", 10.0, "Maximum pion momentum [GeV]"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};
    clas12::framework::Property<bool> m_enable_detector_region_cut{"enable_detector_region_cut", true, "Enable detector region cut"};
    clas12::framework::Property<bool> m_enable_fiducial_cuts{"enable_fiducial_cuts", true, "Enable PCAL and DC fiducial cuts"};
    clas12::framework::Property<bool> m_enable_chi2pid_cut{"enable_chi2pid_cut", true, "Enable chi2 PID cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
    clas12::framework::Property<bool> m_enable_delta_t_cut{"enable_delta_t_cut", true, "Enable delta-t cut"};
    clas12::framework::Property<bool> m_enable_central_pions{"enable_central_pions", false, "Enable central detector pion selection"};
    clas12::framework::Property<double> m_delta_t_sigma{"delta_t_sigma", 3.0, "Number of sigma for delta_t cut"};
    clas12::framework::Property<std::vector<double>> m_sigma_scan_values{"sigma_scan_values", {}, "Sigma values for delta_t scan (empty=disabled)"};

    // Service pointers (cached in on_initialize via svc<T>())
    clas12::framework::HistogramService* m_hist = nullptr;
    clas12::framework::ParticleSelectorService* m_particle = nullptr;
    clas12::framework::LoggingService* m_log = nullptr;
    clas12::framework::NTupleService* m_ntuple = nullptr;

    // N-tuple writers (replace individual variables + manual registration)
    clas12::framework::NTupleWriter<PionNTupleData> m_ntuple_piplus;
    clas12::framework::NTupleWriter<PionNTupleData> m_ntuple_piminus;

    // Lock-free atomic histogram structure for pion analysis
    struct PionHistograms {
        // Post-cut 1D histograms
        clas12::framework::AtomicHistogram1D* momentum = nullptr;
        clas12::framework::AtomicHistogram1D* theta = nullptr;
        clas12::framework::AtomicHistogram1D* phi = nullptr;
        clas12::framework::AtomicHistogram1D* vz = nullptr;
        clas12::framework::AtomicHistogram1D* chi2pid = nullptr;
        clas12::framework::AtomicHistogram1D* beta = nullptr;

        // Pre-cut 1D histograms (for efficiency studies)
        clas12::framework::AtomicHistogram1D* momentum_precut = nullptr;
        clas12::framework::AtomicHistogram1D* theta_precut = nullptr;
        clas12::framework::AtomicHistogram1D* phi_precut = nullptr;
        clas12::framework::AtomicHistogram1D* vz_precut = nullptr;
        clas12::framework::AtomicHistogram1D* chi2pid_precut = nullptr;
        clas12::framework::AtomicHistogram1D* delta_t_precut = nullptr;

        // Post-cut 2D histograms
        clas12::framework::AtomicHistogram2D* beta_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* p_vs_theta = nullptr;

        // PCAL fiducial diagnostics (pre-cut)
        clas12::framework::AtomicHistogram1D* pcal_lv_precut = nullptr;
        clas12::framework::AtomicHistogram1D* pcal_lw_precut = nullptr;
        clas12::framework::AtomicHistogram2D* pcal_lw_vs_lv = nullptr;

        // Additional 2D correlations (pre-cut for diagnostics)
        clas12::framework::AtomicHistogram2D* chi2pid_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* vz_vs_theta = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_theta = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_p_precut = nullptr;

        // Central detector histograms (pre-cut)
        clas12::framework::AtomicHistogram1D* momentum_central = nullptr;
        clas12::framework::AtomicHistogram1D* theta_central = nullptr;
        clas12::framework::AtomicHistogram1D* phi_central = nullptr;
        clas12::framework::AtomicHistogram1D* vz_central = nullptr;
        clas12::framework::AtomicHistogram1D* chi2pid_central = nullptr;
        clas12::framework::AtomicHistogram1D* beta_central = nullptr;
        clas12::framework::AtomicHistogram1D* delta_t_central = nullptr;
        clas12::framework::AtomicHistogram2D* beta_vs_p_central = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_p_central = nullptr;
        clas12::framework::AtomicHistogram2D* p_vs_theta_central = nullptr;

        // Sector-level histograms (post-cut)
        std::array<clas12::framework::AtomicHistogram1D*, 6> momentum_sector = {};
        std::array<clas12::framework::AtomicHistogram1D*, 6> theta_sector = {};
        std::array<clas12::framework::AtomicHistogram2D*, 6> delta_t_vs_p_sector = {};

        // Per-sector delta_t vs p (pre-cut, forward only)
        std::array<clas12::framework::AtomicHistogram2D*, 6> delta_t_vs_p_precut_sector = {};

        // N-1 histograms (forward only)
        clas12::framework::AtomicHistogram1D* chi2pid_nminus1 = nullptr;
        clas12::framework::AtomicHistogram1D* momentum_nminus1 = nullptr;
        clas12::framework::AtomicHistogram1D* vz_nminus1 = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_p_nminus1 = nullptr;
        clas12::framework::AtomicHistogram2D* beta_vs_p_nminus1 = nullptr;

        // Additional diagnostics
        clas12::framework::AtomicHistogram2D* beta_diff_vs_p = nullptr;
        clas12::framework::AtomicHistogram1D* tof_mass2 = nullptr;
        clas12::framework::AtomicHistogram1D* tof_mass2_precut = nullptr;
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    auto define_cutflows() -> void {
        auto define = [this](clas12::framework::ContextCutflow<PionCandidate>& cf) {
            cf.define_always("All Pions")
              .define("Detector Region", [this](const PionCandidate& c) {
                  if (!m_enable_detector_region_cut.get()) return true;
                  return c.is_forward_detector ||
                         (c.is_central_detector && m_enable_central_pions.get());
              })
              .define("DC Fiducial", [this](const PionCandidate& c) {
                  if (!m_enable_fiducial_cuts.get()) return true;
                  if (!c.is_forward_detector) return true;  // Central: no fiducial
                  return c.passes_dc_fiducial;
              })
              .define("Chi2 PID", [this](const PionCandidate& c) {
                  return !m_enable_chi2pid_cut.get() || c.passes_chi2pid;
              })
              .define("Momentum", [this](const PionCandidate& c) {
                  return !m_enable_momentum_cut.get() || c.passes_momentum;
              })
              .define("Vertex Z", [this](const PionCandidate& c) {
                  return !m_enable_vz_cut.get() || c.passes_vz;
              })
              .define("Delta-t", [this](const PionCandidate& c) {
                  if (!m_enable_delta_t_cut.get()) return true;
                  if (!c.is_forward_detector) return true;  // Central: no delta_t
                  return c.passes_delta_t;
              });
        };
        define(m_cutflow_piplus);
        define(m_cutflow_piminus);
    }

    auto book_histograms() -> void {
        auto* svc = m_hist;

        // Helper lambda to book histograms for a pion charge
        auto book_pion_histograms = [svc](PionHistograms& h, const std::string& dir, const std::string& label) {
            std::string fwd = dir + "/Forward";

            // Forward post-cut 1D histograms
            h.momentum = svc->book_momentum_hist(fwd + "/momentum", label + " Forward Momentum");
            h.theta = svc->book_theta_hist(fwd + "/theta", label + " Forward #theta");
            h.phi = svc->book_phi_hist(fwd + "/phi", label + " Forward #phi");
            h.vz = svc->book_hist(fwd + "/vz", label + " Forward v_{z};v_{z} [cm];Events", 200, -40, 40);
            h.chi2pid = svc->book_hist(fwd + "/chi2pid", label + " Forward #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);
            h.beta = svc->book_hist(fwd + "/beta", label + " Forward #beta;#beta;Events", 100, 0.7, 1.05);

            // Forward pre-cut 1D histograms (for efficiency studies)
            h.momentum_precut = svc->book_momentum_hist(fwd + "/momentum_precut", label + " Forward Momentum (pre-cut)");
            h.theta_precut = svc->book_theta_hist(fwd + "/theta_precut", label + " Forward #theta (pre-cut)");
            h.phi_precut = svc->book_phi_hist(fwd + "/phi_precut", label + " Forward #phi (pre-cut)");
            h.vz_precut = svc->book_hist(fwd + "/vz_precut", label + " Forward v_{z} (pre-cut);v_{z} [cm];Events", 200, -40, 40);
            h.chi2pid_precut = svc->book_hist(fwd + "/chi2pid_precut", label + " Forward #chi^{2}_{PID} (pre-cut);#chi^{2};Events", 200, -10, 10);
            h.delta_t_precut = svc->book_hist(fwd + "/delta_t_precut", label + " Forward #Deltat (pre-cut);#Deltat [ns];Events", 200, -2, 2);

            // PCAL fiducial diagnostics (pre-cut, before fiducial cuts)
            h.pcal_lv_precut = svc->book_hist(fwd + "/pcal_lv_precut", label + " Forward PCAL lv (pre-cut);lv [cm];Events", 200, 0, 450);
            h.pcal_lw_precut = svc->book_hist(fwd + "/pcal_lw_precut", label + " Forward PCAL lw (pre-cut);lw [cm];Events", 200, 0, 450);
            h.pcal_lw_vs_lv = svc->book_hist2d(fwd + "/pcal_lw_vs_lv", label + " Forward PCAL lw vs lv (pre-cut);lv [cm];lw [cm]",
                200, 0, 450, 200, 0, 450);

            // Forward post-cut 2D histograms
            h.beta_vs_p = svc->book_hist2d(fwd + "/beta_vs_p", label + " Forward #beta vs p;p [GeV/c];#beta",
                100, 0, 10, 100, 0.7, 1.05);
            h.delta_t_vs_p = svc->book_hist2d(fwd + "/delta_t_vs_p", label + " Forward #Deltat vs p;p [GeV/c];#Deltat [ns]",
                100, 0, 10, 1000, -2, 2);
            h.p_vs_theta = svc->book_hist2d(fwd + "/p_vs_theta", label + " Forward p vs #theta", 200, 0, 50, 200, 0, 10);

            // Forward pre-cut 2D correlation histograms (for diagnostics)
            h.chi2pid_vs_p = svc->book_hist2d(fwd + "/chi2pid_vs_p", label + " Forward #chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}",
                100, 0, 10, 100, -10, 10);
            h.vz_vs_theta = svc->book_hist2d(fwd + "/vz_vs_theta", label + " Forward v_{z} vs #theta;#theta [deg];v_{z} [cm]",
                90, 0, 45, 100, -40, 40);
            h.delta_t_vs_theta = svc->book_hist2d(fwd + "/delta_t_vs_theta", label + " Forward #Deltat vs #theta;#theta [deg];#Deltat [ns]",
                90, 0, 45, 100, -2, 2);
            h.delta_t_vs_p_precut = svc->book_hist2d(fwd + "/delta_t_vs_p_precut", label + " Forward #Deltat vs p (pre-cut);p [GeV/c];#Deltat [ns]",
                100, 0, 10, 1000, -2, 2);

            // Central detector histograms (pre-cut)
            h.momentum_central = svc->book_momentum_hist(dir + "/Central/momentum", label + " Central Momentum");
            h.theta_central = svc->book_theta_hist(dir + "/Central/theta", label + " Central #theta");
            h.phi_central = svc->book_phi_hist(dir + "/Central/phi", label + " Central #phi");
            h.vz_central = svc->book_hist(dir + "/Central/vz", label + " Central v_{z};v_{z} [cm];Events", 200, -40, 40);
            h.chi2pid_central = svc->book_hist(dir + "/Central/chi2pid", label + " Central #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);
            h.beta_central = svc->book_hist(dir + "/Central/beta", label + " Central #beta;#beta;Events", 100, 0.0, 1.2);
            h.delta_t_central = svc->book_hist(dir + "/Central/delta_t", label + " Central #Deltat;#Deltat [ns];Events", 200, -2, 2);
            h.beta_vs_p_central = svc->book_hist2d(dir + "/Central/beta_vs_p", label + " Central #beta vs p;p [GeV/c];#beta",
                100, 0, 10, 100, 0.0, 1.2);
            h.delta_t_vs_p_central = svc->book_hist2d(dir + "/Central/delta_t_vs_p", label + " Central #Deltat vs p;p [GeV/c];#Deltat [ns]",
                100, 0, 10, 1000, -2, 2);
            h.p_vs_theta_central = svc->book_hist2d(dir + "/Central/p_vs_theta", label + " Central p vs #theta;#theta [deg];p [GeV/c]",
                200, 0, 140, 200, 0, 5);

            // Forward sector-level histograms (post-cut)
            for (int sector = 1; sector <= 6; ++sector) {
                std::string sec_suffix = fmt::format("_S{}", sector);
                h.momentum_sector[sector - 1] = svc->book_momentum_hist(
                    fwd + "/momentum" + sec_suffix,
                    fmt::format("{} Forward Momentum (Sector {})", label, sector));
                h.theta_sector[sector - 1] = svc->book_theta_hist(
                    fwd + "/theta" + sec_suffix,
                    fmt::format("{} Forward #theta (Sector {})", label, sector));
                h.delta_t_vs_p_sector[sector - 1] = svc->book_hist2d(
                    fwd + "/delta_t_vs_p" + sec_suffix,
                    fmt::format("{} Forward #Deltat vs p (Sector {});p [GeV/c];#Deltat [ns]", label, sector),
                    100, 0, 10, 200, -2, 2);
                h.delta_t_vs_p_precut_sector[sector - 1] = svc->book_hist2d(
                    fwd + "/delta_t_vs_p_precut" + sec_suffix,
                    fmt::format("{} Forward #Deltat vs p pre-cut (Sector {});p [GeV/c];#Deltat [ns]", label, sector),
                    100, 0, 10, 200, -2, 2);
            }

            // N-1 histograms (forward only)
            std::string nm1 = fwd + "/NMinus1";
            h.chi2pid_nminus1 = svc->book_hist(nm1 + "/chi2pid", label + " Forward #chi^{2}_{PID} (N-1);#chi^{2};Events", 200, -10, 10);
            h.momentum_nminus1 = svc->book_momentum_hist(nm1 + "/momentum", label + " Forward Momentum (N-1)");
            h.vz_nminus1 = svc->book_hist(nm1 + "/vz", label + " Forward v_{z} (N-1);v_{z} [cm];Events", 200, -40, 40);
            h.delta_t_vs_p_nminus1 = svc->book_hist2d(nm1 + "/delta_t_vs_p", label + " Forward #Deltat vs p (N-1);p [GeV/c];#Deltat [ns]",
                100, 0, 10, 200, -2, 2);
            h.beta_vs_p_nminus1 = svc->book_hist2d(nm1 + "/beta_vs_p", label + " Forward #beta vs p (N-1);p [GeV/c];#beta",
                100, 0, 10, 100, 0.7, 1.05);

            // Additional diagnostics
            h.beta_diff_vs_p = svc->book_hist2d(fwd + "/beta_diff_vs_p", label + " Forward #Delta#beta vs p;p [GeV/c];#beta_{meas} - #beta_{exp}",
                100, 0, 10, 200, -0.1, 0.1);
            h.tof_mass2 = svc->book_hist(fwd + "/tof_mass2", label + " Forward TOF m^{2};m^{2} [GeV^{2}];Events", 200, -0.5, 1.5);
            h.tof_mass2_precut = svc->book_hist(fwd + "/tof_mass2_precut", label + " Forward TOF m^{2} (pre-cut);m^{2} [GeV^{2}];Events", 200, -0.5, 1.5);
        };

        // Book histograms for π⁺ and π⁻
        book_pion_histograms(h_piplus, "/Pion/PiPlus", "#pi^{+}");
        book_pion_histograms(h_piminus, "/Pion/PiMinus", "#pi^{-}");
    }

    auto setup_ntuples() -> void {
        // Setup N-tuple using NTupleWriter
        m_ntuple_piplus.init(m_ntuple, "pion_plus");
        m_ntuple_piminus.init(m_ntuple, "pion_minus");

        Log::info(name(), "N-tuple branches registered for π⁺ and π⁻");
    }

    /**
     * @brief Build fully-computed PionCandidate objects from particle list
     *
     * Computes all kinematic variables once and applies cuts, storing results
     * in the candidate struct to avoid redundant calculations downstream.
     */
    auto build_pion_candidates(
        const std::vector<clas12::Particle>& pions,
        const hipo::bank_view& REC_Scintillator,
        const hipo::bank_view& REC_Calorimeter,
        const hipo::bank_view& REC_Traj,
        double start_time,
        bool is_piplus
    ) -> std::vector<PionCandidate> {
        std::vector<PionCandidate> candidates;
        if (pions.empty()) return candidates;

        for (const auto& pion : pions) {
            // Skip if PID does not match
            if (is_piplus && pion.pdg() != clas12::PIDs::PIPLUS) continue;
            if (!is_piplus && pion.pdg() != clas12::PIDs::PIMINUS) continue;

            PionCandidate cand;
            cand.pion = pion;

            // Extract basic kinematics
            cand.p = pion.p();
            cand.theta = pion.theta();
            cand.phi = pion.phi();
            cand.vz = pion.vz();
            cand.vx = pion.vx();
            cand.vy = pion.vy();
            cand.chi2pid = pion.chi2pid();
            cand.beta = pion.beta();

            // Check forward detector status and count detector regions
            cand.is_forward_detector = clas12::framework::CandidateUtils::is_forward_detector(pion.status());
            cand.is_central_detector = clas12::framework::CandidateUtils::is_central_detector(pion.status());
            if (is_piplus) {
                if (cand.is_forward_detector) m_piplus_forward++;
                else if (cand.is_central_detector) m_piplus_central++;
                else m_piplus_other++;
            } else {
                if (cand.is_forward_detector) m_piminus_forward++;
                else if (cand.is_central_detector) m_piminus_central++;
                else m_piminus_other++;
            }

            // Get scintillator info (FTOF for forward pions only)
            if (cand.is_forward_detector) {
                clas12::ScintillatorBank scintillator = clas12::read_Scintillator_bank(REC_Scintillator, pion.index());
                cand.sector = scintillator.sector;
                cand.tof_time = scintillator.time;
                cand.tof_path = scintillator.path;
            }

            // Calculate timing quantities
            auto timing = clas12::framework::CandidateUtils::compute_timing(
                cand.p, clas12::Constants::PIMASS, cand.beta,
                cand.tof_time, cand.tof_path, start_time);
            cand.beta_expected = timing.beta_expected;
            cand.delta_t = timing.delta_t;
            cand.tof_mass2 = timing.tof_mass2;
            cand.beta_diff = timing.beta_diff;

            // Read calorimeter and trajectory banks for fiducial cuts
            auto calo = clas12::read_Calorimeter_bank(REC_Calorimeter, pion.index());
            auto traj = clas12::read_Traj_bank(REC_Traj, pion.index());

            // Store PCAL fiducial variables and count NaN
            cand.pcal_lv = calo.pcal.lv;
            cand.pcal_lw = calo.pcal.lw;
            if (cand.is_forward_detector && (std::isnan(cand.pcal_lv) || std::isnan(cand.pcal_lw))) {
                if (is_piplus) m_piplus_pcal_nan++;
                else m_piminus_pcal_nan++;
            }

            // Cache individual cut results (used by histograms and sigma scan)
            cand.passes_dc_fiducial = pass_dc_fiducial_cut(traj, is_piplus);
            cand.passes_chi2pid = pass_chi2pid_cut(cand.chi2pid);
            cand.passes_momentum = pass_momentum_cut(cand.p);
            cand.passes_vz = pass_vz_cut(cand.vz, is_piplus);
            cand.passes_delta_t = pass_delta_t_cut(cand.p, cand.delta_t, is_piplus);
            // Note: passes_all_cuts is set later via ContextCutflow::evaluate()

            candidates.push_back(std::move(cand));
        }

        return candidates;
    }

    /**
     * @brief Fill histograms for candidates before kinematic cuts (for efficiency studies)
     */
    auto fill_pre_cut_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& h = is_piplus ? h_piplus : h_piminus;

        for (const auto& cand : candidates) {
            // Forward detector pre-cut histograms
            if (cand.is_forward_detector) {
                if (h.momentum_precut) h.momentum_precut->fill(cand.p);
                if (h.theta_precut) h.theta_precut->fill(cand.theta);
                if (h.phi_precut) h.phi_precut->fill(cand.phi);
                if (h.vz_precut) h.vz_precut->fill(cand.vz);
                if (h.chi2pid_precut) h.chi2pid_precut->fill(cand.chi2pid);
                if (h.delta_t_precut && cand.delta_t != 0.0) h.delta_t_precut->fill(cand.delta_t);

                // PCAL fiducial diagnostics (before fiducial cuts)
                if (h.pcal_lv_precut) h.pcal_lv_precut->fill(cand.pcal_lv);
                if (h.pcal_lw_precut) h.pcal_lw_precut->fill(cand.pcal_lw);
                if (h.pcal_lw_vs_lv) h.pcal_lw_vs_lv->fill(cand.pcal_lv, cand.pcal_lw);

                // 2D correlations (pre-cut for diagnostics)
                if (h.chi2pid_vs_p) h.chi2pid_vs_p->fill(cand.p, cand.chi2pid);
                if (h.vz_vs_theta) h.vz_vs_theta->fill(cand.theta, cand.vz);
                if (h.delta_t_vs_theta && cand.delta_t != 0.0) h.delta_t_vs_theta->fill(cand.theta, cand.delta_t);
                if (h.delta_t_vs_p_precut && cand.delta_t != 0.0) h.delta_t_vs_p_precut->fill(cand.p, cand.delta_t);

                // Per-sector delta_t vs p (pre-cut)
                if (cand.sector >= 1 && cand.sector <= 6) {
                    int idx = cand.sector - 1;
                    if (h.delta_t_vs_p_precut_sector[idx] && cand.delta_t != 0.0) h.delta_t_vs_p_precut_sector[idx]->fill(cand.p, cand.delta_t);
                }

                // Additional diagnostics (pre-cut)
                if (h.beta_diff_vs_p) h.beta_diff_vs_p->fill(cand.p, cand.beta_diff);
                if (h.tof_mass2_precut) h.tof_mass2_precut->fill(cand.tof_mass2);
            }

            // Central detector pre-cut histograms
            if (cand.is_central_detector) {
                if (h.momentum_central) h.momentum_central->fill(cand.p);
                if (h.theta_central) h.theta_central->fill(cand.theta);
                if (h.phi_central) h.phi_central->fill(cand.phi);
                if (h.vz_central) h.vz_central->fill(cand.vz);
                if (h.chi2pid_central) h.chi2pid_central->fill(cand.chi2pid);
                if (h.beta_central) h.beta_central->fill(cand.beta);
                if (h.delta_t_central && cand.delta_t != 0.0) h.delta_t_central->fill(cand.delta_t);
                if (h.beta_vs_p_central) h.beta_vs_p_central->fill(cand.p, cand.beta);
                if (h.delta_t_vs_p_central && cand.delta_t != 0.0) h.delta_t_vs_p_central->fill(cand.p, cand.delta_t);
                if (h.p_vs_theta_central) h.p_vs_theta_central->fill(cand.theta, cand.p);
            }
        }
    }

    /**
     * @brief Fill histograms for candidates that passed all cuts
     */
    auto fill_post_cut_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& h = is_piplus ? h_piplus : h_piminus;

        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;

            // Basic kinematics (post-cut)
            if (h.momentum) h.momentum->fill(cand.p);
            if (h.theta) h.theta->fill(cand.theta);
            if (h.phi) h.phi->fill(cand.phi);
            if (h.vz) h.vz->fill(cand.vz);
            if (h.chi2pid) h.chi2pid->fill(cand.chi2pid);
            if (h.beta) h.beta->fill(cand.beta);

            // 2D histograms (post-cut)
            if (h.beta_vs_p) h.beta_vs_p->fill(cand.p, cand.beta);
            if (h.delta_t_vs_p && cand.delta_t != 0.0) h.delta_t_vs_p->fill(cand.p, cand.delta_t);
            if (h.p_vs_theta) h.p_vs_theta->fill(cand.theta, cand.p);

            // TOF mass² (post-cut)
            if (h.tof_mass2) h.tof_mass2->fill(cand.tof_mass2);

            // Sector-level histograms
            if (cand.sector >= 1 && cand.sector <= 6) {
                int idx = cand.sector - 1;
                if (h.momentum_sector[idx]) h.momentum_sector[idx]->fill(cand.p);
                if (h.theta_sector[idx]) h.theta_sector[idx]->fill(cand.theta);
                if (h.delta_t_vs_p_sector[idx] && cand.delta_t != 0.0) h.delta_t_vs_p_sector[idx]->fill(cand.p, cand.delta_t);
            }
        }
    }

    /**
     * @brief Fill N-tuple for passing candidates
     */
    auto fill_ntuple(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& ntuple = is_piplus ? m_ntuple_piplus : m_ntuple_piminus;
        if (!ntuple) return;

        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;
            ntuple.data().from(cand);
            ntuple.fill();
        }
    }

    /**
     * @brief Check if a candidate passes all enabled cuts except the one at index `skip`
     *
     * Indices: 0=detector_region, 1=dc_fiducial, 2=chi2pid, 3=momentum, 4=vz, 5=delta_t
     */
    auto passes_all_except_pion(const PionCandidate& c, int skip) const -> bool {
        bool pass_det = (skip == 0) || !m_enable_detector_region_cut.get() ||
                        c.is_forward_detector ||
                        (c.is_central_detector && m_enable_central_pions.get());
        bool pass_fid = (skip == 1) || !m_enable_fiducial_cuts.get() ||
                        !c.is_forward_detector || c.passes_dc_fiducial;
        bool pass_chi2 = (skip == 2) || !m_enable_chi2pid_cut.get() || c.passes_chi2pid;
        bool pass_mom = (skip == 3) || !m_enable_momentum_cut.get() || c.passes_momentum;
        bool pass_vz = (skip == 4) || !m_enable_vz_cut.get() || c.passes_vz;
        bool pass_dt = (skip == 5) || !m_enable_delta_t_cut.get() ||
                       !c.is_forward_detector || c.passes_delta_t;
        return pass_det && pass_fid && pass_chi2 && pass_mom && pass_vz && pass_dt;
    }

    /**
     * @brief Fill N-1 histograms for forward pion candidates
     */
    auto fill_nminus1_histograms(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& h = is_piplus ? h_piplus : h_piminus;

        for (const auto& cand : candidates) {
            if (!cand.is_forward_detector) continue;

            // N-1 w.r.t. chi2pid (skip=2)
            if (passes_all_except_pion(cand, 2) && h.chi2pid_nminus1) {
                h.chi2pid_nminus1->fill(cand.chi2pid);
            }
            // N-1 w.r.t. momentum (skip=3)
            if (passes_all_except_pion(cand, 3) && h.momentum_nminus1) {
                h.momentum_nminus1->fill(cand.p);
            }
            // N-1 w.r.t. vertex Z (skip=4)
            if (passes_all_except_pion(cand, 4) && h.vz_nminus1) {
                h.vz_nminus1->fill(cand.vz);
            }
            // N-1 w.r.t. delta-t (skip=5): show timing and beta bands
            if (passes_all_except_pion(cand, 5)) {
                if (h.delta_t_vs_p_nminus1 && cand.delta_t != 0.0) h.delta_t_vs_p_nminus1->fill(cand.p, cand.delta_t);
                if (h.beta_vs_p_nminus1) h.beta_vs_p_nminus1->fill(cand.p, cand.beta);
            }
        }
    }

    // Note: update_cutflow() removed -- ContextCutflow::evaluate() handles per-candidate tracking

    /**
     * @brief Collect particles that passed all cuts from candidates
     */
    auto collect_passing_particles(const std::vector<PionCandidate>& candidates) -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> selected;
        for (const auto& cand : candidates) {
            if (cand.passes_all_cuts) {
                selected.push_back(cand.pion);
            }
        }
        return selected;
    }

    /**
     * @brief Collect particles passing cumulative pion cuts up to a given step
     *
     * Steps: 0=forward only, 1=+DC fiducial, 2=+chi2PID, 3=+momentum, 4=+vz, 5=+delta_t
     * Each step adds one more cut on top of all previous ones.
     */
    auto collect_passing_cumulative(const std::vector<PionCandidate>& candidates, int max_step) const
        -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> selected;
        for (const auto& cand : candidates) {
            if (!cand.is_forward_detector) continue;
            if (max_step >= 1 && m_enable_fiducial_cuts.get() && !cand.passes_dc_fiducial) continue;
            if (max_step >= 2 && m_enable_chi2pid_cut.get() && !cand.passes_chi2pid) continue;
            if (max_step >= 3 && m_enable_momentum_cut.get() && !cand.passes_momentum) continue;
            if (max_step >= 4 && m_enable_vz_cut.get() && !cand.passes_vz) continue;
            if (max_step >= 5 && m_enable_delta_t_cut.get() && cand.is_forward_detector && !cand.passes_delta_t) continue;
            selected.push_back(cand.pion);
        }
        return selected;
    }

    /**
     * @brief Second-order polynomial helper: a*x² + b*x + c
     */
    static auto second_order_poly(double x, double a, double b, double c) -> double {
        return a * x * x + b * x + c;
    }

    /**
     * @brief Coefficients for delta_t cut parameterization
     * Format: {a, b, c} for polynomial a*p² + b*p + c
     */
    struct DeltaTCoefficients {
        std::array<double, 3> mean;   // Mean coefficients {a, b, c}
        std::array<double, 3> sigma;  // Sigma coefficients {a, b, c}
    };

    /**
     * @brief Get delta_t cut coefficients for pi+ based on target
     */
    auto get_piplus_delta_t_coefficients() const -> DeltaTCoefficients {
        switch (m_target) {
            case clas12::Target::LD2:
                return {{0.000922, -0.010435, 0.046222},
                        {0.002655, -0.021940, 0.114758}};
            case clas12::Target::CxC:
                return {{-0.000063, -0.004623, 0.039045},
                        {0.001879, -0.017147, 0.108605}};
            case clas12::Target::Cu:
                return {{0.001801, -0.015591, 0.051889},
                        {0.003742, -0.027802, 0.120968}};
            case clas12::Target::Sn:
                return {{-0.000403, -0.001866, 0.031458},
                        {0.001510, -0.014369, 0.104379}};
            default:
                // Default to LD2 coefficients
                return {{0.000922, -0.010435, 0.046222},
                        {0.002655, -0.021940, 0.114758}};
        }
    }

    /**
     * @brief Get delta_t cut coefficients for pi- based on target
     */
    auto get_piminus_delta_t_coefficients() const -> DeltaTCoefficients {
        switch (m_target) {
            case clas12::Target::LD2:
                return {{-0.001635, 0.009566, -0.025434},
                        {0.012220, -0.089536, 0.229427}};
            case clas12::Target::CxC:
                return {{-0.001694, 0.009962, -0.025835},
                        {0.012632, -0.092243, 0.232097}};
            case clas12::Target::Cu:
                return {{-0.001349, 0.007585, -0.022235},
                        {0.013710, -0.095575, 0.233176}};
            case clas12::Target::Sn:
                return {{-0.001719, 0.010436, -0.029489},
                        {0.015162, -0.104777, 0.245153}};
            default:
                // Default to LD2 coefficients
                return {{-0.001635, 0.009566, -0.025434},
                        {0.012220, -0.089536, 0.229427}};
        }
    }

    /**
     * @brief Vertex z cut range structure
     */
    struct VzCutRange {
        double min;
        double max;

        [[nodiscard]] auto contains(double vz) const -> bool {
            return vz >= min && vz <= max;
        }
    };

    /**
     * @brief Get vertex z cut range for pi+ based on target
     */
    auto get_piplus_vz_range() const -> VzCutRange {
        switch (m_target) {
            case clas12::Target::LD2:
                return {-15.00, 5.00};
            case clas12::Target::CxC:
                return {-9.75, 5.00};
            case clas12::Target::Cu:
                return {-9.21, -5.80};
            case clas12::Target::Sn:
                return {-4.83, -0.34};
            default:
                // Default to LD2 range
                return {-15.00, 5.00};
        }
    }

    /**
     * @brief Get vertex z cut range for pi- based on target
     */
    auto get_piminus_vz_range() const -> VzCutRange {
        switch (m_target) {
            case clas12::Target::LD2:
                return {-15.00, 5.00};
            case clas12::Target::CxC:
                return {-11.69, 5.00};
            case clas12::Target::Cu:
                return {-11.57, -5.57};
            case clas12::Target::Sn:
                return {-6.52, -0.61};
            default:
                // Default to LD2 range
                return {-15.00, 5.00};
        }
    }

    // Fiducial cut functions
    static auto pass_dc_fiducial_cut(const clas12::DCTrajBank& traj, bool is_piplus) -> bool {
        double r1_min = is_piplus ? 3.875 : 2.125;
        double r2_min = is_piplus ? 4.375 : 3.625;
        double r3_min = is_piplus ? 2.625 : 9.625;
        return traj.region1.edge > r1_min &&
               traj.region2.edge > r2_min &&
               traj.region3.edge > r3_min;
    }

    // Cut functions - using configurable properties
    auto pass_chi2pid_cut(double chi2pid) -> bool {
        return m_chi2pid_min.get() < chi2pid && chi2pid < m_chi2pid_max.get();
    }

    auto pass_momentum_cut(double p) -> bool {
        return p > m_momentum_min.get() && p < m_momentum_max.get();
    }

    /**
     * @brief Vertex z cut using target-specific and charge-specific ranges
     */
    auto pass_vz_cut(double vz, bool is_piplus) -> bool {
        auto vz_range = is_piplus ? get_piplus_vz_range() : get_piminus_vz_range();
        return vz_range.contains(vz);
    }

    /**
     * @brief Time-of-flight cut with configurable number of sigma
     */
    auto pass_delta_t_cut(const double p, const double delta_t, bool is_piplus, double n_sigma) -> bool {
        auto coeffs = is_piplus ? get_piplus_delta_t_coefficients()
                                : get_piminus_delta_t_coefficients();

        double mean = second_order_poly(p, coeffs.mean[0], coeffs.mean[1], coeffs.mean[2]);
        double sigma = second_order_poly(p, coeffs.sigma[0], coeffs.sigma[1], coeffs.sigma[2]);

        double cut_low = mean - n_sigma * sigma;
        double cut_high = mean + n_sigma * sigma;

        return cut_low < delta_t && delta_t < cut_high;
    }

    /**
     * @brief Time-of-flight cut using default sigma from property
     */
    auto pass_delta_t_cut(const double p, const double delta_t, bool is_piplus) -> bool {
        return pass_delta_t_cut(p, delta_t, is_piplus, m_delta_t_sigma.get());
    }

    // ========================================================================
    // Sigma scan helpers
    // ========================================================================

    static auto format_sigma_label(double sigma) -> std::string {
        return fmt::format("sigma_{:.1f}", sigma);
    }

    static auto format_sigma_key(const std::string& base, double sigma) -> std::string {
        return fmt::format("{}_{}", base, format_sigma_label(sigma));
    }

    /**
     * @brief Collect particles passing all cuts with a specific delta_t sigma
     *
     * Reuses already-built candidates, only re-evaluating the delta_t decision.
     */
    auto collect_passing_with_sigma(const std::vector<PionCandidate>& candidates,
                                    bool is_piplus, double n_sigma) -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> selected;
        for (const auto& cand : candidates) {
            bool scan_passes_delta_t = pass_delta_t_cut(cand.p, cand.delta_t, is_piplus, n_sigma);

            bool passes_detector_region = !m_enable_detector_region_cut.get() ||
                                          cand.is_forward_detector ||
                                          (cand.is_central_detector && m_enable_central_pions.get());
            bool passes_fiducial = !m_enable_fiducial_cuts.get() ||
                                   !cand.is_forward_detector ||
                                   cand.passes_dc_fiducial;
            bool passes_timing = !m_enable_delta_t_cut.get() ||
                                 (cand.is_forward_detector ? scan_passes_delta_t : true);

            bool passes_all = passes_detector_region &&
                              passes_fiducial &&
                              (!m_enable_chi2pid_cut.get() || cand.passes_chi2pid) &&
                              (!m_enable_momentum_cut.get() || cand.passes_momentum) &&
                              (!m_enable_vz_cut.get() || cand.passes_vz) &&
                              passes_timing;

            if (passes_all) {
                selected.push_back(cand.pion);
            }
        }
        return selected;
    }
};

}  // namespace clas12::algorithms
