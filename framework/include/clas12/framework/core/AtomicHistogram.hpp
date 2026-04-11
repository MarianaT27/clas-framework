#pragma once

/**
 * @file AtomicHistogram.hpp
 * @brief Lock-free histogram implementations for parallel event processing
 *
 * Provides atomic histogram classes that can be filled from multiple threads
 * without mutex contention. Uses atomic operations for bin accumulation,
 * then merges into ROOT histograms for output.
 */

#include <atomic>
#include <cmath>
#include <cstddef>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>

namespace clas12::framework {

/**
 * @brief Lock-free 1D histogram for parallel accumulation
 *
 * Uses atomic doubles for bin contents and sum of weights squared.
 * Multiple threads can fill simultaneously without locks.
 * After processing, merge into a ROOT TH1D for output.
 *
 * Example usage:
 * @code
 * AtomicHistogram1D hist(100, 0.0, 10.0);
 *
 * // Fill from multiple threads (no synchronization needed)
 * hist.fill(value);
 * hist.fill(value, weight);
 *
 * // Merge into ROOT histogram for output
 * TH1D* root_hist = new TH1D("name", "title", 100, 0, 10);
 * hist.merge_into(root_hist);
 * @endcode
 */
class AtomicHistogram1D {
   public:
    /**
     * @brief Construct an AtomicHistogram1D
     *
     * @param nbins Number of bins
     * @param xmin Lower edge of first bin
     * @param xmax Upper edge of last bin
     */
    AtomicHistogram1D(int nbins, double xmin, double xmax)
        : m_nbins(nbins)
        , m_xmin(xmin)
        , m_xmax(xmax)
        , m_bin_width((xmax - xmin) / nbins)
        , m_bins(nbins + 2)         // +2 for underflow and overflow
        , m_sum_w2(nbins + 2)
        , m_entries(0) {
        // Initialize all atomic values to zero
        for (auto& bin : m_bins) {
            bin.store(0.0, std::memory_order_relaxed);
        }
        for (auto& sw2 : m_sum_w2) {
            sw2.store(0.0, std::memory_order_relaxed);
        }
    }

