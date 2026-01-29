/**
 * @file pi0.cpp
 * @brief Pi0 reconstruction analysis using ApplicationBuilder pattern
 */

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <clas12/FileUtils.hpp>
#include <clas12/StringUtils.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/ApplicationBuilder.hpp>
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

// ============================================================================
// Configuration
// ============================================================================
struct Pi0Config {
    clas12::Target target = clas12::Target::LD2;
    clas12::Polarity polarity = clas12::Polarity::Outbending;
    double percentage = 100.0;
    int num_threads = 1;
    std::string output_file;
    std::vector<std::string> input_files;

    // Pi0-specific cuts
    double mass_window = 0.2;
    double min_energy = 0.3;

    [[nodiscard]] auto is_valid() const -> bool {
        return !input_files.empty() && !output_file.empty() && percentage > 0;
    }
};

// Forward declaration
[[nodiscard]] auto find_default_input_files(clas12::Target target, clas12::Polarity polarity)
    -> std::vector<std::string>;

// ============================================================================
// Command-line parsing
// ============================================================================
[[nodiscard]] auto parse_arguments(int argc, char* argv[]) -> std::optional<Pi0Config> {
    cxxopts::Options options("pi0_analysis", "Pi0 reconstruction analysis");

    options.add_options()
        ("target", "Target (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value("LD2"))
        ("polarity", "Polarity (Outbending, Inbending)", cxxopts::value<std::string>()->default_value("Outbending"))
        ("percentage", "Events to process (%)", cxxopts::value<double>()->default_value("100"))
        ("num_threads", "Thread count", cxxopts::value<int>()->default_value("1"))
        ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("pi0_analysis_{target}_{polarity}.root"))
        ("input", "Input (file, directory, or comma-separated)", cxxopts::value<std::string>()->default_value(""))
        ("mass_window", "Pi0 mass window [GeV]", cxxopts::value<double>()->default_value("0.2"))
        ("min_energy", "Minimum photon energy [GeV]", cxxopts::value<double>()->default_value("0.3"))
        ("h,help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return std::nullopt;
    }

    Pi0Config config;
    config.target = clas12::string_to_target(args["target"].as<std::string>());
    config.polarity = clas12::string_to_polarity(args["polarity"].as<std::string>());
    config.percentage = args["percentage"].as<double>();
    config.num_threads = args["num_threads"].as<int>();
    config.mass_window = args["mass_window"].as<double>();
    config.min_energy = args["min_energy"].as<double>();

    config.output_file = args["output"].as<std::string>();
    config.output_file = clas12::StringUtils::replace_all(
        config.output_file, "{target}", clas12::to_string(config.target));
    config.output_file = clas12::StringUtils::replace_all(
        config.output_file, "{polarity}", clas12::to_string(config.polarity));

    std::string input_str = args["input"].as<std::string>();
    if (!input_str.empty()) {
        if (std::filesystem::is_directory(input_str)) {
            config.input_files = clas12::FileUtils::read_recursive_hipo_files(input_str);
        } else if (input_str.find(',') != std::string::npos) {
            config.input_files = clas12::StringUtils::split(input_str, ',');
        } else {
            config.input_files = {input_str};
        }
    } else {
        config.input_files = find_default_input_files(config.target, config.polarity);
    }

    return config;
}

[[nodiscard]] auto find_default_input_files(clas12::Target target, clas12::Polarity polarity)
    -> std::vector<std::string> {

    struct DataSource {
        clas12::Target target;
        std::string_view outbending_path;
        std::string_view inbending_path;
    };

    constexpr std::array<DataSource, 3> sources = {{
        {clas12::Target::LD2,
         "/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {clas12::Target::CxC,
         "/cache/clas12/rg-d/production/pass1/recon/CxC/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {clas12::Target::CuSn,
         "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
    }};

    for (const auto& src : sources) {
        if (src.target == target) {
            std::string path = (polarity == clas12::Polarity::Outbending)
                ? std::string(src.outbending_path)
                : std::string(src.inbending_path);
            return clas12::FileUtils::read_recursive_hipo_files(path);
        }
    }
    return {};
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
        clas12::framework::Log::error("Main", "Invalid configuration or no input files found");
        return EXIT_FAILURE;
    }

    // Build application using fluent API
    auto app = clas12::framework::make_application("Pi0 Analysis")
        .with_input_files(config->input_files)
        .with_output(config->output_file)
        .with_percentage(config->percentage)
        .with_threads(config->num_threads)
        .with_standard_services()
        .with_version("2.0")
        .with_beam_energy(10.53)
        .add_service<clas12::framework::ElectronCutsService>()
        .add_service<clas12::framework::FiducialService>()
        .with_target(clas12::to_string(config->target))
        .add_service<clas12::framework::PhotonCutsService>()
        .add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<clas12::algorithms::ElectronSelectionAlg>()
        .add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
        .add_algorithm<clas12::algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", config->mass_window, config->min_energy)
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) {
        clas12::framework::Log::error("Main", "Initialization failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    // Print configuration using enhanced logger
    clas12::framework::Log::section("Pi0 Reconstruction Analysis");
    clas12::framework::Log::subsection("Configuration");
    clas12::framework::Log::config("Config", "Target", clas12::to_string(config->target));
    clas12::framework::Log::config("Config", "Polarity", clas12::to_string(config->polarity));
    clas12::framework::Log::config("Config", "Input files", config->input_files.size());
    clas12::framework::Log::config("Config", "Threads", config->num_threads);
    clas12::framework::Log::config("Config", "Output", config->output_file);
    clas12::framework::Log::subsection("Pi0 Cuts");
    clas12::framework::Log::config("Cuts", "Mass window", fmt::format("{} GeV", config->mass_window));
    clas12::framework::Log::config("Cuts", "Min photon energy", fmt::format("{} GeV", config->min_energy));
    clas12::framework::Log::blank();

    // Run with timing
    {
        auto timer = clas12::framework::Log::scoped("Main", "Event processing");
        if (auto sc = app.run(); sc.is_failure()) {
            clas12::framework::Log::error("Main", "Processing failed: {}", sc.message());
            return EXIT_FAILURE;
        }
    }

    auto result = app.finalize();
    if (result.is_failure()) {
        clas12::framework::Log::error("Main", "Finalization failed: {}", result.message());
        return EXIT_FAILURE;
    }
    clas12::framework::Log::success("Main", "Analysis complete: {}", config->output_file);
    return EXIT_SUCCESS;
}
