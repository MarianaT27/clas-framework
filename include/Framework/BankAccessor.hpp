#pragma once

/**
 * @file BankAccessor.hpp
 * @brief Simplified HIPO bank access for CLAS12 algorithms
 *
 * This header provides convenient abstractions for accessing HIPO banks
 * in analysis algorithms, reducing boilerplate code and improving readability.
 */

#include <hipo4/hipoeventiterator.h>

#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Framework {

/**
 * @brief Lightweight wrapper providing convenient access to common REC banks
 *
 * Usage:
 * @code
 * auto execute(Framework::Event& event) -> Framework::StatusCode override {
 *     RECBanks banks(event);
 *
 *     if (!banks.has_event()) return Framework::StatusCode::SUCCESS();
 *
 *     double start_time = banks.start_time();
 *     int n_particles = banks.particle().get_rows();
 *
 *     // Access calorimeter for particle at index 0
 *     auto& calo = banks.calorimeter();
 *     // ...
 * }
 * @endcode
 */
class RECBanks {
   public:
    explicit RECBanks(hipo::iter_event& hipo_event) : m_event(hipo_event) {}

    // =========================================================================
    // Core banks - lazily loaded
    // =========================================================================

    /// REC::Event bank (event-level info like start time)
    [[nodiscard]] auto event() -> hipo::bank_map& {
        if (!m_event_loaded) {
            m_rec_event = m_event.get_bank_map("REC::Event");
            m_event_loaded = true;
        }
        return m_rec_event;
    }

    /// REC::Particle bank
    [[nodiscard]] auto particle() -> hipo::bank_map& {
        if (!m_particle_loaded) {
            m_rec_particle = m_event.get_bank_map("REC::Particle");
            m_particle_loaded = true;
        }
        return m_rec_particle;
    }

    /// REC::Calorimeter bank
    [[nodiscard]] auto calorimeter() -> hipo::bank_map& {
        if (!m_calorimeter_loaded) {
            m_rec_calorimeter = m_event.get_bank_map("REC::Calorimeter");
            m_calorimeter_loaded = true;
        }
        return m_rec_calorimeter;
    }

    /// REC::Cherenkov bank
    [[nodiscard]] auto cherenkov() -> hipo::bank_map& {
        if (!m_cherenkov_loaded) {
            m_rec_cherenkov = m_event.get_bank_map("REC::Cherenkov");
            m_cherenkov_loaded = true;
        }
        return m_rec_cherenkov;
    }

    /// REC::Scintillator bank
    [[nodiscard]] auto scintillator() -> hipo::bank_map& {
        if (!m_scintillator_loaded) {
            m_rec_scintillator = m_event.get_bank_map("REC::Scintillator");
            m_scintillator_loaded = true;
        }
        return m_rec_scintillator;
    }

    /// REC::Track bank
    [[nodiscard]] auto track() -> hipo::bank_map& {
        if (!m_track_loaded) {
            m_rec_track = m_event.get_bank_map("REC::Track");
            m_track_loaded = true;
        }
        return m_rec_track;
    }

    /// REC::Traj bank
    [[nodiscard]] auto traj() -> hipo::bank_map& {
        if (!m_traj_loaded) {
            m_rec_traj = m_event.get_bank_map("REC::Traj");
            m_traj_loaded = true;
        }
        return m_rec_traj;
    }

    // =========================================================================
    // Convenience methods
    // =========================================================================

    /// Check if REC::Event bank has data
    [[nodiscard]] auto has_event() -> bool { return event().get_rows() > 0; }

    /// Check if REC::Particle bank has data
    [[nodiscard]] auto has_particles() -> bool { return particle().get_rows() > 0; }

    /// Get event start time (returns 0 if not available)
    [[nodiscard]] auto start_time() -> double {
        if (!has_event()) return 0.0;
        return event().get_bank().get<float>("startTime", 0);
    }

    /// Get number of particles
    [[nodiscard]] auto n_particles() -> int { return particle().get_rows(); }

