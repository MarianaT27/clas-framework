#pragma once

#include <Core/Particle.hpp>
#include <Core/Constantes.hpp>
#include <cmath>
#include <vector>

namespace Core {

/**
 * @brief Collection of kinematic calculation functions for physics analysis
 */
namespace Kinematics {

    /**
     * @brief Beam energy (default CLAS12 RG-A: 10.6 GeV)
     */
    struct BeamConfig {
        double energy = 10.6;  // GeV
        double px = 0.0;
        double py = 0.0;
        double pz = 10.6;
    };

    /**
     * @brief Calculate Q² (four-momentum transfer squared)
     *
     * Q² = -q² = -(k - k')² where k is beam, k' is scattered electron
     *
     * @param electron Scattered electron
     * @param beam Beam configuration
     * @return Q² in GeV²
     */
    inline double Q2(const Core::Particle& electron, const BeamConfig& beam = {}) {
        double E_beam = beam.energy;
        double E_e = electron.E();
        double px_e = electron.px();
        double py_e = electron.py();
        double pz_e = electron.pz();

        // q = k - k'
        double qx = beam.px - px_e;
        double qy = beam.py - py_e;
        double qz = beam.pz - pz_e;
        double q0 = E_beam - E_e;

        // Q² = -q²= -(q0² - qx² - qy² - qz²)
        return -(q0 * q0 - qx * qx - qy * qy - qz * qz);
    }

    /**
     * @brief Calculate W (invariant mass of hadronic system)
     *
     * W² = (P + q)² where P is target proton, q is virtual photon
     *
     * @param electron Scattered electron
     * @param beam Beam configuration
     * @return W in GeV
     */
    inline double W(const Core::Particle& electron, const BeamConfig& beam = {}) {
        double E_beam = beam.energy;
        double E_e = electron.E();
        double px_e = electron.px();
        double py_e = electron.py();
        double pz_e = electron.pz();

        double M_p = Core::Constantes::ProtonMass;

        // Virtual photon four-momentum
        double qx = beam.px - px_e;
        double qy = beam.py - py_e;
        double qz = beam.pz - pz_e;
        double q0 = E_beam - E_e;

        // W² = (P + q)² = M² + 2M*q0 - q²
        double q2 = q0 * q0 - qx * qx - qy * qy - qz * qz;
        double W2 = M_p * M_p + 2.0 * M_p * q0 - q2;

        return std::sqrt(std::max(0.0, W2));
    }

    /**
     * @brief Calculate ν (energy transfer)
     *
     * ν = E_beam - E_electron
     *
     * @param electron Scattered electron
     * @param beam Beam configuration
     * @return ν in GeV
     */
    inline double nu(const Core::Particle& electron, const BeamConfig& beam = {}) {
        return beam.energy - electron.E();
    }

    /**
     * @brief Calculate x_Bjorken (Bjorken x)
     *
     * x_B = Q² / (2 M ν)
     *
     * @param electron Scattered electron
     * @param beam Beam configuration
     * @return x_Bjorken (dimensionless)
     */
    inline double x_bjorken(const Core::Particle& electron, const BeamConfig& beam = {}) {
        double q2 = Q2(electron, beam);
        double nu_val = nu(electron, beam);
        double M_p = Core::Constantes::ProtonMass;

        return q2 / (2.0 * M_p * nu_val);
    }

    /**
     * @brief Calculate y (inelasticity)
     *
     * y = ν / E_beam
     *
     * @param electron Scattered electron
     * @param beam Beam configuration
     * @return y (dimensionless)
     */
    inline double inelasticity(const Core::Particle& electron, const BeamConfig& beam = {}) {
        return nu(electron, beam) / beam.energy;
    }

    /**
     * @brief Calculate missing mass for ep → e'pX reaction
     *
     * @param electron Scattered electron
     * @param proton Detected proton
     * @param beam Beam configuration
     * @return Missing mass in GeV
     */
    inline double missing_mass(const Core::Particle& electron,
                               const Core::Particle& proton,
                               const BeamConfig& beam = {}) {
        double M_p = Core::Constantes::ProtonMass;

        // Initial state: beam + target
        double E_init = beam.energy + M_p;
        double px_init = beam.px;
        double py_init = beam.py;
        double pz_init = beam.pz;

        // Final state: electron + proton
        double E_final = electron.E() + proton.E();
        double px_final = electron.px() + proton.px();
        double py_final = electron.py() + proton.py();
        double pz_final = electron.pz() + proton.pz();

        // Missing four-momentum
        double E_miss = E_init - E_final;
        double px_miss = px_init - px_final;
        double py_miss = py_init - py_final;
        double pz_miss = pz_init - pz_final;

        // Missing mass² = E² - p²
        double MM2 = E_miss * E_miss - px_miss * px_miss - py_miss * py_miss - pz_miss * pz_miss;

        return std::sqrt(std::max(0.0, MM2));
    }

