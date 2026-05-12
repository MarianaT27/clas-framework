#pragma once

/**
 * @file AnalysisRunner.hpp
 * @brief Single-flag entry point for analyses: `--config <path.toml>`.
 *
 * Every knob lives in TOML now — input files, threads, output ROOT
 * file, percentage, target, polarity, and per-algorithm `Property`
 * values. The user's `main()` only needs to declare *which* algorithms
 * + services compose this analysis; the runner loads the TOML, builds
 * the Application, applies the per-algorithm `[<Algorithm.name()>]`
 * section to each algorithm's properties, and runs the event loop.
 *
 * Example user main():
 * @code
 * #include <clas12/framework/core/AnalysisRunner.hpp>
 * #include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
 *
 * auto main(int argc, char* argv[]) -> int {
 *     return framework::AnalysisRunner::run(
 *         argc, argv, "Electron Analysis", "electron_analysis",
 *         [](auto& builder, const auto& config) {
 *             builder
 *                 .add_service<ElectronCutsService>()
 *                 .add_algorithm<EventMonitor>("EventMonitor")
 *                 .add_algorithm<ElectronSelectionAlg>(
 *                     "Selection", config.target, config.polarity);
 *         });
 * }
 * @endcode
 *
 * Example invocation:
 * @code
 *   ./electron_analysis --config config/electron_analysis.toml
 * @endcode
 *
 * Required TOML schema:
 * @code
 *   [run]
 *   input       = ["data/run_5032.hipo"]   # list, or single string
 *   output      = "electron_analysis.root"
 *   num_threads = 4                        # optional, default 1
 *   percentage  = 100.0                    # optional, default 100
 *
 *   [analysis]
 *   target      = "LD2"                    # one of: LD2, CxC, CuSn, Cu, Sn
 *   polarity    = "Outbending"             # Outbending or Inbending
 *   beam_energy = 10.53                    # optional
 *   version     = "2.0"                    # optional
 *
 *   # Per-algorithm Property values; section name = algorithm name.
 *   [ElectronAnalysis]
 *   enable_chi2pid_cut = true
 *   ...
 * @endcode
 */

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <memory>
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
#include <clas12/framework/services/WorkflowVisualizationService.hpp>
#include <ioOptions.h>

namespace framework {

/**
 * @brief Parse a `[IO]` TOML table into a hipo::IOOptions.
 *
 * Recognised keys (all optional):
 *   * prefetch_depth     (int)   default 0 = legacy synchronous reads
 *   * record_cache_bytes (int)   default 0 = no LRU record cache
 *                                accepts a plain int (bytes) or any of
 *                                "<N>KB" / "<N>MB" / "<N>GB" when given
 *                                as a string.
 *   * io_threads         (int)   default 0 = auto (1 local / 2 remote)
 *   * network_read_chunk (int)   default 0 = backend-picked
 *   * bank_filter        (array of strings)  CPU-side bank whitelist
 *
 * Unrecognised keys are silently ignored so old / new TOMLs co-exist.
 */
inline auto io_options_from_toml(const toml::table& root) -> hipo::IOOptions {
    hipo::IOOptions opts;
    const auto* io = root["IO"].as_table();
    if (!io) return opts;

    if (auto v = (*io)["prefetch_depth"].value<int64_t>())     opts.prefetch_depth = static_cast<int>(*v);
    if (auto v = (*io)["io_threads"].value<int64_t>())         opts.io_threads     = static_cast<int>(*v);
    if (auto v = (*io)["network_read_chunk"].value<int64_t>()) opts.network_read_chunk = static_cast<int>(*v);

    // record_cache_bytes accepts either a plain integer count of bytes
    // or a string with a "KB" / "MB" / "GB" suffix for ergonomic configs.
    if (auto v = (*io)["record_cache_bytes"].value<int64_t>()) {
        opts.record_cache_bytes = static_cast<std::size_t>(*v);
    } else if (auto s = (*io)["record_cache_bytes"].value<std::string>()) {
        std::string text = *s;
        std::size_t mul = 1;
        auto ends_with = [&](std::string_view suf) {
            return text.size() >= suf.size() &&
                   std::equal(suf.rbegin(), suf.rend(), text.rbegin(),
                              [](char a, char b){ return std::tolower(a) == std::tolower(b); });
        };
        if (ends_with("kb"))      { mul = 1024ULL;                   text.resize(text.size() - 2); }
        else if (ends_with("mb")) { mul = 1024ULL * 1024;            text.resize(text.size() - 2); }
        else if (ends_with("gb")) { mul = 1024ULL * 1024 * 1024;     text.resize(text.size() - 2); }
        try {
            opts.record_cache_bytes = static_cast<std::size_t>(std::stoll(text)) * mul;
        } catch (...) {
            // ignore malformed entry; leaves the cache disabled
        }
    }

    if (auto arr = (*io)["bank_filter"].as_array()) {
        for (auto& el : *arr) {
            if (auto s = el.value<std::string>()) opts.bank_filter.push_back(*s);
        }
    }

    return opts;
}

/**
 * @brief Run-control + analysis-level config parsed from the TOML file.
 *
 * Per-algorithm Property values are NOT in this struct — they go
 * straight into each algorithm's PropertyMixin via the parsed
 * `toml::table` carried in `config_table`. The fields here are the
 * ones the user's `configure_fn` lambda reads to construct
 * algorithms (target/polarity/beam_energy).
 */
struct AnalysisConfig {
    Target           target = Target::LD2;
    Polarity         polarity = Polarity::Outbending;
    double                   percentage  = 100.0;
    int                      num_threads = 1;
    double                   beam_energy = 10.53;
    std::string              version     = "2.0";
    std::string              output_file;
    std::vector<std::string> input_files;
    /// Optional half-open record slice `[first, last)`. Set via the
    /// `[run].record_range = [first, last]` TOML key or the `--record-range
    /// first:last` CLI override. Default (0, -1) processes every record.
    int                      record_first = 0;
    int                      record_last  = -1;
    /// Optional per-analysis HIPO-output override exposed to skim binaries
    /// that consult an analysis-specific TOML key (e.g.
    /// `[skim].hipo_output`). Set via the `--hipo-output FILE` CLI flag —
    /// empty string means "fall back to whatever the analysis lambda parses
    /// out of the TOML". Lets local_batch slice a single big HIPO file
    /// across N processes by writing a distinct skim output per slice.
    std::string              hipo_output_override;

