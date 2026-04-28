#pragma once

#include <fmt/format.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"

namespace framework {

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
        // Look up OutputFileService through the service manager (DI). The
        // declared dependency in ApplicationBuilder guarantees it has already
        // been initialized by the time we get here.
        if (!m_output_svc) {
            m_output_svc = locate_service<OutputFileService>("OutputFileService");
        }
        if (!m_output_svc) {
            Log::error(name(), "OutputFileService not available via ServiceManager");
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
        // Convert atomic histograms to ROOT format before writing
        merge_histograms();

        if (m_auto_write) {
            write_histograms();
        } else {
            Log::info(name(), "Auto-write disabled, histograms not written");
        }
        Log::info(name(), "Finalized");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Convert all atomic histograms to ROOT TH1D/TH2D format
     *
     * Called automatically during finalize(). Atomic histograms are
     * converted to ROOT format and stored in the regular histogram maps.
     */
    auto merge_histograms() -> void;

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

    // ========================================================================
    // Histogram Booking API (Thread-Local Accumulators)
    // ========================================================================
    //
    // `book_hist1D` / `book_hist2D` return ThreadLocalHistogram*D handles
    // that analyses fill during event processing. Each histogram keeps
    // per-worker bin arrays internally; `merge_histograms()` (called
    // automatically during finalize()) sums every worker's slot into a
    // single ROOT TH1D/TH2D that gets written to disk.
    //
    // `get_output_hist1D` / `get_output_hist2D` return the post-merge
    // ROOT histograms - only valid after `merge_histograms()` has run.

    /**
     * @brief Look up the merged ROOT TH1D for a previously-booked path.
     *
     * Returns nullptr before merge_histograms() has been called (i.e.
     * during the event loop). Intended for post-processing / test
     * assertions that need to inspect the final ROOT histogram.
     */
    auto get_output_hist1D(const std::string& path) -> TH1D* {
        auto it = m_hist1D.find(path);
        return (it != m_hist1D.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Look up the merged ROOT TH2D for a previously-booked path.
     */
    auto get_output_hist2D(const std::string& path) -> TH2D* {
        auto it = m_hist2D.find(path);
        return (it != m_hist2D.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Look up the merged ROOT TH3D for a previously-booked path.
     */
    auto get_output_hist3D(const std::string& path) -> TH3D* {
        auto it = m_hist3D.find(path);
        return (it != m_hist3D.end()) ? it->second.get() : nullptr;
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
                      int nbins, double xmin, double xmax) -> TProfile*;

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

    /**
     * @brief Write all histograms via OutputFileService
     */
    auto write_histograms() -> void;

    /**
     * @brief Enable/disable histogram verification after writing
     */
    auto set_verification(bool enable) -> void {
        m_verify_output = enable;
    }

    /**
     * @brief Clone a histogram with a new suffix
     *
     * Creates a copy of an existing histogram with a modified path.
     * Useful for creating sector-specific or cut-variation histograms.
     *
     * @param base_path Path to the original histogram
     * @param suffix Suffix to append (e.g., "_sector1", "_afterCut")
     * @return TH1D* Pointer to the cloned histogram, or nullptr if base not found
     */
    auto clone_hist1D_with_suffix(const std::string& base_path,
                                   const std::string& suffix) -> TH1D*;

    /**
     * @brief Clone a 2D histogram with a new suffix
     *
     * @param base_path Path to the original histogram
     * @param suffix Suffix to append
     * @return TH2D* Pointer to the cloned histogram, or nullptr if base not found
     */
    auto clone_hist2D_with_suffix(const std::string& base_path,
                                   const std::string& suffix) -> TH2D*;

    // ========================================================================
    // Lock-Free Atomic Histogram Booking (for true parallel accumulation)
    // ========================================================================

    /**
     * @brief Book a lock-free 1D atomic histogram
     *
     * Atomic histograms use compare-exchange operations for lock-free filling.
     * Multiple threads can fill simultaneously without any synchronization.
     * On finalize, atomic histograms are automatically converted to ROOT format.
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram
     */
    auto book_hist1D(const std::string& path, const std::string& title,
                            int nbins, double xmin, double xmax) -> ThreadLocalHistogram1D*;

    /**
     * @brief Book a lock-free 2D atomic histogram
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbinsx Number of x bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @param nbinsy Number of y bins
     * @param ymin Minimum y value
     * @param ymax Maximum y value
     * @return ThreadLocalHistogram2D* Pointer to the atomic histogram
     */
    auto book_hist2D(const std::string& path, const std::string& title,
                            int nbinsx, double xmin, double xmax,
                            int nbinsy, double ymin, double ymax) -> ThreadLocalHistogram2D*;

    /**
     * @brief Book a lock-free 3D atomic histogram.
     *
     * Use when the analysis would otherwise keep a stack of per-slice 2D
     * histograms keyed by some binned variable (momentum, theta, etc.).
     * Put the slicing variable on the X axis and project at analysis time
     * via `TH3::ProjectionYZ(..., binlo, binhi)`.
     */
    auto book_hist3D(const std::string& path, const std::string& title,
                     int nbinsx, double xmin, double xmax,
                     int nbinsy, double ymin, double ymax,
                     int nbinsz, double zmin, double zmax) -> ThreadLocalHistogram3D*;

    /**
     * @brief Get an atomic 1D histogram by path
     * @param path Histogram path
     * @return ThreadLocalHistogram1D* Pointer to histogram or nullptr if not found
     */
    auto get_hist1D(const std::string& path) -> ThreadLocalHistogram1D* {
        auto it = m_atomic_hist1D.find(path);
        return (it != m_atomic_hist1D.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Get an atomic 2D histogram by path
     * @param path Histogram path
     * @return ThreadLocalHistogram2D* Pointer to histogram or nullptr if not found
     */
    auto get_hist2D(const std::string& path) -> ThreadLocalHistogram2D* {
        auto it = m_atomic_hist2D.find(path);
        return (it != m_atomic_hist2D.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Get an atomic 3D histogram by path
     */
    auto get_hist3D(const std::string& path) -> ThreadLocalHistogram3D* {
        auto it = m_atomic_hist3D.find(path);
        return (it != m_atomic_hist3D.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Get number of booked atomic 1D histograms
     */
    auto num_atomic_hist1D() const -> size_t {
        return m_atomic_hist1D.size();
    }

    /**
     * @brief Get number of booked atomic 2D histograms
     */
    auto num_atomic_hist2D() const -> size_t {
        return m_atomic_hist2D.size();
    }

    /**
     * @brief Get number of booked atomic 3D histograms
     */
    auto num_atomic_hist3D() const -> size_t {
        return m_atomic_hist3D.size();
    }

    /**
     * @brief Get number of booked 3D histograms (post-merge)
     */
    auto num_hist3D() const -> size_t {
        return m_hist3D.size();
    }

   private:
    bool m_auto_write = true;     // Automatically write histograms on finalize
    bool m_verify_output = false; // Verify histograms after writing (disabled by default)
    std::map<std::string, std::unique_ptr<TH1D>> m_hist1D;
    std::map<std::string, std::unique_ptr<TH2D>> m_hist2D;
    std::map<std::string, std::unique_ptr<TH3D>> m_hist3D;
    std::map<std::string, std::unique_ptr<TProfile>> m_profiles;
    // Lock-free atomic histograms for parallel accumulation
    std::map<std::string, std::unique_ptr<ThreadLocalHistogram1D>> m_atomic_hist1D;
    std::map<std::string, std::unique_ptr<ThreadLocalHistogram2D>> m_atomic_hist2D;
    std::map<std::string, std::unique_ptr<ThreadLocalHistogram3D>> m_atomic_hist3D;
    // Metadata for atomic histograms (needed for ROOT conversion)
    struct AtomicHistMeta {
        std::string title;
    };
    std::map<std::string, AtomicHistMeta> m_atomic_hist1D_meta;
    std::map<std::string, AtomicHistMeta> m_atomic_hist2D_meta;
    std::map<std::string, AtomicHistMeta> m_atomic_hist3D_meta;
    OutputFileService* m_output_svc = nullptr;

    auto extract_name(const std::string& path) -> std::string;

    auto create_directories(TFile* file, const std::string& path) -> void;
};

}  // namespace framework
