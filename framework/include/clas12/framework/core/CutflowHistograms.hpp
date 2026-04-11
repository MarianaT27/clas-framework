#pragma once

/**
 * @file CutflowHistograms.hpp
 * @brief Unified cutflow and histogram management for particle analysis
 *
 * Combines predicate-based cutflow tracking with automatic histogram filling
 * at each cut stage. This eliminates the common pattern of defining cuts and
 * histogram stages separately.
 *
 * @see AutoCutflow, ContextCutflow, ThreadLocalHistogramTemplates
 */

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/AutoCutflow.hpp>
#include <clas12/framework/core/HistogramTemplates.hpp>
#include <clas12/framework/services/HistogramService.hpp>

namespace clas12::framework {

/**
 * @brief Unified cutflow and histogram management for analysis
 *
 * CutflowHistograms combines predicate-based cut definition with automatic
 * histogram filling at each cut stage. When evaluate_and_fill() is called,
 * it evaluates cuts in sequence and fills histograms after each passed cut.
 *
 * This eliminates the common pattern of:
 * 1. Defining cuts in cutflow
 * 2. Separately booking histograms for each cut stage
 * 3. Manually calling cutflow.pass() and fill_histograms() after each cut
 *
 * Example usage:
 * @code
 * struct ElectronContext {
 *     clas12::Particle electron;
 *     double SF;
 *     int sector;
 * };
 *
 * class ElectronAnalysis : public SimpleAlgorithm<ElectronAnalysis> {
 *     CutflowHistograms<ElectronContext> m_analysis{"Electron Selection"};
 *
 *     auto on_initialize() -> StatusCode override {
 *         auto* hist_svc = svc<HistogramService>();
 *
 *         // Define cuts with predicates
 *         m_analysis
 *             .define("All Events", [](const auto&) { return true; })
 *             .define("Chi2 PID", [this](const auto& ctx) {
 *                 return m_cuts_svc->pass_chi2pid_cut(ctx.electron.chi2pid());
 *             })
 *             .define("Momentum", [this](const auto& ctx) {
 *                 return ctx.electron.p() > 0.8;
 *             })
 *             .define("Vertex Z", [this](const auto& ctx) {
 *                 return m_cuts_svc->pass_vz_cut(ctx.electron.vz());
 *             });
 *
 *         // Define what to fill at each cut stage
 *         m_analysis.set_filler([](ThreadLocalHistogramGroup* h, const ElectronContext& ctx) {
 *             h->fill("p", ctx.electron.p());
 *             h->fill("theta", ctx.electron.theta());
 *             h->fill2d("sf_vs_p", ctx.electron.p(), ctx.SF);
 *         });
 *
 *         // Initialize histograms (creates one set per cut stage)
 *         m_analysis.initialize(hist_svc, "/Electron");
 *
 *         return StatusCode::SUCCESS();
 *     }
 *
 *     auto process(Event& event) -> StatusCode override {
 *         ElectronContext ctx = build_context(event);
 *
 *         // Evaluate cuts and fill histograms in one call
 *         if (!m_analysis.evaluate_and_fill(ctx)) {
 *             return StatusCode::FILTERED();
 *         }
 *
 *         return StatusCode::SUCCESS();
 *     }
 *
 *     auto on_finalize() -> StatusCode override {
 *         m_analysis.print();  // Print cutflow summary
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 *
 * @tparam Context Type of per-event context passed to predicates and filler
 */
template <typename Context>
class CutflowHistograms {
public:
    using Predicate = std::function<bool(const Context&)>;
    using Filler = std::function<void(ThreadLocalHistogramGroup*, const Context&)>;

    /**
     * @brief Construct a CutflowHistograms with the given name
     * @param name Name for the cutflow (displayed in summary)
     */
    explicit CutflowHistograms(std::string name)
        : m_cutflow(std::move(name)) {}

    // ========================================================================
    // Cut Definition
    // ========================================================================

    /**
     * @brief Define a cut with its predicate
     *
     * @param cut_name Name of the cut (used in cutflow and as histogram directory)
     * @param pred Predicate function that returns true if cut passes
     * @return CutflowHistograms& Reference to this for method chaining
     */
    auto define(std::string cut_name, Predicate pred) -> CutflowHistograms& {
        // Store predicate locally for individual evaluation
        m_cuts.emplace_back(cut_name, pred);
        // Also register with underlying cutflow for tracking
        m_cutflow.define(cut_name, std::move(pred));
        return *this;
    }

