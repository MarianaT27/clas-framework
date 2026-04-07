#pragma once

#include <atomic>
#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/format.h>

#include "bank_view.h"
#include "dictionary.h"
#include "reader.h"
#include "record.h"
#include "result.h"
#include "threadpool.hpp"
#include "progresstracker.hpp"

namespace hipo {

/**
 * @brief Metadata for a single file within a chain.
 *
 * Populated during chain::scan() or chain::open() and used internally
 * to build the parallel work list.
 */
struct chain_file_info {
    std::string      path;                            /**< @brief Filesystem path of the file. */
    int              index        = 0;                /**< @brief Zero-based index in the chain's file list. */
    int64_t          total_events = -1;               /**< @brief Total events in the file (-1 if not yet scanned). */
    int              num_records  = -1;               /**< @brief Number of data records (-1 if not yet scanned). */
    std::uintmax_t   file_size    = 0;                /**< @brief File size in bytes. */
    bool             valid        = true;             /**< @brief Whether the file could be opened and scanned successfully. */
    std::string      error;                           /**< @brief Error message if the file is invalid. */
    file_event_index event_index;                     /**< @brief Pre-built record index (populated during open). */
    bool             has_index    = false;             /**< @brief Whether event_index is populated. */
    std::shared_ptr<file_source> shared_source;       /**< @brief Shared file source for all threads (opened during process). */
};

/**
 * @brief Accumulated processing statistics for a chain run.
 *
 * Uses atomics so that worker threads can safely increment counters
 * during parallel processing.
 */
struct chain_stats {
    std::atomic<int64_t> total_events{0};           /**< @brief Total events targeted for processing. */
    std::atomic<int64_t> events_processed{0};       /**< @brief Number of events processed so far. */
    std::chrono::steady_clock::time_point start_time;   /**< @brief Wall-clock start time. */
    std::chrono::steady_clock::time_point end_time;     /**< @brief Wall-clock end time. */

    /** @brief Reset all counters and record a new start time. */
    void reset() noexcept {
        total_events = 0;
        events_processed = 0;
        start_time = std::chrono::steady_clock::now();
    }

    /**
     * @brief Return the elapsed wall-clock time in seconds.
     * @return Elapsed seconds between start_time and end_time.
     */
    double elapsed_seconds() const noexcept {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        return ms.count() / 1000.0;
    }

    /**
     * @brief Return the processing throughput in events per second.
     * @return Events per second, or 0 if elapsed time is zero.
     */
    double throughput() const noexcept {
        double s = elapsed_seconds();
        return s > 0 ? static_cast<double>(events_processed.load()) / s : 0.0;
    }
};

/**
 * @brief Options controlling chain behaviour.
 *
 * @see chain
 */
struct chain_options {
    int                threads        = 0;      /**< @brief Number of worker threads (0 = hardware_concurrency). */
    bool               show_progress  = true;   /**< @brief Display a progress bar during processing. */
    bool               verbose        = false;  /**< @brief Print detailed statistics after processing. */
    reader_options     reader_opts    = {};      /**< @brief Options forwarded to each per-thread reader. */
};

/**
 * @brief Concept for an event processing callback.
 *
 * Must be invocable as `void(bank_view_set&, int file_idx, int64_t event_idx)`.
 *
 * @tparam F Callable type.
 */
template <typename F>
concept EventProcessor = std::invocable<F, bank_view_set&, int, int64_t>;

/**
 * @brief Concept for an event filtering predicate.
 *
 * Must be invocable as `bool(const bank_view_set&, int file_idx, int64_t event_idx)`.
 * Returning false causes the event to be skipped (the processor callback is not invoked).
 *
 * @tparam G Callable type.
 */
template <typename G>
concept EventFilter = std::invocable<G, const bank_view_set&, int, int64_t>
    && std::same_as<std::invoke_result_t<G, const bank_view_set&, int, int64_t>, bool>;

/**
 * @brief Multi-file parallel event processor.
 *
 * A chain manages a list of HIPO files and distributes records across
 * a thread pool for parallel processing. Each worker thread gets its own
 * reader, record, and bank_view_set instances so no synchronisation is
 * needed within the user callback.
 *
 * Typical usage:
 * @code
 * hipo::chain c({.threads = 4});
 * c.add("file1.hipo").add("file2.hipo");
 * c.open();
 * c.process([](hipo::bank_view_set& banks, int file, int64_t event) {
 *     // process event ...
 * }, {"event::particle"});
 * c.print_statistics();
 * @endcode
 *
 * @note The chain itself is NOT thread-safe; its public API should be called
 *       from a single thread. The internal worker threads are managed
 *       exclusively by the process/process_to methods.
 *
 * @see reader, bank_view_set, EventProcessor, EventFilter
 */
class chain {
public:
    /**
     * @brief Construct a chain with the given options.
     * @param opts Chain options (thread count, progress display, etc.).
     */
    explicit chain(chain_options opts = {});
    ~chain() = default;

