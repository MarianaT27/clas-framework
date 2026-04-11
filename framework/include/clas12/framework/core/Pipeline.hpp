#pragma once

/**
 * @file Pipeline.hpp
 * @brief Declarative event + per-particle pipelines.
 *
 * Two thin fluent builders replace the usual imperative
 * `if (!decision) return FILTERED(); ... m_cutflow.pass(...); ...`
 * body in algorithms. They match the shape of RDataFrame / coffea
 * processors without the JIT / awkward-array machinery: everything
 * runs eagerly in plain C++.
 *
 * `EventPipeline` is for event-level flow. It short-circuits on the
 * first failing filter and exposes the outcome via `.status()`:
 *
 * @code
 * return pipeline(event, m_cutflow)
 *     .require(Decisions::HasGoodDISElectron)
 *     .filter("Q2 Cut", kin.q2 > 1.0)
 *     .filter("W Cut",  kin.w  > 2.0)
 *     .tap([&]{
 *         h_Q2->fill(kin.q2);
 *         h_W->fill(kin.w);
 *     })
 *     .publish(Keys::DIS_Q2, kin.q2)
 *     .publish(Keys::DIS_W,  kin.w)
 *     .set_decision(Decisions::HasGoodDISElectron, true)
 *     .status();
 * @endcode
 *
 * `ParticlePipeline` is for per-particle flow. Filters, counts, and
 * taps are accumulated lazily on a stage stack; the input is walked
 * once when a terminal (`collect` / `run`) is called. Cutflow is
 * incremented per stage per surviving particle:
 *
 * @code
 * auto selected = particles(candidates, m_cutflow)
 *     .count("AI / Energy Cut")
 *     .filter("PCAL Fiducial", [&](const auto& p) { ... })
 *     .filter("Beta Cut",      [&](const auto& p) { ... })
 *     .tap([&](const auto& p) { h_energy_selected->fill(p.E()); })
 *     .collect();
 * @endcode
 *
 * The two types compose naturally: an algorithm's `process()` can
 * use both, passing the collected survivors from a `ParticlePipeline`
 * into an `EventPipeline::publish`, etc.
 */

#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

namespace clas12::framework {

// =============================================================================
// EventPipeline
// =============================================================================

/**
 * @brief Declarative event-level pipeline.
 *
 * Chains `require` / `filter` / `histo1D` / `histo2D` / `tap` /
 * `publish` / `set_decision` / `status` on a single `Event&`. Any
 * failing filter (or missing decision) flips the pipeline into a
 * filtered state; subsequent calls silently no-op and the terminal
 * `.status()` returns `StatusCode::FILTERED()`.
 *
 * Evaluation is eager. `filter` takes a plain bool, not a lambda,
 * because at call time the user already has the values in scope —
 * we only need the short-circuit + cutflow-bookkeeping wrapper.
 */
class EventPipeline {
   public:
    EventPipeline(Event& evt, Cutflow* cf = nullptr) noexcept
        : m_event(&evt), m_cutflow(cf) {}

    /// True while no prior stage has filtered the event out.
    [[nodiscard]] auto passed() const noexcept -> bool { return !m_filtered; }

    /// Terminal: convert to a StatusCode for the enclosing `process()`.
    [[nodiscard]] auto status() const noexcept -> StatusCode {
        return m_filtered ? StatusCode::FILTERED() : StatusCode::SUCCESS();
    }

    /**
     * @brief Require a typed decision set by an upstream algorithm.
     *
     * Filters the event out if the decision is not present or is false.
     */
    auto require(TypedKey<bool> decision) -> EventPipeline& {
        if (m_filtered) return *this;
        if (!m_event->passes_decision_typed(decision)) m_filtered = true;
        return *this;
    }

    /**
     * @brief Boolean filter with a cutflow label.
     *
     * On pass, increments the cutflow counter for `label` and returns.
     * On fail, flips the pipeline to the filtered state.
     */
    auto filter(std::string_view label, bool condition) -> EventPipeline& {
        if (m_filtered) return *this;
        if (!condition) {
            m_filtered = true;
            return *this;
        }
        if (m_cutflow) m_cutflow->pass(std::string(label));
        return *this;
    }

    /**
     * @brief Mark a cutflow step unconditionally (no filtering).
     *
     * Useful for the initial "All Events" step in a cutflow and for
     * counting survivors of an upstream prerequisite.
     */
    auto count(std::string_view label) -> EventPipeline& {
        if (m_filtered) return *this;
        if (m_cutflow) m_cutflow->pass(std::string(label));
        return *this;
    }

    /// Fill a 1D histogram with `value`. No-op when filtered or when `h` is null.
    auto histo1D(ThreadLocalHistogram1D* h, double value) -> EventPipeline& {
        if (m_filtered || !h) return *this;
        h->fill(value);
        return *this;
    }

    /// Weighted 1D fill.
    auto histo1D(ThreadLocalHistogram1D* h, double value, double weight) -> EventPipeline& {
        if (m_filtered || !h) return *this;
        h->fill(value, weight);
        return *this;
    }

    /// Fill a 2D histogram.
    auto histo2D(ThreadLocalHistogram2D* h, double x, double y) -> EventPipeline& {
        if (m_filtered || !h) return *this;
        h->fill(x, y);
        return *this;
    }

    /// Weighted 2D fill.
    auto histo2D(ThreadLocalHistogram2D* h, double x, double y, double weight) -> EventPipeline& {
        if (m_filtered || !h) return *this;
        h->fill(x, y, weight);
        return *this;
    }

