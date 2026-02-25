#pragma once

/**
 * @file AnalysisRunner.hpp
 * @brief Shared utility to eliminate boilerplate in analysis entry points
 *
 * Every analysis entry point repeats ~90 lines of identical code:
 * - AnalysisConfig struct with target/polarity/percentage/threads/output/input
 * - CLI parsing with cxxopts
 * - Default input file lookup
 * - ROOT thread safety setup
 * - ApplicationBuilder init/run/finalize lifecycle
 *
 * AnalysisRunner extracts all of this into a single `run()` call.
 * The user provides only a lambda that configures the ApplicationBuilder
 * with analysis-specific services and algorithms.
 *
 * Inspired by ATLAS AnalysisBase's standard job runner pattern.
 *
 * Example:
 * @code
 * #include <clas12/framework/core/AnalysisRunner.hpp>
 * #include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
 * #include <clas12/algorithms/analysis/PionAnalysisAlg.hpp>
 *
 * auto main(int argc, char* argv[]) -> int {
 *     return clas12::framework::AnalysisRunner::run(
 *         argc, argv, "Pion Analysis", "pion_analysis",
 *         [](auto& builder, const auto& config) {
 *             builder
 *                 .add_service<ElectronCutsService>()
 *                 .add_service<FiducialService>()
 *                 .add_algorithm<EventMonitor>("EventMonitor")
 *                 .add_algorithm<ElectronSelectionAlg>("Selection", config.target, config.polarity)
 *                 .add_algorithm<PionAnalysisAlg>("Analysis", config.target, config.polarity);
 *         });
 * }
 * @endcode
 */

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <toml++/toml.hpp>

#include <clas12/FileUtils.hpp>
#include <clas12/StringUtils.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/ApplicationBuilder.hpp>

namespace clas12::framework {

/**
 * @brief Standard analysis configuration parsed from CLI arguments
 */
struct AnalysisConfig {
    clas12::Target target = clas12::Target::LD2;
    clas12::Polarity polarity = clas12::Polarity::Outbending;
    double percentage = 100.0;
    int num_threads = 1;
    std::string output_file;
    std::vector<std::string> input_files;

    /// Extensible configuration for analysis-specific options.
    /// ParseExtraFn writes into this table; algorithms read from it.
    /// Eliminates the need for static variable bridges in entry points.
    toml::table custom;

    [[nodiscard]] auto is_valid() const -> bool {
        return !input_files.empty() && !output_file.empty() && percentage > 0;
    }
};

/**
 * @brief Eliminates boilerplate in analysis entry points
 *
 * Handles CLI parsing, input file discovery, ROOT setup, and the
 * init/run/finalize lifecycle. The user provides only a lambda
 * to configure the ApplicationBuilder with their specific algorithms.
 */
class AnalysisRunner {
public:
    /// Type for the builder configuration callback
    using ConfigureFn = std::function<void(ApplicationBuilder&, const AnalysisConfig&)>;

    /// Type for optional extra CLI options callback
    using ExtraOptionsFn = std::function<void(cxxopts::Options&)>;

    /// Type for optional extra options parsing callback
    using ParseExtraFn = std::function<void(const cxxopts::ParseResult&, AnalysisConfig&)>;