    /// Access underlying hipo::bank directly (for advanced use)
    [[nodiscard]] auto particle_bank() -> hipo::bank& { return particle().get_bank(); }
    [[nodiscard]] auto calorimeter_bank() -> hipo::bank& { return calorimeter().get_bank(); }
    [[nodiscard]] auto cherenkov_bank() -> hipo::bank& { return cherenkov().get_bank(); }
    [[nodiscard]] auto scintillator_bank() -> hipo::bank& { return scintillator().get_bank(); }
    [[nodiscard]] auto track_bank() -> hipo::bank& { return track().get_bank(); }
    [[nodiscard]] auto traj_bank() -> hipo::bank& { return traj().get_bank(); }

   private:
    hipo::iter_event& m_event;

    // Cached bank maps (lazy loading)
    hipo::bank_map m_rec_event;
    hipo::bank_map m_rec_particle;
    hipo::bank_map m_rec_calorimeter;
    hipo::bank_map m_rec_cherenkov;
    hipo::bank_map m_rec_scintillator;
    hipo::bank_map m_rec_track;
    hipo::bank_map m_rec_traj;

    // Loading flags
    bool m_event_loaded = false;
    bool m_particle_loaded = false;
    bool m_calorimeter_loaded = false;
    bool m_cherenkov_loaded = false;
    bool m_scintillator_loaded = false;
    bool m_track_loaded = false;
    bool m_traj_loaded = false;
};


/**
 * @brief Generic bank accessor for any HIPO bank with caching
 *
 * Usage:
 * @code
 * class MyAlgorithm : public Framework::Algorithm {
 *     BankAccessor<"REC::Particle", "REC::Calorimeter", "REC::Track"> m_banks;
 *
 *     auto execute(Framework::Event& event) -> Framework::StatusCode override {
 *         m_banks.load(event.hipo_event());
 *
 *         auto& particle = m_banks.get<"REC::Particle">();
 *         auto& calo = m_banks.get<"REC::Calorimeter">();
 *         // ...
 *     }
 * };
 * @endcode
 */
class BankCache {
   public:
    BankCache() = default;

    /// Load/refresh a bank by name
    auto load(hipo::iter_event& event, const std::string& bank_name) -> hipo::bank_map& {
        auto it = m_cache.find(bank_name);
        if (it == m_cache.end()) {
            m_cache[bank_name] = event.get_bank_map(bank_name);
            return m_cache[bank_name];
        }
        // Refresh existing bank
        return it->second = event.get_bank_map(bank_name);
    }

    /// Get a previously loaded bank (must call load first)
    [[nodiscard]] auto get(const std::string& bank_name) -> hipo::bank_map& {
        return m_cache.at(bank_name);
    }

    /// Check if bank is loaded and has rows
    [[nodiscard]] auto has_data(const std::string& bank_name) -> bool {
        auto it = m_cache.find(bank_name);
        return it != m_cache.end() && it->second.get_rows() > 0;
    }

    /// Get row count for a bank (0 if not loaded)
    [[nodiscard]] auto rows(const std::string& bank_name) -> int {
        auto it = m_cache.find(bank_name);
        return it != m_cache.end() ? it->second.get_rows() : 0;
    }

    /// Clear all cached banks
    auto clear() -> void { m_cache.clear(); }

   private:
    std::unordered_map<std::string, hipo::bank_map> m_cache;
};

/**
 * @brief Load multiple banks at once using structured bindings
 *
 * Usage:
 * @code
 * auto execute(Framework::Event& event) -> Framework::StatusCode override {
 *     auto [rec_event, particle, calo] = load_banks(
 *         event.hipo_event(),
 *         "REC::Event", "REC::Particle", "REC::Calorimeter"
 *     );
 *
 *     if (rec_event.get_rows() == 0) return Framework::StatusCode::SUCCESS();
 *     // ...
 * }
 * @endcode
 */
template <typename... BankNames>
auto load_banks(hipo::iter_event& event, BankNames&&... names) {
    return std::make_tuple(event.get_bank_map(std::forward<BankNames>(names))...);
}

/**
 * @brief Helper to check if all banks have data
 */
template <typename... Banks>
[[nodiscard]] auto all_have_data(Banks&... banks) -> bool {
    return ((banks.get_rows() > 0) && ...);
}

/**
 * @brief Helper to check if any bank has data
 */
template <typename... Banks>
[[nodiscard]] auto any_has_data(Banks&... banks) -> bool {
    return ((banks.get_rows() > 0) || ...);
}

}  // namespace Framework
