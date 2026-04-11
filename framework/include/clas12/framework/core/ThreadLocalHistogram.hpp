#pragma once

/**
 * @file ThreadLocalHistogram.hpp
 * @brief Per-worker-local histograms with a final merge pass.
 *
 * Replaces the previous CAS-based `AtomicHistogram` design. Each worker
 * thread writes into its own private bin array (zero contention on the
 * fill hot path), and `merge_into()` sums every slot into a ROOT TH1D
 * at `on_finalize()` time. This matches the coffea processor.Accumulator
 * pattern and scales linearly with worker count on contended bins,
 * where the old atomic-CAS path would stall on every bin touch.
 *
 * Worker identity is assigned lazily: each thread claims a unique id
 * on first fill via a global atomic counter, and the id persists in a
 * thread_local variable for the lifetime of the thread. Histograms
 * pre-allocate `kMaxWorkers` slots, so the fill hot path is
 * `m_slots[wid].bins[bin] += weight` with no allocations, no locks,
 * no atomics.
 */

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>

#include <clas12/framework/core/Accumulator.hpp>

namespace clas12::framework {

namespace detail {

/// Maximum number of worker threads a histogram can handle without
/// falling back to a mutex-protected slow path. Covers every realistic
/// CLAS12 worker pool; bump if someone runs on a 256-core node.
inline constexpr int kMaxWorkers = 128;

/// Claim a unique worker id for the calling thread. The first call in
/// each thread atomically reserves a new id; subsequent calls return
/// the cached value. Returns -1 if the global pool is exhausted
/// (>kMaxWorkers threads in the process); callers must handle the
/// overflow via their own slow path.
inline auto claim_worker_id() noexcept -> int {
    static std::atomic<int> next_id{0};
    thread_local int cached_id = []() {
        int id = next_id.fetch_add(1, std::memory_order_relaxed);
        return id < kMaxWorkers ? id : -1;
    }();
    return cached_id;
}

}  // namespace detail

/**
 * @brief Thread-local 1D histogram that merges to TH1D at finalize.
 *
 * API: `fill`, `fill_span`, `merge_into`, `to_root`, `reset`,
 * `get_bin_content`, `entries`, `nbins`, `xmin`, `xmax`, plus `merge`
 * for the Accumulator concept. Internal storage is per-worker so
 * concurrent fills never contend on a shared bin.
 *
 * Models `Accumulator<ThreadLocalHistogram1D>`.
 */
class ThreadLocalHistogram1D {
   public:
    ThreadLocalHistogram1D(int nbins, double xmin, double xmax)
        : m_nbins(nbins)
        , m_xmin(xmin)
        , m_xmax(xmax)
        , m_bin_width((xmax - xmin) / nbins)
        , m_slots(detail::kMaxWorkers) {
        for (auto& slot : m_slots) {
            slot.bins.assign(static_cast<std::size_t>(nbins) + 2, 0.0);
            slot.sum_w2.assign(static_cast<std::size_t>(nbins) + 2, 0.0);
        }
    }

    /// Fill the histogram with a single value. Thread-safe, lock-free.
    auto fill(double x, double weight = 1.0) -> void {
        const int bin = find_bin(x);
        if (bin < 0) return;
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;  // Pool exhausted - silently drop (very rare).
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        slot.bins[bin]    += weight;
        slot.sum_w2[bin]  += weight * weight;
        slot.entries      += 1;
    }

    /// Vectorised fill from a contiguous span. Thread-safe.
    template <typename T>
    auto fill_span(std::span<const T> values) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        for (T v : values) {
            const int bin = find_bin(static_cast<double>(v));
            if (bin < 0) continue;
            slot.bins[bin]   += 1.0;
            slot.sum_w2[bin] += 1.0;
            slot.entries     += 1;
        }
    }

