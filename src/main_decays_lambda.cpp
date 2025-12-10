/**
 * @file main_decays_lambda.cpp
 * @brief Lambda analysis using DECAYS::Particle bank
 *
 * This executable reads pre-reconstructed Lambda candidates from the
 * DECAYS::Particle bank and performs analysis.
 */

#include <fmt/format.h>
#include <cstdlib>
#include <cxxopts.hpp>
#include <string>
#include <TROOT.h>
#include "Algorithms/DecaysLambdaAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/StringUtils.hpp"
#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/MetadataService.hpp"
#include "Framework/Services/ProfilingService.hpp"

int main(int argc, char* argv[]) {
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();
    
    cxxopts::Options options("decays_lambda_analysis", "Lambda analysis using DECAYS::Particle bank");

    options.add_options() 
            ("percentage", "Percentage of events to process (0-100)", cxxopts::value<double>()->default_value("100"))
            ("num_threads", "Number of threads", cxxopts::value<int>()->default_value("1"))
            ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("decays_lambda_analysis.root"))
            ("input", "Input HIPO files (comma-separated or single file)", cxxopts::value<std::string>()->default_value(""))
            ("mass_window", "Lambda mass window in GeV", cxxopts::value<double>()->default_value("0.1"))
            ("min_momentum", "Minimum Lambda momentum in GeV", cxxopts::value<double>()->default_value("0.0"))
            ("min_decay_length", "Minimum decay length in cm (0=disabled)", cxxopts::value<double>()->default_value("0.0"))
            ("min_vertex_detach", "Min vertex detachment z in cm (decay_vz - origin_vz)", cxxopts::value<double>()->default_value("-1000"))
            ("max_vertex_detach", "Max vertex detachment z in cm (0=no limit)", cxxopts::value<double>()->default_value("0"))
            ("help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return EXIT_SUCCESS;
    }

    double percentage = args["percentage"].as<double>();
    int num_threads = args["num_threads"].as<int>();
    std::string output_file = args["output"].as<std::string>();
    std::string input_str = args["input"].as<std::string>();
    double mass_window = args["mass_window"].as<double>();
    double min_momentum = args["min_momentum"].as<double>();
    double min_decay_length = args["min_decay_length"].as<double>();
    double min_vertex_detach = args["min_vertex_detach"].as<double>();
    double max_vertex_detach = args["max_vertex_detach"].as<double>();

    // Determine input files
    std::vector<std::string> input_files = {"/work/clas12/skuditha/skims/phys_val/ana_v2/020082_lambda.hipo"};

    // Create application
    Framework::Application app("DECAYS Lambda Analysis");

    // Create services
    auto log_svc = std::make_unique<Framework::LoggingService>();
    auto ntuple_svc = std::make_unique<Framework::NTupleService>();
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    auto metadata_svc = std::make_unique<Framework::MetadataService>();
    auto profiling_svc = std::make_unique<Framework::ProfilingService>();

    // Configure output
    ntuple_svc->set_output_file(output_file);
    hist_svc->set_output_file(output_file);
    metadata_svc->set_output_file(output_file);
    ntuple_svc->enable_incremental_writing();

    // Configure metadata
    metadata_svc->set_version("1.0");
    metadata_svc->set_command_line(argc, argv);
    metadata_svc->add_parameter("percentage", percentage);
    metadata_svc->add_parameter("num_threads", num_threads);
    metadata_svc->add_parameter("mass_window", mass_window);
    metadata_svc->add_parameter("min_momentum", min_momentum);
    metadata_svc->add_parameter("min_decay_length", min_decay_length);
    metadata_svc->add_parameter("min_vertex_detach", min_vertex_detach);
    metadata_svc->add_parameter("max_vertex_detach", max_vertex_detach);

    // Add services
    app.service_manager().add_service(std::move(log_svc));
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(ntuple_svc));
    app.service_manager().add_service(std::move(metadata_svc));
    app.service_manager().add_service(std::move(profiling_svc));

    // Add algorithms
    auto monitoring_alg = std::make_unique<Algorithms::EventMonitor>("EventMonitor");
    app.add_algorithm(std::move(monitoring_alg));

    auto decays_lambda_alg = std::make_unique<Algorithms::DecaysLambdaAlg>("DecaysLambdaAnalysis");
    decays_lambda_alg->set_mass_window(mass_window);
    decays_lambda_alg->set_min_momentum(min_momentum);
    decays_lambda_alg->set_min_decay_length(min_decay_length);
    decays_lambda_alg->set_min_vertex_detach(min_vertex_detach);
    decays_lambda_alg->set_max_vertex_detach(max_vertex_detach);
    app.add_algorithm(std::move(decays_lambda_alg));

    // Configure input
    app.set_input_files(input_files);
    app.set_percentage(percentage);
    app.set_num_threads(num_threads);

    // Run analysis
    auto sc = app.initialize();
    if (sc.is_failure()) {
        Framework::Log::error("Application", "Initialization failed");
        return EXIT_FAILURE;
    }

    Framework::Log::info("Configuration", "DECAYS Lambda Analysis Configuration:");
    Framework::Log::info("Configuration", "  Percentage: {}", percentage);
    Framework::Log::info("Configuration", "  Threads: {}", num_threads);
    Framework::Log::info("Configuration", "  Output file: {}", output_file);
    Framework::Log::info("Configuration", "  Input files: {}", input_files.size());
    Framework::Log::info("Configuration", "  Mass window: {} GeV", mass_window);
    Framework::Log::info("Configuration", "  Min momentum: {} GeV", min_momentum);
    Framework::Log::info("Configuration", "  Min decay length: {} cm", min_decay_length);
    Framework::Log::info("Configuration", "  Min vertex detach: {} cm", min_vertex_detach);
    Framework::Log::info("Configuration", "  Max vertex detach: {} cm", max_vertex_detach);

    sc = app.run();
    if (sc.is_failure()) {
        Framework::Log::error("Application", "Event processing failed");
        return EXIT_FAILURE;
    }

    sc = app.finalize();
    if (sc.is_failure()) {
        Framework::Log::warning("Application", "Finalization had issues");
    }

    Framework::Log::info("Application", "DECAYS Lambda analysis complete! Output: {}", output_file);

    return EXIT_SUCCESS;
}
