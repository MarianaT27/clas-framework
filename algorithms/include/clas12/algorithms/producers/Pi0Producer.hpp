#pragma once

#include <vector>
#include <clas12/PIDs.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Pipeline.hpp>
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
class Pi0Producer : public clas12::framework::SimpleAlgorithm {
   public:
    explicit Pi0Producer(
        std::string name = "Pi0Producer",
        double mass_window_center = 0.135,  // GeV/c²
        double mass_window_width = 0.050)   // GeV/c²
        : SimpleAlgorithm(std::move(name)), m_mass_center(mass_window_center), m_mass_width(mass_window_width) {
        declare_bank("REC::Particle");
    }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_particle_svc = svc<clas12::framework::ParticleSelectorService>();

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
     * @brief Produce π⁰ candidates from photon pairs.
     *
     * The heavy lifting (photon pair enumeration, mass-window filter)
     * is delegated to `CompositeParticleBuilder::reconstruct_pi0`
     * followed by an in-place mass-window filter; the pipeline owns
     * the ntuple-style bookkeeping (processed counter, publish,
     * decision flag).
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        using namespace clas12::framework;

        ParticleMap particles = m_particle_svc->extract_particles(
            event.get_bank("REC::Particle"), {clas12::PIDs::PHOTON});
        const auto& photons = m_particle_svc->get_photons(particles);
        auto pi0_candidates = clas12::physics::CompositeParticleBuilder::reconstruct_pi0(photons);

        std::vector<clas12::physics::CompositeParticle> good_pi0s;
        good_pi0s.reserve(pi0_candidates.size());
        for (const auto& pi0 : pi0_candidates) {
            if (pi0.in_mass_window(m_mass_center, m_mass_width)) good_pi0s.push_back(pi0);
        }

        return pipeline(event)
            .tap([&] {
                ++m_events_processed;
                m_pi0_produced += good_pi0s.size();
            })
            .publish(Keys::Pi0Candidates, good_pi0s)
            .set_decision(Decisions::HasPi0, !good_pi0s.empty())
            .status();
    }

   private:
    double m_mass_center;
    double m_mass_width;
    size_t m_events_processed = 0;
    size_t m_pi0_produced = 0;
    clas12::framework::ParticleSelectorService* m_particle_svc = nullptr;
};

}  // namespace clas12::algorithms
