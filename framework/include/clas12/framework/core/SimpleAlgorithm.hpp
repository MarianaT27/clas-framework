#pragma once

/**
 * @file SimpleAlgorithm.hpp
 * @brief Simplified algorithm base classes with reduced boilerplate
 *
 * This file provides CRTP-based algorithm base classes that auto-generate
 * common algorithm methods, reducing boilerplate and allowing developers
 * to focus on physics logic rather than framework mechanics.
 *
 * @see Requirements 4.1, 4.2, 4.3
 */

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/Concepts.hpp>
#include <clas12/framework/core/IAlgorithm.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceHandle.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

// Forward declaration
class Event;

/**
 * @brief Base interface for type-erased service handle storage
 *
 * This interface allows SimpleAlgorithm to store and initialize
 * service handles of different types uniformly.
 */
class IServiceHandleBase {
public:
    virtual ~IServiceHandleBase() = default;
    
    /**
     * @brief Initialize the service handle from the service manager
     * @param svc_mgr Reference to the service manager
     * @return StatusCode SUCCESS if initialized, FAILURE otherwise
     */
    virtual auto initialize(ServiceManager& svc_mgr) -> StatusCode = 0;
    
    /**
     * @brief Get the service name
     * @return const std::string& Service name
     */
    [[nodiscard]] virtual auto name() const -> const std::string& = 0;
    
    /**
     * @brief Check if the handle is valid
     * @return bool True if service was found
     */
    [[nodiscard]] virtual auto is_valid() const -> bool = 0;
};

/**
 * @brief Type-erased wrapper for ServiceHandle<T>
 *
 * Allows storing service handles of different types in a container
 * while preserving the ability to initialize them.
 *
 * @tparam T Service type (must satisfy ServiceConcept)
 */
template <ServiceConcept T>
class ServiceHandleWrapper : public IServiceHandleBase {
public:
    explicit ServiceHandleWrapper(std::string service_name)
        : m_handle(std::move(service_name)) {}
    
    auto initialize(ServiceManager& svc_mgr) -> StatusCode override {
        return m_handle.initialize(svc_mgr);
    }
    
    [[nodiscard]] auto name() const -> const std::string& override {
        return m_handle.name();
    }
    
    [[nodiscard]] auto is_valid() const -> bool override {
        return m_handle.is_valid();
    }
    
    /**
     * @brief Get the underlying service handle
     * @return ServiceHandle<T>& Reference to the handle
     */
    auto handle() -> ServiceHandle<T>& {
        return m_handle;
    }
    
    /**
     * @brief Get the service pointer
     * @return T* Pointer to the service
     */
    auto get() -> T* {
        return m_handle.get();
    }

private:
    ServiceHandle<T> m_handle;
};


/**
 * @brief CRTP base class for simplified algorithm implementations
 *
 * SimpleAlgorithm provides a simplified interface for writing algorithms
 * with automatic service injection and reduced boilerplate. Algorithms
 * inherit from this class using the CRTP pattern.
 *
 * The derived class should:
 * - Override configure() to declare services and properties
 * - Override process() to implement per-event logic
 * - Optionally override on_initialize() and on_finalize() for custom setup/cleanup
 *
 * Example usage:
 * @code
 * class MyAnalysis : public SimpleAlgorithm<MyAnalysis> {
 * public:
 *     MyAnalysis() : SimpleAlgorithm("MyAnalysis") {
 *         // Declare properties in constructor
 *         declare_property(m_momentum_cut);
 *     }
 *
 *     auto configure() -> void override {
 *         // Declare required services
 *         m_hist_svc = &require_service<HistogramService>("HistogramService");
 *     }
 *
 *     auto process(Event& event) -> StatusCode override {
 *         // Per-event processing logic
 *         for (const auto& electron : event.particles(clas12::PIDs::ELECTRON)) {
 *             if (electron.p() > m_momentum_cut) {
 *                 m_hist_svc->get()->fill("electron_p", electron.p());
 *             }
 *         }
 *         return StatusCode::SUCCESS();
 *     }
 *
 * private:
 *     Property<double> m_momentum_cut{"momentum_cut", 2.0, "Minimum momentum [GeV]"};
 *     ServiceHandle<HistogramService>* m_hist_svc = nullptr;
 * };
 * @endcode
 *
 * @tparam Derived The derived class (CRTP pattern)
 *
 * @see Requirement 4.1
 */
