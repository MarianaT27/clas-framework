#pragma once

#include <cmath>
#include <limits>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/physics/CompositeParticle.hpp>
#include "fmt/base.h"

namespace physics {

namespace detail {
/// Check if a double is finite (not NaN or Inf)
inline auto is_finite(double x) -> bool { return std::isfinite(x); }

/// Validate that a particle's 4-vector components are all finite
inline auto is_valid_4vec(const Particle& p) -> bool {
    return is_finite(p.px()) && is_finite(p.py()) &&
           is_finite(p.pz()) && is_finite(p.E());
}
}  // namespace detail

/**
 * @brief Kinematic calculator utilities for particle physics
 *
 * Provides functions for calculating various kinematic quantities
 * used in particle physics analyses.
 *
 * All methods validate inputs and return NaN if any particle has
 * non-finite (NaN/Inf) 4-vector components.
 */
class KinematicCalculator {
   public:
    /**
     * @brief Calculate missing mass squared
     * @param beam Beam particle
     * @param target Target particle (at rest)
     * @param detected_particles Detected particles in final state
     * @return Missing mass squared in GeV²
     */
    static auto missing_mass_squared(const Particle& beam,
                                     const Particle& target,
                                     const std::vector<Particle>& detected_particles) -> double {
        if (!detail::is_valid_4vec(beam) || !detail::is_valid_4vec(target)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        for (const auto& p : detected_particles) {
            if (!detail::is_valid_4vec(p)) return std::numeric_limits<double>::quiet_NaN();
        }
        // Four-momentum conservation: P_initial = P_final + P_missing
        double px_missing = beam.px() + target.px();
        double py_missing = beam.py() + target.py();
        double pz_missing = beam.pz() + target.pz();
        double E_missing = beam.E() + target.E();

        for (const auto& p : detected_particles) {
            px_missing -= p.px();
            py_missing -= p.py();
            pz_missing -= p.pz();
            E_missing -= p.E();
        }

        double p2 = px_missing * px_missing + py_missing * py_missing + pz_missing * pz_missing;
        return E_missing * E_missing - p2;
    }

    /**
     * @brief Calculate missing mass
     */
    static auto missing_mass(const Particle& beam,
                             const Particle& target,
                             const std::vector<Particle>& detected_particles) -> double {
        double mm2 = missing_mass_squared(beam, target, detected_particles);
        return (mm2 > 0) ? std::sqrt(mm2) : -std::sqrt(-mm2);
    }

    /**
     * @brief Calculate W (invariant mass of γ*-p system)
     * @param beam Beam electron
     * @param scattered Scattered electron
     * @param target Target proton
     * @return W in GeV
     */
    static auto calculate_W(const Particle& beam,
                            const Particle& scattered,
                            const Particle& target) -> double {
        if (!detail::is_valid_4vec(beam) || !detail::is_valid_4vec(scattered) || !detail::is_valid_4vec(target)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        // Virtual photon four-momentum
        double q_px = beam.px() - scattered.px();
        double q_py = beam.py() - scattered.py();
        double q_pz = beam.pz() - scattered.pz();
        double q_E = beam.E() - scattered.E();

        // W² = (q + P)² where P is target four-momentum
        double W_px = q_px + target.px();
        double W_py = q_py + target.py();
        double W_pz = q_pz + target.pz();
        double W_E = q_E + target.E();

        double W2 = W_E * W_E - (W_px * W_px + W_py * W_py + W_pz * W_pz);
        return (W2 > 0) ? std::sqrt(W2) : 0.0;
    }

    /**
     * @brief Calculate Q² (virtuality of photon)
     * @param beam Beam electron
     * @param scattered Scattered electron
     * @return Q² in GeV²
     */
    static auto calculate_Q2(const Particle& beam, const Particle& scattered) -> double {
        if (!detail::is_valid_4vec(beam) || !detail::is_valid_4vec(scattered)) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return 4.0 * beam.E() * scattered.E() * std::pow(std::sin(scattered.theta() * Constants::DEG_TO_RAD / 2.0), 2);
    }

    /**
     * @brief Calculate Bjorken x
     * @param Q2 Four-momentum transfer squared
     * @param nu Energy transfer (beam - scattered energy)
     * @param M Target mass
     * @return Bjorken x
     */
    static auto calculate_x_bjorken(double Q2, double nu, double M = 0.938) -> double {
        if (!detail::is_finite(Q2) || !detail::is_finite(nu) || !detail::is_finite(M) || (M * nu) == 0.0) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return Q2 / (2.0 * M * nu);
    }

    /**
     * @brief Calculate inelasticity y
     * @param beam Beam electron
     * @param scattered Scattered electron
     * @return y
     */
    static auto calculate_y(const Particle& beam,
                            const Particle& scattered) -> double {
        if (!detail::is_valid_4vec(beam) || !detail::is_valid_4vec(scattered) || beam.E() == 0.0) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        double nu = beam.E() - scattered.E();
        return nu / beam.E();
    }

    /**
     * @brief Calculate DIS kinematics
     */
    struct DISKinematics {
        double Q2;
        double W;
        double x_bj;
        double y;
        double nu;
    };

    static auto calculate_DIS(const Particle& beam,
                              const Particle& scattered,
                              const Particle& target) -> DISKinematics {
        DISKinematics kin;
        kin.Q2 = calculate_Q2(beam, scattered);
        kin.W = calculate_W(beam, scattered, target);
        kin.nu = beam.E() - scattered.E();
        kin.y = calculate_y(beam, scattered);
        kin.x_bj = calculate_x_bjorken(kin.Q2, kin.nu, target.mass());
        return kin;
    }

    /**
     * @brief Calculate opening angle between two particles (degrees)
     */
    static auto opening_angle(const Particle& p1, const Particle& p2) -> double {
        if (!detail::is_valid_4vec(p1) || !detail::is_valid_4vec(p2) || p1.p() == 0.0 || p2.p() == 0.0) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        double dot = p1.px() * p2.px() + p1.py() * p2.py() + p1.pz() * p2.pz();
        double cos_theta = dot / (p1.p() * p2.p());
        cos_theta = std::max(-1.0, std::min(1.0, cos_theta));  // Clamp to [-1, 1]
        return std::acos(cos_theta) * 180.0 / M_PI;
    }

    /**
     * @brief Calculate boost to center-of-mass frame
     */
    struct BoostVector {
        double beta_x, beta_y, beta_z, gamma;
    };

    static auto boost_to_cm(const Particle& p1, const Particle& p2) -> BoostVector {
        double px_tot = p1.px() + p2.px();
        double py_tot = p1.py() + p2.py();
        double pz_tot = p1.pz() + p2.pz();
        double E_tot = p1.E() + p2.E();

        double p_tot = std::sqrt(px_tot * px_tot + py_tot * py_tot + pz_tot * pz_tot);
        double gamma = E_tot / std::sqrt(E_tot * E_tot - p_tot * p_tot);

        BoostVector boost;
        boost.beta_x = px_tot / E_tot;
        boost.beta_y = py_tot / E_tot;
        boost.beta_z = pz_tot / E_tot;
        boost.gamma = gamma;

        return boost;
    }

    /**
     * @brief Calculate t (Mandelstam variable)
     * For photoproduction: t = (p_gamma - p_meson)²
     */
    static auto calculate_t(const Particle& photon,
                            const CompositeParticle& meson) -> double {
        double dpx = photon.px() - meson.px();
        double dpy = photon.py() - meson.py();
        double dpz = photon.pz() - meson.pz();

        // Calculate energy from momentum and mass
        double E_photon = std::sqrt(photon.p() * photon.p() + photon.mass() * photon.mass());
        double E_meson = meson.energy();
        double dE = E_photon - E_meson;

        return dE * dE - (dpx * dpx + dpy * dpy + dpz * dpz);
    }
};

}  // namespace physics
