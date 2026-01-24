#pragma once

#include <map>
#include <memory>
#include <string>
#include <clas12/framework/core/Concepts.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

// Forward declarations
class Event;
class ServiceManager;
template <ServiceConcept T>
class ServiceHandle;

/**
 * @brief Base interface for all algorithms in the framework
 *
 * Algorithms are the building blocks of analysis. They process events
 * and can access services for common functionality. This follows the
 * algorithm framework pattern used in Gaudi/Athena/CMSSW.
 *
 * Lifecycle:
 * 1. Construction
 * 2. initialize() - Called once at the beginning
 * 3. execute() - Called for each event
 * 4. finalize() - Called once at the end
 */
class IAlgorithm {
   public:
    virtual ~IAlgorithm() = default;

    /**
     * @brief Initialize the algorithm
     *
     * Called once before event processing begins. Use this to book
     * histograms, retrieve services, and set up resources.
     *
     * @return StatusCode indicating success or failure
     */
    virtual auto initialize() -> StatusCode = 0;

    /**
     * @brief Execute the algorithm for one event
     *
     * Called for each event in the input file(s). Perform event selection,
     * fill histograms, and any per-event calculations here.
     *
     * @param event The current event data
     * @return StatusCode indicating success or failure
     */
    virtual auto execute(Event& event) -> StatusCode = 0;

    /**
     * @brief Finalize the algorithm
     *
     * Called once after all events have been processed. Use this for
     * cleanup, printing statistics, and writing output.
     *
     * @return StatusCode indicating success or failure
     */
    virtual auto finalize() -> StatusCode = 0;

    /**
     * @brief Get the algorithm name
     * @return const std::string& Algorithm name
     */
    virtual auto name() const -> const std::string& = 0;

    /**
     * @brief Set the service manager for this algorithm
     * @param svc_mgr Pointer to the service manager
     */
    virtual auto set_service_manager(ServiceManager* svc_mgr) -> void = 0;
};

/**
 * @brief Base class for algorithm implementations
 */
class Algorithm : public IAlgorithm {
   public:
    explicit Algorithm(std::string name) : m_name(std::move(name)) {}

    auto name() const -> const std::string& override { return m_name; }

    auto set_service_manager(ServiceManager* svc_mgr) -> void override {
        m_svc_mgr = svc_mgr;
    }

   protected:
    std::string m_name;
    ServiceManager* m_svc_mgr = nullptr;

    /**
     * @brief Retrieve a service by name
     *
     * The template parameter is constrained by ServiceConcept, ensuring
     * at compile time that only valid service types can be requested.
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param service_name Name of the service
     * @return T* Pointer to the service, or nullptr if not found
     * @see ServiceConcept
     */
    template <ServiceConcept T>
    auto service(const std::string& service_name) -> T*;

    /**
     * @brief Initialize a service handle
     *
     * Helper method to initialize a ServiceHandle during algorithm initialization.
     * Reports errors with the algorithm name for better diagnostics.
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param handle The service handle to initialize
     * @return StatusCode SUCCESS if handle was initialized, FAILURE with message otherwise
     * @see ServiceConcept
     */
    template <ServiceConcept T>
    auto initialize_handle(ServiceHandle<T>& handle) -> StatusCode;
};

}  // namespace clas12::framework
