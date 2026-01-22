/**
 * @file example_builder.cpp
 * @brief Example demonstrating the ApplicationBuilder pattern
 *
 * Shows how to use the fluent builder API for cleaner application setup.
 * Compare this to the verbose setup in other examples.
 */

#include <cstdlib>

#include <TROOT.h>
#include <fmt/format.h>

#include "Algorithms/ElectronAnalysisAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/ApplicationBuilder.hpp"

auto main(int argc, char** argv) -> int {
    ROOT::EnableThreadSafety();

    // Parse simple arguments
    std::string data_dir = (argc > 1) ? argv[1] : "data/";
    int num_threads = (argc > 2) ? std::stoi(argv[2]) : 4;

    // Find input files
    auto input_files = CLAS::FileUtils::read_recursive_hipo_files(data_dir);
    if (input_files.empty()) {
        Framework::Log::error("Main", "No HIPO files found in {}", data_dir);
        return EXIT_FAILURE;
    }

    // Configure algorithm
    Algorithms::ElectronAnalysisConfig alg_config;
    alg_config.target = CLAS::Target::LD2;
    alg_config.polarity = CLAS::Polarity::Outbending;

    // Build application using fluent API
    auto app = Framework::make_application("Builder Example")
        .with_input_files(input_files)
        .with_output("builder_example.root")
        .with_threads(num_threads)
        .with_standard_services()
        .with_target("LD2")
        .with_beam_energy(10.53)
        .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<Algorithms::ElectronAnalysisAlg>("ElectronAnalysis", alg_config)
        .build();

    // Run analysis
    if (auto sc = app.initialize(); sc.is_failure()) {
        Framework::Log::error("Main", "Initialization failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    if (auto sc = app.run(); sc.is_failure()) {
        Framework::Log::error("Main", "Processing failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    app.finalize();

    fmt::print("Analysis complete!\n");
    return EXIT_SUCCESS;
}
