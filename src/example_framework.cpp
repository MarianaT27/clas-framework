/**
 * @file example_framework.cpp
 * @brief Example analysis using the new CLAS12 framework
 *
 * This demonstrates how to use the improved framework with:
 * - Service-oriented architecture
 * - Algorithm sequence
 * - Cutflow tracking
 * - Histogram management
 */

#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Core/Helpers.hpp"
#include <fmt/format.h>

int main() {
    // Create application
    Framework::Application app("CLAS12 Example Analysis");

    // Configure services
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("framework_example.root");

    auto cond_svc = std::make_unique<Framework::ConditionsService>();
    cond_svc->set_run_number(5032);

    // Add services
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(cond_svc));

    // Add algorithms
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection",
        Core::Target::LD2,
        Core::Polarity::Outbending
    );
    app.add_algorithm(std::move(electron_alg));

    // Configure input
    std::vector<std::string> input_files = Core::read_recursive_file_in_directory("data/");
    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No input files found in data/\n");
        return 1;
    }
    app.set_input_files(input_files);
    app.set_max_events(10000);  // Process 10k events for testing
    app.set_num_threads(4);

    // Run the analysis
    auto sc = app.initialize();
    if (sc.is_failure()) {
        fmt::print(stderr, "ERROR: Initialization failed\n");
        return 1;
    }

    sc = app.run();
    if (sc.is_failure()) {
        fmt::print(stderr, "ERROR: Event processing failed\n");
        return 1;
    }

    sc = app.finalize();
    if (sc.is_failure()) {
        fmt::print(stderr, "WARNING: Finalization had issues\n");
    }

    fmt::print("\n✓ Analysis complete!\n");
    fmt::print("  Output: framework_example.root\n\n");

    return 0;
}
