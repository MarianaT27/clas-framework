#pragma once

#include <atomic>
#include <vector>
#include <clas12/PIDs.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/physics/CompositeParticle.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

/// Configuration for Pi0Producer.
struct Pi0ProducerConfig {
    double mass_window_center = 0.135;  // GeV/c²
    double mass_window_width = 0.050;   // GeV/c²
};

/**
 * @brief Producer algorithm for π⁰ reconstruction
 *
 * Reconstructs neutral pions from two-photon combinations within a mass
 * window and stores them in the event for downstream use.
 */
class Pi0Producer : public clas12::framework::Algorithm {
   public:
    using Config = Pi0ProducerConfig;
    using StatusCode = clas12::framework::StatusCode;

    explicit Pi0Producer(std::string name = "Pi0Producer", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        declare_banks("REC::Particle");
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "π⁰ mass window: {:.3f} ± {:.3f} GeV/c²", m_config.mass_window_center, m_config.mass_window_width);
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        const auto events = m_events_processed.load(std::memory_order_relaxed);
        const auto produced = m_pi0_produced.load(std::memory_order_relaxed);
        Log::info(name(), "π⁰ Producer Statistics:");
        Log::info(name(), "  Events processed: {}", events);
        Log::info(name(), "  π⁰ candidates produced: {}", produced);
        Log::info(name(), "  Average per event: {:.2f}", events > 0 ? double(produced) / events : 0.0);
        return StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> StatusCode override {
        m_events_processed.fetch_add(1, std::memory_order_relaxed);

        auto pi0_candidates = clas12::physics::CompositeParticleBuilder::reconstruct_pi0(event.photons());

        std::vector<clas12::physics::CompositeParticle> good_pi0s;
        for (const auto& pi0 : pi0_candidates) {
            if (pi0.in_mass_window(m_config.mass_window_center, m_config.mass_window_width)) {
                good_pi0s.push_back(pi0);
            }
        }

        m_pi0_produced.fetch_add(good_pi0s.size(), std::memory_order_relaxed);

        event.store_typed(clas12::framework::Keys::Pi0Candidates, good_pi0s);
        if (!good_pi0s.empty()) {
            event.set_decision_typed(clas12::framework::Decisions::HasPi0, true);
        }

        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;
    std::atomic<size_t> m_events_processed{0};
    std::atomic<size_t> m_pi0_produced{0};
};

}  // namespace clas12::algorithms
