#pragma once

#include <fmt/format.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <toml++/toml.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/PerformanceMonitor.hpp>
#include <chain.h>
#include <reader.h>
#include <dictionary.h>

namespace framework {

/**
 * @brief Main application class for running analyses
 *
 * The Application class manages the overall analysis workflow:
 * 1. Initialize services
 * 2. Initialize algorithms
 * 3. Process events through the algorithm sequence
 * 4. Finalize algorithms and services
 *
 * This follows the application framework pattern used in Gaudi/Athena.
 */
class Application {
   public:
    Application(std::string name) : m_name(std::move(name)) {}

    // Move-only: std::atomic is neither copyable nor movable
    Application(Application&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_svc_mgr(std::move(other.m_svc_mgr))
        , m_algorithms(std::move(other.m_algorithms))
        , m_input_files(std::move(other.m_input_files))
        , m_config(std::move(other.m_config))
        , m_record_first(other.m_record_first)
        , m_record_last(other.m_record_last)
        , m_progress_fd(other.m_progress_fd)
        , m_slice_id(other.m_slice_id)
        , m_quiet_mode(other.m_quiet_mode)
        , m_percentage(other.m_percentage)
        , m_num_threads(other.m_num_threads)
        , m_exception_count(other.m_exception_count.load())
        , m_profiling(other.m_profiling)
        , m_perf(std::move(other.m_perf)) {}
    Application& operator=(Application&&) = delete;
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /**
     * @brief Add an algorithm to the processing sequence
     * @param algorithm Unique pointer to the algorithm
     */
    auto add_algorithm(std::unique_ptr<Algorithm> algorithm) -> void {
        m_algorithms.push_back(std::move(algorithm));
    }

    /**
     * @brief Get the service manager
     * @return ServiceManager& Reference to service manager
     */
    auto service_manager() -> ServiceManager& {
        return m_svc_mgr;
    }

    /**
     * @brief Read-only view of registered algorithms (in execution order)
     *
     * Intended for introspection tools like the service-graph emitter.
     * Do not mutate the algorithms through this reference.
     */
    [[nodiscard]] auto algorithms() const
        -> const std::vector<std::unique_ptr<Algorithm>>& {
        return m_algorithms;
    }

    /**
     * @brief Set input files
     * @param files Vector of file paths
     */
    auto set_input_files(std::vector<std::string> files) -> void {
        m_input_files = std::move(files);
    }

    /**
     * @brief Set percentage of events to process
     * @param percentage Percentage of events (0-100)
     */
    auto set_percentage(double percentage) -> void {
        m_percentage = percentage;
    }

    /**
     * @brief Set number of threads for parallel processing
     * @param num_threads Number of threads
     */
    auto set_num_threads(int num_threads) -> void {
        m_num_threads = num_threads;
    }

    /**
     * @brief Restrict event processing to a half-open record range
     *        [first, last). Negative `last` means "to the end of every file".
     *
     * Used by the local batch driver to slice one big skim file across N
     * concurrent processes — each process gets a disjoint record slice and
     * a unique output file. Default range covers the whole file.
     */
    auto set_record_range(int first, int last) -> void {
        m_record_first = first;
        m_record_last  = last;
    }

    [[nodiscard]] auto record_range_first() const -> int { return m_record_first; }
    [[nodiscard]] auto record_range_last()  const -> int { return m_record_last; }

    /**
     * @brief Optional out-of-band progress reporter for tools/local_batch.
     *
     * When `fd >= 0`, `run()` suppresses the chain's terminal progress bar
     * and instead spawns a small background thread that periodically
     * writes one line `<slice_id> <events_processed> <total_events>\n`
     * to the given file descriptor. The parent batch driver reads from a
     * shared pipe and renders a single aggregate bar.
     *
     * Setting `fd < 0` (the default) leaves the legacy in-process bar on.
     */
    auto set_progress_reporter(int fd, int slice_id) -> void {
        m_progress_fd = fd;
        m_slice_id    = slice_id;
    }

    /**
     * @brief Quiet mode — suppresses every LoggingService console write.
     *
     * Used by `tools/local_batch` when many worker processes are running
     * in parallel: the parent prints one config banner + one progress
     * bar; each child's per-algorithm Configuration tables and cutflow
     * summaries get redirected to a per-slice log file at exec time, so
     * silencing the console here avoids double-redirecting the terminal
     * inside the child as well.
     */
    auto set_quiet_mode(bool quiet) -> void {
        m_quiet_mode = quiet;
    }

    /**
     * @brief Enable opt-in per-step performance profiling (wall vs thread-CPU
     *        per algorithm + a synthetic "Input" step), reported at finalize.
     *        Off by default → the event loop uses its untimed path.
     */
    auto set_profiling(bool enable) -> void {
        m_profiling = enable;
    }

    /**
     * @brief Set logging service for structured output
     * Sets the global Log singleton for easy access from anywhere
     */
    auto set_logger(LoggingService* log) -> void {
        Log::set_instance(log);
    }

    /**
     * @brief Attach the parsed TOML config table.
     *
     * The framework will look up `[<algorithm.name()>]` for every
     * registered algorithm during `initialize()` and apply matching
     * `Property` values via `PropertyMixin::configure_from_toml`.
     * Pass nullptr (or skip the call) for property defaults.
     */
    auto set_config(std::shared_ptr<const toml::table> config) -> void {
        m_config = std::move(config);
    }

    /**
     * @brief Initialize the application
     * @return StatusCode Success or failure
     */
    auto initialize() -> StatusCode {
        // Initialize services first
        auto sc = m_svc_mgr.initialize_all();
        if (sc.is_failure()) {
            return sc;
        }

        // `m_quiet_mode` historically suppressed LoggingService console
        // output too, but in tools/local_batch's setup the child's
        // stdout is already dup2'd to its per-slice log file at fork
        // time — so the per-algorithm Configuration banners *should*
        // reach that file. The only thing we keep suppressed is the
        // in-process progress bar (done via `chain.set_progress(false)`
        // in `run()` when `m_progress_fd >= 0`).

        // Auto-detect LoggingService if not manually set
        if (!Log::is_available()) {
            auto* log = m_svc_mgr.get_service<LoggingService>("LoggingService");
            if (log) {
                Log::set_instance(log);
            }
        }

        Log::info("Application", "═══════════════════════════════════════════════════════════════");
        Log::info("Application", "  Initializing Application: {}", m_name);
        Log::info("Application", "═══════════════════════════════════════════════════════════════");
        Log::info("Application", "");
        Log::info("Application", "Services initialized");

        Log::info("Application", "");
        Log::info("Application", "Initializing Algorithms:");

        // Initialize algorithms. Apply per-algorithm TOML config (if
        // any) BEFORE the algorithm's initialize() runs so on_initialize()
        // sees the configured Property values when it books histograms,
        // sets cuts, etc.
        //
        // `[analysis]` keys are merged into every per-algorithm sub-table
        // as defaults — so settings like `target = "LD2"` set once in the
        // analysis section are visible to any algorithm Config that lists
        // that field. Per-algorithm overrides win on conflicts.
        const toml::table* analysis_section = m_config
            ? (*m_config)["analysis"].as_table()
            : nullptr;

        for (auto& alg : m_algorithms) {
            alg->set_service_manager(&m_svc_mgr);
            Log::info("Application", "  - {}", alg->name());

            if (m_config) {
                toml::table merged;
                if (analysis_section) merged = *analysis_section;
                if (auto sub = (*m_config)[alg->name()].as_table()) {
                    for (auto&& [k, v] : *sub) {
                        merged.insert_or_assign(k, v);
                    }
                }
                if (!merged.empty()) {
                    auto cfg_sc = alg->configure_from_toml(merged);
                    if (cfg_sc.is_failure()) {
                        Log::error("Application",
                            "Algorithm '{}' rejected TOML config: {}",
                            alg->name(), cfg_sc.message());
                        return cfg_sc;
                    }
                }
            }

            sc = alg->initialize();
            if (sc.is_failure()) {
                Log::error("Application", "Failed to initialize algorithm '{}'", alg->name());
                return sc;
            }
        }

        // Warn about top-level TOML sections that don't map to any
        // registered algorithm or to the framework's reserved keys —
        // catches typos like `[ElectronAnaylsis]` early.
        if (m_config) {
            std::set<std::string> known{"run", "analysis"};
            for (auto& alg : m_algorithms) known.insert(alg->name());
            for (auto&& [k, v] : *m_config) {
                if (!known.contains(std::string(k.str()))) {
                    Log::warning("Application",
                        "TOML section '[{}]' does not match any registered "
                        "algorithm; check for typos.", k.str());
                }
            }
        }

        Log::info("Application", "");
        Log::info("Application", "Initialization complete!");
        Log::info("Application", "═══════════════════════════════════════════════════════════════");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Run the event loop
     * @return StatusCode Success or failure
     */
    auto run() -> StatusCode {
        if (m_input_files.empty()) {
            Log::error("Application", "No input files specified");
            return StatusCode::FAILURE();
        }

        // Probe the schema dictionary from the first input file so we can
        // catch missing required banks up-front. The chain's own dictionary
        // is built lazily; a one-shot reader open is cheaper than waiting
        // for the first worker to fail.
        hipo::dictionary dict;
        {
            const std::string& first = m_input_files.front();
            hipo::reader probe;
            probe.open(first.c_str());
            if (!probe.is_open()) {
                Log::error("Application",
                    "Cannot open input file '{}'. Check the path exists and is a "
                    "readable HIPO file (a glob in [run].input that matches nothing "
                    "is dropped, which can leave the list empty).",
                    first);
                return StatusCode::FAILURE();
            }
            probe.readDictionary(dict);
        }

        std::set<std::string> unique_banks;
        for (auto& alg : m_algorithms) {
            for (auto& bank : alg->required_banks()) {
                if (!dict.hasSchema(bank.c_str())) {
                    Log::error("Application",
                        "Required bank '{}' (algorithm '{}') not found in input dictionary",
                        bank, alg->name());
                    return StatusCode::FAILURE();
                }
                unique_banks.insert(bank);
            }
        }
        for (auto& alg : m_algorithms) {
            for (auto& bank : alg->optional_banks()) {
                if (dict.hasSchema(bank.c_str())) {
                    unique_banks.insert(bank);
                } else {
                    Log::warning("Application",
                        "Optional bank '{}' not in input dictionary; algorithm '{}' will see 0 rows",
                        bank, alg->name());
                }
            }
        }
        // Always include REC::Particle as it's needed by the Event class
        unique_banks.insert("REC::Particle");
        auto all_required_banks = std::vector<std::string>(unique_banks.begin(), unique_banks.end());

        // Create HIPO chain for parallel processing. The Lz4ByPage v2 +
        // mmap zero-copy fast path is wired in chain.h itself; no per-run
        // I/O tuning surface remains.
        //
        // Drive the chain's terminal progress bar based on quiet mode. When
        // `m_progress_fd >= 0` an external aggregator (tools/local_batch) is
        // rendering one bar across all worker processes, so the per-process
        // bar would just collide with it.
        const bool show_chain_progress = !m_quiet_mode && (m_progress_fd < 0);
        hipo::chain chain(m_num_threads, show_chain_progress, /*verbose=*/false);
        for (const auto& file : m_input_files) {
            chain.add(file);
        }
        chain.set_record_range(m_record_first, m_record_last);

        Log::info("Application", "");
        Log::info("Application", "Starting event processing...");
        Log::info("Application", "  Input files: {}", m_input_files.size());
        Log::info("Application", "  Algorithms: {}", m_algorithms.size());
        Log::info("Application", "  Required banks: {}", all_required_banks.size());
        Log::info("Application", "  Threads: {}", m_num_threads);
        if (m_record_first > 0 || m_record_last >= 0) {
            Log::info("Application",
                      "  Record range: [{}, {})",
                      m_record_first,
                      m_record_last < 0 ? std::string("end") : std::to_string(m_record_last));
        }

        // Optional out-of-band progress reporter for tools/local_batch.
        // Activated when `--progress-fd N` was passed at startup; the parent
        // batch driver dup2'd the write end of a pipe onto that FD and reads
        // aggregated lines from the other side. We sample chain.events_processed()
        // every 200 ms and emit one text line per sample so the parent can
        // demultiplex by slice id and render a single bar.
        std::atomic<bool>       progress_stop{false};
        std::thread             progress_thread;
        const bool              report_progress = (m_progress_fd >= 0);
        if (report_progress) {
            progress_thread = std::thread([this, &chain, &progress_stop]() {
                using namespace std::chrono_literals;
                long last_total_seen = -1;
                while (!progress_stop.load(std::memory_order_relaxed)) {
                    const long ev = chain.events_processed();
                    const long tot = chain.total_events();
                    if (tot != last_total_seen || ev > 0) {
                        last_total_seen = tot;
                        auto line = fmt::format("{} {} {}\n", m_slice_id, ev, tot);
                        // Best-effort write — closed pipe / EPIPE just stops the loop.
                        if (::write(m_progress_fd, line.data(), line.size()) < 0) break;
                    }
                    std::this_thread::sleep_for(200ms);
                }
                // Final flush so the parent sees the actual processed count
                // even if the loop happened to skip the last sample.
                const long ev = chain.events_processed();
                const long tot = chain.total_events();
                auto line = fmt::format("{} {} {}\n", m_slice_id, ev, tot);
                (void) ::write(m_progress_fd, line.data(), line.size());
            });
        }

        // Run one algorithm with the established exception handling.
        // Returns: 0 = ok (continue), 1 = filtered (stop event), 2 = failure.
        auto exec_one = [this](Algorithm& alg, Event& event, long evt_idx) -> int {
            try {
                auto sc = alg.execute(event);
                if (sc.is_failure()) {
                    fmt::print(stderr, "ERROR: Algorithm '{}' failed on event {}\n", alg.name(), evt_idx);
                    m_exception_count++;
                    return 2;
                }
                if (sc.is_filtered()) return 1;
                return 0;
            } catch (const std::exception& e) {
                fmt::print(stderr, "ERROR: Exception in algorithm '{}': {}\n", alg.name(), e.what());
                m_exception_count++;
                return 2;
            } catch (...) {
                fmt::print(stderr, "ERROR: Unknown exception in algorithm '{}'\n", alg.name());
                m_exception_count++;
                return 2;
            }
        };

        // Process events. Two event-loop bodies selected ONCE here, outside the
        // hot path: the untimed default (zero added overhead) and, under
        // --profile, a timed body that records per-algorithm wall + thread-CPU
        // (plus a synthetic "Input (read+decode)" step = the gap between
        // consecutive events on a worker thread) into the PerformanceMonitor.
        if (m_profiling) {
            m_perf = std::make_unique<perf::PerformanceMonitor>();
            std::vector<std::string> step_names;
            step_names.reserve(m_algorithms.size() + 1);
            for (auto& alg : m_algorithms) step_names.push_back(alg->name());
            step_names.emplace_back("Input (read+decode)");
            m_perf->init(std::move(step_names));
            const std::size_t input_idx = m_perf->input_index();

            chain.process([this, &exec_one, input_idx](hipo::chain_event& chain_evt, int file_idx, long evt_idx) {
                Event event(chain_evt);
                event.set_event_number(evt_idx);
                event.set_file_index(file_idx);

                auto* ts = m_perf->thread_state();
                const std::uint64_t ev_w0 = perf::now_wall_ns();
                const std::uint64_t ev_c0 = perf::now_cpu_ns();
                if (ts->have_last) {  // time the chain spent reading+decoding since the last event
                    ts->steps[input_idx].wall_ns += ev_w0 - ts->last_wall;
                    ts->steps[input_idx].cpu_ns  += ev_c0 - ts->last_cpu;
                    ts->steps[input_idx].count   += 1;
                }
                for (std::size_t i = 0; i < m_algorithms.size(); ++i) {
                    const std::uint64_t w0 = perf::now_wall_ns();
                    const std::uint64_t c0 = perf::now_cpu_ns();
                    const int r = exec_one(*m_algorithms[i], event, evt_idx);
                    ts->steps[i].wall_ns += perf::now_wall_ns() - w0;
                    ts->steps[i].cpu_ns  += perf::now_cpu_ns()  - c0;
                    ts->steps[i].count   += 1;
                    if (r != 0) break;
                }
                ts->last_wall = perf::now_wall_ns();
                ts->last_cpu  = perf::now_cpu_ns();
                ts->have_last = true;
            }, m_percentage);
        } else {
            chain.process([this, &exec_one](hipo::chain_event& chain_evt, int file_idx, long evt_idx) {
                Event event(chain_evt);
                event.set_event_number(evt_idx);
                event.set_file_index(file_idx);
                for (auto& alg : m_algorithms) {
                    if (exec_one(*alg, event, evt_idx) != 0) break;
                }
            }, m_percentage);
        }
        (void)all_required_banks;  // kept around for the dictionary probe / future banklist pre-binding

        if (report_progress) {
            progress_stop.store(true, std::memory_order_relaxed);
            if (progress_thread.joinable()) progress_thread.join();
        }

        if (m_exception_count > 0) {
            fmt::print("WARNING: {} exception(s) caught during event processing\n", m_exception_count.load());
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Finalize the application
     * @return StatusCode Success or failure
     */
    auto finalize() -> StatusCode {
        Log::info("Application", "");
        Log::info("Application", "═══════════════════════════════════════════════════════════════");
        Log::info("Application", "  Finalizing Application");
        Log::info("Application", "═══════════════════════════════════════════════════════════════");
        Log::info("Application", "");
        Log::info("Application", "Finalizing Algorithms:");

        // Finalize algorithms
        for (auto& alg : m_algorithms) {
            Log::info("Application", "  - {}", alg->name());
            auto sc = alg->finalize();
            if (sc.is_failure()) {
                Log::warning("Application", "Failed to finalize algorithm '{}'", alg->name());
            }
        }

        Log::info("Application", "");
        Log::info("Application", "Finalizing Services:");

        // Finalize services
        auto sc = m_svc_mgr.finalize_all();

        Log::info("Application", "");
        Log::info("Application", "Finalization complete!");
        Log::info("Application", "═══════════════════════════════════════════════════════════════");

        // Opt-in per-step performance report. Printed last so it's the final
        // thing on screen / in the per-slice log. Under local_batch we also
        // stream the per-step PERF records back over the progress pipe (already
        // joined in run(), so no interleaving) for the aggregated batch table.
        if (m_profiling && m_perf) {
            m_perf->report();
            if (m_progress_fd >= 0) {
                const std::string lines = m_perf->perf_lines(m_slice_id);
                (void)::write(m_progress_fd, lines.data(), lines.size());
            }
        }
        return sc;
    }

    /**
     * @brief Get the number of exceptions caught during event processing
     * @return Number of exceptions caught
     */
    auto exception_count() const -> long {
        return m_exception_count.load();
    }

   private:
    std::string m_name;
    ServiceManager m_svc_mgr;
    std::vector<std::unique_ptr<Algorithm>> m_algorithms;
    std::vector<std::string> m_input_files;
    /// Optional parsed TOML config; consulted once per algorithm at
    /// initialize() time. Null = use Property C++ defaults.
    std::shared_ptr<const toml::table> m_config;
    /// Half-open record slice [m_record_first, m_record_last). Last < 0
    /// means "to end of file". Default = process the whole input.
    int m_record_first = 0;
    int m_record_last  = -1;
    /// Optional progress-pipe descriptor handed to us by a parent batch
    /// driver. `m_progress_fd >= 0` activates the periodic reporter
    /// thread in `run()`. `m_slice_id` tags each pipe line so the
    /// aggregator can demultiplex.
    int m_progress_fd = -1;
    int m_slice_id    = -1;
    bool m_quiet_mode = false;
    double m_percentage = 100;
    int m_num_threads = 1;
    mutable std::atomic<long> m_exception_count{0};
    /// Opt-in per-step profiling (--profile / [analysis].profile). When true
    /// run() uses a timed event-loop lambda feeding m_perf; finalize() reports.
    bool m_profiling = false;
    std::unique_ptr<perf::PerformanceMonitor> m_perf;
};

}  // namespace framework
