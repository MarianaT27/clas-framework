#pragma once

#include <memory>
#include <string>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

/**
 * @brief Service lifecycle states
 *
 * Tracks the current state of a service to prevent invalid state transitions
 * and enable health monitoring.
 */
enum class ServiceState : std::uint8_t {
    Created = 0,      ///< Service constructed but not yet initialized
    Initializing = 1, ///< Currently in initialize() call
    Ready = 2,        ///< Initialized and ready for use
    Running = 3,      ///< Active and processing (optional state for busy services)
    Finalizing = 4,   ///< Currently in finalize() call
    Finalized = 5,    ///< Finalized, no longer usable
    Error = 6         ///< Service encountered an error
};

/**
 * @brief Base interface for all services in the framework
 *
 * Services provide functionality that can be accessed by multiple algorithms,
 * such as particle identification, histogram management, calibrations, etc.
 * This follows the service-oriented architecture pattern used in Gaudi/Athena.
 *
 * Lifecycle:
 * 1. Construction -> Created
 * 2. initialize() -> Initializing -> Ready
 * 3. (optional) on_start() -> Running
 * 4. (optional) on_stop() -> Ready
 * 5. finalize() -> Finalizing -> Finalized
 */
class IService {
   public:
    virtual ~IService() = default;

    /**
     * @brief Initialize the service
     * @return StatusCode indicating success or failure
     */
    virtual auto initialize() -> StatusCode = 0;

    /**
     * @brief Finalize the service (cleanup)
     * @return StatusCode indicating success or failure
     */
    virtual auto finalize() -> StatusCode = 0;

    /**
     * @brief Get the service name
     * @return const std::string& Service name
     */
    virtual auto name() const -> const std::string& = 0;

    /**
     * @brief Get the current service state
     * @return ServiceState Current lifecycle state
     */
    [[nodiscard]] virtual auto state() const -> ServiceState = 0;

    /**
     * @brief Check if the service is healthy and operational
     *
     * Default implementation returns SUCCESS if state is Ready or Running.
     * Override to add custom health checks.
     *
     * @return StatusCode SUCCESS if healthy, FAILURE with diagnostic message otherwise
     */
    [[nodiscard]] virtual auto health_check() const -> StatusCode {
        auto s = state();
        if (s == ServiceState::Ready || s == ServiceState::Running) {
            return StatusCode::SUCCESS();
        }
        return StatusCode::failure("Service not in ready state");
    }

    /**
     * @brief Check if service is in a usable state
     * @return bool True if Ready or Running
     */
    [[nodiscard]] virtual auto is_ready() const -> bool {
        auto s = state();
        return s == ServiceState::Ready || s == ServiceState::Running;
    }

    /**
     * @brief Optional hook called when service should start active processing
     *
     * Override for services that have an active running mode distinct from
     * just being ready (e.g., background threads, event loops).
     *
     * @return StatusCode SUCCESS to continue, FAILURE to abort
     */
    virtual auto on_start() -> StatusCode { return StatusCode::SUCCESS(); }

    /**
     * @brief Optional hook called when service should stop active processing
     *
     * Override for services that have an active running mode.
     *
     * @return StatusCode SUCCESS or FAILURE
     */
    virtual auto on_stop() -> StatusCode { return StatusCode::SUCCESS(); }
};

/**
 * @brief Base class for service implementations
 *
 * Provides default implementations with state tracking.
 * Derived services should call set_state() appropriately or
 * call the base class methods.
 */
class Service : public IService {
   public:
    explicit Service(std::string name) : m_name(std::move(name)) {}

    auto name() const -> const std::string& override { return m_name; }

    /**
     * @brief Get the current service state
     * @return ServiceState Current lifecycle state
     */
    [[nodiscard]] auto state() const -> ServiceState override {
        return m_state;
    }

    /**
     * @brief Initialize the service with state tracking
     *
     * Sets state to Initializing, calls on_initialize(), then sets state to Ready.
     * Override on_initialize() for custom initialization logic.
     *
     * @return StatusCode indicating success or failure
     */
    auto initialize() -> StatusCode override {
        if (m_state != ServiceState::Created) {
            return StatusCode::failure("Service already initialized");
        }

        m_state = ServiceState::Initializing;
        auto result = on_initialize();

        if (result.is_success()) {
            m_state = ServiceState::Ready;
        } else {
            m_state = ServiceState::Error;
        }

        return result;
    }

    /**
     * @brief Finalize the service with state tracking
     *
     * Sets state to Finalizing, calls on_finalize(), then sets state to Finalized.
     * Override on_finalize() for custom finalization logic.
     *
     * @return StatusCode indicating success or failure
     */
    auto finalize() -> StatusCode override {
        if (m_state != ServiceState::Ready && m_state != ServiceState::Running) {
            return StatusCode::failure("Service not in ready state for finalization");
        }

        m_state = ServiceState::Finalizing;
        auto result = on_finalize();
        m_state = ServiceState::Finalized;

        return result;
    }

    /**
     * @brief Set the service locator (called by ServiceManager before initialize)
     *
     * Allows services to discover other services during initialization
     * without requiring manual wiring in ApplicationBuilder.
     */
    auto set_service_manager(class ServiceManager* mgr) -> void {
        m_svc_mgr = mgr;
    }

   protected:
    std::string m_name;
    ServiceState m_state = ServiceState::Created;
    class ServiceManager* m_svc_mgr = nullptr;

    /**
     * @brief Locate another service by name (available during and after initialize)
     *
     * @tparam T Service type
     * @param name Service name
     * @return T* Pointer to the service, or nullptr if not found
     */
    template <typename T>
    auto locate_service(const std::string& name) -> T* {
        if (!m_svc_mgr) return nullptr;
        // Forward declaration prevents direct call; implemented after ServiceManager definition
        return locate_service_impl<T>(name);
    }

    /**
     * @brief Set the service state
     *
     * Called by derived classes to update state.
     *
     * @param new_state The new state to set
     */
    auto set_state(ServiceState new_state) -> void {
        m_state = new_state;
    }

   private:
    template <typename T>
    auto locate_service_impl(const std::string& name) -> T*;

    /**
     * @brief Custom initialization hook (override in derived classes)
     *
     * Called by initialize() after state is set to Initializing.
     * Override this instead of initialize() for custom initialization.
     *
     * @return StatusCode SUCCESS to continue, FAILURE to abort
     */
    virtual auto on_initialize() -> StatusCode {
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Custom finalization hook (override in derived classes)
     *
     * Called by finalize() after state is set to Finalizing.
     * Override this instead of finalize() for custom finalization.
     *
     * @return StatusCode SUCCESS or FAILURE
     */
    virtual auto on_finalize() -> StatusCode {
        return StatusCode::SUCCESS();
    }
};

}  // namespace clas12::framework