    /**
     * @brief Fill the histogram with a value
     *
     * Thread-safe: can be called from multiple threads simultaneously.
     * Silently skips NaN values.
     *
     * @param x Value to fill
     * @param weight Optional weight (default 1.0)
     */
    auto fill(double x, double weight = 1.0) -> void {
        int bin = find_bin(x);
        if (bin < 0) return;  // Skip NaN values
        atomic_add(m_bins[bin], weight);
        atomic_add(m_sum_w2[bin], weight * weight);
        m_entries.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Vectorised fill from a contiguous span of values.
     *
     * Equivalent to calling `fill(v)` once per element, but avoids the
     * per-call function overhead and lets the compiler hoist a local
     * accumulator per bin out of the inner loop when the values are
     * statistically spread across bins. Thread-safe.
     */
    template <typename T>
    auto fill_span(std::span<const T> values) -> void {
        for (T v : values) fill(static_cast<double>(v));
    }

    /**
     * @brief Vectorised weighted fill from parallel spans of values + weights.
     *
     * `values` and `weights` must be the same length. Thread-safe.
     */
    template <typename T, typename W>
    auto fill_span(std::span<const T> values, std::span<const W> weights) -> void {
        const std::size_t n = std::min(values.size(), weights.size());
        for (std::size_t i = 0; i < n; ++i) {
            fill(static_cast<double>(values[i]), static_cast<double>(weights[i]));
        }
    }

    /**
     * @brief Get the bin content
     * @param bin Bin index (0 = underflow, nbins+1 = overflow)
     * @return double Bin content
     */
    [[nodiscard]] auto get_bin_content(int bin) const -> double {
        if (bin < 0 || bin >= static_cast<int>(m_bins.size())) {
            return 0.0;
        }
        return m_bins[bin].load(std::memory_order_relaxed);
    }

    /**
     * @brief Get the bin error (sqrt of sum of weights squared)
     * @param bin Bin index
     * @return double Bin error
     */
    [[nodiscard]] auto get_bin_error(int bin) const -> double {
        if (bin < 0 || bin >= static_cast<int>(m_sum_w2.size())) {
            return 0.0;
        }
        return std::sqrt(m_sum_w2[bin].load(std::memory_order_relaxed));
    }

    /**
     * @brief Get the number of entries
     * @return std::size_t Number of fill calls
     */
    [[nodiscard]] auto entries() const -> std::size_t {
        return m_entries.load(std::memory_order_relaxed);
    }

    /**
     * @brief Get the number of bins
     * @return int Number of bins (not including under/overflow)
     */
    [[nodiscard]] auto nbins() const -> int { return m_nbins; }

    /**
     * @brief Get the lower edge
     * @return double X minimum
     */
    [[nodiscard]] auto xmin() const -> double { return m_xmin; }

    /**
     * @brief Get the upper edge
     * @return double X maximum
     */
    [[nodiscard]] auto xmax() const -> double { return m_xmax; }

    /**
     * @brief Merge accumulated data into a ROOT TH1D
     *
     * Adds the atomic histogram contents to the ROOT histogram.
     * The ROOT histogram should have compatible binning.
     *
     * @param target Target ROOT histogram to merge into
     */
    auto merge_into(TH1D* target) const -> void {
        if (!target) return;

        // Verify compatible binning
        if (target->GetNbinsX() != m_nbins) {
            return;  // Incompatible binning
        }

        // Add contents to target
        for (int i = 0; i <= m_nbins + 1; ++i) {
            double content = m_bins[i].load(std::memory_order_relaxed);
            double error2 = m_sum_w2[i].load(std::memory_order_relaxed);

            if (content != 0.0) {
                target->SetBinContent(i, target->GetBinContent(i) + content);
                double old_error = target->GetBinError(i);
                target->SetBinError(i, std::sqrt(old_error * old_error + error2));
            }
        }

        // Update entries
        target->SetEntries(target->GetEntries() + static_cast<double>(entries()));
    }

    /**
     * @brief Create a ROOT TH1D from this histogram
     *
     * @param name Histogram name
     * @param title Histogram title
     * @return std::unique_ptr<TH1D> New ROOT histogram with copied data
     */
    [[nodiscard]] auto to_root(const std::string& name,
                                const std::string& title) const -> std::unique_ptr<TH1D> {
        auto hist = std::make_unique<TH1D>(name.c_str(), title.c_str(),
                                            m_nbins, m_xmin, m_xmax);

        for (int i = 0; i <= m_nbins + 1; ++i) {
            hist->SetBinContent(i, m_bins[i].load(std::memory_order_relaxed));
            hist->SetBinError(i, std::sqrt(m_sum_w2[i].load(std::memory_order_relaxed)));
        }
        hist->SetEntries(static_cast<double>(entries()));

        return hist;
    }

    /**
     * @brief Reset the histogram to zero
     */
    auto reset() -> void {
        for (auto& bin : m_bins) {
            bin.store(0.0, std::memory_order_relaxed);
        }
        for (auto& sw2 : m_sum_w2) {
            sw2.store(0.0, std::memory_order_relaxed);
        }
        m_entries.store(0, std::memory_order_relaxed);
    }

   private:
    int m_nbins;
    double m_xmin;
    double m_xmax;
    double m_bin_width;

    std::vector<std::atomic<double>> m_bins;
    std::vector<std::atomic<double>> m_sum_w2;
    std::atomic<std::size_t> m_entries;

    /**
     * @brief Find the bin index for a value
     *
     * @param x Value
     * @return int Bin index (0 = underflow, nbins+1 = overflow, -1 if NaN)
     */
    [[nodiscard]] auto find_bin(double x) const -> int {
        // Handle NaN - return -1 to signal skip
        if (std::isnan(x)) return -1;

        if (x < m_xmin) return 0;                // underflow
        if (x >= m_xmax) return m_nbins + 1;     // overflow

        int bin = static_cast<int>((x - m_xmin) / m_bin_width) + 1;
        return std::min(bin, m_nbins);
    }

    /**
     * @brief Atomically add a value to an atomic double
     *
     * Uses compare-exchange loop for atomic floating-point addition.
     */
    static auto atomic_add(std::atomic<double>& target, double value) -> void {
        double current = target.load(std::memory_order_relaxed);
        while (!target.compare_exchange_weak(current, current + value,
                                              std::memory_order_relaxed,
                                              std::memory_order_relaxed)) {
            // current is updated by compare_exchange_weak on failure
        }
    }
};

/**
 * @brief Lock-free 2D histogram for parallel accumulation
 *
 * Uses atomic doubles for bin contents and sum of weights squared.
 * Multiple threads can fill simultaneously without locks.
 */
class AtomicHistogram2D {
   public:
    /**
     * @brief Construct an AtomicHistogram2D
     *
     * @param nbinsx Number of bins in X
     * @param xmin Lower edge of first X bin
     * @param xmax Upper edge of last X bin
     * @param nbinsy Number of bins in Y
     * @param ymin Lower edge of first Y bin
     * @param ymax Upper edge of last Y bin
     */
    AtomicHistogram2D(int nbinsx, double xmin, double xmax,
                      int nbinsy, double ymin, double ymax)
        : m_nbinsx(nbinsx)
        , m_nbinsy(nbinsy)
        , m_xmin(xmin)
        , m_xmax(xmax)
        , m_ymin(ymin)
        , m_ymax(ymax)
        , m_bin_width_x((xmax - xmin) / nbinsx)
        , m_bin_width_y((ymax - ymin) / nbinsy)
        , m_total_bins((nbinsx + 2) * (nbinsy + 2))
        , m_bins(m_total_bins)
        , m_sum_w2(m_total_bins)
        , m_entries(0) {
        // Initialize all atomic values to zero
        for (auto& bin : m_bins) {
            bin.store(0.0, std::memory_order_relaxed);
        }
        for (auto& sw2 : m_sum_w2) {
            sw2.store(0.0, std::memory_order_relaxed);
        }
    }