    /**
     * @brief Run a standard analysis with minimal boilerplate
     *
     * @param argc Argument count from main()
     * @param argv Argument values from main()
     * @param analysis_name Human-readable analysis name (e.g., "Pion Analysis")
     * @param output_prefix Output file prefix (e.g., "pion_analysis")
     *        The output file will be "{prefix}_{target}_{polarity}.root"
     * @param configure_fn Lambda to configure the ApplicationBuilder
     * @param extra_options_fn Optional: add extra CLI options
     * @param parse_extra_fn Optional: parse extra CLI options into config
     * @return EXIT_SUCCESS or EXIT_FAILURE
     */
    static auto run(
        int argc, char* argv[],
        const std::string& analysis_name,
        const std::string& output_prefix,
        ConfigureFn configure_fn,
        ExtraOptionsFn extra_options_fn = nullptr,
        ParseExtraFn parse_extra_fn = nullptr
    ) -> int {
        ROOT::EnableThreadSafety();
        ROOT::EnableImplicitMT();

        auto config = parse_arguments(argc, argv, analysis_name, output_prefix,
                                       extra_options_fn, parse_extra_fn);
        if (!config) return EXIT_SUCCESS;  // --help was requested

        if (!config->is_valid()) {
            Log::error("Main", "Invalid configuration or no input files found");
            return EXIT_FAILURE;
        }

        // Build application with standard services
        auto builder = make_application(analysis_name)
            .with_input_files(config->input_files)
            .with_output(config->output_file)
            .with_percentage(config->percentage)
            .with_threads(config->num_threads)
            .with_standard_services()
            .with_version("2.0")
            .with_beam_energy(10.53)
            .with_target(clas12::to_string(config->target));

        // Let the user configure their specific services and algorithms
        configure_fn(builder, *config);

        auto app = builder.build();

        // Initialize
        if (auto sc = app.initialize(); sc.is_failure()) {
            Log::error("Main", "Initialization failed: {}", sc.message());
            return EXIT_FAILURE;
        }

        // Log configuration
        Log::section(analysis_name);
        Log::config("Config", "Target", clas12::to_string(config->target));
        Log::config("Config", "Polarity", clas12::to_string(config->polarity));
        Log::config("Config", "Input files", config->input_files.size());
        Log::config("Config", "Threads", config->num_threads);
        Log::config("Config", "Output", config->output_file);
        Log::blank();

        // Run
        {
            auto timer = Log::scoped("Main", "Event processing");
            if (auto sc = app.run(); sc.is_failure()) {
                Log::error("Main", "Processing failed: {}", sc.message());
                return EXIT_FAILURE;
            }
        }

        // Finalize
        auto result = app.finalize();
        if (result.is_failure()) {
            Log::error("Main", "Finalization failed: {}", result.message());
            return EXIT_FAILURE;
        }

        Log::success("Main", "Analysis complete: {}", config->output_file);
        return EXIT_SUCCESS;
    }

private:
    [[nodiscard]] static auto parse_arguments(
        int argc, char* argv[],
        const std::string& analysis_name,
        const std::string& output_prefix,
        const ExtraOptionsFn& extra_options_fn,
        const ParseExtraFn& parse_extra_fn
    ) -> std::optional<AnalysisConfig> {
        std::string default_output = output_prefix + "_{target}_{polarity}.root";
        cxxopts::Options options(output_prefix, analysis_name);

        options.add_options()
            ("target", "Target (LD2, CxC, CuSn, Cu, Sn)",
                cxxopts::value<std::string>()->default_value("LD2"))
            ("polarity", "Polarity (Outbending, Inbending)",
                cxxopts::value<std::string>()->default_value("Outbending"))
            ("percentage", "Events to process (%)",
                cxxopts::value<double>()->default_value("100"))
            ("num_threads", "Thread count",
                cxxopts::value<int>()->default_value("1"))
            ("output", "Output ROOT file",
                cxxopts::value<std::string>()->default_value(default_output))
            ("input", "Input (file, directory, or comma-separated)",
                cxxopts::value<std::string>()->default_value(""))
            ("h,help", "Show help");

        // Allow analyses to add extra options
        if (extra_options_fn) {
            extra_options_fn(options);
        }

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

        // Let analyses parse extra options
        if (parse_extra_fn) {
            parse_extra_fn(args, config);
        }

        return config;
    }

    [[nodiscard]] static auto find_default_input_files(
        clas12::Target target, clas12::Polarity polarity
    ) -> std::vector<std::string> {
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
};

}  // namespace clas12::framework
