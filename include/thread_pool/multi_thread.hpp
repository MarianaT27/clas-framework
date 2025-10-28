// C++ headers
#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
// <fmt> headers
#include <fmt/core.h>
#include <fmt/format.h>
#include "thread_pool/ProgressTracker.hpp"
#include "thread_pool/ThreadPool.hpp"

/**
 * @brief Process multiple files in parallel using a reader function with ThreadPool
 * @tparam Reader Type of the reader function
 * @param reader Function to read/process a single file
 * @param files Vector of file paths to process
 * @param num_threads Number of threads to use (default: 6)
 * @param update_interval Progress update interval in milliseconds (default: 1000ms)
 */
template <typename Reader>
auto multithread_reader(Reader& reader, const std::vector<std::string>& files, unsigned int num_threads = 6, unsigned int update_interval = 1000) -> void {
    if (files.empty()) {
        fmt::print("No files to process.\n");
        return;
    }

    num_threads = std::max(1u, num_threads);
    fmt::print("System info: {} hardware threads, using {} worker threads\n", std::thread::hardware_concurrency(), num_threads);

    // Create thread pool
    ThreadPool pool(num_threads);

    // Configure progress tracker with custom update interval
    ProgressTracker::Config config;
    config.update_interval = std::chrono::milliseconds(update_interval);

    ProgressTracker progress(files.size(), config);

    std::vector<std::future<void>> futures;
    futures.reserve(files.size());
    progress.start();

    // Submit all tasks
    for (const auto& file : files) {
        futures.emplace_back(pool.submit([&file, &progress, &reader]() -> void { reader(file); progress.increment(); return; }));
    }

    // Wait for all tasks to complete
    for (auto& future : futures) {
        try {
            future.get();
        } catch (const std::exception& e) {
            fmt::print("Error processing file: {}\n", e.what());
        }
    }

    progress.finish();
    pool.shutdown(std::chrono::seconds{2});

    const auto& stats = pool.get_statistics();
    fmt::print("\nStatistics:\n");
    fmt::print("  - Tasks submitted: {}\n", stats.tasks_submitted.load());
    fmt::print("  - Tasks completed: {}\n", stats.tasks_completed.load());
    fmt::print("  - Tasks failed: {}\n", stats.tasks_failed.load());
    fmt::print("  - Throughput: {:.2f} tasks/sec\n", stats.get_throughput());
    fmt::print("  - Uptime: {}\n", ProgressTracker::format_duration(std::chrono::duration_cast<std::chrono::seconds>(stats.get_uptime())));
    fmt::print("All files processed.\n");
}