    /**
     * @brief Fill the histogram with a value
     *
     * Thread-safe: can be called from multiple threads simultaneously.
     * Silently skips NaN values.
     *
     * @param x X value
     * @param y Y value
     * @param weight Optional weight (default 1.0)
     */
    auto fill(double x, double y, double weight = 1.0) -> void {
        int bin = find_bin(x, y);
        if (bin < 0) return;  // Skip NaN values
        atomic_add(m_bins[bin], weight);
        atomic_add(m_sum_w2[bin], weight * weight);
        m_entries.fetch_add(1, std::memory_order_relaxed);
    }

    /**
     * @brief Vectorised fill from parallel spans of x and y coordinates.
     *
     * `xs` and `ys` must be the same length. Thread-safe.
     */
    template <typename TX, typename TY>
    auto fill_span(std::span<const TX> xs, std::span<const TY> ys) -> void {
        const std::size_t n = std::min(xs.size(), ys.size());
        for (std::size_t i = 0; i < n; ++i) {
            fill(static_cast<double>(xs[i]), static_cast<double>(ys[i]));
        }
    }

    /**
     * @brief Vectorised weighted fill from parallel spans of x, y, weight.
     *
     * `xs`, `ys`, and `weights` must be the same length. Thread-safe.
     */
    template <typename TX, typename TY, typename W>
    auto fill_span(std::span<const TX> xs, std::span<const TY> ys,
                   std::span<const W> weights) -> void {
        const std::size_t n = std::min({xs.size(), ys.size(), weights.size()});
        for (std::size_t i = 0; i < n; ++i) {
            fill(static_cast<double>(xs[i]), static_cast<double>(ys[i]),
                 static_cast<double>(weights[i]));
        }
    }

    /**
     * @brief Get the bin content
     * @param binx X bin index
     * @param biny Y bin index
     * @return double Bin content
     */
    [[nodiscard]] auto get_bin_content(int binx, int biny) const -> double {
        int global_bin = get_global_bin(binx, biny);
        if (global_bin < 0 || global_bin >= static_cast<int>(m_bins.size())) {
            return 0.0;
        }
        return m_bins[global_bin].load(std::memory_order_relaxed);
    }

    /**
     * @brief Get the number of entries
     * @return std::size_t Number of fill calls
     */
    [[nodiscard]] auto entries() const -> std::size_t {
        return m_entries.load(std::memory_order_relaxed);
    }

