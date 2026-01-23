#pragma once

#include <map>
#include <memory>
#include <string>

#include "Framework/Concepts.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/Property.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/ServiceManager.hpp"
#include "Framework/StatusCode.hpp"

// Forward declaration of ServiceHandleWrapper from SimpleAlgorithm.hpp
namespace Framework {
class IServiceHandleBase;
template <ServiceConcept T>
class ServiceHandleWrapper;
}  // namespace Framework

#include "Framework/SimpleAlgorithm.hpp"

namespace Framework {

/**
 * @brief Filter/Selection algorithm base class
 *
 * FilterAlgorithms make yes/no decisions about events. They should:
 * - Return true/false from filter()
 * - Set event decisions for downstream algorithms
 * - Be used for event selection, trigger simulation, etc.
 *
 * Example: Trigger electron selection, DIS event selection
 *
 * This follows the pattern from:
 * - ATLAS: IReentrantAlgorithm with filter mode
 * - CMS: EDFilter
 * - Gaudi: FilterAlgorithm
 */
class FilterAlgorithm : public Algorithm {
   public:
    explicit FilterAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Execute filter decision on event
     * @param event Event to process
     * @return StatusCode Operation status
     *
     * Calls the pure virtual filter() method and sets decision automatically.
     */
    auto execute(Event& event) -> StatusCode final override {
        bool passes = filter(event);

        // Automatically set decision with algorithm name
        event.set_decision(name(), passes);

        return StatusCode::SUCCESS();
    }

   protected:
    /**
     * @brief Filter decision logic
     * @param event Event to evaluate
     * @return true if event passes filter, false otherwise
     *
     * Implement this method in derived classes to define filter logic.
     */
    virtual auto filter(Event& event) -> bool = 0;
};

/**
 * @brief Producer algorithm base class
 *
 * ProducerAlgorithms create new data products from existing data. They should:
 * - Build composite particles (e.g., π⁰ from two photons)
 * - Reconstruct objects (e.g., jets, missing ET)
 * - Store products in the event store for downstream use
 *
 * Example: Neutral pion reconstruction, jet building
 *
 * This follows the pattern from:
 * - ATLAS: AlgTool pattern
 * - CMS: EDProducer
 * - Gaudi: GaudiAlgorithm with DataObjects
 */
class ProducerAlgorithm : public Algorithm {
   public:
    explicit ProducerAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Execute production on event
     * @param event Event to process
     * @return StatusCode Operation status
     *
     * Calls the pure virtual produce() method.
     */
    auto execute(Event& event) -> StatusCode final override {
        return produce(event);
    }

   protected:
    /**
     * @brief Production logic
     * @param event Event to process, add products to event store
     * @return StatusCode Operation status
     *
     * Implement this method to create and store new data products.
     * Use event.store<T>(key, product) to save results.
     */
    virtual auto produce(Event& event) -> StatusCode = 0;
};

/**
 * @brief Analysis algorithm base class
 *
 * AnalysisAlgorithms fill histograms and perform measurements. They should:
 * - Fill histograms using HistogramService
 * - Track cutflows
 * - Perform physics measurements
 * - Not modify event data (read-only)
 *
 * Example: Electron kinematics analysis, cross-section measurements
 *
 * This follows the pattern from:
 * - ATLAS: AthAnalysisAlgorithm
 * - CMS: EDAnalyzer
 * - Gaudi: GaudiAlgorithm (analysis mode)
 */
class AnalysisAlgorithm : public Algorithm {
   public:
    explicit AnalysisAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Execute analysis on event
     * @param event Event to analyze (should not be modified)
     * @return StatusCode Operation status
     *
     * Calls the pure virtual analyze() method.
     */
    auto execute(Event& event) -> StatusCode final override {
        return analyze(event);
    }

