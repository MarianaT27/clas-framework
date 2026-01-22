#pragma once

#include <fmt/format.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "Framework/IService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/OutputFileService.hpp"
#include "Framework/ThreadSafeHistogram.hpp"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"

namespace Framework {

// Forward declaration
class OutputFileService;

/**
 * @brief Service for managing ROOT histograms
 *
 * This service provides centralized histogram booking and management,
 * similar to the THistSvc in Gaudi/Athena. Histograms are organized
 * by path (e.g., "/ElectronKinematics/momentum") and automatically
 * written to the output file via OutputFileService.
 */
class HistogramService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "HistogramService"; }

    explicit HistogramService(std::string name = "HistogramService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        if (!m_output_svc) {
            Log::error(name(), "OutputFileService not set! Call set_output_file_service() before initialize()");
            return StatusCode::FAILURE();
        }

        if (!m_output_svc->is_file_open()) {
            Log::error(name(), "OutputFileService file is not open");
            return StatusCode::FAILURE();
        }

        Log::info(name(), "Initialized with OutputFileService");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        if (m_auto_write) {
            write_histograms();
        } else {
            Log::info(name(), "Auto-write disabled, histograms not written");
        }
        Log::info(name(), "Finalized");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set the OutputFileService (REQUIRED)
     * Must be called before initialize()
     */
    auto set_output_file_service(OutputFileService* svc) -> void {
        m_output_svc = svc;
    }

    /**
     * @brief Get the output file path from OutputFileService
     */
    auto get_output_file() const -> std::string {
        return m_output_svc ? m_output_svc->get_output_file() : "";
    }

    /**
     * @brief Enable/disable automatic histogram writing
     * @param enable If true, histograms will be written automatically on finalize
     */
    auto set_auto_write(bool enable) -> void {
        m_auto_write = enable;
    }

    /**
     * @brief Get number of booked 1D histograms
     */
    auto num_hist1D() const -> size_t {
        return m_hist1D.size();
    }

    /**
     * @brief Get number of booked 2D histograms
     */
    auto num_hist2D() const -> size_t {
        return m_hist2D.size();
    }

    /**
     * @brief Get number of booked profiles
     */
    auto num_profiles() const -> size_t {
        return m_profiles.size();
    }

    /**
     * @brief Book a 1D histogram
     * @param path Full path (e.g., "/ElectronKinematics/momentum")
     * @param title Histogram title with axis labels
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @return TH1D* Pointer to the histogram
     */
    auto book_hist1D(const std::string& path, const std::string& title, int nbins, double xmin, double xmax) -> TH1D* {
        if (m_hist1D.find(path) != m_hist1D.end()) {
            Log::warning(name(), "Histogram '{}' already exists", path);
            return m_hist1D[path].get();
        }

        std::string name = extract_name(path);
        auto hist = std::make_unique<TH1D>(name.c_str(), title.c_str(), nbins, xmin, xmax);
        hist->SetDirectory(nullptr);  // Don't let ROOT own it

        TH1D* ptr = hist.get();
        m_hist1D[path] = std::move(hist);
        return ptr;
    }

    /**
     * @brief Book a 2D histogram
     * @param path Full path
     * @param title Histogram title with axis labels
     * @param nbinsx Number of x bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @param nbinsy Number of y bins
     * @param ymin Minimum y value
     * @param ymax Maximum y value
     * @return TH2D* Pointer to the histogram
     */
    auto book_hist2D(const std::string& path, const std::string& title,
                     int nbinsx, double xmin, double xmax,
                     int nbinsy, double ymin, double ymax) -> TH2D* {
        if (m_hist2D.find(path) != m_hist2D.end()) {
            Log::warning(name(), "Histogram '{}' already exists", path);
            return m_hist2D[path].get();
        }

        std::string name = extract_name(path);
        auto hist = std::make_unique<TH2D>(name.c_str(), title.c_str(),
                                           nbinsx, xmin, xmax,
                                           nbinsy, ymin, ymax);
        hist->SetDirectory(nullptr);

        TH2D* ptr = hist.get();
        m_hist2D[path] = std::move(hist);
        return ptr;
    }

    /**
     * @brief Get a 1D histogram by path
     * @param path Histogram path
     * @return TH1D* Pointer to histogram, or nullptr if not found
     */
    auto get_hist1D(const std::string& path) -> TH1D* {
        auto it = m_hist1D.find(path);
        if (it != m_hist1D.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * @brief Get a 2D histogram by path
     * @param path Histogram path
     * @return TH2D* Pointer to histogram, or nullptr if not found
     */
    auto get_hist2D(const std::string& path) -> TH2D* {
        auto it = m_hist2D.find(path);
        if (it != m_hist2D.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    /**
     * @brief Book a TProfile histogram
     * @param path Full path
     * @param title Histogram title with axis labels
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @return TProfile* Pointer to the profile
     */
    auto book_profile(const std::string& path, const std::string& title,
                      int nbins, double xmin, double xmax) -> TProfile* {
        if (m_profiles.find(path) != m_profiles.end()) {
            Log::warning(name(), "Profile '{}' already exists", path);
            return m_profiles[path].get();
        }

        std::string name = extract_name(path);
        auto prof = std::make_unique<TProfile>(name.c_str(), title.c_str(),
                                               nbins, xmin, xmax);
        prof->SetDirectory(nullptr);

        TProfile* ptr = prof.get();
        m_profiles[path] = std::move(prof);
        return ptr;
    }

    /**
     * @brief Get a profile by path
     * @param path Profile path
     * @return TProfile* Pointer to profile, or nullptr if not found
     */
    auto get_profile(const std::string& path) -> TProfile* {
        auto it = m_profiles.find(path);
        if (it != m_profiles.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    // ========================================================================
    // Thread-Safe Histogram Booking Methods
    // ========================================================================

    /**
     * @brief Book a thread-safe 1D histogram
     *
     * Returns a ThreadSafeHist1D wrapper that provides mutex-protected fill operations.
     * Use this method when processing events in parallel.
     *
     * @param path Full path (e.g., "/ElectronKinematics/momentum")
     * @param title Histogram title with axis labels
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @return ThreadSafeHist1D* Pointer to thread-safe histogram wrapper
     */
    auto book_safe_hist1D(const std::string& path, const std::string& title,
                          int nbins, double xmin, double xmax) -> ThreadSafeHist1D* {
        auto* raw_hist = book_hist1D(path, title, nbins, xmin, xmax);
        if (!raw_hist) return nullptr;

        // Use piecewise_construct for in-place construction (mutex is non-copyable)
        m_safe_hist1D.emplace(std::piecewise_construct,
                              std::forward_as_tuple(path),
                              std::forward_as_tuple(raw_hist));
        return &m_safe_hist1D.at(path);
    }

    /**
     * @brief Book a thread-safe 2D histogram
     *
     * Returns a ThreadSafeHist2D wrapper that provides mutex-protected fill operations.
     *
     * @param path Full path
     * @param title Histogram title with axis labels
     * @param nbinsx Number of x bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @param nbinsy Number of y bins
     * @param ymin Minimum y value
     * @param ymax Maximum y value
     * @return ThreadSafeHist2D* Pointer to thread-safe histogram wrapper
     */
    auto book_safe_hist2D(const std::string& path, const std::string& title,
                          int nbinsx, double xmin, double xmax,
                          int nbinsy, double ymin, double ymax) -> ThreadSafeHist2D* {
        auto* raw_hist = book_hist2D(path, title, nbinsx, xmin, xmax, nbinsy, ymin, ymax);
        if (!raw_hist) return nullptr;

        // Use piecewise_construct for in-place construction (mutex is non-copyable)
        m_safe_hist2D.emplace(std::piecewise_construct,
                              std::forward_as_tuple(path),
                              std::forward_as_tuple(raw_hist));
        return &m_safe_hist2D.at(path);
    }

    /**
     * @brief Book a thread-safe TProfile
     *
     * Returns a ThreadSafeProfile wrapper that provides mutex-protected fill operations.
     *
     * @param path Full path
     * @param title Histogram title with axis labels
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @return ThreadSafeProfile* Pointer to thread-safe profile wrapper
     */
    auto book_safe_profile(const std::string& path, const std::string& title,
                           int nbins, double xmin, double xmax) -> ThreadSafeProfile* {
        auto* raw_prof = book_profile(path, title, nbins, xmin, xmax);
        if (!raw_prof) return nullptr;

        // Use piecewise_construct for in-place construction (mutex is non-copyable)
        m_safe_profiles.emplace(std::piecewise_construct,
                                std::forward_as_tuple(path),
                                std::forward_as_tuple(raw_prof));
        return &m_safe_profiles.at(path);
    }

    /**
     * @brief Get a thread-safe 1D histogram by path
     * @param path Histogram path
     * @return ThreadSafeHist1D* Pointer to wrapper, or nullptr if not found
     */
    auto get_safe_hist1D(const std::string& path) -> ThreadSafeHist1D* {
        auto it = m_safe_hist1D.find(path);
        return (it != m_safe_hist1D.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Get a thread-safe 2D histogram by path
     * @param path Histogram path
     * @return ThreadSafeHist2D* Pointer to wrapper, or nullptr if not found
     */
    auto get_safe_hist2D(const std::string& path) -> ThreadSafeHist2D* {
        auto it = m_safe_hist2D.find(path);
        return (it != m_safe_hist2D.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Get a thread-safe profile by path
     * @param path Profile path
     * @return ThreadSafeProfile* Pointer to wrapper, or nullptr if not found
     */
    auto get_safe_profile(const std::string& path) -> ThreadSafeProfile* {
        auto it = m_safe_profiles.find(path);
        return (it != m_safe_profiles.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Write all histograms via OutputFileService
     */
    auto write_histograms() -> void {
        if (!m_output_svc || !m_output_svc->is_file_open()) {
            Log::error(name(), "OutputFileService not available for writing histograms");
            return;
        }

        size_t total = m_hist1D.size() + m_hist2D.size() + m_profiles.size();
        if (total == 0) {
            Log::info(name(), "No histograms to write");
            return;
        }

        Log::info(name(), "Writing {} histograms via OutputFileService", total);

        // Use the service's mutex for thread safety
        std::lock_guard<std::mutex> lock(m_output_svc->mutex());
        TFile* file = m_output_svc->get_file();
        if (!file) return;

        // Create directory structure and write 1D histograms
        for (auto& [path, hist] : m_hist1D) {
            create_directories(file, path);
            hist->Write("", TObject::kOverwrite);
        }

        // Write 2D histograms
        for (auto& [path, hist] : m_hist2D) {
            create_directories(file, path);
            hist->Write("", TObject::kOverwrite);
        }

        // Write profiles
        for (auto& [path, prof] : m_profiles) {
            create_directories(file, path);
            prof->Write("", TObject::kOverwrite);
        }

        Log::info(name(), "Wrote {} 1D, {} 2D histograms, and {} profiles",
                  m_hist1D.size(), m_hist2D.size(), m_profiles.size());
    }

    /**
     * @brief Enable/disable histogram verification after writing
     */
    auto set_verification(bool enable) -> void {
        m_verify_output = enable;
    }

   private:
    bool m_auto_write = true;     // Automatically write histograms on finalize
    bool m_verify_output = false; // Verify histograms after writing (disabled by default)
    std::map<std::string, std::unique_ptr<TH1D>> m_hist1D;
    std::map<std::string, std::unique_ptr<TH2D>> m_hist2D;
    std::map<std::string, std::unique_ptr<TProfile>> m_profiles;
    // Thread-safe wrappers (stored by value, not unique_ptr, since they wrap raw pointers)
    std::map<std::string, ThreadSafeHist1D> m_safe_hist1D;
    std::map<std::string, ThreadSafeHist2D> m_safe_hist2D;
    std::map<std::string, ThreadSafeProfile> m_safe_profiles;
    OutputFileService* m_output_svc = nullptr;

    auto extract_name(const std::string& path) -> std::string {
        size_t pos = path.find_last_of('/');
        if (pos != std::string::npos) {
            return path.substr(pos + 1);
        }
        return path;
    }

    auto create_directories(TFile* file, const std::string& path) -> void {
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos || pos == 0) {
            file->cd();
            return;
        }

        std::string dir_path = path.substr(0, pos);
        if (dir_path[0] == '/') {
            dir_path = dir_path.substr(1);
        }

        // Navigate or create directory structure
        file->cd();
        std::string current_dir;
        size_t start = 0;
        while (start < dir_path.length()) {
            size_t end = dir_path.find('/', start);
            if (end == std::string::npos) {
                end = dir_path.length();
            }
            std::string dir_name = dir_path.substr(start, end - start);
            if (!gDirectory->GetDirectory(dir_name.c_str())) {
                gDirectory->mkdir(dir_name.c_str());
            }
            gDirectory->cd(dir_name.c_str());
            start = end + 1;
        }
    }
};

}  // namespace Framework
