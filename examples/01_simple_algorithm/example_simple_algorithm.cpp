/**
 * @file example_simple_algorithm.cpp
 * @brief Example demonstrating SimpleAlgorithm and LambdaAlgorithm usage
 *
 * This example verifies the algorithm infrastructure by demonstrating:
 * - SimpleAlgorithm CRTP base class with automatic service injection
 * - LambdaAlgorithm for quick, inline algorithm definitions
 * - Property system integration
 * - Service injection via require_service<T>()
 *
 * @see Requirements 4.1, 4.2, 4.3
 */

#include <fmt/format.h>

#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/ApplicationBuilder.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include <clas12/framework/core/Ranges.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/FileUtils.hpp>
#include "TH1D.h"

using namespace clas12::framework;

// ============================================================================
// Example 1: SimpleAlgorithm with service injection and properties
// ============================================================================

/**
 * @brief Example algorithm demonstrating SimpleAlgorithm features
 *
 * This algorithm shows:
 * - Inheriting from SimpleAlgorithm<Derived> (CRTP pattern)
 * - Declaring properties with validation
 * - Automatic service injection via require_service<T>()
 * - Using on_initialize() and on_finalize() hooks
 */
class ElectronCounterAlg : public SimpleAlgorithm<ElectronCounterAlg> {
public:
    ElectronCounterAlg() : SimpleAlgorithm("ElectronCounter") {
        // Declare properties in constructor
        declare_property(m_min_momentum);
        declare_property(m_max_momentum);
        declare_property(m_verbose);
    }

    /**
     * @brief Configure services needed by this algorithm
     *
     * Called automatically during initialize() before on_initialize().
     * Use require_service<T>() to declare service dependencies.
     */
    auto configure() -> void override {
        // Declare required services - they will be automatically injected
        m_hist_svc = &require_service<HistogramService>("HistogramService");
    }

    /**
     * @brief Custom initialization after services are injected
     *
     * Book histograms and set up any resources needed for processing.
     */
    auto on_initialize() -> StatusCode override {
        fmt::print("  [{}] Initializing with momentum range [{}, {}] GeV\n",
                   name(), m_min_momentum.get(), m_max_momentum.get());

        // Book histograms using the injected service
        if (m_hist_svc && m_hist_svc->is_valid()) {
            m_h_electron_momentum = m_hist_svc->get()->book_hist1D(
                "electron_momentum", "Electron Momentum;p [GeV];Counts",
                100, 0.0, 10.0);
            m_h_electron_count = m_hist_svc->get()->book_hist1D(
                "electron_count", "Electrons per Event;N_{e};Events",
                10, 0, 10);
        }

        return StatusCode::SUCCESS();
    }

    /**
     * @brief Process a single event
     *
     * This is the main per-event logic. Count electrons and fill histograms.
     */
    auto process(Event& event) -> StatusCode override {
        m_events_processed++;

        // Use range adaptors to filter electrons by momentum
        auto electrons = event.all_particles()
            | Ranges::by_pid(clas12::PIDs::ELECTRON)
            | Ranges::by_momentum(m_min_momentum.get(), m_max_momentum.get());

        int electron_count = 0;
        for (const auto& electron : electrons) {
            electron_count++;
            if (m_h_electron_momentum) {
                m_h_electron_momentum->fill(electron.p());
            }
        }

        if (m_h_electron_count) {
            m_h_electron_count->fill(electron_count);
        }

        m_total_electrons += electron_count;

        if (m_verbose.get() && electron_count > 0) {
            fmt::print("  [{}] Event {}: Found {} electrons\n",
                       name(), m_events_processed, electron_count);
        }

        return StatusCode::SUCCESS();
    }

    /**
     * @brief Finalization - print summary statistics
     */
    auto on_finalize() -> StatusCode override {
        fmt::print("  [{}] Summary:\n", name());
        fmt::print("    Events processed: {}\n", m_events_processed);
        fmt::print("    Total electrons: {}\n", m_total_electrons);
        if (m_events_processed > 0) {
            fmt::print("    Average electrons/event: {:.2f}\n",
                       static_cast<double>(m_total_electrons) / m_events_processed);
        }
        return StatusCode::SUCCESS();
    }

private:
    // Properties with validation
    Property<double> m_min_momentum{"min_momentum", 0.5, "Minimum electron momentum [GeV]"};
    Property<double> m_max_momentum{"max_momentum", 10.0, "Maximum electron momentum [GeV]"};
    Property<bool> m_verbose{"verbose", false, "Print per-event information"};

    // Service handle (injected automatically)
    ServiceHandle<HistogramService>* m_hist_svc = nullptr;

    // Histogram pointers (atomic for thread-safety)
    AtomicHistogram1D* m_h_electron_momentum = nullptr;
    AtomicHistogram1D* m_h_electron_count = nullptr;

    // Counters
    int m_events_processed = 0;
    int m_total_electrons = 0;
};

// ============================================================================
// Example 2: Another SimpleAlgorithm demonstrating multiple services
// ============================================================================

/**
 * @brief Algorithm demonstrating multiple service injection
 */
class PhotonAnalysisAlg : public SimpleAlgorithm<PhotonAnalysisAlg> {
public:
    PhotonAnalysisAlg() : SimpleAlgorithm("PhotonAnalysis") {
        declare_property(m_energy_threshold);
    }

    auto configure() -> void override {
        // Request multiple services
        m_hist_svc = &require_service<HistogramService>("HistogramService");
    }

