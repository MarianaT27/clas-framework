/**
 * @file lambda.cpp
 * @brief Lambda reconstruction analysis (Λ → p + π⁻)
 */

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "Algorithms/LambdaReconstructionAlg.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/StringUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/ApplicationBuilder.hpp"
#include "Physics/DISAnalysis.hpp"

// ============================================================================
// Configuration
// ============================================================================
struct LambdaConfig {
    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;
    double percentage = 100.0;
    int num_threads = 1;
    std::string output_file;
    std::vector<std::string> input_files;

    // Lambda-specific cuts
    double mass_window = 0.5;
    double max_delta_vz = 5.0;
    double coplanarity = 0.0;
    double beam_energy = 10.53;
    double min_displaced_vz = -1000.0;
    double max_displaced_vz = 0.0;

    [[nodiscard]] auto is_valid() const -> bool {
        return !input_files.empty() && !output_file.empty() && percentage > 0;
    }
};

// ============================================================================
// Command-line parsing
// ============================================================================
[[nodiscard]] auto parse_arguments(int argc, char* argv[]) -> std::optional<LambdaConfig> {
    cxxopts::Options options("lambda_analysis", "Lambda reconstruction (Λ → p + π⁻)");

    options.add_options()
        ("target", "Target (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value("LD2"))
        ("polarity", "Polarity (Outbending, Inbending)", cxxopts::value<std::string>()->default_value("Outbending"))
        ("percentage", "Events to process (%)", cxxopts::value<double>()->default_value("100"))
        ("num_threads", "Thread count", cxxopts::value<int>()->default_value("1"))
        ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("lambda_analysis_{target}_{polarity}.root"))
        ("input", "Input (file, directory, or comma-separated)", cxxopts::value<std::string>()->default_value(""))
        ("mass_window", "Lambda mass window [GeV]", cxxopts::value<double>()->default_value("0.5"))
        ("max_delta_vz", "Max vertex z difference [cm]", cxxopts::value<double>()->default_value("5.0"))
        ("coplanarity", "Max coplanarity angle [deg] (0=disabled)", cxxopts::value<double>()->default_value("0"))
        ("beam_energy", "Beam energy [GeV]", cxxopts::value<double>()->default_value("10.53"))
        ("min_displaced_vz", "Min displaced vertex z [cm]", cxxopts::value<double>()->default_value("-1000"))
        ("max_displaced_vz", "Max displaced vertex z [cm] (0=no limit)", cxxopts::value<double>()->default_value("0"))
        ("h,help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return std::nullopt;
    }

    LambdaConfig config;
    config.target = CLAS::string_to_target(args["target"].as<std::string>());
    config.polarity = CLAS::string_to_polarity(args["polarity"].as<std::string>());
    config.percentage = args["percentage"].as<double>();
    config.num_threads = args["num_threads"].as<int>();
    config.mass_window = args["mass_window"].as<double>();
    config.max_delta_vz = args["max_delta_vz"].as<double>();
    config.coplanarity = args["coplanarity"].as<double>();
    config.beam_energy = args["beam_energy"].as<double>();
    config.min_displaced_vz = args["min_displaced_vz"].as<double>();
    config.max_displaced_vz = args["max_displaced_vz"].as<double>();

    config.output_file = args["output"].as<std::string>();
    config.output_file = CLAS::StringUtils::replace_all(
        config.output_file, "{target}", CLAS::to_string(config.target));
    config.output_file = CLAS::StringUtils::replace_all(
        config.output_file, "{polarity}", CLAS::to_string(config.polarity));

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
        Framework::Log::error("Main", "Invalid configuration or no input files found");
        return EXIT_FAILURE;
    }

    // Create lambda algorithm with cuts configured
    auto lambda_alg = std::make_unique<Algorithms::LambdaReconstructionAlg>(
        "LambdaReconstruction", config->mass_window);
    lambda_alg->set_max_delta_vz(config->max_delta_vz);
    lambda_alg->set_coplanarity_cut(config->coplanarity);
    lambda_alg->set_beam_energy(config->beam_energy);
    lambda_alg->set_min_displaced_vz(config->min_displaced_vz);
    lambda_alg->set_max_displaced_vz(config->max_displaced_vz);

    // Build application using fluent API
    auto app = Framework::make_application("Lambda Analysis")
        .with_input_files(config->input_files)
        .with_output(config->output_file)
        .with_percentage(config->percentage)
        .with_threads(config->num_threads)
        .with_standard_services()
        .with_version("2.0")
        .with_beam_energy(config->beam_energy)
        .with_target(CLAS::to_string(config->target))
        .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<Algorithms::ElectronSelectionAlg>(
            "ElectronSelection", config->target, config->polarity)
        .add_algorithm<Algorithms::DISSelectionAlg>(
            "DISSelection", Physics::DISAnalysis::standard_dis_region())
        .add_algorithm(std::move(lambda_alg))
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) {
        Framework::Log::error("Main", "Initialization failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    // Print configuration using enhanced logger
    Framework::Log::section("Lambda Reconstruction Analysis");
    Framework::Log::subsection("Configuration");
    Framework::Log::config("Config", "Target", CLAS::to_string(config->target));
    Framework::Log::config("Config", "Polarity", CLAS::to_string(config->polarity));
    Framework::Log::config("Config", "Input files", config->input_files.size());
    Framework::Log::config("Config", "Threads", config->num_threads);
    Framework::Log::config("Config", "Output", config->output_file);
    Framework::Log::subsection("Lambda Cuts");
    Framework::Log::config("Cuts", "Mass window", fmt::format("{} GeV", config->mass_window));
    Framework::Log::config("Cuts", "Max delta vz", fmt::format("{} cm", config->max_delta_vz));
    Framework::Log::blank();

    // Run with timing
    {
        auto timer = Framework::Log::scoped("Main", "Event processing");
        if (auto sc = app.run(); sc.is_failure()) {
            Framework::Log::error("Main", "Processing failed: {}", sc.message());
            return EXIT_FAILURE;
        }
    }

    app.finalize();
    Framework::Log::success("Main", "Analysis complete: {}", config->output_file);
    return EXIT_SUCCESS;
}
