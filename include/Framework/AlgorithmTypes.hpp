#pragma once

#include <string>
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/StatusCode.hpp"

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
    [[nodiscard]] auto would_execute(const Event& event) const -> bool {
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

}  // namespace Framework
