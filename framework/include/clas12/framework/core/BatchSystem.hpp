#pragma once

/**
 * @file BatchSystem.hpp
 * @brief SWIF2-based farm submission with per-job TOML synthesis.
 *
 * The submission flow is:
 *
 *   1. The user writes a "farm TOML" (see config/electron_analysis.farm.toml).
 *      It points at the analysis TOML, names the SWIF2 workflow, and
 *      declares input source and output destination on JLab paths.
 *
 *   2. For each chunk of `files_per_job` inputs, this module synthesises a
 *      *per-job* TOML that inherits from the analysis TOML and overrides
 *      `[run].input` (local SWIF2-staged names), `[run].output`, and
 *      `[run].num_threads`.
 *
 *   3. The module emits one `swif2 add-job ...` command per chunk, with
 *      `-input <local> <source>` lines mapping local names to /mss or
 *      /cache paths and `-output <local> <dest>` lines mapping the
 *      analysis output back to /volatile or /work.
 *
 *   4. All those commands plus the framing `swif2 create` / `swif2 run`
 *      go into a single shell script under `batch_scripts/`. Dry-run
 *      keeps the script on disk; `--submit` executes it.
 *
 * The job script itself just runs `executable --config job.toml` against
 * the staged local files — no /mss vs /cache branching, no manual cp glue.
 */

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <toml++/toml.hpp>

namespace framework {

class BatchSystem {
public:
    /// One job's SWIF2 / scheduler resource ask. Populated from
    /// `[farm.swif2]` plus CLI overrides; shared across all jobs of a
    /// workflow.
    struct Swif2JobConfig {
        std::string workflow;

        // Empty by default. Only set this for cross-site workflows that
        // move data between JLab and an external compute facility — those
        // require `swif2 create -site SITENAME` plus globus/login config.
        // For normal in-house JLab submission, leave it empty.
        std::string site;
        std::string account;
        std::string partition;
        std::string constraint;
        int         cores      = 1;
        int         ram_gb     = 4;
        int         disk_gb    = 10;
        std::string time       = "12:00:00";

        // KEY=VALUE labels propagated to every job via -tag.
        std::map<std::string, std::string> tags;

        // `module load` lines emitted at the top of each job's shell.
        std::vector<std::string> modules;

        // Extra environment exports.
        std::map<std::string, std::string> env;

        // Path to the analysis executable as visible from the job's PWD
        // (typically an absolute path on /work or /home).
        std::string executable;

        // The base analysis TOML. Read once, used as the template for every
        // synthesized per-job TOML.
        std::string analysis_config_path;

        // Where per-job ROOT files / logs land on /volatile or /work.
        std::string output_dir;
        std::string log_dir;
    };

    /// One unit of work in the workflow.
    struct JobSpec {
        std::string job_name;

        /// (local_name, source_path) pairs for SWIF2 -input.
        /// `local_name` matches a string in the synthesized per-job TOML's
        /// `[run].input`; `source_path` is a /mss or /cache path.
        std::vector<std::pair<std::string, std::string>> inputs;

        /// (local_name, dest_path) pairs for SWIF2 -output. Always at least
        /// `(<analysis_output_local>, <output_dir>/<job_name>/<analysis_output_local>)`
        /// and `("log.txt", <log_dir>/<job_name>.log)`.
        std::vector<std::pair<std::string, std::string>> outputs;

        /// Path on the submitting host to the per-job TOML. Staged into the
        /// job as `job.toml`.
        std::string job_toml_path;
    };

    // ========================================================================
    // Input slicing
    // ========================================================================

    /// Split a flat input list into `files_per_job`-sized chunks.
    static auto split_input_files(
        const std::vector<std::string>& input_files,
        int files_per_job
    ) -> std::vector<std::vector<std::string>> {
        std::vector<std::vector<std::string>> chunks;
        if (files_per_job <= 0) files_per_job = 1;
        for (std::size_t i = 0; i < input_files.size(); i += static_cast<std::size_t>(files_per_job)) {
            std::size_t end = std::min(i + static_cast<std::size_t>(files_per_job), input_files.size());
            chunks.emplace_back(input_files.begin() + i, input_files.begin() + end);
        }
        return chunks;
    }

    // ========================================================================
    // Per-job TOML synthesis
    // ========================================================================

