/**
 * @file main_advanced.cpp
 * @brief Advanced framework demo showing different algorithm types
 *
 * This demonstrates the enhanced framework with:
 * - FilterAlgorithm: Event selection with automatic decisions
 * - ProducerAlgorithm: Creating composite particles
 * - AnalysisAlgorithm: Filling histograms
 * - MonitoringAlgorithm: Performance tracking
 * - AlgorithmSequence: Grouping related algorithms
 * - ConditionalSequence: Conditional execution based on filters
 */

#include "Framework/Application.hpp"
#include "Framework/AlgorithmTypes.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/OutputFileService.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"
#include "Algorithms/TriggerElectronFilter.hpp"
#include "Algorithms/Pi0Producer.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "Algorithms/ElectronAnalysisAlg.hpp"
#include "Algorithms/PionSelectionAlg.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/Target.hpp"
#include <fmt/format.h>
#include <string>

int main(int argc, char** argv) {
    // Parse command line arguments
    std::string data_dir = (argc > 1) ? argv[1] : "data/";
    long max_events = (argc > 2) ? std::stol(argv[2]) : -1;
    int num_threads = (argc > 3) ? std::stoi(argv[3]) : 4;
    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;

    fmt::print("\n");
    fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
    fmt::print("║        CLAS12 Advanced Framework - Algorithm Types Demo       ║\n");
    fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
    fmt::print("\n");

    // Create application
    Framework::Application app("CLAS12 Advanced Analysis");

    // ═══════════════════════════════════════════════════════════════════
    // Configure Services
    // ═══════════════════════════════════════════════════════════════════

    fmt::print("Configuring Services:\n");

    auto output_svc = std::make_unique<Framework::OutputFileService>();
    output_svc->set_output_file("advanced_analysis.root");
    Framework::OutputFileService* output_svc_ptr = output_svc.get();
    fmt::print("  ✓ OutputFileService → advanced_analysis.root\n");

    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file_service(output_svc_ptr);
    fmt::print("  ✓ HistogramService\n");

    auto cond_svc = std::make_unique<Framework::ConditionsService>();
    cond_svc->set_run_number(5032);
    fmt::print("  ✓ ConditionsService → Run 5032\n");

    auto particle_svc = std::make_unique<Framework::ParticleSelectorService>();
    fmt::print("  ✓ ParticleSelectorService\n");

    auto sf_svc = std::make_unique<Framework::SamplingFractionService>();
    fmt::print("  ✓ SamplingFractionService\n");

    app.service_manager().add_service(std::move(output_svc));
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(cond_svc));
    app.service_manager().add_service(std::move(particle_svc));
    app.service_manager().add_service(std::move(sf_svc));

    // ═══════════════════════════════════════════════════════════════════
    // Configure Algorithm Workflow
    // ═══════════════════════════════════════════════════════════════════

    fmt::print("\nConfiguring Algorithm Workflow:\n");
    fmt::print("\n[1] Global Monitoring (runs on all events)\n");

    // 1. Event Monitor - runs on all events
    auto event_monitor = std::make_unique<Algorithms::EventMonitor>("EventMonitor");
    fmt::print("  ✓ EventMonitor (MonitoringAlgorithm)\n");
    app.add_algorithm(std::move(event_monitor));

    // 2. Trigger Filter - selects events with good trigger electron
    fmt::print("\n[2] Event Selection Filter\n");
    auto trigger_filter = std::make_unique<Algorithms::TriggerElectronFilter>(
        "TriggerElectronFilter", target, polarity);
    fmt::print("  ✓ TriggerElectronFilter (FilterAlgorithm)\n");
    fmt::print("    → Sets decision: 'TriggerElectronFilter' (true/false)\n");
    app.add_algorithm(std::move(trigger_filter));

    // 3. Create a conditional sequence for electron-tagged events
    fmt::print("\n[3] Electron-Tagged Event Sequence (conditional)\n");
    auto electron_seq = std::make_unique<Framework::ConditionalSequence>(
        "ElectronSequence",
        "TriggerElectronFilter"  // Only run if this decision is true
    );

    // 3a. Electron Analysis (using config struct)
    Algorithms::ElectronAnalysisConfig electron_config;
    electron_config.target = target;
    electron_config.polarity = polarity;
    auto electron_analysis = std::make_unique<Algorithms::ElectronAnalysisAlg>(
        "ElectronAnalysis", electron_config);
    fmt::print("  ✓ ElectronAnalysis (AnalysisAlgorithm)\n");
    electron_seq->add_algorithm(std::move(electron_analysis));

    // 3b. Pion Selection (within electron-tagged events)
    auto pion_selection = std::make_unique<Algorithms::PionSelectionAlg>(
        "PionSelection", target, true);
    fmt::print("  ✓ PionSelection (AnalysisAlgorithm)\n");
    electron_seq->add_algorithm(std::move(pion_selection));

    fmt::print("  → Sequence runs only if 'TriggerElectronFilter' = true\n");

    app.add_algorithm(std::move(electron_seq));

    // 4. Create a separate sequence for neutral particle reconstruction
    fmt::print("\n[4] Neutral Particle Production (runs on all events)\n");
    auto pi0_producer = std::make_unique<Algorithms::Pi0Producer>(
        "Pi0Producer", 0.135, 0.050);
    fmt::print("  ✓ Pi0Producer (ProducerAlgorithm)\n");
    fmt::print("    → Produces: 'Pi0Candidates' in event store\n");
    fmt::print("    → Sets decision: 'HasPi0' if π⁰ found\n");
    app.add_algorithm(std::move(pi0_producer));

    // ═══════════════════════════════════════════════════════════════════
    // Summary of Algorithm Flow
    // ═══════════════════════════════════════════════════════════════════

    fmt::print("\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Algorithm Execution Flow\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("\n");
    fmt::print("For each event:\n");
    fmt::print("  1. EventMonitor → Track statistics (all events)\n");
    fmt::print("  2. TriggerElectronFilter → Select events (sets decision)\n");
    fmt::print("  3. IF TriggerElectronFilter = true:\n");
    fmt::print("     a. ElectronAnalysis → Fill electron histograms\n");
    fmt::print("     b. PionSelection → Fill pion histograms\n");
    fmt::print("  4. Pi0Producer → Reconstruct π⁰ (all events)\n");
    fmt::print("\n");

    // ═══════════════════════════════════════════════════════════════════
    // Configure Input
    // ═══════════════════════════════════════════════════════════════════

    fmt::print("Configuring Input:\n");

    std::vector<std::string> input_files = CLAS::FileUtils::read_recursive_hipo_files(data_dir);
    if (input_files.empty()) {
        Framework::Log::error("Main", "No .hipo files found in {}", data_dir);
        fmt::print("\nUsage: {} [data_dir] [max_events] [num_threads]\n", argv[0]);
        return 1;
    }

    fmt::print("  Data directory: {}\n", data_dir);
    fmt::print("  Input files: {}\n", input_files.size());
    fmt::print("  Max events: {}\n", max_events == -1 ? "all" : std::to_string(max_events));
    fmt::print("  Threads: {}\n", num_threads);

    app.set_input_files(input_files);
    // app.set_max_events(max_events);
    app.set_num_threads(num_threads);

    // ═══════════════════════════════════════════════════════════════════
    // Initialize
    // ═══════════════════════════════════════════════════════════════════

    auto sc = app.initialize();
    if (sc.is_failure()) {
        Framework::Log::error("Main", "Initialization failed");
        return 1;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Run Event Loop
    // ═══════════════════════════════════════════════════════════════════

    fmt::print("\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Starting Event Processing\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("\n");

    auto start_time = std::chrono::high_resolution_clock::now();

    sc = app.run();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (sc.is_failure()) {
        Framework::Log::error("Main", "Event processing failed");
        return 1;
    }

    // ═══════════════════════════════════════════════════════════════════
    // Finalize
    // ═══════════════════════════════════════════════════════════════════

    sc = app.finalize();
    if (sc.is_failure()) {
        Framework::Log::warning("Main", "Finalization had issues");
    }

    // ═══════════════════════════════════════════════════════════════════
    // Print Summary
    // ═══════════════════════════════════════════════════════════════════

    fmt::print("\n");
    fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
    fmt::print("║                 Advanced Analysis Complete!                    ║\n");
    fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
    fmt::print("\n");
    fmt::print("Performance:\n");
    fmt::print("  Processing time: {:.2f} seconds\n", duration.count() / 1000.0);
    fmt::print("\n");
    fmt::print("Output:\n");
    fmt::print("  ROOT file: advanced_analysis.root\n");
    fmt::print("    /Monitor/     - Event monitoring histograms\n");
    fmt::print("    /Electron/    - Electron analysis histograms\n");
    fmt::print("    /Pion/        - Pion selection histograms\n");
    fmt::print("\n");
    fmt::print("Framework Features Demonstrated:\n");
    fmt::print("  ✓ FilterAlgorithm      - Event selection with decisions\n");
    fmt::print("  ✓ ProducerAlgorithm    - Composite particle creation\n");
    fmt::print("  ✓ AnalysisAlgorithm    - Histogram filling\n");
    fmt::print("  ✓ MonitoringAlgorithm  - Performance tracking\n");
    fmt::print("  ✓ ConditionalSequence  - Conditional algorithm groups\n");
    fmt::print("  ✓ Event Store          - Inter-algorithm communication\n");
    fmt::print("\n");

    return 0;
}
