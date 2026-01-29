#pragma once

#include <clas12/Particle.hpp>

#include <any>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <typeinfo>
#include <vector>

// Forward declaration for MCParticle
namespace clas12 {
class MCParticle;
}

namespace clas12::framework {

/**
 * @brief Type-safe key for event store access
 *
 * TypedKey provides compile-time type association for event store keys.
 * This enables type-safe storage and retrieval of values, catching type
 * mismatches at compile time rather than runtime.
 *
 * @tparam T The type of value associated with this key
 *
 * Example usage:
 * @code
 * // Define typed keys
 * inline constexpr TypedKey<double> Q2{"Q2"};
 * inline constexpr TypedKey<std::vector<clas12::Particle>> Electrons{"electrons"};
 *
 * // Use with TypedEventStore
 * store.store(Q2, 4.5);
 * auto q2_value = store.retrieve(Q2);  // Returns std::optional<double>
 * @endcode
 *
 * @see Requirements 8.1
 */
template <typename T>
struct TypedKey {
    std::string name;

    /**
     * @brief Construct a TypedKey from a string literal
     * @param n The key name
     */
    constexpr TypedKey(const char* n) : name(n) {}

    /**
     * @brief Construct a TypedKey from a std::string
     * @param n The key name
     */
    TypedKey(std::string n) : name(std::move(n)) {}

    /**
     * @brief Get the type hash for this key's value type
     * @return std::size_t Hash code of the value type
     */
    static auto type_hash() -> std::size_t {
        return typeid(T).hash_code();
    }
};

/**
 * @brief Type-safe event store for inter-algorithm communication
 *
 * TypedEventStore provides a type-safe mechanism for storing and retrieving
 * computed quantities in events. It uses TypedKey<T> to associate types with
 * keys at compile time, preventing runtime type errors.
 *
 * The store validates types on both storage and retrieval:
 * - On storage: Records the type hash for the key
 * - On retrieval: Verifies the requested type matches the stored type
 *
 * Example usage:
 * @code
 * TypedEventStore store;
 *
 * // Store values with typed keys
 * store.store(Keys::Q2, 4.5);
 * store.store(Keys::Electrons, electron_vector);
 *
 * // Retrieve with type safety
 * auto q2 = store.retrieve(Keys::Q2);  // std::optional<double>
 * if (q2) {
 *     std::cout << "Q2 = " << *q2 << std::endl;
 * }
 *
 * // Check existence
 * if (store.contains(Keys::HasGoodElectron)) {
 *     // ...
 * }
 * @endcode
 *
 * @see Requirements 8.1, 8.2, 8.3
 */
class TypedEventStore {
   public:
    /**
     * @brief Store a value with a typed key
     *
     * Stores the value and records its type hash for later validation.
     * If a value already exists for this key, it will be overwritten.
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @param value The value to store
     *
     * @see Requirements 8.2
     */
    template <typename T>
    auto store(TypedKey<T> key, T value) -> void {
        m_store[key.name] = std::any(std::move(value));
        m_types[key.name] = TypedKey<T>::type_hash();
    }

    /**
     * @brief Retrieve a value by typed key
     *
     * Returns the stored value if it exists and the type matches.
     * Returns std::nullopt if the key doesn't exist or type mismatches.
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @return std::optional<T> The value, or nullopt if not found/type mismatch
     *
     * @see Requirements 8.2, 8.3
     */
    template <typename T>
    auto retrieve(TypedKey<T> key) const -> std::optional<T> {
        auto it = m_store.find(key.name);
        if (it == m_store.end()) {
            return std::nullopt;
        }

        // Type check
        auto type_it = m_types.find(key.name);
        if (type_it == m_types.end() || type_it->second != TypedKey<T>::type_hash()) {
            return std::nullopt;  // Type mismatch
        }

        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return std::nullopt;
        }
    }