    /**
     * @brief Define a cut that always passes (e.g., "All Events")
     *
     * @param cut_name Name of the cut
     * @return CutflowHistograms& Reference to this for method chaining
     */
    auto define_always(std::string cut_name) -> CutflowHistograms& {
        return define(std::move(cut_name), [](const Context&) { return true; });
    }

    // ========================================================================
    // Histogram Configuration
    // ========================================================================

    /**
     * @brief Set the function that fills histograms for each cut stage
     *
     * The filler receives an ThreadLocalHistogramGroup for the current cut stage
     * and the event context. It should fill all relevant histograms.
     *
     * @param filler Function that fills histograms given a histogram group and context
     * @return CutflowHistograms& Reference to this for method chaining
     */
    auto set_filler(Filler filler) -> CutflowHistograms& {
        m_filler = std::move(filler);
        return *this;
    }

    /**
     * @brief Use a predefined histogram template for each cut stage
     *
     * Creates histograms using the specified template factory function.
     * The template is applied to each cut stage with the base path
     * extended by the sanitized cut name.
     *
     * @param template_factory Factory function that creates histogram groups
     * @return CutflowHistograms& Reference to this for method chaining
     */
    auto use_template(std::function<std::unique_ptr<ThreadLocalHistogramGroup>(
                          HistogramService*, const std::string&)> template_factory) -> CutflowHistograms& {
        m_template_factory = std::move(template_factory);
        return *this;
    }

    // ========================================================================
    // Initialization
    // ========================================================================

    /**
     * @brief Initialize histograms for all cut stages
     *
     * Creates histogram groups for each defined cut using either the custom
     * template or a default set of electron-like histograms.
     *
     * @param svc HistogramService for booking histograms
     * @param base_path Base path for histogram directories (e.g., "/Electron")
     * @return bool True if initialization successful
     */
    auto initialize(HistogramService* svc, const std::string& base_path) -> bool {
        if (!svc) return false;

        m_service = svc;
        m_base_path = base_path;

        // Get cut names from the underlying cutflow
        const auto& cuts = m_cutflow.cutflow().cuts();

        for (const auto& cut_name : cuts) {
            std::string sanitized = sanitize_name(cut_name);
            std::string path = fmt::format("{}/{}", base_path, sanitized);

            if (m_template_factory) {
                // Use custom template
                m_histograms[cut_name] = m_template_factory(svc, path);
            } else {
                // Use default electron histograms
                m_histograms[cut_name] = ThreadLocalHistogramTemplates::ElectronHistograms::create(svc, path);
            }
        }

        m_initialized = true;
        return true;
    }

    // ========================================================================
    // Evaluation and Filling
    // ========================================================================

    /**
     * @brief Evaluate all cuts and fill histograms after each passed cut
     *
     * For each cut in sequence:
     * 1. Evaluate the predicate
     * 2. If it passes, record in cutflow and fill histograms
     * 3. If it fails, stop and return false
     *
     * @param ctx Context to pass to predicates and filler
     * @return true if all cuts pass, false on first failure
     */
    [[nodiscard]] auto evaluate_and_fill(const Context& ctx) -> bool {
        for (const auto& [cut_name, predicate] : m_cuts) {
            // Evaluate the cut predicate
            if (!predicate(ctx)) {
                return false;
            }

            // Cut passed - record in cutflow
            m_cutflow.cutflow().pass(cut_name);

            // Fill histograms for this stage
            fill_stage(cut_name, ctx);
        }

        return true;
    }

    /**
     * @brief Apply cuts and return StatusCode
     *
     * @param ctx Context to pass to predicates and filler
     * @return StatusCode SUCCESS if all cuts pass, FILTERED otherwise
     */
    [[nodiscard]] auto apply(const Context& ctx) -> StatusCode {
        return evaluate_and_fill(ctx) ? StatusCode::SUCCESS() : StatusCode::FILTERED();
    }

    // ========================================================================
    // Access and Utilities
    // ========================================================================

    /**
     * @brief Print cutflow summary table
     */
    auto print() const -> void {
        m_cutflow.print();
    }

    /**
     * @brief Get the underlying ContextCutflow
     */
    auto cutflow() -> ContextCutflow<Context>& { return m_cutflow; }

    [[nodiscard]] auto cutflow() const -> const ContextCutflow<Context>& { return m_cutflow; }

    /**
     * @brief Get histogram group for a specific cut stage
     */
    auto histograms(const std::string& cut_name) -> ThreadLocalHistogramGroup* {
        auto it = m_histograms.find(cut_name);
        return it != m_histograms.end() ? it->second.get() : nullptr;
    }

