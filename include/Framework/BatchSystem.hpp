#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <fstream>

namespace Framework {

/**
 * @brief Batch job system for HPC clusters
 *
 * Provides tools for:
 * - Job submission to SLURM, PBS, Condor
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
        SLURM,      // Simple Linux Utility for Resource Management
        PBS,        // Portable Batch System
        Condor,     // HTCondor
        Local       // Local execution (for testing)
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
        std::string root_setup = "source /path/to/root/bin/thisroot.sh";
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
        int files_per_job
    ) -> std::vector<std::vector<std::string>> {
        std::vector<std::vector<std::string>> chunks;

        for (size_t i = 0; i < input_files.size(); i += files_per_job) {
            size_t end = std::min(i + files_per_job, input_files.size());
            std::vector<std::string> chunk(
                input_files.begin() + i,
                input_files.begin() + end
            );
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
        const std::vector<std::string>& input_chunk
    ) -> std::string {
        std::ostringstream script;

        // SLURM directives
        script << "#!/bin/bash\n";
        script << "#SBATCH --job-name=" << config.job_name << "_" << job_index << "\n";
        script << "#SBATCH --output=" << config.log_dir << "/" << config.job_name
               << "_" << job_index << "_%j.out\n";
        script << "#SBATCH --error=" << config.log_dir << "/" << config.job_name
               << "_" << job_index << "_%j.err\n";
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

        // Load modules
        for (const auto& module : config.modules) {
            script << "module load " << module << "\n";
        }

        // Setup ROOT
        if (!config.root_setup.empty()) {
            script << config.root_setup << "\n";
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
     * @brief Generate PBS submission script
     */
    static auto generate_pbs_script(
        const JobConfig& config,
        int job_index,
        const std::vector<std::string>& input_chunk
    ) -> std::string {
        std::ostringstream script;

        // PBS directives
        script << "#!/bin/bash\n";
        script << "#PBS -N " << config.job_name << "_" << job_index << "\n";
        script << "#PBS -o " << config.log_dir << "/" << config.job_name
               << "_" << job_index << ".out\n";
        script << "#PBS -e " << config.log_dir << "/" << config.job_name
               << "_" << job_index << ".err\n";
        script << "#PBS -l nodes=1:ppn=" << config.n_cores << "\n";
        script << "#PBS -l mem=" << config.memory_gb << "gb\n";
        script << "#PBS -l walltime=" << config.time_limit << "\n";

        if (!config.queue.empty()) {
            script << "#PBS -q " << config.queue << "\n";
        }

        // Email notifications
        if (!config.email.empty()) {
            script << "#PBS -M " << config.email << "\n";
            std::string mail_options;
            if (config.email_on_start) mail_options += "b";
            if (config.email_on_end) mail_options += "e";
            if (config.email_on_fail) mail_options += "a";
            if (!mail_options.empty()) {
                script << "#PBS -m " << mail_options << "\n";
            }
        }

        script << "\n";

        // Change to working directory
        script << "cd $PBS_O_WORKDIR\n";
        script << "\n";

        // Load modules
        for (const auto& module : config.modules) {
            script << "module load " << module << "\n";
        }

        // Setup ROOT
        if (!config.root_setup.empty()) {
            script << config.root_setup << "\n";
        }

        // Environment variables
        for (const auto& env : config.env_vars) {
            script << "export " << env << "\n";
        }

        script << "\n";

        // Job info
        script << "echo \"Job started at: $(date)\"\n";
        script << "echo \"Running on node: $(hostname)\"\n";
        script << "echo \"Job ID: $PBS_JOBID\"\n";
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
     * @brief Generate Condor submission file
     */
    static auto generate_condor_submit(
        const JobConfig& config,
        int job_index,
        const std::vector<std::string>& input_chunk
    ) -> std::string {
        std::ostringstream submit;

        submit << "# HTCondor submission file\n";
        submit << "universe = vanilla\n";
        submit << "executable = " << config.executable << "\n";

        // Build comma-separated input file list
        std::string input_list;
        for (size_t i = 0; i < input_chunk.size(); ++i) {
            if (i > 0) input_list += ",";
            input_list += input_chunk[i];
        }

        // Arguments
        submit << "arguments = --input=" << input_list;
        for (const auto& arg : config.extra_args) {
            submit << " " << arg;
        }
        submit << "\n";

        // Resources
        submit << "request_cpus = " << config.n_cores << "\n";
        submit << "request_memory = " << config.memory_gb << "GB\n";

        // Output
        submit << "output = " << config.log_dir << "/" << config.job_name
               << "_" << job_index << ".out\n";
        submit << "error = " << config.log_dir << "/" << config.job_name
               << "_" << job_index << ".err\n";
        submit << "log = " << config.log_dir << "/" << config.job_name
               << "_" << job_index << ".log\n";

        // Environment
        if (!config.env_vars.empty()) {
            submit << "environment = \"";
            for (const auto& env : config.env_vars) {
                submit << env << " ";
            }
            submit << "\"\n";
        }

        // Email notifications
        if (!config.email.empty()) {
            submit << "notify_user = " << config.email << "\n";
            if (config.email_on_end || config.email_on_fail) {
                submit << "notification = ";
                if (config.email_on_end && config.email_on_fail) {
                    submit << "Always\n";
                } else if (config.email_on_fail) {
                    submit << "Error\n";
                } else {
                    submit << "Complete\n";
                }
            }
        }

        submit << "queue\n";

        return submit.str();
    }

    /**
     * @brief Write script to file
     */
    static auto write_script(
        const std::string& filename,
        const std::string& content
    ) -> bool {
        try {
            std::filesystem::create_directories(
                std::filesystem::path(filename).parent_path()
            );

            std::ofstream file(filename);
            if (!file.is_open()) return false;

            file << content;
            file.close();

            // Make executable
            std::filesystem::permissions(
                filename,
                std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec,
                std::filesystem::perm_options::add
            );

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
        System system = System::SLURM
    ) -> std::vector<std::string> {
        // Split input files
        auto chunks = split_input_files(config.input_files, config.files_per_job);

        // Create directories
        std::filesystem::create_directories(config.output_dir);
        std::filesystem::create_directories(config.log_dir);
        std::filesystem::create_directories("./batch_scripts");

        std::vector<std::string> job_ids;

        // Generate and submit scripts for each chunk
        for (size_t i = 0; i < chunks.size(); ++i) {
            std::string script_content;
            std::string script_file;
            std::string submit_command;

            switch (system) {
                case System::SLURM:
                    script_content = generate_slurm_script(config, i, chunks[i]);
                    script_file = "./batch_scripts/job_" + std::to_string(i) + ".slurm";
                    submit_command = "sbatch " + script_file;
                    break;

                case System::PBS:
                    script_content = generate_pbs_script(config, i, chunks[i]);
                    script_file = "./batch_scripts/job_" + std::to_string(i) + ".pbs";
                    submit_command = "qsub " + script_file;
                    break;

                case System::Condor:
                    script_content = generate_condor_submit(config, i, chunks[i]);
                    script_file = "./batch_scripts/job_" + std::to_string(i) + ".condor";
                    submit_command = "condor_submit " + script_file;
                    break;

                case System::Local:
                    // For local execution, just save the script
                    script_content = generate_slurm_script(config, i, chunks[i]);
                    script_file = "./batch_scripts/job_" + std::to_string(i) + ".sh";
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
            case System::SLURM: return "SLURM";
            case System::PBS: return "PBS";
            case System::Condor: return "Condor";
            case System::Local: return "Local";
            default: return "Unknown";
        }
    }
};

}  // namespace Framework
