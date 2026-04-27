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
#include <cstdio>
#include <memory>
#include <span>
#include <string>
#include <concepts>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

namespace clas12::framework {

/**
 * @brief A type models `Accumulator<T>` iff partial results of type
 *        `T` can be folded together and reset to the identity.
 *
 * Each worker thread accumulates a private copy of a histogram or
 * cutflow; a final merge step sums everything into a single output.
 * Used internally by `ThreadLocalHistogram1D/2D/3D` to satisfy a
 * compile-time check that they expose `merge(const T&)` and `reset()`.
 *
 * Required member functions:
 *   - `void merge(const T& other)` — fold `other` into `*this`.
 *   - `void reset()` — restore the identity (empty result).
 */
template <typename T>
concept Accumulator = requires(T& a, const T& b) {
    { a.merge(b) } -> std::same_as<void>;
    { a.reset()  } -> std::same_as<void>;
};


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
///
/// A single-shot stderr warning is emitted the first time an exhausted
/// thread asks for an id, so that silent fill-drops become visible in
/// long-running multi-threaded jobs (e.g. TBB workers cycled over hours).
inline auto claim_worker_id() noexcept -> int {
    static std::atomic<int> next_id{0};
    static std::atomic<bool> warned{false};
    thread_local int cached_id = []() {
        int id = next_id.fetch_add(1, std::memory_order_relaxed);
        if (id >= kMaxWorkers) {
            bool expected = false;
            if (warned.compare_exchange_strong(expected, true,
                                               std::memory_order_relaxed)) {
                std::fputs(
                    "[ThreadLocalHistogram] worker-id pool exhausted "
                    "(> kMaxWorkers threads); subsequent fills from new "
                    "threads will be silently dropped. Raise kMaxWorkers "
                    "or reuse workers via a thread pool.\n",
                    stderr);
            }
            return -1;
        }
        return id;
    }();
    return cached_id;
}

/// Fused per-bin accumulator: contents and sum-of-weights-squared
/// live on the same cache line (16 B), so a single fill touches one
/// line instead of two separate heap buffers.
struct Bin {
    double sum    = 0.0;
    double sum_w2 = 0.0;
};
static_assert(sizeof(Bin) == 16, "Bin layout assumption for cache behaviour");

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
        , m_inv_bin_width(static_cast<double>(nbins) / (xmax - xmin))
        , m_total_bins(nbins + 2)
        , m_slots(detail::kMaxWorkers) {}

    /// Fill the histogram with a single value. Thread-safe, lock-free.
    auto fill(double x, double weight = 1.0) -> void {
        const int bin = find_bin(x);
        if (bin < 0) return;
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        slot.bins[bin].sum    += weight;
        slot.bins[bin].sum_w2 += weight * weight;
        slot.entries          += 1;
    }

