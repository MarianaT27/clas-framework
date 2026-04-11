#pragma once

#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Pipeline.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>

namespace clas12::algorithms {

/**
 * @brief Filter algorithm for trigger electron selection
 *
 * This algorithm makes a yes/no decision about whether an event contains
 * a good trigger electron. It sets the decision for downstream algorithms.
 *
 * Usage:
 * @code
 * auto filter = std::make_unique<TriggerElectronFilter>("TriggerFilter");
 * app.add_algorithm(std::move(filter));
 * // Decision "TriggerFilter" is set (true/false)
 * @endcode
 */
class TriggerElectronFilter : public clas12::framework::SimpleAlgorithm {
   public:
    explicit TriggerElectronFilter(
        std::string name = "TriggerElectronFilter",
        clas12::Target target = clas12::Target::LD2,
        clas12::Polarity /*polarity*/ = clas12::Polarity::Outbending)
        : SimpleAlgorithm(std::move(name)), m_target(target), m_cutflow("Trigger Electron Filter") {
        declare_property(m_enable_trigger_status_cut);
        declare_property(m_enable_chi2pid_cut);
        declare_property(m_enable_momentum_cut);
        declare_property(m_enable_vz_cut);
        declare_bank("REC::Particle");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_particle = svc<clas12::framework::ParticleSelectorService>();
        m_electron_cuts = svc<clas12::framework::ElectronCutsService>();
        if (!m_electron_cuts) {
            clas12::framework::Log::error(name(), "ElectronCutsService not registered");
            return clas12::framework::StatusCode::FAILURE();
        }

        fmt::print("  {} - Initializing\n", name());

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Trigger Status");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum");
        m_cutflow.add_cut("Vertex Z");

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        fmt::print("\n");
        m_cutflow.print();
        fmt::print("\nTrigger Electron Filter Statistics:\n");
        fmt::print("  Events with trigger electron: {}\n", m_cutflow.get_count("Vertex Z"));
        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Filter logic - check if event has good trigger electron.
     *
     * Uses the declarative `EventPipeline` builder: every filter step
     * short-circuits the chain on failure and `.status()` returns the
     * right StatusCode. The "set decision = false on any failure"
     * behaviour is handled by a single `.set_decision(..., false)` up
     * front that is overridden later on success.
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        using namespace clas12::framework;

        // Default decision: filtered out. Overwritten to `true` below
        // if every filter stage passes.
        event.set_decision_typed(Decisions::HasTriggerElectron, false);

        const auto& REC_Particle = event.get_bank("REC::Particle");
        auto particles_map = m_particle->extract_particles(REC_Particle, {clas12::PIDs::ELECTRON});
        const auto& electrons = m_particle->get_electrons(particles_map);

        auto trigger_e = !electrons.empty()
            ? clas12::physics::ParticleUtils::find_trigger_electron(electrons, {2000, 4000})
            : std::optional<clas12::Particle>{};

        // When trigger-status cut is disabled we fall back to the first
        // electron; otherwise the filter below drops the event if the
        // trigger electron isn't found.
        clas12::Particle electron = (trigger_e.has_value())
            ? *trigger_e
            : (electrons.empty() ? clas12::Particle{} : electrons.front());

        return pipeline(event, m_cutflow)
            .count("All Events")
            .filter("Has Electron",    !electrons.empty())
            .filter("Trigger Status",  !m_enable_trigger_status_cut.get() || trigger_e.has_value())
            .filter("Chi2 PID",        !m_enable_chi2pid_cut.get()
                                          || m_electron_cuts->pass_chi2pid_cut(electron.chi2pid()))
            .filter("Momentum",        !m_enable_momentum_cut.get()
                                          || m_electron_cuts->pass_momentum_cut(electron.p()))
            .filter("Vertex Z",        !m_enable_vz_cut.get()
                                          || m_electron_cuts->pass_vz_cut(electron.vz(), m_target))
            .publish(Keys::TriggerElectron, electron)
            .set_decision(Decisions::HasTriggerElectron, true)
            .status();
    }

   private:
    clas12::Target m_target;
    clas12::framework::Cutflow m_cutflow;
    clas12::framework::ParticleSelectorService* m_particle = nullptr;
    clas12::framework::ElectronCutsService* m_electron_cuts = nullptr;

    // Cut toggle properties
    clas12::framework::Property<bool> m_enable_trigger_status_cut{"enable_trigger_status_cut", true, "Enable trigger status cut"};
    clas12::framework::Property<bool> m_enable_chi2pid_cut{"enable_chi2pid_cut", true, "Enable chi2 PID cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
};

}  // namespace clas12::algorithms
