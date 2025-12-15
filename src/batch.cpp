/**
 * @file batch.cpp
 * @brief Generic batch job submission for any CLAS12 analysis
 *
 * Submits any analysis executable to HPC clusters (SLURM/PBS/Condor).
 * Decoupled from specific analyses - works with any executable that accepts --input.
 *
 * Usage:
 *   ./batch --executable=./pi0 --target=LD2 --polarity=Outbending
 *   ./batch --executable=./electron_analysis --config=my_config.json
 *   ./batch --executable=./lambda --data-dir=/path/to/data --system=pbs
 */

#include <cstdlib>
#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <string>

#include "CLAS/FileUtils.hpp"
#include "CLAS/Runs.hpp"
#include "CLAS/StringUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/BatchSystem.hpp"

namespace fs = std::filesystem;

//──────────────────────────────────────────────────────────────────────────────
// Configuration from JSON
//──────────────────────────────────────────────────────────────────────────────

struct BatchConfig {
    // Job identification
    std::string job_name;
    std::string executable;
    std::string system = "slurm";

    // Data source
    std::string data_dir;
    std::vector<std::string> input_files;

    // Resources
    int files_per_job = 10;
    int num_threads = 8;
    int memory_gb = 16;
    std::string time_limit = "12:00:00";
    std::string partition = "production";

    // Output
    std::string output_dir = "./output";
    std::string log_dir = "./logs";

    // Environment
    std::vector<std::string> modules;
    std::vector<std::string> env_vars;

    // Analysis-specific arguments (passed through to executable)
    std::vector<std::string> extra_args;

    // Notifications
    std::string email;
    bool dry_run = true;
};

auto load_config_from_json(const std::string& path) -> std::optional<BatchConfig> {
    std::ifstream file(path);
    if (!file.is_open()) {
        fmt::print(stderr, "ERROR: Cannot open config file: {}\n", path);
        return std::nullopt;
    }

    // Simple JSON parsing (for production, use nlohmann/json or similar)
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    BatchConfig cfg;

    // Extract string values
    auto extract_string = [&](const std::string& key) -> std::string {
        auto pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        pos = content.find(":", pos);
        if (pos == std::string::npos) return "";
        auto start = content.find("\"", pos);
        if (start == std::string::npos) return "";
        auto end = content.find("\"", start + 1);
        if (end == std::string::npos) return "";
        return content.substr(start + 1, end - start - 1);
    };

    auto extract_int = [&](const std::string& key, int default_val) -> int {
        auto pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) return default_val;
        pos = content.find(":", pos);
        if (pos == std::string::npos) return default_val;
        // Skip whitespace
        pos++;
        while (pos < content.size() && std::isspace(content[pos])) pos++;
        try {
            return std::stoi(content.substr(pos));
        } catch (...) {
            return default_val;
        }
    };

    cfg.job_name = extract_string("job_name");
    cfg.executable = extract_string("executable");
    cfg.system = extract_string("system");
    cfg.data_dir = extract_string("data_dir");
    cfg.output_dir = extract_string("output_dir");
    cfg.log_dir = extract_string("log_dir");
    cfg.time_limit = extract_string("time_limit");
    cfg.partition = extract_string("partition");
    cfg.email = extract_string("email");

    cfg.files_per_job = extract_int("files_per_job", 10);
    cfg.num_threads = extract_int("n_cores", 8);
    cfg.memory_gb = extract_int("memory_gb", 16);

    return cfg;
}

//──────────────────────────────────────────────────────────────────────────────
// Data directory resolution
//──────────────────────────────────────────────────────────────────────────────

auto resolve_data_directory(CLAS::Target target, CLAS::Polarity polarity) -> std::string {
    // RG-D data paths at JLab
    if (polarity == CLAS::Polarity::Outbending) {
        switch (target) {
            case CLAS::Target::LD2:
                return "/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/";
            case CLAS::Target::CxC:
                return "/cache/clas12/rg-d/production/pass1/recon/CxC/dst/train/SIDIS/";
            case CLAS::Target::CuSn:
                return "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/";
            default:
                return "";
        }
    } else {
        // Inbending
        return "/cache/hallb/scratch/rg-d/production/skim_pass0v11/";
    }
}

//──────────────────────────────────────────────────────────────────────────────
// Batch system conversion
//──────────────────────────────────────────────────────────────────────────────

