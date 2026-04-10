#pragma once

#include <bank_view.h>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/physics/ParticleUtils.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>

namespace clas12::framework {

/**
 * @brief Type alias for storing particles grouped by their PID type
 */
using ParticleMap = std::map<clas12::PIDs, std::vector<clas12::Particle>>;

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
     * @brief Construct an Event from a HIPO bank view set
     * @param banks Reference to the bank view set for this event
     */
    explicit Event(hipo::bank_view_set& banks)
        : m_banks(banks) {}

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
     * @return const std::vector<clas12::Particle>& Vector of particles
     */
    auto particles(clas12::PIDs pid) -> const std::vector<clas12::Particle>& {
        if (!m_particles_loaded) {
            load_particles();
        }
        auto it = m_particles.find(pid);
        if (it != m_particles.end()) {
            return it->second;
        }
        static const std::vector<clas12::Particle> empty;
        return empty;
    }

    /**
     * @brief Get all particles as a flat range-compatible view
     *
     * Returns a lazy-evaluated view of all particles across all PID types.
     * This view is compatible with C++20 range adaptors and can be composed
     * using the pipe operator with filters from clas12::framework::Ranges.
     *
     * The view uses lazy evaluation - particles are only accessed when
     * the view is iterated, and filtering operations are applied on-the-fly.
     *
     * @return A range view of all particles
     *
     * Example usage:
     * @code
     * // Get all electrons with momentum > 2 GeV in Forward Detector
     * auto electrons = event.all_particles()
     *     | Ranges::by_pid(clas12::PIDs::ELECTRON)
     *     | Ranges::by_min_momentum(2.0)
     *     | Ranges::in_detector(clas12::FD);
     *
     * for (const auto& e : electrons) {
     *     // Process electron
     * }
     * @endcode
     *
     * @see Requirements 3.1, 3.3
     */
    auto all_particles() {
        if (!m_particles_loaded) {
            load_particles();
        }
        // Flatten all particle vectors into a single view using std::views::join
        // This creates a lazy view that iterates over all particles across all PID types
        return m_particles
            | std::views::values  // Get just the vectors (not the PID keys)
            | std::views::join;   // Flatten into a single range
    }

    /**
     * @brief Get all particles as a flat vector
     *
     * Returns a vector containing all particles from all PID types.
     * Unlike all_particles(), this eagerly evaluates and copies all particles.
     *
     * Use this when you need to store the particles or iterate multiple times.
     * For single-pass filtering, prefer all_particles() for better performance.
     *
     * @return std::vector<clas12::Particle> Vector of all particles
     */
    auto all_particles_vector() -> std::vector<clas12::Particle> {
        if (!m_particles_loaded) {
            load_particles();
        }
        std::vector<clas12::Particle> result;
        for (const auto& [pid, particles] : m_particles) {
            result.insert(result.end(), particles.begin(), particles.end());
        }
        return result;
    }

    /**
     * @brief Get the underlying bank view set (e.g., for event serialization)
     * @return const hipo::bank_view_set& Reference to the bank view set
     */
    auto get_bank_view_set() const -> const hipo::bank_view_set& {
        return m_banks;
    }

    /**
     * @brief Get a HIPO bank view by name
     * @param bank_name Name of the bank (e.g., "REC::Particle")
     * @return const hipo::bank_view& Reference to the bank view
     */
    auto get_bank(const std::string& bank_name) -> const hipo::bank_view& {
        return m_banks[bank_name];
    }

    /**
     * @brief Get multiple banks at once using structured bindings
     *
     * Returns a tuple of const references to the requested bank views.
     *
     * @tparam Names Variadic string types for bank names
     * @param names Bank names (e.g., "REC::Particle", "REC::Calorimeter")
     * @return std::tuple of const bank_view references
     *
     * Example usage:
     * @code
     * auto [particles, calo, track] = event.banks(
     *     "REC::Particle", "REC::Calorimeter", "REC::Track"
     * );
     *
     * // Process particles
     * for (int i = 0; i < particles.get().rows(); ++i) {
     *     auto px = particles.get().get<float>("px", i);
     *     // ...
     * }
     * @endcode
     */
    template <typename... Names>
    auto banks(Names&&... names) {
        return std::make_tuple(std::cref(m_banks[std::forward<Names>(names)])...);
    }

