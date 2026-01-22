#pragma once

#include <fmt/format.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "Framework/IService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "TFile.h"
#include "TObject.h"

namespace Framework {

/**
 * @brief Service for centralized ROOT file I/O management
 *
 * This service provides centralized ROOT file management for:
 * - Managing multiple output ROOT files
 * - Coordinating file access between HistogramService and NTupleService
 * - Thread-safe file operations
 * - Automatic file lifecycle management
 *
 * Usage:
 * @code
 * auto root_svc = std::make_unique<RootFileService>();
 * root_svc->set_default_output("analysis.root");
 * root_svc->open_file("secondary", "secondary_output.root", "RECREATE");
 * app.service_manager().add_service(std::move(root_svc));
 * @endcode
 */
class RootFileService : public Service {
public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "RootFileService"; }

    explicit RootFileService(std::string name = "RootFileService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        Log::info(name(), "Initializing RootFileService");

        // Open default output file if specified
        if (!m_default_output.empty()) {
            if (!open_file("default", m_default_output, "RECREATE")) {
                return StatusCode::FAILURE();
            }
        }

        Log::info(name(), "Initialized with {} file(s)", m_files.size());

        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        Log::info(name(), "Finalizing - closing {} file(s)", m_files.size());

        // Close all open files
        close_all_files();

        Log::info(name(), "Finalized");

        return StatusCode::SUCCESS();
    }

    // ========== Configuration ==========

    /**
     * @brief Set the default output file path
     * @param filename Path to the default output ROOT file
     */
    auto set_default_output(const std::string& filename) -> void {
        m_default_output = filename;
    }

    /**
     * @brief Get the default output file path
     */
    auto get_default_output() const -> const std::string& {
        return m_default_output;
    }

    // ========== File Operations ==========

    /**
     * @brief Open a ROOT file with a given alias
     * @param alias Unique identifier for this file
     * @param filename Path to the ROOT file
     * @param option ROOT file open option (RECREATE, UPDATE, READ, etc.)
     * @param compression Compression level (0-9, default 1)
     * @return true if successful
     */
    auto open_file(const std::string& alias, const std::string& filename,
                   const std::string& option = "RECREATE", int compression = 1) -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Check if alias already exists
        if (m_files.find(alias) != m_files.end()) {
            Log::warning(name(), "File alias '{}' already exists, closing existing file", alias);
            close_file_internal(alias);
        }

