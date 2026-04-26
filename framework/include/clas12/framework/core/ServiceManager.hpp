#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <clas12/framework/core/Concepts.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Result.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

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
     * @brief Add a service to the manager
     *
     * Dependencies are pulled from the service itself via
     * IService::required_dependencies() — services that call
     * Service::require_service<T>() in their constructor participate in the
     * topological sort automatically, with no additional call-site changes.
     */
    auto add_service(std::unique_ptr<IService> service) -> StatusCode {
        auto deps = service->required_dependencies();
        return add_service(std::move(service), std::move(deps));
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
            clas12::framework::Log::error("ServiceManager", "Service '{}' already exists", name);
            return StatusCode::failure(fmt::format("Service '{}' already exists", name));
        }
        // Cache the dynamic type so get_service<T>() can do an O(1) lookup
        // without a dynamic_cast loop across every registered service.
        IService* raw = service.get();
        m_type_cache[std::type_index(typeid(*raw))] = raw;
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
            clas12::framework::Log::warning("ServiceManager", "Service '{}' not found", name);
            return nullptr;
        }
        return dynamic_cast<T*>(it->second.get());
    }

    /**
     * @brief Retrieve a service by concrete type (O(1) lookup)
     *
     * Uses a type-indexed cache populated in add_service. Only the concrete
     * type is matched; a base-class lookup returns nullptr. For interface
     * lookups or multiple instances of the same type, use the name-based
     * overload instead.
     */
    template <ServiceConcept T>
    auto get_service() -> T* {
        auto it = m_type_cache.find(std::type_index(typeid(T)));
        if (it == m_type_cache.end()) return nullptr;
        return static_cast<T*>(it->second);
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
            clas12::framework::Log::error("ServiceManager", "{}", sort_result.error_message());
            return StatusCode::failure(sort_result.error_message());
        }

        const auto& order = sort_result.value();
        for (const auto& name : order) {
            auto it = m_services.find(name);
            if (it == m_services.end()) continue;

            // Inject service locator so services can discover dependencies
            if (auto* svc = dynamic_cast<Service*>(it->second.get())) {
                svc->set_service_manager(this);
            }

            clas12::framework::Log::info("ServiceManager", "Initializing service: {}", name);

            auto sc = it->second->initialize();
            if (sc.is_failure()) {
                std::string error_msg = sc.message().empty()
                                            ? fmt::format("Failed to initialize service '{}'", name)
                                            : fmt::format("Failed to initialize service '{}': {}", name, sc.message());
                clas12::framework::Log::error("ServiceManager", "{}", error_msg);
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
            clas12::framework::Log::error("ServiceManager", "{}", error_msg);
            return StatusCode::failure(error_msg);
        }

        // Finalize in REVERSE order (opposite of initialization)
        std::vector<std::string> finalize_order = result.value();
        std::reverse(finalize_order.begin(), finalize_order.end());

        for (const auto& name : finalize_order) {
            auto it = m_services.find(name);
            if (it == m_services.end()) continue;

            clas12::framework::Log::info("ServiceManager", "Finalizing service: {}", name);
            auto sc = it->second->finalize();
            if (sc.is_failure()) clas12::framework::Log::warning("ServiceManager", "Failed to finalize service '{}'", name);
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

    /**
     * @brief Return the names of all registered services, in no particular order
     *
     * Used by introspection tools (service-graph emitter, etc.).
     */
    /// Collect the union of required_banks() from every registered service.
    [[nodiscard]] auto collect_service_banks() const -> std::vector<std::string> {
        std::vector<std::string> banks;
        for (const auto& [name, svc] : m_services) {
            for (auto& bank : svc->required_banks()) {
                banks.push_back(std::move(bank));
            }
        }
        return banks;
    }

    [[nodiscard]] auto service_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_services.size());
        for (const auto& [name, _] : m_services) {
            names.push_back(name);
        }
        return names;
    }

   private:
    std::map<std::string, std::unique_ptr<IService>> m_services;
    std::map<std::string, std::vector<std::string>> m_dependencies;
    std::unordered_map<std::type_index, IService*> m_type_cache;
};

}  // namespace clas12::framework

namespace clas12::framework {

// Service::locate_service_impl - deferred until ServiceManager is complete
template <typename T>
auto Service::locate_service_impl(const std::string& name) -> T* {
    return m_svc_mgr->get_service<T>(name);
}
}  // namespace clas12::framework
