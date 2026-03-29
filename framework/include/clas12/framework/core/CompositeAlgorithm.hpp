#pragma once

/**
 * @file CompositeAlgorithm.hpp
 * @brief Composite pattern for building reusable algorithm pipelines
 *
 * Provides classes for composing multiple algorithms into a single unit:
 * - AlgorithmSequence: Execute algorithms in order, stop on FAILURE
 * - FilterSequence: Execute algorithms in order, stop on FILTERED or FAILURE
 * - ConditionalSequence: Execute only if event passes a decision flag
 * - ParallelSequence: Execute algorithms in parallel (future)
 */

#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/IAlgorithm.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

// Forward declaration
class Event;

/**
 * @brief Sequence of algorithms executed in order
 *
 * Executes child algorithms in the order they were added.
 * Stops on FAILURE, continues on SUCCESS and FILTERED.
 *
 * Example usage:
 * @code
 * AlgorithmSequence seq("MySequence");
 * seq.add<ElectronSelection>("ElectronSel");
 * seq.add<DISSelection>("DISSel");
 * seq.add<FillHistograms>("Histograms");
 *
 * app.add_algorithm(std::make_unique<AlgorithmSequence>(std::move(seq)));
 * @endcode
 */
class AlgorithmSequence : public Algorithm {
   public:
    explicit AlgorithmSequence(std::string name) : Algorithm(std::move(name)) {}

    /**
     * @brief Add an algorithm to the sequence
     *
     * @tparam T Algorithm type
     * @tparam Args Constructor argument types
     * @param name Algorithm name
     * @param args Constructor arguments
     * @return T* Pointer to the added algorithm
     */
    template <typename T, typename... Args>
    auto add(std::string name, Args&&... args) -> T* {
        auto alg = std::make_unique<T>(std::move(name), std::forward<Args>(args)...);
        T* ptr = alg.get();
        m_algorithms.push_back(std::move(alg));
        return ptr;
    }

    /**
     * @brief Add an existing algorithm to the sequence
     *
     * @param alg Algorithm to add (ownership transferred)
     */
    auto add(std::unique_ptr<IAlgorithm> alg) -> void {
        m_algorithms.push_back(std::move(alg));
    }

    /**
     * @brief Get the number of algorithms in the sequence
     * @return std::size_t Number of algorithms
     */
    [[nodiscard]] auto size() const -> std::size_t {
        return m_algorithms.size();
    }

    auto initialize() -> StatusCode override {
        set_state(AlgorithmState::Initializing);

        for (auto& alg : m_algorithms) {
            alg->set_service_manager(m_svc_mgr);
            auto sc = alg->initialize();
            if (sc.is_failure()) {
                Log::error(m_name, "Failed to initialize algorithm '{}'", alg->name());
                return sc;
            }
        }

        set_state(AlgorithmState::Initialized);
        return StatusCode::SUCCESS();
    }

    auto execute(Event& event) -> StatusCode override {
        for (auto& alg : m_algorithms) {
            auto sc = alg->execute(event);
            if (sc.is_failure()) {
                return sc;  // Stop on failure
            }
            // Continue on SUCCESS and FILTERED
        }
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        set_state(AlgorithmState::Finalizing);

        StatusCode result = StatusCode::SUCCESS();
        for (auto& alg : m_algorithms) {
            auto sc = alg->finalize();
            if (sc.is_failure()) {
                Log::warning(m_name, "Algorithm '{}' failed to finalize", alg->name());
                result = sc;  // Continue but remember failure
            }
        }

        set_state(AlgorithmState::Finalized);
        return result;
    }

   protected:
    std::vector<std::unique_ptr<IAlgorithm>> m_algorithms;

   private:
    // Helper to set state (using Algorithm's protected method)
    auto set_state(AlgorithmState state) -> void {
        Algorithm::set_state(state);
    }
};

