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

#include <clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/FileUtils.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/ApplicationBuilder.hpp>

auto main(int argc, char** argv) -> int {
    ROOT::EnableThreadSafety();

    // Parse simple arguments
    std::string data_dir = (argc > 1) ? argv[1] : "data/";
    int num_threads = (argc > 2) ? std::stoi(argv[2]) : 4;

    // Find input files
    auto input_files = FileUtils::read_recursive_hipo_files(data_dir);
    if (input_files.empty()) {
        framework::Log::error("Main", "No HIPO files found in {}", data_dir);
        return EXIT_FAILURE;
    }

    // Configure algorithm
    algorithms::ElectronAnalysisConfig alg_config;
    alg_config.target = Target::LD2;
    alg_config.polarity = Polarity::Outbending;

    // Build application using fluent API
    auto app = framework::make_application("Builder Example")
        .with_input_files(input_files)
        .with_output("builder_example.root")
        .with_threads(num_threads)
        .with_standard_services()
        .with_target("LD2")
        .with_beam_energy(10.53)
        .add_algorithm<algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<algorithms::ElectronAnalysisAlg>("ElectronAnalysis", alg_config)
        .build();

    // Run analysis
    if (auto sc = app.initialize(); sc.is_failure()) {
        framework::Log::error("Main", "Initialization failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    if (auto sc = app.run(); sc.is_failure()) {
        framework::Log::error("Main", "Processing failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    app.finalize();

    fmt::print("Analysis complete!\n");
    return EXIT_SUCCESS;
}