   protected:
    /**
     * @brief Analysis logic
     * @param event Event to analyze (read-only in principle)
     * @return StatusCode Operation status
     *
     * Implement this method to fill histograms and perform measurements.
     * Should not modify event data or set decisions (use FilterAlgorithm for that).
     */
    virtual auto analyze(Event& event) -> StatusCode = 0;
};

/**
 * @brief Monitoring algorithm base class
 *
 * MonitoringAlgorithms track performance and data quality. They should:
 * - Monitor processing rates
 * - Check data quality
 * - Track resource usage
 * - Report statistics
 *
 * Example: Event rate monitoring, detector occupancy checks
 *
 * This follows the pattern from:
 * - ATLAS: AthMonitorAlgorithm
 * - CMS: DQM modules
 * - Gaudi: MonitoringAlgorithm
 */
class MonitoringAlgorithm : public Algorithm {
   public:
    explicit MonitoringAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Execute monitoring on event
     * @param event Event to monitor
     * @return StatusCode Operation status
     *
     * Calls the pure virtual monitor() method.
     */
    auto execute(Event& event) -> StatusCode final override {
        return monitor(event);
    }

   protected:
    /**
     * @brief Monitoring logic
     * @param event Event to monitor
     * @return StatusCode Operation status
     *
     * Implement this method to track performance and quality metrics.
     */
    virtual auto monitor(Event& event) -> StatusCode = 0;
};

/**
 * @brief Sequence of algorithms that execute together
 *
 * AlgorithmSequence groups algorithms into logical units. All algorithms
 * in the sequence execute if the sequence's filter passes.
 *
 * Example: "ElectronSequence" = [ElectronFilter, ElectronAnalysis, ElectronMonitor]
 *
 * This follows the pattern from:
 * - ATLAS: AthSequencer
 * - CMS: Sequence
 * - Gaudi: GaudiSequencer
 *
 * C++ Core Guidelines:
 * - C.21: If you define or =delete any copy, move, or destructor function, define or =delete them all
 * - R.1: Manage resources automatically using resource handles and RAII
 */
class AlgorithmSequence : public Algorithm {
public:
    explicit AlgorithmSequence(std::string name) : Algorithm(std::move(name)) {}

    // Rule of five: default move, no copy (owns unique_ptrs)
    AlgorithmSequence(AlgorithmSequence&&) = default;
    AlgorithmSequence& operator=(AlgorithmSequence&&) = default;
    AlgorithmSequence(const AlgorithmSequence&) = delete;
    AlgorithmSequence& operator=(const AlgorithmSequence&) = delete;
    ~AlgorithmSequence() override = default;

    /// Add algorithm to sequence
    auto add_algorithm(std::unique_ptr<IAlgorithm> algorithm) -> AlgorithmSequence& {
        m_algorithms.push_back(std::move(algorithm));
        return *this;  // Enable chaining
    }

    /// Propagate service manager to all contained algorithms
    auto set_service_manager(ServiceManager* svc_mgr) -> void override {
        Algorithm::set_service_manager(svc_mgr);
        for (auto& alg : m_algorithms) {
            alg->set_service_manager(svc_mgr);
        }
    }

    /// Initialize all algorithms in sequence
    auto initialize() -> StatusCode override {
        for (auto& alg : m_algorithms) {
            if (auto sc = alg->initialize(); sc.is_failure()) {
                return sc;
            }
        }
        return StatusCode::SUCCESS();
    }

    /// Execute all algorithms in sequence
    auto execute(Event& event) -> StatusCode override {
        for (auto& alg : m_algorithms) {
            if (auto sc = alg->execute(event); sc.is_failure()) {
                return sc;
            }
            // Note: FILTERED status continues to next algorithm in sequence
        }
        return StatusCode::SUCCESS();
    }

    /// Finalize all algorithms in sequence
    auto finalize() -> StatusCode override {
        StatusCode result = StatusCode::SUCCESS();
        for (auto& alg : m_algorithms) {
            // Continue finalizing even if one fails (collect all errors)
            if (auto sc = alg->finalize(); sc.is_failure()) {
                result = sc;
            }
        }
        return result;
    }

    /// Get number of algorithms in sequence
    [[nodiscard]] auto size() const noexcept -> std::size_t {
        return m_algorithms.size();
    }

    /// Check if sequence is empty
    [[nodiscard]] auto empty() const noexcept -> bool {
        return m_algorithms.empty();
    }

