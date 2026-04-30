/**
 * @file batch.cpp
 * @brief SWIF2-based farm submission for CLAS12 analyses.
 *
 * Reads a farm TOML that points at an analysis TOML and declares SWIF2
 * resources + JLab paths, slices the input file list, synthesizes a per-job
 * TOML for each chunk (overriding `[run].input`/`[run].output`), and emits
 * a `batch_scripts/<workflow>.swif2.sh` that creates the workflow, adds one
 * job per chunk with explicit `-input` / `-output` staging, and runs it.
 *
 * Usage:
 *   ./batch <farm.toml>                # dry-run: synthesize scripts, do not submit
 *   ./batch <farm.toml> --submit       # actually run the swif2 script
 *   ./batch <farm.toml> --print-job 0  # print the synthesized TOML for job 0
 */

#include <cstdlib>
#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <toml++/toml.hpp>

#include <clas12/FileUtils.hpp>
#include <clas12/StringUtils.hpp>
#include <clas12/framework/core/BatchSystem.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace fs = std::filesystem;
using framework::Log;
using framework::BatchSystem;

// =============================================================================
// Farm TOML model
// =============================================================================
//
// One in-memory mirror of the farm TOML, populated by parse_farm_toml.
// Field names align with the TOML schema documented in
// docs/mkdocs/user-guide/batch-system.md.

struct FarmConfig {
    // [farm]
    std::string analysis_config;
    std::string executable;
    int         files_per_job = 10;
    std::vector<std::string> input_sources;   // /mss/, /cache/, file paths, dirs
    std::string output_dir;
    std::string log_dir;

    // [farm.swif2]
    BatchSystem::Swif2JobConfig swif2;
};

// =============================================================================
// Parsing
// =============================================================================

/// Read either a TOML string or array-of-strings into a vector. Both forms
/// are accepted everywhere a path-list is expected.
static auto parse_string_list(const toml::node_view<const toml::node>& node)
    -> std::vector<std::string>
{
    std::vector<std::string> out;
    if (auto arr = node.as_array()) {
        for (const auto& el : *arr) {
            if (auto s = el.value<std::string>()) out.push_back(*s);
        }
    } else if (auto s = node.value<std::string>()) {
        out.push_back(*s);
    }
    return out;
}

static auto parse_farm_toml(const std::string& path) -> std::optional<FarmConfig> {
    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        Log::error("batch", "Failed to parse {}: {}", path, e.description());
        return std::nullopt;
    }

    const auto* farm = tbl["farm"].as_table();
    if (!farm) {
        Log::error("batch", "{} missing required [farm] section", path);
        return std::nullopt;
    }

    FarmConfig cfg;

    if (auto v = (*farm)["analysis_config"].value<std::string>()) cfg.analysis_config = *v;
    if (auto v = (*farm)["executable"].value<std::string>())      cfg.executable      = *v;
    if (auto v = (*farm)["files_per_job"].value<int64_t>())       cfg.files_per_job   = static_cast<int>(*v);
    if (auto v = (*farm)["output_dir"].value<std::string>())      cfg.output_dir      = *v;
    if (auto v = (*farm)["log_dir"].value<std::string>())         cfg.log_dir         = *v;

    cfg.input_sources = parse_string_list((*farm)["inputs"]);

    // [farm.swif2]
    auto& s = cfg.swif2;
    s.executable           = cfg.executable;
    s.analysis_config_path = cfg.analysis_config;
    s.output_dir           = cfg.output_dir;
    s.log_dir              = cfg.log_dir;

    if (const auto* swif2 = (*farm)["swif2"].as_table()) {
        if (auto v = (*swif2)["workflow"].value<std::string>())   s.workflow   = *v;
        if (auto v = (*swif2)["site"].value<std::string>())       s.site       = *v;
        if (auto v = (*swif2)["account"].value<std::string>())    s.account    = *v;
        if (auto v = (*swif2)["partition"].value<std::string>())  s.partition  = *v;
        if (auto v = (*swif2)["constraint"].value<std::string>()) s.constraint = *v;
        if (auto v = (*swif2)["cores"].value<int64_t>())          s.cores      = static_cast<int>(*v);
        if (auto v = (*swif2)["ram_gb"].value<int64_t>())         s.ram_gb     = static_cast<int>(*v);
        if (auto v = (*swif2)["disk_gb"].value<int64_t>())        s.disk_gb    = static_cast<int>(*v);
        if (auto v = (*swif2)["time"].value<std::string>())       s.time       = *v;

        if (const auto* tags = (*swif2)["tags"].as_table()) {
            for (auto&& [k, val] : *tags) {
                if (auto sv = val.value<std::string>()) s.tags.emplace(std::string(k.str()), *sv);
            }
        }
        if (const auto* mods = (*swif2)["modules"].as_table()) {
            s.modules = parse_string_list((*mods)["load"]);
        }
    }

    if (const auto* env = (*farm)["env"].as_table()) {
        for (auto&& [k, val] : *env) {
            if (auto sv = val.value<std::string>()) s.env.emplace(std::string(k.str()), *sv);
        }
    }

    return cfg;
}