    /**
     * @brief Check if a typed key exists in the store
     *
     * Returns true only if the key exists AND the stored type matches
     * the key's type parameter.
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @return bool True if key exists with matching type
     *
     * @see Requirements 8.3
     */
    template <typename T>
    auto contains(TypedKey<T> key) const -> bool {
        auto it = m_store.find(key.name);
        if (it == m_store.end()) {
            return false;
        }

        auto type_it = m_types.find(key.name);
        return type_it != m_types.end() && type_it->second == TypedKey<T>::type_hash();
    }

    /**
     * @brief Remove a value from the store
     *
     * @tparam T The value type (inferred from key)
     * @param key The typed key
     * @return bool True if the key was found and removed
     */
    template <typename T>
    auto remove(TypedKey<T> key) -> bool {
        auto it = m_store.find(key.name);
        if (it == m_store.end()) {
            return false;
        }

        m_store.erase(it);
        m_types.erase(key.name);
        return true;
    }

    /**
     * @brief Clear all stored values
     */
    auto clear() -> void {
        m_store.clear();
        m_types.clear();
    }

    /**
     * @brief Get the number of stored values
     * @return std::size_t Number of stored key-value pairs
     */
    auto size() const -> std::size_t {
        return m_store.size();
    }

    /**
     * @brief Check if the store is empty
     * @return bool True if no values are stored
     */
    auto empty() const -> bool {
        return m_store.empty();
    }

   private:
    std::map<std::string, std::any> m_store;
    std::map<std::string, std::size_t> m_types;
};

/**
 * @brief Pre-defined typed keys for common physics quantities
 *
 * This namespace provides standard keys for commonly used data types
 * in CLAS12 analysis. Using these keys ensures consistency across
 * algorithms and enables type-safe inter-algorithm communication.
 *
 * Example usage:
 * @code
 * // In electron selection algorithm
 * store.store(Keys::Electrons, selected_electrons);
 * store.store(Keys::HasGoodElectron, !selected_electrons.empty());
 *
 * // In DIS analysis algorithm
 * if (store.contains(Keys::HasGoodElectron)) {
 *     auto electrons = store.retrieve(Keys::Electrons);
 *     // Calculate kinematics...
 *     store.store(Keys::Q2, q2_value);
 *     store.store(Keys::W, w_value);
 * }
 * @endcode
 *
 * @see Requirements 8.4
 */
