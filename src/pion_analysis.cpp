/**
 * @file pion_analysis.cpp
 * @brief Pion analysis with calorimeter SF and DC quality histograms
 */

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "Algorithms/PionAnalysisAlg.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/StringUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/ApplicationBuilder.hpp"

// ============================================================================
// Configuration
// ============================================================================
struct AnalysisConfig {
    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;
    double percentage = 100.0;
    int num_threads = 1;
    std::string output_file;
    std::vector<std::string> input_files;

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
[[nodiscard]] auto parse_arguments(int argc, char* argv[]) -> std::optional<AnalysisConfig> {
    cxxopts::Options options("pion_analysis", "Pion analysis with SF and DC quality histograms");

    options.add_options()
        ("target", "Target (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value("LD2"))
        ("polarity", "Polarity (Outbending, Inbending)", cxxopts::value<std::string>()->default_value("Outbending"))
        ("percentage", "Events to process (%)", cxxopts::value<double>()->default_value("100"))
        ("num_threads", "Thread count", cxxopts::value<int>()->default_value("1"))
        ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("pion_analysis_{target}_{polarity}.root"))
        ("input", "Input (file, directory, or comma-separated)", cxxopts::value<std::string>()->default_value(""))
        ("h,help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return std::nullopt;
    }

    AnalysisConfig config;
    config.target = CLAS::string_to_target(args["target"].as<std::string>());
    config.polarity = CLAS::string_to_polarity(args["polarity"].as<std::string>());
    config.percentage = args["percentage"].as<double>();
    config.num_threads = args["num_threads"].as<int>();

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

    constexpr std::array<DataSource, 3> sources = {{
        {CLAS::Target::LD2,
         "/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {CLAS::Target::CxC,
         "/cache/clas12/rg-d/production/pass1/recon/CxC/dst/train/SIDIS/",
         "/cache/hallb/scratch/rg-d/production/skim_pass0v11/"},
        {CLAS::Target::CuSn,
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
    auto app = Framework::make_application("Pion Analysis")
        .with_input_files(config->input_files)
        .with_output(config->output_file)
        .with_percentage(config->percentage)
        .with_threads(config->num_threads)
        .with_standard_services()
        .with_version("2.0")
        .with_beam_energy(10.53)
        .with_target(CLAS::to_string(config->target))
        .add_algorithm<Algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<Algorithms::ElectronSelectionAlg>(
            "ElectronSelection", config->target, config->polarity)
        .add_algorithm<Algorithms::PionAnalysisAlg>(
            "PionAnalysis", config->target, config->polarity)
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) {
        fmt::print(stderr, "Initialization failed: {}\n", sc.message());
        return EXIT_FAILURE;
    }

    Framework::Log::info("Main", "Pion Analysis Configuration:");
    Framework::Log::info("Main", "  Target: {}", CLAS::to_string(config->target));
    Framework::Log::info("Main", "  Polarity: {}", CLAS::to_string(config->polarity));
    Framework::Log::info("Main", "  Input files: {}", config->input_files.size());
    Framework::Log::info("Main", "  Output: {}", config->output_file);

    if (auto sc = app.run(); sc.is_failure()) {
        fmt::print(stderr, "Processing failed: {}\n", sc.message());
        return EXIT_FAILURE;
    }

    app.finalize();
    Framework::Log::info("Main", "Analysis complete: {}", config->output_file);
    return EXIT_SUCCESS;
}
