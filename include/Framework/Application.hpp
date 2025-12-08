#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceManager.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Event.hpp"
#include "Framework/StatusCode.hpp"
#include "hipoeventiterator.h"
#include "hipo4/chain.h"
#include <vector>
#include <memory>
#include <string>
#include <fmt/format.h>

namespace Framework {

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
     * Also sets the global Log singleton for easy access from anywhere
     */
    auto set_logger(LoggingService* log) -> void {
        m_log = log;
        // Set the global logger singleton for easy access
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
        if (!m_log) {
            m_log = m_svc_mgr.get_service<LoggingService>("LoggingService");
            if (m_log) {
                Log::set_instance(m_log);
            }
        }

        if (m_log) {
            m_log->info("Application", "═══════════════════════════════════════════════════════════════");
            m_log->info("Application", "  Initializing Application: {}", m_name);
            m_log->info("Application", "═══════════════════════════════════════════════════════════════");
            m_log->info("Application", "");
            m_log->info("Application", "Services initialized");
        }

        if (m_log) {
            m_log->info("Application", "");
            m_log->info("Application", "Initializing Algorithms:");
        }

        // Initialize algorithms
        for (auto& alg : m_algorithms) {
            alg->set_service_manager(&m_svc_mgr);
            if (m_log) {
                m_log->info("Application", "  - {}", alg->name());
            }
            sc = alg->initialize();
            if (sc.is_failure()) {
                if (m_log) {
                    m_log->error("Application", "Failed to initialize algorithm '{}'", alg->name());
                } else {
                    fmt::print(stderr, "ERROR: Failed to initialize algorithm '{}'\n", alg->name());
                }
                return sc;
            }
        }

        if (m_log) {
            m_log->info("Application", "");
            m_log->info("Application", "Initialization complete!");
            m_log->info("Application", "═══════════════════════════════════════════════════════════════");
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Run the event loop
     * @return StatusCode Success or failure
     */
    auto run() -> StatusCode {
        if (m_input_files.empty()) {
            if (m_log) {
                m_log->error("Application", "No input files specified");
            } else {
                fmt::print(stderr, "ERROR: No input files specified\n");
            }
            return StatusCode::FAILURE();
        }

        if (m_log) {
            m_log->info("Application", "");
            m_log->info("Application", "Starting event processing...");
            m_log->info("Application", "  Input files: {}", m_input_files.size());
            m_log->info("Application", "  Algorithms: {}", m_algorithms.size());
            m_log->info("Application", "  Threads: {}", m_num_threads);
        }

        // Create HIPO chain for parallel processing
        hipo::chain chain(m_num_threads, true, false);
        for (const auto& file : m_input_files) {
            chain.add(file);
        }


        // Process events
        chain.process([this](hipo::iter_event& hipo_event, int file_idx, long evt_idx) {

            // Create Event wrapper
            Event event(hipo_event);
            event.set_event_number(evt_idx);
            event.set_file_index(file_idx);

            // Execute algorithm sequence
            for (auto& alg : m_algorithms) {
                try {
                    auto sc = alg->execute(event);
                    if (sc.is_failure()) {
                        fmt::print(stderr, "ERROR: Algorithm '{}' failed on event {}\n", alg->name(), evt_idx);
                        m_exception_count++;
                    }
                } catch (const std::exception& e) {
                    fmt::print(stderr, "ERROR: Exception in algorithm '{}': {}\n", alg->name(), e.what());
                    m_exception_count++;
                } catch (...) {
                    fmt::print(stderr, "ERROR: Unknown exception in algorithm '{}'\n", alg->name());
                    m_exception_count++;
                }
            }
        }, m_percentage);

        if (m_exception_count > 0) {
            fmt::print("WARNING: {} exception(s) caught during event processing\n", m_exception_count);
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Finalize the application
     * @return StatusCode Success or failure
     */
    auto finalize() -> StatusCode {
        if (m_log) {
            m_log->info("Application", "");
            m_log->info("Application", "═══════════════════════════════════════════════════════════════");
            m_log->info("Application", "  Finalizing Application");
            m_log->info("Application", "═══════════════════════════════════════════════════════════════");
            m_log->info("Application", "");
            m_log->info("Application", "Finalizing Algorithms:");
        }

        // Finalize algorithms
        for (auto& alg : m_algorithms) {
            if (m_log) {
                m_log->info("Application", "  - {}", alg->name());
            }
            auto sc = alg->finalize();
            if (sc.is_failure()) {
                if (m_log) {
                    m_log->warning("Application", "Failed to finalize algorithm '{}'", alg->name());
                } else {
                    fmt::print(stderr, "WARNING: Failed to finalize algorithm '{}'\n", alg->name());
                }
            }
        }

        if (m_log) {
            m_log->info("Application", "");
            m_log->info("Application", "Finalizing Services:");
        }

        // Finalize services
        auto sc = m_svc_mgr.finalize_all();

        if (m_log) {
            m_log->info("Application", "");
            m_log->info("Application", "Finalization complete!");
            m_log->info("Application", "═══════════════════════════════════════════════════════════════");
        }
        return sc;
    }

    /**
     * @brief Get the number of exceptions caught during event processing
     * @return Number of exceptions caught
     */
    auto exception_count() const -> long {
        return m_exception_count;
    }

private:
    std::string m_name;
    ServiceManager m_svc_mgr;
    std::vector<std::unique_ptr<IAlgorithm>> m_algorithms;
    std::vector<std::string> m_input_files;
    double m_percentage = 100;
    int m_num_threads = 1;
    mutable long m_exception_count = 0;
    LoggingService* m_log = nullptr;
};

}  // namespace Framework