    /// Access algorithms (for inspection)
    [[nodiscard]] auto algorithms() const -> const std::vector<std::unique_ptr<IAlgorithm>>& {
        return m_algorithms;
    }

private:
    std::vector<std::unique_ptr<IAlgorithm>> m_algorithms;
};

/**
 * @brief Conditional sequence that runs only if a decision passes
 *
 * ConditionalSequence wraps a sequence with a decision check.
 * All algorithms in the sequence are skipped if the decision fails.
 *
 * Example: Run pion analysis sequence only if "HasGoodElectron" is true
 *
 * C++ Core Guidelines:
 * - C.128: Virtual functions should specify exactly one of virtual, override, or final
 */
class ConditionalSequence : public AlgorithmSequence {
public:
    explicit ConditionalSequence(std::string name, std::string required_decision)
        : AlgorithmSequence(std::move(name)), m_required_decision(std::move(required_decision)) {}

    /// Execute sequence only if required decision passes
    auto execute(Event& event) -> StatusCode override {
        if (!event.passes_decision(m_required_decision)) {
            return StatusCode::SUCCESS();  // Skip sequence silently
        }
        return AlgorithmSequence::execute(event);
    }

    /// Get the required decision name
    [[nodiscard]] auto required_decision() const noexcept -> const std::string& {
        return m_required_decision;
    }

    /// Check if sequence would execute for given event
    [[nodiscard]] auto would_execute(Event& event) const -> bool {
        return event.passes_decision(m_required_decision);
    }

private:
    std::string m_required_decision;
};

/**
 * @brief Sequence that stops on first FILTERED status
 *
 * Unlike AlgorithmSequence which continues on FILTERED,
 * this sequence treats FILTERED as a signal to stop processing
 * remaining algorithms (useful for cut-based selection chains).
 */
class FilterSequence : public AlgorithmSequence {
public:
    using AlgorithmSequence::AlgorithmSequence;

    /// Execute algorithms, stopping on first FILTERED
    auto execute(Event& event) -> StatusCode override {
        for (auto& alg : algorithms()) {
            auto sc = alg->execute(event);
            if (sc.is_failure() || sc.is_filtered()) {
                return sc;  // Stop on failure OR filtered
            }
        }
        return StatusCode::SUCCESS();
    }
};

// ============================================================================
// CRTP-based simplified algorithm types with automatic service injection
// ============================================================================

/**
 * @brief CRTP base class for simplified filter algorithm implementations
 *
 * SimpleFilterAlgorithm provides a simplified interface for writing filter
 * algorithms with automatic service injection and reduced boilerplate.
 * Filters make yes/no decisions about events and automatically set
 * event decisions.
 *
 * The derived class should:
 * - Override configure() to declare services and properties
 * - Override filter() to implement the filter decision (return bool)
 * - Optionally override on_initialize() and on_finalize() for custom setup/cleanup
 *
 * Example usage:
 * @code
 * class ElectronFilter : public SimpleFilterAlgorithm<ElectronFilter> {
 * public:
 *     ElectronFilter() : SimpleFilterAlgorithm("ElectronFilter") {
 *         declare_property(m_min_momentum);
 *     }
 *
 *     auto configure() -> void override {
 *         m_cut_svc = &require_service<CutService>("CutService");
 *     }
 *
 *     auto filter(Event& event) -> bool override {
 *         auto electrons = event.particles(CLAS::PIDs::ELECTRON);
 *         return !electrons.empty() &&
 *                electrons.front().p() > m_min_momentum;
 *     }
 *
 * private:
 *     Property<double> m_min_momentum{"min_momentum", 2.0, "Minimum momentum [GeV]"};
 *     ServiceHandle<CutService>* m_cut_svc = nullptr;
 * };
 * @endcode
 *
 * @tparam Derived The derived class (CRTP pattern)
 */
template <typename Derived>
class SimpleFilterAlgorithm : public Algorithm, public PropertyMixin<Derived> {
public:
    /**
     * @brief Construct a SimpleFilterAlgorithm with the given name
     * @param name Algorithm name
     */
    explicit SimpleFilterAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Configure the algorithm (override in derived class)
     *
     * Called during initialization before services are injected.
     * Use this to declare required services via require_service<T>().
     */
    virtual auto configure() -> void {}