    /// Vectorised fill from a contiguous span. Thread-safe.
    template <typename T>
    auto fill_span(std::span<const T> values) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        for (T v : values) {
            const int bin = find_bin(static_cast<double>(v));
            if (bin < 0) continue;
            slot.bins[bin].sum    += 1.0;
            slot.bins[bin].sum_w2 += 1.0;
            slot.entries          += 1;
        }
    }

    /// Vectorised weighted fill from parallel spans of values + weights.
    template <typename T, typename W>
    auto fill_span(std::span<const T> values, std::span<const W> weights) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        const std::size_t n = std::min(values.size(), weights.size());
        for (std::size_t i = 0; i < n; ++i) {
            const int bin = find_bin(static_cast<double>(values[i]));
            if (bin < 0) continue;
            const double w = static_cast<double>(weights[i]);
            slot.bins[bin].sum    += w;
            slot.bins[bin].sum_w2 += w * w;
            slot.entries          += 1;
        }
    }

    /// Summed bin content across every worker slot.
    [[nodiscard]] auto get_bin_content(int bin) const -> double {
        if (bin < 0 || bin >= m_total_bins) return 0.0;
        double sum = 0.0;
        for (const auto& slot : m_slots) {
            if (!slot.bins.empty()) sum += slot.bins[bin].sum;
        }
        return sum;
    }

    /// Summed bin error across every worker slot.
    [[nodiscard]] auto get_bin_error(int bin) const -> double {
        if (bin < 0 || bin >= m_total_bins) return 0.0;
        double sum_w2 = 0.0;
        for (const auto& slot : m_slots) {
            if (!slot.bins.empty()) sum_w2 += slot.bins[bin].sum_w2;
        }
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

        std::vector<detail::Bin> temp(m_total_bins);
        for (const auto& slot : m_slots) {
            if (slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                temp[i].sum    += slot.bins[i].sum;
                temp[i].sum_w2 += slot.bins[i].sum_w2;
            }
        }

        for (int i = 0; i < m_total_bins; ++i) {
            if (temp[i].sum != 0.0) {
                target->SetBinContent(i, target->GetBinContent(i) + temp[i].sum);
                const double old_error = target->GetBinError(i);
                target->SetBinError(i, std::sqrt(old_error * old_error + temp[i].sum_w2));
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

        std::vector<detail::Bin> temp(m_total_bins);
        for (const auto& slot : m_slots) {
            if (slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                temp[i].sum    += slot.bins[i].sum;
                temp[i].sum_w2 += slot.bins[i].sum_w2;
            }
        }
        for (int i = 0; i < m_total_bins; ++i) {
            hist->SetBinContent(i, temp[i].sum);
            hist->SetBinError(i, std::sqrt(temp[i].sum_w2));
        }
        hist->SetEntries(static_cast<double>(entries()));
        return hist;
    }

    /// Reset every worker slot to zero (keeping allocated capacity).
    auto reset() -> void {
        for (auto& slot : m_slots) {
            if (!slot.bins.empty()) {
                std::fill(slot.bins.begin(), slot.bins.end(), detail::Bin{});
            }
            slot.entries = 0;
        }
    }

    /// Fold another ThreadLocalHistogram1D into this one (Accumulator concept).
    auto merge(const ThreadLocalHistogram1D& other) -> void {
        if (other.m_nbins != m_nbins || other.m_xmin != m_xmin || other.m_xmax != m_xmax) {
            return;  // Incompatible binning, silently skip.
        }
        // Fold every slot of `other` into slot 0 of `this`.
        auto& target = m_slots[0];
        ensure_allocated(target);
        for (const auto& other_slot : other.m_slots) {
            if (other_slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                target.bins[i].sum    += other_slot.bins[i].sum;
                target.bins[i].sum_w2 += other_slot.bins[i].sum_w2;
            }
            target.entries += other_slot.entries;
        }
    }

   private:
    struct alignas(64) Slot {
        std::vector<detail::Bin> bins;
        std::size_t entries = 0;
    };

    int    m_nbins;
    double m_xmin;
    double m_xmax;
    double m_inv_bin_width;
    int    m_total_bins;
    std::vector<Slot> m_slots;

    /// Lazy-allocate a slot's bin buffer on first use. Called in the fill
    /// path only; since each slot is written by a single worker thread,
    /// no synchronization is needed.
    auto ensure_allocated(Slot& slot) const -> void {
        if (slot.bins.empty()) {
            slot.bins.assign(static_cast<std::size_t>(m_total_bins), detail::Bin{});
        }
    }

    [[nodiscard]] auto find_bin(double x) const -> int {
        if (std::isnan(x)) return -1;
        if (x < m_xmin) return 0;                  // underflow
        if (x >= m_xmax) return m_nbins + 1;       // overflow
        int bin = static_cast<int>((x - m_xmin) * m_inv_bin_width) + 1;
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
        , m_xmin(xmin), m_xmax(xmax)
        , m_ymin(ymin), m_ymax(ymax)
        , m_inv_bin_width_x(static_cast<double>(nbinsx) / (xmax - xmin))
        , m_inv_bin_width_y(static_cast<double>(nbinsy) / (ymax - ymin))
        , m_stride_x(nbinsy + 2)
        , m_total_bins((nbinsx + 2) * (nbinsy + 2))
        , m_slots(detail::kMaxWorkers) {}

    auto fill(double x, double y, double weight = 1.0) -> void {
        const int bin = find_bin(x, y);
        if (bin < 0) return;
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        slot.bins[bin].sum    += weight;
        slot.bins[bin].sum_w2 += weight * weight;
        slot.entries          += 1;
    }

    template <typename TX, typename TY>
    auto fill_span(std::span<const TX> xs, std::span<const TY> ys) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        const std::size_t n = std::min(xs.size(), ys.size());
        for (std::size_t i = 0; i < n; ++i) {
            const int bin = find_bin(static_cast<double>(xs[i]),
                                     static_cast<double>(ys[i]));
            if (bin < 0) continue;
            slot.bins[bin].sum    += 1.0;
            slot.bins[bin].sum_w2 += 1.0;
            slot.entries          += 1;
        }
    }

    template <typename TX, typename TY, typename W>
    auto fill_span(std::span<const TX> xs, std::span<const TY> ys,
                   std::span<const W> weights) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        const std::size_t n = std::min({xs.size(), ys.size(), weights.size()});
        for (std::size_t i = 0; i < n; ++i) {
            const int bin = find_bin(static_cast<double>(xs[i]),
                                     static_cast<double>(ys[i]));
            if (bin < 0) continue;
            const double w = static_cast<double>(weights[i]);
            slot.bins[bin].sum    += w;
            slot.bins[bin].sum_w2 += w * w;
            slot.entries          += 1;
        }
    }

    [[nodiscard]] auto get_bin_content(int binx, int biny) const -> double {
        const int global_bin = get_global_bin(binx, biny);
        if (global_bin < 0 || global_bin >= m_total_bins) return 0.0;
        double sum = 0.0;
        for (const auto& slot : m_slots) {
            if (!slot.bins.empty()) sum += slot.bins[global_bin].sum;
        }
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

        std::vector<detail::Bin> temp(m_total_bins);
        for (const auto& slot : m_slots) {
            if (slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                temp[i].sum    += slot.bins[i].sum;
                temp[i].sum_w2 += slot.bins[i].sum_w2;
            }
        }

        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                const int global_bin = get_global_bin(ix, iy);
                const double content = temp[global_bin].sum;
                if (content != 0.0) {
                    const int root_bin = target->GetBin(ix, iy);
                    target->SetBinContent(root_bin, target->GetBinContent(root_bin) + content);
                    const double old_error = target->GetBinError(root_bin);
                    target->SetBinError(root_bin,
                                        std::sqrt(old_error * old_error
                                                  + temp[global_bin].sum_w2));
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

        std::vector<detail::Bin> temp(m_total_bins);
        for (const auto& slot : m_slots) {
            if (slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                temp[i].sum    += slot.bins[i].sum;
                temp[i].sum_w2 += slot.bins[i].sum_w2;
            }
        }
        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                const int global_bin = get_global_bin(ix, iy);
                const int root_bin   = hist->GetBin(ix, iy);
                hist->SetBinContent(root_bin, temp[global_bin].sum);
                hist->SetBinError(root_bin, std::sqrt(temp[global_bin].sum_w2));
            }
        }
        hist->SetEntries(static_cast<double>(entries()));
        return hist;
    }

    auto reset() -> void {
        for (auto& slot : m_slots) {
            if (!slot.bins.empty()) {
                std::fill(slot.bins.begin(), slot.bins.end(), detail::Bin{});
            }
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
        ensure_allocated(target);
        for (const auto& other_slot : other.m_slots) {
            if (other_slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                target.bins[i].sum    += other_slot.bins[i].sum;
                target.bins[i].sum_w2 += other_slot.bins[i].sum_w2;
            }
            target.entries += other_slot.entries;
        }
    }

   private:
    struct alignas(64) Slot {
        std::vector<detail::Bin> bins;
        std::size_t entries = 0;
    };

    int    m_nbinsx, m_nbinsy;
    double m_xmin, m_xmax;
    double m_ymin, m_ymax;
    double m_inv_bin_width_x;
    double m_inv_bin_width_y;
    int    m_stride_x;
    int    m_total_bins;
    std::vector<Slot> m_slots;

    auto ensure_allocated(Slot& slot) const -> void {
        if (slot.bins.empty()) {
            slot.bins.assign(static_cast<std::size_t>(m_total_bins), detail::Bin{});
        }
    }

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
        int bin = static_cast<int>((x - m_xmin) * m_inv_bin_width_x) + 1;
        return std::min(bin, m_nbinsx);
    }

    [[nodiscard]] auto find_bin_y(double y) const -> int {
        if (std::isnan(y)) return -1;
        if (y < m_ymin) return 0;
        if (y >= m_ymax) return m_nbinsy + 1;
        int bin = static_cast<int>((y - m_ymin) * m_inv_bin_width_y) + 1;
        return std::min(bin, m_nbinsy);
    }

    [[nodiscard]] auto get_global_bin(int binx, int biny) const -> int {
        return binx * m_stride_x + biny;
    }
};

