#pragma once

#include <fmt/format.h>
#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IAlgorithm.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <chain.h>

namespace clas12::framework {

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
    auto add_algorithm(std::unique_ptr<IAlgorithm> algorithm) -> void {
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
        -> const std::vector<std::unique_ptr<IAlgorithm>>& {
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
     * @brief Set logging service for structured output
     * Sets the global Log singleton for easy access from anywhere
     */
    auto set_logger(LoggingService* log) -> void {
        Log::set_instance(log);
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

        // Initialize algorithms
        for (auto& alg : m_algorithms) {
            alg->set_service_manager(&m_svc_mgr);
            Log::info("Application", "  - {}", alg->name());
            sc = alg->initialize();
            if (sc.is_failure()) {
                Log::error("Application", "Failed to initialize algorithm '{}'", alg->name());
                return sc;
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

        // Create HIPO chain for parallel processing
        hipo::chain chain({.threads = m_num_threads, .show_progress = true});
        for (const auto& file : m_input_files) {
            chain.add(file);
        }
        auto open_result = chain.open();
        if (!open_result) {
            Log::error("Application", "Failed to open HIPO chain");
            return StatusCode::FAILURE();
        }

        // Collect only the banks that algorithms actually need
        std::set<std::string> unique_banks;
        for (auto& alg : m_algorithms) {
            for (auto& bank : alg->required_banks()) {
                unique_banks.insert(bank);
            }
        }
        // Always include REC::Particle as it's needed by the Event class
        unique_banks.insert("REC::Particle");
        auto all_required_banks = std::vector<std::string>(unique_banks.begin(), unique_banks.end());

        Log::info("Application", "");
        Log::info("Application", "Starting event processing...");
        Log::info("Application", "  Input files: {}", m_input_files.size());
        Log::info("Application", "  Algorithms: {}", m_algorithms.size());
        Log::info("Application", "  Required banks: {}", all_required_banks.size());
        Log::info("Application", "  Threads: {}", m_num_threads);

        // Process events
        chain.process([this](hipo::bank_view_set& banks, int file_idx, int64_t evt_idx) {
            // Create Event wrapper
            Event event(banks);
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
        }, all_required_banks, m_percentage);

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
    std::vector<std::unique_ptr<IAlgorithm>> m_algorithms;
    std::vector<std::string> m_input_files;
    double m_percentage = 100;
    int m_num_threads = 1;
    mutable std::atomic<long> m_exception_count{0};
};

}  // namespace clas12::framework
