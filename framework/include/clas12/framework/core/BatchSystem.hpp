#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace clas12::framework {

/**
 * @brief Batch job system for HPC clusters
 *
 * Provides tools for:
 * - Job submission to SLURM
 * - Input file splitting
 * - Job configuration
 * - Status monitoring
 * - Result merging
 */
class BatchSystem {
   public:
    /**
     * @brief Supported batch systems
     */
    enum class System {
        SLURM,  // Simple Linux Utility for Resource Management
        Local   // Local execution (for testing)
    };

    /**
     * @brief Job configuration
     */
    struct JobConfig {
        // Job identification
        std::string job_name = "clas12_analysis";
        std::string output_dir = "./output";
        std::string log_dir = "./logs";

        // Resource requirements
        int n_cores = 1;
        int memory_gb = 4;
        std::string time_limit = "24:00:00";  // HH:MM:SS
        std::string partition = "default";
        std::string queue = "default";

        // Job array settings
        bool use_job_array = true;
        int max_concurrent_jobs = 100;

        // Input/output
        std::vector<std::string> input_files;
        int files_per_job = 10;
        std::string executable;
        std::vector<std::string> extra_args;

        // Environment
        std::string module_setup = "source /etc/profile.d/modules.sh\nmodule use /cvmfs/oasis.opensciencegrid.org/jlab/hallb/clas12/sw/modulefiles";
        std::vector<std::string> env_vars;
        std::vector<std::string> modules;  // For module load commands

        // Email notifications
        std::string email;
        bool email_on_start = false;
        bool email_on_end = true;
        bool email_on_fail = true;
    };

    /**
     * @brief Job status
     */
    struct JobStatus {
        std::string job_id;
        std::string state;  // PENDING, RUNNING, COMPLETED, FAILED, etc.
        int tasks_total = 0;
        int tasks_completed = 0;
        int tasks_running = 0;
        int tasks_failed = 0;
        std::string submit_time;
        std::string start_time;
        std::string end_time;
    };

    /**
     * @brief Split input files into chunks for parallel processing
     */
    static auto split_input_files(
        const std::vector<std::string>& input_files,
        int files_per_job) -> std::vector<std::vector<std::string>> {
        std::vector<std::vector<std::string>> chunks;

        for (size_t i = 0; i < input_files.size(); i += files_per_job) {
            size_t end = std::min(i + files_per_job, input_files.size());
            std::vector<std::string> chunk(
                input_files.begin() + i,
                input_files.begin() + end);
            chunks.push_back(chunk);
        }

        return chunks;
    }

    /**
     * @brief Generate SLURM submission script
     */
    static auto generate_slurm_script(
        const JobConfig& config,
        int job_index,
        const std::vector<std::string>& input_chunk) -> std::string {
        std::ostringstream script;

        // SLURM directives
        script << "#!/usr/bin/bash\n";
        script << "#SBATCH --job-name=" << config.job_name << "_" << job_index << "\n";
        script << "#SBATCH --output=" << config.log_dir << "/" << config.job_name << "_" << job_index << "_%j.out\n";
        script << "#SBATCH --error=" << config.log_dir << "/" << config.job_name << "_" << job_index << "_%j.err\n";
        script << "#SBATCH --ntasks=1\n";
        script << "#SBATCH --cpus-per-task=" << config.n_cores << "\n";
        script << "#SBATCH --mem=" << config.memory_gb << "G\n";
        script << "#SBATCH --time=" << config.time_limit << "\n";

        if (!config.partition.empty()) {
            script << "#SBATCH --partition=" << config.partition << "\n";
        }

        // Email notifications
        if (!config.email.empty()) {
            script << "#SBATCH --mail-user=" << config.email << "\n";
            std::string mail_type;
            if (config.email_on_start) mail_type += "BEGIN,";
            if (config.email_on_end) mail_type += "END,";
            if (config.email_on_fail) mail_type += "FAIL,";
            if (!mail_type.empty()) {
                mail_type.pop_back();  // Remove trailing comma
                script << "#SBATCH --mail-type=" << mail_type << "\n";
            }
        }

        script << "\n";

        // Setup module environment
        if (!config.module_setup.empty()) script << config.module_setup << "\n";

        // Load modules
        for (const auto& module : config.modules) {
            script << "module load " << module << "\n";
        }

        // Environment variables
        for (const auto& env : config.env_vars) {
            script << "export " << env << "\n";
        }

        script << "\n";

        // Job info
        script << "echo \"Job started at: $(date)\"\n";
        script << "echo \"Running on node: $(hostname)\"\n";
        script << "echo \"Job ID: $SLURM_JOB_ID\"\n";
        script << "echo \"Number of cores: $SLURM_CPUS_PER_TASK\"\n";
        script << "\n";

        // Create output directory
        std::string output_subdir = config.output_dir + "/job_" + std::to_string(job_index);
        script << "mkdir -p " << output_subdir << "\n";
        script << "cd " << output_subdir << "\n";
        script << "\n";

        // Build comma-separated input file list
        std::string input_list;
        for (size_t i = 0; i < input_chunk.size(); ++i) {
            if (i > 0) input_list += ",";
            input_list += input_chunk[i];
        }

        // Run analysis with all input files
        script << "echo \"Processing " << input_chunk.size() << " files\"\n";
        script << config.executable;
        script << " --input=" << input_list;

        // Add extra arguments
        for (const auto& arg : config.extra_args) {
            script << " " << arg;
        }

        script << "\n";
        script << "if [ $? -ne 0 ]; then\n";
        script << "    echo \"ERROR: Analysis failed\"\n";
        script << "    exit 1\n";
        script << "fi\n";
        script << "\n";

        script << "echo \"Job completed at: $(date)\"\n";

        return script.str();
    }

