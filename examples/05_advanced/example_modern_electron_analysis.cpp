/**
 * @file example_modern_electron_analysis.cpp
 * @brief Modern electron analysis example using C++20/23 framework features
 *
 * This example demonstrates the modernized CLAS12 analysis framework with:
 * - Algorithm base class for reduced boilerplate
 * - Range adaptors for composable particle filtering
 * - CutBuilder for fluent, chainable cut definitions
 * - TypedEventStore for type-safe inter-algorithm communication
 * - Property system for declarative configuration
 * - Automatic service injection
 *
 * The analysis chain consists of:
 * 1. ElectronSelectionAlg - Selects trigger electrons using CutBuilder
 * 2. DISKinematicsAlg - Computes DIS kinematics and stores in TypedEventStore
 * 3. ElectronHistogramAlg - Fills histograms for selected electrons
 *
 * @see Requirements: All (1-12)
 */

#include <ranges>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>

// Framework headers
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

// Service headers
#include <clas12/framework/services/HistogramService.hpp>

// CLAS headers
#include <clas12/Detectors.hpp>
#include <clas12/Particle.hpp>
#include <clas12/PIDs.hpp>

// Physics headers
#include <clas12/physics/Kinematics.hpp>

using namespace clas12::framework;

// ============================================================================
// Algorithm 1: Electron Selection using CutBuilder and Range Adaptors
// ============================================================================

/**
 * @brief Modern electron selection algorithm
 *
 * Demonstrates:
 * - Algorithm base class (Requirement 4.1)
 * - Range adaptors for particle filtering (Requirement 3.1, 3.2)
 * - CutBuilder for fluent cut definitions (Requirement 9.1, 9.2)
 * - TypedEventStore for sharing selected electrons (Requirement 8.1, 8.2)
 * - Property system for configuration (Requirement 2.1, 2.2)
 */
class ModernElectronSelectionAlg : public Algorithm {
public:
    ModernElectronSelectionAlg()
        : Algorithm("ModernElectronSelection")
        , m_cutflow("Electron Selection") {
        // Declare properties with validation (Requirement 2.1, 2.3)
        declare_property(m_min_momentum);
        declare_property(m_max_momentum);
        declare_property(m_min_theta);
        declare_property(m_max_theta);
        declare_property(m_vz_min);
        declare_property(m_vz_max);
    }