    /// Whole parsed TOML, kept around so per-algorithm sections are
    /// available to Application::initialize().
    std::shared_ptr<const toml::table> config_table;

    [[nodiscard]] auto validate() const -> std::optional<std::string> {
        if (input_files.empty()) return std::string{"no input files in [run].input"};
        if (output_file.empty()) return std::string{"empty [run].output"};
        if (!(percentage > 0.0 && percentage <= 100.0))
            return "[run].percentage must be in (0, 100], got " + std::to_string(percentage);
        if (num_threads < 1)
            return "[run].num_threads must be >= 1, got " + std::to_string(num_threads);
        const std::filesystem::path out_path{output_file};
        const auto ext = out_path.extension().string();
        if (ext != ".root" && ext != ".hipo")
            return "[run].output must end in .root or .hipo, got '" + output_file + "'";
        return std::nullopt;
    }
};

/**
 * @brief Runs the standard CLI → TOML → Application lifecycle.
 *
 * The user supplies a single `configure_fn` lambda that registers
 * services and algorithms on the builder. Everything else (CLI,
 * TOML parsing, lifecycle, Property wiring) is handled here.
 */
class AnalysisRunner {
public:
    using ConfigureFn = std::function<void(ApplicationBuilder&, const AnalysisConfig&)>;