    /**
     * @brief Write script to file
     */
    static auto write_script(
        const std::string& filename,
        const std::string& content) -> bool {
        try {
            std::filesystem::create_directories(
                std::filesystem::path(filename).parent_path());

            std::ofstream file(filename);
            if (!file.is_open()) return false;

            file << content;
            file.close();

            // Make executable
            std::filesystem::permissions(
                filename,
                std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec,
                std::filesystem::perm_options::add);

            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }

    /**
     * @brief Submit jobs to batch system
     */
    static auto submit_jobs(
        const JobConfig& config,
        System system = System::SLURM) -> std::vector<std::string> {
        // Split input files
        auto chunks = split_input_files(config.input_files, config.files_per_job);

        // Convert paths to absolute paths
        auto abs_output_dir = std::filesystem::absolute(config.output_dir).string();
        auto abs_log_dir = std::filesystem::absolute(config.log_dir).string();
        auto abs_executable = std::filesystem::absolute(config.executable).string();
        auto abs_batch_dir = std::filesystem::absolute("./batch_scripts").string();

        // Create a config copy with absolute paths
        JobConfig abs_config = config;
        abs_config.output_dir = abs_output_dir;
        abs_config.log_dir = abs_log_dir;
        abs_config.executable = abs_executable;

        // Create directories
        std::filesystem::create_directories(abs_output_dir);
        std::filesystem::create_directories(abs_log_dir);
        std::filesystem::create_directories(abs_batch_dir);

        std::vector<std::string> job_ids;

        // Generate and submit scripts for each chunk
        for (size_t i = 0; i < chunks.size(); ++i) {
            std::string script_content;
            std::string script_file;
            std::string submit_command;

            switch (system) {
                case System::SLURM:
                    script_content = generate_slurm_script(abs_config, i, chunks[i]);
                    script_file = abs_batch_dir + "/job_" + std::to_string(i) + ".slurm";
                    submit_command = "sbatch " + script_file;
                    break;

                case System::Local:
                    // For local execution, just save the script
                    script_content = generate_slurm_script(abs_config, i, chunks[i]);
                    script_file = abs_batch_dir + "/job_" + std::to_string(i) + ".sh";
                    submit_command = "bash " + script_file;
                    break;
            }

            // Write script
            if (!write_script(script_file, script_content)) {
                continue;
            }

            // Note: Actual submission would use system() call
            // For safety, we just generate the scripts
            job_ids.push_back("job_" + std::to_string(i));
        }

        return job_ids;
    }

    /**
     * @brief Get system name as string
     */
    static auto system_name(System sys) -> std::string {
        switch (sys) {
            case System::SLURM:
                return "SLURM";
            case System::Local:
                return "Local";
            default:
                return "Unknown";
        }
    }
};

}  // namespace clas12::framework
