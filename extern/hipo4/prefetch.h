#pragma once

/**
 * @file prefetch.h
 * @brief Async producer/consumer record prefetcher (TFilePrefetch-style).
 *
 * Hides I/O + decompression latency by running them on a dedicated thread
 * ahead of the event-processing workers. A small ring of pre-decompressed
 * `record` slots lets the producer build up a backlog while the workers
 * stay busy on parsing + algorithm work.
 *
 * On warm-cache local-NVMe runs the win is small (`pread` already hits the
 * page cache). The real reason this exists is `ifarm`-style network
 * storage where each `pread` round-trips over the network — there the
 * producer can soak up several records' worth of latency in parallel
 * with the workers' compute.
 *
 * Usage:
 * @code
 *   record_prefetcher pf(8, [&](record_prefetcher::slot& out) {
 *       int idx = next_global.fetch_add(1);
 *       if (idx >= total) return false;
 *       // ... fill out.rec, out.work_idx, etc. Return true once filled.
 *       return true;
 *   });
 *   pf.start();
 *   while (auto* s = pf.pop()) {
 *       // ... process s->rec ...
 *       pf.release(s);
 *   }
 *   pf.stop();
 * @endcode
 */

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "record.h"

namespace hipo {

/**
 * @brief Bounded ring of pre-decompressed records, filled by one producer
 *        thread and consumed by N worker threads.
 *
 * Lifecycle: construct → `start()` (spawns producer thread) → workers
 * call `pop()` / `release()` in a loop → `stop()` (or destructor) joins
 * the producer.
 */
class record_prefetcher {
public:
    /// One slot in the ring. The producer fills `rec` + bookkeeping
    /// fields; the worker reads them. `ok == false` means the load
    /// failed for that record — consumers should release without
    /// processing.
    struct slot {
        record  rec;
        int     work_idx = -1;
        int     rec_idx  = -1;
        int     file_idx = -1;
        bool    ok       = false;
    };

    /// Producer callback. Fills `out` with the next record. Return
    /// true if a record was produced (even if the load failed —
    /// `out.ok` carries that flag). Return false to signal "no more
    /// work".
    using producer_fn = std::function<bool(slot& out)>;

    /// @param capacity     Number of ring slots. Tune to roughly the
    ///                     number of consumers + a small backlog.
    ///                     Larger values smooth out variance at the
    ///                     cost of memory (each slot owns a
    ///                     decompressed buffer).
    /// @param num_producers Number of producer threads. With 1
    ///                     producer the entire chain throughput is
    ///                     gated on a single thread's pread +
    ///                     decompress rate, which is the bottleneck
    ///                     for compute-bound multithreaded jobs.
    ///                     Pass `num_threads` to keep the
    ///                     decompression parallelism the old
    ///                     synchronous design had, while still
    ///                     hiding I/O latency for ifarm-style
    ///                     workloads.
    /// @param fill         Producer callback (see above). It is
    ///                     called concurrently from every producer
    ///                     thread, so the closure must be
    ///                     thread-safe (typically pulls work out of
    ///                     a shared `std::atomic<int>` counter).
    record_prefetcher(int capacity, int num_producers, producer_fn fill);

    ~record_prefetcher();

    record_prefetcher(const record_prefetcher&)            = delete;
    record_prefetcher& operator=(const record_prefetcher&) = delete;

    /// Spawn the producer thread. Call once before the first `pop()`.
    void start();

    /// Signal the producer to exit, then join it. Safe to call
    /// multiple times. The destructor calls this.
    void stop();

    /// Block until a ready slot is available. Returns nullptr when
    /// the producer is done AND the ring is drained.
    [[nodiscard]] auto pop() -> slot*;

    /// Return a slot to the free pool for the producer to refill.
    /// Must be called for every non-null pointer returned by `pop()`.
    auto release(slot* s) -> void;

private:
    void producer_loop();

    std::vector<std::unique_ptr<slot>> m_slots;        ///< Owns the slot storage.
    std::queue<slot*>                  m_free;         ///< Slots the producer will refill.
    std::queue<slot*>                  m_ready;        ///< Slots filled, waiting for a consumer.

    mutable std::mutex                 m_mu;
    std::condition_variable            m_cv_free;      ///< Producer waits when no free slots.
    std::condition_variable            m_cv_ready;     ///< Consumers wait when no ready slots.

    bool m_done    = false;        ///< All producers have finished (no more work).
    bool m_stop    = false;        ///< Producers should bail out ASAP.
    int  m_active_producers = 0;   ///< Number still running; m_done flips when this hits 0.

    producer_fn              m_fill;
    int                      m_num_producers;
    std::vector<std::thread> m_threads;
};

}  // namespace hipo
