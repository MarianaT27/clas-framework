#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Framework/Cutflow.hpp"
#include "Framework/StatusCode.hpp"

namespace Framework {

/**
 * @brief A single cut with its predicate and metadata
 *
 * Encapsulates a named cut with its evaluation function and optional
 * histogram filling callback. Follows C++ Core Guidelines:
 * - F.50: Use a lambda when a function won't do
 * - I.23: Keep the number of function arguments low
 */
struct Cut {
    std::string name;
    std::function<bool()> predicate;
    std::function<void()> on_pass = nullptr;  // Optional callback after passing

    [[nodiscard]] auto evaluate() const -> bool { return predicate(); }
};

/**
 * @brief Manages a sequence of cuts with automatic cutflow tracking
 *
 * CutSequence eliminates repetitive cut application code by encapsulating:
 * - Cut definition with predicates
 * - Automatic cutflow counter updates
 * - Optional histogram filling after each cut
 * - Early return on cut failure
 *
 * Usage:
 * @code
 * CutSequence cuts("Electron Selection");
 *
 * // In initialize():
 * cuts.add("Chi2 PID", [&]{ return pass_chi2pid(e.chi2pid()); },
 *          [&]{ fill_histograms("Chi2PID", e); });
 * cuts.add("Momentum", [&]{ return e.p() > 0.8; });
 *
 * // In execute():
 * if (!cuts.apply()) return StatusCode::FILTERED();
 * @endcode
 *
 * C++ Core Guidelines:
 * - C.4: Make a function a member only if it needs direct access to the representation
 * - F.21: To return multiple "out" values, prefer returning a struct
 */
class CutSequence {
public:
    explicit CutSequence(std::string name) : m_cutflow(std::move(name)) {}

    /**
     * @brief Add a cut to the sequence
     * @param name Display name for cutflow
     * @param predicate Function returning true if cut passes
     * @param on_pass Optional callback executed after cut passes
     */
    auto add(std::string name, std::function<bool()> predicate,
             std::function<void()> on_pass = nullptr) -> CutSequence& {
        m_cutflow.add_cut(name);
        m_cuts.push_back({std::move(name), std::move(predicate), std::move(on_pass)});
        return *this;
    }

    /**
     * @brief Apply all cuts in sequence
     * @return true if all cuts pass, false on first failure
     *
     * Updates cutflow counters and calls on_pass callbacks for passing cuts.
     */
    [[nodiscard]] auto apply() -> bool {
        for (const auto& cut : m_cuts) {
            if (!cut.evaluate()) {
                return false;
            }
            m_cutflow.pass(cut.name);
            if (cut.on_pass) {
                cut.on_pass();
            }
        }
        return true;
    }

    /**
     * @brief Apply cuts and return StatusCode
     * @return SUCCESS if all pass, FILTERED on first failure
     */
    [[nodiscard]] auto apply_status() -> StatusCode {
        return apply() ? StatusCode::SUCCESS() : StatusCode::FILTERED();
    }

    /**
     * @brief Get index of first failing cut (for diagnostics)
     * @return Index of failing cut, or nullopt if all pass
     */
    [[nodiscard]] auto find_failing_cut() const -> std::optional<std::size_t> {
        for (std::size_t i = 0; i < m_cuts.size(); ++i) {
            if (!m_cuts[i].evaluate()) {
                return i;
            }
        }
        return std::nullopt;
    }

    /// Access underlying cutflow for printing/statistics
    [[nodiscard]] auto cutflow() -> Cutflow& { return m_cutflow; }
    [[nodiscard]] auto cutflow() const -> const Cutflow& { return m_cutflow; }

    /// Get cut names
    [[nodiscard]] auto cut_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_cuts.size());
        for (const auto& cut : m_cuts) {
            names.push_back(cut.name);
        }
        return names;
    }

    /// Number of cuts
    [[nodiscard]] auto size() const -> std::size_t { return m_cuts.size(); }

    /// Set logger for cutflow
    auto set_logger(LoggingService* log) -> void { m_cutflow.set_logger(log); }

    /// Print cutflow
    auto print() const -> void { m_cutflow.print(); }

    /// Clear all cuts (for reconfiguration)
    auto clear() -> void {
        m_cuts.clear();
        m_cutflow = Cutflow(m_cutflow.name());
    }

private:
    Cutflow m_cutflow;
    std::vector<Cut> m_cuts;
};

/**
 * @brief Builder for creating cut sequences with fluent API
 *
 * Provides a more readable way to construct cut sequences:
 * @code
 * auto cuts = CutSequenceBuilder("Electron")
 *     .add("Chi2 PID", [&]{ return pass_chi2pid(e); })
 *     .add("Momentum", [&]{ return e.p() > 0.8; })
 *     .build();
 * @endcode
 */
class CutSequenceBuilder {
public:
    explicit CutSequenceBuilder(std::string name) : m_sequence(std::move(name)) {}

    auto add(std::string name, std::function<bool()> predicate,
             std::function<void()> on_pass = nullptr) -> CutSequenceBuilder& {
        m_sequence.add(std::move(name), std::move(predicate), std::move(on_pass));
        return *this;
    }

    [[nodiscard]] auto build() -> CutSequence { return std::move(m_sequence); }

private:
    CutSequence m_sequence;
};

}  // namespace Framework