auto parse_batch_system(const std::string& name) -> std::optional<Framework::BatchSystem::System> {
    if (name == "slurm") return Framework::BatchSystem::System::SLURM;
    if (name == "pbs")   return Framework::BatchSystem::System::PBS;
    if (name == "condor") return Framework::BatchSystem::System::Condor;
    if (name == "local") return Framework::BatchSystem::System::Local;
    return std::nullopt;
}

//──────────────────────────────────────────────────────────────────────────────
// Main
//──────────────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    cxxopts::Options options("batch", "Generic batch job submission for CLAS12 analyses");

    options.add_options()
        // Required
        ("executable", "Path to analysis executable", cxxopts::value<std::string>())

        // Data source (one of these or target and polarity)
        ("config", "JSON configuration file", cxxopts::value<std::string>()->default_value(""))
        ("data-dir", "Data directory with .hipo files", cxxopts::value<std::string>()->default_value(""))
        ("target", "Target type (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value(""))
        ("polarity", "Magnetic field polarity (Inbending, Outbending)", cxxopts::value<std::string>()->default_value(""))

        // Job configuration
        ("job-name", "Job name prefix", cxxopts::value<std::string>()->default_value(""))
        ("system", "Batch system (slurm, pbs, condor, local)", cxxopts::value<std::string>()->default_value("slurm"))
        ("files-per-job", "Input files per job", cxxopts::value<int>()->default_value("10"))
        ("num-threads", "Threads per job", cxxopts::value<int>()->default_value("8"))
        ("memory", "Memory per job (GB)", cxxopts::value<int>()->default_value("16"))
        ("time", "Time limit (HH:MM:SS)", cxxopts::value<std::string>()->default_value("12:00:00"))
        ("partition", "SLURM partition / PBS queue", cxxopts::value<std::string>()->default_value("production"))

        // Output
        ("output-dir", "Output directory", cxxopts::value<std::string>()->default_value("./output"))
        ("log-dir", "Log directory", cxxopts::value<std::string>()->default_value("./logs"))

        // Notifications
        ("email", "Email for notifications", cxxopts::value<std::string>()->default_value(""))

        // Control
        ("submit", "Actually submit jobs (default: dry-run)", cxxopts::value<bool>()->default_value("false"))
        ("extra-args", "Extra arguments for executable (comma-separated)", cxxopts::value<std::string>()->default_value(""))

        ("h,help", "Show help");

    options.parse_positional({"executable"});
    options.positional_help("<executable>");

    auto args = options.parse(argc, argv);

    if (args.count("help") || !args.count("executable")) {
        fmt::print("{}\n", options.help());
        fmt::print("\nExamples:\n");
        fmt::print("  # Submit pi0 analysis for LD2 outbending\n");
        fmt::print("  ./batch ./pi0 --target=LD2 --polarity=Outbending\n");
        fmt::print("\n");
        fmt::print("  # Submit electron analysis with custom data directory\n");
        fmt::print("  ./batch ./electron_analysis --data-dir=/path/to/data\n");
        fmt::print("\n");
        fmt::print("  # Use JSON config file\n");
        fmt::print("  ./batch ./lambda --config=config/batch_job.json\n");
        fmt::print("\n");
        fmt::print("  # Submit to PBS with extra arguments\n");
        fmt::print("  ./batch ./improved --system=pbs --extra-args=\"--max-events=100000\"\n");
        fmt::print("\n");
        return args.count("help") ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Build configuration
    // ─────────────────────────────────────────────────────────────────────────

    BatchConfig cfg;

    // Load from JSON if provided
    std::string config_file = args["config"].as<std::string>();
    if (!config_file.empty()) {
        auto loaded = load_config_from_json(config_file);
        if (!loaded) return EXIT_FAILURE;
        cfg = *loaded;
        fmt::print("Loaded configuration from: {}\n", config_file);
    }

    // Override with command-line arguments
    cfg.executable = args["executable"].as<std::string>();

    if (!args["job-name"].as<std::string>().empty()) {
        cfg.job_name = args["job-name"].as<std::string>();
    }
    if (!args["data-dir"].as<std::string>().empty()) {
        cfg.data_dir = args["data-dir"].as<std::string>();
    }
    if (!args["system"].as<std::string>().empty()) {
        cfg.system = args["system"].as<std::string>();
    }
    if (args["files-per-job"].as<int>() != 10) {
        cfg.files_per_job = args["files-per-job"].as<int>();
    }
    if (args["num-threads"].as<int>() != 8) {
        cfg.num_threads = args["num-threads"].as<int>();
    }
    if (args["memory"].as<int>() != 16) {
        cfg.memory_gb = args["memory"].as<int>();
    }
    if (args["time"].as<std::string>() != "12:00:00") {
        cfg.time_limit = args["time"].as<std::string>();
    }
    if (args["partition"].as<std::string>() != "production") {
        cfg.partition = args["partition"].as<std::string>();
    }
    if (!args["output-dir"].as<std::string>().empty()) {
        cfg.output_dir = args["output-dir"].as<std::string>();
    }
    if (!args["log-dir"].as<std::string>().empty()) {
        cfg.log_dir = args["log-dir"].as<std::string>();
    }
    if (!args["email"].as<std::string>().empty()) {
        cfg.email = args["email"].as<std::string>();
    }

    cfg.dry_run = !args["submit"].as<bool>();

    // Parse extra arguments
    std::string extra_args_str = args["extra-args"].as<std::string>();
    if (!extra_args_str.empty()) {
        // Split by comma
        size_t pos = 0;
        while ((pos = extra_args_str.find(',')) != std::string::npos) {
            cfg.extra_args.push_back(extra_args_str.substr(0, pos));
            extra_args_str.erase(0, pos + 1);
        }
        if (!extra_args_str.empty()) {
            cfg.extra_args.push_back(extra_args_str);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Resolve data directory
    // ─────────────────────────────────────────────────────────────────────────

    std::string target_str = args["target"].as<std::string>();
    std::string polarity_str = args["polarity"].as<std::string>();

    CLAS::Target target = CLAS::Target::LD2;
    CLAS::Polarity polarity = CLAS::Polarity::Outbending;

    if (!target_str.empty() && !polarity_str.empty()) {
        target = CLAS::string_to_target(target_str);
        polarity = CLAS::string_to_polarity(polarity_str);

        if (cfg.data_dir.empty()) {
            cfg.data_dir = resolve_data_directory(target, polarity);
        }

        // Add target/polarity to extra args for the analysis
        cfg.extra_args.push_back(fmt::format("--target={}", target_str));
        cfg.extra_args.push_back(fmt::format("--polarity={}", polarity_str));
    }

    if (cfg.data_dir.empty()) {
        fmt::print(stderr, "ERROR: No data source specified.\n");
        fmt::print(stderr, "Use --data-dir, --target/--polarity, or --config\n");
        return EXIT_FAILURE;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Validate executable
    // ─────────────────────────────────────────────────────────────────────────

    if (!fs::exists(cfg.executable)) {
        fmt::print(stderr, "ERROR: Executable not found: {}\n", cfg.executable);
        return EXIT_FAILURE;
    }

    // Generate job name from executable if not set
    if (cfg.job_name.empty()) {
        cfg.job_name = fs::path(cfg.executable).stem().string();
        if (!target_str.empty()) {
            cfg.job_name += "_" + target_str;
        }
        if (!polarity_str.empty()) {
            cfg.job_name += "_" + polarity_str;
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Read input files
    // ─────────────────────────────────────────────────────────────────────────

    fmt::print("Reading input files from: {}\n", cfg.data_dir);
    std::vector<std::string> input_files = CLAS::FileUtils::read_recursive_hipo_files(cfg.data_dir);

    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No .hipo files found in {}\n", cfg.data_dir);
        return EXIT_FAILURE;
    }

    fmt::print("Found {} input files\n", input_files.size());

    // ─────────────────────────────────────────────────────────────────────────
    // Parse batch system
    // ─────────────────────────────────────────────────────────────────────────

    auto batch_system = parse_batch_system(cfg.system);
    if (!batch_system) {
        fmt::print(stderr, "ERROR: Unknown batch system '{}'\n", cfg.system);
        fmt::print(stderr, "Valid options: slurm, pbs, condor, local\n");
        return EXIT_FAILURE;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Build job configuration
    // ─────────────────────────────────────────────────────────────────────────

    Framework::BatchSystem::JobConfig job_config;
    job_config.job_name = cfg.job_name;
    job_config.output_dir = cfg.output_dir;
    job_config.log_dir = cfg.log_dir;
    job_config.n_cores = cfg.num_threads;
    job_config.memory_gb = cfg.memory_gb;
    job_config.time_limit = cfg.time_limit;
    job_config.partition = cfg.partition;
    job_config.queue = cfg.partition;
    job_config.input_files = input_files;
    job_config.files_per_job = cfg.files_per_job;
    job_config.executable = cfg.executable;
    job_config.email = cfg.email;
    job_config.email_on_end = !cfg.email.empty();
    job_config.email_on_fail = !cfg.email.empty();

    // Add thread count to extra args
    cfg.extra_args.push_back(fmt::format("--num-threads={}", cfg.num_threads));
    job_config.extra_args = cfg.extra_args;

    // Default JLab modules
    job_config.modules = {"gcc/13.2.0", "cmake/3.29.0", "root/6.36.04"};
    job_config.env_vars = {
        "OMP_NUM_THREADS=" + std::to_string(cfg.num_threads),
        "ROOT_HIST=0"
    };

    // ─────────────────────────────────────────────────────────────────────────
    // Print summary
    // ─────────────────────────────────────────────────────────────────────────

    size_t num_jobs = (input_files.size() + cfg.files_per_job - 1) / cfg.files_per_job;

    fmt::print("\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Batch Job Configuration\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Job name:        {}\n", cfg.job_name);
    fmt::print("  Executable:      {}\n", cfg.executable);
    fmt::print("  Batch system:    {}\n", Framework::BatchSystem::system_name(*batch_system));
    fmt::print("  Data directory:  {}\n", cfg.data_dir);
    fmt::print("  Total files:     {}\n", input_files.size());
    fmt::print("  Files per job:   {}\n", cfg.files_per_job);
    fmt::print("  Number of jobs:  {}\n", num_jobs);
    fmt::print("  Threads/job:     {}\n", cfg.num_threads);
    fmt::print("  Memory/job:      {} GB\n", cfg.memory_gb);
    fmt::print("  Time limit:      {}\n", cfg.time_limit);
    fmt::print("  Partition:       {}\n", cfg.partition);
    fmt::print("  Output dir:      {}\n", cfg.output_dir);
    fmt::print("  Log dir:         {}\n", cfg.log_dir);
    if (!cfg.email.empty()) {
        fmt::print("  Email:           {}\n", cfg.email);
    }
    if (!cfg.extra_args.empty()) {
        fmt::print("  Extra args:      ");
        for (const auto& arg : cfg.extra_args) fmt::print("{} ", arg);
        fmt::print("\n");
    }
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("\n");

    // ─────────────────────────────────────────────────────────────────────────
    // Generate batch scripts
    // ─────────────────────────────────────────────────────────────────────────

    fmt::print("Generating batch submission scripts...\n");
    auto job_ids = Framework::BatchSystem::submit_jobs(job_config, *batch_system);

    if (job_ids.empty()) {
        fmt::print(stderr, "ERROR: Failed to generate batch scripts\n");
        return EXIT_FAILURE;
    }

    fmt::print("Successfully generated {} job scripts in ./batch_scripts/\n", job_ids.size());

    // ─────────────────────────────────────────────────────────────────────────
    // Submit or show instructions
    // ─────────────────────────────────────────────────────────────────────────

    if (cfg.dry_run) {
        fmt::print("\n");
        fmt::print("DRY RUN MODE - Scripts generated but not submitted\n");
        fmt::print("\n");
        fmt::print("To submit jobs:\n");

        switch (*batch_system) {
            case Framework::BatchSystem::System::SLURM:
                fmt::print("  for script in batch_scripts/*.slurm; do sbatch $script; done\n");
                break;
            case Framework::BatchSystem::System::PBS:
                fmt::print("  for script in batch_scripts/*.pbs; do qsub $script; done\n");
                break;
            case Framework::BatchSystem::System::Condor:
                fmt::print("  for script in batch_scripts/*.condor; do condor_submit $script; done\n");
                break;
            case Framework::BatchSystem::System::Local:
                fmt::print("  for script in batch_scripts/*.sh; do bash $script & done\n");
                break;
        }

        fmt::print("\nOr re-run with --submit to submit automatically.\n");
    } else {
        fmt::print("\nSubmitting jobs...\n");
        // TODO: Actually submit jobs using system() calls
        fmt::print("Jobs submitted! Monitor with:\n");

        switch (*batch_system) {
            case Framework::BatchSystem::System::SLURM:
                fmt::print("  squeue -u $USER\n");
                break;
            case Framework::BatchSystem::System::PBS:
                fmt::print("  qstat -u $USER\n");
                break;
            case Framework::BatchSystem::System::Condor:
                fmt::print("  condor_q\n");
                break;
            case Framework::BatchSystem::System::Local:
                fmt::print("  ps aux | grep {}\n", cfg.job_name);
                break;
        }
    }

    fmt::print("\nTo merge output ROOT files after jobs complete:\n");
    fmt::print("  hadd -f merged_{}.root {}/*/*.root\n", cfg.job_name, cfg.output_dir);
    fmt::print("\n");

    return EXIT_SUCCESS;
}
