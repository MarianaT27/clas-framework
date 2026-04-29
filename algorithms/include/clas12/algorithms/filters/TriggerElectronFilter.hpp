#pragma once

#include <clas12/ReadBank.hpp>
#include <clas12/Target.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for TriggerElectronFilter.
struct TriggerElectronFilterConfig {
    Target target = Target::LD2;
    Polarity polarity = Polarity::Outbending;

    bool enable_trigger_status_cut = true;
    bool enable_chi2pid_cut = true;
    bool enable_momentum_cut = true;
    bool enable_vz_cut = true;

    static constexpr auto config_fields() {
        using T = TriggerElectronFilterConfig;
        return std::make_tuple(
            framework::field("enable_trigger_status_cut", &T::enable_trigger_status_cut),
            framework::field("enable_chi2pid_cut", &T::enable_chi2pid_cut),
            framework::field("enable_momentum_cut", &T::enable_momentum_cut),
            framework::field("enable_vz_cut", &T::enable_vz_cut));
    }
};

/**
 * @brief Filter algorithm for trigger electron selection
 *
 * Makes a yes/no decision about whether an event contains a good trigger
 * electron and publishes the decision plus the selected particle for
 * downstream algorithms.
 */
class TriggerElectronFilter : public framework::Algorithm {
   public:
    using Config = TriggerElectronFilterConfig;
    using StatusCode = framework::StatusCode;

    explicit TriggerElectronFilter(std::string name = "TriggerElectronFilter", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("Trigger Electron Filter") {
        required_services(m_electron_cuts);
        declare_banks("REC::Particle");
        declare_config(m_config);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Target: {}, Polarity: {}", to_string(m_config.target), to_string(m_config.polarity));

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has Electron");
        m_cutflow.add_cut("Trigger Status");
        m_cutflow.add_cut("Chi2 PID");
        m_cutflow.add_cut("Momentum");
        m_cutflow.add_cut("Vertex Z");

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Finalizing");
        m_cutflow.print();
        Log::info(name(), "Events with trigger electron: {}", m_cutflow.get_count("Vertex Z"));
        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        const auto& electrons = event.electrons();
        if (electrons.empty()) {
            event.set_decision_typed(framework::Decisions::HasTriggerElectron, false);
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("Has Electron");

        auto trigger_e = event.find_trigger_electron({2000, 4000});
        if (m_config.enable_trigger_status_cut && !trigger_e.has_value()) {
            event.set_decision_typed(framework::Decisions::HasTriggerElectron, false);
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("Trigger Status");

        auto electron = trigger_e.has_value() ? trigger_e.value() : electrons.front();

        if (m_config.enable_chi2pid_cut && !m_electron_cuts->pass_chi2pid_cut(electron.chi2pid())) {
            event.set_decision_typed(framework::Decisions::HasTriggerElectron, false);
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("Chi2 PID");

        if (m_config.enable_momentum_cut && !m_electron_cuts->pass_momentum_cut(electron.p())) {
            event.set_decision_typed(framework::Decisions::HasTriggerElectron, false);
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("Momentum");

        if (m_config.enable_vz_cut && !m_electron_cuts->pass_vz_cut(electron.vz(), m_config.target)) {
            event.set_decision_typed(framework::Decisions::HasTriggerElectron, false);
            return StatusCode::FILTERED();
        }
        m_cutflow.pass("Vertex Z");

        event.store_typed(framework::Keys::TriggerElectron, electron);
        event.set_decision_typed(framework::Decisions::HasTriggerElectron, true);
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    framework::Cutflow m_cutflow;

framework::ServiceRef<framework::ElectronCutsService> m_electron_cuts;
};

}  // namespace algorithms