template <typename Derived>
class SimpleAlgorithm : public Algorithm, public PropertyMixin<Derived> {
public:
    /**
     * @brief Construct a SimpleAlgorithm with the given name
     * @param name Algorithm name
     */
    explicit SimpleAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Configure the algorithm (override in derived class)
     *
     * Called during initialization before services are injected.
     * Use this to declare required services via require_service<T>().
     */
    virtual auto configure() -> void {}

    /**
     * @brief Process a single event (must override in derived class)
     *
     * This is the main per-event processing method. Implement your
     * physics analysis logic here.
     *
     * @param event The current event data
     * @return StatusCode SUCCESS to continue, FILTERED to skip event, FAILURE to stop
     */
    virtual auto process(Event& event) -> StatusCode = 0;

    /**
     * @brief Custom initialization hook (optional override)
     *
     * Called after services are injected but before event processing begins.
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
     * It calls configure(), injects services, and then calls on_initialize().
     *
     * @return StatusCode indicating success or failure
     */
    auto initialize() -> StatusCode final {
        // Validate state transition
        auto state_check = this->validate_state_transition(AlgorithmState::Initialized);
        if (state_check.is_failure()) {
            return state_check;
        }

        // Step 1: Call configure() to let derived class declare services
        static_cast<Derived*>(this)->configure();

        // Step 2: Auto-inject all declared required services
        for (auto& [name, handle] : m_required_service_handles) {
            if (!m_svc_mgr) {
                return StatusCode::failure(
                    fmt::format("Algorithm '{}': ServiceManager not set", m_name));
            }

            auto sc = handle->initialize(*m_svc_mgr);
            if (sc.is_failure()) {
                return StatusCode::failure(
                    fmt::format("Algorithm '{}': Failed to get required service '{}': {}",
                                m_name, name, sc.message()));
            }
        }

        // Step 2b: Auto-inject all declared optional services (don't fail on missing)
        for (auto& [name, handle] : m_optional_service_handles) {
            if (m_svc_mgr) {
                // Ignore failures for optional services
                (void)handle->initialize(*m_svc_mgr);
            }
        }

        // Step 3: Call on_initialize() for custom initialization
        auto result = static_cast<Derived*>(this)->on_initialize();

        // Update state on success
        if (result.is_success()) {
            this->set_state(AlgorithmState::Initialized);
        }

        return result;
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
        // Validate state transition
        auto state_check = this->validate_state_transition(AlgorithmState::Running);
        if (state_check.is_failure()) {
            return state_check;
        }

        // Track running state during execution
        auto previous_state = this->state();
        this->set_state(AlgorithmState::Running);

        auto result = static_cast<Derived*>(this)->process(event);

        // Restore to initialized state after processing
        this->set_state(previous_state);

        return result;
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
        // Validate state transition
        auto state_check = this->validate_state_transition(AlgorithmState::Finalized);
        if (state_check.is_failure()) {
            return state_check;
        }

        auto result = static_cast<Derived*>(this)->on_finalize();

        // Update state regardless of result
        this->set_state(AlgorithmState::Finalized);

        return result;
    }

protected:
    /**
     * @brief Declare a required service dependency
     *
     * Call this in configure() to declare services that this algorithm needs.
     * Services are automatically retrieved during initialize().
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param service_name Name of the service to retrieve
     * @return ServiceHandle<T>& Reference to the service handle
     *
     * @see Requirement 4.2
     */
    template <ServiceConcept T>
    auto require_service(const std::string& service_name) -> ServiceHandle<T>& {
        auto wrapper = std::make_unique<ServiceHandleWrapper<T>>(service_name);
        auto& handle = wrapper->handle();
        m_required_service_handles[service_name] = std::move(wrapper);
        return handle;
    }

