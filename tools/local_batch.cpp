/**
 * @file local_batch.cpp
 * @brief Local (non-SWIF2) batch driver — split one big HIPO file across N
 *        concurrent processes by record range.
 *
 * Useful when a single hipo file is large enough that single-process
 * throughput plateaus on mutex contention (NTupleService::fill, ROOT
 * histogram atomic stores, etc.). Slicing the file by record range and
 * running M processes in parallel lets each process keep its own
 * thread-team busy without fighting any single global lock.
 *
 * The driver:
 *   1. Reads the analysis TOML (`--config`) to discover the input file,
 *      base output filename, and the analysis executable to run.
 *   2. Opens the input file once with `hipo::reader` to read its
 *      `getNRecords()`, then chops `[0, NRecords)` into `--slices` equal
 *      half-open record slices.
 *   3. Launches the analysis executable for each slice with
 *      `--record-range FIRST:LAST` and `--output <base>_slice<i>.root`,
 *      keeping at most `--concurrent` subprocesses running at a time.
 *   4. Waits for everything, prints a status summary and the hadd command
 *      to merge per-slice outputs.
 *
 * The analysis binary must be the same TOML-driven entry point used in
 * normal runs — `AnalysisRunner::run` honours `--record-range` and
 * `--output` overrides without any per-analysis code changes.
 *
 * Usage:
 *   ./local_batch \
 *       --exe        ./build/analyses/pi0_multiplicity \
 *       --config     config/pi0_multiplicity.toml \
 *       --slices     8 \
 *       --concurrent 4
 */

#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <cxxopts.hpp>
#include <fmt/format.h>
#include <toml++/toml.hpp>

#include <reader.h>

namespace fs = std::filesystem;

