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
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/Concepts.hpp>
#include <clas12/framework/core/IAlgorithm.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

namespace clas12::framework {

// Forward declaration
class Event;



/**
 * @brief Base class for simplified algorithm implementations
 *
 * SimpleAlgorithm provides a simplified interface for writing algorithms
 * with automatic service injection and reduced boilerplate. Derived classes
 * override the virtual hooks configure(), process(), on_initialize(), and
 * on_finalize().
 *
 * The derived class should:
 * - Override configure() to declare services and properties
 * - Override process() to implement per-event logic
 * - Optionally override on_initialize() and on_finalize() for custom setup/cleanup
 *
 * Example usage:
 * @code
 * class MyAnalysis : public SimpleAlgorithm {
 * public:
 *     MyAnalysis() : SimpleAlgorithm("MyAnalysis") {
 *         // Declare properties in constructor
 *         declare_property(m_momentum_cut);
 *     }
 *
 *     auto process(Event& event) -> StatusCode override {
 *         for (const auto& e : event.particles(clas12::PIDs::ELECTRON)) {
 *             if (e.p() > m_momentum_cut) {
 *                 m_hist_svc->fill("electron_p", e.p());
 *             }
 *         }
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 *
 * @note Previously a CRTP template. The CRTP parameter was dropped because
 *       Application dispatches through std::unique_ptr<IAlgorithm>, so
 *       every execute() call is already a virtual call: the
 *       static_cast<Derived*> shortcut had no codegen benefit and only
 *       caused template bloat.
 *
 * @see Requirement 4.1
 */
class SimpleAlgorithm : public Algorithm, public PropertyMixin {
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

        // Step 1: Call configure() to let derived class declare banks
        this->configure();

        // Step 2: Call on_initialize() for custom initialization
        auto result = this->on_initialize();

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
        // Validate state transition. Do NOT flip m_state here: execute() runs
        // concurrently on many worker threads and a store->restore dance on a
        // shared atomic would race with every sibling worker's own transition.
        // State transitions belong exclusively in initialize()/finalize(),
        // which are serial.
        auto state_check = this->validate_state_transition(AlgorithmState::Running);
        if (state_check.is_failure()) {
            return state_check;
        }

        return this->process(event);
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

        auto result = this->on_finalize();

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
     * @brief Declare a HIPO bank that this algorithm reads only if present
     *
     * Optional banks are included in event processing only if the input file's
     * schema dictionary contains them. On files that do not, Event::has_bank()
     * returns false and Event::get_bank() returns an empty view, so algorithms
     * can share one source tree between MC and real-data runs.
     *
     * @param bank_name HIPO bank name (e.g., "MC::Particle")
     */
    void declare_optional_bank(std::string_view bank_name) {
        m_optional_banks.emplace_back(bank_name);
    }

    /**
     * @brief Get the list of required HIPO banks
     * @return std::vector<std::string> Bank names declared via declare_bank()
     */
    [[nodiscard]] auto required_banks() const -> std::vector<std::string> override {
        return m_required_banks;
    }

    /**
     * @brief Get the list of optional HIPO banks
     * @return std::vector<std::string> Bank names declared via declare_optional_bank()
     */
    [[nodiscard]] auto optional_banks() const -> std::vector<std::string> override {
        return m_optional_banks;
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

    // ========================================================================
    // Data Contract (consumes/produces declarations)
    // ========================================================================

    /**
     * @brief Declare that this algorithm consumes a typed key from the event store
     *
     * Used for pipeline validation at initialization time.
     * Application warns if a consumed key is not produced by any earlier algorithm.
     *
     * @tparam T Value type
     * @param key The typed key to consume
     */
    template <typename T>
    void consumes(const TypedKey<T>& key) {
        m_consumed_keys.push_back(key.name);
    }

    /**
     * @brief Declare that this algorithm produces a typed key in the event store
     *
     * @tparam T Value type
     * @param key The typed key to produce
     */
    template <typename T>
    void produces(const TypedKey<T>& key) {
        m_produced_keys.push_back(key.name);
    }

    /**
     * @brief Get the list of consumed key names
     */
    [[nodiscard]] auto consumed_keys() const -> const std::vector<std::string>& {
        return m_consumed_keys;
    }

    /**
     * @brief Get the list of produced key names
     */
    [[nodiscard]] auto produced_keys() const -> const std::vector<std::string>& {
        return m_produced_keys;
    }

private:
    std::vector<std::string> m_required_banks;
    std::vector<std::string> m_optional_banks;
    std::vector<std::string> m_consumed_keys;
    std::vector<std::string> m_produced_keys;
};

}  // namespace clas12::framework
