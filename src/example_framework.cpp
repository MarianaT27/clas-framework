/**
 * @file example_framework.cpp
 * @brief Example analysis using the new CLAS12 framework
 *
 * This demonstrates how to use the improved framework with:
 * - Configuration loaded from TOML file via ConfigService
 * - Service-oriented architecture
 * - Algorithm sequence
 * - Cutflow tracking
 * - Histogram management
 */

#include "Framework/Application.hpp"
#include "Framework/Services/ConfigService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Core/Helpers.hpp"
#include <fmt/format.h>

int main(int argc, char* argv[]) {
    // Determine config file path (default or from command line)
    std::string config_path = "config/example_framework.toml";
    if (argc > 1) {
        config_path = argv[1];
    }

    // Create and initialize ConfigService first to load configuration
    auto config_svc = std::make_unique<Framework::ConfigService>(config_path);
    auto config_sc = config_svc->initialize();
    if (config_sc.is_failure()) {
        fmt::print(stderr, "ERROR: Failed to load configuration: {}\n", config_sc.message());
        return 1;
    }

    // Validate required configuration keys
    auto validate_result = config_svc->validate({
        "application.name",
        "input.data_directory",
        "output.histogram_file"
    });
    if (validate_result.is_error()) {
        fmt::print(stderr, "ERROR: Configuration validation failed: {}\n", 
                   validate_result.error_message());
        return 1;
    }

    // Read configuration values
    std::string app_name = config_svc->get<std::string>("application.name", "CLAS12 Analysis");
    long max_events = config_svc->get<int64_t>("application.max_events", -1);
    int num_threads = config_svc->get<int64_t>("application.num_threads", 1);
    
    std::string data_dir = config_svc->get<std::string>("input.data_directory", "data/");
    std::string hist_file = config_svc->get<std::string>("output.histogram_file", "output.root");
    
    int run_number = config_svc->get<int64_t>("conditions.run_number", 5032);
    
    // Parse target and polarity from config
    std::string target_str = config_svc->get<std::string>("electron_selection.target", "LD2");
    std::string polarity_str = config_svc->get<std::string>("electron_selection.polarity", "Outbending");
    
    // Map target string to enum
    Core::Target target = Core::Target::LD2;
    if (target_str == "CxC") {
        target = Core::Target::CxC;
    } else if (target_str == "CuSn") {
        target = Core::Target::CuSn;
    } else if (target_str == "Cu") {
        target = Core::Target::Cu;
    } else if (target_str == "Sn") {
        target = Core::Target::Sn;
    }
    
    Core::Polarity polarity = Core::Polarity::Outbending;
    if (polarity_str == "Inbending") {
        polarity = Core::Polarity::Inbending;
    }

    // Create application with name from config
    Framework::Application app(app_name);

    // Configure services
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file(hist_file);

    auto cond_svc = std::make_unique<Framework::ConditionsService>();
    cond_svc->set_run_number(run_number);

    // Add services (ConfigService is added for potential use by algorithms)
    app.service_manager().add_service(std::move(config_svc));
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(cond_svc));

    // Add algorithms with configuration from TOML
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection",
        target,
        polarity
    );
    app.add_algorithm(std::move(electron_alg));

    // Configure input from config
    std::vector<std::string> input_files = Core::read_recursive_file_in_directory(data_dir);
    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No input files found in {}\n", data_dir);
        return 1;
    }
    app.set_input_files(input_files);
    app.set_max_events(max_events);
    app.set_num_threads(num_threads);

    fmt::print("Configuration loaded from: {}\n", config_path);
    fmt::print("  Application: {}\n", app_name);
    fmt::print("  Max events: {}\n", max_events);
    fmt::print("  Threads: {}\n", num_threads);
    fmt::print("  Data directory: {}\n", data_dir);
    fmt::print("  Output file: {}\n", hist_file);
    fmt::print("  Target: {}\n", target_str);
    fmt::print("  Polarity: {}\n", polarity_str);
    fmt::print("\n");

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
    fmt::print("  Output: {}\n\n", hist_file);

    return 0;
}
