#pragma once

#include "filehandle.h"
#include "record.h"
#include "recordcache.h"

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>

namespace hipo {

///
/// @brief RNTuple-RClusterPool analog for HIPO records.
///
/// Owns 1+ dedicated I/O threads that asynchronously fetch + decompress
/// records from a FileHandle and stash them in a rendezvous map (and the
/// optional shared chain-wide cache). Consumer threads pull through `get()`,
/// which returns immediately on cache hit and otherwise blocks until the
/// record is ready.
///
/// Lifetime is tied to a single FileHandle / file. The chain creates a new
/// RecordPrefetcher when it switches to a new file in the work list.
///
class RecordPrefetcher {
public:
    ///
    /// @param fh             open FileHandle owned by the chain
    /// @param file_idx       global file index (for cache keys)
    /// @param cache          optional cross-file LRU; may be nullptr
    /// @param io_threads     >= 1
    /// @param prefetch_depth maximum pending fetches in flight (0 = unbounded)
    ///
    RecordPrefetcher(std::shared_ptr<FileHandle> fh,
                     int file_idx,
                     RecordCache* cache,
                     int io_threads,
                     int prefetch_depth)
        : m_fh(std::move(fh)),
          m_file_idx(file_idx),
          m_cache(cache),
          m_prefetch_depth(prefetch_depth) {
        if (io_threads < 1) io_threads = 1;
        m_io_threads.reserve(static_cast<std::size_t>(io_threads));
        for (int i = 0; i < io_threads; ++i) {
            m_io_threads.emplace_back(&RecordPrefetcher::io_worker_loop, this);
        }
    }

    RecordPrefetcher(const RecordPrefetcher&) = delete;
    RecordPrefetcher& operator=(const RecordPrefetcher&) = delete;

    ~RecordPrefetcher() { shutdown(); }

    /// Queue record indices [begin, end) for the I/O thread to fetch. Returns
    /// promptly. Already-cached or already-in-flight indices are skipped.
    void prefetch_range(int begin, int end) {
        if (!m_fh) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        for (int i = begin; i < end; ++i) {
            schedule_locked(i);
        }
        m_cv_work.notify_all();
    }

    /// Get record `rec_idx`. Cache hit → returns immediately. Otherwise
    /// blocks until the I/O thread completes the fetch (scheduling one if
    /// no prefetch already queued it).
    std::shared_ptr<hipo::record> get(int rec_idx) {
        if (!m_fh) return nullptr;
        // Cache short-circuit.
        if (m_cache) {
            if (auto cached = m_cache->get(m_file_idx, rec_idx)) {
                return cached;
            }
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        while (true) {
            // Pending bucket (filled by I/O thread, drained by consumers).
            auto it = m_pending.find(rec_idx);
            if (it != m_pending.end()) {
                auto result = it->second;
                m_pending.erase(it);
                if (result && m_cache) {
                    // Re-publish to cache so other consumers can find it too.
                    m_cache->put(m_file_idx, rec_idx, result);
                }
                return result;
            }
            // Already queued / executing — wait for the I/O thread.
            if (m_in_flight.count(rec_idx)) {
                m_cv_ready.wait(lock);
                continue;
            }
            // Not in flight — schedule and wait.
            schedule_locked(rec_idx);
            m_cv_work.notify_one();
            m_cv_ready.wait(lock);
        }
    }

    /// Stop the I/O threads and drain the queue. Idempotent.
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_stop) return;
            m_stop = true;
            m_queue.clear();
        }
        m_cv_work.notify_all();
        m_cv_ready.notify_all();
        for (auto& t : m_io_threads) {
            if (t.joinable()) t.join();
        }
        m_io_threads.clear();
    }

private:
    // Caller must hold m_mutex.
    void schedule_locked(int rec_idx) {
        if (rec_idx < 0 || rec_idx >= m_fh->num_records()) return;
        if (m_in_flight.count(rec_idx)) return;
        // Cache may already have it; skip a redundant fetch.
        if (m_cache && m_cache->get(m_file_idx, rec_idx)) return;
        // Backpressure: don't run more than prefetch_depth concurrently.
        if (m_prefetch_depth > 0 &&
            static_cast<int>(m_in_flight.size()) >= m_prefetch_depth) {
            // Drop: the consumer will re-trigger via get() if it still needs
            // this record. Avoids unbounded queue growth when a worker
            // pre-hints far ahead.
            return;
        }
        m_in_flight.insert(rec_idx);
        m_queue.push_back(rec_idx);
    }

    void io_worker_loop() {
        while (true) {
            int rec_idx;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv_work.wait(lock, [this]{ return !m_queue.empty() || m_stop; });
                if (m_stop && m_queue.empty()) return;
                rec_idx = m_queue.front();
                m_queue.pop_front();
            }

            std::shared_ptr<hipo::record> rec;
            try {
                rec = m_fh->loadRecordShared(rec_idx);
            } catch (...) {
                rec = nullptr;
            }

            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_cache && rec) {
                    m_cache->put(m_file_idx, rec_idx, rec);
                }
                m_pending[rec_idx] = rec;
                m_in_flight.erase(rec_idx);
            }
            m_cv_ready.notify_all();
        }
    }

    std::shared_ptr<FileHandle>           m_fh;
    int                                    m_file_idx = 0;
    RecordCache*                           m_cache    = nullptr;
    int                                    m_prefetch_depth = 0;

    std::vector<std::thread>               m_io_threads;
    std::deque<int>                        m_queue;
    std::unordered_map<int, std::shared_ptr<hipo::record>> m_pending;
    std::set<int>                          m_in_flight;
    std::mutex                             m_mutex;
    std::condition_variable                m_cv_work;
    std::condition_variable                m_cv_ready;
    bool                                   m_stop = false;
};

} // namespace hipo
