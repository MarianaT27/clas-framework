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

#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <vector>

#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace framework {

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
 *     const Particle& electron;
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

    using FillFn = std::function<void(const Context&)>;

    /// Function type for extracting the cut variable from context (for N-1 histograms)
    using VariableExtractor = std::function<double(const Context&)>;

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
        m_fill_fns.emplace_back(nullptr);
        m_variable_extractors.emplace_back(nullptr);
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
     * @brief Define a cut with an associated histogram fill callback
     *
     * Like define(), but also registers a fill function that is called
     * when this cut passes (during evaluate_and_fill()). This enables
     * ATLAS-style N-1 histograms and per-cut-stage distributions.
     *
     * @param cut_name Name of the cut
     * @param pred Predicate that returns true if cut passes
     * @param fill_fn Callback to fill histograms when this cut passes
     * @return ContextCutflow& Reference to this for method chaining
     *
     * Example:
     * @code
     * cutflow.define_with_fill("All Events",
     *     [](const auto&) { return true; },
     *     [&](const PionCandidate& c) {
     *         h_precut.momentum->fill(c.p);
     *         h_precut.theta->fill(c.theta);
     *     });
     * cutflow.define_with_fill("Momentum",
     *     [](const auto& c) { return c.passes_momentum; },
     *     [&](const PionCandidate& c) {
     *         h_postmom.momentum->fill(c.p);
     *     });
     * @endcode
     */
    auto define_with_fill(std::string cut_name, Predicate pred, FillFn fill_fn) -> ContextCutflow& {
        m_cutflow.add_cut(cut_name);
        m_cuts.emplace_back(std::move(cut_name), std::move(pred));
        m_fill_fns.emplace_back(std::move(fill_fn));
        m_variable_extractors.emplace_back(nullptr);
        return *this;
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
     * @brief Evaluate all cuts AND fill histograms at each passing stage
     *
     * Like evaluate(), but also calls the fill callback (if any) for
     * each cut that passes. This enables filling histograms at different
     * cut stages in a single pass through the cut chain.
     *
     * @param ctx Context to pass to predicates and fill functions
     * @return true if all cuts pass, false on first failure
     */
    [[nodiscard]] auto evaluate_and_fill(const Context& ctx) -> bool {
        for (std::size_t i = 0; i < m_cuts.size(); ++i) {
            auto& [name, predicate] = m_cuts[i];
            if (!predicate(ctx)) {
                return false;
            }
            m_cutflow.pass(name);
            if (i < m_fill_fns.size() && m_fill_fns[i]) {
                m_fill_fns[i](ctx);
            }
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

    // ========================================================================
    // N-1 Histogram Support
    // ========================================================================

    /**
     * @brief Define a cut with an associated variable extractor for N-1 histograms
     *
     * Like define(), but also registers a function that extracts the cut variable
     * from the context. This enables automatic N-1 histogram filling via evaluate_all().
     *
     * @param cut_name Name of the cut
     * @param pred Predicate that returns true if cut passes
     * @param var_fn Function that extracts the cut variable from context
     * @return ContextCutflow& Reference to this for method chaining
     *
     * @code
     * cutflow.define_with_variable("Delta Vz",
     *     [](const auto& c) { return std::abs(c.delta_vz) < 5.0; },
     *     [](const auto& c) { return c.delta_vz; });
     * @endcode
     */
    auto define_with_variable(std::string cut_name, Predicate pred,
                              VariableExtractor var_fn) -> ContextCutflow& {
        m_cutflow.add_cut(cut_name);
        m_cuts.emplace_back(std::move(cut_name), std::move(pred));
        m_fill_fns.emplace_back(nullptr);
        m_variable_extractors.emplace_back(std::move(var_fn));
        return *this;
    }

    // ========================================================================
    // Toggleable Cuts
    // ========================================================================

    /**
     * @brief Define a cut that can be toggled on/off via a Property<bool>
     *
     * When the property is false, the cut always passes (is skipped).
     * Eliminates the `!enable.get() || pred(ctx)` pattern.
     *
     * @param cut_name Name of the cut
     * @param pred Predicate that returns true if cut passes
     * @param enable_prop Reference to a Property<bool> that controls this cut
     * @return ContextCutflow& Reference to this for method chaining
     */
    auto define_toggleable(std::string cut_name, Predicate pred,
                           const Property<bool>& enable_prop) -> ContextCutflow& {
        auto wrapped = [pred = std::move(pred), &enable_prop](const Context& ctx) {
            return !enable_prop.get() || pred(ctx);
        };
        return define(std::move(cut_name), std::move(wrapped));
    }

    /**
     * @brief Define a toggleable cut with a variable extractor for N-1 histograms
     *
     * Combines toggleable behavior with N-1 variable extraction.
     *
     * @param cut_name Name of the cut
     * @param pred Predicate that returns true if cut passes
     * @param enable_prop Reference to a Property<bool> that controls this cut
     * @param var_fn Function that extracts the cut variable from context
     * @return ContextCutflow& Reference to this for method chaining
     */
    auto define_toggleable(std::string cut_name, Predicate pred,
                           const Property<bool>& enable_prop,
                           VariableExtractor var_fn) -> ContextCutflow& {
        auto wrapped = [pred = std::move(pred), &enable_prop](const Context& ctx) {
            return !enable_prop.get() || pred(ctx);
        };
        return define_with_variable(std::move(cut_name), std::move(wrapped), std::move(var_fn));
    }

    // ========================================================================
    // N-1 Evaluation
    // ========================================================================

    /**
     * @brief Results from evaluating all cuts (for N-1 analysis)
     *
     * Unlike evaluate() which short-circuits, evaluate_all() runs every cut
     * and stores per-cut pass/fail results. This enables N-1 histogram filling
     * where you fill a histogram for cut N when all cuts EXCEPT N pass.
     */
    struct CutResults {
        std::vector<bool> passed;  ///< Per-cut pass/fail results
        bool all_passed = false;   ///< True if every cut passed

        /**
         * @brief Check if all cuts pass except the one at skip_index
         * @param skip_index Index of the cut to exclude from the check
         * @return true if all other cuts passed
         */
        [[nodiscard]] auto passes_all_except(std::size_t skip_index) const -> bool {
            for (std::size_t i = 0; i < passed.size(); ++i) {
                if (i == skip_index) continue;
                if (!passed[i]) return false;
            }
            return true;
        }
    };

    /**
     * @brief Evaluate ALL cuts without short-circuiting (for N-1 analysis)
     *
     * Runs every predicate regardless of failures. Updates cutflow counters
     * only for cuts that pass. Returns a CutResults object that can be used
     * to determine N-1 combinations.
     *
     * @param ctx Context to pass to predicates
     * @return CutResults with per-cut pass/fail and overall result
     *
     * @code
     * auto results = m_cutflow.evaluate_all(candidate);
     * // Fill N-1 histograms
     * for (size_t i = 0; i < m_cutflow.num_cuts(); ++i) {
     *     if (results.passes_all_except(i)) {
     *         nminus1_hists[i]->fill(m_cutflow.variable(i, candidate));
     *     }
     * }
     * if (results.all_passed) { // accept }
     * @endcode
     */
    [[nodiscard]] auto evaluate_all(const Context& ctx) -> CutResults {
        CutResults results;
        results.passed.resize(m_cuts.size());
        results.all_passed = true;

        for (std::size_t i = 0; i < m_cuts.size(); ++i) {
            results.passed[i] = m_cuts[i].second(ctx);
            if (results.passed[i]) {
                m_cutflow.pass(m_cuts[i].first);
            } else {
                results.all_passed = false;
            }
        }
        return results;
    }

    /**
     * @brief Extract the cut variable for the cut at the given index
     *
     * Uses the variable extractor registered via define_with_variable().
     * Returns NaN if no extractor was registered for that cut.
     *
     * @param idx Cut index
     * @param ctx Context to extract from
     * @return The extracted variable value, or NaN if no extractor
     */
    [[nodiscard]] auto variable(std::size_t idx, const Context& ctx) const -> double {
        if (idx < m_variable_extractors.size() && m_variable_extractors[idx]) {
            return m_variable_extractors[idx](ctx);
        }
        return std::numeric_limits<double>::quiet_NaN();
    }

    /**
     * @brief Get the cut name at the given index
     * @param idx Cut index
     * @return The cut name
     */
    [[nodiscard]] auto cut_name(std::size_t idx) const -> const std::string& {
        return m_cuts.at(idx).first;
    }

private:
    Cutflow m_cutflow;
    std::vector<std::pair<std::string, Predicate>> m_cuts;
    std::vector<FillFn> m_fill_fns;
    std::vector<VariableExtractor> m_variable_extractors;

    /// Ensure all parallel vectors stay in sync when using define() (no variable extractor)
    void sync_variable_extractors() {
        while (m_variable_extractors.size() < m_cuts.size()) {
            m_variable_extractors.emplace_back(nullptr);
        }
    }
};

}  // namespace framework
