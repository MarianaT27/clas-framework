#pragma once

#include <algorithm>
#include <cstddef>
#include <span>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "mh/thread.hpp"

namespace mh {

// Clone-and-merge parallel fill. Works with any storage type — each worker
// fills a private clone via fill_n, and the clones are tree-merged at the
// end. Returns the merged histogram.
//
// Spans must all have equal length. n_threads is clamped to [1, n_points].
template <typename Hist, typename... Spans>
    requires (sizeof...(Spans) == Hist::rank)
[[nodiscard]] Hist parallel_fill(const Hist& proto,
                                 std::size_t n_threads,
                                 Spans... cols)
{
    const std::array<std::size_t, sizeof...(Spans)> sizes{ cols.size()... };
    const std::size_t n = sizes[0];
    for (std::size_t k = 1; k < sizes.size(); ++k) {
        if (sizes[k] != n) return proto;
    }
    if (n == 0 || n_threads == 0) return proto;
    n_threads = std::min(n_threads, n);

    thread_pool<Hist> pool(proto, n_threads);
    auto spans = std::tuple{cols...};

    {
        std::vector<std::jthread> workers;
        workers.reserve(n_threads);
        const std::size_t chunk = (n + n_threads - 1) / n_threads;
        for (std::size_t t = 0; t < n_threads; ++t) {
            const std::size_t base = t * chunk;
            const std::size_t len  = std::min(chunk, n - base);
            workers.emplace_back([&, t, base, len] {
                [&]<std::size_t... Ks>(std::index_sequence<Ks...>) {
                    pool[t].fill_n(std::get<Ks>(spans).subspan(base, len)...);
                }(std::index_sequence_for<Spans...>{});
            });
        }
    }  // jthreads join here

    return pool.merge();
}

template <typename Hist, typename... Spans>
    requires (sizeof...(Spans) == Hist::rank)
[[nodiscard]] Hist parallel_fill_weighted(const Hist& proto,
                                          std::size_t n_threads,
                                          std::span<const double> w,
                                          Spans... cols)
{
    const std::array<std::size_t, sizeof...(Spans)> sizes{ cols.size()... };
    const std::size_t n = sizes[0];
    for (std::size_t k = 1; k < sizes.size(); ++k) {
        if (sizes[k] != n) return proto;
    }
    if (w.size() != n) return proto;
    if (n == 0 || n_threads == 0) return proto;
    n_threads = std::min(n_threads, n);

    thread_pool<Hist> pool(proto, n_threads);
    auto spans = std::tuple{cols...};

    {
        std::vector<std::jthread> workers;
        workers.reserve(n_threads);
        const std::size_t chunk = (n + n_threads - 1) / n_threads;
        for (std::size_t t = 0; t < n_threads; ++t) {
            const std::size_t base = t * chunk;
            const std::size_t len  = std::min(chunk, n - base);
            workers.emplace_back([&, t, base, len] {
                const std::span<const double> wt = w.subspan(base, len);
                [&]<std::size_t... Ks>(std::index_sequence<Ks...>) {
                    pool[t].fill_n_weighted(wt,
                        std::get<Ks>(spans).subspan(base, len)...);
                }(std::index_sequence_for<Spans...>{});
            });
        }
    }

    return pool.merge();
}

// Shared-storage parallel fill: requires Hist::is_atomic. All workers fill
// the same backing storage via atomic ops. Lower memory than clone-merge;
// best when atomic_fill specialization is competitive (CountAccumulator).
template <typename Hist, typename... Spans>
    requires (sizeof...(Spans) == Hist::rank && Hist::is_atomic)
void parallel_fill_atomic(Hist& target,
                          std::size_t n_threads,
                          Spans... cols)
{
    const std::array<std::size_t, sizeof...(Spans)> sizes{ cols.size()... };
    const std::size_t n = sizes[0];
    for (std::size_t k = 1; k < sizes.size(); ++k) {
        if (sizes[k] != n) return;
    }
    if (n == 0 || n_threads == 0) return;
    n_threads = std::min(n_threads, n);

    auto spans = std::tuple{cols...};
    std::vector<std::jthread> workers;
    workers.reserve(n_threads);
    const std::size_t chunk = (n + n_threads - 1) / n_threads;
    for (std::size_t t = 0; t < n_threads; ++t) {
        const std::size_t base = t * chunk;
        const std::size_t len  = std::min(chunk, n - base);
        workers.emplace_back([&, base, len] {
            [&]<std::size_t... Ks>(std::index_sequence<Ks...>) {
                target.fill_n(std::get<Ks>(spans).subspan(base, len)...);
            }(std::index_sequence_for<Spans...>{});
        });
    }
}

template <typename Hist, typename... Spans>
    requires (sizeof...(Spans) == Hist::rank && Hist::is_atomic)
void parallel_fill_atomic_weighted(Hist& target,
                                   std::size_t n_threads,
                                   std::span<const double> w,
                                   Spans... cols)
{
    const std::array<std::size_t, sizeof...(Spans)> sizes{ cols.size()... };
    const std::size_t n = sizes[0];
    for (std::size_t k = 1; k < sizes.size(); ++k) {
        if (sizes[k] != n) return;
    }
    if (w.size() != n) return;
    if (n == 0 || n_threads == 0) return;
    n_threads = std::min(n_threads, n);

    auto spans = std::tuple{cols...};
    std::vector<std::jthread> workers;
    workers.reserve(n_threads);
    const std::size_t chunk = (n + n_threads - 1) / n_threads;
    for (std::size_t t = 0; t < n_threads; ++t) {
        const std::size_t base = t * chunk;
        const std::size_t len  = std::min(chunk, n - base);
        workers.emplace_back([&, base, len] {
            const std::span<const double> wt = w.subspan(base, len);
            [&]<std::size_t... Ks>(std::index_sequence<Ks...>) {
                target.fill_n_weighted(wt,
                    std::get<Ks>(spans).subspan(base, len)...);
            }(std::index_sequence_for<Spans...>{});
        });
    }
}

}