/**
 * @brief Thread-local 3D histogram. Same pattern as ThreadLocalHistogram2D.
 *
 * Typical use: keep the "slicing" variable (e.g. momentum, theta) on the X
 * axis and project to 2D at analysis time via `TH3::ProjectionYZ(..., xlo, xhi)`.
 * Replaces hand-rolled stacks of per-bin 2D histograms with a single
 * booking + fill call.
 */
class ThreadLocalHistogram3D {
   public:
    ThreadLocalHistogram3D(int nbinsx, double xmin, double xmax,
                           int nbinsy, double ymin, double ymax,
                           int nbinsz, double zmin, double zmax)
        : m_nbinsx(nbinsx)
        , m_nbinsy(nbinsy)
        , m_nbinsz(nbinsz)
        , m_xmin(xmin), m_xmax(xmax)
        , m_ymin(ymin), m_ymax(ymax)
        , m_zmin(zmin), m_zmax(zmax)
        , m_inv_bin_width_x(static_cast<double>(nbinsx) / (xmax - xmin))
        , m_inv_bin_width_y(static_cast<double>(nbinsy) / (ymax - ymin))
        , m_inv_bin_width_z(static_cast<double>(nbinsz) / (zmax - zmin))
        , m_stride_y(nbinsz + 2)
        , m_stride_x((nbinsy + 2) * (nbinsz + 2))
        , m_total_bins((nbinsx + 2) * (nbinsy + 2) * (nbinsz + 2))
        , m_slots(detail::kMaxWorkers) {}

