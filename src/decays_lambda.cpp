/**
 * @file decays_lambda.cpp
 * @brief Lambda analysis using DECAYS::Particle bank
 *
 * This executable reads pre-reconstructed Lambda candidates from the
 * DECAYS::Particle bank and performs analysis.
 */

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include "Algorithms/DecaysLambdaAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/StringUtils.hpp"
#include "Framework/ApplicationBuilder.hpp"

// ============================================================================
// Configuration
// ============================================================================
struct DecaysLambdaConfig {
    double percentage = 100.0;
    int num_threads = 1;
    std::string output_file = "decays_lambda_analysis.root";
    std::vector<std::string> input_files;

    // Lambda-specific cuts
    double mass_window = 0.2;
    double min_momentum = 0.0;
    double min_decay_length = 0.0;
    double min_vertex_detach = 1.0;
    double max_vertex_detach = 100.0;

    [[nodiscard]] auto is_valid() const -> bool {
        return !input_files.empty() && !output_file.empty() && percentage > 0;
    }
};

// ============================================================================
// Command-line parsing
// ============================================================================
[[nodiscard]] auto parse_arguments(int argc, char* argv[]) -> std::optional<DecaysLambdaConfig> {
    cxxopts::Options options("decays_lambda_analysis", "Lambda analysis using DECAYS::Particle bank");

    options.add_options()
        ("percentage", "Events to process (%)", cxxopts::value<double>()->default_value("100"))
        ("num_threads", "Thread count", cxxopts::value<int>()->default_value("1"))
        ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("decays_lambda_analysis.root"))
        ("input", "Input (file, directory, or comma-separated)", cxxopts::value<std::string>()->default_value(""))
        ("mass_window", "Lambda mass window [GeV]", cxxopts::value<double>()->default_value("0.2"))
        ("min_momentum", "Minimum Lambda momentum [GeV]", cxxopts::value<double>()->default_value("0.0"))
        ("min_decay_length", "Minimum decay length [cm] (0=disabled)", cxxopts::value<double>()->default_value("0.0"))
        ("min_vertex_detach", "Min vertex detachment z [cm]", cxxopts::value<double>()->default_value("1.0"))
        ("max_vertex_detach", "Max vertex detachment z [cm] (0=no limit)", cxxopts::value<double>()->default_value("100"))
        ("h,help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return std::nullopt;
    }

    DecaysLambdaConfig config;
    config.percentage = args["percentage"].as<double>();
    config.num_threads = args["num_threads"].as<int>();
    config.output_file = args["output"].as<std::string>();
    config.mass_window = args["mass_window"].as<double>();
    config.min_momentum = args["min_momentum"].as<double>();
    config.min_decay_length = args["min_decay_length"].as<double>();
    config.min_vertex_detach = args["min_vertex_detach"].as<double>();
    config.max_vertex_detach = args["max_vertex_detach"].as<double>();

    std::string input_str = args["input"].as<std::string>();
    if (!input_str.empty()) {
        if (std::filesystem::is_directory(input_str)) {
            config.input_files = CLAS::FileUtils::read_recursive_hipo_files(input_str);
        } else if (input_str.find(',') != std::string::npos) {
            config.input_files = CLAS::StringUtils::split(input_str, ',');
        } else {
            config.input_files = {input_str};
        }
    } else {
        config.input_files = {"/work/clas12/skuditha/skims/phys_val/ana_v2/020082_lambda.hipo"};
    }

    return config;
}

// ============================================================================
// Main
// ============================================================================
auto main(int argc, char* argv[]) -> int {
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();

    auto config = parse_arguments(argc, argv);
    if (!config) return EXIT_SUCCESS;

    if (!config->is_valid()) {
        fmt::print(stderr, "ERROR: Invalid configuration or no input files found\n");
        return EXIT_FAILURE;
    }

    // Create algorithm with cuts configured
    auto decays_lambda_alg = std::make_unique<Algorithms::DecaysLambdaAlg>("DecaysLambdaAnalysis");
    decays_lambda_alg->set_mass_window(config->mass_window);
    decays_lambda_alg->set_min_momentum(config->min_momentum);
    decays_lambda_alg->set_min_decay_length(config->min_decay_length);
    decays_lambda_alg->set_min_vertex_detach(config->min_vertex_detach);
    decays_lambda_alg->set_max_vertex_detach(config->max_vertex_detach);

    // Build application using fluent API
    auto app = Framework::make_application("DECAYS Lambda Analysis")
        .with_input_files(config->input_files)
        .with_output(config->output_file)
        .with_percentage(config->percentage)
        .with_threads(config->num_threads)
        .with_standard_services()
        .with_version("2.0")
        .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
        .add_algorithm(std::move(decays_lambda_alg))
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) {
        fmt::print(stderr, "Initialization failed: {}\n", sc.message());
        return EXIT_FAILURE;
    }

    Framework::Log::info("Main", "DECAYS Lambda Analysis Configuration:");
    Framework::Log::info("Main", "  Percentage: {}%", config->percentage);
    Framework::Log::info("Main", "  Threads: {}", config->num_threads);
    Framework::Log::info("Main", "  Input files: {}", config->input_files.size());
    Framework::Log::info("Main", "  Output: {}", config->output_file);
    Framework::Log::info("Main", "  Mass window: {} GeV", config->mass_window);
    Framework::Log::info("Main", "  Min momentum: {} GeV", config->min_momentum);
    Framework::Log::info("Main", "  Vertex detach: [{}, {}] cm", config->min_vertex_detach, config->max_vertex_detach);

    if (auto sc = app.run(); sc.is_failure()) {
        fmt::print(stderr, "Processing failed: {}\n", sc.message());
        return EXIT_FAILURE;
    }

    app.finalize();
    Framework::Log::info("Main", "Analysis complete: {}", config->output_file);
    return EXIT_SUCCESS;
}