    /**
     * @brief Get the number of defined cuts
     */
    [[nodiscard]] auto num_cuts() const -> size_t { return m_cutflow.num_cuts(); }

    /**
     * @brief Get overall efficiency
     */
    [[nodiscard]] auto efficiency() const -> double { return m_cutflow.efficiency(); }

    /**
     * @brief Check if initialized
     */
    [[nodiscard]] auto is_initialized() const -> bool { return m_initialized; }

private:
    ContextCutflow<Context> m_cutflow;
    HistogramService* m_service = nullptr;
    std::string m_base_path;
    Filler m_filler;
    std::function<std::unique_ptr<ThreadLocalHistogramGroup>(HistogramService*, const std::string&)> m_template_factory;
    std::map<std::string, std::unique_ptr<ThreadLocalHistogramGroup>> m_histograms;
    bool m_initialized = false;

    // Store cuts with predicates for individual evaluation
    std::vector<std::pair<std::string, Predicate>> m_cuts;

    /**
     * @brief Fill histograms for a specific cut stage
     */
    auto fill_stage(const std::string& cut_name, const Context& ctx) -> void {
        if (!m_initialized) return;

        auto* group = histograms(cut_name);
        if (!group) return;

        if (m_filler) {
            m_filler(group, ctx);
        } else {
            // Default filler for common electron-like quantities
            // Users should provide their own filler for proper filling
        }
    }

    /**
     * @brief Sanitize cut name for use as directory name
     */
    static auto sanitize_name(const std::string& name) -> std::string {
        std::string result = name;
        for (char& c : result) {
            if (std::isspace(static_cast<unsigned char>(c)) ||
                c == '>' || c == '<' || c == '/' || c == '\\' ||
                c == ':' || c == '*' || c == '?' || c == '"' ||
                c == '|') {
                c = '_';
            }
        }
        return result;
    }
};


/**
 * @brief Simple cutflow with histogram filling (no context)
 *
 * For cases where cuts don't need per-event context.
 */
class SimpleCutflowHistograms {
public:
    using Predicate = std::function<bool()>;
    using Filler = std::function<void(ThreadLocalHistogramGroup*)>;

    explicit SimpleCutflowHistograms(std::string name)
        : m_cutflow(std::move(name)) {}

    auto define(std::string cut_name, Predicate pred) -> SimpleCutflowHistograms& {
        m_cutflow.define(std::move(cut_name), std::move(pred));
        return *this;
    }

    auto define_always(std::string cut_name) -> SimpleCutflowHistograms& {
        m_cutflow.define_always(std::move(cut_name));
        return *this;
    }

    auto set_filler(Filler filler) -> SimpleCutflowHistograms& {
        m_filler = std::move(filler);
        return *this;
    }

    auto initialize(HistogramService* svc, const std::string& base_path) -> bool {
        if (!svc) return false;

        m_service = svc;
        const auto& cuts = m_cutflow.cutflow().cuts();

        for (const auto& cut_name : cuts) {
            std::string sanitized = sanitize_name(cut_name);
            std::string path = fmt::format("{}/{}", base_path, sanitized);
            m_histograms[cut_name] = ThreadLocalHistogramTemplates::ElectronHistograms::create(svc, path);
        }

        m_initialized = true;
        return true;
    }

    [[nodiscard]] auto evaluate_and_fill() -> bool {
        if (!m_cutflow.evaluate()) {
            return false;
        }

        // Fill histograms for all passed cuts
        if (m_filler) {
            const auto& cuts = m_cutflow.cutflow().cuts();
            for (const auto& cut_name : cuts) {
                if (auto* group = histograms(cut_name)) {
                    m_filler(group);
                }
            }
        }

        return true;
    }

    auto print() const -> void { m_cutflow.print(); }

    auto histograms(const std::string& cut_name) -> ThreadLocalHistogramGroup* {
        auto it = m_histograms.find(cut_name);
        return it != m_histograms.end() ? it->second.get() : nullptr;
    }

private:
    AutoCutflow m_cutflow;
    HistogramService* m_service = nullptr;
    Filler m_filler;
    std::map<std::string, std::unique_ptr<ThreadLocalHistogramGroup>> m_histograms;
    bool m_initialized = false;

    static auto sanitize_name(const std::string& name) -> std::string {
        std::string result = name;
        for (char& c : result) {
            if (std::isspace(static_cast<unsigned char>(c)) ||
                c == '>' || c == '<' || c == '/' || c == '\\' ||
                c == ':' || c == '*' || c == '?' || c == '"' ||
                c == '|') {
                c = '_';
            }
        }
        return result;
    }
};

}  // namespace clas12::framework
