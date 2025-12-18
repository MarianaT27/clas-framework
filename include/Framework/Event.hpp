#pragma once

#include <hipo4/hipoeventiterator.h>
#include <CLAS/PIDs.hpp>
#include <CLAS/Particle.hpp>
#include <Physics/ParticleUtils.hpp>

#include <any>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Framework {

/**
 * @brief Type alias for storing particles grouped by their PID type
 */
using ParticleMap = std::map<CLAS::PIDs, std::vector<CLAS::Particle>>;

/**
 * @brief Event Data Model for CLAS12 analysis
 *
 * This class provides a high-level interface to event data, abstracting
 * away the details of HIPO bank access. It follows the Event Data Model
 * pattern used in ATLAS (xAOD) and CMS (EDM).
 *
 * The Event class uses lazy evaluation - data is only loaded from HIPO
 * banks when explicitly requested.
 */
class Event {
   public:
    /**
     * @brief Construct an Event from a HIPO event iterator
     * @param hipo_event Reference to the HIPO event iterator
     */
    explicit Event(hipo::iter_event& hipo_event)
        : m_hipo_event(hipo_event) {}

    /**
     * @brief Get the underlying HIPO event
     * @return hipo::iter_event& Reference to HIPO event
     */
    auto hipo_event() -> hipo::iter_event& {
        return m_hipo_event;
    }

    /**
     * @brief Get all particles (lazy loading)
     * @return const ParticleMap& Map of PID to particle vectors
     */
    auto particles() -> const ParticleMap& {
        if (!m_particles_loaded) {
            load_particles();
        }
        return m_particles;
    }

    /**
     * @brief Get particles of a specific type
     * @param pid Particle ID
     * @return const std::vector<CLAS::Particle>& Vector of particles
     */
    auto particles(CLAS::PIDs pid) -> const std::vector<CLAS::Particle>& {
        if (!m_particles_loaded) {
            load_particles();
        }
        auto it = m_particles.find(pid);
        if (it != m_particles.end()) {
            return it->second;
        }
        static const std::vector<CLAS::Particle> empty;
        return empty;
    }

    /**
     * @brief Get a HIPO bank by name
     * @param bank_name Name of the bank (e.g., "REC::Particle")
     * @return hipo::bank& Reference to the bank
     */
    auto get_bank(const std::string& bank_name) -> hipo::bank& {
        return m_hipo_event.get(bank_name.c_str());
    }

    /**
     * @brief Get event start time
     * @return double Start time in ns
     */
    auto start_time() -> double {
        if (!m_start_time.has_value()) {
            load_event_info();
        }
        return m_start_time.value_or(0.0);
    }

    /**
     * @brief Get event number
     * @return long Event number
     */
    auto event_number() const -> long {
        return m_event_number;
    }

    /**
     * @brief Set event number
     * @param evt_num Event number
     */
    auto set_event_number(long evt_num) -> void {
        m_event_number = evt_num;
    }

    /**
     * @brief Get file index
     * @return int File index
     */
    auto file_index() const -> int {
        return m_file_index;
    }

    /**
     * @brief Set file index
     * @param file_idx File index
     */
    auto set_file_index(int file_idx) -> void {
        m_file_index = file_idx;
    }

    /**
     * @brief Store a custom object in the event
     * @tparam T Object type
     * @param key Storage key
     * @param value Object to store
     */
    template <typename T>
    auto store(const std::string& key, T value) -> void {
        m_store[key] = std::any(std::move(value));
    }