// =============================================================================
// Input expansion
// =============================================================================

/// Resolved view of `[farm].inputs`: the post-brace-expansion sources we
/// actually scanned, plus the final flat HIPO file list.
struct ResolvedInputs {
    std::vector<std::string> sources;  ///< after brace expansion, before scan
    std::vector<std::string> files;    ///< final flat list of .hipo files
};

/// Expand `[farm].inputs` to a final flat list of HIPO files.
///
/// Each raw entry first goes through brace expansion (e.g.
/// `/mss/.../recon/{018944,018945}` -> two paths). Then each expanded
/// result is dereferenced: directories are scanned recursively for `.hipo`,
/// explicit files are returned verbatim. Both intermediate and final lists
/// are returned so the dry-run summary can show the user exactly which
/// directories were reached.
static auto expand_inputs(const std::vector<std::string>& raw_sources)
    -> ResolvedInputs
{
    ResolvedInputs r;
    for (const auto& raw : raw_sources) {
        auto expanded = StringUtils::expand_braces(raw);
        for (auto& src : expanded) {
            r.sources.push_back(src);
            if (fs::is_directory(src)) {
                auto more = FileUtils::read_recursive_hipo_files(src);
                r.files.insert(r.files.end(), more.begin(), more.end());
            } else {
                r.files.push_back(std::move(src));
            }
        }
    }
    return r;
}

// =============================================================================
// Job synthesis
// =============================================================================