    /**
     * @brief Run a side-effect closure when not filtered.
     *
     * Use for batched work that doesn't slot into `histo1D` / `publish`
     * (e.g. filling an ntuple, writing to a log, iterating over a group
     * of histograms in one block).
     */
    template <typename Fn>
    auto tap(Fn&& fn) -> EventPipeline& {
        if (m_filtered) return *this;
        fn();
        return *this;
    }

    /// Store a typed value in the event store (no-op when filtered).
    template <typename T>
    auto publish(TypedKey<T> key, T value) -> EventPipeline& {
        if (m_filtered) return *this;
        m_event->store_typed(key, std::move(value));
        return *this;
    }

    /// Set a typed decision flag (no-op when filtered).
    auto set_decision(TypedKey<bool> key, bool passed) -> EventPipeline& {
        if (m_filtered) return *this;
        m_event->set_decision_typed(key, passed);
        return *this;
    }

    /// Access the underlying event (advanced use).
    [[nodiscard]] auto event() -> Event& { return *m_event; }

   private:
    Event*   m_event    = nullptr;
    Cutflow* m_cutflow  = nullptr;
    bool     m_filtered = false;
};

inline auto pipeline(Event& event, Cutflow& cf) -> EventPipeline {
    return EventPipeline(event, &cf);
}

inline auto pipeline(Event& event) -> EventPipeline {
    return EventPipeline(event, nullptr);
}

// =============================================================================
// ParticlePipeline
// =============================================================================

/**
 * @brief Lazy per-particle pipeline with cutflow, filters, and taps.
 *
 * Wraps a `std::span<const clas12::Particle>` plus an optional
 * `Cutflow*`. Each builder method pushes one `Stage` onto an internal
 * stack - nothing runs until a terminal call. Terminals walk the
 * input once, applying every stage per particle in order.
 *
 * Stage kinds:
 *  - `count(label)`   : increment `label` per particle reaching this stage.
 *  - `filter(label, pred)` : drop particles failing `pred`; on pass,
 *                            increment `label`.
 *  - `tap(fn)`        : side-effect per surviving particle (no cutflow).
 *
 * Terminal kinds:
 *  - `collect()` : return the surviving particles as a
 *                  `std::vector<clas12::Particle>`.
 *  - `run()`     : walk the pipeline without materialising; return
 *                  the survivor count.
 *
 * Cutflow semantics: the cutflow label for a `count` stage counts
 * every particle that makes it to that stage, and for a `filter`
 * stage counts every particle that passes the predicate. That
 * matches the "pass X cut" convention used elsewhere in the
 * framework.
 */
class ParticlePipeline {
   public:
    using Predicate = std::function<bool(const clas12::Particle&)>;
    using Tap       = std::function<void(const clas12::Particle&)>;

    ParticlePipeline(std::span<const clas12::Particle> input, Cutflow* cf = nullptr)
        : m_input(input), m_cutflow(cf) {}

    /// Unconditional per-particle cutflow bump.
    auto count(std::string label) -> ParticlePipeline& {
        m_stages.push_back(Stage{StageKind::Count, std::move(label), {}, {}});
        return *this;
    }

    /// Predicate filter with cutflow label.
    auto filter(std::string label, Predicate pred) -> ParticlePipeline& {
        m_stages.push_back(Stage{StageKind::Filter, std::move(label), std::move(pred), {}});
        return *this;
    }

    /// Per-survivor side effect.
    auto tap(Tap fn) -> ParticlePipeline& {
        m_stages.push_back(Stage{StageKind::Tap, {}, {}, std::move(fn)});
        return *this;
    }

    /// Terminal: materialise the surviving particles.
    [[nodiscard]] auto collect() -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> out;
        out.reserve(m_input.size());
        for (const auto& p : m_input) {
            if (run_stages(p)) out.push_back(p);
        }
        return out;
    }

    /// Terminal: walk the pipeline without collecting; return survivor count.
    auto run() -> std::size_t {
        std::size_t n = 0;
        for (const auto& p : m_input) {
            if (run_stages(p)) ++n;
        }
        return n;
    }

    /// Terminal: return true iff at least one particle survives.
    [[nodiscard]] auto any() -> bool {
        for (const auto& p : m_input) {
            if (run_stages(p)) return true;
        }
        return false;
    }

   private:
    enum class StageKind { Count, Filter, Tap };

    struct Stage {
        StageKind    kind;
        std::string  label;
        Predicate    pred;
        Tap          tap;
    };

    /// Walk every stage in order for one particle. Returns false as
    /// soon as a filter predicate rejects.
    auto run_stages(const clas12::Particle& p) -> bool {
        for (const auto& s : m_stages) {
            switch (s.kind) {
                case StageKind::Count:
                    if (m_cutflow) m_cutflow->pass(s.label);
                    break;
                case StageKind::Filter:
                    if (!s.pred(p)) return false;
                    if (m_cutflow) m_cutflow->pass(s.label);
                    break;
                case StageKind::Tap:
                    s.tap(p);
                    break;
            }
        }
        return true;
    }

    std::span<const clas12::Particle> m_input;
    Cutflow*                          m_cutflow = nullptr;
    std::vector<Stage>                m_stages;
};

inline auto particles(std::span<const clas12::Particle> input,
                      Cutflow& cf) -> ParticlePipeline {
    return ParticlePipeline(input, &cf);
}

inline auto particles(std::span<const clas12::Particle> input) -> ParticlePipeline {
    return ParticlePipeline(input, nullptr);
}

}  // namespace clas12::framework
