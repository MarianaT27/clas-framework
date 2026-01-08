#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "Concepts.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "IService.hpp"
#include "Result.hpp"
#include "StatusCode.hpp"

namespace Framework {

// Forward declaration
class LoggingService;

/**
 * @brief Manager for framework services
 *
 * The ServiceManager owns and provides access to all services in the framework.
 * Algorithms can retrieve services by name and type.
 * Services can declare dependencies on other services, and the manager will
 * initialize them in the correct order.
 */
class ServiceManager {
   public:
    /**
     * @brief Add a service to the manager (backward compatible)
     * @param service Unique pointer to the service
     * @return StatusCode indicating success or failure
     */
    auto add_service(std::unique_ptr<IService> service) -> StatusCode {
        return add_service(std::move(service), {});
    }

    /**
     * @brief Add a service to the manager with dependencies
     * @param service Unique pointer to the service
     * @param dependencies List of service names this service depends on
     * @return StatusCode indicating success or failure
     */
    auto add_service(std::unique_ptr<IService> service,
                     std::vector<std::string> dependencies) -> StatusCode {
        const std::string& name = service->name();
        if (m_services.find(name) != m_services.end()) {
            fmt::print(stderr, "ERROR: Service '{}' already exists\n", name);
            return StatusCode::failure(fmt::format("Service '{}' already exists", name));
        }
        m_services[name] = std::move(service);
        m_dependencies[name] = std::move(dependencies);
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Retrieve a service by name
     *
     * The template parameter is constrained by ServiceConcept, ensuring
     * at compile time that only valid service types can be requested.
     * This provides clear error messages when incompatible types are used.
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param name Name of the service
     * @return T* Pointer to the service, or nullptr if not found
     * @see ServiceConcept
     * @see Requirement 1.4
     */
    template <ServiceConcept T>
    auto get_service(const std::string& name) -> T* {
        auto it = m_services.find(name);
        if (it == m_services.end()) {
            fmt::print(stderr, "WARNING: Service '{}' not found\n", name);
            return nullptr;
        }
        return dynamic_cast<T*>(it->second.get());
    }

    /**
     * @brief Set the logging service to use for service initialization/finalization
     */
    auto set_logger(LoggingService* log) -> void {
        m_log = log;
    }

    /**
     * @brief Initialize all services in dependency order
     * @return StatusCode indicating success or failure
     *
     * Services are initialized in topologically sorted order, ensuring that
     * each service is initialized only after all its dependencies.
     */
    auto initialize_all() -> StatusCode {
        auto sort_result = topological_sort();
        if (sort_result.is_error()) {
            Framework::Log::error("ServiceManager", "{}", sort_result.error_message());
            return StatusCode::failure(sort_result.error_message());
        }

        const auto& order = sort_result.value();
        for (const auto& name : order) {
            auto it = m_services.find(name);
            if (it == m_services.end()) continue;

            Framework::Log::info("ServiceManager", "Initializing service: {}", name);

            auto sc = it->second->initialize();
            if (sc.is_failure()) {
                std::string error_msg = sc.message().empty()
                                            ? fmt::format("Failed to initialize service '{}'", name)
                                            : fmt::format("Failed to initialize service '{}': {}", name, sc.message());
                Framework::Log::error("ServiceManager", "{}", error_msg);
                return StatusCode::failure(error_msg);
            }
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Perform topological sort of services based on dependencies
     * @return Result containing ordered service names or error message
     *
     * Uses Kahn's algorithm to produce a valid initialization order.
     * Detects and reports circular dependencies.
     */
    auto topological_sort() const -> Result<std::vector<std::string>> {
        // Build in-degree map and adjacency list
        std::map<std::string, int> in_degree;
        std::map<std::string, std::vector<std::string>> dependents;

        // Initialize in-degree for all services
        for (const auto& [name, _] : m_services) {
            in_degree[name] = 0;
        }

        // Calculate in-degrees and build dependents map
        for (const auto& [name, deps] : m_dependencies) {
            for (const auto& dep : deps) {
                // Check if dependency exists
                if (m_services.find(dep) == m_services.end()) {
                    return Result<std::vector<std::string>>::error(
                        fmt::format("Service '{}' depends on non-existent service '{}'", name, dep));
                }
                in_degree[name]++;
                dependents[dep].push_back(name);
            }
        }

        // Kahn's algorithm
        std::vector<std::string> result;
        std::vector<std::string> queue;

        // Start with services that have no dependencies
        for (const auto& [name, degree] : in_degree) {
            if (degree == 0) {
                queue.push_back(name);
            }
        }

        while (!queue.empty()) {
            std::string current = queue.back();
            queue.pop_back();
            result.push_back(current);

            // Reduce in-degree for dependents
            for (const auto& dependent : dependents[current]) {
                in_degree[dependent]--;
                if (in_degree[dependent] == 0) {
                    queue.push_back(dependent);
                }
            }
        }

        // Check for cycles
        if (result.size() != m_services.size()) {
            // Find services involved in cycle
            std::vector<std::string> cycle_services;
            for (const auto& [name, degree] : in_degree) {
                if (degree > 0) {
                    cycle_services.push_back(name);
                }
            }
            return Result<std::vector<std::string>>::error(
                fmt::format("Circular dependency detected involving services: {}",
                            fmt::join(cycle_services, ", ")));
        }

        // SPECIAL CASE: NTupleService must finalize FIRST (initialize LAST)
        // Move NTupleService to the end of initialization order
        auto ntuple_it = std::find(result.begin(), result.end(), "NTupleService");
        if (ntuple_it != result.end()) {
            std::string ntuple_name = *ntuple_it;
            result.erase(ntuple_it);
            result.push_back(ntuple_name);
        }

        return Result<std::vector<std::string>>::ok(result);
    }

    /**
     * @brief Get the dependencies of a service
     * @param name Service name
     * @return Vector of dependency names (empty if service not found)
     */
    auto get_dependencies(const std::string& name) const -> std::vector<std::string> {
        auto it = m_dependencies.find(name);
        if (it != m_dependencies.end()) {
            return it->second;
        }
        return {};
    }

    /**
     * @brief Finalize all services
     * @return StatusCode indicating success or failure
     */
    auto finalize_all() -> StatusCode {
        // Get topological order
        auto result = topological_sort();
        if (!result.is_ok()) {
            std::string error_msg = fmt::format("Failed to determine service order: {}", result.error_message());
            Framework::Log::error("ServiceManager", "{}", error_msg);
            return StatusCode::failure(error_msg);
        }

        // Finalize in REVERSE order (opposite of initialization)
        std::vector<std::string> finalize_order = result.value();
        std::reverse(finalize_order.begin(), finalize_order.end());

        for (const auto& name : finalize_order) {
            auto it = m_services.find(name);
            if (it == m_services.end()) continue;

            Framework::Log::info("ServiceManager", "Finalizing service: {}", name);
            auto sc = it->second->finalize();
            if (sc.is_failure()) Framework::Log::warning("ServiceManager", "Failed to finalize service '{}'", name);
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Get the number of registered services
     * @return size_t Number of services
     */
    auto size() const -> size_t {
        return m_services.size();
    }

   private:
    std::map<std::string, std::unique_ptr<IService>> m_services;
    std::map<std::string, std::vector<std::string>> m_dependencies;
    LoggingService* m_log = nullptr;
};

}  // namespace Framework

// Include IAlgorithm.hpp for the Algorithm::service template implementation
#include "IAlgorithm.hpp"
#include "ServiceHandle.hpp"

// Template implementation for Algorithm::service
namespace Framework {
template <ServiceConcept T>
auto Algorithm::service(const std::string& service_name) -> T* {
    if (!m_svc_mgr) {
        fmt::print(stderr, "ERROR: ServiceManager not set for algorithm '{}'\n", m_name);
        return nullptr;
    }
    return m_svc_mgr->get_service<T>(service_name);
}

template <ServiceConcept T>
auto Algorithm::initialize_handle(ServiceHandle<T>& handle) -> StatusCode {
    if (!m_svc_mgr) {
        return StatusCode::failure(
            fmt::format("Algorithm '{}': ServiceManager not set", m_name));
    }

    auto sc = handle.initialize(*m_svc_mgr);
    if (sc.is_failure()) {
        return StatusCode::failure(
            fmt::format("Algorithm '{}': {}", m_name, sc.message()));
    }

    return StatusCode::SUCCESS();
}
}  // namespace Framework
