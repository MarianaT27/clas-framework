#pragma once

#include "IService.hpp"
#include "StatusCode.hpp"
#include <map>
#include <memory>
#include <string>
#include <fmt/format.h>

namespace Framework {

/**
 * @brief Manager for framework services
 *
 * The ServiceManager owns and provides access to all services in the framework.
 * Algorithms can retrieve services by name and type.
 */
class ServiceManager {
public:
    /**
     * @brief Add a service to the manager
     * @param service Unique pointer to the service
     * @return StatusCode indicating success or failure
     */
    auto add_service(std::unique_ptr<IService> service) -> StatusCode {
        const std::string& name = service->name();
        if (m_services.find(name) != m_services.end()) {
            fmt::print(stderr, "ERROR: Service '{}' already exists\n", name);
            return StatusCode::FAILURE();
        }
        m_services[name] = std::move(service);
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Retrieve a service by name
     * @tparam T Service type
     * @param name Name of the service
     * @return T* Pointer to the service, or nullptr if not found
     */
    template<typename T>
    auto get_service(const std::string& name) -> T* {
        auto it = m_services.find(name);
        if (it == m_services.end()) {
            fmt::print(stderr, "WARNING: Service '{}' not found\n", name);
            return nullptr;
        }
        return dynamic_cast<T*>(it->second.get());
    }

    /**
     * @brief Initialize all services
     * @return StatusCode indicating success or failure
     */
    auto initialize_all() -> StatusCode {
        for (auto& [name, service] : m_services) {
            fmt::print("Initializing service: {}\n", name);
            auto sc = service->initialize();
            if (sc.is_failure()) {
                fmt::print(stderr, "ERROR: Failed to initialize service '{}'\n", name);
                return StatusCode::FAILURE();
            }
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Finalize all services
     * @return StatusCode indicating success or failure
     */
    auto finalize_all() -> StatusCode {
        for (auto& [name, service] : m_services) {
            fmt::print("Finalizing service: {}\n", name);
            auto sc = service->finalize();
            if (sc.is_failure()) {
                fmt::print(stderr, "WARNING: Failed to finalize service '{}'\n", name);
            }
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
};

}  // namespace Framework

// Template implementation for Algorithm::service
namespace Framework {
    template<typename T>
    auto Algorithm::service(const std::string& service_name) -> T* {
        if (!m_svc_mgr) {
            fmt::print(stderr, "ERROR: ServiceManager not set for algorithm '{}'\n", m_name);
            return nullptr;
        }
        return m_svc_mgr->get_service<T>(service_name);
    }
}
