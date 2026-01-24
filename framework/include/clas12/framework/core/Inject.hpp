#pragma once

/**
 * @file Inject.hpp
 * @brief Service dependency injection markers and InjectableAlgorithm base class
 *
 * This file provides automatic service injection for algorithms using
 * declarative dependency markers. Algorithms can declare service dependencies
 * as members, and they will be automatically injected during initialization.
 *
 * @see Requirements 12.1, 12.2, 12.3, 12.4
 */

#include <fmt/format.h>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/Concepts.hpp>
#include <clas12/framework/core/IAlgorithm.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

/**
 * @brief Base interface for type-erased injection markers
 *
 * This interface allows InjectableAlgorithm to store and resolve
 * injection markers of different service types uniformly.
 */
class IInjection {
public:
    virtual ~IInjection() = default;

    /**
     * @brief Resolve the service from the service manager
     * @param svc_mgr Reference to the service manager
     * @return StatusCode SUCCESS if resolved (or optional and missing), FAILURE if required and missing
     */
    virtual auto resolve(ServiceManager& svc_mgr) -> StatusCode = 0;

    /**
     * @brief Get the service name
     * @return const std::string& Service name
     */
    [[nodiscard]] virtual auto name() const -> const std::string& = 0;

    /**
     * @brief Check if this dependency is required
     * @return bool True if required, false if optional
     */
    [[nodiscard]] virtual auto is_required() const -> bool = 0;

    /**
     * @brief Check if the service was successfully resolved
     * @return bool True if service pointer is valid
     */
    [[nodiscard]] virtual auto is_resolved() const -> bool = 0;
};

/**
 * @brief Service dependency injection marker
 *
 * Inject<T> is used to declare service dependencies in algorithms.
 * When used with InjectableAlgorithm, services are automatically
 * retrieved during initialization.
 *
 * Features:
 * - Required dependencies (default): Initialization fails if service is missing
 * - Optional dependencies: Initialization continues with null service
 * - Type-safe access via operator-> and operator*
 * - Explicit bool conversion to check if service is available
 *
 * Example usage:
 * @code
 * class MyAlgorithm : public InjectableAlgorithm<MyAlgorithm> {
 * public:
 *     MyAlgorithm() : InjectableAlgorithm("MyAlgorithm") {
 *         // Register injections
 *         register_injection(m_hist_svc);
 *         register_injection(m_optional_svc);
 *     }
 *
 *     auto on_initialize() -> StatusCode override {
 *         // Services are already injected here
 *         if (m_hist_svc) {
 *             m_hist_svc->book_hist1D(...);
 *         }
 *         return StatusCode::SUCCESS();
 *     }
 *
 * private:
 *     Inject<HistogramService> m_hist_svc{"HistogramService"};  // Required
 *     Inject<OptionalService> m_optional_svc{"OptionalService", false};  // Optional
 * };
 * @endcode
 *
 * @tparam T Service type (must satisfy ServiceConcept)
 *
 * @see Requirement 12.1
 */
template <ServiceConcept T>
class Inject : public IInjection {
public:
    /**
     * @brief Construct a required injection marker
     * @param service_name Name of the service to inject
     */
    explicit Inject(std::string service_name)
        : m_name(std::move(service_name)), m_required(true) {}

    /**
     * @brief Construct an injection marker with required/optional flag
     * @param service_name Name of the service to inject
     * @param required True if service is required, false if optional
     */
    Inject(std::string service_name, bool required)
        : m_name(std::move(service_name)), m_required(required) {}

    /**
     * @brief Resolve the service from the service manager
     *
     * For required services, returns FAILURE if service is not found.
     * For optional services, returns SUCCESS even if service is not found.
     *
     * @param svc_mgr Reference to the service manager
     * @return StatusCode indicating success or failure
     */
    auto resolve(ServiceManager& svc_mgr) -> StatusCode override {
        m_service = svc_mgr.get_service<T>(m_name);

        if (!m_service && m_required) {
            return StatusCode::failure(
                fmt::format("Required service '{}' not found or wrong type", m_name));
        }

        return StatusCode::SUCCESS();
    }

    /**
     * @brief Get the service name
     * @return const std::string& Service name
     */
    [[nodiscard]] auto name() const -> const std::string& override {
        return m_name;
    }

    /**
     * @brief Check if this dependency is required
     * @return bool True if required
     */
    [[nodiscard]] auto is_required() const -> bool override {
        return m_required;
    }

    /**
     * @brief Check if the service was successfully resolved
     * @return bool True if service pointer is valid
     */
    [[nodiscard]] auto is_resolved() const -> bool override {
        return m_service != nullptr;
    }

    /**
     * @brief Arrow operator for accessing service methods
     * @return T* Pointer to the service
     * @note Behavior is undefined if service is not resolved
     */
    auto operator->() const -> T* {
        return m_service;
    }

    /**
     * @brief Dereference operator for accessing the service
     * @return T& Reference to the service
     * @note Behavior is undefined if service is not resolved
     */
    auto operator*() const -> T& {
        return *m_service;
    }

    /**
     * @brief Get raw pointer to the service
     * @return T* Pointer to the service, or nullptr if not resolved
     */
    [[nodiscard]] auto get() const -> T* {
        return m_service;
    }

    /**
     * @brief Boolean conversion operator
     *
     * Allows using Inject<T> in boolean contexts to check if
     * the service is available.
     *
     * @return bool True if service is resolved
     */
    explicit operator bool() const {
        return m_service != nullptr;
    }

private:
    std::string m_name;
    T* m_service = nullptr;
    bool m_required;
};