    /**
     * @brief Declare an optional service dependency
     *
     * Call this in configure() to declare services that this algorithm can use
     * but does not require. If the service is not available, initialization
     * will still succeed but the handle will be invalid.
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param service_name Name of the service to retrieve
     * @return ServiceHandle<T>& Reference to the service handle (may be invalid after init)
     */
    template <ServiceConcept T>
    auto optional_service(const std::string& service_name) -> ServiceHandle<T>& {
        auto wrapper = std::make_unique<ServiceHandleWrapper<T>>(service_name);
        auto& handle = wrapper->handle();
        m_optional_service_handles[service_name] = std::move(wrapper);
        return handle;
    }

    /**
     * @brief Get the number of declared services (required + optional)
     * @return std::size_t Number of services
     */
    [[nodiscard]] auto service_count() const -> std::size_t {
        return m_required_service_handles.size() + m_optional_service_handles.size();
    }

    /**
     * @brief Check if a service with the given name is declared
     * @param name Service name
     * @return bool True if declared (as required or optional)
     */
    [[nodiscard]] auto has_service(const std::string& name) const -> bool {
        return m_required_service_handles.contains(name) ||
               m_optional_service_handles.contains(name);
    }

    /**
     * @brief Check if a required service with the given name is declared
     * @param name Service name
     * @return bool True if declared as required
     */
    [[nodiscard]] auto has_required_service(const std::string& name) const -> bool {
        return m_required_service_handles.contains(name);
    }

    /**
     * @brief Check if an optional service with the given name is declared
     * @param name Service name
     * @return bool True if declared as optional
     */
    [[nodiscard]] auto has_optional_service(const std::string& name) const -> bool {
        return m_optional_service_handles.contains(name);
    }

    /**
     * @brief Declare a required HIPO bank for this algorithm
     *
     * Call this in the constructor or configure() to declare banks that
     * this algorithm needs. The Application collects the union of all
     * declared banks and passes them to hipo::chain::process().
     *
     * @param bank_name HIPO bank name (e.g., "REC::Particle")
     */
    void declare_bank(std::string_view bank_name) {
        m_required_banks.emplace_back(bank_name);
    }

    /**
     * @brief Get the list of required HIPO banks
     * @return std::vector<std::string> Bank names declared via declare_bank()
     */
    [[nodiscard]] auto required_banks() const -> std::vector<std::string> override {
        return m_required_banks;
    }

    // ========================================================================
    // Common Service Accessors - Direct access without manual declaration
    // ========================================================================
    //
    // These methods provide direct access to commonly-used services without
    // requiring manual declaration in configure(). Simply call the accessor
    // in on_initialize() or process(). Returns nullptr if the service is
    // not available.
    //
    // Usage:
    // @code
    // #include <clas12/framework/services/HistogramService.hpp>
    //
    // auto on_initialize() -> StatusCode override {
    //     if (auto* svc = svc<HistogramService>()) {
    //         svc->book_hist1D("/Electron/p", "Momentum", 100, 0, 10);
    //     }
    //     return StatusCode::SUCCESS();
    // }
    // @endcode

    /**
     * @brief Get any service by type (convenience accessor)
     *
     * Looks up the service using its static service_name() method.
     * This eliminates the need to manually declare services in configure()
     * for simple use cases.
     *
     * @tparam T Service type (must satisfy ServiceConcept and have static service_name())
     * @return T* Pointer to the service, or nullptr if not available
     *
     * Usage:
     * @code
     * // Instead of declaring in configure() and storing in member:
     * auto* hist = svc<HistogramService>();
     * hist->book_hist1D("/Electron/p", "p", 100, 0, 10);
     *
     * auto* ntuple = svc<NTupleService>();
     * ntuple->register_branch("tree", "var", &m_var);
     * @endcode
     */
    template <ServiceConcept T>
    auto svc() -> T* {
        if (!m_svc_mgr) {
            return nullptr;
        }
        // Use static service_name() if available, otherwise use type name
        if constexpr (requires { T::service_name(); }) {
            return m_svc_mgr->get_service<T>(std::string(T::service_name()));
        } else {
            // Fallback: try common naming convention (type name)
            return nullptr;
        }
    }

