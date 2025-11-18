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
 * Similar to ROOT's TChain, this class allows processing multiple HIPO files
 * as a single logical dataset. All records from all files are processed
 * together in parallel.
 *
 * Features:
 * - Add files individually or by wildcard pattern
 * - Process ALL records from ALL files in parallel
 * - ThreadPool and ProgressTracker integration
 * - Automatic file management
 * - Unified statistics across all files
 */

#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include "fmt/base.h"
#include "hipoeventiterator.h"
#include "threadpool.hpp"
#include "progresstracker.hpp"

namespace hipo {

/**
 * @brief Chain multiple HIPO files for unified parallel processing
 *
 * This class chains multiple HIPO files and processes ALL records from
 * ALL files together in parallel (not file-by-file). This maximizes
 * parallelism and throughput.
 *
 * Usage:
 *   chain chain(4);  // 4 threads
 *   chain.Add("file1.hipo");
 *   chain.Add("file2.hipo");
 *   chain.AddPattern("data_*.hipo");  // Wildcard support
 *
 *   chain.Process([&](auto& evt, int file_idx, int rec_idx, int evt_idx) {
 *       auto particles = evt.get_bank_map("REC::Particle");
 *       // Process...
 *   });
 */
class chain {
public:
    struct fileinfo {
        std::string filename;
        int file_index;
        long total_events;
        std::unique_ptr<hipo::hipoeventfile> eventfile;

        fileinfo(const std::string& name, int idx)
            : filename(name), file_index(idx), total_events(0), eventfile(nullptr) {}
    };

    struct eventlocation {
        int file_index;
        long event_index_in_file;

        eventlocation(int f, long e) : file_index(f), event_index_in_file(e) {}
    };

    struct recordlocation {
        int file_index;
        int record_index;
        long first_event_in_file; // Event index in file where this record starts
        long num_events;          // Number of events in this record

        recordlocation(int f, int r, long first_in_file, long n)
            : file_index(f), record_index(r), first_event_in_file(first_in_file), num_events(n) {}
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
    std::vector<eventlocation> all_events;  
    std::vector<recordlocation> all_records;
    int num_threads;
    bool show_progress;
    bool verbose;
    std::vector<long> tags;
    ThreadPool thread_pool;
    chainstatistics stats;
    std::mutex chain_mutex;

public:
    /**
     * @brief Constructor
     * @param threads Number of threads (0 = auto-detect)
     * @param progress Show progress bar
     * @param verb Enable verbose output
     */
    explicit chain(int threads = 0, bool progress = true, bool verb = false)
        : num_threads(threads == 0 ? std::thread::hardware_concurrency() : threads),
          show_progress(progress),
          verbose(verb),
          thread_pool(num_threads, "ChainWorker") {
        if (verbose) fmt::print("[chain] Initialized with {} threads\n", num_threads);
    }

    /**
     * @brief Add a single file to the chain
     * @param filename Path to HIPO file
     * @return Number of files in chain
     */
    int add(const std::string& filename) {
        if (!std::filesystem::exists(filename)) {
            fmt::print(stderr, "[chain] Warning: File not found: {}\n", filename);
            return files.size();
        }
        std::lock_guard<std::mutex> lock(chain_mutex);
        files.emplace_back(filename, files.size());
        if (verbose) fmt::print("[chain] Added file {} : {}\n", files.size(), filename);        
        return files.size();
    }

    /**
    * @brief Add multiple files from a list
     * @param filenames List of file paths
     * @return Number of files in chain
     */
    int add(const std::vector<std::string>& filenames) {
        for (const auto& filename : filenames) {
            add(filename);
        }
        return files.size();
    }

    /**
     * @brief Add multiple files using wildcard pattern
     * @param pattern Wildcard pattern (e.g., "data_*.hipo")
     * @return Number of files added
     */
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

    /**
     * @brief Scan and list all files in the chain with detailed statistics
     */
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

                hipo::hipoeventfile temp_reader(file_info.filename, tags, verbose ? 1 : 0);
                int records = temp_reader.nrecords();
                long events = temp_reader.entries();
                file_info.total_events = events;
                total_records += records;
                total_events += events;
                valid_files++;

                std::string size_str;
                if (file_size > 1024*1024*1024) { size_str = fmt::format("{} GB", file_size / (1024*1024*1024)); }
                else if (file_size > 1024*1024) { size_str = fmt::format("{} MB", file_size / (1024*1024)); }
                else if (file_size > 1024) { size_str = fmt::format("{} KB", file_size / 1024); }
                else { size_str = fmt::format("{} B", file_size); }

