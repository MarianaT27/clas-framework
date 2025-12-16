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
 */
class AlgorithmSequence : public Algorithm {
   public:
    explicit AlgorithmSequence(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Add algorithm to sequence
     * @param algorithm Algorithm to add
     */
    auto add_algorithm(std::unique_ptr<IAlgorithm> algorithm) -> void {
        m_algorithms.push_back(std::move(algorithm));
    }

    /**
     * @brief Initialize all algorithms in sequence
     */
    auto initialize() -> StatusCode override {
        for (auto& alg : m_algorithms) {
            auto sc = alg->initialize();
            if (sc.is_failure()) {
                return sc;
            }
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Execute all algorithms in sequence
     */
    auto execute(Event& event) -> StatusCode override {
        for (auto& alg : m_algorithms) {
            auto sc = alg->execute(event);
            if (sc.is_failure()) {
                return sc;
            }
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Finalize all algorithms in sequence
     */
    auto finalize() -> StatusCode override {
        for (auto& alg : m_algorithms) {
            auto sc = alg->finalize();
            if (sc.is_failure()) {
                return sc;
            }
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Get number of algorithms in sequence
     */
    auto size() const -> size_t {
        return m_algorithms.size();
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
 */
class ConditionalSequence : public AlgorithmSequence {
   public:
    explicit ConditionalSequence(std::string name, std::string required_decision)
        : AlgorithmSequence(std::move(name)), m_required_decision(std::move(required_decision)) {}

    /**
     * @brief Execute sequence only if required decision passes
     */
    auto execute(Event& event) -> StatusCode override {
        // Check required decision
        if (!event.passes_decision(m_required_decision)) {
            return StatusCode::SUCCESS();  // Skip sequence
        }

        // Execute all algorithms in sequence
        return AlgorithmSequence::execute(event);
    }

    /**
     * @brief Get the required decision name
     */
    auto required_decision() const -> const std::string& {
        return m_required_decision;
    }

   private:
    std::string m_required_decision;
};

}  // namespace Framework