/**
 * @brief CRTP base class for algorithms with automatic service injection
 *
 * InjectableAlgorithm provides automatic service injection during initialization.
 * Algorithms inherit from this class using the CRTP pattern and register their
 * Inject<T> members for automatic resolution.
 *
 * The derived class should:
 * - Register Inject<T> members in the constructor using register_injection()
 * - Override process() to implement per-event logic
 * - Optionally override on_initialize() and on_finalize() for custom setup/cleanup
 *
 * Example usage:
 * @code
 * class MyAnalysis : public InjectableAlgorithm<MyAnalysis> {
 * public:
 *     MyAnalysis() : InjectableAlgorithm("MyAnalysis") {
 *         register_injection(m_hist_svc);
 *         register_injection(m_log_svc);
 *     }
 *
 *     auto process(Event& event) -> StatusCode override {
 *         m_hist_svc->fill("momentum", event.particles()[0].p());
 *         return StatusCode::SUCCESS();
 *     }
 *
 * private:
 *     Inject<HistogramService> m_hist_svc{"HistogramService"};
 *     Inject<LoggingService> m_log_svc{"LoggingService"};
 * };
 * @endcode
 *
 * @tparam Derived The derived class (CRTP pattern)
 *
 * @see Requirements 12.2, 12.3, 12.4
 */
template <typename Derived>
class InjectableAlgorithm : public Algorithm {
public:
    /**
     * @brief Construct an InjectableAlgorithm with the given name
     * @param name Algorithm name
     */
    explicit InjectableAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Process a single event (must override in derived class)
     *
     * This is the main per-event processing method. Implement your
     * physics analysis logic here. All registered services are
     * guaranteed to be injected before this is called.
     *
     * @param event The current event data
     * @return StatusCode SUCCESS to continue, FILTERED to skip event, FAILURE to stop
     */
    virtual auto process(Event& event) -> StatusCode = 0;

    /**
     * @brief Custom initialization hook (optional override)
     *
     * Called after all services are injected but before event processing begins.
     * Use this for booking histograms, setting up resources, etc.
     *
     * @return StatusCode SUCCESS to continue, FAILURE to abort
     */
    virtual auto on_initialize() -> StatusCode { return StatusCode::SUCCESS(); }

    /**
     * @brief Custom finalization hook (optional override)
     *
     * Called after all events have been processed.
     * Use this for cleanup, printing statistics, etc.
     *
     * @return StatusCode SUCCESS or FAILURE
     */
    virtual auto on_finalize() -> StatusCode { return StatusCode::SUCCESS(); }

    // ========================================================================
    // Framework methods - auto-implemented (final to prevent override)
    // ========================================================================

    /**
     * @brief Initialize the algorithm
     *
     * This method is auto-implemented and should not be overridden.
     * It injects all registered services and then calls on_initialize().
     *
     * @return StatusCode indicating success or failure
     */
    auto initialize() -> StatusCode final {
        // Step 1: Inject all registered services
        auto sc = inject_services();
        if (sc.is_failure()) {
            return sc;
        }

        // Step 2: Call on_initialize() for custom initialization
        return static_cast<Derived*>(this)->on_initialize();
    }

    /**
     * @brief Execute the algorithm for one event
     *
     * This method is auto-implemented and should not be overridden.
     * It delegates to process().
     *
     * @param event The current event data
     * @return StatusCode from process()
     */
    auto execute(Event& event) -> StatusCode final {
        return static_cast<Derived*>(this)->process(event);
    }

    /**
     * @brief Finalize the algorithm
     *
     * This method is auto-implemented and should not be overridden.
     * It delegates to on_finalize().
     *
     * @return StatusCode from on_finalize()
     */
    auto finalize() -> StatusCode final {
        return static_cast<Derived*>(this)->on_finalize();
    }

protected:
    /**
     * @brief Register an injection marker for automatic resolution
     *
     * Call this in the constructor for each Inject<T> member that
     * should be automatically resolved during initialization.
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param injection Reference to the Inject<T> member
     */
    template <ServiceConcept T>
    auto register_injection(Inject<T>& injection) -> void {
        m_injections.push_back(&injection);
    }

    /**
     * @brief Get the number of registered injections
     * @return std::size_t Number of injections
     */
    [[nodiscard]] auto injection_count() const -> std::size_t {
        return m_injections.size();
    }

private:
    std::vector<IInjection*> m_injections;

    /**
     * @brief Inject all registered services
     *
     * Iterates through all registered Inject<T> markers and resolves
     * them from the service manager. For required services, returns
     * FAILURE if the service is not found. For optional services,
     * continues even if the service is not found.
     *
     * @return StatusCode SUCCESS if all required services found, FAILURE otherwise
     */
    auto inject_services() -> StatusCode {
        if (!m_svc_mgr) {
            return StatusCode::failure(
                fmt::format("Algorithm '{}': ServiceManager not set", m_name));
        }

        for (auto* injection : m_injections) {
            auto sc = injection->resolve(*m_svc_mgr);
            if (sc.is_failure()) {
                // Only fail for required services
                if (injection->is_required()) {
                    return StatusCode::failure(
                        fmt::format("Algorithm '{}': {}", m_name, sc.message()));
                }
                // For optional services, log warning but continue
                // (logging would be done by the resolve method)
            }
        }

        return StatusCode::SUCCESS();
    }
};

}  // namespace clas12::framework