namespace Keys {

// Particle collections
inline const TypedKey<std::vector<clas12::Particle>> Electrons{"electrons"};
inline const TypedKey<std::vector<clas12::Particle>> Positrons{"positrons"};
inline const TypedKey<std::vector<clas12::Particle>> Photons{"photons"};
inline const TypedKey<std::vector<clas12::Particle>> Protons{"protons"};
inline const TypedKey<std::vector<clas12::Particle>> Neutrons{"neutrons"};
inline const TypedKey<std::vector<clas12::Particle>> PiPlus{"pi_plus"};
inline const TypedKey<std::vector<clas12::Particle>> PiMinus{"pi_minus"};
inline const TypedKey<std::vector<clas12::Particle>> KPlus{"k_plus"};
inline const TypedKey<std::vector<clas12::Particle>> KMinus{"k_minus"};

// Kinematic variables
inline const TypedKey<double> Q2{"Q2"};
inline const TypedKey<double> W{"W"};
inline const TypedKey<double> Nu{"nu"};
inline const TypedKey<double> Xbj{"x_bjorken"};
inline const TypedKey<double> Y{"y"};

// Pi0 reconstruction variables
inline const TypedKey<double> Pi0Mass{"pi0_mass"};
inline const TypedKey<double> Pi0Momentum{"pi0_momentum"};
inline const TypedKey<double> Pi0Theta{"pi0_theta"};
inline const TypedKey<double> Pi0Phi{"pi0_phi"};
inline const TypedKey<double> PhotonOpeningAngle{"photon_opening_angle"};

// Event flags / Algorithm decisions
inline const TypedKey<bool> HasGoodElectron{"has_good_electron"};
inline const TypedKey<bool> HasTriggerElectron{"has_trigger_electron"};
inline const TypedKey<bool> HasGoodDISElectron{"has_good_dis_electron"};
inline const TypedKey<bool> IsDIS{"is_dis"};
inline const TypedKey<bool> PassedFiducialCuts{"passed_fiducial_cuts"};
inline const TypedKey<bool> HasPi0{"has_pi0"};
inline const TypedKey<bool> HasPhotonPair{"has_photon_pair"};
inline const TypedKey<bool> PassedElectronCuts{"passed_electron_cuts"};
inline const TypedKey<bool> PassedPhotonCuts{"passed_photon_cuts"};

// Selected particle collections (algorithm outputs)
inline const TypedKey<std::vector<clas12::Particle>> SelectedElectrons{"selected_electrons"};
inline const TypedKey<std::vector<clas12::Particle>> SelectedPhotons{"selected_photons"};
inline const TypedKey<std::vector<clas12::Particle>> SelectedProtons{"selected_protons"};
inline const TypedKey<std::vector<clas12::Particle>> SelectedPiPlus{"selected_pi_plus"};
inline const TypedKey<std::vector<clas12::Particle>> SelectedPiMinus{"selected_pi_minus"};

// Trigger electron (single particle)
inline const TypedKey<clas12::Particle> TriggerElectron{"trigger_electron"};
inline const TypedKey<clas12::Particle> BestElectron{"best_electron"};

// Beam and target info
inline const TypedKey<double> BeamEnergy{"beam_energy"};
inline const TypedKey<double> TargetMass{"target_mass"};

// =============================================================================
// DIS kinematic variables (algorithm-compatible naming)
// =============================================================================
inline const TypedKey<double> DIS_Q2{"dis_Q2"};
inline const TypedKey<double> DIS_W{"dis_W"};
inline const TypedKey<double> DIS_xB{"dis_xB"};
inline const TypedKey<double> DIS_y{"dis_y"};
inline const TypedKey<double> DIS_nu{"dis_nu"};

// Selected DIS electron
inline const TypedKey<std::vector<clas12::Particle>> SelectedDISElectron{"SelectedDISElectron"};

// =============================================================================
// Algorithm-compatible selected particle keys (capital naming convention)
// =============================================================================
inline const TypedKey<std::vector<clas12::Particle>> SelectedElectronsCapital{"SelectedElectrons"};
inline const TypedKey<std::vector<clas12::Particle>> SelectedPiPlusCapital{"SelectedPiPlus"};
inline const TypedKey<std::vector<clas12::Particle>> SelectedPiMinusCapital{"SelectedPiMinus"};

// =============================================================================
// Lambda reconstruction variables
// =============================================================================
inline const TypedKey<double> LambdaMass{"lambda_mass"};
inline const TypedKey<double> LambdaMomentum{"lambda_p"};
inline const TypedKey<double> LambdaTheta{"lambda_theta"};
inline const TypedKey<double> LambdaCoplanarity{"lambda_coplanarity"};
inline const TypedKey<double> LambdaMissingMass{"lambda_missing_mass"};

// =============================================================================
// Monte Carlo Truth Keys
// =============================================================================
inline const TypedKey<bool> IsMonteCarlo{"is_monte_carlo"};
inline const TypedKey<bool> HasMCTruth{"has_mc_truth"};
inline const TypedKey<std::vector<clas12::MCParticle>> MCParticles{"mc_particles"};
inline const TypedKey<std::vector<clas12::MCParticle>> MCElectrons{"mc_electrons"};
inline const TypedKey<std::vector<clas12::MCParticle>> MCPhotons{"mc_photons"};
inline const TypedKey<std::vector<clas12::MCParticle>> MCProtons{"mc_protons"};
inline const TypedKey<std::vector<clas12::MCParticle>> MCPiPlus{"mc_pi_plus"};
inline const TypedKey<std::vector<clas12::MCParticle>> MCPiMinus{"mc_pi_minus"};

// MC matching flags
inline const TypedKey<bool> ElectronMatchedToMC{"electron_matched_to_mc"};
inline const TypedKey<double> ElectronMCMatchQuality{"electron_mc_match_quality"};

}  // namespace Keys

}  // namespace clas12::framework
