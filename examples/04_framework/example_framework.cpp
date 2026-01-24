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

#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/services/ConfigService.hpp>
#include <clas12/framework/services/ConditionsService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/FileUtils.hpp>
#include <fmt/format.h>

int main(int argc, char* argv[]) {
    // Determine config file path (default or from command line)
    std::string config_path = "config/example_framework.toml";
    if (argc > 1) {
        config_path = argv[1];
    }

    // Create and initialize ConfigService first to load configuration
    auto config_svc = std::make_unique<clas12::framework::ConfigService>(config_path);
    auto config_sc = config_svc->initialize();
    if (config_sc.is_failure()) {
        clas12::framework::Log::error("Main", "Failed to load configuration: {}", config_sc.message());
        return 1;
    }

    // Validate required configuration keys
    auto validate_result = config_svc->validate({
        "application.name",
        "input.data_directory",
        "output.histogram_file"
    });
    if (validate_result.is_error()) {
        clas12::framework::Log::error("Main", "Configuration validation failed: {}",
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
    clas12::Target target = clas12::Target::LD2;
    if (target_str == "CxC") {
        target = clas12::Target::CxC;
    } else if (target_str == "CuSn") {
        target = clas12::Target::CuSn;
    } else if (target_str == "Cu") {
        target = clas12::Target::Cu;
    } else if (target_str == "Sn") {
        target = clas12::Target::Sn;
    }
    
    clas12::Polarity polarity = clas12::Polarity::Outbending;
    if (polarity_str == "Inbending") {
        polarity = clas12::Polarity::Inbending;
    }

    // Create application with name from config
    clas12::framework::Application app(app_name);

    // Configure services
    auto output_svc = std::make_unique<clas12::framework::OutputFileService>();
    output_svc->set_output_file(hist_file);
    clas12::framework::OutputFileService* output_svc_ptr = output_svc.get();

    auto hist_svc = std::make_unique<clas12::framework::HistogramService>();
    hist_svc->set_output_file_service(output_svc_ptr);

    auto cond_svc = std::make_unique<clas12::framework::ConditionsService>();
    cond_svc->set_run_number(run_number);

    // Add ParticleSelectorService
    auto particle_svc = std::make_unique<clas12::framework::ParticleSelectorService>();

    // Add SamplingFractionService
    auto sf_svc = std::make_unique<clas12::framework::SamplingFractionService>();

    // Add services (ConfigService is added for potential use by algorithms)
    app.service_manager().add_service(std::move(config_svc));
    app.service_manager().add_service(std::move(output_svc));
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(cond_svc));
    app.service_manager().add_service(std::move(particle_svc));
    app.service_manager().add_service(std::move(sf_svc));

    // Add algorithms with configuration from TOML
    auto electron_alg = std::make_unique<clas12::algorithms::ElectronSelectionAlg>(
        "ElectronSelection",
        target,
        polarity
    );
    app.add_algorithm(std::move(electron_alg));

    // Configure input from config
    std::vector<std::string> input_files = clas12::FileUtils::read_recursive_hipo_files(data_dir);
    if (input_files.empty()) {
        clas12::framework::Log::error("Main", "No input files found in {}", data_dir);
        return 1;
    }
    app.set_input_files(input_files);
    // app.set_max_events(max_events);
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
        clas12::framework::Log::error("Main", "Initialization failed");
        return 1;
    }

    sc = app.run();
    if (sc.is_failure()) {
        clas12::framework::Log::error("Main", "Event processing failed");
        return 1;
    }

    sc = app.finalize();
    if (sc.is_failure()) {
        clas12::framework::Log::warning("Main", "Finalization had issues");
    }

    fmt::print("\n✓ Analysis complete!\n");
    fmt::print("  Output: {}\n\n", hist_file);

    return 0;
}