    /**
     * @param argc           argv from main()
     * @param argv           argv from main()
     * @param analysis_name  human-readable name used in log banners
     * @param output_prefix  prefix for the GraphViz dump
     *                       ("<prefix>_service_graph.dot")
     * @param configure_fn   lambda that adds services + algorithms
     *                       to the builder using config.target etc.
     */
    static auto run(
        int argc, char* argv[],
        const std::string& analysis_name,
        const std::string& output_prefix,
        ConfigureFn configure_fn
    ) -> int {
        ROOT::EnableThreadSafety();
        ROOT::EnableImplicitMT();

        auto config = parse_arguments(argc, argv, analysis_name, output_prefix);
        if (!config) return EXIT_SUCCESS;  // --help path

        if (auto err = config->validate(); err) {
            Log::error("Main", "Invalid configuration: {}", *err);
            return EXIT_FAILURE;
        }

        auto builder = make_application(analysis_name)
            .with_input_files(config->input_files)
            .with_output(config->output_file)
            .with_percentage(config->percentage)
            .with_threads(config->num_threads)
            .with_standard_services()
            .with_version(config->version)
            .with_beam_energy(config->beam_energy)
            .with_target(to_string(config->target))
            .with_config(config->config_table);

        if (config->config_table) {
            builder.with_io_options(io_options_from_toml(*config->config_table));
        }

        builder.with_record_range(config->record_first, config->record_last);

        configure_fn(builder, *config);

        auto app = builder.build();

        if (auto sc = app.initialize(); sc.is_failure()) {
            Log::error("Main", "Initialization failed: {}", sc.message());
            return EXIT_FAILURE;
        }

        emit_service_graph(app, output_prefix);

        Log::section(analysis_name);
        Log::config("Config", "TOML",       config_path_for_log());
        Log::config("Config", "Target",     to_string(config->target));
        Log::config("Config", "Polarity",   to_string(config->polarity));
        Log::config("Config", "Input files", config->input_files.size());
        Log::config("Config", "Threads",    config->num_threads);
        Log::config("Config", "Output",     config->output_file);
        Log::blank();

        {
            auto timer = Log::scoped("Main", "Event processing");
            if (auto sc = app.run(); sc.is_failure()) {
                Log::error("Main", "Processing failed: {}", sc.message());
                return EXIT_FAILURE;
            }
        }

        auto result = app.finalize();
        if (result.is_failure()) {
            Log::error("Main", "Finalization failed: {}", result.message());
            return EXIT_FAILURE;
        }

        Log::success("Main", "Analysis complete: {}", config->output_file);
        return EXIT_SUCCESS;
    }

private:
    static inline std::string s_config_path;

    static auto config_path_for_log() -> const std::string& { return s_config_path; }

    static auto emit_service_graph(Application& app, const std::string& output_prefix) -> void {
        WorkflowVisualizationService viz;
        auto& mgr = app.service_manager();
        for (const auto& name : mgr.service_names()) {
            viz.register_algorithm(name, {}, WorkflowVisualizationService::NodeType::SERVICE);
        }
        std::string previous;
        for (const auto& alg : app.algorithms()) {
            const auto& name = alg->name();
            if (previous.empty()) viz.register_algorithm(name);
            else                  viz.register_algorithm(name, {previous});
            previous = name;
        }
        const std::string dot_path = output_prefix + "_service_graph.dot";
        if (!viz.generate_dot(dot_path)) {
            Log::warning("Main", "Could not emit service graph to {}", dot_path);
        } else {
            Log::info("Main", "Service graph written to {}", dot_path);
        }
    }

