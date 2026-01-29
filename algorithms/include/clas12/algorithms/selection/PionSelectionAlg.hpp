#pragma once

#include <array>
#include <cmath>
#include <clas12/Constants.hpp>
#include <clas12/ElectronCuts.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
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
    double chi2pid = 0.0;
    double beta = 0.0;
    double delta_t = 0.0;

    NTUPLE_FIELDS(p, theta, phi, vz, chi2pid, beta, delta_t)
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

    // Cached cut decisions
    bool is_forward_detector = false;
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
                              bool separate_charges = true)
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
    }

    /**
     * @brief Configure required and optional services
     */
    auto configure() -> void override {
        // Required services
        m_hist_svc = &require_service<clas12::framework::HistogramService>("HistogramService");
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");

        // Optional services - analysis continues even if not available
        m_log_svc = &optional_service<clas12::framework::LoggingService>("LoggingService");
        m_ntuple_svc = &optional_service<clas12::framework::NTupleService>("NTupleService");
    }

    /**
     * @brief Custom initialization after services are injected
     */
    auto on_initialize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Initializing pion selection algorithm");
        Log::info(name(), "Target: {}, Separate charges: {}",
                  m_target == clas12::Target::LD2 ? "LD2" : "Other",
                  m_separate_charges ? "yes" : "no");
        Log::info(name(), "Chi2PID range: [{}, {}], Momentum range: [{}, {}] GeV",
                  m_chi2pid_min.get(), m_chi2pid_max.get(),
                  m_momentum_min.get(), m_momentum_max.get());

        // Setup cutflows
        setup_cutflows();

        // Book histograms using new histogram templates
        book_histograms();

        // Setup N-tuple branches if service is available
        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
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

        // Get banks using structured bindings
        auto& hipo_event = event.hipo_event();
        auto [REC_Event, REC_Particle, REC_Scintillator] =
            hipo_event.banks("REC::Event", "REC::Particle", "REC::Scintillator");

        if (REC_Event->rows() == 0) return clas12::framework::StatusCode::SUCCESS();

        double start_time = REC_Event->get<double>("startTime", 0);

        // Extract pions using ParticleSelectorService
        clas12::framework::ParticleMap particles = m_particle_svc->get()->extract_particles(
            *REC_Particle,
            {clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS});

        // Build π⁺ candidates with all kinematics computed once
        const auto& pions_plus = clas12::framework::ParticleSelectorService::get_pions_plus(particles);
        auto piplus_candidates = build_pion_candidates(pions_plus, *REC_Scintillator, start_time, true);

        // Fill pre-cut histograms, update cutflow, fill post-cut histograms and N-tuple
        fill_pre_cut_histograms(piplus_candidates, true);
        update_cutflow(piplus_candidates, true);
        fill_post_cut_histograms(piplus_candidates, true);
        fill_ntuple(piplus_candidates, true);

        // Collect passing particles
        auto selected_piplus = collect_passing_particles(piplus_candidates);

        // Build π⁻ candidates with all kinematics computed once
        const auto& pions_minus = clas12::framework::ParticleSelectorService::get_pions_minus(particles);
        auto piminus_candidates = build_pion_candidates(pions_minus, *REC_Scintillator, start_time, false);

        // Fill pre-cut histograms, update cutflow, fill post-cut histograms and N-tuple
        fill_pre_cut_histograms(piminus_candidates, false);
        update_cutflow(piminus_candidates, false);
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

        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print cutflow summary
     */
    auto on_finalize() -> clas12::framework::StatusCode override {
        Log::info(name(), "Finalizing pion selection algorithm");

        auto piplus_count = m_cutflow_piplus.get_count("Delta-t");
        auto piminus_count = m_cutflow_piminus.get_count("Delta-t");

        Log::info(name(), "π⁺ passing all cuts: {}", piplus_count);
        Log::info(name(), "π⁻ passing all cuts: {}", piminus_count);

        if (m_ntuple_svc && m_ntuple_svc->is_valid()) {
            auto piplus_entries = m_ntuple_svc->get()->get_entries("pion_plus");
            auto piminus_entries = m_ntuple_svc->get()->get_entries("pion_minus");
            Log::info(name(), "N-tuple entries: π⁺={}, π⁻={}", piplus_entries, piminus_entries);
        }

        fmt::print("\n");
        m_cutflow_piplus.print();
        fmt::print("\n");
        m_cutflow_piminus.print();

        fmt::print("\nPion Analysis Statistics:\n");
        fmt::print("  π⁺ passing all cuts: {}\n", m_cutflow_piplus.get_count("Delta-t"));
        fmt::print("  π⁻ passing all cuts: {}\n", m_cutflow_piminus.get_count("Delta-t"));

        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::Target m_target;
    bool m_separate_charges;
    clas12::framework::Cutflow m_cutflow_piplus;
    clas12::framework::Cutflow m_cutflow_piminus;

    // Configurable properties
    clas12::framework::Property<double> m_chi2pid_min{"chi2pid_min", -10.0, "Minimum chi2PID"};
    clas12::framework::Property<double> m_chi2pid_max{"chi2pid_max", 10.0, "Maximum chi2PID"};
    clas12::framework::Property<double> m_momentum_min{"momentum_min", 0.3, "Minimum pion momentum [GeV]"};
    clas12::framework::Property<double> m_momentum_max{"momentum_max", 10.0, "Maximum pion momentum [GeV]"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles (auto-injected via require_service/optional_service)
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::LoggingService>* m_log_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;

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

        // Additional 2D correlations (pre-cut for diagnostics)
        clas12::framework::AtomicHistogram2D* chi2pid_vs_p = nullptr;
        clas12::framework::AtomicHistogram2D* vz_vs_theta = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_theta = nullptr;
        clas12::framework::AtomicHistogram2D* delta_t_vs_p_precut = nullptr;

        // Sector-level histograms (post-cut)
        std::array<clas12::framework::AtomicHistogram1D*, 6> momentum_sector = {};
        std::array<clas12::framework::AtomicHistogram1D*, 6> theta_sector = {};
    };

    PionHistograms h_piplus;
    PionHistograms h_piminus;

    auto setup_cutflows() -> void {
        // π⁺ cutflow
        m_cutflow_piplus.add_cut("All Events");
        m_cutflow_piplus.add_cut("Has π⁺");
        m_cutflow_piplus.add_cut("Forward detector");
        m_cutflow_piplus.add_cut("Chi2 PID");
        m_cutflow_piplus.add_cut("Momentum");
        m_cutflow_piplus.add_cut("Vertex Z");
        m_cutflow_piplus.add_cut("Delta-t");

        // π⁻ cutflow
        m_cutflow_piminus.add_cut("All Events");
        m_cutflow_piminus.add_cut("Has π⁻");
        m_cutflow_piminus.add_cut("Forward detector");
        m_cutflow_piminus.add_cut("Chi2 PID");
        m_cutflow_piminus.add_cut("Momentum");
        m_cutflow_piminus.add_cut("Vertex Z");
        m_cutflow_piminus.add_cut("Delta-t");
    }

    auto book_histograms() -> void {
        auto* svc = m_hist_svc->get();

        // Helper lambda to book histograms for a pion charge
        auto book_pion_histograms = [svc](PionHistograms& h, const std::string& dir, const std::string& label) {
            // Post-cut 1D histograms
            h.momentum = svc->book_momentum_hist(dir + "/momentum", label + " Momentum");
            h.theta = svc->book_theta_hist(dir + "/theta", label + " #theta");
            h.phi = svc->book_phi_hist(dir + "/phi", label + " #phi");
            h.vz = svc->book_hist(dir + "/vz", label + " v_{z};v_{z} [cm];Events", 200, -40, 40);
            h.chi2pid = svc->book_hist(dir + "/chi2pid", label + " #chi^{2}_{PID};#chi^{2};Events", 200, -10, 10);
            h.beta = svc->book_hist(dir + "/beta", label + " #beta;#beta;Events", 100, 0.7, 1.05);

            // Pre-cut 1D histograms (for efficiency studies)
            h.momentum_precut = svc->book_momentum_hist(dir + "/momentum_precut", label + " Momentum (pre-cut)");
            h.theta_precut = svc->book_theta_hist(dir + "/theta_precut", label + " #theta (pre-cut)");
            h.phi_precut = svc->book_phi_hist(dir + "/phi_precut", label + " #phi (pre-cut)");
            h.vz_precut = svc->book_hist(dir + "/vz_precut", label + " v_{z} (pre-cut);v_{z} [cm];Events", 200, -40, 40);
            h.chi2pid_precut = svc->book_hist(dir + "/chi2pid_precut", label + " #chi^{2}_{PID} (pre-cut);#chi^{2};Events", 200, -10, 10);
            h.delta_t_precut = svc->book_hist(dir + "/delta_t_precut", label + " #Deltat (pre-cut);#Deltat [ns];Events", 200, -2, 2);

            // Post-cut 2D histograms
            h.beta_vs_p = svc->book_hist2d(dir + "/beta_vs_p", label + " #beta vs p;p [GeV/c];#beta",
                100, 0, 10, 100, 0.7, 1.05);
            h.delta_t_vs_p = svc->book_hist2d(dir + "/delta_t_vs_p", label + " #Deltat vs p;p [GeV/c];#Deltat [ns]",
                100, 0, 10, 200, -2, 2);
            h.p_vs_theta = svc->book_hist2d(dir + "/p_vs_theta", label + " p vs #theta", 200, 0, 50, 200, 0, 10);

            // Pre-cut 2D correlation histograms (for diagnostics)
            h.chi2pid_vs_p = svc->book_hist2d(dir + "/chi2pid_vs_p", label + " #chi^{2}_{PID} vs p;p [GeV/c];#chi^{2}_{PID}",
                100, 0, 10, 100, -10, 10);
            h.vz_vs_theta = svc->book_hist2d(dir + "/vz_vs_theta", label + " v_{z} vs #theta;#theta [deg];v_{z} [cm]",
                90, 0, 45, 100, -40, 40);
            h.delta_t_vs_theta = svc->book_hist2d(dir + "/delta_t_vs_theta", label + " #Deltat vs #theta;#theta [deg];#Deltat [ns]",
                90, 0, 45, 100, -2, 2);
            h.delta_t_vs_p_precut = svc->book_hist2d(dir + "/delta_t_vs_p_precut", label + " #Deltat vs p;p [GeV/c];#Deltat [ns]",
                100, 0, 10, 200, -2, 2);

            // Sector-level histograms (post-cut)
            for (int sector = 1; sector <= 6; ++sector) {
                std::string sec_suffix = fmt::format("_S{}", sector);
                h.momentum_sector[sector - 1] = svc->book_momentum_hist(
                    dir + "/momentum" + sec_suffix,
                    fmt::format("{} Momentum (Sector {})", label, sector));
                h.theta_sector[sector - 1] = svc->book_theta_hist(
                    dir + "/theta" + sec_suffix,
                    fmt::format("{} #theta (Sector {})", label, sector));
            }
        };

        // Book histograms for π⁺ and π⁻
        book_pion_histograms(h_piplus, "/Pion/PiPlus", "#pi^{+}");
        book_pion_histograms(h_piminus, "/Pion/PiMinus", "#pi^{-}");
    }

    auto setup_ntuples() -> void {
        // Setup N-tuple using NTupleWriter
        m_ntuple_piplus.init(m_ntuple_svc->get(), "pion_plus");
        m_ntuple_piminus.init(m_ntuple_svc->get(), "pion_minus");

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
        hipo::bank& REC_Scintillator,
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
            cand.chi2pid = pion.chi2pid();
            cand.beta = pion.beta();

            // Check forward detector status
            cand.is_forward_detector = (pion.status() >= 2000 && pion.status() < 4000);

            // Get scintillator info
            clas12::ScintillatorBank scintillator = clas12::read_Scintillator_bank(
                REC_Scintillator, pion.index());
            cand.sector = scintillator.sector;
            cand.tof_time = scintillator.time;
            cand.tof_path = scintillator.path;

            // Calculate timing quantities
            double m = clas12::Constants::PIMASS;
            cand.beta_expected = cand.p / std::sqrt(cand.p * cand.p + m * m);
            double t_expected = cand.tof_path / (cand.beta_expected * clas12::Constants::SPEED_OF_LIGHT);
            double measured_time = cand.tof_time - start_time;
            cand.delta_t = measured_time - t_expected;

            // Apply cuts and cache results
            cand.passes_chi2pid = pass_chi2pid_cut(cand.chi2pid);
            cand.passes_momentum = pass_momentum_cut(cand.p);
            cand.passes_vz = pass_vz_cut(cand.vz);
            cand.passes_delta_t = pass_delta_t_cut(cand.p, cand.delta_t);
            cand.passes_all_cuts = cand.is_forward_detector &&
                                   cand.passes_chi2pid &&
                                   cand.passes_momentum &&
                                   cand.passes_vz &&
                                   cand.passes_delta_t;

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
            // Only fill for forward detector candidates (basic quality)
            if (!cand.is_forward_detector) continue;

            if (h.momentum_precut) h.momentum_precut->fill(cand.p);
            if (h.theta_precut) h.theta_precut->fill(cand.theta);
            if (h.phi_precut) h.phi_precut->fill(cand.phi);
            if (h.vz_precut) h.vz_precut->fill(cand.vz);
            if (h.chi2pid_precut) h.chi2pid_precut->fill(cand.chi2pid);
            if (h.delta_t_precut) h.delta_t_precut->fill(cand.delta_t);

            // 2D correlations (pre-cut for diagnostics)
            if (h.chi2pid_vs_p) h.chi2pid_vs_p->fill(cand.p, cand.chi2pid);
            if (h.vz_vs_theta) h.vz_vs_theta->fill(cand.theta, cand.vz);
            if (h.delta_t_vs_theta) h.delta_t_vs_theta->fill(cand.theta, cand.delta_t);
            if (h.delta_t_vs_p_precut) h.delta_t_vs_p_precut->fill(cand.p, cand.delta_t);
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
            if (h.delta_t_vs_p) h.delta_t_vs_p->fill(cand.p, cand.delta_t);
            if (h.p_vs_theta) h.p_vs_theta->fill(cand.theta, cand.p);

            // Sector-level histograms
            if (cand.sector >= 1 && cand.sector <= 6) {
                int idx = cand.sector - 1;
                if (h.momentum_sector[idx]) h.momentum_sector[idx]->fill(cand.p);
                if (h.theta_sector[idx]) h.theta_sector[idx]->fill(cand.theta);
            }
        }
    }

    /**
     * @brief Fill N-tuple for passing candidates
     */
    auto fill_ntuple(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& ntuple = is_piplus ? m_ntuple_piplus : m_ntuple_piminus;

        for (const auto& cand : candidates) {
            if (!cand.passes_all_cuts) continue;

            if (ntuple) {
                ntuple.data().p = cand.p;
                ntuple.data().theta = cand.theta;
                ntuple.data().phi = cand.phi;
                ntuple.data().vz = cand.vz;
                ntuple.data().chi2pid = cand.chi2pid;
                ntuple.data().beta = cand.beta;
                ntuple.data().delta_t = cand.delta_t;
                ntuple.fill();
            }
        }
    }

    /**
     * @brief Update cutflow counters based on candidate cut results
     */
    auto update_cutflow(const std::vector<PionCandidate>& candidates, bool is_piplus) -> void {
        auto& cutflow = is_piplus ? m_cutflow_piplus : m_cutflow_piminus;

        cutflow.pass("All Events");

        if (candidates.empty()) return;
        cutflow.pass(is_piplus ? "Has π⁺" : "Has π⁻");

        bool any_fd = false, any_chi2 = false, any_p = false, any_vz = false, any_dt = false;

        for (const auto& cand : candidates) {
            any_fd |= cand.is_forward_detector;
            if (cand.is_forward_detector) {
                any_chi2 |= cand.passes_chi2pid;
                if (cand.passes_chi2pid) {
                    any_p |= cand.passes_momentum;
                    if (cand.passes_momentum) {
                        any_vz |= cand.passes_vz;
                        if (cand.passes_vz) {
                            any_dt |= cand.passes_delta_t;
                        }
                    }
                }
            }
        }

        if (any_fd) cutflow.pass("Forward detector");
        if (any_chi2) cutflow.pass("Chi2 PID");
        if (any_p) cutflow.pass("Momentum");
        if (any_vz) cutflow.pass("Vertex Z");
        if (any_dt) cutflow.pass("Delta-t");
    }

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
     * @brief Third-order polynomial helper
     */
    static auto third_order_poly(double x, double a, double b, double c, double d) -> double {
        return a + b * x + c * x * x + d * x * x * x;
    }

    // Cut functions - using configurable properties
    auto pass_chi2pid_cut(double chi2pid) -> bool {
        return m_chi2pid_min.get() < chi2pid && chi2pid < m_chi2pid_max.get();
    }

    auto pass_momentum_cut(double p) -> bool {
        return p > m_momentum_min.get() && p < m_momentum_max.get();
    }

    auto pass_vz_cut(double vz) -> bool {
        // Use target-specific vertex cuts
        auto vz_range = clas12::ElectronCuts::get_vz_range(m_target);
        return vz_range.contains(vz);
    }

    /**
     * @brief Time-of-flight cut using momentum-dependent parameterization
     *
     * This uses a 3-sigma cut around a momentum-dependent mean with
     * parameterized sigma values from pion calibration studies.
     */
    auto pass_delta_t_cut(const double p, const double delta_t) -> bool {
        // Parameterization from pion timing studies
        const double popt_mean[4] = {-0.02872196, 0.00598219, 0.00158193, -0.00028493};
        const double popt_sigma[4] = {0.25132972, -0.09788015, 0.01734634, -0.00103597};

        double mean = third_order_poly(p, popt_mean[0], popt_mean[1], popt_mean[2], popt_mean[3]);
        double sigma = third_order_poly(p, popt_sigma[0], popt_sigma[1], popt_sigma[2], popt_sigma[3]);

        double cut_low = mean - 3.0 * sigma;
        double cut_high = mean + 3.0 * sigma;

        return cut_low < delta_t && delta_t < cut_high;
    }
};

}  // namespace clas12::algorithms