    chain(const chain&) = delete;
    chain& operator=(const chain&) = delete;

    /**
     * @brief Add a HIPO file path to the chain.
     * @param path Filesystem path to a HIPO file.
     * @return Reference to this chain (for method chaining).
     */
    chain& add(std::string_view path);

    /**
     * @brief Open all files: scan metadata and merge dictionaries.
     * @return result<void> indicating success or an error from the first invalid file.
     */
    result<void> open();

    /**
     * @brief Scan all files for metadata (event counts, record counts) without opening for reading.
     */
    void scan();

    /**
     * @brief Process events in parallel using a callback.
     *
     * Records are distributed across worker threads. Each event is
     * deserialized into a thread-local bank_view_set and passed to the
     * callback.
     *
     * @tparam F Callable satisfying EventProcessor.
     * @param func           Callback invoked for each event.
     * @param required_banks Bank names to bind in each thread's bank_view_set.
     * @param pct            Percentage of total events to process (0.0 - 100.0).
     */
    template <EventProcessor F>
    void process(F&& func, std::vector<std::string> required_banks, double pct = 100.0);

    /**
     * @brief Process events in parallel with an event filter.
     *
     * Events for which the filter returns false are skipped (the processor
     * callback is not invoked for those events).
     *
     * @tparam F Callable satisfying EventProcessor.
     * @tparam G Callable satisfying EventFilter.
     * @param func           Callback invoked for each accepted event.
     * @param filter         Predicate returning true for events to process.
     * @param required_banks Bank names to bind in each thread's bank_view_set.
     * @param pct            Percentage of total events to process (0.0 - 100.0).
     */
    template <EventProcessor F, EventFilter G>
    void process(F&& func, G&& filter, std::vector<std::string> required_banks, double pct = 100.0);

    /**
     * @brief Process events with map-reduce: each event produces a value, results are combined.
     *
     * Each worker thread accumulates a thread-local result via the reducer.
     * After all events are processed, thread-local results are merged into
     * a single global result.
     *
     * @tparam R       Result type (must be movable and copyable).
     * @tparam F       Callable producing R from (bank_view_set&, int, int64_t).
     * @tparam Reducer Callable combining two R values into one.
     * @param func           Map function invoked per event.
     * @param reducer        Binary function combining partial results.
     * @param init           Initial value for each thread-local accumulator.
     * @param required_banks Bank names to bind in each thread's bank_view_set.
     * @param pct            Percentage of total events to process (0.0 - 100.0).
     * @return The final reduced result.
     */
    template <typename R, typename F, typename Reducer>
        requires std::invocable<F, bank_view_set&, int, int64_t>
              && std::convertible_to<std::invoke_result_t<F, bank_view_set&, int, int64_t>, R>
              && std::invocable<Reducer, R, R>
              && std::movable<R> && std::copyable<R>
    R process_to(F&& func, Reducer&& reducer, R init,
                 std::vector<std::string> required_banks, double pct = 100.0);

