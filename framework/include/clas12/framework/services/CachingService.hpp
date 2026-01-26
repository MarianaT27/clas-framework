#pragma once

/**
 * @file CachingService.hpp
 * @brief Service for caching expensive computations
 *
 * Provides a thread-safe cache for storing and retrieving computed values.
 * Useful for caching results of expensive calculations that may be
 * needed multiple times during event processing.
 */

#include <any>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <typeindex>
#include <unordered_map>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

/**
 * @brief Statistics about cache performance
 */
struct CacheStats {
    std::size_t hits = 0;           ///< Number of cache hits
    std::size_t misses = 0;         ///< Number of cache misses
    std::size_t evictions = 0;      ///< Number of cache evictions
    std::size_t current_size = 0;   ///< Current number of entries
    std::size_t max_size = 0;       ///< Maximum allowed entries
    std::size_t memory_bytes = 0;   ///< Approximate memory usage in bytes

    /**
     * @brief Get the cache hit rate
     * @return double Hit rate as a fraction (0.0 to 1.0)
     */
    [[nodiscard]] auto hit_rate() const -> double {
        auto total = hits + misses;
        return total > 0 ? static_cast<double>(hits) / static_cast<double>(total) : 0.0;
    }
};

/**
 * @brief Entry metadata for cache management
 */
struct CacheEntryMeta {
    std::chrono::steady_clock::time_point created;
    std::chrono::steady_clock::time_point last_accessed;
    std::size_t access_count = 0;
    std::size_t estimated_size = 0;  // Estimated memory size in bytes
};

/**
 * @brief Service for caching expensive computations
 *
 * Provides a type-safe, thread-safe cache with configurable eviction policies.
 * The cache supports:
 * - Type-safe storage and retrieval
 * - Maximum entry count limit
 * - Maximum memory limit (approximate)
 * - LRU (Least Recently Used) eviction
 * - TTL (Time To Live) expiration
 *
 * Example usage:
 * @code
 * auto& cache = *svc_mgr.get_service<CachingService>("CachingService");
 *
 * // Get or compute a value
 * auto result = cache.get_or_compute<double>("expensive_calc", [&]() {
 *     return expensive_computation();
 * });
 *
 * // Check if value exists
 * if (auto val = cache.get<double>("key")) {
 *     use(*val);
 * }
 * @endcode
 */