        try {
            auto file = std::make_unique<TFile>(filename.c_str(), option.c_str(), "", compression);

            if (file->IsZombie() || !file->IsOpen()) {
                Log::error(name(), "Failed to open file: {}", filename);
                return false;
            }

            m_files[alias] = std::move(file);
            m_file_paths[alias] = filename;

            Log::info(name(), "Opened file '{}': {} ({})", alias, filename, option);

            return true;
        } catch (const std::exception& e) {
            Log::error(name(), "Exception opening file {}: {}", filename, e.what());
            return false;
        }
    }

    /**
     * @brief Get a file by alias
     * @param alias File alias (use "default" for the default output file)
     * @return TFile* pointer, or nullptr if not found
     */
    auto get_file(const std::string& alias = "default") -> TFile* {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_files.find(alias);
        if (it != m_files.end() && it->second && it->second->IsOpen()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * @brief Check if a file is open
     * @param alias File alias
     * @return true if file exists and is open
     */
    auto is_file_open(const std::string& alias = "default") const -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_files.find(alias);
        return it != m_files.end() && it->second && it->second->IsOpen();
    }

    /**
     * @brief Close a specific file
     * @param alias File alias
     * @return true if successful
     */
    auto close_file(const std::string& alias) -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);
        return close_file_internal(alias);
    }

    /**
     * @brief Close all open files
     */
    auto close_all_files() -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto& [alias, file] : m_files) {
            if (file && file->IsOpen()) {
                file->Write();
                file->Close();
                Log::info(name(), "Closed file '{}': {}", alias, m_file_paths[alias]);
            }
        }

        m_files.clear();
        m_file_paths.clear();
    }

    /**
     * @brief Write all objects to a file
     * @param alias File alias
     * @return true if successful
     */
    auto write_file(const std::string& alias = "default") -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_files.find(alias);
        if (it == m_files.end() || !it->second || !it->second->IsOpen()) {
            Log::error(name(), "Cannot write: file '{}' not open", alias);
            return false;
        }

        it->second->Write();
        Log::debug(name(), "Wrote file '{}'", alias);
        return true;
    }

    /**
     * @brief Change to a file's directory (for writing objects)
     * @param alias File alias
     * @return true if successful
     */
    auto cd(const std::string& alias = "default") -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_files.find(alias);
        if (it == m_files.end() || !it->second || !it->second->IsOpen()) {
            return false;
        }

        it->second->cd();
        return true;
    }

    /**
     * @brief Create or navigate to a directory within a file
     * @param alias File alias
     * @param dir_path Directory path (e.g., "Electrons/Kinematics")
     * @return true if successful
     */
    auto mkdir(const std::string& alias, const std::string& dir_path) -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_files.find(alias);
        if (it == m_files.end() || !it->second || !it->second->IsOpen()) {
            return false;
        }

        TFile* file = it->second.get();
        file->cd();

        // Navigate or create directory structure
        std::string current_path;
        size_t start = 0;
        while (start < dir_path.length()) {
            size_t end = dir_path.find('/', start);
            if (end == std::string::npos) {
                end = dir_path.length();
            }
            std::string dir_name = dir_path.substr(start, end - start);
            if (!dir_name.empty()) {
                if (!gDirectory->GetDirectory(dir_name.c_str())) {
                    gDirectory->mkdir(dir_name.c_str());
                }
                gDirectory->cd(dir_name.c_str());
            }
            start = end + 1;
        }

        return true;
    }

    /**
     * @brief Write an object to a file
     * @param alias File alias
     * @param obj Object to write
     * @param name Object name (optional, uses object's name if empty)
     * @return true if successful
     */
    auto write_object(const std::string& alias, TObject* obj,
                      const std::string& obj_name = "") -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_files.find(alias);
        if (it == m_files.end() || !it->second || !it->second->IsOpen()) {
            Log::error(name(), "Cannot write object: file '{}' not open", alias);
            return false;
        }

        it->second->cd();
        if (obj_name.empty()) {
            obj->Write();
        } else {
            obj->Write(obj_name.c_str());
        }

        return true;
    }

    // ========== Statistics ==========

    /**
     * @brief Get number of open files
     */
    auto num_files() const -> size_t {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_files.size();
    }

    /**
     * @brief Get all file aliases
     */
    auto get_file_aliases() const -> std::vector<std::string> {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> aliases;
        for (const auto& [alias, file] : m_files) {
            aliases.push_back(alias);
        }
        return aliases;
    }

    /**
     * @brief Get file path for an alias
     */
    auto get_file_path(const std::string& alias) const -> std::string {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_file_paths.find(alias);
        return (it != m_file_paths.end()) ? it->second : "";
    }

    /**
     * @brief Print summary of managed files
     */
    auto print_summary() const -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        fmt::print("\nRootFileService Summary:\n");
        fmt::print("  Default output: {}\n", m_default_output.empty() ? "(none)" : m_default_output);
        fmt::print("  Open files: {}\n", m_files.size());

        if (!m_files.empty()) {
            fmt::print("\n  File Details:\n");
            for (const auto& [alias, file] : m_files) {
                std::string status = (file && file->IsOpen()) ? "open" : "closed";
                auto path_it = m_file_paths.find(alias);
                std::string path = (path_it != m_file_paths.end()) ? path_it->second : "(unknown)";
                fmt::print("    {:<15} : {} ({})\n", alias, path, status);
            }
        }
        fmt::print("\n");
    }

private:
    std::string m_default_output;
    std::map<std::string, std::unique_ptr<TFile>> m_files;
    std::map<std::string, std::string> m_file_paths;
    mutable std::mutex m_mutex;

    auto close_file_internal(const std::string& alias) -> bool {
        auto it = m_files.find(alias);
        if (it == m_files.end()) {
            return false;
        }

        if (it->second && it->second->IsOpen()) {
            it->second->Write();
            it->second->Close();
            Log::info(name(), "Closed file '{}': {}", alias, m_file_paths[alias]);
        }

        m_files.erase(it);
        m_file_paths.erase(alias);
        return true;
    }
};

}  // namespace Framework
