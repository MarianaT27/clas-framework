#pragma once

#include <hipo4/hipoeventiterator.h>
#include <Core/Banks.hpp>
#include <Core/PIDs.hpp>
#include <Core/Particle.hpp>
#include <Core/ParticleSelector.hpp>
#include <Core/ReadBank.hpp>
#include <memory>
#include <optional>

namespace Core {

/**
 * @brief Wrapper class for easier access to event data
 *
 * This class provides a clean interface for accessing particles and detector banks
 * from a HIPO event, with caching for performance.
 */
class EventData {
   public:
    explicit EventData(hipo::iter_event& event, const std::vector<Core::PIDs>& types = {}) : m_event(event) {
        hipo::bank& REC_Particle = m_event.get_bank("REC::Particle");
        m_particles = ParticleSelector::extract_particles(REC_Particle, types);
        m_particles_cached = true;
    }

    /**
     * @brief Get all particles grouped by type (cached)
     */
    const ParticleMap& particles(const std::vector<Core::PIDs>& types = {}) {
        if (!m_particles_cached) {
            auto& REC_Particle = m_event.get_bank("REC::Particle");
            m_particles = ParticleSelector::extract_particles(REC_Particle, types);
            m_particles_cached = true;
        }
        return m_particles;
    }

    /**
     * @brief Get particles of a specific type
     */
    const std::vector<Core::Particle>& particles(PIDs type) {
        const auto& particle_map = particles();
        auto it = particle_map.find(type);
        if (it != particle_map.end()) {
            return it->second;
        }
        static const std::vector<Core::Particle> empty_vector;
        return empty_vector;
    }

    /**
     * @brief Get electrons from the event
     */
    const std::vector<Core::Particle>& electrons() {
        return particles(PIDs::Electron);
    }

    /**
     * @brief Get protons from the event
     */
    const std::vector<Core::Particle>& protons() {
        return particles(PIDs::Proton);
    }

    /**
     * @brief Get positive pions from the event
     */
    const std::vector<Core::Particle>& pions_plus() {
        return particles(PIDs::PionPlus);
    }

    /**
     * @brief Get negative pions from the event
     */
    const std::vector<Core::Particle>& pions_minus() {
        return particles(PIDs::PionMinus);
    }

    /**
     * @brief Get positive kaons from the event
     */
    const std::vector<Core::Particle>& kaons_plus() {
        return particles(PIDs::KaonPlus);
    }

    /**
     * @brief Get negative kaons from the event
     */
    const std::vector<Core::Particle>& kaons_minus() {
        return particles(PIDs::KaonMinus);
    }

    /**
     * @brief Get trigger electron (cached)
     */
    std::optional<Core::Particle> trigger_electron(std::pair<double, double> status_range = {2000, 4000}) {
        if (!m_trigger_electron_cached) {
            m_trigger_electron = Core::find_trigger_electron(electrons(), status_range);
            m_trigger_electron_cached = true;
        }
        return m_trigger_electron;
    }

    /**
     * @brief Read calorimeter data for a particle
     */
    CalorimeterBank calorimeter(const Core::Particle& particle) {
        return Core::read_Calorimeter_bank(*REC_Calorimeter().get_bank(), particle.index());
    }

    /**
     * @brief Read Cherenkov data for a particle
     */
    CherenkovBank cherenkov(const Core::Particle& particle) {
        return Core::read_Cherenkov_bank(*REC_Cherenkov().get_bank(), particle.index());
    }

    /**
     * @brief Read track data for a particle
     */
    TrackBank track(const Core::Particle& particle) {
        return Core::read_Track_bank(*REC_Track().get_bank(), particle.index());
    }

    /**
     * @brief Read trajectory data for a particle
     */
    DCTrajBank trajectory(const Core::Particle& particle) {
        return Core::read_Traj_bank(*REC_Traj().get_bank(), particle.index());
    }

    /**
     * @brief Read scintillator data for a particle
     */
    ScintillatorBank scintillator(const Core::Particle& particle) {
        return Core::read_Scintillator_bank(*REC_Scintillator().get_bank(), particle.index());
    }

    /**
     * @brief Check if event has particles
     */
    bool has_particles() const {
        return m_event.get_bank("REC::Particle").getRows() > 0;
    }

    /**
     * @brief Get number of particles in event
     */
    int particle_count() const {
        return m_event.get_bank("REC::Particle").getRows();
    }

    /**
     * @brief Access to underlying event
     */
    hipo::iter_event& event() { return m_event; }

    /**
     * @brief Clear all caches (call for new event)
     */
    void reset() {
        m_particles_cached = false;
        m_trigger_electron_cached = false;
        m_particles.clear();
        m_trigger_electron = std::nullopt;
    }

   private:
    // Bank accessors (lazy loaded)
    hipo::bank_map REC_Particle() { return m_event.get_bank_map("REC::Particle"); }

    hipo::bank_map REC_Calorimeter() { return m_event.get_bank_map("REC::Calorimeter"); }

    hipo::bank_map REC_Cherenkov() { return m_event.get_bank_map("REC::Cherenkov"); }

    hipo::bank_map REC_Track() { return m_event.get_bank_map("REC::Track"); }

    hipo::bank_map REC_Traj() { return m_event.get_bank_map("REC::Traj"); }

    hipo::bank_map REC_Scintillator() { return m_event.get_bank_map("REC::Scintillator"); }

    hipo::iter_event& m_event;

    // Cached data
    ParticleMap m_particles;
    bool m_particles_cached = false;

    std::optional<Core::Particle> m_trigger_electron;
    bool m_trigger_electron_cached = false;
};

}  // namespace Core
