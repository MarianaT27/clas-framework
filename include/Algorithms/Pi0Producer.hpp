#pragma once

#include "Framework/AlgorithmTypes.hpp"
#include "Physics/CompositeParticle.hpp"
#include "Core/PIDs.hpp"
#include <vector>

namespace Algorithms {

/**
 * @brief Producer algorithm for π⁰ reconstruction
 *
 * This is a ProducerAlgorithm that reconstructs neutral pions from
 * two-photon combinations and stores them in the event for downstream use.
 *
 * Usage:
 * @code
 * auto producer = std::make_unique<Pi0Producer>("Pi0Producer");
 * app.add_algorithm(std::move(producer));
 *
 * // In downstream algorithm:
 * auto pi0s = event.retrieve<std::vector<Physics::CompositeParticle>>("Pi0Candidates");
 * if (pi0s.has_value()) {
 *     for (const auto& pi0 : pi0s.value()) {
 *         // Use reconstructed π⁰
 *     }
 * }
 * @endcode
 */
class Pi0Producer : public Framework::ProducerAlgorithm {
public:
    explicit Pi0Producer(
        std::string name = "Pi0Producer",
        double mass_window_center = 0.135,  // GeV/c²
        double mass_window_width = 0.050)   // GeV/c²
        : ProducerAlgorithm(std::move(name))
        , m_mass_center(mass_window_center)
        , m_mass_width(mass_window_width) {}

    auto initialize() -> Framework::StatusCode override {
        fmt::print("  {} - Initializing\n", name());
        fmt::print("    π⁰ mass window: {:.3f} ± {:.3f} GeV/c²\n", m_mass_center, m_mass_width);
        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        fmt::print("\nπ⁰ Producer Statistics:\n");
        fmt::print("  Events processed: {}\n", m_events_processed);
        fmt::print("  π⁰ candidates produced: {}\n", m_pi0_produced);
        fmt::print("  Average per event: {:.2f}\n",
                   m_events_processed > 0 ? double(m_pi0_produced) / m_events_processed : 0.0);
        return Framework::StatusCode::SUCCESS();
    }

protected:
    /**
     * @brief Produce π⁰ candidates from photon pairs
     */
    auto produce(Framework::Event& event) -> Framework::StatusCode override {
        m_events_processed++;

        auto& hipo_event = event.hipo_event();
        hipo::bank_map REC_Particle = hipo_event.get_bank_map("REC::Particle");

        // Extract photons
        Core::ParticleMap particles = Core::ParticleSelector::extract_particles(
            REC_Particle, {Core::PIDs::PHOTON});

        const auto& photons = particles[Core::PIDs::PHOTON];

        // Reconstruct π⁰ candidates
        auto pi0_candidates = Physics::CompositeParticleBuilder::reconstruct_pi0(photons);

        // Filter by mass window
        std::vector<Physics::CompositeParticle> good_pi0s;
        for (const auto& pi0 : pi0_candidates) {
            if (pi0.in_mass_window(m_mass_center, m_mass_width)) {
                good_pi0s.push_back(pi0);
            }
        }

        m_pi0_produced += good_pi0s.size();

        // Store π⁰ candidates in event for downstream algorithms
        event.store("Pi0Candidates", good_pi0s);

        // Also set a decision if we found at least one π⁰
        if (!good_pi0s.empty()) {
            event.set_decision("HasPi0", true);
        }

        return Framework::StatusCode::SUCCESS();
    }

private:
    double m_mass_center;
    double m_mass_width;
    size_t m_events_processed = 0;
    size_t m_pi0_produced = 0;
};

}  // namespace Algorithms