    /**
     * @brief Get any service by name (convenience accessor)
     *
     * Allows accessing any service by name without prior declaration.
     * The service must be registered with the ServiceManager.
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param service_name Name of the service to retrieve
     * @return T* Pointer to the service, or nullptr if not available
     *
     * Usage:
     * @code
     * // Get a service by explicit name:
     * auto* hist = svc<HistogramService>("HistogramService");
     * auto* custom = svc<MyCustomService>("MyCustomService");
     * @endcode
     */
    template <ServiceConcept T>
    auto svc(const std::string& service_name) -> T* {
        if (!m_svc_mgr) {
            return nullptr;
        }
        return m_svc_mgr->get_service<T>(service_name);
    }

    /**
     * @brief Get a required service by type (throws if not available)
     *
     * Similar to svc<T>() but throws an exception if the service is not found.
     * Use this in on_initialize() for services that are mandatory.
     *
     * @tparam T Service type (must satisfy ServiceConcept and have service_name())
     * @return T& Reference to the service
     * @throws std::runtime_error if service is not available
     *
     * Usage:
     * @code
     * auto on_initialize() -> StatusCode override {
     *     auto& hist = require<HistogramService>();
     *     hist.book_hist1D("/p", "p", 100, 0, 10);
     *     return StatusCode::SUCCESS();
     * }
     * @endcode
     */
    template <ServiceConcept T>
    auto require() -> T& {
        auto* s = svc<T>();
        if (!s) {
            throw std::runtime_error(
                fmt::format("Algorithm '{}': Required service '{}' not available",
                            m_name, std::string(T::service_name())));
        }
        return *s;
    }

private:
    std::map<std::string, std::unique_ptr<IServiceHandleBase>> m_required_service_handles;
    std::map<std::string, std::unique_ptr<IServiceHandleBase>> m_optional_service_handles;
    std::vector<std::string> m_required_banks;
};


/**
 * @brief Lambda-based algorithm for simple cases
 *
 * LambdaAlgorithm allows defining simple algorithms using lambda functions
 * without creating a full class. This is useful for quick prototyping or
 * simple event filters.
 *
 * Example usage:
 * @code
 * auto electron_filter = make_algorithm("ElectronFilter",
 *     [](Event& event) -> StatusCode {
 *         auto electrons = event.particles(clas12::PIDs::ELECTRON);
 *         if (electrons.empty()) {
 *             return StatusCode::FILTERED();
 *         }
 *         return StatusCode::SUCCESS();
 *     });
 *
 * app.add_algorithm(std::move(electron_filter));
 * @endcode
 *
 * @see Requirement 4.3
 */
class LambdaAlgorithm : public Algorithm {
public:
    /**
     * @brief Function type for the process callback
     */
    using ProcessFunc = std::function<StatusCode(Event&)>;
    
    /**
     * @brief Function type for optional initialize callback
     */
    using InitFunc = std::function<StatusCode()>;
    
    /**
     * @brief Function type for optional finalize callback
     */
    using FinalizeFunc = std::function<StatusCode()>;

    /**
     * @brief Construct a LambdaAlgorithm with name and process function
     *
     * @param name Algorithm name
     * @param process_func Function to call for each event
     */
    LambdaAlgorithm(std::string name, ProcessFunc process_func)
        : Algorithm(std::move(name))
        , m_process(std::move(process_func)) {}

    /**
     * @brief Construct a LambdaAlgorithm with all callbacks
     *
     * @param name Algorithm name
     * @param process_func Function to call for each event
     * @param init_func Function to call during initialization
     * @param finalize_func Function to call during finalization
     */
    LambdaAlgorithm(std::string name, ProcessFunc process_func,
                    InitFunc init_func, FinalizeFunc finalize_func)
        : Algorithm(std::move(name))
        , m_process(std::move(process_func))
        , m_init(std::move(init_func))
        , m_finalize(std::move(finalize_func)) {}

