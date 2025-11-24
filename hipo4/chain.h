//******************************************************************************
//*       ██╗  ██╗██╗██████╗  ██████╗     ██╗  ██╗    ██████╗                  *
//*       ██║  ██║██║██╔══██╗██╔═══██╗    ██║  ██║   ██╔═████╗                 *
//*       ███████║██║██████╔╝██║   ██║    ███████║   ██║██╔██║                 *
//*       ██╔══██║██║██╔═══╝ ██║   ██║    ╚════██║   ████╔╝██║                 *
//*       ██║  ██║██║██║     ╚██████╔╝         ██║██╗╚██████╔╝                 *
//*       ╚═╝  ╚═╝╚═╝╚═╝      ╚═════╝          ╚═╝╚═╝ ╚═════╝                  *
//************************ Jefferson National Lab (2017) ***********************
/*
 * chain - Chain multiple HIPO files for parallel processing
 *
 */

#pragma once

// C++ headers
#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <regex>

// <fmt> headers
#include "fmt/base.h"

// Project headers
#include "hipoeventiterator.h"
#include "threadpool.hpp"
#include "progresstracker.hpp"

namespace hipo {

class chain {
public:
    struct fileinfo {
        std::string filename;
        int file_index;
        long total_events;      // Will be -1 until actually queried
        int num_records;        // Will be -1 until actually queried

        fileinfo(const std::string& name, int idx)
            : filename(name), file_index(idx), total_events(-1), num_records(-1) {}
    };

    struct chainstatistics {
        std::atomic<long> total_events{0};
        std::atomic<long> events_processed{0};
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        void reset() {
            total_events = 0;
            events_processed = 0;
            start_time = std::chrono::steady_clock::now();
        }

        double get_throughput() const {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                end_time - start_time).count();
            return elapsed > 0 ? static_cast<double>(events_processed) / elapsed : 0.0;
        }
    };

private:
    std::vector<fileinfo> files;
    int num_threads;
    bool show_progress;
    bool verbose;
    std::vector<long> tags;
    ThreadPool thread_pool;
    chainstatistics stats;
    std::mutex chain_mutex;
    bool metadata_loaded;

    /**
     * @brief Get event count for a file (lazy loading)
     */
    long get_file_event_count(int file_idx) {
        if (files[file_idx].total_events < 0) {
            // Need to load metadata - open file briefly
            try {
                hipo::hipoeventfile temp(files[file_idx].filename, tags, 0);
                files[file_idx].total_events = temp.entries();
                files[file_idx].num_records = temp.nrecords();
            } catch (const std::exception& e) {
                fmt::print(stderr, "[chain] Error reading file {}: {}\n", files[file_idx].filename, e.what());
                files[file_idx].total_events = 0;
                files[file_idx].num_records = 0;
            }
        }
        return files[file_idx].total_events;
    }

public:
    explicit chain(int threads = 0, bool progress = true, bool verb = false)
        : num_threads(threads == 0 ? std::thread::hardware_concurrency() : threads),
          show_progress(progress),
          verbose(verb),
          thread_pool(num_threads, "ChainWorker"),
          metadata_loaded(false) {
        if (verbose) fmt::print("[chain] Initialized with {} threads (zero-overhead mode)\n", num_threads);
    }

    int add(const std::string& filename) {
        if (!std::filesystem::exists(filename)) {
            fmt::print(stderr, "[chain] Warning: File not found: {}\n", filename);
            return files.size();
        }
        std::lock_guard<std::mutex> lock(chain_mutex);
        files.emplace_back(filename, files.size());
        metadata_loaded = false;  // Invalidate metadata cache
        if (verbose) fmt::print("[chain] Added file {} : {}\n", files.size(), filename);
        return files.size();
    }

    int add(const std::vector<std::string>& filenames) {
        for (const auto& filename : filenames) {
            add(filename);
        }
        return files.size();
    }

    int add_pattern(const std::string& pattern) {
        int added = 0;
        std::filesystem::path pattern_path(pattern);
        std::filesystem::path dir = pattern_path.parent_path();
        if (dir.empty()) dir = ".";

        std::string filename_pattern = pattern_path.filename().string();
        std::string regex_pattern = filename_pattern;
        size_t pos = 0;
        while ((pos = regex_pattern.find("*", pos)) != std::string::npos) {
            regex_pattern.replace(pos, 1, ".*");
            pos += 2;
        }
        pos = 0;
        while ((pos = regex_pattern.find("?", pos)) != std::string::npos) {
            regex_pattern.replace(pos, 1, ".");
            pos += 1;
        }

        std::regex file_regex(regex_pattern);

        try {
            for (const auto& entry : std::filesystem::directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    std::string fname = entry.path().filename().string();
                    if (std::regex_match(fname, file_regex)) {
                        add(entry.path().string());
                        added++;
                    }
                }
            }
        } catch (const std::exception& e) {
            fmt::print(stderr, "[chain] Error scanning directory: {}\n", e.what());
        }

