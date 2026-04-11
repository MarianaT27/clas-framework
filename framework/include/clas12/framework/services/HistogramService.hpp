#pragma once

#include <array>
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
#include "TProfile.h"

namespace clas12::framework {

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

    // ========================================================================
    // Physics-Specific Histogram Templates (Thread-Safe Atomic by Default)
    // ========================================================================
    //
    // All physics-specific booking methods now use atomic histograms by default
    // for thread-safe filling. These are converted to ROOT format on finalize.

    /**
     * @brief Book a momentum histogram with standard CLAS12 binning (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title (";Momentum [GeV/c];Counts" added automatically)
     * @param nbins Number of bins (default: 100)
     * @param pmin Minimum momentum (default: 0 GeV/c)
     * @param pmax Maximum momentum (default: 11 GeV/c, typical beam energy)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram
     */
    auto book_momentum_hist(const std::string& path, const std::string& title,
                            int nbins = 100, double pmin = 0.0, double pmax = 11.0) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";Momentum [GeV/c];Counts";
        return book_hist1D(path, full_title, nbins, pmin, pmax);
    }

    /**
     * @brief Book a theta angle histogram with standard binning (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins (default: 180)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram (0-180 degrees)
     */
    auto book_theta_hist(const std::string& path, const std::string& title,
                         int nbins = 180) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";#theta [deg];Counts";
        return book_hist1D(path, full_title, nbins, 0.0, 180.0);
    }

    /**
     * @brief Book a phi angle histogram with standard binning (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins (default: 360)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram (-180 to 180 degrees)
     */
    auto book_phi_hist(const std::string& path, const std::string& title,
                       int nbins = 360) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";#phi [deg];Counts";
        return book_hist1D(path, full_title, nbins, -180.0, 180.0);
    }

    /**
     * @brief Book an energy histogram with standard binning (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins (default: 100)
     * @param emin Minimum energy (default: 0 GeV)
     * @param emax Maximum energy (default: 11 GeV)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram
     */
    auto book_energy_hist(const std::string& path, const std::string& title,
                          int nbins = 100, double emin = 0.0, double emax = 11.0) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";Energy [GeV];Counts";
        return book_hist1D(path, full_title, nbins, emin, emax);
    }

    /**
     * @brief Book an invariant mass histogram (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins
     * @param mmin Minimum mass [GeV/c²]
     * @param mmax Maximum mass [GeV/c²]
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram
     */
    auto book_mass_hist(const std::string& path, const std::string& title,
                        int nbins, double mmin, double mmax) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";M [GeV/c^{2}];Counts";
        return book_hist1D(path, full_title, nbins, mmin, mmax);
    }

    /**
     * @brief Book a vertex Z histogram with standard binning (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins (default: 100)
     * @param zmin Minimum Z (default: -40 cm)
     * @param zmax Maximum Z (default: 40 cm)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram
     */
    auto book_vertex_z_hist(const std::string& path, const std::string& title,
                            int nbins = 100, double zmin = -40.0, double zmax = 40.0) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";Vertex Z [cm];Counts";
        return book_hist1D(path, full_title, nbins, zmin, zmax);
    }

    /**
     * @brief Book a Q² histogram with standard binning for DIS (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins (default: 100)
     * @param q2min Minimum Q² (default: 0 GeV²)
     * @param q2max Maximum Q² (default: 12 GeV²)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram
     */
    auto book_q2_hist(const std::string& path, const std::string& title,
                      int nbins = 100, double q2min = 0.0, double q2max = 12.0) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";Q^{2} [GeV^{2}];Counts";
        return book_hist1D(path, full_title, nbins, q2min, q2max);
    }

    /**
     * @brief Book an x_B (Bjorken x) histogram (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins (default: 100)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram (0 to 1)
     */
    auto book_xb_hist(const std::string& path, const std::string& title,
                      int nbins = 100) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";x_{B};Counts";
        return book_hist1D(path, full_title, nbins, 0.0, 1.0);
    }

    /**
     * @brief Book a W (invariant mass of hadronic system) histogram (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbins Number of bins (default: 100)
     * @param wmin Minimum W (default: 0 GeV)
     * @param wmax Maximum W (default: 5 GeV)
     * @return ThreadLocalHistogram1D* Pointer to the atomic histogram
     */
    auto book_w_hist(const std::string& path, const std::string& title,
                     int nbins = 100, double wmin = 0.0, double wmax = 5.0) -> ThreadLocalHistogram1D* {
        std::string full_title = title + ";W [GeV];Counts";
        return book_hist1D(path, full_title, nbins, wmin, wmax);
    }

    /**
     * @brief Book a 2D kinematic histogram (theta vs momentum) (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbinsp Number of momentum bins (default: 50)
     * @param pmin Minimum momentum (default: 0)
     * @param pmax Maximum momentum (default: 11)
     * @param nbinsth Number of theta bins (default: 90)
     * @return ThreadLocalHistogram2D* Pointer to the atomic histogram
     */
    auto book_theta_vs_p(const std::string& path, const std::string& title,
                         int nbinsp = 50, double pmin = 0.0, double pmax = 11.0,
                         int nbinsth = 90) -> ThreadLocalHistogram2D* {
        std::string full_title = title + ";Momentum [GeV/c];#theta [deg];Counts";
        return book_hist2D(path, full_title, nbinsp, pmin, pmax, nbinsth, 0.0, 180.0);
    }

    /**
     * @brief Book a 2D kinematic histogram (phi vs theta) (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbinsth Number of theta bins (default: 90)
     * @param nbinsphi Number of phi bins (default: 180)
     * @return ThreadLocalHistogram2D* Pointer to the atomic histogram
     */
    auto book_phi_vs_theta(const std::string& path, const std::string& title,
                           int nbinsth = 90, int nbinsphi = 180) -> ThreadLocalHistogram2D* {
        std::string full_title = title + ";#theta [deg];#phi [deg];Counts";
        return book_hist2D(path, full_title, nbinsth, 0.0, 180.0, nbinsphi, -180.0, 180.0);
    }

    /**
     * @brief Book a 2D DIS kinematic histogram (Q² vs x_B) (atomic/thread-safe)
     *
     * @param path Full path for the histogram
     * @param title Histogram title
     * @param nbinsxb Number of x_B bins (default: 50)
     * @param nbinsq2 Number of Q² bins (default: 50)
     * @param q2max Maximum Q² (default: 12)
     * @return ThreadLocalHistogram2D* Pointer to the atomic histogram
     */
    auto book_q2_vs_xb(const std::string& path, const std::string& title,
                       int nbinsxb = 50, int nbinsq2 = 50, double q2max = 12.0) -> ThreadLocalHistogram2D* {
        std::string full_title = title + ";x_{B};Q^{2} [GeV^{2}];Counts";
        return book_hist2D(path, full_title, nbinsxb, 0.0, 1.0, nbinsq2, 0.0, q2max);
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

    /**
     * @brief Book histograms for all 6 sectors
     *
     * Creates 6 copies of a histogram template, one for each CLAS12 sector.
     *
     * @param base_path Base path (e.g., "/Electrons/momentum")
     * @param title Base title
     * @param nbins Number of bins
     * @param xmin Minimum x value
     * @param xmax Maximum x value
     * @return std::array<TH1D*, 6> Array of pointers to sector histograms
     */
    auto book_sector_hist1D(const std::string& base_path, const std::string& title,
                            int nbins, double xmin, double xmax) -> std::array<ThreadLocalHistogram1D*, 6> {
        std::array<ThreadLocalHistogram1D*, 6> hists{};
        for (int sector = 1; sector <= 6; ++sector) {
            std::string path = fmt::format("{}_sector{}", base_path, sector);
            std::string sect_title = fmt::format("{} (Sector {})", title, sector);
            hists[sector - 1] = book_hist1D(path, sect_title, nbins, xmin, xmax);
        }
        return hists;
    }

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

   private:
    bool m_auto_write = true;     // Automatically write histograms on finalize
    bool m_verify_output = false; // Verify histograms after writing (disabled by default)
    std::map<std::string, std::unique_ptr<TH1D>> m_hist1D;
    std::map<std::string, std::unique_ptr<TH2D>> m_hist2D;
    std::map<std::string, std::unique_ptr<TProfile>> m_profiles;
    // Lock-free atomic histograms for parallel accumulation
    std::map<std::string, std::unique_ptr<ThreadLocalHistogram1D>> m_atomic_hist1D;
    std::map<std::string, std::unique_ptr<ThreadLocalHistogram2D>> m_atomic_hist2D;
    // Metadata for atomic histograms (needed for ROOT conversion)
    struct AtomicHistMeta {
        std::string title;
    };
    std::map<std::string, AtomicHistMeta> m_atomic_hist1D_meta;
    std::map<std::string, AtomicHistMeta> m_atomic_hist2D_meta;
    OutputFileService* m_output_svc = nullptr;

    auto extract_name(const std::string& path) -> std::string;

    auto create_directories(TFile* file, const std::string& path) -> void;
};

}  // namespace clas12::framework