                fmt::print("  [{}] {}\n      Records: {}, Events: {}, Size: {}\n", file_info.file_index, file_info.filename, records, events, size_str);

            } catch (const std::exception& e) {
                fmt::print("  [{}] {} - ERROR: {}\n", file_info.file_index, file_info.filename, e.what());
            }

            if (progress) progress->increment();
        }

        if (progress) progress->finish();

        if (verbose) {
            fmt::print("Scan Summary\n");
            fmt::print("Total files:       {}\n", files.size());
            fmt::print("Valid files:       {}\n", valid_files);
            fmt::print("Total records:     {}\n", total_records);
            fmt::print("Total events:      {}\n", total_events);
            
            std::string total_size_str;
            if (total_size > 1024*1024*1024) { total_size_str = fmt::format("{} GB", total_size / (1024*1024*1024)); }
            else if (total_size > 1024*1024) { total_size_str = fmt::format("{} MB", total_size / (1024*1024)); }
            else { total_size_str = fmt::format("{} KB", total_size / 1024); }
            fmt::print("Total size:        {}\n", total_size_str);            
            if (valid_files > 0) fmt::print("Average events/file: {}\n", total_events / valid_files);
        }
    }

    /**
     * @brief Get number of files in chain
     */
    int get_nb_files() const { return files.size(); }

    /**
     * @brief Get file info by index
     */
    const fileinfo& file_info(int index) const {
        if (index < 0 || index >= files.size()) throw std::out_of_range("chain: File index out of range");
        return files[index];
    }

    /**
     * @brief Set tags for filtering
     */
    void set_tags(const std::vector<long>& t) { tags = t; }

    /**
     * @brief Open all files and build the event index
     * This MUST be called before process()
     */
    void open() {
        if (files.empty()) throw std::runtime_error("chain: No files added to chain");
        if (verbose) std::cout << "[chain] Opening " << files.size() << " files..." << std::endl;

        std::unique_ptr<ProgressTracker> progress;
        if (show_progress) {
            ProgressTracker::Config config;
            config.label = "Opening files";
            config.show_eta = false;
            progress = std::make_unique<ProgressTracker>(files.size(), config);
            progress->start();
        }

        all_events.clear();
        all_records.clear();
        stats.total_events = 0;

        for (auto& file_info : files) {
            try {
                file_info.eventfile = std::make_unique<hipo::hipoeventfile>(file_info.filename, tags, verbose ? 1 : 0);
                file_info.total_events = file_info.eventfile->entries();
                int num_records = file_info.eventfile->nrecords();
                stats.total_events += file_info.total_events;

                long events_per_record = (file_info.total_events + num_records - 1) / num_records;
                long event_offset_in_file = 0;

                for (int r = 0; r < num_records; r++) {
                    long events_in_record = std::min(events_per_record, file_info.total_events - event_offset_in_file);
                    if (events_in_record > 0) {
                        all_records.emplace_back(file_info.file_index, r, event_offset_in_file, events_in_record);
                        event_offset_in_file += events_in_record;
                    }
                }

                for (long e = 0; e < file_info.total_events; e++) {
                    all_events.emplace_back(file_info.file_index, e);
                }

                if (verbose) fmt::print("[chain] File {} : {} events in {} records\n", file_info.file_index, file_info.total_events, num_records);
            } catch (const std::exception& e) {
                fmt::print(stderr, "[chain] Error opening {} : {}\n", file_info.filename, e.what());
            }

            if (progress) progress->increment();
        }
        if (progress) progress->finish();
        if (verbose) {
            fmt::print("[chain] Chain ready:\n");
            fmt::print("  Files:         {}\n", files.size());
            fmt::print("  Total events:  {}\n", stats.total_events.load());
        }
    }

    /**
     * @brief Process ALL events from ALL files in parallel
     *
     * This is the main processing method. It processes all events from all
     * files together in parallel, maximizing throughput.
     *
     * Note: Since hipoeventfile is not thread-safe, each thread creates
     * its own hipoeventfile instance for each file.
     *
     * @param process_func Function with signature:
     *        void(iter_event&, int file_idx, long event_idx_in_file)
     * @param percentage Percentage of data to process (0.0 to 100.0), default 100.0
     */
    template<typename ProcessFunc>
    void process(ProcessFunc&& process_func, double percentage = 100.0) {
        if (all_events.empty()) {
            fmt::print(stderr, "[chain] Warning: Chain not opened. Call open() first.\n");
            return;
        }

        // Clamp percentage to valid range
        percentage = std::max(0.0, std::min(100.0, percentage));
        percentage /= 100.0;
        
        // Calculate how many records to process based on percentage
        size_t total_records = all_records.size();
        size_t records_to_process = static_cast<size_t>(total_records * percentage);
        
        // Calculate total events that will be processed
        long total_events_to_process = 0;
        for (size_t i = 0; i < records_to_process; i++) {
            total_events_to_process += all_records[i].num_events;
        }

        stats.reset();
        stats.total_events = total_events_to_process;

        if (verbose) {
            fmt::print("[chain] Processing {:.1f}% of data: {} events from {} records (out of {} total records)...\n",
                      percentage * 100.0, total_events_to_process, records_to_process, total_records);
        }
        
        std::unique_ptr<ProgressTracker> progress;
        if (show_progress && stats.total_events > 0) {
            ProgressTracker::Config config;
            config.label = fmt::format("Processing chain ({:.1f}%)", percentage * 100.0);
            config.show_eta = true;
            config.show_rate = true;
            config.use_colors = true;
            progress = std::make_unique<ProgressTracker>(stats.total_events, config);
            progress->start();
        }

        std::atomic<size_t> next_record{0};
        std::vector<std::future<void>> futures;
        for (int t = 0; t < num_threads; t++) {
            futures.push_back(thread_pool.submit([&]() {
                std::unordered_map<int, std::unique_ptr<hipo::hipoeventfile>> thread_files;
                while (true) {
                    size_t record_idx = next_record.fetch_add(1);
                    if (record_idx >= records_to_process) break;

                    const auto& rec_loc = all_records[record_idx];
                    int file_idx = rec_loc.file_index;

                    if (thread_files.find(file_idx) == thread_files.end()) {
                        const fileinfo& file_info = files[file_idx];
                        thread_files[file_idx] = std::make_unique<hipo::hipoeventfile>(file_info.filename, tags, 0);
                    }

                    auto& file = thread_files[file_idx];

                    for (long i = 0; i < rec_loc.num_events; i++) {
                        long evt_idx_in_file = rec_loc.first_event_in_file + i;

                        if (file->goto_event(evt_idx_in_file)) {
                            hipo::event temp_event;
                            if (file->read_next_event(temp_event)) {
                                iter_event evt(&temp_event, file.get());
                                process_func(evt, file_idx, evt_idx_in_file);
                            }
                        }

                        stats.events_processed++;
                        if (progress) progress->increment();
                    }
                }
            }));
        }

        for (auto& future : futures) {
            future.get();
        }

        stats.end_time = std::chrono::steady_clock::now();
        if (progress) progress->finish();
        if (verbose) print_statistics();
    }

    /**
     * @brief Print chain statistics
     */
    void print_statistics() const {
        fmt::print("chain statistics\n");
        fmt::print("Files:             {}\n", files.size());
        fmt::print("Total events:      {}\n", stats.total_events.load());
        fmt::print("Events processed:  {}\n", stats.events_processed.load());
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(stats.end_time - stats.start_time);
        fmt::print("Processing time:   {} seconds\n", duration.count());
        fmt::print("Throughput:        {:.1f} events/sec\n", stats.get_throughput());
    }

    /**
     * @brief List all files in chain
     */
    void list() const {
        fmt::print("chain file list\n");
        fmt::print("Total files: {}\n", files.size());
        for (const auto& file : files) {
            fmt::print("  [{}] {}", file.file_index, file.filename);
            if (file.total_events > 0) fmt::print(" ({} events)", file.total_events);
            fmt::print("\n");
        }
    }

    /**
     * @brief Clear all files from chain
     */
    void clear() {
        std::lock_guard<std::mutex> lock(chain_mutex);
        files.clear();
        all_events.clear();
        stats.reset();
        if (verbose) fmt::print("[chain] Cleared all files\n");
    }

    /**
     * @brief Get chain statistics
     */
    const chainstatistics& statistics() const { return stats; }

    /**
     * @brief Get ThreadPool reference
     */
    ThreadPool& threadpool() { return thread_pool; }

    /**
     * @brief Get total number of events across all files
     */
    long total_events_count() const { return all_events.size(); }

};

} // namespace hipo

