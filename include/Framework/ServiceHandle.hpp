#pragma once

#include <fmt/format.h>
#include <string>
#include "Concepts.hpp"
#include "StatusCode.hpp"

namespace Framework {

// Forward declaration
class ServiceManager;

/**
 * @brief Typed handle for accessing framework services
 *
 * ServiceHandle provides type-safe access to services with validation
 * at initialization time. This follows the pattern used in Gaudi/Athena
 * where service handles are declared as members and initialized during
 * algorithm initialization.
 *
 * The template parameter is constrained by ServiceConcept, which ensures
 * at compile time that only valid service types can be used. This provides
 * clear error messages when incompatible types are used.
 *
 * Usage:
 * @code
 * class MyAlgorithm : public Algorithm {
 *     ServiceHandle<HistogramService> m_hist_svc{"HistogramService"};
 *
 *     auto initialize() -> StatusCode override {
 *         if (!m_hist_svc.initialize(*m_svc_mgr)) {
 *             return StatusCode::failure("Failed to get HistogramService");
 *         }
 *         // Use service via m_hist_svc->method()
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 *
 * @tparam T The service type (must satisfy ServiceConcept)
 * @see ServiceConcept
 * @see Requirements 1.3
 */
template <ServiceConcept T>
class ServiceHandle {
   public:
    /**
     * @brief Construct a service handle with the given service name
     * @param name Name of the service to retrieve
     */
    explicit ServiceHandle(std::string name) : m_name(std::move(name)) {}

    /**
     * @brief Default constructor (requires later call to set_name)
     */
    ServiceHandle() = default;

    /**
     * @brief Set the service name (for default-constructed handles)
     * @param name Name of the service to retrieve
     */
    auto set_name(std::string name) -> void {
        m_name = std::move(name);
    }

    /**
     * @brief Get the service name
     * @return const std::string& The service name
     */
    auto name() const -> const std::string& {
        return m_name;
    }

    /**
     * @brief Initialize the handle by retrieving the service from ServiceManager
     *
     * This must be called during algorithm initialization. It retrieves the
     * service from the ServiceManager and validates that it exists and is
     * of the correct type.
     *
     * @param svc_mgr Reference to the ServiceManager
     * @return StatusCode SUCCESS if service was found, FAILURE with message otherwise
     */
    auto initialize(ServiceManager& svc_mgr) -> StatusCode;

    /**
     * @brief Arrow operator for accessing service methods
     * @return T* Pointer to the service
     * @note Behavior is undefined if handle is not valid
     */
    auto operator->() const -> T* {
        return m_service;
    }

    /**
     * @brief Dereference operator for accessing the service
     * @return T& Reference to the service
     * @note Behavior is undefined if handle is not valid
     */
    auto operator*() const -> T& {
        return *m_service;
    }

    /**
     * @brief Get raw pointer to the service
     * @return T* Pointer to the service, or nullptr if not initialized
     */
    auto get() const -> T* {
        return m_service;
    }

    /**
     * @brief Check if the handle is valid (service was found)
     * @return bool True if service pointer is valid
     */
    auto is_valid() const -> bool {
        return m_service != nullptr;
    }

    /**
     * @brief Boolean conversion operator
     * @return bool True if handle is valid
     */
    explicit operator bool() const {
        return is_valid();
    }

   private:
    std::string m_name;
    T* m_service = nullptr;
};

}  // namespace Framework

// Include ServiceManager for template implementation
#include "ServiceManager.hpp"

namespace Framework {

template <ServiceConcept T>
auto ServiceHandle<T>::initialize(ServiceManager& svc_mgr) -> StatusCode {
    if (m_name.empty()) {
        return StatusCode::failure("ServiceHandle: Service name is empty");
    }

    m_service = svc_mgr.get_service<T>(m_name);

    if (!m_service) {
        return StatusCode::failure(
            fmt::format("ServiceHandle: Service '{}' not found or wrong type", m_name));
    }

    return StatusCode::SUCCESS();
}

}  // namespace Framework
