#pragma once

#include <fmt/format.h>
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>
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
 * @brief Algorithm lifecycle states
 *
 * Tracks the current state of an algorithm to prevent invalid state transitions.
 * - Uninitialized: Algorithm created but not yet initialized
 * - Initialized: initialize() called successfully, ready to process events
 * - Running: Currently executing (within execute() call)
 * - Finalized: finalize() called, algorithm is done
 */
enum class AlgorithmState : std::uint8_t {
    Uninitialized = 0,  ///< Algorithm created, not yet initialized
    Initialized = 1,    ///< Ready to process events
    Running = 2,        ///< Currently processing an event
    Finalized = 3       ///< Finalized, no more processing allowed
};

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
 *
 * THREAD SAFETY:
 * Application invokes execute() concurrently from N worker threads on the
 * same algorithm instance (see Application::run -> hipo::chain::process).
 * Derived classes MUST ensure all mutable member state accessed from
 * execute()/process() is either:
 *   1. atomic (std::atomic<T>, AtomicHistogram, atomic Cutflow counters)
 *   2. mutex-guarded
 *   3. thread-local (one instance per worker via thread_local storage)
 *   4. append-only after initialize() (e.g. configuration, ServiceHandle)
 * initialize() and finalize() are serial; execute()/process() is concurrent.
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

    /**
     * @brief Get the current algorithm state
     * @return AlgorithmState Current lifecycle state
     */
    [[nodiscard]] virtual auto state() const -> AlgorithmState = 0;

    /**
     * @brief Check if algorithm is in a valid state to process events
     * @return bool True if initialized and not finalized
     */
    [[nodiscard]] virtual auto is_ready() const -> bool {
        auto s = state();
        return s == AlgorithmState::Initialized || s == AlgorithmState::Running;
    }

    /**
     * @brief Get the list of HIPO bank names required by this algorithm
     *
     * Used by the Application to collect the union of all required banks
     * before starting event processing with hipo::chain::process().
     *
     * @return std::vector<std::string> Bank names (e.g., "REC::Particle")
     */
    [[nodiscard]] virtual auto required_banks() const -> std::vector<std::string> { return {}; }
};

/**
 * @brief Base class for algorithm implementations
 *
 * Provides state tracking and service access for algorithms.
 * Derived classes should call set_state() to update the lifecycle state.
 */
class Algorithm : public IAlgorithm {
   public:
    explicit Algorithm(std::string name) : m_name(std::move(name)) {}

    auto name() const -> const std::string& override { return m_name; }

    auto set_service_manager(ServiceManager* svc_mgr) -> void override {
        m_svc_mgr = svc_mgr;
    }

    /**
     * @brief Get the current algorithm state
     * @return AlgorithmState Current lifecycle state
     */
    [[nodiscard]] auto state() const -> AlgorithmState override {
        return m_state.load(std::memory_order_acquire);
    }

   protected:
    std::string m_name;
    ServiceManager* m_svc_mgr = nullptr;
    // Atomic because execute() runs concurrently on many worker threads and
    // some call paths observe state() to gate work. Transitions only happen
    // from the serial initialize()/finalize() stages.
    std::atomic<AlgorithmState> m_state{AlgorithmState::Uninitialized};

    /**
     * @brief Set the algorithm state
     *
     * Called by derived classes to update state during lifecycle transitions.
     *
     * @param new_state The new state to set
     */
    auto set_state(AlgorithmState new_state) -> void {
        m_state.store(new_state, std::memory_order_release);
    }

    /**
     * @brief Validate state transition
     *
     * Checks if transitioning to the given state is valid from the current state.
     * State validation is permissive for backward compatibility:
     * - Initialize can only be called once (from Uninitialized)
     * - Execute requires Initialized or Running state
     * - Finalize can be called from any state (for cleanup safety)
     *
     * @param target_state The state we want to transition to
     * @return StatusCode SUCCESS if valid, FAILURE with message if invalid
     */
    [[nodiscard]] auto validate_state_transition(AlgorithmState target_state) const -> StatusCode {
        const auto current = m_state.load(std::memory_order_acquire);
        switch (target_state) {
            case AlgorithmState::Initialized:
                if (current != AlgorithmState::Uninitialized) {
                    return StatusCode::failure(
                        fmt::format("Algorithm '{}': Cannot initialize from state {}",
                                    m_name, static_cast<int>(current)));
                }
                break;
            case AlgorithmState::Running:
                if (current != AlgorithmState::Initialized && current != AlgorithmState::Running) {
                    return StatusCode::failure(
                        fmt::format("Algorithm '{}': Cannot execute from state {}",
                                    m_name, static_cast<int>(current)));
                }
                break;
            case AlgorithmState::Finalized:
                // Allow finalize from any state for backward compatibility
                // and safe cleanup even if initialization failed
                break;
            default:
                break;
        }
        return StatusCode::SUCCESS();
    }

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