    /**
     * @brief Retrieve a custom object from the event
     * @tparam T Object type
     * @param key Storage key
     * @return std::optional<T> The stored object, or std::nullopt if not found
     */
    template <typename T>
    auto retrieve(const std::string& key) const -> std::optional<T> {
        auto it = m_store.find(key);
        if (it == m_store.end()) {
            return std::nullopt;
        }
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    /**
     * @brief Check if a key exists in the event store
     * @param key Storage key
     * @return bool True if key exists
     */
    auto contains(const std::string& key) const -> bool {
        return m_store.find(key) != m_store.end();
    }

    /**
     * @brief Store a particle collection in the event
     */
    template <typename ParticleType>
    auto store_particles(const std::string& key, std::vector<ParticleType> particles) -> void {
        m_store[key] = std::any(std::move(particles));
    }

    /**
     * @brief Retrieve a particle collection from the event
     */
    template <typename ParticleType>
    auto get_particles(const std::string& key) const -> const std::vector<ParticleType>* {
        auto it = m_store.find(key);
        if (it == m_store.end()) {
            return nullptr;
        }
        return std::any_cast<std::vector<ParticleType>>(&it->second);
    }

    /**
     * @brief Set an algorithm decision flag
     */
    auto set_decision(const std::string& decision_name, bool passed) -> void {
        store(decision_name, passed);
    }

    /**
     * @brief Check if an algorithm decision was passed
     */
    auto passes_decision(const std::string& decision_name) -> bool {
        auto result = retrieve<bool>(decision_name);
        return result.value_or(false);
    }

   private:
    hipo::iter_event& m_hipo_event;
    ParticleMap m_particles;
    bool m_particles_loaded = false;
    std::optional<double> m_start_time;
    long m_event_number = -1;
    int m_file_index = -1;
    std::map<std::string, std::any> m_store;

    /**
     * @brief Load particles from REC::Particle bank
     */
    auto load_particles() -> void {
        hipo::bank& REC_Particle = m_hipo_event.get("REC::Particle");

        // Initialize particle vectors for common types
        std::vector<CLAS::PIDs> target_types = {
            CLAS::PIDs::ELECTRON, CLAS::PIDs::POSITRON,
            CLAS::PIDs::PHOTON,
            CLAS::PIDs::PROTON, CLAS::PIDs::NEUTRON,
            CLAS::PIDs::PIPLUS, CLAS::PIDs::PIMINUS,
            CLAS::PIDs::KPLUS, CLAS::PIDs::KMINUS};

        for (const auto& type : target_types) {
            m_particles[type] = std::vector<CLAS::Particle>();
        }

        // Extract particles from bank
        for (int i = 0; i < REC_Particle.rows(); i++) {
            const int pid = REC_Particle.get<int>("pid", i);

            // Find matching particle type
            CLAS::PIDs particle_type;
            bool found = false;
            for (const auto& type : target_types) {
                if (pid == static_cast<int>(type)) {
                    particle_type = type;
                    found = true;
                    break;
                }
            }

            if (!found) continue;

            const int status = REC_Particle.get<int>("status", i);
            const int charge = REC_Particle.get<int>("charge", i);
            // REC::Particle stores momentum and vertex as floats
            const double px = static_cast<double>(REC_Particle.get<float>("px", i));
            const double py = static_cast<double>(REC_Particle.get<float>("py", i));
            const double pz = static_cast<double>(REC_Particle.get<float>("pz", i));
            const double vx = static_cast<double>(REC_Particle.get<float>("vx", i));
            const double vy = static_cast<double>(REC_Particle.get<float>("vy", i));
            const double vz = static_cast<double>(REC_Particle.get<float>("vz", i));
            const double vt = static_cast<double>(REC_Particle.get<float>("vt", i));
            const double beta = static_cast<double>(REC_Particle.get<float>("beta", i));
            const double chi2pid = static_cast<double>(REC_Particle.get<float>("chi2pid", i));

            const double mass = Physics::ParticleUtils::get_mass(pid);
            const double E = Physics::ParticleUtils::compute_energy(px, py, pz, pid);

            m_particles[particle_type].emplace_back(
                pid, status, i, charge, mass,
                px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
        }

        m_particles_loaded = true;
    }

    auto load_event_info() -> void {
        hipo::bank& REC_Event = m_hipo_event.get("REC::Event");
        if (REC_Event.rows() > 0) {
            m_start_time = REC_Event.get<double>("startTime", 0);
        }
    }
};

}  // namespace Framework
