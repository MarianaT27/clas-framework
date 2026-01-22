#pragma once

#include <fmt/format.h>
#include <memory>
#include <mutex>
#include <string>
#include "Framework/IService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "TDirectory.h"
#include "TFile.h"

namespace Framework {

/**
 * @brief Service for managing the output ROOT file
 *
 * This service provides centralized management of the output ROOT file,
 * allowing multiple algorithms and services to write to the same file.
 * The file is opened during initialize() and kept open throughout the run,
 * then properly closed during finalize().
 * 
 * Usage:
 *   auto& out_svc = require_service<OutputFileService>("OutputFileService");
 *   
 *   // Get the file and write objects
 *   if (auto* file = out_svc.get()->get_file()) {
 *       out_svc.get()->cd("MyDirectory");  // Creates if doesn't exist
 *       myTree->Write();
 *   }
 */
class OutputFileService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "OutputFileService"; }

    explicit OutputFileService(std::string name = "OutputFileService")
        : Service(std::move(name)) {}

    ~OutputFileService() {
        close_file();
    }

    auto initialize() -> StatusCode override {
        if (m_output_file.empty()) {
            m_output_file = "output.root";
            Log::info(name(), "Using default output file 'output.root'");
        }

        // Open the file
        if (!open_file()) {
            Log::error(name(), "Failed to open output file: {}", m_output_file);
            return StatusCode::FAILURE();
        }

        Log::info(name(), "Opened output file: {}", m_output_file);
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        Log::info(name(), "Finalizing OutputFileService, closing file: {}", m_output_file);
        close_file();
        Log::info(name(), "Finalized - file closed successfully");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set the output ROOT file path (must be called before initialize)
     * @param filename Output file path
     */
    auto set_output_file(const std::string& filename) -> void {
        if (m_file) {
            Log::warning(name(), "Cannot change output file after initialization");
            return;
        }
        m_output_file = filename;
    }

    /**
     * @brief Get the output file path
     * @return Current output file path
     */
    auto get_output_file() const -> const std::string& {
        return m_output_file;
    }

    /**
     * @brief Get the open TFile pointer
     * @return TFile* pointer, or nullptr if file is not open
     * 
     * Note: The returned pointer is owned by this service. Do not delete it.
     * Use with lock_guard for thread safety in multi-threaded environments.
     */
    auto get_file() -> TFile* {
        return m_file.get();
    }

    /**
     * @brief Check if the file is open and valid
     */
    auto is_file_open() const -> bool {
        return m_file && !m_file->IsZombie() && m_file->IsOpen();
    }

    /**
     * @brief Get mutex for thread-safe file access
     * @return Reference to the file mutex
     * 
     * Usage:
     *   std::lock_guard<std::mutex> lock(out_svc.get()->mutex());
     *   auto* file = out_svc.get()->get_file();
     *   // ... write operations ...
     */
    auto mutex() -> std::mutex& {
        return m_mutex;
    }

    /**
     * @brief Change to a directory in the file, creating it if necessary
     * @param path Directory path (e.g., "Rho0" or "Analysis/Results")
     * @return true if successful, false otherwise
     * 
     * Thread-safe: acquires lock internally
     */
    auto cd(const std::string& path) -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);
        return cd_internal(path);
    }

    /**
     * @brief Change to root directory of the file
     * @return true if successful, false otherwise
     * 
     * Thread-safe: acquires lock internally
     */
    auto cd_root() -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!is_file_open()) return false;
        m_file->cd();
        return true;
    }

    /**
     * @brief Write an object to the file at the specified path
     * @param obj Object to write (TTree, TH1, etc.)
     * @param path Directory path where to write
     * @param option Write option (default: overwrite)
     * @return true if successful, false otherwise
     * 
     * Thread-safe: acquires lock internally
     */
    template<typename T>
    auto write(T* obj, const std::string& path = "", Int_t option = TObject::kOverwrite) -> bool {
        if (!obj) return false;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!is_file_open()) return false;

        if (!path.empty()) {
            if (!cd_internal(path)) return false;
        }

        obj->Write("", option);
        return true;
    }

    /**
     * @brief Create a directory structure and return to it
     * @param path Directory path to create
     * @return TDirectory* pointer to the directory, or nullptr on failure
     * 
     * Thread-safe: acquires lock internally
     */
    auto mkdir(const std::string& path) -> TDirectory* {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!is_file_open()) return nullptr;

        m_file->cd();
        return create_directory_path(path);
    }

   private:
    std::string m_output_file;
    std::unique_ptr<TFile> m_file;
    std::mutex m_mutex;

    auto open_file() -> bool {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Close any existing file first
        if (m_file) {
            m_file->Close();
            m_file.reset();
        }

        m_file = std::make_unique<TFile>(m_output_file.c_str(), "RECREATE");
        
        if (!m_file || m_file->IsZombie() || !m_file->IsOpen()) {
            m_file.reset();
            return false;
        }

        return true;
    }

    auto close_file() -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_file) {
            if (m_file->IsOpen()) {
                m_file->Write();  // Write any pending objects
                m_file->Close();
            }
            m_file.reset();
        }
    }

    /**
     * @brief Internal cd - assumes lock is already held
     */
    auto cd_internal(const std::string& path) -> bool {
        if (!is_file_open()) return false;
        if (path.empty()) {
            m_file->cd();
            return true;
        }

        // Try to cd to existing directory first
        if (m_file->cd(path.c_str())) {
            return true;
        }

        // Directory doesn't exist, create it
        m_file->cd();
        auto* dir = create_directory_path(path);
        return dir != nullptr;
    }

    /**
     * @brief Create nested directory structure
     * @param path Directory path like "Dir1/Dir2/Dir3"
     * @return TDirectory* to the final directory, or nullptr on failure
     */
    auto create_directory_path(const std::string& path) -> TDirectory* {
        if (path.empty()) return gDirectory;

        std::string clean_path = path;
        // Remove leading slash if present
        if (!clean_path.empty() && clean_path[0] == '/') {
            clean_path = clean_path.substr(1);
        }
        // Remove trailing slash if present
        if (!clean_path.empty() && clean_path.back() == '/') {
            clean_path.pop_back();
        }

        TDirectory* current = m_file.get();
        size_t start = 0;

        while (start < clean_path.length()) {
            size_t end = clean_path.find('/', start);
            if (end == std::string::npos) {
                end = clean_path.length();
            }

            std::string dir_name = clean_path.substr(start, end - start);
            if (dir_name.empty()) {
                start = end + 1;
                continue;
            }

            TDirectory* subdir = current->GetDirectory(dir_name.c_str());
            if (!subdir) {
                subdir = current->mkdir(dir_name.c_str());
                if (!subdir) {
                    Log::error(name(), "Failed to create directory: {}", dir_name);
                    return nullptr;
                }
            }
            current = subdir;
            current->cd();
            start = end + 1;
        }

        return current;
    }
};

}  // namespace Framework