    /**
     * @brief Merge accumulated data into a ROOT TH2D
     *
     * @param target Target ROOT histogram to merge into
     */
    auto merge_into(TH2D* target) const -> void {
        if (!target) return;

        // Verify compatible binning
        if (target->GetNbinsX() != m_nbinsx || target->GetNbinsY() != m_nbinsy) {
            return;
        }

        // Add contents to target
        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                int global_bin = get_global_bin(ix, iy);
                double content = m_bins[global_bin].load(std::memory_order_relaxed);
                double error2 = m_sum_w2[global_bin].load(std::memory_order_relaxed);

                if (content != 0.0) {
                    int root_bin = target->GetBin(ix, iy);
                    target->SetBinContent(root_bin,
                                          target->GetBinContent(root_bin) + content);
                    double old_error = target->GetBinError(root_bin);
                    target->SetBinError(root_bin, std::sqrt(old_error * old_error + error2));
                }
            }
        }

        target->SetEntries(target->GetEntries() + static_cast<double>(entries()));
    }

    /**
     * @brief Create a ROOT TH2D from this histogram
     *
     * @param name Histogram name
     * @param title Histogram title
     * @return std::unique_ptr<TH2D> New ROOT histogram with copied data
     */
    [[nodiscard]] auto to_root(const std::string& name,
                                const std::string& title) const -> std::unique_ptr<TH2D> {
        auto hist = std::make_unique<TH2D>(name.c_str(), title.c_str(),
                                            m_nbinsx, m_xmin, m_xmax,
                                            m_nbinsy, m_ymin, m_ymax);

        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                int global_bin = get_global_bin(ix, iy);
                int root_bin = hist->GetBin(ix, iy);
                hist->SetBinContent(root_bin, m_bins[global_bin].load(std::memory_order_relaxed));
                hist->SetBinError(root_bin, std::sqrt(m_sum_w2[global_bin].load(std::memory_order_relaxed)));
            }
        }
        hist->SetEntries(static_cast<double>(entries()));

        return hist;
    }

    /**
     * @brief Reset the histogram to zero
     */
    auto reset() -> void {
        for (auto& bin : m_bins) {
            bin.store(0.0, std::memory_order_relaxed);
        }
        for (auto& sw2 : m_sum_w2) {
            sw2.store(0.0, std::memory_order_relaxed);
        }
        m_entries.store(0, std::memory_order_relaxed);
    }

   private:
    int m_nbinsx;
    int m_nbinsy;
    double m_xmin, m_xmax;
    double m_ymin, m_ymax;
    double m_bin_width_x;
    double m_bin_width_y;
    int m_total_bins;

    std::vector<std::atomic<double>> m_bins;
    std::vector<std::atomic<double>> m_sum_w2;
    std::atomic<std::size_t> m_entries;

    /**
     * @brief Find the global bin index for x, y values
     * @return int Global bin index, or -1 if either x or y is NaN
     */
    [[nodiscard]] auto find_bin(double x, double y) const -> int {
        int binx = find_bin_x(x);
        int biny = find_bin_y(y);
        if (binx < 0 || biny < 0) return -1;  // NaN in either coordinate
        return get_global_bin(binx, biny);
    }

    [[nodiscard]] auto find_bin_x(double x) const -> int {
        if (std::isnan(x)) return -1;  // Signal NaN
        if (x < m_xmin) return 0;
        if (x >= m_xmax) return m_nbinsx + 1;
        int bin = static_cast<int>((x - m_xmin) / m_bin_width_x) + 1;
        return std::min(bin, m_nbinsx);
    }

    [[nodiscard]] auto find_bin_y(double y) const -> int {
        if (std::isnan(y)) return -1;  // Signal NaN
        if (y < m_ymin) return 0;
        if (y >= m_ymax) return m_nbinsy + 1;
        int bin = static_cast<int>((y - m_ymin) / m_bin_width_y) + 1;
        return std::min(bin, m_nbinsy);
    }

    [[nodiscard]] auto get_global_bin(int binx, int biny) const -> int {
        return binx * (m_nbinsy + 2) + biny;
    }

    static auto atomic_add(std::atomic<double>& target, double value) -> void {
        double current = target.load(std::memory_order_relaxed);
        while (!target.compare_exchange_weak(current, current + value,
                                              std::memory_order_relaxed,
                                              std::memory_order_relaxed)) {
        }
    }
};

}  // namespace clas12::framework