    /**
     * @brief Process events with map-reduce and an event filter.
     *
     * Combines filtering and reduction: only events accepted by the filter
     * contribute to the reduced result.
     *
     * @tparam R       Result type (must be movable and copyable).
     * @tparam F       Callable producing R from (bank_view_set&, int, int64_t).
     * @tparam G       Callable satisfying EventFilter.
     * @tparam Reducer Callable combining two R values into one.
     * @param func           Map function invoked per accepted event.
     * @param filter         Predicate returning true for events to include.
     * @param reducer        Binary function combining partial results.
     * @param init           Initial value for each thread-local accumulator.
     * @param required_banks Bank names to bind in each thread's bank_view_set.
     * @param pct            Percentage of total events to process (0.0 - 100.0).
     * @return The final reduced result.
     */
    template <typename R, typename F, EventFilter G, typename Reducer>
        requires std::invocable<F, bank_view_set&, int, int64_t>
              && std::convertible_to<std::invoke_result_t<F, bank_view_set&, int, int64_t>, R>
              && std::invocable<Reducer, R, R>
              && std::movable<R> && std::copyable<R>
    R process_to(F&& func, G&& filter, Reducer&& reducer, R init,
                 std::vector<std::string> required_banks, double pct = 100.0);

    /**
     * @brief Return a const reference to the accumulated processing statistics.
     * @return The chain_stats struct.
     */
    const chain_stats& statistics() const noexcept { return m_stats; }

    /**
     * @brief Return the total number of events across all valid files.
     *
     * Triggers a metadata scan if not already performed.
     *
     * @return Total event count.
     */
    int64_t total_events();

    /** @brief Return a const reference to the merged schema dictionary. */
    const dictionary& dict() const noexcept { return m_dict; }

    /** @brief Return the number of files in the chain. */
    int num_files() const noexcept { return static_cast<int>(m_files.size()); }

    /** @brief Print processing statistics (elapsed time, throughput, event counts) to stdout. */
    void print_statistics() const;

private:
    void load_metadata();