class CachingService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "CachingService"; }

    explicit CachingService(std::string name = "CachingService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        Log::info(this->name(), "Initialized (max_entries={}, max_memory={})",
                  m_max_entries, m_max_memory_bytes);
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        auto stats = this->stats();
        Log::info(this->name(), "Finalized (hits={}, misses={}, hit_rate={:.2f}%)",
                  stats.hits, stats.misses, stats.hit_rate() * 100.0);
        clear();
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    /**
     * @brief Set the maximum number of cache entries
     * @param max_entries Maximum entries (0 = unlimited)
     */
    auto set_max_entries(std::size_t max_entries) -> void {
        std::unique_lock lock(m_mutex);
        m_max_entries = max_entries;
    }

    /**
     * @brief Set the maximum memory usage in bytes
     * @param max_bytes Maximum memory in bytes (0 = unlimited)
     */
    auto set_max_memory(std::size_t max_bytes) -> void {
        std::unique_lock lock(m_mutex);
        m_max_memory_bytes = max_bytes;
    }

    /**
     * @brief Set the default TTL for cache entries
     * @param ttl Time to live (0 = no expiration)
     */
    auto set_default_ttl(std::chrono::seconds ttl) -> void {
        std::unique_lock lock(m_mutex);
        m_default_ttl = ttl;
    }

    // ========================================================================
    // Cache Operations
    // ========================================================================

    /**
     * @brief Get a value from the cache or compute and store it
     *
     * If the key exists in the cache, returns the cached value.
     * Otherwise, calls the compute function, stores the result, and returns it.
     *
     * @tparam T Value type
     * @tparam F Callable type that returns T
     * @param key Cache key
     * @param compute Function to compute the value if not cached
     * @param estimated_size Optional estimated size in bytes for eviction
     * @return T& Reference to the cached value
     */
    template <typename T, typename F>
    auto get_or_compute(const std::string& key, F&& compute,
                        std::size_t estimated_size = sizeof(T)) -> T {
        // First, try to get with shared lock (fast path)
        {
            std::shared_lock lock(m_mutex);
            auto it = m_cache.find(key);
            if (it != m_cache.end() && !is_expired(it->second.meta)) {
                // Update access metadata
                it->second.meta.last_accessed = std::chrono::steady_clock::now();
                it->second.meta.access_count++;
                m_stats.hits++;

                try {
                    return std::any_cast<T>(it->second.value);
                } catch (const std::bad_any_cast&) {
                    // Type mismatch - will recompute below
                }
            }
        }

        // Cache miss - compute and store with exclusive lock
        std::unique_lock lock(m_mutex);

        // Double-check after acquiring exclusive lock
        auto it = m_cache.find(key);
        if (it != m_cache.end() && !is_expired(it->second.meta)) {
            it->second.meta.last_accessed = std::chrono::steady_clock::now();
            it->second.meta.access_count++;
            m_stats.hits++;

            try {
                return std::any_cast<T>(it->second.value);
            } catch (const std::bad_any_cast&) {
                // Type mismatch - remove and recompute
                m_current_memory -= it->second.meta.estimated_size;
                m_cache.erase(it);
            }
        }

        m_stats.misses++;

        // Compute the value
        T value = std::invoke(std::forward<F>(compute));

        // Evict if necessary before inserting
        evict_if_needed(estimated_size);

        // Store in cache
        CacheEntry entry;
        entry.value = value;
        entry.meta.created = std::chrono::steady_clock::now();
        entry.meta.last_accessed = entry.meta.created;
        entry.meta.access_count = 1;
        entry.meta.estimated_size = estimated_size;

        m_cache[key] = std::move(entry);
        m_current_memory += estimated_size;

        return value;
    }

    /**
     * @brief Get a value from the cache
     *
     * @tparam T Value type
     * @param key Cache key
     * @return std::optional<T> The value if found and valid type, empty otherwise
     */
    template <typename T>
    [[nodiscard]] auto get(const std::string& key) -> std::optional<T> {
        std::shared_lock lock(m_mutex);

        auto it = m_cache.find(key);
        if (it == m_cache.end() || is_expired(it->second.meta)) {
            m_stats.misses++;
            return std::nullopt;
        }

        it->second.meta.last_accessed = std::chrono::steady_clock::now();
        it->second.meta.access_count++;
        m_stats.hits++;

        try {
            return std::any_cast<T>(it->second.value);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    /**
     * @brief Store a value in the cache
     *
     * @tparam T Value type
     * @param key Cache key
     * @param value Value to store
     * @param estimated_size Optional estimated size in bytes
     */
    template <typename T>
    auto put(const std::string& key, T value,
             std::size_t estimated_size = sizeof(T)) -> void {
        std::unique_lock lock(m_mutex);

        // Remove existing entry if present
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            m_current_memory -= it->second.meta.estimated_size;
            m_cache.erase(it);
        }

        // Evict if necessary
        evict_if_needed(estimated_size);

        // Store
        CacheEntry entry;
        entry.value = std::move(value);
        entry.meta.created = std::chrono::steady_clock::now();
        entry.meta.last_accessed = entry.meta.created;
        entry.meta.access_count = 0;
        entry.meta.estimated_size = estimated_size;

        m_cache[key] = std::move(entry);
        m_current_memory += estimated_size;
    }

    /**
     * @brief Check if a key exists in the cache
     * @param key Cache key
     * @return bool True if key exists and is not expired
     */
    [[nodiscard]] auto contains(const std::string& key) const -> bool {
        std::shared_lock lock(m_mutex);
        auto it = m_cache.find(key);
        return it != m_cache.end() && !is_expired(it->second.meta);
    }

    /**
     * @brief Remove a key from the cache
     * @param key Cache key
     * @return bool True if key was removed
     */
    auto remove(const std::string& key) -> bool {
        std::unique_lock lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            m_current_memory -= it->second.meta.estimated_size;
            m_cache.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Clear all cache entries
     */
    auto clear() -> void {
        std::unique_lock lock(m_mutex);
        m_cache.clear();
        m_current_memory = 0;
    }

    /**
     * @brief Get cache statistics
     * @return CacheStats Current statistics
     */
    [[nodiscard]] auto stats() const -> CacheStats {
        std::shared_lock lock(m_mutex);
        CacheStats s = m_stats;
        s.current_size = m_cache.size();
        s.max_size = m_max_entries;
        s.memory_bytes = m_current_memory;
        return s;
    }

    /**
     * @brief Remove expired entries from the cache
     * @return std::size_t Number of entries removed
     */
    auto prune_expired() -> std::size_t {
        std::unique_lock lock(m_mutex);

        std::size_t removed = 0;
        auto now = std::chrono::steady_clock::now();

        for (auto it = m_cache.begin(); it != m_cache.end();) {
            if (is_expired(it->second.meta, now)) {
                m_current_memory -= it->second.meta.estimated_size;
                it = m_cache.erase(it);
                removed++;
                m_stats.evictions++;
            } else {
                ++it;
            }
        }

        return removed;
    }

   private:
    struct CacheEntry {
        std::any value;
        CacheEntryMeta meta;
    };

    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, CacheEntry> m_cache;

    std::size_t m_max_entries = 10000;
    std::size_t m_max_memory_bytes = 100 * 1024 * 1024;  // 100 MB default
    std::chrono::seconds m_default_ttl{0};  // 0 = no expiration
    std::size_t m_current_memory = 0;

    mutable CacheStats m_stats;

    /**
     * @brief Check if an entry is expired
     */
    [[nodiscard]] auto is_expired(const CacheEntryMeta& meta,
                                  std::chrono::steady_clock::time_point now =
                                      std::chrono::steady_clock::now()) const -> bool {
        if (m_default_ttl.count() == 0) {
            return false;
        }
        return (now - meta.created) > m_default_ttl;
    }

    /**
     * @brief Evict entries if cache is full
     *
     * Uses LRU (Least Recently Used) policy for eviction.
     */
    auto evict_if_needed(std::size_t new_entry_size) -> void {
        // Check entry count limit
        while (m_max_entries > 0 && m_cache.size() >= m_max_entries) {
            evict_lru();
        }

        // Check memory limit
        while (m_max_memory_bytes > 0 &&
               m_current_memory + new_entry_size > m_max_memory_bytes &&
               !m_cache.empty()) {
            evict_lru();
        }
    }

    /**
     * @brief Evict the least recently used entry
     */
    auto evict_lru() -> void {
        if (m_cache.empty()) return;

        // Find LRU entry
        auto lru_it = m_cache.begin();
        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            if (it->second.meta.last_accessed < lru_it->second.meta.last_accessed) {
                lru_it = it;
            }
        }

        m_current_memory -= lru_it->second.meta.estimated_size;
        m_cache.erase(lru_it);
        m_stats.evictions++;
    }
};

}  // namespace clas12::framework