    /// Load the base analysis TOML and override `[run].input`, `[run].output`,
    /// and `[run].num_threads` for one chunked job. Returns the serialized
    /// TOML text, ready to write to `batch_scripts/<workflow>_job_N.toml`.
    ///
    /// `local_input_names` are the SWIF2-staged filenames (e.g.
    /// `"input_0.hipo"`), NOT the original /mss source paths.
    /// `local_output_name` is the filename the analysis writes to in its
    /// CWD before SWIF2 copies it back out (e.g. `"result.root"`).
    static auto synthesize_job_toml(
        const std::string&              base_analysis_toml_path,
        const std::vector<std::string>& local_input_names,
        const std::string&              local_output_name,
        int                             num_threads
    ) -> std::string {
        auto base = toml::parse_file(base_analysis_toml_path);

        toml::array inputs;
        for (const auto& n : local_input_names) inputs.push_back(n);

        toml::table* run_tbl = base["run"].as_table();
        toml::table  fresh_run;
        if (!run_tbl) {
            base.insert("run", fresh_run);
            run_tbl = base["run"].as_table();
        }
        run_tbl->insert_or_assign("input",       std::move(inputs));
        run_tbl->insert_or_assign("output",      local_output_name);
        run_tbl->insert_or_assign("num_threads", static_cast<int64_t>(num_threads));

        std::ostringstream os;
        os << base;
        return os.str();
    }

    // ========================================================================
    // Per-job SWIF2 invocation
    // ========================================================================

    /// Render one `swif2 add-job ...` command line. The shell block
    /// invokes `<executable> --config job.toml` and tees output to
    /// `log.txt` (which itself is a SWIF2 -output, so it lands in the
    /// log_dir alongside the result).
    static auto swif2_add_job_command(
        const Swif2JobConfig& cfg,
        const JobSpec&        job
    ) -> std::string {
        std::ostringstream s;
        s << "swif2 add-job"
          << " -workflow " << shell_quote(cfg.workflow)
          << " -name "     << shell_quote(job.job_name);

        if (!cfg.account.empty())    s << " -account "    << shell_quote(cfg.account);
        if (!cfg.partition.empty())  s << " -partition "  << shell_quote(cfg.partition);
        if (!cfg.constraint.empty()) s << " -constraint " << shell_quote(cfg.constraint);

        s << " -cores " << cfg.cores
          << " -ram "   << cfg.ram_gb  << "G"
          << " -disk "  << cfg.disk_gb << "G"
          << " -time "  << shell_quote(format_swif2_time(cfg.time));

        for (const auto& [k, v] : cfg.tags) {
            s << " -tag " << shell_quote(k) << " " << shell_quote(v);
        }

        for (const auto& [local, source] : job.inputs) {
            s << " \\\n    -input " << shell_quote(local) << " " << shell_quote(source);
        }
        // The per-job TOML itself is staged in as `job.toml`.
        s << " \\\n    -input job.toml " << shell_quote(job.job_toml_path);

        for (const auto& [local, dest] : job.outputs) {
            s << " \\\n    -output " << shell_quote(local) << " " << shell_quote(dest);
        }

        // Build the inline shell block. SWIF2 makes `$SWIF2_JOB_CORES`
        // available in the job; we reuse it for OMP_NUM_THREADS so the
        // analysis matches whatever the scheduler granted.
        std::ostringstream sh;
        for (const auto& m : cfg.modules) {
            sh << "module load " << m << "\n";
        }
        sh << "export OMP_NUM_THREADS=${SWIF2_JOB_CORES:-" << cfg.cores << "}\n";
        for (const auto& [k, v] : cfg.env) {
            sh << "export " << k << "=" << v << "\n";
        }
        sh << cfg.executable << " --config job.toml 2>&1 | tee log.txt\n";

        s << " \\\n    -shell " << shell_quote(sh.str());
        return s.str();
    }

    // ========================================================================
    // Workflow-level script synthesis
    // ========================================================================