    [[nodiscard]] static auto parse_arguments(
        int argc, char* argv[],
        const std::string& analysis_name,
        const std::string& output_prefix
    ) -> std::optional<AnalysisConfig> {
        cxxopts::Options options(output_prefix, analysis_name);
        options.add_options()
            ("c,config", "Path to TOML configuration file (required)",
                cxxopts::value<std::string>())
            ("record-range",
                "Override [run].record_range with FIRST:LAST (half-open). "
                "LAST=-1 means \"to end of file\". Used by the local batch driver "
                "to slice one big file across N processes.",
                cxxopts::value<std::string>())
            ("output",
                "Override [run].output (per-slice output filenames).",
                cxxopts::value<std::string>())
            ("hipo-output",
                "Override an analysis-specific HIPO output path (e.g. the "
                "[skim].hipo_output key of pi0_multiplicity_skim). Empty / "
                "absent = use the TOML value.",
                cxxopts::value<std::string>())
            ("h,help",   "Show help");

        auto args = options.parse(argc, argv);

        if (args.count("help") || !args.count("config")) {
            fmt::print("{}\n", options.help());
            if (!args.count("help")) {
                fmt::print(stderr,
                    "\nERROR: --config <path> is required.\n"
                    "Pass a TOML file with [run], [analysis], and per-algorithm sections.\n");
                return std::nullopt;  // caller treats as success-with-help; we'll fail later via validate()
            }
            return std::nullopt;
        }

        const auto path = args["config"].as<std::string>();
        s_config_path = path;

        toml::table parsed;
        try {
            parsed = toml::parse_file(path);
        } catch (const toml::parse_error& e) {
            Log::error("Main", "Failed to parse TOML '{}': {}", path, e.description());
            return std::nullopt;
        }

        AnalysisConfig cfg;
        cfg.config_table = std::make_shared<const toml::table>(std::move(parsed));
        const auto& tbl = *cfg.config_table;

        // [run]
        if (auto run_tbl = tbl["run"].as_table()) {
            if (auto v = (*run_tbl)["output"].value<std::string>()) cfg.output_file = *v;
            if (auto v = (*run_tbl)["num_threads"].value<int64_t>()) cfg.num_threads = static_cast<int>(*v);
            if (auto v = (*run_tbl)["percentage"].value<double>())   cfg.percentage = *v;
            cfg.input_files = parse_input(run_tbl);
            // [run].record_range = [first, last]   (half-open, last < 0 = "to end")
            if (auto arr = (*run_tbl)["record_range"].as_array(); arr && arr->size() == 2) {
                if (auto f = (*arr)[0].value<int64_t>()) cfg.record_first = static_cast<int>(*f);
                if (auto l = (*arr)[1].value<int64_t>()) cfg.record_last  = static_cast<int>(*l);
            }
        } else {
            Log::error("Main", "TOML missing required [run] section");
            return std::nullopt;
        }

        // CLI overrides win over TOML so the batch driver can launch one
        // process per slice without rewriting the TOML.
        if (args.count("record-range")) {
            const auto spec = args["record-range"].as<std::string>();
            const auto colon = spec.find(':');
            if (colon == std::string::npos) {
                Log::error("Main",
                    "--record-range expects FIRST:LAST, got '{}'", spec);
                return std::nullopt;
            }
            try {
                cfg.record_first = std::stoi(spec.substr(0, colon));
                cfg.record_last  = std::stoi(spec.substr(colon + 1));
            } catch (const std::exception& e) {
                Log::error("Main", "Bad --record-range '{}': {}", spec, e.what());
                return std::nullopt;
            }
        }
        if (args.count("output")) {
            cfg.output_file = args["output"].as<std::string>();
        }
        if (args.count("hipo-output")) {
            cfg.hipo_output_override = args["hipo-output"].as<std::string>();
        }

        // [analysis]
        if (auto an_tbl = tbl["analysis"].as_table()) {
            if (auto v = (*an_tbl)["target"].value<std::string>())
                cfg.target = string_to_target(*v);
            if (auto v = (*an_tbl)["polarity"].value<std::string>())
                cfg.polarity = string_to_polarity(*v);
            if (auto v = (*an_tbl)["beam_energy"].value<double>())   cfg.beam_energy = *v;
            if (auto v = (*an_tbl)["version"].value<std::string>())  cfg.version = *v;
        }

        return cfg;
    }

    /// `[run].input` may be either a single string (path or directory) or
    /// an array of strings. Three convenience expansions are applied:
    ///
    ///   1. Brace-expansion `{a,b,c}` (StringUtils::expand_braces) lets
    ///      analysts write `/mss/.../recon/{018944,018945,018946}` to mean
    ///      three sibling paths.
    ///   2. A path that resolves to a directory is recursively scanned for
    ///      `.hipo` files (FileUtils::read_recursive_hipo_files).
    ///   3. A comma-separated string is split into its parts.
    ///
    /// Anything else is treated as a literal file path and passed through.
    [[nodiscard]] static auto parse_input(const toml::table* run_tbl)
        -> std::vector<std::string>
    {
        std::vector<std::string> out;
        if (!run_tbl) return out;

        auto add_resolved = [&](const std::string& s) {
            if (s.empty()) return;
            if (std::filesystem::is_directory(s)) {
                auto more = FileUtils::read_recursive_hipo_files(s);
                out.insert(out.end(), more.begin(), more.end());
            } else if (s.find(',') != std::string::npos) {
                for (auto& part : StringUtils::split(s, ',')) {
                    if (!part.empty()) out.push_back(part);
                }
            } else {
                out.push_back(s);
            }
        };

        auto add_one = [&](const std::string& raw) {
            for (auto& expanded : StringUtils::expand_braces(raw)) {
                add_resolved(expanded);
            }
        };

        if (auto arr = (*run_tbl)["input"].as_array()) {
            for (auto const& el : *arr) {
                if (auto s = el.value<std::string>()) add_one(*s);
            }
        } else if (auto s = (*run_tbl)["input"].value<std::string>()) {
            add_one(*s);
        }
        return out;
    }
};

}  // namespace framework
