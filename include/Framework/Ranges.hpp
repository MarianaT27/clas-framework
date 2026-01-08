#pragma once

/**
 * @file Ranges.hpp
 * @brief C++20 range adaptors for particle filtering and transformation
 *
 * This file provides composable, lazy-evaluated range adaptors for filtering
 * and transforming particle collections. These adaptors follow the C++20
 * ranges library conventions and can be composed using the pipe operator.
 *
 * @see Requirements 3.1, 3.2, 3.3, 3.4
 */

#include <ranges>
#include <functional>
#include <cstdint>

#include <CLAS/Particle.hpp>
#include <CLAS/PIDs.hpp>
#include <CLAS/Detectors.hpp>

// ROOT headers for Lorentz vector
#include <Math/Vector4D.h>

namespace Framework::Ranges {

/**
 * @brief Filter particles by particle ID (PDG code)
 *
 * Creates a range adaptor that filters particles matching the specified PID.
 *
 * @param pid The particle ID to filter by (CLAS::PIDs enum)
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto electrons = particles | Ranges::by_pid(CLAS::PIDs::ELECTRON);
 * @endcode
 *
 * @see Requirement 3.2
 */
inline constexpr auto by_pid = [](CLAS::PIDs pid) {
    return std::views::filter([pid](const CLAS::Particle& p) {
        return p.pdg() == static_cast<int>(pid);
    });
};

/**
 * @brief Filter particles by integer PDG code
 *
 * Creates a range adaptor that filters particles matching the specified
 * integer PDG code. Useful when working with non-standard particle IDs.
 *
 * @param pdg_code The integer PDG code to filter by
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto custom_particles = particles | Ranges::by_pdg(12345);
 * @endcode
 */
inline constexpr auto by_pdg = [](int pdg_code) {
    return std::views::filter([pdg_code](const CLAS::Particle& p) {
        return p.pdg() == pdg_code;
    });
};

/**
 * @brief Filter particles by charge
 *
 * Creates a range adaptor that filters particles with the specified charge.
 *
 * @param charge The charge value to filter by (-1, 0, +1, etc.)
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto negative_particles = particles | Ranges::by_charge(-1);
 * auto neutral_particles = particles | Ranges::by_charge(0);
 * @endcode
 *
 * @see Requirement 3.2
 */
inline constexpr auto by_charge = [](int charge) {
    return std::views::filter([charge](const CLAS::Particle& p) {
        return p.charge() == charge;
    });
};

/**
 * @brief Filter particles by momentum range
 *
 * Creates a range adaptor that filters particles with momentum in the
 * specified range [p_min, p_max].
 *
 * @param p_min Minimum momentum (inclusive) in GeV
 * @param p_max Maximum momentum (inclusive) in GeV
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto high_p = particles | Ranges::by_momentum(2.0, 10.0);
 * @endcode
 *
 * @see Requirement 3.2
 */
inline constexpr auto by_momentum = [](double p_min, double p_max) {
    return std::views::filter([p_min, p_max](const CLAS::Particle& p) {
        const double momentum = p.p();
        return momentum >= p_min && momentum <= p_max;
    });
};

/**
 * @brief Filter particles by minimum momentum
 *
 * Creates a range adaptor that filters particles with momentum >= p_min.
 *
 * @param p_min Minimum momentum (inclusive) in GeV
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto high_p = particles | Ranges::by_min_momentum(2.0);
 * @endcode
 */
inline constexpr auto by_min_momentum = [](double p_min) {
    return std::views::filter([p_min](const CLAS::Particle& p) {
        return p.p() >= p_min;
    });
};

/**
 * @brief Filter particles by theta angle range
 *
 * Creates a range adaptor that filters particles with theta angle in the
 * specified range [theta_min, theta_max]. Theta is in degrees.
 *
 * @param theta_min Minimum theta angle (inclusive) in degrees
 * @param theta_max Maximum theta angle (inclusive) in degrees
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto forward = particles | Ranges::by_theta(5.0, 35.0);
 * @endcode
 *
 * @see Requirement 3.2
 */
inline constexpr auto by_theta = [](double theta_min, double theta_max) {
    return std::views::filter([theta_min, theta_max](const CLAS::Particle& p) {
        const double theta = p.theta();
        return theta >= theta_min && theta <= theta_max;
    });
};

/**
 * @brief Filter particles by detector region
 *
 * Creates a range adaptor that filters particles detected in the specified
 * detector region. The detector is determined from the particle status code.
 *
 * For CLAS12, the status code encodes the detector region:
 * - |status| in [2000, 3999]: Forward Detector (FD)
 * - |status| in [4000, 4999]: Central Detector (CD)
 * - |status| in [1000, 1999]: Forward Tagger (FT)
 *
 * @param detector The detector region code (CLAS::FD, CLAS::CD, CLAS::FT)
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto fd_particles = particles | Ranges::in_detector(CLAS::FD);
 * auto cd_particles = particles | Ranges::in_detector(CLAS::CD);
 * @endcode
 *
 * @see Requirement 3.2
 */
inline constexpr auto in_detector = [](uint16_t detector) {
    return std::views::filter([detector](const CLAS::Particle& p) {
        const int abs_status = std::abs(p.status());
        switch (detector) {
            case CLAS::FD:
                return abs_status >= 2000 && abs_status < 4000;
            case CLAS::CD:
                return abs_status >= 4000 && abs_status < 5000;
            case CLAS::FT:
                return abs_status >= 1000 && abs_status < 2000;
            default:
                return false;
        }
    });
};

/**
 * @brief Filter particles by status code range
 *
 * Creates a range adaptor that filters particles with status code in the
 * specified range. Uses absolute value of status.
 *
 * @param status_min Minimum absolute status (inclusive)
 * @param status_max Maximum absolute status (inclusive)
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto by_status = [](int status_min, int status_max) {
    return std::views::filter([status_min, status_max](const CLAS::Particle& p) {
        const int abs_status = std::abs(p.status());
        return abs_status >= status_min && abs_status <= status_max;
    });
};

/**
 * @brief Filter particles by vertex z position range
 *
 * Creates a range adaptor that filters particles with vertex z in the
 * specified range [vz_min, vz_max].
 *
 * @param vz_min Minimum vertex z (inclusive) in cm
 * @param vz_max Maximum vertex z (inclusive) in cm
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto target_region = particles | Ranges::by_vertex_z(-10.0, 5.0);
 * @endcode
 */
inline constexpr auto by_vertex_z = [](double vz_min, double vz_max) {
    return std::views::filter([vz_min, vz_max](const CLAS::Particle& p) {
        const double vz = p.vz();
        return vz >= vz_min && vz <= vz_max;
    });
};

/**
 * @brief Filter particles by beta range
 *
 * Creates a range adaptor that filters particles with beta (v/c) in the
 * specified range [beta_min, beta_max].
 *
 * @param beta_min Minimum beta (inclusive)
 * @param beta_max Maximum beta (inclusive)
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto by_beta = [](double beta_min, double beta_max) {
    return std::views::filter([beta_min, beta_max](const CLAS::Particle& p) {
        const double beta = p.beta();
        return beta >= beta_min && beta <= beta_max;
    });
};

/**
 * @brief Filter to keep only valid particles
 *
 * Creates a range adaptor that filters out particles with NaN values
 * in their kinematic properties.
 *
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto valid_only = std::views::filter(
    [](const CLAS::Particle& p) { return p.is_valid(); }
);

/**
 * @brief Transform particles to their 4-momentum (Lorentz vector)
 *
 * Creates a range adaptor that transforms particles to their
 * ROOT::Math::PxPyPzEVector representation.
 *
 * @return A range adaptor that can be composed with other adaptors
 *
 * Example usage:
 * @code
 * auto momenta = particles 
 *     | Ranges::by_pid(CLAS::PIDs::ELECTRON)
 *     | Ranges::to_lorentz_vector;
 * @endcode
 *
 * @see Requirement 3.4
 */
inline constexpr auto to_lorentz_vector = std::views::transform(
    [](const CLAS::Particle& p) { return p.PxPyPzEVector(); }
);

/**
 * @brief Transform particles to their 3-momentum vector
 *
 * Creates a range adaptor that transforms particles to their
 * ROOT::Math::XYZVector representation (3-momentum).
 *
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto to_momentum_vector = std::views::transform(
    [](const CLAS::Particle& p) { return p.PxPyPzVector(); }
);

/**
 * @brief Transform particles to their vertex position vector
 *
 * Creates a range adaptor that transforms particles to their
 * ROOT::Math::XYZVector vertex position.
 *
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto to_vertex_vector = std::views::transform(
    [](const CLAS::Particle& p) { return p.VxVyVzVector(); }
);

/**
 * @brief Transform particles to their momentum magnitude
 *
 * Creates a range adaptor that extracts the momentum magnitude from particles.
 *
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto to_momentum = std::views::transform(
    [](const CLAS::Particle& p) { return p.p(); }
);

/**
 * @brief Transform particles to their theta angle
 *
 * Creates a range adaptor that extracts the theta angle (in degrees) from particles.
 *
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto to_theta = std::views::transform(
    [](const CLAS::Particle& p) { return p.theta(); }
);

/**
 * @brief Transform particles to their phi angle
 *
 * Creates a range adaptor that extracts the phi angle (in degrees) from particles.
 *
 * @return A range adaptor that can be composed with other adaptors
 */
inline constexpr auto to_phi = std::views::transform(
    [](const CLAS::Particle& p) { return p.phi(); }
);

}  // namespace Framework::Ranges
