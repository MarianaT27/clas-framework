#pragma once

#include <fmt/format.h>
#include <algorithm>
#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include "Framework/IService.hpp"

namespace Framework {

/**
 * @brief Statistics for algorithm execution
 */
struct AlgorithmStats {
    std::string name;
    size_t call_count = 0;
    double total_time_ms = 0.0;
    double min_time_ms = std::numeric_limits<double>::max();
    double max_time_ms = 0.0;
    double avg_time_ms = 0.0;

    auto update(double time_ms) -> void {
        call_count++;
        total_time_ms += time_ms;
        min_time_ms = std::min(min_time_ms, time_ms);
        max_time_ms = std::max(max_time_ms, time_ms);
        avg_time_ms = total_time_ms / call_count;
    }
};

/**
 * @brief Service for profiling algorithm performance
 *
 * Automatically tracks:
 * - Execution time per algorithm
 * - Total time, min, max, average
 * - Call counts
 * - Performance bottlenecks
 *
 * Usage:
 * @code
 * auto profiling = service<ProfilingService>("ProfilingService");
 * auto timer = profiling->start_timer("MyAlgorithm");
 * // ... algorithm execution ...
 * profiling->stop_timer(timer);
 * @endcode
 */
class ProfilingService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "ProfilingService"; }

    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using Timer = std::pair<std::string, TimePoint>;

    explicit ProfilingService(std::string name = "ProfilingService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        m_start_time = std::chrono::high_resolution_clock::now();
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        m_end_time = std::chrono::high_resolution_clock::now();
        print_summary();
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Start timing an algorithm
     */
    auto start_timer(const std::string& algorithm_name) -> Timer {
        return {algorithm_name, std::chrono::high_resolution_clock::now()};
    }

    /**
     * @brief Stop timer and record statistics
     */
    auto stop_timer(const Timer& timer) -> void {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - timer.second);
        double time_ms = duration.count() / 1000.0;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats[timer.first].name = timer.first;
        m_stats[timer.first].update(time_ms);
    }

    /**
     * @brief Get statistics for an algorithm
     */
    auto get_stats(const std::string& algorithm_name) const -> const AlgorithmStats* {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_stats.find(algorithm_name);
        return (it != m_stats.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Get all statistics
     */
    auto get_all_stats() const -> std::map<std::string, AlgorithmStats> {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stats;
    }

    /**
     * @brief Print performance summary
     */
    auto print_summary() const -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_stats.empty()) {
            Framework::Log::info("ProfilingService", "No profiling data available");
            return;
        }

        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            m_end_time - m_start_time);

        fmt::print("\n");
        fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
        fmt::print("║              Algorithm Performance Summary                     ║\n");
        fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
        fmt::print("\n");
        fmt::print("Total execution time: {:.2f} seconds\n", total_duration.count() / 1000.0);
        fmt::print("\n");

        // Sort by total time (descending)
        std::vector<AlgorithmStats> sorted_stats;
        for (const auto& [name, stats] : m_stats) {
            sorted_stats.push_back(stats);
        }
        std::sort(sorted_stats.begin(), sorted_stats.end(),
                  [](const AlgorithmStats& a, const AlgorithmStats& b) {
                      return a.total_time_ms > b.total_time_ms;
                  });

        // Print header
        fmt::print("{:<30} {:>10} {:>12} {:>10} {:>10} {:>10} {:>8}\n",
                   "Algorithm", "Calls", "Total (ms)", "Avg (ms)", "Min (ms)", "Max (ms)", "% Time");
        fmt::print("{:-<30} {:->10} {:->12} {:->10} {:->10} {:->10} {:->8}\n",
                   "", "", "", "", "", "", "");

        // Calculate total time for percentage
        double total_alg_time = 0.0;
        for (const auto& stats : sorted_stats) {
            total_alg_time += stats.total_time_ms;
        }

        // Print statistics
        for (const auto& stats : sorted_stats) {
            double percent = (total_alg_time > 0.0) ? (stats.total_time_ms / total_alg_time * 100.0) : 0.0;

            fmt::print("{:<30} {:>10} {:>12.2f} {:>10.3f} {:>10.3f} {:>10.3f} {:>7.1f}%\n",
                       stats.name,
                       stats.call_count,
                       stats.total_time_ms,
                       stats.avg_time_ms,
                       stats.min_time_ms,
                       stats.max_time_ms,
                       percent);
        }

        fmt::print("\n");

        // Identify bottlenecks (>20% of total time)
        fmt::print("Performance Analysis:\n");
        bool has_bottleneck = false;
        for (const auto& stats : sorted_stats) {
            double percent = (total_alg_time > 0.0) ? (stats.total_time_ms / total_alg_time * 100.0) : 0.0;
            if (percent > 20.0) {
                fmt::print("  ⚠️  {} is a bottleneck ({:.1f}% of total time)\n",
                           stats.name, percent);
                has_bottleneck = true;
            }
        }

        if (!has_bottleneck) {
            fmt::print("  ✓ No significant bottlenecks detected\n");
        }

        // Throughput
        size_t total_calls = 0;
        for (const auto& stats : sorted_stats) {
            total_calls += stats.call_count;
        }
        if (total_duration.count() > 0) {
            double events_per_sec = (total_calls > 0) ? (1000.0 / (total_alg_time / total_calls)) : 0.0;
            fmt::print("  ℹ️  Average throughput: {:.1f} events/second\n", events_per_sec);
        }

        fmt::print("\n");
    }

    /**
     * @brief Reset all statistics
     */
    auto reset() -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats.clear();
    }

   private:
    mutable std::mutex m_mutex;
    std::map<std::string, AlgorithmStats> m_stats;
    TimePoint m_start_time;
    TimePoint m_end_time;
};

/**
 * @brief RAII timer for automatic profiling
 *
 * Usage:
 * @code
 * {
 *     AutoTimer timer(profiling_service, "MyAlgorithm");
 *     // ... code to profile ...
 * } // Automatically stops timer on scope exit
 * @endcode
 */
class AutoTimer {
   public:
    AutoTimer(ProfilingService* service, const std::string& name)
        : m_service(service), m_timer(service ? service->start_timer(name) : ProfilingService::Timer{name, {}}) {}

    ~AutoTimer() {
        if (m_service) {
            m_service->stop_timer(m_timer);
        }
    }

    // Non-copyable, non-movable
    AutoTimer(const AutoTimer&) = delete;
    AutoTimer& operator=(const AutoTimer&) = delete;
    AutoTimer(AutoTimer&&) = delete;
    AutoTimer& operator=(AutoTimer&&) = delete;

   private:
    ProfilingService* m_service;
    ProfilingService::Timer m_timer;
};

}  // namespace Framework