    /**
     * @brief Filter decision logic (must override in derived class)
     *
     * This is the main filter method. Return true if the event passes
     * the filter, false otherwise. The decision is automatically stored
     * in the event with this algorithm's name.
     *
     * @param event The current event data
     * @return bool True if event passes filter, false otherwise
     */
    virtual auto filter(Event& event) -> bool = 0;

    /**
     * @brief Custom initialization hook (optional override)
     *
     * Called after services are injected but before event processing begins.
     *
     * @return StatusCode SUCCESS to continue, FAILURE to abort
     */
    virtual auto on_initialize() -> StatusCode { return StatusCode::SUCCESS(); }

    /**
     * @brief Custom finalization hook (optional override)
     *
     * Called after all events have been processed.
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
        // Step 1: Call configure() to let derived class declare services
        static_cast<Derived*>(this)->configure();

        // Step 2: Auto-inject all declared services
        for (auto& [svc_name, handle] : m_service_handles) {
            if (!m_svc_mgr) {
                return StatusCode::failure(
                    fmt::format("Algorithm '{}': ServiceManager not set", m_name));
            }

            auto sc = handle->initialize(*m_svc_mgr);
            if (sc.is_failure()) {
                return StatusCode::failure(
                    fmt::format("Algorithm '{}': Failed to get service '{}': {}",
                                m_name, svc_name, sc.message()));
            }
        }

        // Step 3: Call on_initialize() for custom initialization
        return static_cast<Derived*>(this)->on_initialize();
    }

    /**
     * @brief Execute the filter for one event
     *
     * This method is auto-implemented and should not be overridden.
     * It delegates to filter() and automatically sets the event decision.
     *
     * @param event The current event data
     * @return StatusCode Always SUCCESS (filter result stored as decision)
     */
    auto execute(Event& event) -> StatusCode final {
        bool passes = static_cast<Derived*>(this)->filter(event);
        event.set_decision(name(), passes);
        return StatusCode::SUCCESS();
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
     * @brief Declare a required service dependency
     *
     * Call this in configure() to declare services that this algorithm needs.
     * Services are automatically retrieved during initialize().
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param service_name Name of the service to retrieve
     * @return ServiceHandle<T>& Reference to the service handle
     */
    template <ServiceConcept T>
    auto require_service(const std::string& service_name) -> ServiceHandle<T>& {
        auto wrapper = std::make_unique<ServiceHandleWrapper<T>>(service_name);
        auto& handle = wrapper->handle();
        m_service_handles[service_name] = std::move(wrapper);
        return handle;
    }

    /**
     * @brief Get the number of declared services
     * @return std::size_t Number of services
     */
    [[nodiscard]] auto service_count() const -> std::size_t {
        return m_service_handles.size();
    }

    /**
     * @brief Check if a service with the given name is declared
     * @param svc_name Service name
     * @return bool True if declared
     */
    [[nodiscard]] auto has_service(const std::string& svc_name) const -> bool {
        return m_service_handles.contains(svc_name);
    }

private:
    std::map<std::string, std::unique_ptr<IServiceHandleBase>> m_service_handles;
};


/**
 * @brief CRTP base class for simplified monitoring algorithm implementations
 *
 * SimpleMonitoringAlgorithm provides a simplified interface for writing monitoring
 * algorithms with automatic service injection and reduced boilerplate.
 * Monitoring algorithms track performance and data quality metrics.
 *
 * The derived class should:
 * - Override configure() to declare services and properties
 * - Override monitor() to implement monitoring logic
 * - Optionally override on_initialize() and on_finalize() for custom setup/cleanup
 *
 * Example usage:
 * @code
 * class EventRateMonitor : public SimpleMonitoringAlgorithm<EventRateMonitor> {
 * public:
 *     EventRateMonitor() : SimpleMonitoringAlgorithm("EventRateMonitor") {}
 *
 *     auto configure() -> void override {
 *         m_hist_svc = &require_service<HistogramService>("HistogramService");
 *     }
 *
 *     auto on_initialize() -> StatusCode override {
 *         m_hist_svc->get()->book("event_rate", 100, 0, 1000);
 *         return StatusCode::SUCCESS();
 *     }
 *
 *     auto monitor(Event& event) -> StatusCode override {
 *         ++m_event_count;
 *         m_hist_svc->get()->fill("event_rate", m_event_count);
 *         return StatusCode::SUCCESS();
 *     }
 *
 * private:
 *     ServiceHandle<HistogramService>* m_hist_svc = nullptr;
 *     std::size_t m_event_count = 0;
 * };
 * @endcode
 *
 * @tparam Derived The derived class (CRTP pattern)
 */
template <typename Derived>
class SimpleMonitoringAlgorithm : public Algorithm, public PropertyMixin<Derived> {
public:
    /**
     * @brief Construct a SimpleMonitoringAlgorithm with the given name
     * @param name Algorithm name
     */
    explicit SimpleMonitoringAlgorithm(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Configure the algorithm (override in derived class)
     *
     * Called during initialization before services are injected.
     * Use this to declare required services via require_service<T>().
     */
    virtual auto configure() -> void {}

    /**
     * @brief Monitoring logic (must override in derived class)
     *
     * This is the main monitoring method. Implement performance tracking,
     * data quality checks, and statistics collection here.
     *
     * @param event The current event data
     * @return StatusCode SUCCESS to continue, FAILURE to stop
     */
    virtual auto monitor(Event& event) -> StatusCode = 0;

    /**
     * @brief Custom initialization hook (optional override)
     *
     * Called after services are injected but before event processing begins.
     * Use this for booking monitoring histograms, setting up counters, etc.
     *
     * @return StatusCode SUCCESS to continue, FAILURE to abort
     */
    virtual auto on_initialize() -> StatusCode { return StatusCode::SUCCESS(); }

    /**
     * @brief Custom finalization hook (optional override)
     *
     * Called after all events have been processed.
     * Use this for printing statistics, generating reports, etc.
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
        // Step 1: Call configure() to let derived class declare services
        static_cast<Derived*>(this)->configure();

        // Step 2: Auto-inject all declared services
        for (auto& [svc_name, handle] : m_service_handles) {
            if (!m_svc_mgr) {
                return StatusCode::failure(
                    fmt::format("Algorithm '{}': ServiceManager not set", m_name));
            }

            auto sc = handle->initialize(*m_svc_mgr);
            if (sc.is_failure()) {
                return StatusCode::failure(
                    fmt::format("Algorithm '{}': Failed to get service '{}': {}",
                                m_name, svc_name, sc.message()));
            }
        }

        // Step 3: Call on_initialize() for custom initialization
        return static_cast<Derived*>(this)->on_initialize();
    }

    /**
     * @brief Execute monitoring for one event
     *
     * This method is auto-implemented and should not be overridden.
     * It delegates to monitor().
     *
     * @param event The current event data
     * @return StatusCode from monitor()
     */
    auto execute(Event& event) -> StatusCode final {
        return static_cast<Derived*>(this)->monitor(event);
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
     * @brief Declare a required service dependency
     *
     * Call this in configure() to declare services that this algorithm needs.
     * Services are automatically retrieved during initialize().
     *
     * @tparam T Service type (must satisfy ServiceConcept)
     * @param service_name Name of the service to retrieve
     * @return ServiceHandle<T>& Reference to the service handle
     */
    template <ServiceConcept T>
    auto require_service(const std::string& service_name) -> ServiceHandle<T>& {
        auto wrapper = std::make_unique<ServiceHandleWrapper<T>>(service_name);
        auto& handle = wrapper->handle();
        m_service_handles[service_name] = std::move(wrapper);
        return handle;
    }

    /**
     * @brief Get the number of declared services
     * @return std::size_t Number of services
     */
    [[nodiscard]] auto service_count() const -> std::size_t {
        return m_service_handles.size();
    }

    /**
     * @brief Check if a service with the given name is declared
     * @param svc_name Service name
     * @return bool True if declared
     */
    [[nodiscard]] auto has_service(const std::string& svc_name) const -> bool {
        return m_service_handles.contains(svc_name);
    }

private:
    std::map<std::string, std::unique_ptr<IServiceHandleBase>> m_service_handles;
};

}  // namespace Framework