namespace {

struct DriverConfig {
    std::string analysis_toml;
    std::string executable;
    int         slices       = 0;
    int         concurrent   = 0;
    bool        no_run       = false;
    std::string output_dir;          // optional: overrides what's in the TOML
    std::string slice_suffix = "_slice"; // <output_base>_slice<i>.root
};

struct AnalysisTomlInfo {
    std::string input_file;          // first input file (driver only supports one)
    std::string output_base;         // output basename from [run].output
    std::string hipo_output_base;    // optional [skim].hipo_output base; empty = none
};

[[nodiscard]] auto load_analysis_toml(const std::string& path)
    -> std::optional<AnalysisTomlInfo>
{
    toml::table tbl;
    try {
        tbl = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        fmt::print(stderr, "[local_batch] Failed to parse {}: {}\n",
                   path, e.description());
        return std::nullopt;
    }

    const auto* run = tbl["run"].as_table();
    if (!run) {
        fmt::print(stderr, "[local_batch] {} missing [run] section\n", path);
        return std::nullopt;
    }

    AnalysisTomlInfo info;
    if (auto v = (*run)["output"].value<std::string>()) info.output_base = *v;
    // Optional skim-specific HIPO output: pi0_multiplicity_skim and any
    // future skim binary that consults `[skim].hipo_output` will pick this
    // up via the `--hipo-output` CLI override.
    if (auto v = tbl["skim"]["hipo_output"].value<std::string>()) info.hipo_output_base = *v;

    // [run].input may be a string or a one-element array; record-range
    // slicing is well-defined only for a single file.
    if (auto arr = (*run)["input"].as_array()) {
        if (arr->size() != 1) {
            fmt::print(stderr,
                "[local_batch] [run].input has {} entries; slicing requires "
                "exactly one input file\n", arr->size());
            return std::nullopt;
        }
        if (auto s = (*arr)[0].value<std::string>()) info.input_file = *s;
    } else if (auto s = (*run)["input"].value<std::string>()) {
        info.input_file = *s;
    }
    if (info.input_file.empty()) {
        fmt::print(stderr, "[local_batch] [run].input is empty\n");
        return std::nullopt;
    }
    if (info.output_base.empty()) {
        fmt::print(stderr, "[local_batch] [run].output is empty\n");
        return std::nullopt;
    }
    return info;
}

/// Insert `<suffix><i>` before the `.root`/`.hipo` extension. Falls back
/// to a plain suffix append if the basename has no extension.
[[nodiscard]] auto make_slice_output(
    const std::string& output_base,
    const std::string& dir_override,
    const std::string& suffix,
    int idx) -> std::string
{
    fs::path base{output_base};
    if (!dir_override.empty()) base = fs::path(dir_override) / base.filename();
    const std::string stem = base.stem().string();
    const std::string ext  = base.extension().string();
    const std::string name = fmt::format("{}{}{:03d}{}", stem, suffix, idx, ext);
    return (base.parent_path() / name).string();
}

[[nodiscard]] auto probe_num_records(const std::string& input_file) -> int {
    hipo::reader r;
    r.open(input_file.c_str());
    if (!r.is_open()) {
        fmt::print(stderr, "[local_batch] Failed to open {}\n", input_file);
        return -1;
    }
    return r.getNRecords();
}

struct SliceResult {
    int slice;
    int first, last;
    int exit_code;
    long ms;
    std::string output_file;
};

/// fork/exec one slice; blocks until the child exits. Stdio is left
/// connected to the parent so progress lines stream out interleaved.
///
/// `hipo_output_file` is forwarded as `--hipo-output FILE` only when
/// non-empty — analysis binaries that don't honour that flag (the
/// non-skim ones) ignore it harmlessly via cxxopts' unknown-option
/// tolerance is not assumed; we just skip the argument instead.
[[nodiscard]] auto run_one_slice(
    const std::string& exe,
    const std::string& analysis_toml,
    int first, int last,
    const std::string& output_file,
    const std::string& hipo_output_file) -> int
{
    const std::string range_arg = fmt::format("{}:{}", first, last);

    pid_t pid = fork();
    if (pid < 0) {
        fmt::print(stderr, "[local_batch] fork() failed\n");
        return 127;
    }
    if (pid == 0) {
        // Build argv at runtime so `--hipo-output` only gets passed for
        // slices that actually have a skim output to write.
        std::vector<const char*> argv;
        argv.push_back(exe.c_str());
        argv.push_back("--config");       argv.push_back(analysis_toml.c_str());
        argv.push_back("--record-range"); argv.push_back(range_arg.c_str());
        argv.push_back("--output");       argv.push_back(output_file.c_str());
        if (!hipo_output_file.empty()) {
            argv.push_back("--hipo-output");
            argv.push_back(hipo_output_file.c_str());
        }
        argv.push_back(nullptr);
        execv(exe.c_str(), const_cast<char* const*>(argv.data()));
        // Only reached on exec failure.
        fmt::print(stderr, "[local_batch] execv('{}') failed: {}\n",
                   exe, std::strerror(errno));
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        fmt::print(stderr, "[local_batch] waitpid() failed for slice\n");
        return 127;
    }
    if (WIFEXITED(status))   return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 127;
}

} // namespace

