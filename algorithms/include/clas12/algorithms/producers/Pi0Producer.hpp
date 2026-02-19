#pragma once

#include <vector>
#include <clas12/PIDs.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>
#include <clas12/physics/CompositeParticle.hpp>

namespace clas12::algorithms {

/**
 * @brief Producer algorithm for π⁰ reconstruction
 *
 * This algorithm reconstructs neutral pions from two-photon combinations
 * and stores them in the event for downstream use.
 *
 * Usage:
 * @code
 * auto producer = std::make_unique<Pi0Producer>("Pi0Producer");
 * app.add_algorithm(std::move(producer));
 *
 * // In downstream algorithm:
 * auto pi0s = event.retrieve<std::vector<clas12::physics::CompositeParticle>>("Pi0Candidates");
 * if (pi0s.has_value()) {
 *     for (const auto& pi0 : pi0s.value()) {
 *         // Use reconstructed π⁰
 *     }
 * }
 * @endcode
 */
class Pi0Producer : public clas12::framework::SimpleAlgorithm<Pi0Producer> {
   public:
    explicit Pi0Producer(
        std::string name = "Pi0Producer",
        double mass_window_center = 0.135,  // GeV/c²
        double mass_window_width = 0.050)   // GeV/c²
        : SimpleAlgorithm(std::move(name)), m_mass_center(mass_window_center), m_mass_width(mass_window_width) {
        declare_bank("REC::Particle");
    }

    auto configure() -> void override {
        m_particle_svc = &require_service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        fmt::print("  {} - Initializing\n", name());
        fmt::print("    π⁰ mass window: {:.3f} ± {:.3f} GeV/c²\n", m_mass_center, m_mass_width);
        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        fmt::print("\nπ⁰ Producer Statistics:\n");
        fmt::print("  Events processed: {}\n", m_events_processed);
        fmt::print("  π⁰ candidates produced: {}\n", m_pi0_produced);
        fmt::print("  Average per event: {:.2f}\n",
                   m_events_processed > 0 ? double(m_pi0_produced) / m_events_processed : 0.0);
        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Produce π⁰ candidates from photon pairs
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        m_events_processed++;

        const auto& REC_Particle = event.get_bank("REC::Particle");

        // Extract photons using ParticleSelectorService
        clas12::framework::ParticleMap particles = m_particle_svc->get()->extract_particles(
            REC_Particle, {clas12::PIDs::PHOTON});

        const auto& photons = m_particle_svc->get()->get_photons(particles);

        // Reconstruct π⁰ candidates
        auto pi0_candidates = clas12::physics::CompositeParticleBuilder::reconstruct_pi0(photons);

        // Filter by mass window
        std::vector<clas12::physics::CompositeParticle> good_pi0s;
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

        return clas12::framework::StatusCode::SUCCESS();
    }

   private:
    double m_mass_center;
    double m_mass_width;
    size_t m_events_processed = 0;
    size_t m_pi0_produced = 0;
    clas12::framework::ServiceHandle<clas12::framework::ParticleSelectorService>* m_particle_svc = nullptr;
};

}  // namespace clas12::algorithms
