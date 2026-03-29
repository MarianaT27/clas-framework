#pragma once

#include <clas12/ElectronCuts.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
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
class TriggerElectronFilter : public clas12::framework::SimpleAlgorithm<TriggerElectronFilter> {
   public:
    explicit TriggerElectronFilter(
        std::string name = "TriggerElectronFilter",
        clas12::Target target = clas12::Target::LD2,
        clas12::Polarity polarity = clas12::Polarity::Outbending)
        : SimpleAlgorithm(std::move(name)), m_target(target), m_polarity(polarity), m_cutflow("Trigger Electron Filter") {
        declare_property(m_enable_trigger_status_cut);
        declare_property(m_enable_chi2pid_cut);
        declare_property(m_enable_momentum_cut);
        declare_property(m_enable_vz_cut);
        declare_bank("REC::Particle");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_particle = svc<clas12::framework::ParticleSelectorService>();

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
     * @brief Filter logic - check if event has good trigger electron
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_cutflow.pass("All Events");

        const auto& REC_Particle = event.get_bank("REC::Particle");

        // Extract electrons using ParticleSelectorService
        clas12::framework::ParticleMap particles = m_particle->extract_particles(
            REC_Particle, {clas12::PIDs::ELECTRON});

        const auto& electrons = m_particle->get_electrons(particles);
        if (electrons.empty()) {
            event.set_decision_typed(clas12::framework::Decisions::HasTriggerElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        // Find trigger electron
        auto trigger_e = clas12::physics::ParticleUtils::find_trigger_electron(electrons, {2000, 4000});
        if (m_enable_trigger_status_cut.get() && !trigger_e.has_value()) {
            event.set_decision_typed(clas12::framework::Decisions::HasTriggerElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Trigger Status");

        // Use trigger electron if found, otherwise use first electron
        auto electron = trigger_e.has_value() ? trigger_e.value() : electrons.front();

        // Apply basic cuts
        if (m_enable_chi2pid_cut.get() && !clas12::ElectronCuts::pass_chi2pid_cut(electron.chi2pid())) {
            event.set_decision_typed(clas12::framework::Decisions::HasTriggerElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Chi2 PID");

        if (m_enable_momentum_cut.get() && !clas12::ElectronCuts::pass_electron_momentum_cut(electron.p())) {
            event.set_decision_typed(clas12::framework::Decisions::HasTriggerElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Momentum");

        if (m_enable_vz_cut.get() && !clas12::ElectronCuts::pass_electron_vz_cut(electron.vz(), m_target)) {
            event.set_decision_typed(clas12::framework::Decisions::HasTriggerElectron, false);
            return clas12::framework::StatusCode::FILTERED();
        }
        m_cutflow.pass("Vertex Z");

        // Store the trigger electron for downstream use
        event.store_typed(clas12::framework::Keys::TriggerElectron, electron);

        // Event passes filter
        event.set_decision_typed(clas12::framework::Decisions::HasTriggerElectron, true);
        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    clas12::Target m_target;
    clas12::Polarity m_polarity;
    clas12::framework::Cutflow m_cutflow;
    clas12::framework::ParticleSelectorService* m_particle = nullptr;

    // Cut toggle properties
    clas12::framework::Property<bool> m_enable_trigger_status_cut{"enable_trigger_status_cut", true, "Enable trigger status cut"};
    clas12::framework::Property<bool> m_enable_chi2pid_cut{"enable_chi2pid_cut", true, "Enable chi2 PID cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_vz_cut{"enable_vz_cut", true, "Enable vertex Z cut"};
};

}  // namespace clas12::algorithms
