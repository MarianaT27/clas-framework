/**
 * @file electron_analysis.cpp
 * @brief Electron analysis using the ApplicationBuilder pattern
 */

#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include <clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/FileUtils.hpp>
#include <clas12/StringUtils.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/ApplicationBuilder.hpp>

// ============================================================================
// Configuration
// ============================================================================
struct AnalysisConfig {
    clas12::Target target = clas12::Target::LD2;
    clas12::Polarity polarity = clas12::Polarity::Outbending;
    double percentage = 100.0;
    int num_threads = 1;
    std::string output_file;
    std::vector<std::string> input_files;

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
[[nodiscard]] auto parse_arguments(int argc, char* argv[]) -> std::optional<AnalysisConfig> {
    cxxopts::Options options("electron_analysis", "Electron analysis");

    options.add_options()
        ("target", "Target (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value("LD2"))
        ("polarity", "Polarity (Outbending, Inbending)", cxxopts::value<std::string>()->default_value("Outbending"))
        ("percentage", "Events to process (%)", cxxopts::value<double>()->default_value("100"))
        ("num_threads", "Thread count", cxxopts::value<int>()->default_value("1"))
        ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("electron_analysis_{target}_{polarity}.root"))
        ("input", "Input (file, directory, or comma-separated)", cxxopts::value<std::string>()->default_value(""))
        ("h,help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return std::nullopt;
    }

    AnalysisConfig config;
    config.target = clas12::string_to_target(args["target"].as<std::string>());
    config.polarity = clas12::string_to_polarity(args["polarity"].as<std::string>());
    config.percentage = args["percentage"].as<double>();
    config.num_threads = args["num_threads"].as<int>();

    config.output_file = args["output"].as<std::string>();
    config.output_file = clas12::StringUtils::replace_all(config.output_file, "{target}", clas12::to_string(config.target));
    config.output_file = clas12::StringUtils::replace_all(config.output_file, "{polarity}", clas12::to_string(config.polarity));

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

    // Configure algorithm
    clas12::algorithms::ElectronAnalysisConfig alg_config;
    alg_config.target = config->target;
    alg_config.polarity = config->polarity;

    // Build application using fluent API
    auto app = clas12::framework::make_application("Electron Analysis")
        .with_input_files(config->input_files)
        .with_output(config->output_file)
        .with_percentage(config->percentage)
        .with_threads(config->num_threads)
        .with_standard_services()
        .add_service<clas12::framework::ElectronCutsService>()
        .with_version("2.0")
        .with_beam_energy(10.53)
        .with_target(clas12::to_string(config->target))
        .add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
        .add_algorithm<clas12::algorithms::ElectronAnalysisAlg>("ElectronAnalysis", alg_config)
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) {
        clas12::framework::Log::error("Main", "Initialization failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    // Print configuration using enhanced logger
    clas12::framework::Log::section("Electron Analysis");
    clas12::framework::Log::config("Config", "Target", clas12::to_string(config->target));
    clas12::framework::Log::config("Config", "Polarity", clas12::to_string(config->polarity));
    clas12::framework::Log::config("Config", "Input files", config->input_files.size());
    clas12::framework::Log::config("Config", "Threads", config->num_threads);
    clas12::framework::Log::config("Config", "Output", config->output_file);
    clas12::framework::Log::blank();

    // Run with timing
    {
        auto timer = clas12::framework::Log::scoped("Main", "Event processing");
        if (auto sc = app.run(); sc.is_failure()) {
            clas12::framework::Log::error("Main", "Processing failed: {}", sc.message());
            return EXIT_FAILURE;
        }
    }

    app.finalize();
    clas12::framework::Log::success("Main", "Analysis complete: {}", config->output_file);
    return EXIT_SUCCESS;
}