    /**
     * @brief Initialize algorithm - obtain services, set up cutflow and histograms
     */
    auto on_initialize() -> StatusCode override {
        m_hist_svc = svc<HistogramService>();
        Log::info(name(), "Initializing with momentum range [{}, {}] GeV",
                  m_min_momentum.get(), m_max_momentum.get());
        Log::info(name(), "Theta range: [{}, {}] deg", m_min_theta.get(), m_max_theta.get());
        Log::info(name(), "Vertex Z range: [{}, {}] cm", m_vz_min.get(), m_vz_max.get());

        // Set up cutflow (Requirement 9.3)
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Forward Detector");
        m_cutflow.add_cut("Momentum Cut");
        m_cutflow.add_cut("Theta Cut");
        m_cutflow.add_cut("Vertex Z Cut");

        // Book histograms
        if (m_hist_svc) {
            auto* svc = m_hist_svc;
            m_h_p_before = svc->book_hist1D(
                "/Selection/p_before", "Electron p (before cuts);p [GeV/c];Events",
                120, 0, 12);
            m_h_p_after = svc->book_hist1D(
                "/Selection/p_after", "Electron p (after cuts);p [GeV/c];Events",
                120, 0, 12);
            m_h_theta_before = svc->book_hist1D(
                "/Selection/theta_before", "Electron #theta (before cuts);#theta [deg];Events",
                90, 0, 90);
            m_h_theta_after = svc->book_hist1D(
                "/Selection/theta_after", "Electron #theta (after cuts);#theta [deg];Events",
                90, 0, 90);
            m_h_vz_before = svc->book_hist1D(
                "/Selection/vz_before", "Electron v_{z} (before cuts);v_{z} [cm];Events",
                200, -30, 30);
            m_h_vz_after = svc->book_hist1D(
                "/Selection/vz_after", "Electron v_{z} (after cuts);v_{z} [cm];Events",
                200, -30, 30);
        }

        return StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     *
     * Uses range adaptors and CutBuilder for clean, expressive selection logic.
     */
    auto process(Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        // ====================================================================
        // Step 1: Pull Forward Detector electrons from Event's cached
        // per-PID vector and gate on |status| ∈ [FD, CD).
        // ====================================================================
        std::vector<clas12::Particle> electron_candidates;
        for (const auto& e : event.electrons()) {
            const double abs_status = std::abs(e.status());
            if (clas12::FD <= abs_status && abs_status < clas12::CD) {
                electron_candidates.push_back(e);
            }
        }

        if (electron_candidates.empty()) {
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");
        m_cutflow.pass("Forward Detector");

        // Fill "before cuts" histograms
        for (const auto& e : electron_candidates) {
            if (m_h_p_before) m_h_p_before->fill(e.p());
            if (m_h_theta_before) m_h_theta_before->fill(e.theta());
            if (m_h_vz_before) m_h_vz_before->fill(e.vz());
        }

        // ====================================================================
        // Step 2: Find trigger electron (highest momentum)
        // ====================================================================
        auto trigger_it = std::ranges::max_element(electron_candidates,
            [](const auto& a, const auto& b) { return a.p() < b.p(); });

        if (trigger_it == electron_candidates.end()) {
            return StatusCode::FILTERED();
        }

        const auto& trigger_electron = *trigger_it;
        const double p = trigger_electron.p();
        const double theta = trigger_electron.theta();
        const double vz = trigger_electron.vz();

        // ====================================================================
        // Step 3: Apply cuts using AutoCutflow
        // ====================================================================
        bool pass_momentum = p >= m_min_momentum.get() && p <= m_max_momentum.get();
        bool pass_theta = theta >= m_min_theta.get() && theta <= m_max_theta.get();
        bool pass_vz = vz >= m_vz_min.get() && vz <= m_vz_max.get();

        if (pass_momentum) m_cutflow.pass("Momentum Cut");
        else return StatusCode::FILTERED();
        if (pass_theta) m_cutflow.pass("Theta Cut");
        else return StatusCode::FILTERED();
        if (pass_vz) m_cutflow.pass("Vertex Z Cut");
        else return StatusCode::FILTERED();

        // Fill "after cuts" histograms
        if (m_h_p_after) m_h_p_after->fill(p);
        if (m_h_theta_after) m_h_theta_after->fill(theta);
        if (m_h_vz_after) m_h_vz_after->fill(vz);

        // ====================================================================
        // Step 4: Store selected electron in TypedEventStore
        // (Requirement 8.1, 8.2, 8.4 - type-safe inter-algorithm communication)
        // ====================================================================
        m_event_store.clear();  // Clear previous event data
        m_event_store.store(Keys::TriggerElectron, trigger_electron);
        m_event_store.store(Decisions::HasGoodElectron, true);

        // Also store all passing electrons
        std::vector<clas12::Particle> selected_electrons;
        for (const auto& e : electron_candidates) {
            if (e.p() >= m_min_momentum.get() && e.p() <= m_max_momentum.get() &&
                e.theta() >= m_min_theta.get() && e.theta() <= m_max_theta.get() &&
                e.vz() >= m_vz_min.get() && e.vz() <= m_vz_max.get()) {
                selected_electrons.push_back(e);
            }
        }
        m_event_store.store(Keys::Electrons, selected_electrons);

        // Store in event for downstream algorithms
        event.store_typed(TypedKey<TypedEventStore>("electron_store"), m_event_store);

        m_selected_events++;
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Finalize - print cutflow summary
     */
    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Selected {} events with good electrons", m_selected_events);
        m_cutflow.print();
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Get the event store for downstream algorithms
     */
    auto get_event_store() -> TypedEventStore& { return m_event_store; }

private:
    // Properties with validation (Requirement 2.1, 2.3)
    Property<double> m_min_momentum{"min_momentum", 1.0, "Minimum electron momentum [GeV]"};
    Property<double> m_max_momentum{"max_momentum", 10.5, "Maximum electron momentum [GeV]"};
    Property<double> m_min_theta{"min_theta", 5.0, "Minimum theta angle [deg]"};
    Property<double> m_max_theta{"max_theta", 35.0, "Maximum theta angle [deg]"};
    Property<double> m_vz_min{"vz_min", -12.0, "Minimum vertex z [cm]"};
    Property<double> m_vz_max{"vz_max", 10.0, "Maximum vertex z [cm]"};

    // Service pointer
    HistogramService* m_hist_svc = nullptr;

    // Cutflow for tracking selection efficiency
    Cutflow m_cutflow;

    // Type-safe event store (Requirement 8.1, 8.2)
    TypedEventStore m_event_store;

    // Histograms (atomic for thread-safety)
    ThreadLocalHistogram1D* m_h_p_before = nullptr;
    ThreadLocalHistogram1D* m_h_p_after = nullptr;
    ThreadLocalHistogram1D* m_h_theta_before = nullptr;
    ThreadLocalHistogram1D* m_h_theta_after = nullptr;
    ThreadLocalHistogram1D* m_h_vz_before = nullptr;
    ThreadLocalHistogram1D* m_h_vz_after = nullptr;

    // Counters
    int m_selected_events = 0;
};

// ============================================================================
// Algorithm 2: DIS Kinematics Calculator
// ============================================================================

/**
 * @brief Computes DIS kinematics from selected electrons
 *
 * Demonstrates:
 * - TypedEventStore retrieval (Requirement 8.2, 8.3)
 * - Property system for beam energy configuration
 * - Storing computed quantities for downstream use
 */
class DISKinematicsAlg : public Algorithm {
public:
    DISKinematicsAlg() : Algorithm("DISKinematics") {
        declare_property(m_beam_energy);
        declare_property(m_target_mass);
    }

    auto on_initialize() -> StatusCode override {
        m_hist_svc = svc<HistogramService>();
        Log::info(name(), "Beam energy: {} GeV", m_beam_energy.get());
        Log::info(name(), "Target mass: {} GeV", m_target_mass.get());

        if (m_hist_svc) {
            auto* svc = m_hist_svc;
            m_h_Q2 = svc->book_hist1D(
                "/Kinematics/Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
            m_h_W = svc->book_hist1D(
                "/Kinematics/W", "W;W [GeV];Events", 100, 0, 5);
            m_h_xbj = svc->book_hist1D(
                "/Kinematics/xbj", "x_{Bj};x_{Bj};Events", 100, 0, 1);
            m_h_y = svc->book_hist1D(
                "/Kinematics/y", "y;y;Events", 100, 0, 1);
            m_h_Q2_vs_xbj = svc->book_hist2D(
                "/Kinematics/Q2_vs_xbj", "Q^{2} vs x_{Bj};x_{Bj};Q^{2} [GeV^{2}]",
                100, 0, 1, 100, 0, 10);
            m_h_W_vs_Q2 = svc->book_hist2D(
                "/Kinematics/W_vs_Q2", "W vs Q^{2};Q^{2} [GeV^{2}];W [GeV]",
                100, 0, 10, 100, 0, 5);
        }

        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        // ====================================================================
        // Retrieve electron from TypedEventStore (Requirement 8.2, 8.3)
        // ====================================================================
        auto store_opt = event.retrieve_typed(TypedKey<TypedEventStore>("electron_store"));
        if (!store_opt) {
            return StatusCode::FILTERED();
        }

        auto& store = *store_opt;

        // Check if we have a good electron
        auto has_electron = store.retrieve(Decisions::HasGoodElectron);
        if (!has_electron || !*has_electron) {
            return StatusCode::FILTERED();
        }

        // Get the trigger electron
        auto electron_opt = store.retrieve(Keys::TriggerElectron);
        if (!electron_opt) {
            return StatusCode::FILTERED();
        }

        const auto& electron = *electron_opt;

        // ====================================================================
        // Compute DIS kinematics using KinematicCalculator
        // ====================================================================
        const double E_beam = m_beam_energy.get();

        // Create beam particle (electron along z-axis)
        clas12::Particle beam(11, 0, -1, -1, clas12::Constants::ELECTRONMASS,
                            0.0, 0.0, E_beam, E_beam, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

        // Create target particle (proton at rest)
        clas12::Particle target(2212, 0, -1, 1, clas12::Constants::PROTONMASS,
                              0.0, 0.0, 0.0, clas12::Constants::PROTONMASS, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

        // Compute DIS kinematics
        auto dis_kin = clas12::physics::KinematicCalculator::calculate_DIS(beam, electron, target);
        double Q2 = dis_kin.Q2;
        double W = dis_kin.W;
        double xbj = dis_kin.x_bj;
        double y = dis_kin.y;
        double nu = dis_kin.nu;

        // Fill histograms
        if (m_h_Q2) m_h_Q2->fill(Q2);
        if (m_h_W) m_h_W->fill(W);
        if (m_h_xbj) m_h_xbj->fill(xbj);
        if (m_h_y) m_h_y->fill(y);
        if (m_h_Q2_vs_xbj) m_h_Q2_vs_xbj->fill(xbj, Q2);
        if (m_h_W_vs_Q2) m_h_W_vs_Q2->fill(Q2, W);

        // ====================================================================
        // Store computed kinematics in TypedEventStore for downstream use
        // ====================================================================
        TypedEventStore kinematics_store;
        kinematics_store.store(Keys::Q2, Q2);
        kinematics_store.store(Keys::W, W);
        kinematics_store.store(Keys::Xbj, xbj);
        kinematics_store.store(Keys::Y, y);
        kinematics_store.store(Keys::Nu, nu);

        // Apply DIS cuts
        bool is_dis = (Q2 > 1.0) && (W > 2.0) && (y < 0.85);
        kinematics_store.store(Decisions::IsDIS, is_dis);

        event.store_typed(TypedKey<TypedEventStore>("kinematics_store"), kinematics_store);

        if (is_dis) {
            m_dis_events++;
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "DIS events (Q2>1, W>2, y<0.85): {}", m_dis_events);
        return StatusCode::SUCCESS();
    }

private:
    Property<double> m_beam_energy{"beam_energy", 10.6, "Beam energy [GeV]"};
    Property<double> m_target_mass{"target_mass", 0.938272, "Target mass [GeV]"};

    HistogramService* m_hist_svc = nullptr;

    ThreadLocalHistogram1D* m_h_Q2 = nullptr;
    ThreadLocalHistogram1D* m_h_W = nullptr;
    ThreadLocalHistogram1D* m_h_xbj = nullptr;
    ThreadLocalHistogram1D* m_h_y = nullptr;
    ThreadLocalHistogram2D* m_h_Q2_vs_xbj = nullptr;
    ThreadLocalHistogram2D* m_h_W_vs_Q2 = nullptr;

    int m_dis_events = 0;
};

// ============================================================================
// Algorithm 3: Electron Histogram Filler (using LambdaAlgorithm)
// ============================================================================

/**
 * @brief Creates a lambda-based algorithm for additional histogram filling
 *
 * Demonstrates LambdaAlgorithm for simple, inline algorithm definitions
 * (Requirement 4.3)
 */
class ElectronHistogramAlg : public Algorithm {
public:
    ElectronHistogramAlg() : Algorithm("ElectronHistograms") {}

    auto on_initialize() -> StatusCode override {
        m_hist_svc = svc<HistogramService>();
        if (m_hist_svc) {
            m_h_p_theta_ratio = m_hist_svc->book_hist1D(
                "/Analysis/p_theta_ratio", "p/#theta;p/#theta [GeV/deg];Events",
                100, 0, 1);
            m_h_phi_vs_sector = m_hist_svc->book_hist2D(
                "/Analysis/phi_vs_sector", "#phi vs Sector;Sector;#phi [deg]",
                6, 0.5, 6.5, 72, -180, 180);
        }
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        auto store_opt = event.retrieve_typed(TypedKey<TypedEventStore>("electron_store"));
        if (!store_opt) {
            return StatusCode::FILTERED();
        }

        auto& store = *store_opt;
        auto electrons_opt = store.retrieve(Keys::Electrons);
        if (!electrons_opt || electrons_opt->empty()) {
            return StatusCode::FILTERED();
        }

        for (const auto& e : *electrons_opt) {
            double ratio = e.p() / e.theta();
            if (m_h_p_theta_ratio) m_h_p_theta_ratio->fill(ratio);

            // Compute sector from phi
            double phi = e.phi();
            int sector = static_cast<int>((phi + 180.0) / 60.0) + 1;
            if (sector > 6) sector = 6;
            if (sector < 1) sector = 1;

            if (m_h_phi_vs_sector) m_h_phi_vs_sector->fill(sector, phi);
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Additional histograms filled");
        return StatusCode::SUCCESS();
    }

private:
    HistogramService* m_hist_svc = nullptr;
    ThreadLocalHistogram1D* m_h_p_theta_ratio = nullptr;
    ThreadLocalHistogram2D* m_h_phi_vs_sector = nullptr;
};

// ============================================================================
// Main — uses AnalysisRunner so the binary takes a single --config flag.
// ============================================================================
//
// Run with:
//   ./example_modern_electron_analysis --config config/example_modern_electron.toml
//
// The TOML drives input/output/threads as well as every algorithm
// `Property<T>` (e.g. ModernElectronSelection.min_momentum,
// DISKinematics.beam_energy, ...). See README for the schema.

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Modern Electron Analysis", "modern_electron_analysis",
        [](auto& builder, const auto& /*config*/) {
            builder
                .template add_algorithm<ModernElectronSelectionAlg>()
                .template add_algorithm<DISKinematicsAlg>()
                .template add_algorithm<ElectronHistogramAlg>();
        });
}
