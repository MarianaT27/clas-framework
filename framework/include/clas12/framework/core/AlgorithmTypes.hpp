#pragma once

#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/IAlgorithm.hpp>
#include <clas12/framework/core/StatusCode.hpp>

#include <clas12/framework/core/SimpleAlgorithm.hpp>

namespace clas12::framework {

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

}  // namespace clas12::framework
