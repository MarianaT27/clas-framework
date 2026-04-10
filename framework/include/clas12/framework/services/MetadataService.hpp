#pragma once

#include <fmt/format.h>
#include <chrono>
#include <ctime>
#include <map>
#include <string>
#include <vector>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include "TFile.h"
#include "TObjString.h"
#include "TTree.h"

namespace clas12::framework {

/**
 * @brief Service for storing analysis metadata and provenance
 *
 * Stores critical information about the analysis for reproducibility:
 * - Software version
 * - Run number and conditions
 * - Input file list
 * - Command line arguments
 * - Analysis parameters
 * - Timestamps
 * - Git commit hash
 *
 * Metadata is written to ROOT file as a TTree for easy access.
 *
 * Example:
 * @code
 * auto metadata = service<MetadataService>("MetadataService");
 * metadata->set_version("2.1.0");
 * metadata->set_run_number(5032);
 * metadata->add_input_file("data1.hipo");
 * metadata->add_parameter("cut_chi2pid", 3.0);
 * @endcode
 */
class MetadataService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "MetadataService"; }

    explicit MetadataService(std::string name = "MetadataService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        m_start_time = std::chrono::system_clock::now();
        // Optional dependency: OutputFileService preferred, but the service
        // can fall back to opening m_output_file directly if unavailable.
        if (!m_output_svc) {
            m_output_svc = locate_service<OutputFileService>("OutputFileService");
        }
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        m_end_time = std::chrono::system_clock::now();
        if (!m_output_file.empty()) {
            write_metadata();
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set output ROOT file (used as fallback if no OutputFileService)
     */
    auto set_output_file(const std::string& filename) -> void {
        m_output_file = filename;
    }

    /**
     * @brief Set OutputFileService to use for writing metadata
     *
     * When set, metadata will be written to the file managed by OutputFileService
     * instead of opening the file independently. This avoids file access conflicts.
     */
    auto set_output_file_service(OutputFileService* svc) -> void {
        m_output_svc = svc;
    }

    /**
     * @brief Set software version
     */
    auto set_version(const std::string& version) -> void {
        m_version = version;
    }

    /**
     * @brief Set git commit hash
     */
    auto set_git_hash(const std::string& hash) -> void {
        m_git_hash = hash;
    }

    /**
     * @brief Set run number
     */
    auto set_run_number(int run) -> void {
        m_run_number = run;
    }

    /**
     * @brief Add input file
     */
    auto add_input_file(const std::string& filename) -> void {
        m_input_files.push_back(filename);
    }

    /**
     * @brief Set command line arguments
     */
    auto set_command_line(int argc, char** argv) -> void {
        m_command_line.clear();
        for (int i = 0; i < argc; ++i) {
            m_command_line.push_back(argv[i]);
        }
    }

    /**
     * @brief Add analysis parameter
     */
    template <typename T>
    auto add_parameter(const std::string& name, T value) -> void {
        m_parameters[name] = fmt::format("{}", value);
    }

    /**
     * @brief Add custom metadata
     */
    auto add_metadata(const std::string& key, const std::string& value) -> void {
        m_custom_metadata[key] = value;
    }

    /**
     * @brief Set number of events processed
     */
    auto set_events_processed(size_t count) -> void {
        m_events_processed = count;
    }

    /**
     * @brief Set target type
     */
    auto set_target(const std::string& target) -> void {
        m_target = target;
    }

    /**
     * @brief Set beam energy
     */
    auto set_beam_energy(double energy) -> void {
        m_beam_energy = energy;
    }

    /**
     * @brief Print metadata summary
     */
    auto print_summary() const -> void {
        fmt::print("\n");
        fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
        fmt::print("║                    Analysis Metadata                           ║\n");
        fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
        fmt::print("\n");

        fmt::print("Software:\n");
        fmt::print("  Version: {}\n", m_version.empty() ? "unknown" : m_version);
        if (!m_git_hash.empty()) {
            fmt::print("  Git hash: {}\n", m_git_hash);
        }

        fmt::print("\nRun Information:\n");
        fmt::print("  Run number: {}\n", m_run_number);
        if (!m_target.empty()) {
            fmt::print("  Target: {}\n", m_target);
        }
        if (m_beam_energy > 0) {
            fmt::print("  Beam energy: {:.2f} GeV\n", m_beam_energy);
        }

        fmt::print("\nInput Files: {}\n", m_input_files.size());
        if (m_input_files.size() <= 5) {
            for (const auto& file : m_input_files) {
                fmt::print("  - {}\n", file);
            }
        } else {
            fmt::print("  - {} (showing first 3)\n", m_input_files[0]);
            fmt::print("  - {}\n", m_input_files[1]);
            fmt::print("  - {}\n", m_input_files[2]);
            fmt::print("  - ... ({} more)\n", m_input_files.size() - 3);
        }

        if (!m_parameters.empty()) {
            fmt::print("\nAnalysis Parameters:\n");
            for (const auto& [name, value] : m_parameters) {
                fmt::print("  {}: {}\n", name, value);
            }
        }

        fmt::print("\nTiming:\n");
        auto start_time_t = std::chrono::system_clock::to_time_t(m_start_time);
        auto end_time_t = std::chrono::system_clock::to_time_t(m_end_time);
        fmt::print("  Start: {}", std::ctime(&start_time_t));
        fmt::print("  End: {}", std::ctime(&end_time_t));

        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            m_end_time - m_start_time);
        fmt::print("  Duration: {} seconds\n", duration.count());
        fmt::print("  Events processed: {}\n", m_events_processed);

        fmt::print("\n");
    }

    /**
     * @brief Write metadata to ROOT file
     *
     * If OutputFileService is set, writes directly to its managed file.
     * Otherwise, opens the file in UPDATE mode (legacy behavior).
     */
    auto write_metadata() -> void {
        // Determine which file to use
        TFile* file_ptr = nullptr;
        std::unique_ptr<TFile> owned_file;

        if (m_output_svc && m_output_svc->is_file_open()) {
            // Use OutputFileService's file handle (preferred)
            file_ptr = m_output_svc->get_file();
            clas12::framework::Log::info("MetadataService", "Writing metadata to: {}",
                                 m_output_svc->get_output_file());
        } else if (!m_output_file.empty()) {
            // Fallback: open file directly (legacy behavior)
            clas12::framework::Log::info("MetadataService", "Writing metadata to: {}", m_output_file);
            owned_file = std::make_unique<TFile>(m_output_file.c_str(), "UPDATE");
            if (owned_file->IsZombie() || !owned_file->IsOpen()) {
                clas12::framework::Log::error("MetadataService", "Failed to open file for metadata");
                return;
            }
            file_ptr = owned_file.get();
        } else {
            return;
        }

        // Create metadata tree
        file_ptr->cd();
        TTree tree("metadata", "Analysis Metadata");

        // Branch variables
        std::string version = m_version;
        std::string git_hash = m_git_hash;
        int run_number = m_run_number;
        std::string target = m_target;
        double beam_energy = m_beam_energy;
        Long64_t events_processed = m_events_processed;
        Long64_t start_timestamp = std::chrono::system_clock::to_time_t(m_start_time);
        Long64_t end_timestamp = std::chrono::system_clock::to_time_t(m_end_time);
        Long64_t duration_sec = std::chrono::duration_cast<std::chrono::seconds>(
                                    m_end_time - m_start_time)
                                    .count();

        // Register branches
        tree.Branch("version", &version);
        tree.Branch("git_hash", &git_hash);
        tree.Branch("run_number", &run_number);
        tree.Branch("target", &target);
        tree.Branch("beam_energy", &beam_energy);
        tree.Branch("events_processed", &events_processed);
        tree.Branch("start_timestamp", &start_timestamp);
        tree.Branch("end_timestamp", &end_timestamp);
        tree.Branch("duration_sec", &duration_sec);

        // Fill tree (single entry)
        tree.Fill();

        // Write tree
        tree.Write();

        // Write additional metadata as TObjStrings
        file_ptr->cd();
        file_ptr->mkdir("metadata_details");
        file_ptr->cd("metadata_details");

        // Input files
        TObjString input_files_str(fmt::format("{}", fmt::join(m_input_files, "\n")).c_str());
        input_files_str.Write("input_files");

        // Command line
        if (!m_command_line.empty()) {
            TObjString cmd_str(fmt::format("{}", fmt::join(m_command_line, " ")).c_str());
            cmd_str.Write("command_line");
        }

        // Parameters
        for (const auto& [name, value] : m_parameters) {
            TObjString param_str(value.c_str());
            param_str.Write(fmt::format("param_{}", name).c_str());
        }

        // Custom metadata
        for (const auto& [key, value] : m_custom_metadata) {
            TObjString meta_str(value.c_str());
            meta_str.Write(key.c_str());
        }

        // Only close if we own the file
        if (owned_file) {
            owned_file->Close();
        }

        clas12::framework::Log::info("MetadataService", "Metadata written successfully");
    }

   private:
    OutputFileService* m_output_svc = nullptr;
    std::string m_output_file;
    std::string m_version;
    std::string m_git_hash;
    int m_run_number = 0;
    std::string m_target;
    double m_beam_energy = 0.0;
    size_t m_events_processed = 0;
    std::vector<std::string> m_input_files;
    std::vector<std::string> m_command_line;
    std::map<std::string, std::string> m_parameters;
    std::map<std::string, std::string> m_custom_metadata;
    std::chrono::system_clock::time_point m_start_time;
    std::chrono::system_clock::time_point m_end_time;
};

}  // namespace clas12::framework
