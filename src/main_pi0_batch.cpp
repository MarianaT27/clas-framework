/**
 * @file main_pi0_batch.cpp
 * @brief Batch job submission for Pi0 reconstruction analysis
 *
 * This executable generates SLURM/PBS/Condor submission scripts
 * for parallel processing of Pi0 reconstruction on HPC clusters.
 *
 * Requirements: 3.1, 3.2, 4.4
 */

#include <fmt/format.h>
#include <cstdlib>
#include <cxxopts.hpp>
#include <string>
#include <filesystem>
#include "CLAS/FileUtils.hpp"
#include "CLAS/Target.hpp"
#include "CLAS/Runs.hpp"
#include "CLAS/StringUtils.hpp"
#include "Framework/BatchSystem.hpp"
#include "fmt/base.h"

#include <fmt/format.h>
#include <cstdlib>
#include <cxxopts.hpp>
#include <string>
#include <TROOT.h>
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/Runs.hpp"
#include "CLAS/StringUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/Application.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/PhotonCutsService.hpp"
#include "Framework/Services/MetadataService.hpp"
#include "Framework/Services/ProfilingService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"
#include "fmt/base.h"

int main(int argc, char* argv[]) {
    cxxopts::Options options("pi0_batch", "Generate batch jobs for Pi0 reconstruction analysis");

    options.add_options()
            ("target", "Target type (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value("LD2"))
            ("polarity", "Magnetic field polarity (In, Out)", cxxopts::value<std::string>()->default_value("Outbending"))
            ("percentage", "Percentage of events to process (0-100)", cxxopts::value<long>()->default_value("100"))
            ("system", "Batch system (slurm, pbs, condor, local)", cxxopts::value<std::string>()->default_value("slurm"))
            ("files-per-job", "Number of input files per job", cxxopts::value<int>()->default_value("10"))
            ("num-threads", "Number of threads per job", cxxopts::value<int>()->default_value("8"))
            ("memory", "Memory per job in GB", cxxopts::value<int>()->default_value("16"))
            ("time", "Time limit per job (HH:MM:SS)", cxxopts::value<std::string>()->default_value("12:00:00"))
            ("partition", "SLURM partition or PBS queue", cxxopts::value<std::string>()->default_value("default"))
            ("output-dir", "Output directory for results", cxxopts::value<std::string>()->default_value("./output"))
            ("log-dir", "Log directory for job logs", cxxopts::value<std::string>()->default_value("./logs"))
            ("executable", "Path to pi0 analysis executable", cxxopts::value<std::string>()->default_value("./build/main_pi0"))
            ("email", "Email for notifications", cxxopts::value<std::string>()->default_value(""))
            ("dry-run", "Generate scripts without submitting", cxxopts::value<bool>()->default_value("true"))
            ("help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return EXIT_SUCCESS;
    }

    // Parse arguments
    std::string target_str = args["target"].as<std::string>();
    std::string polarity_str = args["polarity"].as<std::string>();
    long percentage = args["percentage"].as<long>();
    std::string system_str = args["system"].as<std::string>();
    int files_per_job = args["files-per-job"].as<int>();
    int num_threads = args["num-threads"].as<int>();
    int memory_gb = args["memory"].as<int>();
    std::string time_limit = args["time"].as<std::string>();
    std::string partition = args["partition"].as<std::string>();
    std::string output_dir = args["output-dir"].as<std::string>();
    std::string log_dir = args["log-dir"].as<std::string>();
    std::string executable = args["executable"].as<std::string>();
    std::string email = args["email"].as<std::string>();
    bool dry_run = args["dry-run"].as<bool>();

    // Validate target and polarity
    CLAS::Target target = CLAS::string_to_target(target_str);
    CLAS::Polarity polarity = CLAS::string_to_polarity(polarity_str);

    // Determine batch system
    Framework::BatchSystem::System batch_system;
    if (system_str == "slurm") {
        batch_system = Framework::BatchSystem::System::SLURM;
    } else if (system_str == "pbs") {
        batch_system = Framework::BatchSystem::System::PBS;
    } else if (system_str == "condor") {
        batch_system = Framework::BatchSystem::System::Condor;
    } else if (system_str == "local") {
        batch_system = Framework::BatchSystem::System::Local;
    } else {
        fmt::print(stderr, "ERROR: Unknown batch system '{}'\n", system_str);
        fmt::print(stderr, "Valid options: slurm, pbs, condor, local\n");
        return EXIT_FAILURE;
    }

    // Determine data directory based on target and polarity
    std::array sources = (polarity == CLAS::Polarity::Outbending)
                             ? std::array{std::tuple{CLAS::Target::CuSn, "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/", &CLAS::outbending_runs_CuSn},
                                          std::tuple{CLAS::Target::CxC, "/cache/clas12/rg-d/production/pass1/recon/CxC/dst/train/SIDIS/", &CLAS::outbending_runs_CxC},
                                          std::tuple{CLAS::Target::LD2, "/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/", &CLAS::outbending_runs_LD2}}
                             : std::array{std::tuple{CLAS::Target::CuSn, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &CLAS::inbending_runs_CuSn},
                                          std::tuple{CLAS::Target::CxC, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &CLAS::inbending_runs_CxC},
                                          std::tuple{CLAS::Target::LD2, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &CLAS::inbending_runs_LD2}};
    
    std::string data_dir;
    if (target_str != "" && polarity_str != "") {
        bool valid = false;
        for (const auto& [src_target, src_dir, _] : sources) {
            if (target == src_target) {
                data_dir = src_dir;
                valid = true;
                break;
            }
        }
        if (!valid) {
            fmt::println(stderr, "ERROR: Invalid target '{}' for polarity '{}'", target_str, polarity_str);
            fmt::println(stderr, "Valid targets are: LD2, CxC, CuSn, Cu, or Sn");
            return EXIT_FAILURE;
        }
    }

    // Read input files
    fmt::print("Reading input files from: {}\n", data_dir);
    std::vector<std::string> input_files = CLAS::FileUtils::read_recursive_hipo_files(data_dir);

    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No input files found in {}\n", data_dir);
        return EXIT_FAILURE;
    }

    fmt::print("Found {} input files\n", input_files.size());

    // Configure batch job
    Framework::BatchSystem::JobConfig config;
    config.job_name = fmt::format("pi0_{}_{}", CLAS::to_string(target), CLAS::to_string(polarity));
    config.output_dir = output_dir;
    config.log_dir = log_dir;
    config.n_cores = num_threads;
    config.memory_gb = memory_gb;
    config.time_limit = time_limit;
    config.partition = partition;
    config.queue = partition;  // Same for PBS
    config.input_files = input_files;
    config.files_per_job = files_per_job;
    config.executable = executable;
    config.email = email;
    config.email_on_end = !email.empty();
    config.email_on_fail = !email.empty();

    // Add analysis-specific arguments
    config.extra_args = {
        fmt::format("--target={}", target_str),
        fmt::format("--polarity={}", polarity_str),
        fmt::format("--percentage={}", percentage),
        fmt::format("--num_threads={}", num_threads)
    };

    // Environment setup for JLab/CLAS12
    config.modules = {
        "gcc/12.1.0",
        "cmake/3.24.2"
    };

    config.root_setup = "source /apps/root/6.28.04/bin/thisroot.sh";

    config.env_vars = {
        "OMP_NUM_THREADS=" + std::to_string(num_threads),
        "ROOT_HIST=0"  // Disable ROOT histogram addition
    };

    // Print configuration summary
    fmt::print("\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Batch Job Configuration\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Job name:        {}\n", config.job_name);
    fmt::print("  Batch system:    {}\n", Framework::BatchSystem::system_name(batch_system));
    fmt::print("  Target:          {}\n", CLAS::to_string(target));
    fmt::print("  Polarity:        {}\n", CLAS::to_string(polarity));
    fmt::print("  Data directory:  {}\n", data_dir);
    fmt::print("  Total files:     {}\n", input_files.size());
    fmt::print("  Files per job:   {}\n", files_per_job);
    fmt::print("  Number of jobs:  {}\n", (input_files.size() + files_per_job - 1) / files_per_job);
    fmt::print("  Threads/job:     {}\n", num_threads);
    fmt::print("  Memory/job:      {} GB\n", memory_gb);
    fmt::print("  Time limit:      {}\n", time_limit);
    fmt::print("  Partition:       {}\n", partition);
    fmt::print("  Output dir:      {}\n", output_dir);
    fmt::print("  Log dir:         {}\n", log_dir);
    fmt::print("  Executable:      {}\n", executable);
    if (!email.empty()) {
        fmt::print("  Email:           {}\n", email);
    }
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("\n");

    // Generate batch scripts
    fmt::print("Generating batch submission scripts...\n");
    auto job_ids = Framework::BatchSystem::submit_jobs(config, batch_system);

    if (job_ids.empty()) {
        fmt::print(stderr, "ERROR: Failed to generate batch scripts\n");
        return EXIT_FAILURE;
    }

    fmt::print("Successfully generated {} job scripts in ./batch_scripts/\n", job_ids.size());

    if (dry_run) {
        fmt::print("\n");
        fmt::print("DRY RUN MODE - Scripts generated but not submitted\n");
        fmt::print("\n");
        fmt::print("To submit jobs manually:\n");

        if (batch_system == Framework::BatchSystem::System::SLURM) {
            fmt::print("  for script in batch_scripts/*.slurm; do sbatch $script; done\n");
        } else if (batch_system == Framework::BatchSystem::System::PBS) {
            fmt::print("  for script in batch_scripts/*.pbs; do qsub $script; done\n");
        } else if (batch_system == Framework::BatchSystem::System::Condor) {
            fmt::print("  for script in batch_scripts/*.condor; do condor_submit $script; done\n");
        } else if (batch_system == Framework::BatchSystem::System::Local) {
            fmt::print("  for script in batch_scripts/*.sh; do bash $script & done\n");
        }

        fmt::print("\n");
        fmt::print("To merge output ROOT files after jobs complete:\n");
        fmt::print("  hadd -f merged_output.root {}/*/*.root\n", output_dir);
    } else {
        fmt::print("\n");
        fmt::print("Jobs submitted! Monitor with:\n");

        if (batch_system == Framework::BatchSystem::System::SLURM) {
            fmt::print("  squeue -u $USER\n");
        } else if (batch_system == Framework::BatchSystem::System::PBS) {
            fmt::print("  qstat -u $USER\n");
        } else if (batch_system == Framework::BatchSystem::System::Condor) {
            fmt::print("  condor_q\n");
        }
    }

    return EXIT_SUCCESS;
}