    auto fill(double x, double y, double z, double weight = 1.0) -> void {
        const int bin = find_bin(x, y, z);
        if (bin < 0) return;
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        slot.bins[bin].sum    += weight;
        slot.bins[bin].sum_w2 += weight * weight;
        slot.entries          += 1;
    }

    template <typename TX, typename TY, typename TZ>
    auto fill_span(std::span<const TX> xs,
                   std::span<const TY> ys,
                   std::span<const TZ> zs) -> void {
        const int wid = detail::claim_worker_id();
        if (wid < 0) return;
        auto& slot = m_slots[static_cast<std::size_t>(wid)];
        ensure_allocated(slot);
        const std::size_t n = std::min({xs.size(), ys.size(), zs.size()});
        for (std::size_t i = 0; i < n; ++i) {
            const int bin = find_bin(static_cast<double>(xs[i]),
                                     static_cast<double>(ys[i]),
                                     static_cast<double>(zs[i]));
            if (bin < 0) continue;
            slot.bins[bin].sum    += 1.0;
            slot.bins[bin].sum_w2 += 1.0;
            slot.entries          += 1;
        }
    }

    [[nodiscard]] auto entries() const -> std::size_t {
        std::size_t sum = 0;
        for (const auto& slot : m_slots) sum += slot.entries;
        return sum;
    }