/**
 * @brief Sequence that stops on FILTERED status
 *
 * Similar to AlgorithmSequence, but also stops processing remaining
 * algorithms when any algorithm returns FILTERED. This is useful for
 * cut-based event selection where you want to stop processing as soon
 * as an event fails a cut.
 */
class FilterSequence : public AlgorithmSequence {
   public:
    explicit FilterSequence(std::string name) : AlgorithmSequence(std::move(name)) {}

    auto execute(Event& event) -> StatusCode override {
        for (auto& alg : m_algorithms) {
            auto sc = alg->execute(event);
            if (!sc.is_success()) {
                return sc;  // Stop on FAILURE or FILTERED
            }
        }
        return StatusCode::SUCCESS();
    }
};

/**
 * @brief Sequence that executes only if event passes a typed decision
 *
 * Checks a TypedKey<bool> decision flag in the event store before executing.
 * If the flag is not set or is false, the sequence is skipped.
 *
 * Example usage:
 * @code
 * // Earlier algorithm sets decision:
 * event.set_decision_typed(Decisions::HasGoodElectron, true);
 *
 * // Conditional sequence only runs if decision is true
 * TypedConditionalSequence seq("ElectronAnalysis", Decisions::HasGoodElectron);
 * seq.add<ElectronAnalysis>("Analysis");
 * @endcode
 */
class TypedConditionalSequence : public AlgorithmSequence {
   public:
    TypedConditionalSequence(std::string name, TypedKey<bool> decision_key)
        : AlgorithmSequence(std::move(name))
        , m_decision_key(std::move(decision_key)) {}

    auto execute(Event& event) -> StatusCode override;

   private:
    TypedKey<bool> m_decision_key;
};

/**
 * @brief Sequence that collects multiple results without short-circuiting
 *
 * Executes all child algorithms regardless of their return status.
 * Returns the "worst" status (FAILURE > FILTERED > SUCCESS).
 * Useful for monitoring and cleanup tasks.
 */
class CollectingSequence : public AlgorithmSequence {
   public:
    explicit CollectingSequence(std::string name) : AlgorithmSequence(std::move(name)) {}

    auto execute(Event& event) -> StatusCode override {
        StatusCode worst = StatusCode::SUCCESS();

        for (auto& alg : m_algorithms) {
            auto sc = alg->execute(event);

            // Track worst status
            if (sc.is_failure()) {
                worst = sc;
            } else if (sc.is_filtered() && worst.is_success()) {
                worst = sc;
            }
        }

        return worst;
    }
};

/**
 * @brief Factory function to create an AlgorithmSequence
 *
 * @param name Sequence name
 * @return std::unique_ptr<AlgorithmSequence> New sequence
 */
inline auto make_sequence(std::string name) -> std::unique_ptr<AlgorithmSequence> {
    return std::make_unique<AlgorithmSequence>(std::move(name));
}

/**
 * @brief Factory function to create a FilterSequence
 *
 * @param name Sequence name
 * @return std::unique_ptr<FilterSequence> New filter sequence
 */
inline auto make_filter_sequence(std::string name) -> std::unique_ptr<FilterSequence> {
    return std::make_unique<FilterSequence>(std::move(name));
}

/**
 * @brief Factory function to create a TypedConditionalSequence
 *
 * @param name Sequence name
 * @param decision_key Typed decision key to check
 * @return std::unique_ptr<TypedConditionalSequence> New conditional sequence
 */
inline auto make_conditional_sequence(std::string name, TypedKey<bool> decision_key)
    -> std::unique_ptr<TypedConditionalSequence> {
    return std::make_unique<TypedConditionalSequence>(std::move(name), std::move(decision_key));
}

}  // namespace clas12::framework

// Include Event.hpp for execute() implementations
#include <clas12/framework/core/Event.hpp>

namespace clas12::framework {

inline auto TypedConditionalSequence::execute(Event& event) -> StatusCode {
    if (!event.passes_decision_typed(m_decision_key)) {
        return StatusCode::SUCCESS();  // Skip but don't filter
    }
    return AlgorithmSequence::execute(event);
}

}  // namespace clas12::framework
