#pragma once

#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceManager.hpp"
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
     * @brief Set maximum number of events to process
     * @param max_events Maximum events (-1 for all)
     */
    auto set_max_events(long max_events) -> void {
        m_max_events = max_events;
    }

    /**
     * @brief Set number of threads for parallel processing
     * @param num_threads Number of threads
     */
    auto set_num_threads(int num_threads) -> void {
        m_num_threads = num_threads;
    }

    /**
     * @brief Initialize the application
     * @return StatusCode Success or failure
     */
    auto initialize() -> StatusCode {
        fmt::print("═══════════════════════════════════════════════════════════════\n");
        fmt::print("  Initializing Application: {}\n", m_name);
        fmt::print("═══════════════════════════════════════════════════════════════\n");

        // Initialize services
        fmt::print("\nInitializing Services:\n");
        auto sc = m_svc_mgr.initialize_all();
        if (sc.is_failure()) {
            return sc;
        }

        // Initialize algorithms
        fmt::print("\nInitializing Algorithms:\n");
        for (auto& alg : m_algorithms) {
            alg->set_service_manager(&m_svc_mgr);
            fmt::print("  - {}\n", alg->name());
            sc = alg->initialize();
            if (sc.is_failure()) {
                fmt::print(stderr, "ERROR: Failed to initialize algorithm '{}'\n",
                          alg->name());
                return sc;
            }
        }

        fmt::print("\nInitialization complete!\n");
        fmt::print("═══════════════════════════════════════════════════════════════\n\n");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Run the event loop
     * @return StatusCode Success or failure
     */
    auto run() -> StatusCode {
        if (m_input_files.empty()) {
            fmt::print(stderr, "ERROR: No input files specified\n");
            return StatusCode::FAILURE();
        }

        fmt::print("Starting event processing...\n");
        fmt::print("  Input files: {}\n", m_input_files.size());
        fmt::print("  Algorithms: {}\n", m_algorithms.size());
        fmt::print("  Threads: {}\n", m_num_threads);
        fmt::print("\n");

        // Create HIPO chain for parallel processing
        hipo::chain chain(m_num_threads, true, false);
        for (const auto& file : m_input_files) {
            chain.add(file);
        }

        long events_processed = 0;

        // Process events
        chain.process([this, &events_processed](hipo::iter_event& hipo_event,
                                                 int file_idx,
                                                 long evt_idx) {
            // Check max events limit
            if (m_max_events > 0 && events_processed >= m_max_events) {
                return;
            }
            events_processed++;

            // Create Event wrapper
            Event event(hipo_event);
            event.set_event_number(evt_idx);
            event.set_file_index(file_idx);

            // Execute algorithm sequence
            for (auto& alg : m_algorithms) {
                auto sc = alg->execute(event);
                if (sc.is_failure()) {
                    // Log error but continue processing
                    // In production, you might want to skip the event
                }
            }

            // Print progress
            if (events_processed % 10000 == 0) {
                fmt::print("  Processed {} events\r", events_processed);
                std::cout.flush();
            }
        }, 1);

        fmt::print("\nProcessed {} events\n", events_processed);
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Finalize the application
     * @return StatusCode Success or failure
     */
    auto finalize() -> StatusCode {
        fmt::print("\n");
        fmt::print("═══════════════════════════════════════════════════════════════\n");
        fmt::print("  Finalizing Application\n");
        fmt::print("═══════════════════════════════════════════════════════════════\n");

        // Finalize algorithms
        fmt::print("\nFinalizing Algorithms:\n");
        for (auto& alg : m_algorithms) {
            fmt::print("  - {}\n", alg->name());
            auto sc = alg->finalize();
            if (sc.is_failure()) {
                fmt::print(stderr, "WARNING: Failed to finalize algorithm '{}'\n",
                          alg->name());
            }
        }

        // Finalize services
        fmt::print("\nFinalizing Services:\n");
        auto sc = m_svc_mgr.finalize_all();

        fmt::print("\nFinalization complete!\n");
        fmt::print("═══════════════════════════════════════════════════════════════\n\n");
        return sc;
    }

private:
    std::string m_name;
    ServiceManager m_svc_mgr;
    std::vector<std::unique_ptr<IAlgorithm>> m_algorithms;
    std::vector<std::string> m_input_files;
    long m_max_events = -1;
    int m_num_threads = 1;
};

}  // namespace Framework
