/**
 * @file electron_analysis.cpp
 * @brief Electron analysis using the improved framework patterns
 *
 * Demonstrates:
 * - Structured configuration parsing
 * - Clean service setup
 * - Proper error handling
 */

#include <cstdlib>
#include <optional>
#include <string>
#include <vector>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include "Algorithms/ElectronAnalysisAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/Runs.hpp"
#include "CLAS/StringUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/Application.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HipoFileService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/MetadataService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/ProfilingService.hpp"
#include "Framework/Services/RootFileService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"

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
    cxxopts::Options options("electron_analysis", "Electron analysis");

    options.add_options()
        ("target", "Target (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value("LD2"))
        ("polarity", "Polarity (Outbending, Inbending)", cxxopts::value<std::string>()->default_value("Outbending"))
        ("percentage", "Events to process (%)", cxxopts::value<double>()->default_value("100"))
        ("num_threads", "Thread count", cxxopts::value<int>()->default_value("1"))
        ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("electron_analysis_{target}_{polarity}.root"))
        ("input", "Input HIPO files (comma-separated)", cxxopts::value<std::string>()->default_value(""))
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

    // Build output filename
    config.output_file = args["output"].as<std::string>();
    config.output_file = CLAS::StringUtils::replace_all(
        config.output_file, "{target}", CLAS::to_string(config.target));
    config.output_file = CLAS::StringUtils::replace_all(
        config.output_file, "{polarity}", CLAS::to_string(config.polarity));

    // Get input files
    std::string input_str = args["input"].as<std::string>();
    if (!input_str.empty()) {
        config.input_files = CLAS::StringUtils::split(input_str, ',');
    } else {
        config.input_files = find_default_input_files(config.target, config.polarity);
    }

    return config;
}

[[nodiscard]] auto find_default_input_files(CLAS::Target target, CLAS::Polarity polarity) -> std::vector<std::string> {
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
// Service setup
// ============================================================================
auto setup_services(Framework::Application& app, const AnalysisConfig& config) -> void {
    auto& mgr = app.service_manager();

    // Create services
    auto log_svc = std::make_unique<Framework::LoggingService>();
    auto hipo_svc = std::make_unique<Framework::HipoFileService>();
    auto root_svc = std::make_unique<Framework::RootFileService>();
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    auto ntuple_svc = std::make_unique<Framework::NTupleService>();
    auto metadata_svc = std::make_unique<Framework::MetadataService>();
    auto particle_svc = std::make_unique<Framework::ParticleSelectorService>();
    auto sf_svc = std::make_unique<Framework::SamplingFractionService>();
    auto conditions_svc = std::make_unique<Framework::ConditionsService>();
    auto profiling_svc = std::make_unique<Framework::ProfilingService>();

    // Configure file services
    hipo_svc->set_input_files(config.input_files);
    root_svc->set_default_output(config.output_file);
    hist_svc->set_output_file(config.output_file);
    ntuple_svc->set_output_file(config.output_file);
    ntuple_svc->enable_incremental_writing();
    metadata_svc->set_output_file(config.output_file);

    // Configure metadata
    metadata_svc->set_version("2.0");
    metadata_svc->set_beam_energy(10.53);
    metadata_svc->set_target(CLAS::to_string(config.target));
    metadata_svc->add_parameter("polarity", CLAS::to_string(config.polarity));
    metadata_svc->add_parameter("percentage", config.percentage);
    metadata_svc->add_parameter("num_threads", config.num_threads);

    // Add services in dependency order
    mgr.add_service(std::move(log_svc));
    mgr.add_service(std::move(hipo_svc));
    mgr.add_service(std::move(root_svc));
    mgr.add_service(std::move(conditions_svc));
    mgr.add_service(std::move(hist_svc));
    mgr.add_service(std::move(ntuple_svc));
    mgr.add_service(std::move(particle_svc));
    mgr.add_service(std::move(sf_svc));
    mgr.add_service(std::move(metadata_svc));
    mgr.add_service(std::move(profiling_svc));
}

// ============================================================================
// Algorithm setup
// ============================================================================
auto setup_algorithms(Framework::Application& app, const AnalysisConfig& config) -> void {
    // Event monitor
    app.add_algorithm(std::make_unique<Algorithms::EventMonitor>("EventMonitor"));

    // Electron analysis with configuration
    Algorithms::ElectronAnalysisConfig alg_config;
    alg_config.target = config.target;
    alg_config.polarity = config.polarity;

    app.add_algorithm(std::make_unique<Algorithms::ElectronAnalysisAlg>("ElectronAnalysis", alg_config));
}

// ============================================================================
// Main
// ============================================================================
auto main(int argc, char* argv[]) -> int {
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();

    // Parse configuration
    auto config = parse_arguments(argc, argv);
    if (!config) {
        return EXIT_SUCCESS;  // Help was shown
    }

    if (!config->is_valid()) {
        fmt::print(stderr, "ERROR: Invalid configuration or no input files found\n");
        return EXIT_FAILURE;
    }

    // Create and configure application
    Framework::Application app("Electron Analysis");
    setup_services(app, *config);
    setup_algorithms(app, *config);

    app.set_input_files(config->input_files);
    app.set_percentage(config->percentage);
    app.set_num_threads(config->num_threads);

    // Run analysis
    if (auto sc = app.initialize(); sc.is_failure()) {
        Framework::Log::error("Main", "Initialization failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    Framework::Log::info("Main", "Configuration:");
    Framework::Log::info("Main", "  Target: {}", CLAS::to_string(config->target));
    Framework::Log::info("Main", "  Polarity: {}", CLAS::to_string(config->polarity));
    Framework::Log::info("Main", "  Input files: {}", config->input_files.size());
    Framework::Log::info("Main", "  Output: {}", config->output_file);

    if (auto sc = app.run(); sc.is_failure()) {
        Framework::Log::error("Main", "Processing failed: {}", sc.message());
        return EXIT_FAILURE;
    }

    if (auto sc = app.finalize(); sc.is_failure()) {
        Framework::Log::warning("Main", "Finalization had issues");
    }

    Framework::Log::info("Main", "Analysis complete: {}", config->output_file);
    return EXIT_SUCCESS;
}