/// Build a single JobSpec for chunk index `idx`, writing the synthesized
/// per-job TOML to `<scripts_dir>/<workflow>_job_<idx>.toml` along the way.
///
/// Local SWIF2-staged names follow `input_<j>.hipo` and `result.root`,
/// matching the names baked into the per-job TOML.
static auto build_job(
    const FarmConfig&               farm,
    std::size_t                     idx,
    const std::vector<std::string>& chunk,
    const fs::path&                 scripts_dir
) -> std::optional<BatchSystem::JobSpec> {
    const std::string job_name = fmt::format("{}_job_{}", farm.swif2.workflow, idx);

    std::vector<std::string> local_input_names;
    BatchSystem::JobSpec     job;
    job.job_name = job_name;

    for (std::size_t j = 0; j < chunk.size(); ++j) {
        // Preserve the source extension so SWIF2 stages a file with the
        // right name; downstream HIPO readers don't actually care.
        std::string ext = fs::path(chunk[j]).extension().string();
        if (ext.empty()) ext = ".hipo";
        std::string local = fmt::format("input_{}{}", j, ext);
        local_input_names.push_back(local);
        job.inputs.emplace_back(local, chunk[j]);
    }

    const std::string local_output = "result.root";
    const std::string dest_output  = fmt::format("{}/{}/{}", farm.output_dir, job_name, local_output);
    const std::string dest_log     = fmt::format("{}/{}.log", farm.log_dir, job_name);
    job.outputs.emplace_back(local_output, dest_output);
    job.outputs.emplace_back("log.txt",    dest_log);

    // Synthesize the per-job TOML.
    std::string toml_text;
    try {
        toml_text = BatchSystem::synthesize_job_toml(
            farm.analysis_config,
            local_input_names,
            local_output,
            farm.swif2.cores
        );
    } catch (const std::exception& e) {
        Log::error("batch", "Failed to synthesize TOML for {}: {}", job_name, e.what());
        return std::nullopt;
    }

    const fs::path toml_path = scripts_dir / fmt::format("{}.toml", job_name);
    if (!BatchSystem::write_file(toml_path.string(), toml_text)) {
        Log::error("batch", "Failed to write {}", toml_path.string());
        return std::nullopt;
    }
    job.job_toml_path = fs::absolute(toml_path).string();
    return job;
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char* argv[]) {
    cxxopts::Options options("batch", "SWIF2-based farm submission for CLAS12 analyses");
    options.add_options()
        ("farm-toml",     "Path to the farm TOML (positional, required)",
            cxxopts::value<std::string>())
        ("submit",        "Actually run the swif2 script (default: dry-run)",
            cxxopts::value<bool>()->default_value("false"))
        ("files-per-job", "Override [farm].files_per_job",
            cxxopts::value<int>())
        ("workflow",      "Override [farm.swif2].workflow",
            cxxopts::value<std::string>())
        ("max-jobs",      "Synthesize at most N jobs (for testing)",
            cxxopts::value<int>())
        ("print-job",     "Print the synthesized TOML for job N and exit",
            cxxopts::value<int>())
        ("h,help",        "Show help");
    options.parse_positional({"farm-toml"});
    options.positional_help("<farm.toml>");

    auto args = options.parse(argc, argv);

    if (args.count("help") || !args.count("farm-toml")) {
        fmt::print("{}\n", options.help());
        fmt::print(
            "\nExample:\n"
            "  ./batch config/electron_analysis.farm.toml          # dry-run\n"
            "  ./batch config/electron_analysis.farm.toml --submit # submit\n\n");
        return args.count("help") ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    const std::string farm_toml_path = args["farm-toml"].as<std::string>();

    auto farm_opt = parse_farm_toml(farm_toml_path);
    if (!farm_opt) return EXIT_FAILURE;
    FarmConfig farm = *farm_opt;

    // CLI overrides — applied after TOML so they win on conflicts.
    if (args.count("files-per-job")) farm.files_per_job  = args["files-per-job"].as<int>();
    if (args.count("workflow"))      farm.swif2.workflow = args["workflow"].as<std::string>();

    // ---- Validation ---------------------------------------------------------

    if (farm.swif2.workflow.empty()) {
        Log::error("batch", "[farm.swif2].workflow is required");
        return EXIT_FAILURE;
    }
    if (farm.analysis_config.empty()) {
        Log::error("batch", "[farm].analysis_config is required");
        return EXIT_FAILURE;
    }
    if (!fs::exists(farm.analysis_config)) {
        Log::error("batch", "Analysis TOML not found: {}", farm.analysis_config);
        return EXIT_FAILURE;
    }
    if (farm.executable.empty()) {
        Log::error("batch", "[farm].executable is required");
        return EXIT_FAILURE;
    }
    if (farm.output_dir.empty() || farm.log_dir.empty()) {
        Log::error("batch", "[farm].output_dir and [farm].log_dir are required");
        return EXIT_FAILURE;
    }

    // Make the executable an absolute path so the SWIF2 job (which runs
    // in a fresh CWD on a worker node) can find it.
    if (fs::exists(farm.executable)) {
        farm.executable = fs::absolute(farm.executable).string();
        farm.swif2.executable = farm.executable;
    } else {
        Log::warning("batch",
            "Executable '{}' not found at submission host — assuming it exists on the worker node",
            farm.executable);
    }

    // ---- Input expansion ----------------------------------------------------

    if (farm.input_sources.empty()) {
        Log::error("batch", "[farm].inputs is empty");
        return EXIT_FAILURE;
    }
    auto resolved = expand_inputs(farm.input_sources);
    auto& inputs = resolved.files;
    if (inputs.empty()) {
        Log::error("batch", "No HIPO files found under [farm].inputs");
        return EXIT_FAILURE;
    }

    // ---- Slice into jobs ----------------------------------------------------

    auto chunks = BatchSystem::split_input_files(inputs, farm.files_per_job);
    if (args.count("max-jobs")) {
        const int n = args["max-jobs"].as<int>();
        if (n >= 0 && static_cast<std::size_t>(n) < chunks.size()) {
            chunks.resize(static_cast<std::size_t>(n));
        }
    }

    // ---- Synthesize per-job TOMLs + JobSpecs --------------------------------

    const fs::path scripts_dir = fs::absolute("./batch_scripts");
    fs::create_directories(scripts_dir);

    // Wrapper script — identical for every job of a workflow. Gets staged
    // into each job as `wrapper.sh` via SWIF2 -input.
    const fs::path wrapper_path = scripts_dir / fmt::format("{}_wrapper.sh", farm.swif2.workflow);
    farm.swif2.wrapper_script_path = wrapper_path.string();
    const std::string wrapper = BatchSystem::synthesize_wrapper_script(farm.swif2);
    if (!BatchSystem::write_file(wrapper_path.string(), wrapper)) {
        Log::error("batch", "Failed to write {}", wrapper_path.string());
        return EXIT_FAILURE;
    }
    BatchSystem::make_executable(wrapper_path.string());

    std::vector<BatchSystem::JobSpec> jobs;
    jobs.reserve(chunks.size());
    for (std::size_t i = 0; i < chunks.size(); ++i) {
        auto job = build_job(farm, i, chunks[i], scripts_dir);
        if (!job) return EXIT_FAILURE;
        jobs.push_back(std::move(*job));
    }

    // ---- --print-job N ------------------------------------------------------

    if (args.count("print-job")) {
        const int n = args["print-job"].as<int>();
        if (n < 0 || static_cast<std::size_t>(n) >= jobs.size()) {
            Log::error("batch", "--print-job={} out of range (have {} jobs)", n, jobs.size());
            return EXIT_FAILURE;
        }
        std::ifstream in(jobs[static_cast<std::size_t>(n)].job_toml_path);
        std::cout << in.rdbuf();
        return EXIT_SUCCESS;
    }

    // ---- Workflow script ----------------------------------------------------

    const std::string script = BatchSystem::synthesize_workflow_script(farm.swif2, jobs);
    const fs::path script_path = scripts_dir / fmt::format("{}.swif2.sh", farm.swif2.workflow);
    if (!BatchSystem::write_file(script_path.string(), script)) {
        Log::error("batch", "Failed to write {}", script_path.string());
        return EXIT_FAILURE;
    }
    BatchSystem::make_executable(script_path.string());

    // ---- Summary ------------------------------------------------------------

    fmt::print(
        "\n══════════════════════════════════════════════════════════════════\n"
        "  Workflow:        {}\n"
        "  Analysis TOML:   {}\n"
        "  Executable:      {}\n"
        "  Inputs found:    {}\n"
        "  Files per job:   {}\n"
        "  Jobs:            {}\n"
        "  Per-job TOMLs:   {}/{}_job_*.toml\n"
        "  Workflow script: {}\n"
        "══════════════════════════════════════════════════════════════════\n\n",
        farm.swif2.workflow,
        farm.analysis_config,
        farm.executable,
        inputs.size(),
        farm.files_per_job,
        jobs.size(),
        scripts_dir.string(),
        farm.swif2.workflow,
        script_path.string()
    );

    // Show the post-brace-expansion source list (what we actually scanned)
    // and a small sample of the resolved files. Makes brace-expansion +
    // recursive-scan results visible at a glance.
    fmt::print("[farm].inputs after brace expansion ({} source path{}):\n",
               resolved.sources.size(),
               resolved.sources.size() == 1 ? "" : "s");
    {
        const std::size_t cap = 8;
        for (std::size_t i = 0; i < std::min(cap, resolved.sources.size()); ++i) {
            fmt::print("  {}\n", resolved.sources[i]);
        }
        if (resolved.sources.size() > cap) {
            fmt::print("  ... ({} more)\n", resolved.sources.size() - cap);
        }
    }
    fmt::print("Sample of resolved HIPO files:\n");
    {
        const std::size_t cap = 3;
        for (std::size_t i = 0; i < std::min(cap, inputs.size()); ++i) {
            fmt::print("  {}\n", inputs[i]);
        }
        if (inputs.size() > 2 * cap) {
            fmt::print("  ... ({} more) ...\n", inputs.size() - 2 * cap);
            for (std::size_t i = inputs.size() - cap; i < inputs.size(); ++i) {
                fmt::print("  {}\n", inputs[i]);
            }
        }
    }
    fmt::print("\n");

    if (!jobs.empty()) {
        fmt::print("Sample synthesized TOML ({}):\n", jobs.front().job_toml_path);
        fmt::print("──────────────────────────────────────────────────────────────────\n");
        std::ifstream in(jobs.front().job_toml_path);
        std::cout << in.rdbuf();
        fmt::print("──────────────────────────────────────────────────────────────────\n\n");
    }

    // ---- Submit or print instructions --------------------------------------

    const bool do_submit = args["submit"].as<bool>();
    if (!do_submit) {
        fmt::print(
            "Dry-run only. To submit:\n"
            "    ./batch {} --submit\n"
            "or run the script directly:\n"
            "    bash {}\n\n",
            farm_toml_path,
            script_path.string()
        );
        return EXIT_SUCCESS;
    }

    fmt::print("Running swif2 workflow script...\n");
    int rc = BatchSystem::submit_workflow(script_path.string());
    if (rc != 0) {
        Log::error("batch", "swif2 script exited with status {}", rc);
        return EXIT_FAILURE;
    }
    fmt::print(
        "\nSubmitted. Monitor with:\n"
        "    swif2 status -workflow {}\n"
        "    swif2 list-jobs -workflow {} -display all\n\n"
        "When jobs finish, merge outputs with:\n"
        "    hadd -f merged_{}.root {}/*/result.root\n\n",
        farm.swif2.workflow, farm.swif2.workflow,
        farm.swif2.workflow, farm.output_dir
    );
    return EXIT_SUCCESS;
}
