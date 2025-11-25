#pragma once

#include "Core/Particle.hpp"
#include "Core/ParticleSelector.hpp"
#include "Core/PIDs.hpp"
#include <vector>
#include <optional>
#include <memory>
#include "hipoeventiterator.h"

namespace Framework {

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
     * @return const Core::ParticleMap& Map of PID to particle vectors
     */
    auto particles() -> const Core::ParticleMap& {
        if (!m_particles_loaded) {
            load_particles();
        }
        return m_particles;
    }

    /**
     * @brief Get particles of a specific type
     * @param pid Particle ID
     * @return const std::vector<Core::Particle>& Vector of particles
     */
    auto particles(Core::PIDs pid) -> const std::vector<Core::Particle>& {
        if (!m_particles_loaded) {
            load_particles();
        }
        auto it = m_particles.find(pid);
        if (it != m_particles.end()) {
            return it->second;
        }
        static const std::vector<Core::Particle> empty;
        return empty;
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
    template<typename T>
    auto store(const std::string& key, T value) -> void {
        m_store[key] = std::make_shared<T>(std::move(value));
    }

    /**
     * @brief Retrieve a custom object from the event
     * @tparam T Object type
     * @param key Storage key
     * @return std::optional<T> The stored object, or std::nullopt if not found
     */
    template<typename T>
    auto retrieve(const std::string& key) -> std::optional<T> {
        auto it = m_store.find(key);
        if (it == m_store.end()) {
            return std::nullopt;
        }
        auto ptr = std::static_pointer_cast<T>(it->second);
        return *ptr;
    }

    /**
     * @brief Set an algorithm decision flag
     * @param decision_name Name of the decision (e.g., "HasGoodElectron")
     * @param passed Whether the event passed this decision
     *
     * This is a convenience method for algorithms to communicate selection
     * decisions to downstream algorithms.
     */
    auto set_decision(const std::string& decision_name, bool passed) -> void {
        store(decision_name, passed);
    }

    /**
     * @brief Check if an algorithm decision was passed
     * @param decision_name Name of the decision to check
     * @return bool True if the decision exists and was passed, false otherwise
     */
    auto passes_decision(const std::string& decision_name) -> bool {
        auto result = retrieve<bool>(decision_name);
        return result.value_or(false);
    }

private:
    hipo::iter_event& m_hipo_event;
    Core::ParticleMap m_particles;
    bool m_particles_loaded = false;
    std::optional<double> m_start_time;
    long m_event_number = -1;
    int m_file_index = -1;

    // Generic event store for algorithms to share data
    std::map<std::string, std::shared_ptr<void>> m_store;

    auto load_particles() -> void {
        hipo::bank_map REC_Particle = m_hipo_event.get_bank_map("REC::Particle");
        m_particles = Core::ParticleSelector::extract_particles(
            REC_Particle,
            {Core::PIDs::ELECTRON, Core::PIDs::POSITRON,
             Core::PIDs::PIMINUS, Core::PIDs::PIPLUS,
             Core::PIDs::KMINUS, Core::PIDs::KPLUS,
             Core::PIDs::PROTON, Core::PIDs::NEUTRON,
             Core::PIDs::PHOTON}
        );
        m_particles_loaded = true;
    }

    auto load_event_info() -> void {
        hipo::bank_map REC_Event = m_hipo_event.get_bank_map("REC::Event");
        if (REC_Event.get_rows() > 0) {
            m_start_time = REC_Event.get_as<double>("startTime", 0);
        }
    }
};

}  // namespace Framework