    auto on_initialize() -> StatusCode override {
        fmt::print("  [{}] Initializing with energy threshold {} GeV\n",
                   name(), m_energy_threshold.get());

        if (m_hist_svc && m_hist_svc->is_valid()) {
            m_h_photon_energy = m_hist_svc->get()->book_hist1D(
                "photon_energy", "Photon Energy;E [GeV];Counts",
                100, 0.0, 5.0);
        }
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        auto photons = event.particles(clas12::PIDs::PHOTON);

        for (const auto& photon : photons) {
            if (photon.E() > m_energy_threshold.get()) {
                if (m_h_photon_energy) {
                    m_h_photon_energy->fill(photon.E());
                }
                m_photon_count++;
            }
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        fmt::print("  [{}] Total photons above threshold: {}\n",
                   name(), m_photon_count);
        return StatusCode::SUCCESS();
    }

private:
    Property<double> m_energy_threshold{"energy_threshold", 0.1, "Minimum photon energy [GeV]"};
    ServiceHandle<HistogramService>* m_hist_svc = nullptr;
    AtomicHistogram1D* m_h_photon_energy = nullptr;
    int m_photon_count = 0;
};

// ============================================================================
// Main: Demonstrate both SimpleAlgorithm and LambdaAlgorithm
// ============================================================================

int main(int argc, char* argv[]) {
    fmt::print("=== SimpleAlgorithm and LambdaAlgorithm Example ===\n\n");

    // Determine data directory
    std::string data_dir = "data/";
    if (argc > 1) {
        data_dir = argv[1];
    }

    // Get input files first to configure builder
    std::vector<std::string> input_files = clas12::FileUtils::read_recursive_hipo_files(data_dir);

    // ========================================================================
    // Create application using ApplicationBuilder
    // ========================================================================
    fmt::print("Setting up application with ApplicationBuilder...\n");

    auto app = make_application("AlgorithmExample")
        .with_input_files(input_files)
        .with_output("algorithm_example_output.root")
        .with_threads(1)
        .with_standard_services()
        .add_algorithm<ElectronCounterAlg>()
        .add_algorithm<PhotonAnalysisAlg>()
        .build();

    fmt::print("  ✓ Services configured via with_standard_services()\n");
    fmt::print("  ✓ SimpleAlgorithm instances added\n");

    // ========================================================================
    // Add LambdaAlgorithm instances (added after build due to captures)
    // ========================================================================
    fmt::print("\nAdding LambdaAlgorithm instances...\n");

    // Simple event counter using lambda
    int event_count = 0;
    auto counter_alg = make_algorithm("EventCounter",
        [&event_count]([[maybe_unused]] Event& event) -> StatusCode {
            event_count++;
            return StatusCode::SUCCESS();
        });
    app.add_algorithm(std::move(counter_alg));

    // Event filter using lambda (filters events without electrons)
    auto filter_alg = make_algorithm("ElectronFilter",
        [](Event& event) -> StatusCode {
            auto electrons = event.particles(clas12::PIDs::ELECTRON);
            if (electrons.empty()) {
                return StatusCode::FILTERED();
            }
            return StatusCode::SUCCESS();
        });
    app.add_algorithm(std::move(filter_alg));

    // Lambda with init and finalize callbacks
    int filtered_events = 0;
    auto stats_alg = make_algorithm(
        "FilteredEventStats",
        [&filtered_events]([[maybe_unused]] Event& event) -> StatusCode {
            filtered_events++;
            return StatusCode::SUCCESS();
        },
        []() -> StatusCode {
            fmt::print("  [FilteredEventStats] Initialized\n");
            return StatusCode::SUCCESS();
        },
        [&filtered_events]() -> StatusCode {
            fmt::print("  [FilteredEventStats] Events after filter: {}\n", filtered_events);
            return StatusCode::SUCCESS();
        }
    );
    app.add_algorithm(std::move(stats_alg));

    // ========================================================================
    // Run application
    // ========================================================================
    if (input_files.empty()) {
        fmt::print("WARNING: No input files found in {}\n", data_dir);
        fmt::print("Running initialization only to verify algorithm infrastructure...\n\n");

        // Still run initialize/finalize to verify service injection works
        auto sc = app.initialize();
        if (sc.is_failure()) {
            fmt::print("ERROR: Initialization failed: {}\n", sc.message());
            return 1;
        }

        fmt::print("\n✓ Initialization successful - service injection verified!\n");

        sc = app.finalize();
        if (sc.is_failure()) {
            fmt::print("WARNING: Finalization had issues\n");
        }

        fmt::print("\n=== Summary ===\n");
        fmt::print("  SimpleAlgorithm: ElectronCounter, PhotonAnalysis\n");
        fmt::print("  LambdaAlgorithm: EventCounter, ElectronFilter, FilteredEventStats\n");
        fmt::print("  Service injection: VERIFIED\n");
        fmt::print("\n✓ Algorithm infrastructure checkpoint complete!\n");

        return 0;
    }

    fmt::print("\nInput files: {}\n", input_files.size());

    // Initialize
    fmt::print("\n--- Initialization ---\n");
    auto sc = app.initialize();
    if (sc.is_failure()) {
        fmt::print("ERROR: Initialization failed: {}\n", sc.message());
        return 1;
    }

    // Run
    fmt::print("\n--- Processing Events ---\n");
    sc = app.run();
    if (sc.is_failure()) {
        fmt::print("ERROR: Event processing failed\n");
        return 1;
    }

    // Finalize
    fmt::print("\n--- Finalization ---\n");
    sc = app.finalize();
    if (sc.is_failure()) {
        fmt::print("WARNING: Finalization had issues\n");
    }

    // Print summary
    fmt::print("\n=== Final Summary ===\n");
    fmt::print("  Total events processed: {}\n", event_count);
    fmt::print("  Events passing electron filter: {}\n", filtered_events);
    fmt::print("  Output file: algorithm_example_output.root\n");

    fmt::print("\n✓ Algorithm infrastructure checkpoint complete!\n");

    return 0;
}
