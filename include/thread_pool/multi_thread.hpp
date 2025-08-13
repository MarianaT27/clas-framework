#pragma once
// C++ headers
#include <chrono>
#include <string>
#include <vector>
#include <atomic>
#include <thread>
// <fmt> headers
#include <fmt/core.h>
#include <fmt/format.h>
// Taskflow headers
#include <taskflow/taskflow.hpp>
// Progress bar library
#include <thread_pool/indicators.hpp>

/**
 * @brief Process multiple files in parallel using a reader function with Taskflow
 * @tparam Reader Type of the reader function
 * @param reader Function to read/process a single file
 * @param files Vector of file paths to process
 * @param num_threads Number of threads to use (default: 6)
 * @param update_interval Progress update interval in milliseconds (default: 100ms)
 */
    template <typename Reader>
auto multithread_reader(
    Reader& reader,
    const std::vector<std::string>& files,
    unsigned int num_threads = 6,
    unsigned int update_interval = 100) -> void {
    
    if (files.empty()) {
        fmt::print("No files to process.\n");
        return;
    }
    
    num_threads = std::max(1u, num_threads); // Ensure at least one thread
    
    // Use atomic counters for thread-safe progress tracking
    std::atomic<int> completed_tasks{0};
    std::atomic<int> failed_tasks{0};
    const int total_tasks = static_cast<int>(files.size()); // Use actual file count
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create Taskflow executor with specified number of threads
    tf::Executor executor(num_threads);
    tf::Taskflow taskflow;
    
    // indicators progress bar
    using namespace indicators;
    ProgressBar bar{
        option::BarWidth{50},
        option::Start{"["},
        option::Fill{"█"},
        option::Lead{"█"},
        option::Remainder{"-"},
        option::End{"]"},
        option::PostfixText{"Initializing..."},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
        option::MaxProgress{total_tasks}
    };
    
    fmt::print("Starting processing of {} files using {} threads...\n", total_tasks, num_threads);
    
    // Show initial progress bar at 0
    bar.set_progress(0);
    bar.set_option(option::PostfixText{
        fmt::format("(0/{}) Ready to start...", total_tasks)
    });
    
    // Create tasks for all files
    std::vector<tf::Task> file_tasks;
    file_tasks.reserve(files.size());
    
    for (const auto& file : files) {
        auto task = taskflow.emplace([&reader, file, &completed_tasks, &failed_tasks] {
            try {
                reader(file);
                completed_tasks.fetch_add(1, std::memory_order_relaxed);
            } catch (const std::exception& e) {
                fmt::print(stderr, "Error processing file '{}': {}\n", file, e.what());
                failed_tasks.fetch_add(1, std::memory_order_relaxed);
                completed_tasks.fetch_add(1, std::memory_order_relaxed); // Count as completed (even if failed)
            }
        });
        file_tasks.push_back(task);
    }
    
    // Run the taskflow asynchronously
    auto future = executor.run(taskflow);
    
    // Wait for all tasks to complete with progress updates
    int last_completed = 0;
    while (completed_tasks.load(std::memory_order_relaxed) < total_tasks) {
        int current_completed = completed_tasks.load(std::memory_order_relaxed);
        if (current_completed > last_completed) {
            bar.set_progress(current_completed);
            int current_failed = failed_tasks.load(std::memory_order_relaxed);
            bar.set_option(option::PostfixText{
                fmt::format("({}/{}) Success: {} Failed: {}", 
                           current_completed, total_tasks, 
                           current_completed - current_failed, current_failed)
            });
            
            last_completed = current_completed;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(update_interval));
    }
    
    future.wait();
    
    bar.set_progress(total_tasks);
    bar.set_option(option::PostfixText{"Complete!"});
    bar.mark_as_completed();
        
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    int successful_tasks = total_tasks - failed_tasks.load();
    
    fmt::print("\n");
    fmt::print("Processing completed!\n");
    fmt::print("Total files: {}\n", total_tasks);
    fmt::print("Successful: {}\n", successful_tasks);
    fmt::print("Failed: {}\n", failed_tasks.load());
    fmt::print("Time elapsed: {:.2f} seconds\n", duration.count() / 1000.0);
    fmt::print("Average time per file: {:.2f} ms\n", total_tasks > 0 ? static_cast<double>(duration.count()) / total_tasks : 0.0);
    if (total_tasks > 0) fmt::print("Throughput: {:.2f} files/second\n", static_cast<double>(total_tasks) / (duration.count() / 1000.0));
}