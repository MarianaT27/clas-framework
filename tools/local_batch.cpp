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

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <glob.h>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <cxxopts.hpp>
#include <fmt/format.h>
#include <toml++/toml.hpp>

#include <progresstracker.hpp>
#include <reader.h>
#include <clas12/framework/core/PerformanceMonitor.hpp>  // header-only perf:: helpers

namespace fs = std::filesystem;

namespace {

struct DriverConfig {
    std::string analysis_toml;
    std::string executable;
    int         slices       = 0;
    int         concurrent   = 0;
    bool        no_run       = false;
    bool        automerge    = false; // run hadd (and hipo-utils -merge) on success
    bool        profile      = false; // pass --profile to slices + aggregate the per-step timing
    std::string output_dir;          // optional: overrides what's in the TOML
    std::string slice_suffix = "_slice"; // <output_base>_slice<i>.root
};

struct AnalysisTomlInfo {
    /// Raw [run].input string verbatim from the TOML (path / directory /
    /// comma list / brace expansion). Expanded later to a flat file list
    /// by expand_input_path().
    std::string input_raw;
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

    // [run].input — accept either a single string (path / directory /
    // comma list) or an array. Multiple-array entries are joined with
    // commas so downstream expansion handles them uniformly.
    if (auto arr = (*run)["input"].as_array()) {
        std::string joined;
        for (const auto& el : *arr) {
            if (auto s = el.value<std::string>()) {
                if (!joined.empty()) joined += ',';
                joined += *s;
            }
        }
        info.input_raw = std::move(joined);
    } else if (auto s = (*run)["input"].value<std::string>()) {
        info.input_raw = *s;
    }
    if (info.input_raw.empty()) {
        fmt::print(stderr, "[local_batch] [run].input is empty\n");
        return std::nullopt;
    }
    if (info.output_base.empty()) {
        fmt::print(stderr, "[local_batch] [run].output is empty\n");
        return std::nullopt;
    }
    return info;
}

/// Brace-expand "{a,b,c}" patterns (nested supported; numeric ranges are
/// not — write them out). Mirrors clas12 StringUtils::expand_braces so a
/// [run].input like
///     /cache/.../recon/{018309,018310}/
/// expands to
///     /cache/.../recon/018309/   /cache/.../recon/018310/
/// the same way the framework's AnalysisRunner does — instead of the comma
/// split below shredding the brace group into broken fragments.
[[nodiscard]] auto expand_braces(std::string_view s) -> std::vector<std::string> {
    const auto open = s.find('{');
    if (open == std::string_view::npos) return {std::string(s)};

    int  depth = 1;
    auto close = std::string_view::npos;
    for (auto i = open + 1; i < s.size(); ++i) {
        if (s[i] == '{') ++depth;
        else if (s[i] == '}' && --depth == 0) { close = i; break; }
    }
    if (close == std::string_view::npos) return {std::string(s)};  // unmatched

    const auto contents = s.substr(open + 1, close - open - 1);
    std::vector<std::string_view> options;
    int         d    = 0;
    std::size_t last = 0;
    for (std::size_t j = 0; j < contents.size(); ++j) {
        if      (contents[j] == '{') ++d;
        else if (contents[j] == '}') --d;
        else if (contents[j] == ',' && d == 0) {
            options.push_back(contents.substr(last, j - last));
            last = j + 1;
        }
    }
    options.push_back(contents.substr(last));

    const std::string prefix(s.substr(0, open));
    const std::string suffix(s.substr(close + 1));
    std::vector<std::string> out;
    for (const auto& opt : options) {
        for (auto& expanded : expand_braces(prefix + std::string(opt) + suffix)) {
            out.push_back(std::move(expanded));
        }
    }
    return out;
}

/// Resolve a raw [run].input value (brace pattern / path / directory /
/// comma list) to a flat sorted list of HIPO files. Brace groups are
/// expanded first, then each result is comma-split; directories are scanned
/// recursively for `*.hipo`, single files pass through.
[[nodiscard]] auto expand_input_path(const std::string& raw)
    -> std::vector<std::string>
{
    std::vector<std::string> out;

    std::function<void(std::string)> add_one = [&](std::string s) {
        if (s.empty()) return;
        // Shell glob (*, ?, [...]): expand to the matching paths and resolve
        // each. File matches are filtered to *.hipo (so a broad pattern next to
        // the per-slice .root outputs ignores them); directory matches are
        // scanned. A pattern that matches nothing is skipped with a warning
        // instead of being treated as a literal path.
        if (s.find_first_of("*?[") != std::string::npos) {
            glob_t g{};
            const int rc = ::glob(s.c_str(), GLOB_TILDE | GLOB_NOSORT, nullptr, &g);
            std::vector<std::string> matches;
            if (rc == 0) {
                matches.reserve(g.gl_pathc);
                for (std::size_t i = 0; i < g.gl_pathc; ++i) matches.emplace_back(g.gl_pathv[i]);
            }
            ::globfree(&g);
            std::sort(matches.begin(), matches.end());
            if (matches.empty()) {
                fmt::print(stderr,
                    "[local_batch] Input pattern '{}' matched no files; skipping\n", s);
                return;
            }
            for (auto& m : matches) {
                if (fs::is_directory(m) || fs::path(m).extension() == ".hipo") {
                    add_one(std::move(m));
                }
            }
            return;
        }
        if (fs::is_directory(s)) {
            std::vector<std::string> dir_files;
            for (auto& p : fs::recursive_directory_iterator(s)) {
                if (p.is_regular_file() && p.path().extension() == ".hipo") {
                    dir_files.push_back(fs::absolute(p.path()).string());
                }
            }
            std::sort(dir_files.begin(), dir_files.end());
            out.insert(out.end(), dir_files.begin(), dir_files.end());
        } else if (fs::is_regular_file(s)) {
            out.push_back(std::move(s));
        } else {
            fmt::print(stderr,
                "[local_batch] Input '{}' is neither a file nor a directory; skipping\n",
                s);
        }
    };

    // Brace-expand FIRST so "{a,b}" survives, THEN comma-split each result
    // (brace-free paths have no top-level commas unless the user really
    // passed a comma-separated file list).
    for (const auto& expanded : expand_braces(raw)) {
        std::string cur;
        for (char c : expanded) {
            if (c == ',') { add_one(std::move(cur)); cur.clear(); }
            else cur += c;
        }
        add_one(std::move(cur));
    }
    return out;
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

/// Total event count of one file (reads only the index/trailer, not the
/// data). Returns 0 on failure so a single bad file doesn't poison the sum.
[[nodiscard]] auto probe_num_events(const std::string& input_file) -> long {
    hipo::reader r;
    r.open(input_file.c_str());
    if (!r.is_open()) return 0;
    const long n = r.getEntries();
    return n > 0 ? n : 0;
}

/// Sum the event counts of every file, probed in parallel (the opens are
/// I/O-bound and independent, so a small thread team hides the per-file
/// latency on shared storage). Used to fix the aggregate progress-bar
/// denominator up front, so the total doesn't grow as slices launch.
[[nodiscard]] auto probe_total_events(const std::vector<std::string>& files,
                                      int max_threads) -> long {
    const std::size_t nf = files.size();
    if (nf == 0) return 0;
    std::vector<long> per_file(nf, 0);
    std::atomic<std::size_t> next{0};
    const int nthreads = std::min<int>(std::max(1, max_threads), static_cast<int>(nf));
    std::vector<std::thread> probers;
    probers.reserve(static_cast<std::size_t>(nthreads));
    for (int t = 0; t < nthreads; ++t) {
        probers.emplace_back([&]() {
            for (;;) {
                const std::size_t i = next.fetch_add(1);
                if (i >= nf) break;
                per_file[i] = probe_num_events(files[i]);
            }
        });
    }
    for (auto& th : probers) th.join();
    long total = 0;
    for (long e : per_file) total += e;
    return total;
}

struct SliceResult {
    int slice;
    int exit_code;
    long ms;
    std::string description;   // "records [a, b)" or "N files"
    std::string output_file;
};

/// One slice's plan. Either a record range (single-file input) or a
/// list of files (multi-file / directory input); never both.
struct SlicePlan {
    int                       index = 0;
    int                       record_first = 0;
    int                       record_last  = -1;   // -1 = no range arg
    std::vector<std::string>  input_files;          // empty = no --input arg
    std::string               output_file;
    std::string               hipo_output_file;     // empty = no --hipo-output
};

/// fork/exec one slice; blocks until the child exits. Per the new UX
/// design (see plan.md "local_batch UX") each child:
///   - redirects stdout + stderr to <log_path> (per-slice log file),
///   - dup2s `progress_pipe_write_fd` onto FD 3 + passes
///     `--progress-fd 3 --slice-id <plan.index>` to the analysis,
/// so the parent's terminal only ever sees the aggregated bar.
[[nodiscard]] auto run_one_slice(
    const std::string& exe,
    const std::string& analysis_toml,
    const SlicePlan& plan,
    int progress_pipe_write_fd,
    const std::string& log_path,
    bool profile) -> int
{
    // String storage lives in the caller's frame, but the argv vector is
    // built per call so we own the lifetimes locally and only pass
    // pointers to execv.
    const std::string range_arg = (plan.record_last >= 0)
        ? fmt::format("{}:{}", plan.record_first, plan.record_last)
        : std::string{};
    std::string input_arg;
    if (!plan.input_files.empty()) {
        for (std::size_t i = 0; i < plan.input_files.size(); ++i) {
            if (i) input_arg += ',';
            input_arg += plan.input_files[i];
        }
    }
    const std::string slice_id_arg = fmt::format("{}", plan.index);
    const std::string progress_fd_arg = "3"; // we dup2 into FD 3 below

    pid_t pid = fork();
    if (pid < 0) {
        fmt::print(stderr, "[local_batch] fork() failed\n");
        return 127;
    }
    if (pid == 0) {
        // Redirect stdout + stderr to the per-slice log file so the
        // parent's terminal stays clean. Open here in the child so the
        // file descriptor and seek state belong to this worker only.
        int log_fd = ::open(log_path.c_str(),
                            O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (log_fd < 0) {
            // Stdio is still on the terminal; emit one diagnostic so
            // the parent's screen tells us *something* if log redirect
            // failed.
            fmt::print(stderr, "[local_batch] open('{}') failed: {}\n",
                       log_path, std::strerror(errno));
            _exit(127);
        }
        ::dup2(log_fd, 1);
        ::dup2(log_fd, 2);
        if (log_fd != 1 && log_fd != 2) ::close(log_fd);

        // Wire the progress pipe onto FD 3 so the analysis binary's
        // --progress-fd 3 lookup finds it. dup2 atomically replaces
        // FD 3 if it was already in use.
        if (progress_pipe_write_fd >= 0) {
            ::dup2(progress_pipe_write_fd, 3);
            if (progress_pipe_write_fd != 3) ::close(progress_pipe_write_fd);
        }

        std::vector<const char*> argv;
        argv.push_back(exe.c_str());
        argv.push_back("--config"); argv.push_back(analysis_toml.c_str());
        argv.push_back("--output"); argv.push_back(plan.output_file.c_str());
        if (!range_arg.empty()) {
            argv.push_back("--record-range");
            argv.push_back(range_arg.c_str());
        }
        if (!input_arg.empty()) {
            argv.push_back("--input");
            argv.push_back(input_arg.c_str());
        }
        if (!plan.hipo_output_file.empty()) {
            argv.push_back("--hipo-output");
            argv.push_back(plan.hipo_output_file.c_str());
        }
        if (progress_pipe_write_fd >= 0) {
            argv.push_back("--progress-fd"); argv.push_back(progress_fd_arg.c_str());
            argv.push_back("--slice-id");    argv.push_back(slice_id_arg.c_str());
        }
        if (profile) argv.push_back("--profile");
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

/// fork+execvp a PATH-resolved command and wait. Returns child's exit code
/// (or 128+signal on signal death, 127 on internal failure). Inherits the
/// parent's stdout/stderr so the user sees the merger's progress live.
[[nodiscard]] auto run_command(
    const std::string& cmd,
    const std::vector<std::string>& args) -> int
{
    pid_t pid = fork();
    if (pid < 0) {
        fmt::print(stderr, "[local_batch] fork() failed for '{}': {}\n",
                   cmd, std::strerror(errno));
        return 127;
    }
    if (pid == 0) {
        std::vector<const char*> argv;
        argv.push_back(cmd.c_str());
        for (const auto& a : args) { argv.push_back(a.c_str()); }
        argv.push_back(nullptr);
        execvp(cmd.c_str(), const_cast<char* const*>(argv.data()));
        fmt::print(stderr, "[local_batch] execvp('{}') failed: {}\n",
                   cmd, std::strerror(errno));
        _exit(127);
    }
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        fmt::print(stderr, "[local_batch] waitpid() failed for '{}'\n", cmd);
        return 127;
    }
    if (WIFEXITED(status))   { return WEXITSTATUS(status); }
    if (WIFSIGNALED(status)) { return 128 + WTERMSIG(status); }
    return 127;
}

} // namespace

int main(int argc, char* argv[]) {
    cxxopts::Options opts("local_batch",
        "Local (non-SWIF2) batch driver — shard a HIPO workload across N "
        "concurrent processes. Single-file inputs are split by record "
        "range; directory / multi-file inputs are split by file list.");
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
        ("automerge",    "On success, run hadd (and hipo-utils -merge if a "
                         "[skim].hipo_output is configured) to produce the "
                         "merged output automatically",
            cxxopts::value<bool>()->default_value("false"))
        ("profile",      "Run each slice with --profile and print an aggregated "
                         "per-step (wall vs CPU, IO/CPU-bound, slowest) table at "
                         "the end, summed across all slices.",
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
    if (args.count("output-dir")) { cfg.output_dir = args["output-dir"].as<std::string>(); }
    cfg.no_run        = args["dry-run"].as<bool>();
    cfg.automerge     = args["automerge"].as<bool>();
    cfg.profile       = args["profile"].as<bool>();

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

    fmt::print("[local_batch] Resolving [run].input = '{}' ...\n",
               info.input_raw);
    const auto files = expand_input_path(info.input_raw);
    if (files.empty()) {
        fmt::print(stderr,
            "[local_batch] No HIPO files resolved from '{}'\n", info.input_raw);
        return EXIT_FAILURE;
    }

    // Sharding mode:
    //   * #files >= #slices  →  pure file sharding (each slice owns 1+
    //     whole files, no record range).
    //   * #files <  #slices  →  per-file record sub-slicing. Each slice
    //     owns exactly one file + one record range; slice counts are
    //     allocated proportionally to file size so big files get more
    //     workers and small files don't get an artificial floor.
    const bool needs_record_subslicing =
        cfg.slices > static_cast<int>(files.size());

    // ---- Build per-slice plans --------------------------------------------

    std::vector<SlicePlan> plans;
    plans.reserve(static_cast<std::size_t>(cfg.slices));
    const bool has_hipo_out = !info.hipo_output_base.empty();

    auto attach_outputs = [&](SlicePlan& p) {
        p.output_file = make_slice_output(
            info.output_base, cfg.output_dir, cfg.slice_suffix, p.index);
        if (has_hipo_out) {
            p.hipo_output_file = make_slice_output(
                info.hipo_output_base, cfg.output_dir, cfg.slice_suffix, p.index);
        }
    };

    if (!needs_record_subslicing) {
        // Pure file sharding. Round-robin distribution keeps slice sizes
        // balanced even when file sizes vary, since neighbouring files
        // in train output tend to come from different runs.
        std::vector<std::vector<std::string>> buckets(static_cast<std::size_t>(cfg.slices));
        for (std::size_t i = 0; i < files.size(); ++i) {
            buckets[i % static_cast<std::size_t>(cfg.slices)].push_back(files[i]);
        }
        for (int i = 0; i < cfg.slices; ++i) {
            SlicePlan p;
            p.index = i;
            p.input_files = std::move(buckets[static_cast<std::size_t>(i)]);
            attach_outputs(p);
            plans.push_back(std::move(p));
        }
    } else {
        // Per-file record sub-slicing. Probe each file's record count so
        // we can allocate workers proportionally to total work.
        fmt::print("[local_batch] Probing record counts of {} files ...\n",
                   files.size());
        struct FileRec { std::string path; int nrec; };
        std::vector<FileRec> file_recs;
        file_recs.reserve(files.size());
        long total_records = 0;
        for (const auto& f : files) {
            const int n = probe_num_records(f);
            if (n <= 0) {
                fmt::print(stderr,
                    "[local_batch] Skipping {} (record count = {})\n", f, n);
                continue;
            }
            file_recs.push_back({f, n});
            total_records += n;
        }
        if (file_recs.empty()) {
            fmt::print(stderr,
                "[local_batch] No files with a positive record count\n");
            return EXIT_FAILURE;
        }
        fmt::print("[local_batch] Total records across {} file(s): {}\n",
                   file_recs.size(), total_records);

        // Greedy proportional allocation. After processing file i,
        // `slices_remaining` is dialled down by `n_this`, and the last
        // file always picks up whatever's left so the totals match.
        int  slices_remaining  = cfg.slices;
        long records_remaining = total_records;
        int  next_index        = 0;
        for (std::size_t i = 0; i < file_recs.size(); ++i) {
            const auto& fr = file_recs[i];
            const bool   is_last = (i + 1 == file_recs.size());
            // Reserve at least 1 slice for each file that still follows
            // this one, so a tiny tail file always gets a worker.
            const int    files_after = static_cast<int>(file_recs.size() - 1 - i);
            int n_this;
            if (is_last) {
                n_this = slices_remaining;
            } else {
                const double share = static_cast<double>(fr.nrec) / records_remaining;
                n_this = static_cast<int>(std::lround(share * slices_remaining));
                n_this = std::clamp(n_this, 1, slices_remaining - files_after);
            }

            const int per = fr.nrec / n_this;
            int cursor = 0;
            for (int k = 0; k < n_this; ++k) {
                SlicePlan p;
                p.index        = next_index++;
                p.input_files  = {fr.path};
                p.record_first = cursor;
                p.record_last  = (k + 1 == n_this) ? fr.nrec : (cursor + per);
                attach_outputs(p);
                plans.push_back(std::move(p));
                cursor = plans.back().record_last;
            }
            slices_remaining  -= n_this;
            records_remaining -= fr.nrec;
        }
    }

    // ---- Print banner -----------------------------------------------------

    fmt::print(
        "\n══════════════════════════════════════════════════════════════════\n"
        "  Local batch — {}\n"
        "──────────────────────────────────────────────────────────────────\n"
        "  Analysis TOML:   {}\n"
        "  Executable:      {}\n"
        "  Input:           {}\n"
        "  Files resolved:  {}\n"
        "  Slices:          {}\n"
        "  Max concurrent:  {}\n"
        "══════════════════════════════════════════════════════════════════\n\n",
        needs_record_subslicing
            ? "per-file record sub-slicing"
            : "file-list sharding",
        cfg.analysis_toml, cfg.executable, info.input_raw,
        files.size(), plans.size(), cfg.concurrent);

    if (!cfg.output_dir.empty()) {
        fs::create_directories(cfg.output_dir);
    }
    // Logs directory: each child redirects stdout/stderr into
    // <logs_dir>/slice<NNN>.log. Created up front so the children don't
    // race on mkdir.
    const fs::path logs_dir = cfg.output_dir.empty()
        ? fs::path{"."} / "logs"
        : fs::path{cfg.output_dir} / "logs";
    fs::create_directories(logs_dir);

    fmt::print("Per-slice logs: {}/slice<NNN>.log\n", logs_dir.string());
    fmt::print("\n");
    // The fully-expanded planned-slices table is verbose and the user
    // already approved the slice plan via dry-run; in non-dry runs we
    // collapse to a one-line summary. Each child's log file still
    // contains its own full configuration banner.
    if (cfg.no_run) {
        fmt::print("Planned slices:\n");
        for (const auto& p : plans) {
            std::string desc;
            if (p.record_last >= 0) {
                desc = fmt::format("{} records [{:>7d}, {:>7d})",
                                   fs::path(p.input_files.front()).filename().string(),
                                   p.record_first, p.record_last);
            } else {
                desc = fmt::format("{} file{}", p.input_files.size(),
                                   p.input_files.size() == 1 ? "" : "s");
            }
            if (!p.hipo_output_file.empty()) {
                fmt::print("  slice {:03d}: {} -> {}  +  {}\n",
                           p.index, desc, p.output_file, p.hipo_output_file);
            } else {
                fmt::print("  slice {:03d}: {} -> {}\n",
                           p.index, desc, p.output_file);
            }
        }
        fmt::print("\n--dry-run: not launching any slices.\n");
        return EXIT_SUCCESS;
    }

    // ---- Fixed progress-bar denominator ----------------------------------
    //
    // Sum the event count across every input file up front so the aggregate
    // bar's total is correct from the start. (Summing the children's
    // self-reported totals instead made the denominator grow as slices
    // launched under the concurrency cap.) Probed in parallel; the per-file
    // open only reads the index, not the data.
    fmt::print("[local_batch] Probing total event count of {} file(s) ...\n", files.size());
    const long grand_total_events = probe_total_events(files, cfg.concurrent);
    fmt::print("[local_batch] Total events to process: {}\n\n", grand_total_events);

    // ---- Aggregate progress pipe -----------------------------------------
    //
    // One shared pipe. Each child dup2s the write end onto FD 3 + passes
    // `--progress-fd 3 --slice-id <NNN>` to its analysis binary, which
    // periodically writes `<slice_id> <events_processed> <total_events>`
    // lines while running. The parent reads from the read end in a
    // dedicated thread, maintains per-slice counters, and feeds a single
    // ProgressTracker that renders one bar to the terminal.
    int pipe_fds[2];
    if (::pipe(pipe_fds) < 0) {
        fmt::print(stderr, "[local_batch] pipe() failed: {}\n", std::strerror(errno));
        return EXIT_FAILURE;
    }
    const int pipe_read_fd  = pipe_fds[0];
    const int pipe_write_fd = pipe_fds[1];

    const int n_slices = static_cast<int>(plans.size());
    std::vector<long> events_done (static_cast<std::size_t>(n_slices), 0);
    std::vector<long> events_total(static_cast<std::size_t>(n_slices), 0);
    std::mutex                agg_mu;
    std::atomic<int>          completed_slices{0};
    // Per-step timing aggregated across all slices from PERF pipe records
    // (only populated under --profile). Keyed by step index → (name, summed).
    std::map<int, std::pair<std::string, framework::perf::StepAccum>> perf_agg;

    // Fixed denominator from the up-front probe. Fall back to the
    // children's self-reported totals only if the probe yielded nothing
    // (e.g. all files unreadable here but the workers still ran).
    const bool have_total = grand_total_events > 0;
    ProgressTracker::Config bar_cfg;
    bar_cfg.label = fmt::format("Local batch (0/{} slices)", n_slices);
    bar_cfg.show_eta = true;
    bar_cfg.show_rate = true;
    ProgressTracker tracker(have_total ? static_cast<std::size_t>(grand_total_events) : 1,
                            bar_cfg);
    tracker.start();

    // Reader thread: parses incoming progress lines, updates the aggregate.
    std::thread reader([&]() {
        std::string carry;
        char buf[4096];
        for (;;) {
            ssize_t n = ::read(pipe_read_fd, buf, sizeof(buf));
            if (n <= 0) break;
            carry.append(buf, buf + static_cast<std::size_t>(n));
            std::size_t pos;
            while ((pos = carry.find('\n')) != std::string::npos) {
                std::string line = carry.substr(0, pos);
                carry.erase(0, pos + 1);
                // PERF records (per-step timing, --profile) — aggregate by step.
                {
                    int s_id = -1, idx = -1;
                    framework::perf::StepAccum a;
                    std::string name;
                    if (framework::perf::parse_perf_line(line, s_id, idx, a, name)) {
                        std::lock_guard lk(agg_mu);
                        auto& slot = perf_agg[idx];
                        slot.first = name;
                        slot.second.wall_ns += a.wall_ns;
                        slot.second.cpu_ns  += a.cpu_ns;
                        slot.second.count   += a.count;
                        continue;
                    }
                }
                int  slice_id = -1;
                long ev = 0, tot = 0;
                if (std::sscanf(line.c_str(), "%d %ld %ld", &slice_id, &ev, &tot) != 3) continue;
                if (slice_id < 0 || slice_id >= n_slices) continue;
                std::lock_guard lk(agg_mu);
                events_done [static_cast<std::size_t>(slice_id)] = ev;
                if (tot > 0) events_total[static_cast<std::size_t>(slice_id)] = tot;
                long sum_done = 0, sum_total = 0;
                for (int s = 0; s < n_slices; ++s) {
                    sum_done  += events_done [static_cast<std::size_t>(s)];
                    sum_total += events_total[static_cast<std::size_t>(s)];
                }
                // Keep the denominator fixed at the up-front grand total; only
                // fall back to the (growing) child-reported sum if the probe
                // failed.
                if (!have_total && sum_total > 0) {
                    tracker.set_total(static_cast<std::size_t>(sum_total));
                }
                tracker.set_processed(static_cast<std::size_t>(sum_done));
                tracker.set_label(fmt::format("Local batch ({}/{} slices)",
                                              completed_slices.load(), n_slices));
            }
        }
    });

    // ---- Run with bounded concurrency --------------------------------------
    //
    // Simple semaphore-style pool: spawn one std::thread per slice but
    // gate them through a counting condition variable so at most
    // `cfg.concurrent` are in their fork+wait at any one time.

    std::vector<SliceResult> results(static_cast<std::size_t>(n_slices));
    std::mutex mu;
    std::condition_variable cv;
    int in_flight = 0;

    auto wall_start = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(n_slices));
    for (int i = 0; i < n_slices; ++i) {
        workers.emplace_back([&, i]() {
            {
                std::unique_lock<std::mutex> lk(mu);
                cv.wait(lk, [&]{ return in_flight < cfg.concurrent; });
                ++in_flight;
            }
            const auto t0 = std::chrono::steady_clock::now();
            const auto& p = plans[static_cast<std::size_t>(i)];
            const std::string log_path =
                (logs_dir / fmt::format("slice{:03d}.log", p.index)).string();
            const int rc = run_one_slice(cfg.executable, cfg.analysis_toml,
                                         p, pipe_write_fd, log_path, cfg.profile);
            const auto t1 = std::chrono::steady_clock::now();
            const long ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            std::string desc = !p.input_files.empty()
                ? fmt::format("{} files", p.input_files.size())
                : fmt::format("records [{}, {})", p.record_first, p.record_last);
            results[static_cast<std::size_t>(i)] = {
                i, rc, ms, std::move(desc), p.output_file
            };
            completed_slices.fetch_add(1, std::memory_order_relaxed);
            // Update the bar's label inline so the user sees slice
            // completion counter advance immediately on each finish,
            // even if the next progress line is still ~200 ms away.
            tracker.set_label(fmt::format("Local batch ({}/{} slices)",
                                           completed_slices.load(), n_slices));
            {
                std::lock_guard<std::mutex> lk(mu);
                --in_flight;
            }
            cv.notify_all();
        });
    }
    for (auto& t : workers) t.join();

    // Close the parent's pipe-write copy so the reader thread sees EOF
    // (the children's dup2'd copies were already closed when they
    // exited). Then join the reader and tear down the tracker.
    ::close(pipe_write_fd);
    reader.join();
    ::close(pipe_read_fd);
    tracker.finish();

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

    // Aggregated per-step timing, summed across every slice's PERF records.
    if (cfg.profile) {
        std::vector<std::pair<std::string, framework::perf::StepAccum>> rows;
        {
            std::lock_guard lk(agg_mu);
            for (const auto& [idx, named] : perf_agg) rows.emplace_back(named.first, named.second);
        }
        if (!rows.empty()) {
            fmt::print("{}\n", framework::perf::render_table(
                "local_batch — aggregated per-step timing (summed over all slices)", rows));
        }
    }

    if (failures == 0) {
        const fs::path base{info.output_base};
        fs::path merged = cfg.output_dir.empty()
            ? base
            : (fs::path(cfg.output_dir) / base.filename());
        fs::path merged_hipo;
        if (has_hipo_out) {
            const fs::path hipo_base{info.hipo_output_base};
            merged_hipo = cfg.output_dir.empty()
                ? hipo_base
                : (fs::path(cfg.output_dir) / hipo_base.filename());
        }

        if (!cfg.automerge) {
            fmt::print("All slices succeeded. To merge:\n");
            fmt::print("    hadd -f {} ", merged.string());
            for (const auto& r : results) { fmt::print("{} ", r.output_file); }
            fmt::print("\n");
            if (has_hipo_out) {
                // hipo-utils ships a `hipo-merger` (or `hipo-utils -merge`)
                // helper for the .hipo case. The user picks whichever they
                // have in PATH; we just emit the slice list.
                fmt::print("    # skim HIPO output — merge with hipo-utils:\n");
                fmt::print("    hipo-utils -merge -o {} ", merged_hipo.string());
                for (const auto& p : plans) { fmt::print("{} ", p.hipo_output_file); }
                fmt::print("\n");
            }
            fmt::print("\n");
        } else {
            fmt::print("All slices succeeded. Auto-merging ...\n");
            int merge_rc = 0;

            // hadd -f <merged> <slice0> <slice1> ...
            std::vector<std::string> hadd_args{"-f", merged.string()};
            hadd_args.reserve(2 + results.size());
            for (const auto& r : results) { hadd_args.push_back(r.output_file); }
            fmt::print("[local_batch] $ hadd -f {} (+ {} slice files)\n",
                       merged.string(), results.size());
            int root_rc = run_command("hadd", hadd_args);
            if (root_rc != 0) {
                fmt::print(stderr,
                    "[local_batch] hadd failed (exit {}); leaving slice files in place\n",
                    root_rc);
                merge_rc = root_rc;
            } else {
                fmt::print("[local_batch] ROOT merge OK → {}\n", merged.string());
            }

            if (has_hipo_out) {
                // hipo-utils -merge -o <merged> <slice0> <slice1> ...
                std::vector<std::string> hipo_args{"-merge", "-o", merged_hipo.string()};
                hipo_args.reserve(3 + plans.size());
                for (const auto& p : plans) { hipo_args.push_back(p.hipo_output_file); }
                fmt::print("[local_batch] $ hipo-utils -merge -o {} (+ {} slice files)\n",
                           merged_hipo.string(), plans.size());
                int hipo_rc = run_command("hipo-utils", hipo_args);
                if (hipo_rc != 0) {
                    fmt::print(stderr,
                        "[local_batch] hipo-utils -merge failed (exit {}); "
                        "leaving slice files in place\n", hipo_rc);
                    if (merge_rc == 0) { merge_rc = hipo_rc; }
                } else {
                    fmt::print("[local_batch] HIPO merge OK → {}\n", merged_hipo.string());
                }
            }

            if (merge_rc != 0) { return EXIT_FAILURE; }
            fmt::print("\n");
        }
    }

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
