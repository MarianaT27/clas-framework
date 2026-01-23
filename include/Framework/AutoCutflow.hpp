#pragma once

/**
 * @file AutoCutflow.hpp
 * @brief Predicate-based cutflow that combines cut definition and automatic pass tracking
 *
 * This file provides AutoCutflow and ContextCutflow classes that eliminate the need
 * for separate add_cut() and pass() calls by defining cuts once with their predicates.
 *
 * Example usage:
 * @code
 * // BEFORE: Dual definition required
 * m_cutflow.add_cut("Chi2 PID");           // in on_initialize()
 * if (pass_chi2pid) m_cutflow.pass("Chi2 PID");  // in process() - must match exactly
 *
 * // AFTER: Single definition with automatic tracking
 * ContextCutflow<ElectronContext> m_cutflow{"Electron Selection"};
 * m_cutflow.define("Chi2 PID", [&](const auto& ctx) {
 *     return cuts_svc->pass_chi2pid_cut(ctx.electron.chi2pid());
 * });
 * // ...
 * if (!m_cutflow.evaluate(ctx)) return FILTERED;  // Auto-passes cuts that succeed
 * @endcode
 */

#include <functional>
#include <string>
#include <vector>

#include "Framework/Cutflow.hpp"
#include "Framework/StatusCode.hpp"

namespace Framework {

/**
 * @brief Cutflow with automatic cut registration and execution
 *
 * AutoCutflow eliminates the need for separate add_cut() and pass() calls by
 * defining cuts once with their predicates. When evaluate() is called, it
 * runs each predicate in order and automatically records pass/fail status.
 *
 * Use this for simple cuts that don't need per-event context.
 *
 * Example:
 * @code
 * AutoCutflow cutflow{"My Selection"};
 *
 * // Define cuts (replaces add_cut)
 * cutflow.define_always("All Events");  // Always passes
 * cutflow.define("Has Particles", [&]{ return !particles.empty(); });
 * cutflow.define("Quality Cut", [&]{ return quality > 0.5; });
 *
 * // Evaluate all cuts (replaces manual pass() calls)
 * if (!cutflow.evaluate()) {
 *     return StatusCode::FILTERED();
 * }
 * @endcode
 */
class AutoCutflow {
public:
    using Predicate = std::function<bool()>;

    /**
     * @brief Construct an AutoCutflow with the given name
     * @param name Name for the cutflow (displayed in summary)
     */
    explicit AutoCutflow(std::string name) : m_cutflow(std::move(name)) {}

    /**
     * @brief Define a cut with its predicate
     *
     * The predicate is called during evaluate() and if it returns true,
     * the cut is automatically marked as passed.
     *
     * @param cut_name Name of the cut (for display)
     * @param pred Predicate function that returns true if cut passes
     * @return AutoCutflow& Reference to this for method chaining
     */
    auto define(std::string cut_name, Predicate pred) -> AutoCutflow& {
        m_cutflow.add_cut(cut_name);
        m_cuts.emplace_back(std::move(cut_name), std::move(pred));
        return *this;
    }

    /**
     * @brief Define a cut that always passes (e.g., "All Events")
     *
     * @param cut_name Name of the cut
     * @return AutoCutflow& Reference to this for method chaining
     */
    auto define_always(std::string cut_name) -> AutoCutflow& {
        return define(std::move(cut_name), [] { return true; });
    }

    /**
     * @brief Evaluate all cuts in sequence
     *
     * Runs each predicate in the order they were defined. For each cut that
     * passes, automatically calls pass() on the underlying cutflow.
     * Stops on the first failure.
     *
     * @return true if all cuts pass, false on first failure
     */
    [[nodiscard]] auto evaluate() -> bool {
        for (auto& [name, predicate] : m_cuts) {
            if (!predicate()) {
                return false;
            }
            m_cutflow.pass(name);
        }
        return true;
    }

    /**
     * @brief Evaluate cuts and return StatusCode
     *
     * Convenience method that returns FILTERED on failure, SUCCESS on pass.
     *
     * @return StatusCode SUCCESS if all cuts pass, FILTERED otherwise
     */
    [[nodiscard]] auto apply() -> StatusCode {
        return evaluate() ? StatusCode::SUCCESS() : StatusCode::FILTERED();
    }

    /**
     * @brief Print cutflow summary table
     */
    auto print() const -> void { m_cutflow.print(); }

    /**
     * @brief Get the underlying Cutflow for advanced usage
     * @return Cutflow& Reference to the underlying cutflow
     */
    auto cutflow() -> Cutflow& { return m_cutflow; }

    /**
     * @brief Get the underlying Cutflow (const)
     * @return const Cutflow& Reference to the underlying cutflow
     */
    [[nodiscard]] auto cutflow() const -> const Cutflow& { return m_cutflow; }