    /**
     * @brief Initialize the algorithm
     * @return StatusCode from init function, or SUCCESS if not set
     */
    auto initialize() -> StatusCode override {
        // Validate state transition
        auto state_check = validate_state_transition(AlgorithmState::Initialized);
        if (state_check.is_failure()) {
            return state_check;
        }

        StatusCode result = StatusCode::SUCCESS();
        if (m_init) {
            result = m_init();
        }

        if (result.is_success()) {
            set_state(AlgorithmState::Initialized);
        }

        return result;
    }

    /**
     * @brief Execute the algorithm for one event
     * @param event The current event data
     * @return StatusCode from process function
     */
    auto execute(Event& event) -> StatusCode override {
        // Validate state transition
        auto state_check = validate_state_transition(AlgorithmState::Running);
        if (state_check.is_failure()) {
            return state_check;
        }

        auto previous_state = state();
        set_state(AlgorithmState::Running);

        auto result = m_process(event);

        set_state(previous_state);

        return result;
    }

    /**
     * @brief Finalize the algorithm
     * @return StatusCode from finalize function, or SUCCESS if not set
     */
    auto finalize() -> StatusCode override {
        // Validate state transition
        auto state_check = validate_state_transition(AlgorithmState::Finalized);
        if (state_check.is_failure()) {
            return state_check;
        }

        StatusCode result = StatusCode::SUCCESS();
        if (m_finalize) {
            result = m_finalize();
        }

        set_state(AlgorithmState::Finalized);

        return result;
    }

    /**
     * @brief Set the initialization function
     * @param func Initialization function
     * @return LambdaAlgorithm& Reference to this for chaining
     */
    auto with_init(InitFunc func) -> LambdaAlgorithm& {
        m_init = std::move(func);
        return *this;
    }

    /**
     * @brief Set the finalization function
     * @param func Finalization function
     * @return LambdaAlgorithm& Reference to this for chaining
     */
    auto with_finalize(FinalizeFunc func) -> LambdaAlgorithm& {
        m_finalize = std::move(func);
        return *this;
    }

private:
    ProcessFunc m_process;
    InitFunc m_init;
    FinalizeFunc m_finalize;
};

/**
 * @brief Factory function to create a LambdaAlgorithm
 *
 * Creates a unique_ptr to a LambdaAlgorithm with the given name and
 * process function. This is the preferred way to create lambda-based
 * algorithms.
 *
 * @param name Algorithm name
 * @param process_func Function to call for each event
 * @return std::unique_ptr<LambdaAlgorithm> Pointer to the algorithm
 *
 * Example:
 * @code
 * auto alg = make_algorithm("MyFilter", [](Event& event) {
 *     return event.particles(clas12::PIDs::ELECTRON).empty()
 *         ? StatusCode::FILTERED()
 *         : StatusCode::SUCCESS();
 * });
 * @endcode
 *
 * @see Requirement 4.3
 */
inline auto make_algorithm(std::string name,
                           std::function<StatusCode(Event&)> process_func)
    -> std::unique_ptr<LambdaAlgorithm> {
    return std::make_unique<LambdaAlgorithm>(std::move(name), std::move(process_func));
}

/**
 * @brief Factory function to create a LambdaAlgorithm with all callbacks
 *
 * Creates a unique_ptr to a LambdaAlgorithm with initialization,
 * processing, and finalization functions.
 *
 * @param name Algorithm name
 * @param process_func Function to call for each event
 * @param init_func Function to call during initialization
 * @param finalize_func Function to call during finalization
 * @return std::unique_ptr<LambdaAlgorithm> Pointer to the algorithm
 */
inline auto make_algorithm(std::string name,
                           std::function<StatusCode(Event&)> process_func,
                           std::function<StatusCode()> init_func,
                           std::function<StatusCode()> finalize_func)
    -> std::unique_ptr<LambdaAlgorithm> {
    return std::make_unique<LambdaAlgorithm>(
        std::move(name), std::move(process_func),
        std::move(init_func), std::move(finalize_func));
}

}  // namespace clas12::framework
