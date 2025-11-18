#pragma once

// C++ headers
#include <map>
#include <vector>
#include <functional>

// HIPO headers
#include <hipo4/reader.h>
#include <hipo4/hipoeventiterator.h>

// Project headers
#include <Core/Particle.hpp>
#include <Core/PIDs.hpp>
#include <Core/Helpers.hpp>

namespace Core {

/**
 * @brief Type alias for storing particles grouped by their PID type
 */
using ParticleMap = std::map<Core::PIDs, std::vector<Core::Particle>>;

/**
 * @brief Utility class for selecting and filtering particles from HIPO banks
 *
 * This class provides convenient methods for particle selection and filtering,
 * making analysis code cleaner and more readable.
 */
class ParticleSelector {
public:
    /**
     * @brief Check if a particle matches a specific PID type
     *
     * @param pid The particle ID to check
     * @param type The PID type to match against
     * @return true if the particle matches the type
     */
    static auto is_type(int pid, Core::PIDs type) -> bool {
        return pid == static_cast<int>(type);
    }

    /**
     * @brief Extract all particles from a bank and group them by type
     *
     * @param REC_Particle The HIPO particle bank
     * @param types Vector of particle types to extract (empty = extract all known types)
     * @return ParticleMap containing particles grouped by their PID
     */
    static auto extract_particles(hipo::bank_map& REC_Particle, const std::vector<Core::PIDs>& types = {}) -> ParticleMap {
        ParticleMap particle_map;

        // If no types specified, extract all common particle types
        std::vector<Core::PIDs> target_types = types.empty() ?
            std::vector<Core::PIDs>{
                Core::PIDs::ELECTRON, Core::PIDs::POSITRON,
                Core::PIDs::PROTON, Core::PIDs::NEUTRON,
                Core::PIDs::PIPLUS, Core::PIDs::PIMINUS,
                Core::PIDs::KPLUS, Core::PIDs::KMINUS
            } : types;

        for (const auto& type : target_types) {
            particle_map[type] = std::vector<Core::Particle>();
        }

        for (int i = 0; i < REC_Particle.get_rows(); i++) {
            const int pid = REC_Particle.get_as<int>("pid", i);
            Core::PIDs particle_type;
            bool found = false;
            for (const auto& type : target_types) {
                if (is_type(pid, type)) {
                    particle_type = type;
                    found = true;
                    break;
                }
            }

            if (!found) continue;

            const int status = REC_Particle.get_as<int>("status", i);
            const int charge = REC_Particle.get_as<int>("charge", i);
            const double px = REC_Particle.get_as<double>("px", i);
            const double py = REC_Particle.get_as<double>("py", i);
            const double pz = REC_Particle.get_as<double>("pz", i);
            const double vx = REC_Particle.get_as<double>("vx", i);
            const double vy = REC_Particle.get_as<double>("vy", i);
            const double vz = REC_Particle.get_as<double>("vz", i);
            const double vt = REC_Particle.get_as<double>("vt", i);
            const double beta = REC_Particle.get_as<double>("beta", i);
            const double chi2pid = REC_Particle.get_as<double>("chi2pid", i);

            const double mass = Core::get_mass(pid);
            const double E = Core::compute_energy(px, py, pz, pid);

            particle_map[particle_type].emplace_back(
                pid, status, i, charge, mass,
                px, py, pz, E, vx, vy, vz, vt, beta, chi2pid
            );
        }

        return particle_map;
    }

    /**
     * @brief Filter particles based on a custom predicate
     *
     * @param particles Vector of particles to filter
     * @param predicate Function that returns true for particles to keep
     * @return Filtered vector of particles
     */
    static auto filter(const std::vector<Core::Particle>& particles,
                      std::function<bool(const Core::Particle&)> predicate) -> std::vector<Core::Particle> {
        std::vector<Core::Particle> filtered;
        filtered.reserve(particles.size());

        for (const auto& particle : particles) {
            if (predicate(particle)) {
                filtered.push_back(particle);
            }
        }

        return filtered;
    }

    /**
     * @brief Get particles with momentum in a specific range
     *
     * @param particles Vector of particles to filter
     * @param min_p Minimum momentum (GeV/c)
     * @param max_p Maximum momentum (GeV/c)
     * @return Filtered vector of particles
     */
    static auto with_momentum_range(const std::vector<Core::Particle>& particles,
                                     double min_p, double max_p) -> std::vector<Core::Particle> {
        return filter(particles, [min_p, max_p](const Core::Particle& p) {
            double momentum = p.p();
            return momentum >= min_p && momentum <= max_p;
        });
    }

    /**
     * @brief Get particles with specific charge
     *
     * @param particles Vector of particles to filter
     * @param charge Required charge
     * @return Filtered vector of particles
     */
    static auto with_charge(const std::vector<Core::Particle>& particles,
                            int charge) -> std::vector<Core::Particle> {
        return filter(particles, [charge](const Core::Particle& p) {
            return p.charge() == charge;
        });
    }

    /**
     * @brief Get particles with chi2pid quality in a specific range
     *
     * @param particles Vector of particles to filter
     * @param min_chi2 Minimum chi2pid value
     * @param max_chi2 Maximum chi2pid value
     * @return Filtered vector of particles
     */
    static auto with_chi2pid_range(const std::vector<Core::Particle>& particles,
                                    double min_chi2, double max_chi2) -> std::vector<Core::Particle> {
        return filter(particles, [min_chi2, max_chi2](const Core::Particle& p) {
            double chi2 = p.chi2pid();
            return chi2 >= min_chi2 && chi2 <= max_chi2;
        });
    }

    /**
     * @brief Get particles with vertex z-position in a specific range
     *
     * @param particles Vector of particles to filter
     * @param min_vz Minimum vz value (cm)
     * @param max_vz Maximum vz value (cm)
     * @return Filtered vector of particles
     */
    static auto with_vz_range(const std::vector<Core::Particle>& particles,
                               double min_vz, double max_vz) -> std::vector<Core::Particle> {
        return filter(particles, [min_vz, max_vz](const Core::Particle& p) {
            double vz = p.vz();
            return vz >= min_vz && vz <= max_vz;
        });
    }

    /**
     * @brief Get trigger particles (negative status in specified range)
     *
     * @param particles Vector of particles to filter
     * @param status_range Pair of min/max absolute status values
     * @return Filtered vector of trigger particles
     */
    static auto get_trigger_particles(const std::vector<Core::Particle>& particles,
                                       std::pair<double, double> status_range = {2000, 4000}) -> std::vector<Core::Particle> {
        return filter(particles, [status_range](const Core::Particle& p) {
            const double absStatus = std::abs(p.status());
            return p.status() < 0.0 &&
                   p.p() != 0.0 &&
                   status_range.first <= absStatus &&
                   absStatus < status_range.second;
        });
    }
};

}  // namespace Core
