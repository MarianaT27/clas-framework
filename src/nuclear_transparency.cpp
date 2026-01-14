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

#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "Algorithms/Rho0ReconstructionAlg.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/StringUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/ApplicationBuilder.hpp"
#include "Physics/DISAnalysis.hpp"

// ============================================================================
// Configuration
// ============================================================================
struct NuclearTransparencyConfig {
    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;
    double percentage = 100.0;
    int num_threads = 1;
    std::string output_file;
    std::vector<std::string> input_files;

    // Nuclear transparency-specific cuts
    double mass_window = 1.0;
    double min_energy = 0.3;

    [[nodiscard]] auto is_valid() const -> bool {
        return !input_files.empty() && !output_file.empty() && percentage > 0;
    }
};

// Forward declaration
[[nodiscard]] auto find_default_input_files(CLAS::Target target, CLAS::Polarity polarity)
    -> std::vector<std::string>;

// ============================================================================
// Command-line parsing
// ============================================================================
[[nodiscard]] auto parse_arguments(int argc, char* argv[]) -> std::optional<NuclearTransparencyConfig> {
    cxxopts::Options options("nuclear_transparency_analysis", "Nuclear transparency analysis");

    options.add_options()
        ("target", "Target (LD2, CxC, CuSn, Cu, Sn)", cxxopts::value<std::string>()->default_value("LD2"))
        ("polarity", "Polarity (Outbending, Inbending)", cxxopts::value<std::string>()->default_value("Outbending"))
        ("percentage", "Events to process (%)", cxxopts::value<double>()->default_value("100"))
        ("num_threads", "Thread count", cxxopts::value<int>()->default_value("1"))
        ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("nuclear_transparency_analysis_{target}_{polarity}.root"))
        ("input", "Input (file, directory, or comma-separated)", cxxopts::value<std::string>()->default_value(""))
        ("mass_window", "Pi0 mass window [GeV]", cxxopts::value<double>()->default_value("1.0"))
        ("min_energy", "Minimum photon energy [GeV]", cxxopts::value<double>()->default_value("0.3"))
        ("h,help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return std::nullopt;
    }

    NuclearTransparencyConfig config;
    config.target = CLAS::string_to_target(args["target"].as<std::string>());
    config.polarity = CLAS::string_to_polarity(args["polarity"].as<std::string>());
    config.percentage = args["percentage"].as<double>();
    config.num_threads = args["num_threads"].as<int>();
    config.mass_window = args["mass_window"].as<double>();
    config.min_energy = args["min_energy"].as<double>();

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
        config.input_files = find_default_input_files(config.target, config.polarity);
    }

    return config;
}

[[nodiscard]] auto find_default_input_files(CLAS::Target target, CLAS::Polarity polarity)
    -> std::vector<std::string> {

    struct DataSource {
        CLAS::Target target;
        std::string_view outbending_path;
        std::string_view inbending_path;
    };

    constexpr std::array<DataSource, 5> sources = {{
        {CLAS::Target::LD2,
         "/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {CLAS::Target::CxC,
         "/cache/clas12/rg-d/production/pass1/recon/CxC/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {CLAS::Target::CuSn,
         "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {CLAS::Target::Cu,
         "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {CLAS::Target::Sn,
         "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
    }};

    for (const auto& src : sources) {
        if (src.target == target) {
            std::string path = (polarity == CLAS::Polarity::Outbending)
                ? std::string(src.outbending_path)
                : std::string(src.inbending_path);
            return CLAS::FileUtils::read_recursive_hipo_files(path);
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
        fmt::print(stderr, "ERROR: Invalid configuration or no input files found\n");
        return EXIT_FAILURE;
    }

    // Build application using fluent API
    auto app = Framework::make_application("Nuclear Transparency Analysis")
        .with_input_files(config->input_files)
        .with_output(config->output_file)
        .with_percentage(config->percentage)
        .with_threads(config->num_threads)
        .with_standard_services()
        .with_version("2.0")
        .with_beam_energy(10.53)
        .with_target(CLAS::to_string(config->target))
        .add_service<Framework::ElectronCutsService>()
        .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<Algorithms::ElectronSelectionAlg>()
        .add_algorithm<Algorithms::DISSelectionAlg>("DISSelection", Physics::DISAnalysis::standard_dis_region())
        .add_algorithm<Algorithms::Rho0ReconstructionAlg>("Rho0Reconstruction", config->min_energy, config->mass_window)
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) {
        fmt::print(stderr, "Initialization failed: {}\n", sc.message());
        return EXIT_FAILURE;
    }

    Framework::Log::info("Main", "Nuclear Transparency Analysis Configuration:");
    Framework::Log::info("Main", "  Target: {}", CLAS::to_string(config->target));
    Framework::Log::info("Main", "  Polarity: {}", CLAS::to_string(config->polarity));
    Framework::Log::info("Main", "  Input files: {}", config->input_files.size());
    Framework::Log::info("Main", "  Output: {}", config->output_file);
    Framework::Log::info("Main", "  Mass window: {} GeV", config->mass_window);
    Framework::Log::info("Main", "  Min photon energy: {} GeV", config->min_energy);

    if (auto sc = app.run(); sc.is_failure()) {
        fmt::print(stderr, "Processing failed: {}\n", sc.message());
        return EXIT_FAILURE;
    }

    auto result = app.finalize();
    if (result.is_failure()) {
        fmt::print(stderr, "Finalization failed: {}\n", result.message());
        return EXIT_FAILURE;
    }
    Framework::Log::info("Main", "Analysis complete: {}", config->output_file);
    return EXIT_SUCCESS;
}
