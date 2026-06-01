#pragma once

/**
 * @file QuantileBinner.hpp
 * @brief Thread-safe reservoir sampler that derives equal-statistics
 *        (quantile) bin edges from a stream of values.
 *
 * Hand-picked histogram edges leave some bins nearly empty and others
 * overflowing. `QuantileBinner` instead estimates edges so that every bin
 * holds roughly the same number of entries: feed it the variable during a
 * run, then ask for `equal_stat_edges(n_bins)` at the end.
 *
 * Memory is bounded: the sampler keeps a uniform random subsample of the
 * stream (Vitter's Algorithm R) capped at `capacity`, regardless of how
 * many values are added — so it is safe to drive from a multi-billion-event
 * analysis. Quantiles are linearly interpolated from the sorted sample
 * (the same convention as numpy.quantile's default "linear").
 *
 * Thread safety: `add()` may be called concurrently from every worker
 * thread. The hot path is one relaxed atomic increment plus a thread-local
 * RNG draw; the mutex is taken only when a value is actually written into
 * the reservoir — always during the initial fill, then with probability
 * capacity/n, which decays to ~0 as the stream grows.
 */

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

namespace framework {

class QuantileBinner {
   public:
    explicit QuantileBinner(std::size_t capacity = 1'000'000)
        : m_capacity(capacity == 0 ? 1 : capacity) {
        m_samples.reserve(std::min<std::size_t>(m_capacity, std::size_t{1} << 20));
    }

    /// Add one value to the stream. Non-finite values (NaN/inf) are ignored.
    void add(double x) noexcept {
        if (!std::isfinite(x)) return;
        const long n = m_count.fetch_add(1, std::memory_order_relaxed);
        if (n < static_cast<long>(m_capacity)) {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_samples.push_back(x);
            return;
        }
        // Reservoir full: keep x with probability capacity/(n+1), replacing
        // a uniformly chosen existing slot. RNG is thread-local so the draw
        // itself never contends; the lock is taken only on an actual write.
        thread_local std::mt19937_64 rng{
            std::hash<std::thread::id>{}(std::this_thread::get_id()) ^
            0x9e3779b97f4a7c15ULL};
        std::uniform_int_distribution<long> dist(0, n);
        const long j = dist(rng);
        if (j < static_cast<long>(m_capacity)) {
            std::lock_guard<std::mutex> lk(m_mutex);
            if (j < static_cast<long>(m_samples.size())) {
                m_samples[static_cast<std::size_t>(j)] = x;
            }
        }
    }

    /// Total number of finite values seen (not the reservoir size).
    [[nodiscard]] long count() const noexcept {
        return m_count.load(std::memory_order_relaxed);
    }

    /// Current reservoir occupancy (min(count, capacity)).
    [[nodiscard]] std::size_t sample_size() const {
        std::lock_guard<std::mutex> lk(m_mutex);
        return m_samples.size();
    }

    /// Equal-frequency edges: `n_bins` bins → `n_bins + 1` strictly
    /// increasing edges at quantiles 0, 1/n, …, 1. The outer edges are the
    /// sample min/max. Returns an empty vector when there are fewer samples
    /// than requested bins (caller should fall back to its defaults and warn).
    ///
    /// Degenerate quantiles (a value spike landing several edges on the same
    /// number) are nudged up by one ULP so the result is always strictly
    /// monotonic and usable as histogram edges.
    [[nodiscard]] std::vector<double> equal_stat_edges(int n_bins) const {
        std::vector<double> s;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            s = m_samples;
        }
        return equal_stat_edges_of(std::move(s), n_bins);
    }

    /// Per-bin occupancy of this binner's sample under `edges` — one count
    /// per (edges.size()-1) bin. The whole point of quantile edges is that
    /// these come out ~equal; reporting them confirms that and exposes any
    /// imbalance forced by a value spike.
    [[nodiscard]] std::vector<long> bin_occupancy(const std::vector<double>& edges) const {
        std::vector<double> s;
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            s = m_samples;
        }
        return bin_occupancy_of(s, edges);
    }

    // ---- Free-standing helpers (so a joint multi-axis sampler can derive
    // per-axis edges and occupancy from one shared sample) ----------------

    /// Equal-frequency edges from an explicit (unsorted) sample. Same
    /// algorithm as the instance method.
    [[nodiscard]] static std::vector<double> equal_stat_edges_of(std::vector<double> s,
                                                                 int n_bins) {
        if (n_bins < 1) return {};
        if (s.size() < static_cast<std::size_t>(n_bins) + 1) return {};
        std::sort(s.begin(), s.end());

        std::vector<double> edges(static_cast<std::size_t>(n_bins) + 1);
        edges.front() = s.front();
        edges.back() = s.back();
        for (int i = 1; i < n_bins; ++i) {
            const double pos =
                (static_cast<double>(i) / n_bins) * static_cast<double>(s.size() - 1);
            const std::size_t lo = static_cast<std::size_t>(pos);
            const double frac = pos - static_cast<double>(lo);
            edges[static_cast<std::size_t>(i)] =
                (lo + 1 < s.size()) ? s[lo] * (1.0 - frac) + s[lo + 1] * frac : s[lo];
        }
        for (std::size_t i = 1; i < edges.size(); ++i) {
            if (!(edges[i] > edges[i - 1])) {
                edges[i] = std::nextafter(edges[i - 1],
                                          std::numeric_limits<double>::infinity());
            }
        }
        return edges;
    }

    /// Bin index of `x` under `edges` (size = n_bins + 1), clamped into
    /// [0, n_bins-1]. Returns -1 if `edges` doesn't define at least one bin.
    [[nodiscard]] static int bin_index(double x, const std::vector<double>& edges) {
        const int nb = static_cast<int>(edges.size()) - 1;
        if (nb < 1) return -1;
        int b = static_cast<int>(
                    std::upper_bound(edges.begin(), edges.end(), x) - edges.begin()) - 1;
        if (b < 0) b = 0;
        if (b >= nb) b = nb - 1;
        return b;
    }

    /// Per-bin counts of `samples` under `edges`.
    [[nodiscard]] static std::vector<long> bin_occupancy_of(
        const std::vector<double>& samples, const std::vector<double>& edges) {
        const int nb = static_cast<int>(edges.size()) - 1;
        if (nb < 1) return {};
        std::vector<long> occ(static_cast<std::size_t>(nb), 0);
        for (double x : samples) {
            const int b = bin_index(x, edges);
            if (b >= 0) occ[static_cast<std::size_t>(b)]++;
        }
        return occ;
    }

   private:
    const std::size_t   m_capacity;
    mutable std::mutex  m_mutex;
    std::vector<double> m_samples;
    std::atomic<long>   m_count{0};
};

}  // namespace framework
