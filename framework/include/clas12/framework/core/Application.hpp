#pragma once

#include <fmt/format.h>
#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <toml++/toml.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <chain.h>
#include <reader.h>
#include <dictionary.h>
#include <ioOptions.h>

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
        , m_io_options(std::move(other.m_io_options))
        , m_record_first(other.m_record_first)
        , m_record_last(other.m_record_last)
        , m_percentage(other.m_percentage)
        , m_num_threads(other.m_num_threads)
        , m_exception_count(other.m_exception_count.load()) {}
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
     * @brief Set the hipo::chain I/O options.
     *
     * Drives the new prefetch / record-cache / I/O thread pool surface
     * the chain exposes. Defaults preserve the legacy synchronous read
     * path; opt in by setting prefetch_depth > 0 and / or
     * record_cache_bytes > 0 (typical ifarm values: depth 4, cache 128MB,
     * io_threads 2).
     */
    auto set_io_options(const hipo::IOOptions& opts) -> void {
        m_io_options = opts;
    }

    [[nodiscard]] auto io_options() const -> const hipo::IOOptions& {
        return m_io_options;
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
            hipo::reader probe;
            probe.open(m_input_files.front().c_str());
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

        // Create HIPO chain for parallel processing. The 4-arg ctor wires
        // the I/O options (prefetch depth, record cache, async I/O thread
        // count). With default IOOptions{} the chain falls back to the
        // legacy synchronous read path.
        const bool io_active = m_io_options.prefetch_depth > 0
                            || m_io_options.record_cache_bytes > 0
                            || m_io_options.io_threads > 0;
        hipo::chain chain(m_num_threads, /*progress=*/true, /*verbose=*/false,
                          m_io_options);
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
        if (io_active) {
            Log::info("Application", "  I/O options: prefetch_depth={}, "
                                      "record_cache={} MB, io_threads={}",
                      m_io_options.prefetch_depth,
                      m_io_options.record_cache_bytes / (1024 * 1024),
                      m_io_options.io_threads);
            if (!m_io_options.bank_filter.empty()) {
                Log::info("Application", "  I/O bank filter ({} banks)",
                          m_io_options.bank_filter.size());
            }
        }

        // Process events
        chain.process([this](hipo::chain_event& chain_evt, int file_idx, long evt_idx) {
            // Create Event wrapper around the chain's per-callback view
            Event event(chain_evt);
            event.set_event_number(evt_idx);
            event.set_file_index(file_idx);

            // Execute algorithm sequence
            for (auto& alg : m_algorithms) {
                try {
                    auto sc = alg->execute(event);
                    if (sc.is_failure()) {
                        fmt::print(stderr, "ERROR: Algorithm '{}' failed on event {}\n", alg->name(), evt_idx);
                        m_exception_count++;
                        break;  // Stop processing this event on failure
                    }
                    if (sc.is_filtered()) {
                        // Event filtered out by this algorithm, skip remaining algorithms
                        break;
                    }
                } catch (const std::exception& e) {
                    fmt::print(stderr, "ERROR: Exception in algorithm '{}': {}\n", alg->name(), e.what());
                    m_exception_count++;
                    break;
                } catch (...) {
                    fmt::print(stderr, "ERROR: Unknown exception in algorithm '{}'\n", alg->name());
                    m_exception_count++;
                    break;
                }
            }
        }, m_percentage);
        (void)all_required_banks;  // kept around for the dictionary probe / future banklist pre-binding

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
    /// hipo::chain I/O tuning (prefetch depth, record cache, async I/O
    /// thread count, bank filter). Defaults preserve the legacy
    /// synchronous read path.
    hipo::IOOptions m_io_options{};
    /// Half-open record slice [m_record_first, m_record_last). Last < 0
    /// means "to end of file". Default = process the whole input.
    int m_record_first = 0;
    int m_record_last  = -1;
    double m_percentage = 100;
    int m_num_threads = 1;
    mutable std::atomic<long> m_exception_count{0};
};

}  // namespace framework