int main(int argc, char* argv[]) {
    cxxopts::Options opts("local_batch",
        "Local (non-SWIF2) batch driver — split one big HIPO file across N "
        "concurrent processes by record range.");
    opts.add_options()
        ("c,config",     "Analysis TOML (same one the analysis binary takes)",
            cxxopts::value<std::string>())
        ("exe",          "Path to the analysis executable",
            cxxopts::value<std::string>())
        ("slices",       "Number of record slices to split into",
            cxxopts::value<int>()->default_value("8"))
        ("concurrent",   "Max number of slices to run at once",
            cxxopts::value<int>()->default_value("4"))
        ("output-dir",   "Directory to write per-slice outputs into "
                         "(default: same dir as [run].output)",
            cxxopts::value<std::string>())
        ("dry-run",      "Print the planned slice table without launching",
            cxxopts::value<bool>()->default_value("false"))
        ("h,help",       "Show help");

    auto args = opts.parse(argc, argv);
    if (args.count("help") || !args.count("config") || !args.count("exe")) {
        fmt::print("{}\n", opts.help());
        return args.count("help") ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    DriverConfig cfg;
    cfg.analysis_toml = args["config"].as<std::string>();
    cfg.executable    = args["exe"].as<std::string>();
    cfg.slices        = args["slices"].as<int>();
    cfg.concurrent    = args["concurrent"].as<int>();
    if (args.count("output-dir")) cfg.output_dir = args["output-dir"].as<std::string>();
    cfg.no_run        = args["dry-run"].as<bool>();

    if (cfg.slices < 1 || cfg.concurrent < 1) {
        fmt::print(stderr,
            "[local_batch] --slices and --concurrent must both be >= 1\n");
        return EXIT_FAILURE;
    }

    if (!fs::exists(cfg.executable)) {
        fmt::print(stderr, "[local_batch] Executable not found: {}\n",
                   cfg.executable);
        return EXIT_FAILURE;
    }
    cfg.executable = fs::absolute(cfg.executable).string();

    if (!fs::exists(cfg.analysis_toml)) {
        fmt::print(stderr, "[local_batch] Analysis TOML not found: {}\n",
                   cfg.analysis_toml);
        return EXIT_FAILURE;
    }
    cfg.analysis_toml = fs::absolute(cfg.analysis_toml).string();

    auto info_opt = load_analysis_toml(cfg.analysis_toml);
    if (!info_opt) return EXIT_FAILURE;
    auto& info = *info_opt;

    fmt::print("[local_batch] Probing record count of {} ...\n",
               info.input_file);
    const int nrec = probe_num_records(info.input_file);
    if (nrec <= 0) {
        fmt::print(stderr,
            "[local_batch] Could not get a positive record count "
            "(got {})\n", nrec);
        return EXIT_FAILURE;
    }

    // Slice into nearly-equal half-open ranges. The last slice picks up
    // any remainder from integer division.
    std::vector<std::pair<int, int>> ranges;
    ranges.reserve(static_cast<std::size_t>(cfg.slices));
    const int per = nrec / cfg.slices;
    int cursor = 0;
    for (int i = 0; i < cfg.slices; ++i) {
        const int first = cursor;
        const int last  = (i == cfg.slices - 1) ? nrec : (cursor + per);
        ranges.emplace_back(first, last);
        cursor = last;
    }

    fmt::print(
        "\n══════════════════════════════════════════════════════════════════\n"
        "  Local batch — record-range slicing\n"
        "──────────────────────────────────────────────────────────────────\n"
        "  Analysis TOML:   {}\n"
        "  Executable:      {}\n"
        "  Input file:      {}\n"
        "  Records total:   {}\n"
        "  Slices:          {}\n"
        "  Max concurrent:  {}\n"
        "══════════════════════════════════════════════════════════════════\n\n",
        cfg.analysis_toml, cfg.executable, info.input_file,
        nrec, cfg.slices, cfg.concurrent);

    if (!cfg.output_dir.empty()) {
        fs::create_directories(cfg.output_dir);
    }

    fmt::print("Planned slices:\n");
    std::vector<std::string> outputs(static_cast<std::size_t>(cfg.slices));
    std::vector<std::string> hipo_outputs(static_cast<std::size_t>(cfg.slices));
    const bool has_hipo_out = !info.hipo_output_base.empty();
    for (int i = 0; i < cfg.slices; ++i) {
        outputs[static_cast<std::size_t>(i)] = make_slice_output(
            info.output_base, cfg.output_dir, cfg.slice_suffix, i);
        if (has_hipo_out) {
            hipo_outputs[static_cast<std::size_t>(i)] = make_slice_output(
                info.hipo_output_base, cfg.output_dir, cfg.slice_suffix, i);
        }
        if (has_hipo_out) {
            fmt::print("  slice {:03d}: records [{:>7d}, {:>7d}) -> {}  +  {}\n",
                       i, ranges[i].first, ranges[i].second,
                       outputs[static_cast<std::size_t>(i)],
                       hipo_outputs[static_cast<std::size_t>(i)]);
        } else {
            fmt::print("  slice {:03d}: records [{:>7d}, {:>7d}) -> {}\n",
                       i, ranges[i].first, ranges[i].second,
                       outputs[static_cast<std::size_t>(i)]);
        }
    }
    fmt::print("\n");

    if (cfg.no_run) {
        fmt::print("--dry-run: not launching any slices.\n");
        return EXIT_SUCCESS;
    }

    // ---- Run with bounded concurrency --------------------------------------
    //
    // Simple semaphore-style pool: spawn one std::thread per slice but
    // gate them through a counting condition variable so at most
    // `cfg.concurrent` are in their fork+wait at any one time.

    std::vector<SliceResult> results(static_cast<std::size_t>(cfg.slices));
    std::mutex mu;
    std::condition_variable cv;
    int in_flight = 0;

    auto wall_start = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(cfg.slices));
    for (int i = 0; i < cfg.slices; ++i) {
        workers.emplace_back([&, i]() {
            {
                std::unique_lock<std::mutex> lk(mu);
                cv.wait(lk, [&]{ return in_flight < cfg.concurrent; });
                ++in_flight;
            }
            const auto t0 = std::chrono::steady_clock::now();
            const int rc = run_one_slice(
                cfg.executable, cfg.analysis_toml,
                ranges[i].first, ranges[i].second,
                outputs[static_cast<std::size_t>(i)],
                hipo_outputs[static_cast<std::size_t>(i)]);
            const auto t1 = std::chrono::steady_clock::now();
            const long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            results[static_cast<std::size_t>(i)] = {
                i, ranges[i].first, ranges[i].second, rc, ms,
                outputs[static_cast<std::size_t>(i)]
            };
            {
                std::lock_guard<std::mutex> lk(mu);
                --in_flight;
                fmt::print("[local_batch] slice {:03d} done  rc={}  {} ms\n",
                           i, rc, ms);
            }
            cv.notify_all();
        });
    }
    for (auto& t : workers) t.join();

    const auto wall_end = std::chrono::steady_clock::now();
    const long wall_ms = std::chrono::duration_cast<std::chrono::milliseconds>(wall_end - wall_start).count();

    // ---- Summary -----------------------------------------------------------

    int failures = 0;
    for (const auto& r : results) if (r.exit_code != 0) ++failures;

    fmt::print(
        "\n══════════════════════════════════════════════════════════════════\n"
        "  Summary\n"
        "──────────────────────────────────────────────────────────────────\n"
        "  Wall time:       {:.1f} s\n"
        "  Slices:          {} ({} OK, {} failed)\n"
        "══════════════════════════════════════════════════════════════════\n\n",
        wall_ms / 1000.0, results.size(),
        static_cast<int>(results.size()) - failures, failures);

    if (failures == 0) {
        fmt::print("All slices succeeded. To merge:\n");
        const fs::path base{info.output_base};
        fs::path merged = cfg.output_dir.empty()
            ? base
            : (fs::path(cfg.output_dir) / base.filename());
        fmt::print("    hadd -f {} ", merged.string());
        for (const auto& r : results) fmt::print("{} ", r.output_file);
        fmt::print("\n");

        if (has_hipo_out) {
            // hipo-utils ships a `hipo-merger` (or `hipo-utils -merge`)
            // helper for the .hipo case. The user picks whichever they
            // have in PATH; we just emit the slice list.
            const fs::path hipo_base{info.hipo_output_base};
            fs::path merged_hipo = cfg.output_dir.empty()
                ? hipo_base
                : (fs::path(cfg.output_dir) / hipo_base.filename());
            fmt::print("    # skim HIPO output — merge with hipo-utils:\n");
            fmt::print("    hipo-utils -merge -o {} ", merged_hipo.string());
            for (std::size_t i = 0; i < hipo_outputs.size(); ++i) {
                fmt::print("{} ", hipo_outputs[i]);
            }
            fmt::print("\n");
        }
        fmt::print("\n");
    }

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