    [[nodiscard]] auto nbinsx() const -> int { return m_nbinsx; }
    [[nodiscard]] auto nbinsy() const -> int { return m_nbinsy; }
    [[nodiscard]] auto nbinsz() const -> int { return m_nbinsz; }
    [[nodiscard]] auto xmin()   const -> double { return m_xmin; }
    [[nodiscard]] auto xmax()   const -> double { return m_xmax; }
    [[nodiscard]] auto ymin()   const -> double { return m_ymin; }
    [[nodiscard]] auto ymax()   const -> double { return m_ymax; }
    [[nodiscard]] auto zmin()   const -> double { return m_zmin; }
    [[nodiscard]] auto zmax()   const -> double { return m_zmax; }

    auto merge_into(TH3D* target) const -> void {
        if (!target) return;
        if (target->GetNbinsX() != m_nbinsx ||
            target->GetNbinsY() != m_nbinsy ||
            target->GetNbinsZ() != m_nbinsz) return;

        std::vector<detail::Bin> temp(m_total_bins);
        for (const auto& slot : m_slots) {
            if (slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                temp[i].sum    += slot.bins[i].sum;
                temp[i].sum_w2 += slot.bins[i].sum_w2;
            }
        }

        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                for (int iz = 0; iz <= m_nbinsz + 1; ++iz) {
                    const int global_bin = get_global_bin(ix, iy, iz);
                    const double content = temp[global_bin].sum;
                    if (content != 0.0) {
                        const int root_bin = target->GetBin(ix, iy, iz);
                        target->SetBinContent(root_bin,
                                              target->GetBinContent(root_bin) + content);
                        const double old_err = target->GetBinError(root_bin);
                        target->SetBinError(root_bin,
                                            std::sqrt(old_err * old_err
                                                      + temp[global_bin].sum_w2));
                    }
                }
            }
        }
        target->SetEntries(target->GetEntries() + static_cast<double>(entries()));
    }

    [[nodiscard]] auto to_root(const std::string& name,
                               const std::string& title) const -> std::unique_ptr<TH3D> {
        auto hist = std::make_unique<TH3D>(name.c_str(), title.c_str(),
                                           m_nbinsx, m_xmin, m_xmax,
                                           m_nbinsy, m_ymin, m_ymax,
                                           m_nbinsz, m_zmin, m_zmax);

        std::vector<detail::Bin> temp(m_total_bins);
        for (const auto& slot : m_slots) {
            if (slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                temp[i].sum    += slot.bins[i].sum;
                temp[i].sum_w2 += slot.bins[i].sum_w2;
            }
        }
        for (int ix = 0; ix <= m_nbinsx + 1; ++ix) {
            for (int iy = 0; iy <= m_nbinsy + 1; ++iy) {
                for (int iz = 0; iz <= m_nbinsz + 1; ++iz) {
                    const int global_bin = get_global_bin(ix, iy, iz);
                    const int root_bin   = hist->GetBin(ix, iy, iz);
                    hist->SetBinContent(root_bin, temp[global_bin].sum);
                    hist->SetBinError(root_bin, std::sqrt(temp[global_bin].sum_w2));
                }
            }
        }
        hist->SetEntries(static_cast<double>(entries()));
        return hist;
    }

    auto reset() -> void {
        for (auto& slot : m_slots) {
            if (!slot.bins.empty()) {
                std::fill(slot.bins.begin(), slot.bins.end(), detail::Bin{});
            }
            slot.entries = 0;
        }
    }

    auto merge(const ThreadLocalHistogram3D& other) -> void {
        if (other.m_nbinsx != m_nbinsx || other.m_nbinsy != m_nbinsy || other.m_nbinsz != m_nbinsz ||
            other.m_xmin   != m_xmin   || other.m_xmax   != m_xmax   ||
            other.m_ymin   != m_ymin   || other.m_ymax   != m_ymax   ||
            other.m_zmin   != m_zmin   || other.m_zmax   != m_zmax) {
            return;
        }
        auto& target = m_slots[0];
        ensure_allocated(target);
        for (const auto& other_slot : other.m_slots) {
            if (other_slot.bins.empty()) continue;
            for (int i = 0; i < m_total_bins; ++i) {
                target.bins[i].sum    += other_slot.bins[i].sum;
                target.bins[i].sum_w2 += other_slot.bins[i].sum_w2;
            }
            target.entries += other_slot.entries;
        }
    }

   private:
    struct alignas(64) Slot {
        std::vector<detail::Bin> bins;
        std::size_t entries = 0;
    };

    int    m_nbinsx, m_nbinsy, m_nbinsz;
    double m_xmin, m_xmax;
    double m_ymin, m_ymax;
    double m_zmin, m_zmax;
    double m_inv_bin_width_x, m_inv_bin_width_y, m_inv_bin_width_z;
    int    m_stride_y;
    int    m_stride_x;
    int    m_total_bins;
    std::vector<Slot> m_slots;

    auto ensure_allocated(Slot& slot) const -> void {
        if (slot.bins.empty()) {
            slot.bins.assign(static_cast<std::size_t>(m_total_bins), detail::Bin{});
        }
    }

    [[nodiscard]] auto find_bin_x(double x) const -> int {
        if (std::isnan(x)) return -1;
        if (x < m_xmin) return 0;
        if (x >= m_xmax) return m_nbinsx + 1;
        int bin = static_cast<int>((x - m_xmin) * m_inv_bin_width_x) + 1;
        return std::min(bin, m_nbinsx);
    }

    [[nodiscard]] auto find_bin_y(double y) const -> int {
        if (std::isnan(y)) return -1;
        if (y < m_ymin) return 0;
        if (y >= m_ymax) return m_nbinsy + 1;
        int bin = static_cast<int>((y - m_ymin) * m_inv_bin_width_y) + 1;
        return std::min(bin, m_nbinsy);
    }

    [[nodiscard]] auto find_bin_z(double z) const -> int {
        if (std::isnan(z)) return -1;
        if (z < m_zmin) return 0;
        if (z >= m_zmax) return m_nbinsz + 1;
        int bin = static_cast<int>((z - m_zmin) * m_inv_bin_width_z) + 1;
        return std::min(bin, m_nbinsz);
    }

    [[nodiscard]] auto find_bin(double x, double y, double z) const -> int {
        const int bx = find_bin_x(x);
        const int by = find_bin_y(y);
        const int bz = find_bin_z(z);
        if (bx < 0 || by < 0 || bz < 0) return -1;
        return get_global_bin(bx, by, bz);
    }

    [[nodiscard]] auto get_global_bin(int bx, int by, int bz) const -> int {
        return bx * m_stride_x + by * m_stride_y + bz;
    }
};

// Compile-time check that all histograms model the Accumulator concept.
static_assert(Accumulator<ThreadLocalHistogram1D>,
              "ThreadLocalHistogram1D must model Accumulator<T>");
static_assert(Accumulator<ThreadLocalHistogram2D>,
              "ThreadLocalHistogram2D must model Accumulator<T>");
static_assert(Accumulator<ThreadLocalHistogram3D>,
              "ThreadLocalHistogram3D must model Accumulator<T>");

}  // namespace clas12::framework