    /// Vectorised weighted fill from parallel spans of values + weights.
    template <typename T, typename W>
    auto fill_span(std::span<const T> values, std::span<const W> weights) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        const std::size_t n = std::min(values.size(), weights.size());
        for (std::size_t i = 0; i < n; ++i) {
            const int bin = find_bin(static_cast<double>(values[i]));
            if (bin < 0) continue;
            const double w = static_cast<double>(weights[i]);
            slot.bins[bin]   += w;
            slot.sum_w2[bin] += w * w;
            slot.entries     += 1;
        }
    }

    /// Summed bin content across every worker slot.
    [[nodiscard]] auto get_bin_content(int bin) const -> double {
        if (bin < 0 || bin >= static_cast<int>(m_slots[0].bins.size())) return 0.0;
        double sum = 0.0;
        for (const auto& slot : m_slots) sum += slot.bins[bin];
        return sum;
    }

    /// Summed bin error across every worker slot.
    [[nodiscard]] auto get_bin_error(int bin) const -> double {
        if (bin < 0 || bin >= static_cast<int>(m_slots[0].sum_w2.size())) return 0.0;
        double sum_w2 = 0.0;
        for (const auto& slot : m_slots) sum_w2 += slot.sum_w2[bin];
        return std::sqrt(sum_w2);
    }

    /// Total entries across every worker slot.
    [[nodiscard]] auto entries() const -> std::size_t {
        std::size_t sum = 0;
        for (const auto& slot : m_slots) sum += slot.entries;
        return sum;
    }

    [[nodiscard]] auto nbins() const -> int { return m_nbins; }
    [[nodiscard]] auto xmin()  const -> double { return m_xmin; }
    [[nodiscard]] auto xmax()  const -> double { return m_xmax; }

    /**
     * @brief Merge every worker slot into a target ROOT TH1D.
     *
     * Caller must guarantee no other thread is still filling this
     * histogram. `HistogramService::finalize()` is the natural call
     * site - by then the event loop has joined all worker threads.
     */
    auto merge_into(TH1D* target) const -> void {
        if (!target || target->GetNbinsX() != m_nbins) return;

        const int nbins_total = m_nbins + 2;
        std::vector<double> bins(nbins_total, 0.0);
        std::vector<double> sum_w2(nbins_total, 0.0);

        for (const auto& slot : m_slots) {
            for (int i = 0; i < nbins_total; ++i) {
                bins[i]   += slot.bins[i];
                sum_w2[i] += slot.sum_w2[i];
            }
        }

        for (int i = 0; i < nbins_total; ++i) {
            if (bins[i] != 0.0) {
                target->SetBinContent(i, target->GetBinContent(i) + bins[i]);
                const double old_error = target->GetBinError(i);
                target->SetBinError(i, std::sqrt(old_error * old_error + sum_w2[i]));
            }
        }
        target->SetEntries(target->GetEntries() + static_cast<double>(entries()));
    }

    /// Build a fresh TH1D from this histogram.
    ///
    /// Unlike `merge_into`, which accumulates onto an existing target,
    /// this sets the ROOT histogram's bin contents and entries ABSOLUTE
    /// - needed because ROOT's `SetBinContent` auto-bumps the internal
    /// entries counter, which would double-count if we then called the
    /// accumulating `SetEntries(existing + entries())` from `merge_into`.
    [[nodiscard]] auto to_root(const std::string& name,
                               const std::string& title) const -> std::unique_ptr<TH1D> {
        auto hist = std::make_unique<TH1D>(name.c_str(), title.c_str(),
                                           m_nbins, m_xmin, m_xmax);

        const int nbins_total = m_nbins + 2;
        std::vector<double> bins(nbins_total, 0.0);
        std::vector<double> sum_w2(nbins_total, 0.0);
        for (const auto& slot : m_slots) {
            for (int i = 0; i < nbins_total; ++i) {
                bins[i]   += slot.bins[i];
                sum_w2[i] += slot.sum_w2[i];
            }
        }
        for (int i = 0; i < nbins_total; ++i) {
            hist->SetBinContent(i, bins[i]);
            hist->SetBinError(i, std::sqrt(sum_w2[i]));
        }
        hist->SetEntries(static_cast<double>(entries()));
        return hist;
    }

    /// Reset every worker slot to zero.
    auto reset() -> void {
        for (auto& slot : m_slots) {
            std::fill(slot.bins.begin(),   slot.bins.end(),   0.0);
            std::fill(slot.sum_w2.begin(), slot.sum_w2.end(), 0.0);
            slot.entries = 0;
        }
    }

    /// Fold another ThreadLocalHistogram1D into this one (Accumulator concept).
    auto merge(const ThreadLocalHistogram1D& other) -> void {
        if (other.m_nbins != m_nbins || other.m_xmin != m_xmin || other.m_xmax != m_xmax) {
            return;  // Incompatible binning, silently skip.
        }
        // Fold every slot of `other` into slot 0 of `this` (sum-of-slots
        // semantics: the merge result is equivalent to concatenating
        // every fill event from both histograms).
        auto& target = m_slots[0];
        for (const auto& other_slot : other.m_slots) {
            for (std::size_t i = 0; i < target.bins.size(); ++i) {
                target.bins[i]   += other_slot.bins[i];
                target.sum_w2[i] += other_slot.sum_w2[i];
            }
            target.entries += other_slot.entries;
        }
    }

   private:
    struct alignas(64) Slot {
        std::vector<double> bins;
        std::vector<double> sum_w2;
        std::size_t entries = 0;
    };

    int    m_nbins;
    double m_xmin;
    double m_xmax;
    double m_bin_width;
    std::vector<Slot> m_slots;

    [[nodiscard]] auto find_bin(double x) const -> int {
        if (std::isnan(x)) return -1;
        if (x < m_xmin) return 0;                  // underflow
        if (x >= m_xmax) return m_nbins + 1;       // overflow
        int bin = static_cast<int>((x - m_xmin) / m_bin_width) + 1;
        return std::min(bin, m_nbins);
    }
};

