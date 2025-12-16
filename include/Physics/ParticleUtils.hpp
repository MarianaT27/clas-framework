#pragma once

#include <CLAS/Constants.hpp>
#include <CLAS/Particle.hpp>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <optional>
#include <vector>

namespace Physics {

/**
 * @brief Utility functions for particle physics calculations and selection
 *
 * This namespace contains physics-related helper functions that were previously
 * in CLAS::Helpers. These are specifically for particle physics operations.
 */
namespace ParticleUtils {

/**
 * @brief Get the mass of a particle given its PDG ID
 *
 * @param pid PDG particle ID
 * @return Mass in GeV, or NaN if unknown
 */
inline auto get_mass(const int pid) -> double {
    switch (pid) {
        case 11:
        case -11:
            return CLAS::Constants::ELECTRONMASS;
        case 22:
            return 0.0;
        case 211:
        case -211:
            return CLAS::Constants::PIMASS;
        case 321:
        case -321:
            return CLAS::Constants::KMASS;
        case 2212:
        case -2212:
            return CLAS::Constants::PROTONMASS;
        case 2112:
        case -2112:
            return CLAS::Constants::NEUTRONMASS;
        case 111:
            return CLAS::Constants::PI0MASS;
        default:
            return std::numeric_limits<double>::quiet_NaN();
    }
}

/**
 * @brief Computes the total energy of a particle given its momentum components and particle ID.
 *
 * Uses the relativistic energy-momentum relation: E = sqrt(px^2 + py^2 + pz^2 + m^2)
 *
 * @param px The x-component of the particle's momentum (in GeV/c)
 * @param py The y-component of the particle's momentum (in GeV/c)
 * @param pz The z-component of the particle's momentum (in GeV/c)
 * @param pid The particle ID (PDG code) used to determine the particle's mass
 * @return The total energy of the particle (in GeV). Returns NaN for unsupported PIDs.
 */
inline auto compute_energy(double px, double py, double pz, int pid) -> double {
    double mass = get_mass(pid);
    return std::hypot(std::hypot(px, py, pz), mass);
}

/**
 * @brief Finds the first trigger electron from a list of electrons.
 *
 * Searches for an electron with negative status within the specified status range.
 *
 * @param electrons A vector of CLAS::Particle objects representing electrons
 * @param status_range Pair of (min, max) absolute status values
 * @return The first trigger electron if found, or std::nullopt otherwise
 */
inline auto find_trigger_electron(const std::vector<CLAS::Particle>& electrons,
                                  std::pair<double, double> status_range) -> std::optional<CLAS::Particle> {
    auto it = std::find_if(electrons.begin(), electrons.end(),
                           [&status_range](const CLAS::Particle& electron) {
                               const double absStatus = std::abs(electron.status());
                               return electron.status() < 0.0 &&
                                      electron.p() != 0.0 &&
                                      status_range.first <= absStatus &&
                                      absStatus < status_range.second;
                           });

    return (it != electrons.end()) ? std::optional<CLAS::Particle>(*it) : std::nullopt;
}

/**
 * @brief Finds the most energetic electron from a list of electrons.
 *
 * @param electrons A vector of CLAS::Particle objects representing electrons
 * @return The most energetic electron if found, or std::nullopt if empty/invalid
 */
inline auto find_most_energetic_electron(const std::vector<CLAS::Particle>& electrons) -> std::optional<CLAS::Particle> {
    if (electrons.empty()) return std::nullopt;

    auto result = std::max_element(electrons.begin(), electrons.end(),
                                   [](const CLAS::Particle& a, const CLAS::Particle& b) { return a.E() < b.E(); });

    if (result != electrons.end() && result->p() != 0.0) {
        return *result;
    }
    return std::nullopt;
}

/**
 * @brief Generates all unique pairs of elements from a given container.
 *
 * @tparam Container A container type with forward iterators
 * @param container The input container from which unique pairs are generated
 * @return Vector containing all unique pairs of elements
 *
 * @example
 * std::vector<int> numbers = {1, 2, 3};
 * auto pairs = generate_unique_pairs(numbers);
 * // pairs contains: {(1, 2), (1, 3), (2, 3)}
 */
template <typename Container>
[[nodiscard]] auto generate_unique_pairs(const Container& container)
    -> std::vector<std::pair<typename Container::value_type, typename Container::value_type>> {
    using T = typename Container::value_type;
    std::vector<std::pair<T, T>> pairs;

    if (container.size() < 2) return {};

    const auto n = static_cast<std::ptrdiff_t>(container.size());
    pairs.reserve((n * (n - 1)) / 2);

    for (auto it1 = container.begin(); it1 != container.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != container.end(); ++it2) {
            pairs.emplace_back(*it1, *it2);
        }
    }

    return pairs;
}

/**
 * @brief Computes the floating-point modulo operation with proper boundary handling.
 *
 * @tparam T Floating-point type
 * @param x The dividend
 * @param y The divisor
 * @return The remainder of x/y
 */
template <typename T>
auto mod(const T x, const T y) -> T {
    static_assert(!std::numeric_limits<T>::is_exact, "Mod: floating-point type expected");

    if (0. == y) return x;

    double m = x - y * floor(x / y);

    if (y > 0) {
        if (m >= y) return 0;
        if (m < 0) {
            if (y + m == y) return 0;
            else
                return y + m;
        }
    } else {
        if (m <= y) return 0;
        if (m > 0) {
            if (y + m == y) return 0;
            else
                return y + m;
        }
    }

    return m;
}

/**
 * @brief Wraps an angle to the range [-180, 180] degrees.
 *
 * @param angle The angle in degrees
 * @return The angle normalized to [-180, 180]
 */
inline double wrap_angle_deg(double angle) {
    return mod(angle, 360.0) - 180.0;
}

/**
 * @brief Wraps an angle to the range [-π, π] radians.
 *
 * @param angle The angle in radians
 * @return The angle normalized to [-π, π]
 */
inline double wrap_angle_rad(double angle) {
    return mod(angle, 2.0 * M_PI) - M_PI;
}

}  // namespace ParticleUtils

}  // namespace Physics
