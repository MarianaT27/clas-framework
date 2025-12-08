#pragma once

#include "CLAS/Constants.hpp"
#include "CLAS/Particle.hpp"
#include "CompositeParticle.hpp"
#include "fmt/base.h"
#include <vector>
#include <cmath>

namespace Physics {

/**
 * @brief Kinematic calculator utilities for particle physics
 *
 * Provides functions for calculating various kinematic quantities
 * used in particle physics analyses.
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
    static auto missing_mass_squared(const CLAS::Particle& beam,
                                     const CLAS::Particle& target,
                                     const std::vector<CLAS::Particle>& detected_particles) -> double {
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
    static auto missing_mass(const CLAS::Particle& beam,
                             const CLAS::Particle& target,
                             const std::vector<CLAS::Particle>& detected_particles) -> double {
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
    static auto calculate_W(const CLAS::Particle& beam,
                           const CLAS::Particle& scattered,
                           const CLAS::Particle& target) -> double {
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
    static auto calculate_Q2(const CLAS::Particle& beam, const CLAS::Particle& scattered) -> double {
        return 4.0 * beam.E() * scattered.E() * std::pow(std::sin(scattered.theta() * CLAS::Constants::DEG_TO_RAD / 2.0), 2);
    }

    /**
     * @brief Calculate Bjorken x
     * @param Q2 Four-momentum transfer squared
     * @param nu Energy transfer (beam - scattered energy)
     * @param M Target mass
     * @return Bjorken x
     */
    static auto calculate_x_bjorken(double Q2, double nu, double M = 0.938) -> double {
        return Q2 / (2.0 * M * nu);
    }

    /**
     * @brief Calculate inelasticity y
     * @param beam Beam electron
     * @param scattered Scattered electron
     * @return y
     */
    static auto calculate_y(const CLAS::Particle& beam,
                           const CLAS::Particle& scattered) -> double {
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

    static auto calculate_DIS(const CLAS::Particle& beam,
                             const CLAS::Particle& scattered,
                             const CLAS::Particle& target) -> DISKinematics {
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
    static auto opening_angle(const CLAS::Particle& p1, const CLAS::Particle& p2) -> double {
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

    static auto boost_to_cm(const CLAS::Particle& p1, const CLAS::Particle& p2) -> BoostVector {
        double px_tot = p1.px() + p2.px();
        double py_tot = p1.py() + p2.py();
        double pz_tot = p1.pz() + p2.pz();
        double E_tot = p1.E() + p2.E();

        double p_tot = std::sqrt(px_tot * px_tot + py_tot * py_tot + pz_tot * pz_tot);
        double beta = p_tot / E_tot;
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
    static auto calculate_t(const CLAS::Particle& photon,
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

}  // namespace Physics