        if (verbose) fmt::print("[chain] Pattern '{}' matched {} files\n", pattern, added);
        return added;
    }

    void scan() {
        if (files.empty()) {
            fmt::print("[chain] No files in chain\n");
            return;
        }

        std::unique_ptr<ProgressTracker> progress;
        if (show_progress) {
            ProgressTracker::Config config;
            config.label = "Scanning files";
            config.show_eta = false;
            progress = std::make_unique<ProgressTracker>(files.size(), config);
            progress->start();
        }

        long total_events = 0;
        int total_records = 0;
        long total_size = 0;
        int valid_files = 0;

        for (auto& file_info : files) {
            try {
                if (!std::filesystem::exists(file_info.filename)) {
                    fmt::print("  [{}] {} - FILE NOT FOUND\n", file_info.file_index, file_info.filename);
                    continue;
                }

                uintmax_t file_size = std::filesystem::file_size(file_info.filename);
                total_size += file_size;

                // Open file briefly just to get metadata
                hipo::hipoeventfile temp_reader(file_info.filename, tags, 0);
                int records = temp_reader.nrecords();
                long events = temp_reader.entries();
                file_info.total_events = events;
                file_info.num_records = records;
                total_records += records;
                total_events += events;
                valid_files++;
                // temp_reader closes here

                std::string size_str;
                if (file_size > 1024*1024*1024) { size_str = fmt::format("{:.1f} GB", file_size / (1024.0*1024*1024)); }
                else if (file_size > 1024*1024) { size_str = fmt::format("{:.1f} MB", file_size / (1024.0*1024)); }
                else if (file_size > 1024) { size_str = fmt::format("{:.1f} KB", file_size / 1024.0); }
                else { size_str = fmt::format("{} B", file_size); }

                fmt::print("  [{}] {}\n      Records: {}, Events: {}, Size: {}\n",
                          file_info.file_index, file_info.filename, records, events, size_str);

            } catch (const std::exception& e) {
                fmt::print("  [{}] {} - ERROR: {}\n", file_info.file_index, file_info.filename, e.what());
            }

            if (progress) progress->increment();
        }

        if (progress) progress->finish();

        fmt::print("\nScan Summary\n");
        fmt::print("Total files:         {}\n", files.size());
        fmt::print("Valid files:         {}\n", valid_files);
        fmt::print("Total records:       {}\n", total_records);
        fmt::print("Total events:        {}\n", total_events);

        std::string total_size_str;
        if (total_size > 1024*1024*1024) { total_size_str = fmt::format("{:.1f} GB", total_size / (1024.0*1024*1024)); }
        else if (total_size > 1024*1024) { total_size_str = fmt::format("{:.1f} MB", total_size / (1024.0*1024)); }
        else { total_size_str = fmt::format("{:.1f} KB", total_size / 1024.0); }
        fmt::print("Total size:          {}\n", total_size_str);
        if (valid_files > 0) fmt::print("Average events/file: {}\n", total_events / valid_files);
    }

    int get_nb_files() const { return files.size(); }

    const fileinfo& file_info(int index) const {
        if (index < 0 || index >= files.size()) throw std::out_of_range("chain: File index out of range");
        return files[index];
    }

    void set_tags(const std::vector<long>& t) { tags = t; }

    /**
     * @brief "Open" - but doesn't actually open any files!
     * Just validates they exist and prepares for processing.
     *
     * ZERO OVERHEAD: No files are opened. No metadata is loaded.
     * Everything happens lazily during process().
     */
    void open() {
        if (files.empty()) throw std::runtime_error("chain: No files added to chain");

        if (verbose) {
            fmt::print("[chain] Preparing {} files (zero-overhead mode)\n", files.size());
            fmt::print("[chain] No files opened yet - metadata will be loaded on-demand\n");
        }

        // Just validate files exist
        int missing = 0;
        for (const auto& file_info : files) {
            if (!std::filesystem::exists(file_info.filename)) {
                fmt::print(stderr, "[chain] Warning: File not found: {}\n", file_info.filename);
                missing++;
            }
        }

        if (missing > 0) fmt::print(stderr, "[chain] Warning: {} files not found\n", missing);

        if (verbose) {
            fmt::print("[chain] Chain ready:\n");
            fmt::print("  Files:         {}\n", files.size());
            fmt::print("  Memory used:   ~{} bytes (just filenames)\n", files.size() * 300);  // Rough estimate
        }
    }

    /**
     * @brief Process events with ABSOLUTE minimal memory
     *
     * Files are opened ONE AT A TIME, only when needed.
     * Metadata is loaded lazily as required.
     */
    template<typename ProcessFunc>
    void process(ProcessFunc&& process_func, double percentage = 100.0) {
        if (files.empty()) {
            fmt::print(stderr, "[chain] Warning: No files to process.\n");
            return;
        }

        percentage = std::max(0.0, std::min(100.0, percentage)) / 100.0;

        if (verbose) fmt::print("[chain] Loading file metadata...\n");
        long total_events_available = 0;
        for (size_t i = 0; i < files.size(); i++) {
            total_events_available += get_file_event_count(i);
        }

        long target_events = static_cast<long>(total_events_available * percentage);

        // Build work list
        struct work_item {
            int file_idx;
            long events_to_process;
        };
        std::vector<work_item> work_list;

        long events_accumulated = 0;
        for (size_t i = 0; i < files.size() && events_accumulated < target_events; i++) {
            long events_needed = target_events - events_accumulated;
            long events_from_this_file = std::min(events_needed, files[i].total_events);

            if (events_from_this_file > 0) {
                work_list.push_back({static_cast<int>(i), events_from_this_file});
                events_accumulated += events_from_this_file;
            }
        }

        stats.reset();
        stats.total_events = events_accumulated;

        if (verbose) {
            fmt::print("[chain] Processing {:.1f}% : {} events from {} files\n", percentage * 100.0, events_accumulated, work_list.size());
            fmt::print("[chain] Memory strategy: 1 file open at a time, {} threads\n", num_threads);
        }

        std::unique_ptr<ProgressTracker> progress;
        if (show_progress && stats.total_events > 0) {
            ProgressTracker::Config config;
            config.label = fmt::format("Processing ({:.1f}%)", percentage * 100.0);
            config.show_eta = true;
            config.show_rate = true;
            config.use_colors = true;
            progress = std::make_unique<ProgressTracker>(stats.total_events, config);
            progress->start();
        }

        // Process ONE FILE AT A TIME
        for (const auto& work : work_list) {
            const auto& file_info = files[work.file_idx];

            if (verbose) {
                fmt::print("[chain] Processing file {}/{}: {} ({} events)\n",
                          work.file_idx + 1, work_list.size(), file_info.filename, work.events_to_process);
            }

            // All threads work on THIS file together
            std::atomic<long> next_event{0};
            std::vector<std::future<void>> futures;

            for (int t = 0; t < num_threads; t++) {
                futures.push_back(thread_pool.submit([&]() {
                    try {
                        // Each thread opens its own reader
                        hipo::hipoeventfile reader(file_info.filename, tags, 0);

                        while (true) {
                            long evt_idx = next_event.fetch_add(1);
                            if (evt_idx >= work.events_to_process) break;

                            if (reader.goto_event(evt_idx)) {
                                hipo::event temp_event;
                                if (reader.read_next_event(temp_event)) {
                                    iter_event evt(&temp_event, &reader);
                                    process_func(evt, file_info.file_index, evt_idx);
                                }
                            }

                            stats.events_processed++;
                            if (progress) progress->increment();
                        }

                    } catch (const std::exception& e) {
                        fmt::print(stderr, "[chain] Thread error on file {}: {}\n",
                                  file_info.filename, e.what());
                    }
                }));
            }

            // Wait for all threads to finish THIS file
            for (auto& future : futures) {
                future.get();
            }
        }

        stats.end_time = std::chrono::steady_clock::now();
        if (progress) progress->finish();
        if (verbose) print_statistics();
    }

    void print_statistics() const {
        fmt::print("\nChain Statistics\n");
        fmt::print("Files:             {}\n", files.size());
        fmt::print("Total events:      {}\n", stats.total_events.load());
        fmt::print("Events processed:  {}\n", stats.events_processed.load());
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            stats.end_time - stats.start_time);
        fmt::print("Processing time:   {} seconds\n", duration.count());
        if (duration.count() > 0) {
            fmt::print("Throughput:        {:.1f} events/sec\n", stats.get_throughput());
        }
    }

    void list() const {
        fmt::print("Chain file list\n");
        fmt::print("Total files: {}\n", files.size());
        for (const auto& file : files) {
            fmt::print("  [{}] {}", file.file_index, file.filename);
            if (file.total_events > 0) {
                fmt::print(" ({} events, {} records)", file.total_events, file.num_records);
            } else {
                fmt::print(" (metadata not loaded)");
            }
            fmt::print("\n");
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(chain_mutex);
        files.clear();
        stats.reset();
        metadata_loaded = false;
        if (verbose) fmt::print("[chain] Cleared all files\n");
    }

    const chainstatistics& statistics() const { return stats; }
    ThreadPool& threadpool() { return thread_pool; }
    long total_events_count() const { return stats.total_events.load(); }
};

} // namespace hipo