    /**
     * @brief Calculate invariant mass of two particles
     *
     * @param p1 First particle
     * @param p2 Second particle
     * @return Invariant mass in GeV
     */
    inline double invariant_mass(const Core::Particle& p1, const Core::Particle& p2) {
        double E_total = p1.E() + p2.E();
        double px_total = p1.px() + p2.px();
        double py_total = p1.py() + p2.py();
        double pz_total = p1.pz() + p2.pz();

        double M2 = E_total * E_total - px_total * px_total - py_total * py_total - pz_total * pz_total;

        return std::sqrt(std::max(0.0, M2));
    }

    /**
     * @brief Calculate invariant mass of multiple particles
     *
     * @param particles Vector of particles
     * @return Invariant mass in GeV
     */
    inline double invariant_mass(const std::vector<Core::Particle>& particles) {
        double E_total = 0.0;
        double px_total = 0.0;
        double py_total = 0.0;
        double pz_total = 0.0;

        for (const auto& p : particles) {
            E_total += p.E();
            px_total += p.px();
            py_total += p.py();
            pz_total += p.pz();
        }

        double M2 = E_total * E_total - px_total * px_total - py_total * py_total - pz_total * pz_total;

        return std::sqrt(std::max(0.0, M2));
    }

    /**
     * @brief Calculate angle between two particles (in degrees)
     *
     * @param p1 First particle
     * @param p2 Second particle
     * @return Angle in degrees
     */
    inline double angle_between(const Core::Particle& p1, const Core::Particle& p2) {
        double px1 = p1.px(), py1 = p1.py(), pz1 = p1.pz();
        double px2 = p2.px(), py2 = p2.py(), pz2 = p2.pz();

        double p1_mag = std::sqrt(px1 * px1 + py1 * py1 + pz1 * pz1);
        double p2_mag = std::sqrt(px2 * px2 + py2 * py2 + pz2 * pz2);

        if (p1_mag == 0.0 || p2_mag == 0.0) return 0.0;

        double dot = px1 * px2 + py1 * py2 + pz1 * pz2;
        double cos_angle = dot / (p1_mag * p2_mag);

        // Clamp to [-1, 1] for numerical stability
        cos_angle = std::max(-1.0, std::min(1.0, cos_angle));

        return std::acos(cos_angle) * Core::Constantes::rad_to_deg;
    }


    /**
     * @brief Calculate momentum transfer t for meson production
     *
     * t = (q - p_meson)²
     *
     * @param electron Scattered electron
     * @param meson Produced meson
     * @param beam Beam configuration
     * @return -t in GeV²
     */
    inline double momentum_transfer_t(const Core::Particle& electron,
                                      const Core::Particle& meson,
                                      const BeamConfig& beam = {}) {
        // Virtual photon
        double qx = beam.px - electron.px();
        double qy = beam.py - electron.py();
        double qz = beam.pz - electron.pz();
        double q0 = beam.energy - electron.E();

        // t = (q - p_meson)²
        double tx = qx - meson.px();
        double ty = qy - meson.py();
        double tz = qz - meson.pz();
        double t0 = q0 - meson.E();

        double t = t0 * t0 - tx * tx - ty * ty - tz * tz;

        return -t;  // Return -t (positive value)
    }

    /**
     * @brief Calculate missing energy for exclusive reactions
     *
     * @param electron Scattered electron
     * @param detected_particles Vector of detected final state particles
     * @param beam Beam configuration
     * @return Missing energy in GeV
     */
    inline double missing_energy(const Core::Particle& electron,
                                  const std::vector<Core::Particle>& detected_particles,
                                  const BeamConfig& beam = {}) {
        double M_p = Core::Constantes::ProtonMass;
        double E_init = beam.energy + M_p;

        double E_final = electron.E();
        for (const auto& p : detected_particles) {
            E_final += p.E();
        }

        return E_init - E_final;
    }

    /**
     * @brief Calculate missing momentum for exclusive reactions
     *
     * @param electron Scattered electron
     * @param detected_particles Vector of detected final state particles
     * @param beam Beam configuration
     * @return Missing momentum magnitude in GeV
     */
    inline double missing_momentum(const Core::Particle& electron,
                                    const std::vector<Core::Particle>& detected_particles,
                                    const BeamConfig& beam = {}) {
        double px_init = beam.px;
        double py_init = beam.py;
        double pz_init = beam.pz;

        double px_final = electron.px();
        double py_final = electron.py();
        double pz_final = electron.pz();

        for (const auto& p : detected_particles) {
            px_final += p.px();
            py_final += p.py();
            pz_final += p.pz();
        }

        double px_miss = px_init - px_final;
        double py_miss = py_init - py_final;
        double pz_miss = pz_init - pz_final;

        return std::sqrt(px_miss * px_miss + py_miss * py_miss + pz_miss * pz_miss);
    }

}  // namespace Kinematics

}  // namespace Core