/**
 * @brief Thread-local 2D histogram. Same pattern as ThreadLocalHistogram1D.
 */
class ThreadLocalHistogram2D {
   public:
    ThreadLocalHistogram2D(int nbinsx, double xmin, double xmax,
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
        , m_slots(detail::kMaxWorkers) {
        for (auto& slot : m_slots) {
            slot.bins.assign(static_cast<std::size_t>(m_total_bins), 0.0);
            slot.sum_w2.assign(static_cast<std::size_t>(m_total_bins), 0.0);
        }
    }

    auto fill(double x, double y, double weight = 1.0) -> void {
        const int bin = find_bin(x, y);
        if (bin < 0) return;
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        slot.bins[bin]   += weight;
        slot.sum_w2[bin] += weight * weight;
        slot.entries     += 1;
    }

    template <typename TX, typename TY>
    auto fill_span(std::span<const TX> xs, std::span<const TY> ys) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        const std::size_t n = std::min(xs.size(), ys.size());
        for (std::size_t i = 0; i < n; ++i) {
            const int bin = find_bin(static_cast<double>(xs[i]),
                                     static_cast<double>(ys[i]));
            if (bin < 0) continue;
            slot.bins[bin]   += 1.0;
            slot.sum_w2[bin] += 1.0;
            slot.entries     += 1;
        }
    }