    /**
     * @brief Get the cutflow name
     */
    [[nodiscard]] auto name() const -> const std::string& { return m_cutflow.name(); }

    /**
     * @brief Get the number of defined cuts
     */
    [[nodiscard]] auto num_cuts() const -> std::size_t { return m_cuts.size(); }

    /**
     * @brief Get count for a specific cut
     */
    [[nodiscard]] auto count(const std::string& cut_name) const -> std::size_t {
        return m_cutflow.count(cut_name);
    }

    /**
     * @brief Get overall efficiency
     */
    [[nodiscard]] auto efficiency() const -> double { return m_cutflow.efficiency(); }

private:
    Cutflow m_cutflow;
    std::vector<std::pair<std::string, Predicate>> m_cuts;
};

/**
 * @brief Context-aware cutflow for per-event evaluation
 *
 * ContextCutflow allows defining cuts that depend on per-event context.
 * The context is passed to evaluate() and forwarded to each predicate.
 *
 * This is useful when cuts need access to event-specific data like
 * particle properties, calculated quantities, etc.
 *
 * Example:
 * @code
 * struct ElectronContext {
 *     const CLAS::Particle& electron;
 *     double SF;
 *     int sector;
 * };
 *
 * ContextCutflow<ElectronContext> cutflow{"Electron Selection"};
 *
 * // Define cuts with context access
 * cutflow.define("Chi2 PID", [&](const ElectronContext& ctx) {
 *     return cuts_svc->pass_chi2pid_cut(ctx.electron.chi2pid());
 * });
 * cutflow.define("Sampling Fraction", [&](const ElectronContext& ctx) {
 *     return cuts_svc->pass_sf_cut(ctx.SF, ctx.electron.p(), ctx.sector);
 * });
 *
 * // In event loop:
 * ElectronContext ctx{electron, SF, sector};
 * if (!cutflow.evaluate(ctx)) {
 *     return StatusCode::FILTERED();
 * }
 * @endcode
 *
 * @tparam Context Type of the context passed to predicates
 */
template <typename Context>
class ContextCutflow {
public:
    using Predicate = std::function<bool(const Context&)>;

    /**
     * @brief Construct a ContextCutflow with the given name
     * @param name Name for the cutflow (displayed in summary)
     */
    explicit ContextCutflow(std::string name) : m_cutflow(std::move(name)) {}

    /**
     * @brief Define a cut with its context-aware predicate
     *
     * @param cut_name Name of the cut
     * @param pred Predicate function that takes context and returns true if cut passes
     * @return ContextCutflow& Reference to this for method chaining
     */
    auto define(std::string cut_name, Predicate pred) -> ContextCutflow& {
        m_cutflow.add_cut(cut_name);
        m_cuts.emplace_back(std::move(cut_name), std::move(pred));
        return *this;
    }

    /**
     * @brief Define a cut that always passes (e.g., "All Events")
     *
     * @param cut_name Name of the cut
     * @return ContextCutflow& Reference to this for method chaining
     */
    auto define_always(std::string cut_name) -> ContextCutflow& {
        return define(std::move(cut_name), [](const Context&) { return true; });
    }

    /**
     * @brief Evaluate all cuts with the given context
     *
     * Runs each predicate in order, passing the context to each.
     * Stops on the first failure.
     *
     * @param ctx Context to pass to predicates
     * @return true if all cuts pass, false on first failure
     */
    [[nodiscard]] auto evaluate(const Context& ctx) -> bool {
        for (auto& [name, predicate] : m_cuts) {
            if (!predicate(ctx)) {
                return false;
            }
            m_cutflow.pass(name);
        }
        return true;
    }

    /**
     * @brief Evaluate cuts and return StatusCode
     *
     * @param ctx Context to pass to predicates
     * @return StatusCode SUCCESS if all cuts pass, FILTERED otherwise
     */
    [[nodiscard]] auto apply(const Context& ctx) -> StatusCode {
        return evaluate(ctx) ? StatusCode::SUCCESS() : StatusCode::FILTERED();
    }

    /**
     * @brief Print cutflow summary table
     */
    auto print() const -> void { m_cutflow.print(); }

    /**
     * @brief Get the underlying Cutflow for advanced usage
     */
    auto cutflow() -> Cutflow& { return m_cutflow; }

    [[nodiscard]] auto cutflow() const -> const Cutflow& { return m_cutflow; }

    [[nodiscard]] auto name() const -> const std::string& { return m_cutflow.name(); }

    [[nodiscard]] auto num_cuts() const -> std::size_t { return m_cuts.size(); }

    [[nodiscard]] auto count(const std::string& cut_name) const -> std::size_t {
        return m_cutflow.count(cut_name);
    }

    [[nodiscard]] auto efficiency() const -> double { return m_cutflow.efficiency(); }

private:
    Cutflow m_cutflow;
    std::vector<std::pair<std::string, Predicate>> m_cuts;
};

}  // namespace Framework
