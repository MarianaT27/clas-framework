#pragma once

#include "Framework/AlgorithmTypes.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "CLAS/ElectronCuts.hpp"
#include "CLAS/ReadBank.hpp"
#include "CLAS/Target.hpp"

namespace Algorithms {

/**
 * @brief Filter algorithm for trigger electron selection
 *
 * This is a FilterAlgorithm that makes a yes/no decision about whether
 * an event contains a good trigger electron. It automatically sets the
 * "HasGoodElectron" decision for downstream algorithms.
 *
 * Usage:
 * @code
 * auto filter = std::make_unique<TriggerElectronFilter>("TriggerFilter");
 * app.add_algorithm(std::move(filter));
 * // Decision "TriggerFilter" is automatically set (true/false)
 * @endcode
 */
class TriggerElectronFilter : public Framework::FilterAlgorithm {
public:
    explicit TriggerElectronFilter(
        std::string name = "TriggerElectronFilter",
        CLAS::Target target = CLAS::Target::LD2,
        CLAS::Polarity polarity = CLAS::Polarity::Outbending)
        : FilterAlgorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow("Trigger Electron Filter") {}

    auto initialize() -> Framework::StatusCode override {
        fmt::print("  {} - Initializing\n", name());

        // Get ParticleSelectorService
        m_particle_svc = service<Framework::ParticleSelectorService>("ParticleSelectorService");

        // Setup cutflow
        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Trigger Status");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum");
        m_cutflow.add_cut("Vertex Z");

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        fmt::print("\n");
        m_cutflow.print();
        fmt::print("\nTrigger Electron Filter Statistics:\n");
        fmt::print("  Events with trigger electron: {}\n", m_cutflow.get_count("Vertex Z"));
        return Framework::StatusCode::SUCCESS();
    }

protected:
    /**
     * @brief Filter logic - check if event has good trigger electron
     */
    auto filter(Framework::Event& event) -> bool override {
        m_cutflow.pass("All Events");

        auto& hipo_event = event.hipo_event();
        auto& REC_Particle = hipo_event["REC::Particle"];

        // Extract electrons using ParticleSelectorService
        Framework::ParticleMap particles = m_particle_svc->extract_particles(
            REC_Particle, {CLAS::PIDs::ELECTRON});

        const auto& electrons = m_particle_svc->get_electrons(particles);
        if (electrons.empty()) return false;
        m_cutflow.pass("Has Electron");

        // Find trigger electron
        auto trigger_e = Physics::ParticleUtils::find_trigger_electron(electrons, {2000, 4000});
        if (!trigger_e.has_value()) return false;
        m_cutflow.pass("Trigger Status");

        auto electron = trigger_e.value();

        // Apply basic cuts
        if (!CLAS::ElectronCuts::pass_chi2pid_cut(electron.chi2pid())) return false;
        m_cutflow.pass("Chi2 PID");

        if (!CLAS::ElectronCuts::pass_electron_momentum_cut(electron.p())) return false;
        m_cutflow.pass("Momentum");

        if (!CLAS::ElectronCuts::pass_electron_vz_cut(electron.vz(), m_target)) return false;
        m_cutflow.pass("Vertex Z");

        // Store the trigger electron for downstream use
        event.store("TriggerElectron", electron);

        return true;  // Event passes filter
    }

private:
    CLAS::Target m_target;
    CLAS::Polarity m_polarity;
    Framework::Cutflow m_cutflow;
    Framework::ParticleSelectorService* m_particle_svc = nullptr;
};

}  // namespace Algorithms