    std::vector<chain_file_info>  m_files;
    chain_options                 m_opts;
    chain_stats                   m_stats;
    dictionary                    m_dict;
    bool                          m_metadata_loaded = false;
    int                           m_num_threads;
    std::unique_ptr<ThreadPool>   m_thread_pool;
};

// ============================================================================
// Template implementation
// ============================================================================

template <EventProcessor F>
void chain::process(F&& func, std::vector<std::string> required_banks, double pct) {
    process(std::forward<F>(func),
            [](const bank_view_set&, int, int64_t) -> bool { return true; },
            std::move(required_banks), pct);
}

template <EventProcessor F, EventFilter G>
void chain::process(F&& func, G&& filter, std::vector<std::string> required_banks, double pct) {
    if (m_files.empty()) return;

    pct = std::clamp(pct, 0.0, 100.0) / 100.0;
    load_metadata();

    int64_t target = static_cast<int64_t>(m_stats.total_events.load() * pct);
    m_stats.reset();
    m_stats.total_events = target;

    // Build work list with per-file event limits
    struct work_item {
        int file_idx;
        int num_records;
        int64_t max_events;
    };
    std::vector<work_item> work_list;
    int64_t accumulated = 0;
    for (auto const& f : m_files) {
        if (!f.valid || accumulated >= target) continue;
        int64_t needed = std::min(target - accumulated, f.total_events);
        if (needed > 0) {
            work_list.push_back({f.index, f.num_records, needed});
            accumulated += needed;
        }
    }

    if (work_list.empty()) return;

    std::unique_ptr<ProgressTracker> progress;
    if (m_opts.show_progress && accumulated > 0) {
        ProgressTracker::Config config;
        config.label = fmt::format("Processing ({:.1f}%)", pct * 100.0);
        config.show_eta = true;
        config.show_rate = true;
        progress = std::make_unique<ProgressTracker>(accumulated, config);
        progress->start();
    }

    // Per-file event counters for max_events enforcement
    std::vector<std::atomic<int64_t>> events_per_file(work_list.size());
    for (auto& e : events_per_file) e.store(0);

    // Open one shared file_source per file (single fd shared by all threads)
    for (auto& work : work_list) {
        auto& file = m_files[work.file_idx];
        if (!file.has_index) continue;
        auto src = file_source::open(file.path);
        if (!src) { file.valid = false; continue; }
        file.shared_source = std::make_shared<file_source>(std::move(*src));
    }

    // Build global work queue across all files
    struct global_record {
        int work_idx;
        int record_idx;
    };
    std::vector<global_record> global_queue;
    for (int w = 0; w < static_cast<int>(work_list.size()); ++w) {
        for (int r = 0; r < work_list[w].num_records; ++r) {
            global_queue.push_back({w, r});
        }
    }

    // Prefetch: tell kernel to start loading all record data into page cache
    for (auto const& item : global_queue) {
        auto const& work = work_list[item.work_idx];
        auto const& file = m_files[work.file_idx];
        if (!file.shared_source) continue;
        int64_t pos = file.event_index.record_position(item.record_idx);
        int bytes = file.event_index.record_bytes(item.record_idx);
        if (bytes > 0) {
            file.shared_source->prefetch(pos, bytes);
        }
    }

    std::atomic<int> next_global{0};
    int global_size = static_cast<int>(global_queue.size());

    std::vector<std::future<void>> futures;
    for (int t = 0; t < m_num_threads; ++t) {
        futures.push_back(m_thread_pool->submit([&, this]() {
            bank_view_set banks;
            for (auto const& name : required_banks)
                banks.require(name);
            auto bind_res = banks.bind(m_dict);
            if (!bind_res) return;

            record rec;

            while (true) {
                int idx = next_global.fetch_add(1);
                if (idx >= global_size) break;

                auto [work_idx, rec_idx] = global_queue[idx];
                auto const& work = work_list[work_idx];
                auto const& file = m_files[work.file_idx];

                if (events_per_file[work_idx].load(std::memory_order_relaxed) >= work.max_events)
                    continue;

                if (!file.shared_source) continue;

                auto load_res = reader::load_record_from(
                    *file.shared_source, file.event_index, rec, rec_idx,
                    static_cast<int64_t>(file.file_size));
                if (!load_res) continue;

                int events_in_record = rec.event_count();
                for (int e = 0; e < events_in_record; ++e) {
                    int64_t count = events_per_file[work_idx].fetch_add(1);
                    if (count >= work.max_events) {
                        events_per_file[work_idx].fetch_sub(1);
                        break;
                    }

                    (void)banks.update_from_record(rec, e);

                    if (!filter(banks, file.index, count)) {
                        if (progress) progress->increment();
                        m_stats.events_processed++;
                        continue;
                    }

                    func(banks, file.index, count);
                    m_stats.events_processed++;
                    if (progress) progress->increment();
                }
            }
        }));
    }

    for (auto& f : futures) f.get();

    // Release shared sources
    for (auto& work : work_list) {
        m_files[work.file_idx].shared_source.reset();
    }

    m_stats.end_time = std::chrono::steady_clock::now();
    if (progress) progress->finish();
    if (m_opts.verbose) print_statistics();
}

// --- process_to: non-filter overload delegates to filter overload ---

template <typename R, typename F, typename Reducer>
    requires std::invocable<F, bank_view_set&, int, int64_t>
          && std::convertible_to<std::invoke_result_t<F, bank_view_set&, int, int64_t>, R>
          && std::invocable<Reducer, R, R>
          && std::movable<R> && std::copyable<R>
R chain::process_to(F&& func, Reducer&& reducer, R init,
                    std::vector<std::string> required_banks, double pct) {
    return process_to<R>(
        std::forward<F>(func),
        [](const bank_view_set&, int, int64_t) -> bool { return true; },
        std::forward<Reducer>(reducer),
        std::move(init),
        std::move(required_banks),
        pct);
}

template <typename R, typename F, EventFilter G, typename Reducer>
    requires std::invocable<F, bank_view_set&, int, int64_t>
          && std::convertible_to<std::invoke_result_t<F, bank_view_set&, int, int64_t>, R>
          && std::invocable<Reducer, R, R>
          && std::movable<R> && std::copyable<R>
R chain::process_to(F&& func, G&& filter, Reducer&& reducer, R init,
                    std::vector<std::string> required_banks, double pct) {
    if (m_files.empty()) return init;

    pct = std::clamp(pct, 0.0, 100.0) / 100.0;
    load_metadata();

    int64_t target = static_cast<int64_t>(m_stats.total_events.load() * pct);
    m_stats.reset();
    m_stats.total_events = target;

    struct work_item {
        int file_idx;
        int num_records;
        int64_t max_events;
    };
    std::vector<work_item> work_list;
    int64_t accumulated = 0;
    for (auto const& f : m_files) {
        if (!f.valid || accumulated >= target) continue;
        int64_t needed = std::min(target - accumulated, f.total_events);
        if (needed > 0) {
            work_list.push_back({f.index, f.num_records, needed});
            accumulated += needed;
        }
    }

    if (work_list.empty()) return init;

    std::unique_ptr<ProgressTracker> progress;
    if (m_opts.show_progress && accumulated > 0) {
        ProgressTracker::Config config;
        config.label = fmt::format("Reducing ({:.1f}%)", pct * 100.0);
        config.show_eta = true;
        config.show_rate = true;
        progress = std::make_unique<ProgressTracker>(accumulated, config);
        progress->start();
    }

    std::vector<std::atomic<int64_t>> events_per_file(work_list.size());
    for (auto& e : events_per_file) e.store(0);

    R global_result = init;
    std::mutex result_mutex;

    // Open one shared file_source per file
    for (auto& work : work_list) {
        auto& file = m_files[work.file_idx];
        if (!file.has_index) continue;
        auto src = file_source::open(file.path);
        if (!src) { file.valid = false; continue; }
        file.shared_source = std::make_shared<file_source>(std::move(*src));
    }

    struct global_record {
        int work_idx;
        int record_idx;
    };
    std::vector<global_record> global_queue;
    for (int w = 0; w < static_cast<int>(work_list.size()); ++w) {
        for (int r = 0; r < work_list[w].num_records; ++r) {
            global_queue.push_back({w, r});
        }
    }

    // Prefetch all records
    for (auto const& item : global_queue) {
        auto const& work = work_list[item.work_idx];
        auto const& file = m_files[work.file_idx];
        if (!file.shared_source) continue;
        int64_t pos = file.event_index.record_position(item.record_idx);
        int bytes = file.event_index.record_bytes(item.record_idx);
        if (bytes > 0) {
            file.shared_source->prefetch(pos, bytes);
        }
    }

    std::atomic<int> next_global{0};
    int global_size = static_cast<int>(global_queue.size());

    std::vector<std::future<R>> futures;
    for (int t = 0; t < m_num_threads; ++t) {
        futures.push_back(m_thread_pool->submit([&, local_init = init, this]() mutable -> R {
            R local = std::move(local_init);

            bank_view_set banks;
            for (auto const& name : required_banks)
                banks.require(name);
            auto bind_res = banks.bind(m_dict);
            if (!bind_res) return local;

            record rec;

            while (true) {
                int idx = next_global.fetch_add(1);
                if (idx >= global_size) break;

                auto [work_idx, rec_idx] = global_queue[idx];
                auto const& work = work_list[work_idx];
                auto const& file = m_files[work.file_idx];

                if (events_per_file[work_idx].load(std::memory_order_relaxed) >= work.max_events)
                    continue;

                if (!file.shared_source) continue;

                auto load_res = reader::load_record_from(
                    *file.shared_source, file.event_index, rec, rec_idx,
                    static_cast<int64_t>(file.file_size));
                if (!load_res) continue;

                int events_in_record = rec.event_count();
                for (int e = 0; e < events_in_record; ++e) {
                    int64_t count = events_per_file[work_idx].fetch_add(1);
                    if (count >= work.max_events) {
                        events_per_file[work_idx].fetch_sub(1);
                        break;
                    }

                    (void)banks.update_from_record(rec, e);

                    if (!filter(banks, file.index, count)) {
                        if (progress) progress->increment();
                        m_stats.events_processed++;
                        continue;
                    }

                    local = reducer(std::move(local),
                                    static_cast<R>(func(banks, file.index, count)));
                    m_stats.events_processed++;
                    if (progress) progress->increment();
                }
            }
            return local;
        }));
    }

    for (auto& f : futures) {
        R thread_result = f.get();
        std::lock_guard lock(result_mutex);
        global_result = reducer(std::move(global_result), std::move(thread_result));
    }

    // Release shared sources
    for (auto& work : work_list) {
        m_files[work.file_idx].shared_source.reset();
    }

    m_stats.end_time = std::chrono::steady_clock::now();
    if (progress) progress->finish();
    if (m_opts.verbose) print_statistics();

    return global_result;
}

} // namespace hipo