    /// Render the complete `batch_scripts/<workflow>.swif2.sh`: prelude
    /// that creates the output/log directories (so SWIF2's `-output` copies
    /// have a valid destination), `swif2 create`, N `swif2 add-job` blocks,
    /// then `swif2 run`. Per-job subdirectories under `output_dir` are also
    /// created up-front so all destination paths exist on the submission
    /// host before the workflow starts.
    static auto synthesize_workflow_script(
        const Swif2JobConfig&        cfg,
        const std::vector<JobSpec>&  jobs
    ) -> std::string {
        std::ostringstream s;
        s << "#!/usr/bin/env bash\n"
          << "# Auto-generated by `batch`. Do not edit; regenerate from the farm TOML.\n"
          << "set -euo pipefail\n\n";

        s << "# Ensure output/log destinations exist before SWIF2 copies into them.\n";
        if (!cfg.output_dir.empty()) {
            s << "mkdir -p " << shell_quote(cfg.output_dir) << "\n";
        }
        if (!cfg.log_dir.empty()) {
            s << "mkdir -p " << shell_quote(cfg.log_dir) << "\n";
        }
        // Per-job subdirectories — `-output result.root <output_dir>/<name>/result.root`
        // needs the parent to exist.
        if (!cfg.output_dir.empty() && !jobs.empty()) {
            for (const auto& job : jobs) {
                s << "mkdir -p " << shell_quote(cfg.output_dir + "/" + job.job_name) << "\n";
            }
        }
        s << "\n";

        s << "swif2 create -workflow " << shell_quote(cfg.workflow);
        if (!cfg.site.empty()) s << " -site " << shell_quote(cfg.site);
        s << "\n\n";

        for (std::size_t i = 0; i < jobs.size(); ++i) {
            s << "# job " << i << "\n";
            s << swif2_add_job_command(cfg, jobs[i]) << "\n\n";
        }

        s << "swif2 run -workflow " << shell_quote(cfg.workflow) << "\n";
        return s.str();
    }

    // ========================================================================
    // I/O helpers
    // ========================================================================

    /// Write text to a file, creating parent directories as needed. Returns
    /// false on filesystem error.
    static auto write_file(
        const std::string& path,
        const std::string& content
    ) -> bool {
        try {
            namespace fs = std::filesystem;
            auto parent = fs::path(path).parent_path();
            if (!parent.empty()) fs::create_directories(parent);
            std::ofstream out(path);
            if (!out) return false;
            out << content;
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }

    /// Make a file executable (chmod +x).
    static auto make_executable(const std::string& path) -> void {
        namespace fs = std::filesystem;
        try {
            fs::permissions(path,
                fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                fs::perm_options::add);
        } catch (const std::exception&) {
            // best effort
        }
    }

    /// Run the workflow shell script with `bash` and return its exit code.
    static auto submit_workflow(const std::string& script_path) -> int {
        const std::string cmd = "bash " + shell_quote(script_path);
        return std::system(cmd.c_str());
    }

    /// Convert a time-limit string into a SWIF2-acceptable form.
    ///
    /// SWIF2 expects either an integer with a unit suffix (`12h`, `720m`,
    /// `43200s`) or seconds with no suffix. Users typically write
    /// `HH:MM:SS` (the SLURM/PBS convention), so we accept that and
    /// convert. Anything that already parses as a SWIF2 form is passed
    /// through unchanged.
    ///
    /// Public so the unit tests can exercise the conversion directly.
    static auto format_swif2_time(std::string_view s) -> std::string {
        // Already a SWIF2 form: `<int><suffix>` with suffix in {s,m,h,d}.
        // Or pure-integer (= seconds).
        if (s.empty()) return std::string{s};
        if (s.find(':') == std::string_view::npos) return std::string{s};

        // HH:MM:SS or HH:MM. Anything else is rejected (bubble up the
        // original to give a clear SWIF2 error rather than guessing).
        long h = 0, m = 0, sec = 0;
        int matched = 0;
        const char* cstr = s.data();
        // Limited use of sscanf so we don't need to copy into std::string;
        // s is null-terminated in practice (TOML strings are), and we tolerate
        // the trailing characters by checking matched count.
        // NOTE: this assumes s is null-terminated; both call sites pass
        // strings stored in toml::table or std::string.
        if (std::string_view(s).find_first_not_of("0123456789:") != std::string_view::npos) {
            return std::string{s};
        }
        std::string buf{s};
        matched = std::sscanf(buf.c_str(), "%ld:%ld:%ld", &h, &m, &sec);
        if (matched < 2) return buf;  // fall through; let SWIF2 complain
        if (matched == 2) sec = 0;    // HH:MM treated as HH:MM:00

        const long total_seconds = h * 3600 + m * 60 + sec;
        return std::to_string(total_seconds) + "s";
    }

private:
    /// Single-quote a shell argument, escaping embedded single quotes.
    /// Result is suitable for `sh`/`bash` direct interpolation.
    static auto shell_quote(std::string_view s) -> std::string {
        std::string out;
        out.reserve(s.size() + 2);
        out += '\'';
        for (char c : s) {
            if (c == '\'') out += "'\\''";
            else           out += c;
        }
        out += '\'';
        return out;
    }
};

}  // namespace framework