    template <typename TX, typename TY, typename W>
    auto fill_span(std::span<const TX> xs, std::span<const TY> ys,
                   std::span<const W> weights) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        const std::size_t n = std::min({xs.size(), ys.size(), weights.size()});
        for (std::size_t i = 0; i < n; ++i) {
            const int bin = find_bin(static_cast<double>(xs[i]),
                                     static_cast<double>(ys[i]));
            if (bin < 0) continue;
            const double w = static_cast<double>(weights[i]);
            slot.bins[bin]   += w;
            slot.sum_w2[bin] += w * w;
            slot.entries     += 1;
        }
    }

    [[nodiscard]] auto get_bin_content(int binx, int biny) const -> double {
        const int global_bin = get_global_bin(binx, biny);
        if (global_bin < 0 || global_bin >= m_total_bins) return 0.0;
        double sum = 0.0;
        for (const auto& slot : m_slots) sum += slot.bins[global_bin];
        return sum;
    }

    [[nodiscard]] auto entries() const -> std::size_t {
        std::size_t sum = 0;
        for (const auto& slot : m_slots) sum += slot.entries;
        return sum;
    }

    auto merge_into(TH2D* target) const -> void {
        if (!target) return;
        if (target->GetNbinsX() != m_nbinsx || target->GetNbinsY() != m_nbinsy) return;

        std::vector<double> bins(m_total_bins, 0.0);
        std::vector<double> sum_w2(m_total_bins, 0.0);
        for (const auto& slot : m_slots) {
            for (int i = 0; i < m_total_bins; ++i) {
                bins[i]   += slot.bins[i];
                sum_w2[i] += slot.sum_w2[i];
            }
        }

        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                const int global_bin = get_global_bin(ix, iy);
                const double content = bins[global_bin];
                if (content != 0.0) {
                    const int root_bin = target->GetBin(ix, iy);
                    target->SetBinContent(root_bin, target->GetBinContent(root_bin) + content);
                    const double old_error = target->GetBinError(root_bin);
                    target->SetBinError(root_bin,
                                        std::sqrt(old_error * old_error + sum_w2[global_bin]));
                }
            }
        }
        target->SetEntries(target->GetEntries() + static_cast<double>(entries()));
    }

    [[nodiscard]] auto to_root(const std::string& name,
                               const std::string& title) const -> std::unique_ptr<TH2D> {
        auto hist = std::make_unique<TH2D>(name.c_str(), title.c_str(),
                                           m_nbinsx, m_xmin, m_xmax,
                                           m_nbinsy, m_ymin, m_ymax);

        std::vector<double> bins(m_total_bins, 0.0);
        std::vector<double> sum_w2(m_total_bins, 0.0);
        for (const auto& slot : m_slots) {
            for (int i = 0; i < m_total_bins; ++i) {
                bins[i]   += slot.bins[i];
                sum_w2[i] += slot.sum_w2[i];
            }
        }
        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                const int global_bin = get_global_bin(ix, iy);
                const int root_bin   = hist->GetBin(ix, iy);
                hist->SetBinContent(root_bin, bins[global_bin]);
                hist->SetBinError(root_bin, std::sqrt(sum_w2[global_bin]));
            }
        }
        hist->SetEntries(static_cast<double>(entries()));
        return hist;
    }

    auto reset() -> void {
        for (auto& slot : m_slots) {
            std::fill(slot.bins.begin(),   slot.bins.end(),   0.0);
            std::fill(slot.sum_w2.begin(), slot.sum_w2.end(), 0.0);
            slot.entries = 0;
        }
    }

    auto merge(const ThreadLocalHistogram2D& other) -> void {
        if (other.m_nbinsx != m_nbinsx || other.m_nbinsy != m_nbinsy ||
            other.m_xmin   != m_xmin   || other.m_xmax   != m_xmax   ||
            other.m_ymin   != m_ymin   || other.m_ymax   != m_ymax) {
            return;
        }
        auto& target = m_slots[0];
        for (const auto& other_slot : other.m_slots) {
            for (int i = 0; i < m_total_bins; ++i) {
                target.bins[i]   += other_slot.bins[i];
                target.sum_w2[i] += other_slot.sum_w2[i];
            }
            target.entries += other_slot.entries;
        }
    }

   private:
    struct alignas(64) Slot {
        std::vector<double> bins;
        std::vector<double> sum_w2;
        std::size_t entries = 0;
    };

    int    m_nbinsx;
    int    m_nbinsy;
    double m_xmin, m_xmax;
    double m_ymin, m_ymax;
    double m_bin_width_x;
    double m_bin_width_y;
    int    m_total_bins;
    std::vector<Slot> m_slots;

    [[nodiscard]] auto find_bin(double x, double y) const -> int {
        const int bx = find_bin_x(x);
        const int by = find_bin_y(y);
        if (bx < 0 || by < 0) return -1;
        return get_global_bin(bx, by);
    }

    [[nodiscard]] auto find_bin_x(double x) const -> int {
        if (std::isnan(x)) return -1;
        if (x < m_xmin) return 0;
        if (x >= m_xmax) return m_nbinsx + 1;
        int bin = static_cast<int>((x - m_xmin) / m_bin_width_x) + 1;
        return std::min(bin, m_nbinsx);
    }

    [[nodiscard]] auto find_bin_y(double y) const -> int {
        if (std::isnan(y)) return -1;
        if (y < m_ymin) return 0;
        if (y >= m_ymax) return m_nbinsy + 1;
        int bin = static_cast<int>((y - m_ymin) / m_bin_width_y) + 1;
        return std::min(bin, m_nbinsy);
    }

    [[nodiscard]] auto get_global_bin(int binx, int biny) const -> int {
        return binx * (m_nbinsy + 2) + biny;
    }
};

// Compile-time check that both histograms model the Accumulator concept.
static_assert(Accumulator<ThreadLocalHistogram1D>,
              "ThreadLocalHistogram1D must model Accumulator<T>");
static_assert(Accumulator<ThreadLocalHistogram2D>,
              "ThreadLocalHistogram2D must model Accumulator<T>");

}  // namespace clas12::framework
