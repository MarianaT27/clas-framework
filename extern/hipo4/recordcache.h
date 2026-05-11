#pragma once

#include "record.h"

#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <utility>

namespace hipo {

///
/// @brief Thread-safe byte-bounded LRU cache of decompressed HIPO records.
///
/// Keyed by (file_idx, record_idx). Designed for sharing across the workers
/// of a hipo::chain so two threads needing the same record incur only one
/// network fetch + decompress.
///
/// Capacity is expressed in *decompressed bytes*. Insertions that would push
/// total bytes over the cap evict least-recently-used entries until the cap
/// is satisfied. If a single entry exceeds the cap, the cache is left empty
/// and the entry is dropped on the floor (caller still gets its shared_ptr).
///
class RecordCache {
public:
    using key_t   = std::pair<int, int>;  // (file_idx, record_idx)
    using value_t = std::shared_ptr<hipo::record>;

    RecordCache() = default;
    explicit RecordCache(std::size_t capacity_bytes)
        : m_capacity_bytes(capacity_bytes) {}

    void set_capacity(std::size_t bytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_capacity_bytes = bytes;
        evict_locked();
    }

    std::size_t capacity_bytes() const noexcept { return m_capacity_bytes; }
    std::size_t size_bytes()     const noexcept { return m_size_bytes; }

    /// Look up a record. Returns nullptr on miss. Moves to MRU on hit.
    value_t get(int file_idx, int record_idx) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_map.find({file_idx, record_idx});
        if (it == m_map.end()) {
            ++m_misses;
            return nullptr;
        }
        // Move to MRU.
        m_lru.splice(m_lru.begin(), m_lru, it->second.lru_iter);
        it->second.lru_iter = m_lru.begin();
        ++m_hits;
        return it->second.value;
    }

    /// Insert a record. If already present, replaces and moves to MRU.
    void put(int file_idx, int record_idx, value_t value) {
        if (!value) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        const std::size_t bytes = estimate_bytes(*value);
        key_t k{file_idx, record_idx};

        auto it = m_map.find(k);
        if (it != m_map.end()) {
            m_size_bytes -= it->second.bytes;
            m_lru.erase(it->second.lru_iter);
            m_map.erase(it);
        }
        if (m_capacity_bytes == 0) return;
        if (bytes > m_capacity_bytes) {
            // Single-entry too large — don't pollute the cache.
            return;
        }
        m_lru.push_front(k);
        Entry e{m_lru.begin(), std::move(value), bytes};
        m_size_bytes += bytes;
        m_map.emplace(k, std::move(e));
        evict_locked();
    }

    /// Drop everything.
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_map.clear();
        m_lru.clear();
        m_size_bytes = 0;
    }

    /// Cumulative cache-hit / cache-miss counters since construction.
    std::size_t hits()   const noexcept { return m_hits; }
    std::size_t misses() const noexcept { return m_misses; }

private:
    struct Entry {
        std::list<key_t>::iterator lru_iter;
        value_t                    value;
        std::size_t                bytes;
    };

    struct PairHash {
        std::size_t operator()(const key_t& k) const noexcept {
            return (static_cast<std::size_t>(k.first) << 32) ^
                   static_cast<std::size_t>(k.second);
        }
    };

    static std::size_t estimate_bytes(const hipo::record& /*r*/) noexcept {
        // We don't have direct access to the record's internal buffer sizes
        // without changing record.h's API. Use a conservative default that
        // matches the worst-case HIPO record size (~8 MB per
        // docs/reference/performance.md). Tighten later if record.h grows
        // a memoryFootprint() accessor.
        return 8 * 1024 * 1024;
    }

    void evict_locked() {
        while (m_size_bytes > m_capacity_bytes && !m_lru.empty()) {
            auto& victim_key = m_lru.back();
            auto it = m_map.find(victim_key);
            if (it != m_map.end()) {
                m_size_bytes -= it->second.bytes;
                m_map.erase(it);
            }
            m_lru.pop_back();
        }
    }

    mutable std::mutex                                  m_mutex;
    std::list<key_t>                                    m_lru;          // front = MRU
    std::unordered_map<key_t, Entry, PairHash>          m_map;
    std::size_t                                         m_capacity_bytes = 0;
    std::size_t                                         m_size_bytes     = 0;
    std::size_t                                         m_hits   = 0;
    std::size_t                                         m_misses = 0;
};

} // namespace hipo
