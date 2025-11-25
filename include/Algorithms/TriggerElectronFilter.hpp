#pragma once

#include "Framework/AlgorithmTypes.hpp"
#include "Framework/Cutflow.hpp"
#include "Core/Cuts.hpp"
#include "Core/ReadBank.hpp"
#include "Core/Target.hpp"

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
        Core::Target target = Core::Target::LD2,
        Core::Polarity polarity = Core::Polarity::Outbending)
        : FilterAlgorithm(std::move(name))
        , m_target(target)
        , m_polarity(polarity)
        , m_cutflow("Trigger Electron Filter") {}

    auto initialize() -> Framework::StatusCode override {
        fmt::print("  {} - Initializing\n", name());

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
        hipo::bank_map REC_Particle = hipo_event.get_bank_map("REC::Particle");

        // Extract electrons
        Core::ParticleMap particles = Core::ParticleSelector::extract_particles(
            REC_Particle, {Core::PIDs::ELECTRON});

        const auto& electrons = particles[Core::PIDs::ELECTRON];
        if (electrons.empty()) return false;
        m_cutflow.pass("Has Electron");

        // Find trigger electron
        auto trigger_e = Core::find_trigger_electron(electrons, {2000, 4000});
        if (!trigger_e.has_value()) return false;
        m_cutflow.pass("Trigger Status");

        auto electron = trigger_e.value();

        // Apply basic cuts
        if (!Core::ElectronCuts::pass_chi2pid_cut(electron.chi2pid())) return false;
        m_cutflow.pass("Chi2 PID");

        if (!Core::ElectronCuts::pass_electron_momentum_cut(electron.p())) return false;
        m_cutflow.pass("Momentum");

        if (!Core::ElectronCuts::pass_electron_vz_cut(electron.vz(), m_target)) return false;
        m_cutflow.pass("Vertex Z");

        // Store the trigger electron for downstream use
        event.store("TriggerElectron", electron);

        return true;  // Event passes filter
    }

private:
    Core::Target m_target;
    Core::Polarity m_polarity;
    Framework::Cutflow m_cutflow;
};

}  // namespace Algorithms
