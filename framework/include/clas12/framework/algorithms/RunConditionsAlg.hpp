#pragma once

/**
 * @file RunConditionsAlg.hpp
 * @brief First-in-chain algorithm that resolves per-event run conditions.
 *
 * Each event it reads the run number from `RUN::config`, asks
 * RunConditionsService for that run's RCDB conditions (cached per run), merges
 * the analysis-config overrides on top — **the TOML config always wins; RCDB
 * fills the gaps** — and publishes the result into the event store under
 * `RunConditionsData`. Because the run number is re-read every event, a chain
 * that spans several runs is resolved correctly per run: the "multi-run aware"
 * behaviour. Downstream algorithms read it with:
 *
 * @code
 * if (auto rc = event.retrieve_typed(framework::RunConditionsData)) {
 *     double beam = rc->beam_energy.value_or(default_beam);
 * }
 * @endcode
 *
 * The algorithm is a safe no-op when RCDB is not configured (the service is
 * disabled) and/or `RUN::config` is absent: it then publishes whatever the TOML
 * overrides provide (possibly nothing), so existing analyses are unaffected
 * unless an RCDB connection is set.
 */

#include <atomic>
#include <mutex>
#include <string>
#include <utility>

#include <clas12/bankdefs/RUN_config.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/framework/services/MetadataService.hpp>
#include <clas12/framework/services/RunConditionsService.hpp>

namespace framework {

/// Shared event-store key carrying the per-event resolved run conditions.
/// Producers (RunConditionsAlg) write it; consumers read it via
/// `event.retrieve_typed(RunConditionsData)`.
inline constexpr TypedKey<RunConditions> RunConditionsData{"run_conditions"};

class RunConditionsAlg : public Algorithm {
   public:
    /// @param overrides Analysis-config values that win over RCDB (TOML wins).
    /// @param name      Algorithm instance name (default "RunConditions").
    explicit RunConditionsAlg(RunConditionsOverrides overrides = {},
                              std::string name = "RunConditions")
        : Algorithm(std::move(name)), m_overrides(std::move(overrides)) {
        declare_bank("RUN::config");
    }

    auto on_initialize() -> StatusCode override {
        m_rcdb = svc<RunConditionsService>();
        m_meta = svc<MetadataService>();
        if (!m_rcdb) {
            log_warn(
                "RunConditionsService not available; run conditions will come "
                "only from the analysis config.");
        } else if (!m_rcdb->is_open()) {
            log_info(
                "RCDB snapshot not open; run conditions will come only from the "
                "analysis config.");
        }
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        // Run number from RUN::config row 0. A missing bank / empty rows leaves
        // run == 0, for which RCDB has no conditions (a clean no-op).
        bankdefs::RUN_config cfg;
        cfg.bind(event.get_bank("RUN::config"));
        const int run = cfg.rows() > 0 ? cfg.run(0) : 0;

        RunConditions rc;
        rc.run = run;
        if (m_rcdb) rc = m_rcdb->conditions(run);  // cached per run, thread-safe
        rc = apply_overrides(std::move(rc), m_overrides);  // TOML wins
        rc.run = run;

        event.store_typed(RunConditionsData, rc);

        // Log once per distinct run number seen (cheap, benign under races).
        if (m_last_run.exchange(run, std::memory_order_relaxed) != run) {
            log_info(
                "Run {} conditions -> beam_energy={} GeV, target={}, "
                "torus={}, solenoid={}",
                run, rc.beam_energy.value_or(0.0), rc.target.value_or("(unset)"),
                rc.torus.value_or(0.0), rc.solenoid.value_or(0.0));
        }

        // Populate MetadataService once (first resolved run), MT-safely, so
        // existing algorithms that read beam energy / target from metadata pick
        // up the RCDB-sourced values. For per-event-exact values across a
        // multi-run chain, read RunConditionsData from the event store instead.
        if (m_meta) {
            std::call_once(m_meta_once, [&] {
                m_meta->set_run_number(run);
                if (rc.beam_energy) m_meta->set_beam_energy(*rc.beam_energy);
                if (rc.target) m_meta->set_target(*rc.target);
            });
        }

        return StatusCode::SUCCESS();
    }

   private:
    RunConditionsOverrides m_overrides;
    RunConditionsService* m_rcdb = nullptr;
    MetadataService* m_meta = nullptr;
    std::atomic<int> m_last_run{-1};
    std::once_flag m_meta_once;
};

}  // namespace framework