    /**
     * @brief Check if a bank exists and has data
     * @param bank_name Name of the bank
     * @return true if bank exists and has rows
     */
    auto has_bank(const std::string& bank_name) -> bool {
        return m_banks[bank_name].rows() > 0;
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

    // ========================================================================
    // Type-safe event store API (compile-time type checking)
    // ========================================================================

    /**
     * @brief Store a value with a typed key (type-safe API)
     *
     * This method provides compile-time type safety by using TypedKey<T>.
     * Type mismatches are caught at compile time rather than runtime.
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @param value The value to store
     *
     * Example usage:
     * @code
     * event.store_typed(Keys::Q2, 4.5);
     * event.store_typed(Keys::Electrons, selected_electrons);
     * @endcode
     */
    template <typename T>
    auto store_typed(TypedKey<T> key, T value) -> void {
        m_typed_store.store(key, std::move(value));
    }

    /**
     * @brief Retrieve a value by typed key (type-safe API)
     *
     * Returns the stored value if it exists and the type matches.
     * Returns std::nullopt if the key doesn't exist or type mismatches.
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @return std::optional<T> The value, or nullopt if not found/type mismatch
     *
     * Example usage:
     * @code
     * auto q2 = event.retrieve_typed(Keys::Q2);
     * if (q2) {
     *     std::cout << "Q2 = " << *q2 << std::endl;
     * }
     * @endcode
     */
    template <typename T>
    auto retrieve_typed(TypedKey<T> key) const -> std::optional<T> {
        return m_typed_store.retrieve(key);
    }

    /**
     * @brief Check if a typed key exists in the store
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @return bool True if key exists with matching type
     */
    template <typename T>
    auto contains_typed(TypedKey<T> key) const -> bool {
        return m_typed_store.contains(key);
    }

    /**
     * @brief Check if an algorithm decision was passed (type-safe API)
     *
     * @param key The typed decision key (must be TypedKey<bool>)
     * @return bool True if the decision passed, false otherwise
     *
     * Example usage:
     * @code
     * if (event.passes_decision_typed(Keys::HasGoodElectron)) {
     *     // Process event with good electron
     * }
     * @endcode
     */
    auto passes_decision_typed(TypedKey<bool> key) -> bool {
        return m_typed_store.retrieve(key).value_or(false);
    }

    /**
     * @brief Set an algorithm decision flag (type-safe API)
     *
     * @param key The typed decision key (must be TypedKey<bool>)
     * @param passed Whether the decision passed
     */
    auto set_decision_typed(TypedKey<bool> key, bool passed) -> void {
        m_typed_store.store(key, passed);
    }

    /**
     * @brief Check a decision and retrieve data in one call
     *
     * Combines the common pattern of checking a boolean decision flag
     * and then retrieving associated data from the event store.
     * Returns nullopt if the decision fails or the data is not found.
     *
     * @tparam T The data type to retrieve
     * @param decision The decision key to check
     * @param data The data key to retrieve
     * @return std::optional<T> The data if decision passes and data exists
     *
     * @code
     * // BEFORE: 3-4 lines repeated everywhere
     * if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return FILTERED();
     * auto electrons_opt = event.retrieve_typed(Keys::SelectedElectrons);
     * if (!electrons_opt || electrons_opt->empty()) return FILTERED();
     *
     * // AFTER: 1-2 lines
     * auto electrons = event.require_if(Decisions::HasGoodDISElectron, Keys::SelectedElectrons);
     * if (!electrons) return FILTERED();
     * @endcode
     */
    template <typename T>
    auto require_if(TypedKey<bool> decision, TypedKey<T> data) -> std::optional<T> {
        if (!passes_decision_typed(decision)) return std::nullopt;
        return retrieve_typed(data);
    }

    /**
     * @brief Get access to the underlying typed event store
     *
     * @return TypedEventStore& Reference to the typed store
     */
    auto typed_store() -> TypedEventStore& {
        return m_typed_store;
    }

    /**
     * @brief Get const access to the underlying typed event store
     *
     * @return const TypedEventStore& Const reference to the typed store
     */
    auto typed_store() const -> const TypedEventStore& {
        return m_typed_store;
    }

   private:
    hipo::bank_view_set& m_banks;
    ParticleMap m_particles;
    bool m_particles_loaded = false;
    std::optional<double> m_start_time;
    long m_event_number = -1;
    int m_file_index = -1;
    TypedEventStore m_typed_store;

    /**
     * @brief Load particles from REC::Particle bank
     */
    auto load_particles() -> void {
        const auto& REC_Particle = m_banks["REC::Particle"];

        // Initialize particle vectors for common types
        std::vector<clas12::PIDs> target_types = {
            clas12::PIDs::ELECTRON, clas12::PIDs::POSITRON,
            clas12::PIDs::PHOTON,
            clas12::PIDs::PROTON, clas12::PIDs::NEUTRON,
            clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS,
            clas12::PIDs::KPLUS, clas12::PIDs::KMINUS};

        for (const auto& type : target_types) {
            m_particles[type] = std::vector<clas12::Particle>();
        }

        // Extract particles from bank
        for (int i = 0; i < REC_Particle.rows(); i++) {
            const int pid = REC_Particle.get<int32_t>("pid", i);

            // Find matching particle type
            clas12::PIDs particle_type;
            bool found = false;
            for (const auto& type : target_types) {
                if (pid == static_cast<int>(type)) {
                    particle_type = type;
                    found = true;
                    break;
                }
            }

            if (!found) continue;

            const int status = static_cast<int>(REC_Particle.get<int16_t>("status", i));
            const int charge = static_cast<int>(REC_Particle.get<int8_t>("charge", i));
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

            const double mass = clas12::physics::ParticleUtils::get_mass(pid);
            const double E = clas12::physics::ParticleUtils::compute_energy(px, py, pz, pid);

            m_particles[particle_type].emplace_back(
                pid, status, i, charge, mass,
                px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
        }

        m_particles_loaded = true;
    }

    auto load_event_info() -> void {
        const auto& REC_Event = m_banks["REC::Event"];
        if (REC_Event.rows() > 0) {
            m_start_time = static_cast<double>(REC_Event.get<float>("startTime", 0));
        }
    }
};

}  // namespace clas12::framework
