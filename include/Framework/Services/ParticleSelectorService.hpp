#pragma once

#include <Framework/IService.hpp>
#include <Framework/StatusCode.hpp>
#include <CLAS/Particle.hpp>
#include <CLAS/PIDs.hpp>
#include <Physics/ParticleUtils.hpp>
#include <hipo4/reader.h>
#include <hipo4/hipoeventiterator.h>

#include <functional>
#include <map>
#include <vector>

namespace Framework {

/**
 * @brief Type alias for storing particles grouped by their PID type
 */
using ParticleMap = std::map<CLAS::PIDs, std::vector<CLAS::Particle>>;

/**
 * @brief Service for selecting and filtering particles from HIPO banks
 *
 * This service provides particle extraction and filtering functionality
 * that can be shared across multiple algorithms. It supports:
 * - Extracting particles from REC::Particle bank
 * - Filtering by various criteria (momentum, charge, vertex, etc.)
 * - Configurable default particle types
 * - Caching of particle maps per event (optional)
 *
 * Usage in algorithms:
 * @code
 * auto particle_svc = service<Framework::ParticleSelectorService>("ParticleSelectorService");
 * auto particles = particle_svc->extract_particles(bank);
 * auto electrons = particle_svc->get_electrons(particles);
 * auto filtered = particle_svc->with_momentum_range(electrons, 1.0, 10.0);
 * @endcode
 */
class ParticleSelectorService : public Service {
public:
    explicit ParticleSelectorService(std::string name = "ParticleSelectorService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    // =========================================================================
    // Configuration
    // =========================================================================

    /**
     * @brief Set the default particle types to extract
     */
    void set_default_types(const std::vector<CLAS::PIDs>& types) {
        m_default_types = types;
    }

    /**
     * @brief Get the default particle types
     */
    auto get_default_types() const -> const std::vector<CLAS::PIDs>& {
        return m_default_types;
    }

    // =========================================================================
    // Particle Extraction
    // =========================================================================

    /**
     * @brief Check if a particle matches a specific PID type
     */
    static auto is_type(int pid, CLAS::PIDs type) -> bool {
        return pid == static_cast<int>(type);
    }

    /**
     * @brief Extract all particles from a bank and group them by type
     *
     * @param REC_Particle The HIPO particle bank
     * @param types Vector of particle types to extract (empty = use defaults)
     * @return ParticleMap containing particles grouped by their PID
     */
    auto extract_particles(hipo::bank_map& REC_Particle, const std::vector<CLAS::PIDs>& types = {}) const -> ParticleMap {
        ParticleMap particle_map;

        const auto& target_types = types.empty() ? m_default_types : types;

        for (const auto& type : target_types) {
            particle_map[type] = std::vector<CLAS::Particle>();
        }

        for (int i = 0; i < REC_Particle.get_rows(); i++) {
            const int pid = REC_Particle.get_as<int>("pid", i);

            CLAS::PIDs particle_type;
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

            const double mass = Physics::ParticleUtils::get_mass(pid);
            const double E = Physics::ParticleUtils::compute_energy(px, py, pz, pid);

            particle_map[particle_type].emplace_back(
                pid, status, i, charge, mass,
                px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);
        }

        return particle_map;
    }

    // =========================================================================
    // Convenience Accessors
    // =========================================================================

    /**
     * @brief Get electrons from a particle map
     */
    auto get_electrons(const ParticleMap& particles) const -> const std::vector<CLAS::Particle>& {
        static const std::vector<CLAS::Particle> empty;
        auto it = particles.find(CLAS::PIDs::ELECTRON);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get photons from a particle map
     */
    auto get_photons(const ParticleMap& particles) const -> const std::vector<CLAS::Particle>& {
        static const std::vector<CLAS::Particle> empty;
        auto it = particles.find(CLAS::PIDs::PHOTON);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get protons from a particle map
     */
    auto get_protons(const ParticleMap& particles) const -> const std::vector<CLAS::Particle>& {
        static const std::vector<CLAS::Particle> empty;
        auto it = particles.find(CLAS::PIDs::PROTON);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get π+ from a particle map
     */
    auto get_pions_plus(const ParticleMap& particles) const -> const std::vector<CLAS::Particle>& {
        static const std::vector<CLAS::Particle> empty;
        auto it = particles.find(CLAS::PIDs::PIPLUS);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get π- from a particle map
     */
    auto get_pions_minus(const ParticleMap& particles) const -> const std::vector<CLAS::Particle>& {
        static const std::vector<CLAS::Particle> empty;
        auto it = particles.find(CLAS::PIDs::PIMINUS);
        return (it != particles.end()) ? it->second : empty;
    }

    auto get_particles_by_charge(const ParticleMap& particles) const -> std::map<int, std::vector<CLAS::Particle>> {
        std::map<int, std::vector<CLAS::Particle>> charge_map;
        for (const auto& [type, particles] : particles) {
            for (const auto& particle : particles) {
                charge_map[particle.charge()].emplace_back(particle);
            }
        }
        return charge_map;
    }

    // =========================================================================
    // Filtering Methods
    // =========================================================================

    /**
     * @brief Filter particles based on a custom predicate
     */
    auto filter(const std::vector<CLAS::Particle>& particles,
                std::function<bool(const CLAS::Particle&)> predicate) const
        -> std::vector<CLAS::Particle> {
        std::vector<CLAS::Particle> filtered;
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
     */
    auto with_momentum_range(const std::vector<CLAS::Particle>& particles,
                             double min_p, double max_p) const -> std::vector<CLAS::Particle> {
        return filter(particles, [min_p, max_p](const CLAS::Particle& p) {
            double momentum = p.p();
            return momentum >= min_p && momentum <= max_p;
        });
    }

    /**
     * @brief Get particles with specific charge
     */
    auto with_charge(const std::vector<CLAS::Particle>& particles,
                     int charge) const -> std::vector<CLAS::Particle> {
        return filter(particles, [charge](const CLAS::Particle& p) {
            return p.charge() == charge;
        });
    }

    /**
     * @brief Get particles with chi2pid quality in a specific range
     */
    auto with_chi2pid_range(const std::vector<CLAS::Particle>& particles,
                            double min_chi2, double max_chi2) const -> std::vector<CLAS::Particle> {
        return filter(particles, [min_chi2, max_chi2](const CLAS::Particle& p) {
            double chi2 = p.chi2pid();
            return chi2 >= min_chi2 && chi2 <= max_chi2;
        });
    }

    /**
     * @brief Get particles with vertex z-position in a specific range
     */
    auto with_vz_range(const std::vector<CLAS::Particle>& particles,
                       double min_vz, double max_vz) const -> std::vector<CLAS::Particle> {
        return filter(particles, [min_vz, max_vz](const CLAS::Particle& p) {
            double vz = p.vz();
            return vz >= min_vz && vz <= max_vz;
        });
    }

    /**
     * @brief Get particles with theta angle in a specific range (degrees)
     */
    auto with_theta_range(const std::vector<CLAS::Particle>& particles,
                          double min_theta, double max_theta) const -> std::vector<CLAS::Particle> {
        return filter(particles, [min_theta, max_theta](const CLAS::Particle& p) {
            double theta = p.theta();
            return theta >= min_theta && theta <= max_theta;
        });
    }

    /**
     * @brief Get trigger particles (negative status in specified range)
     */
    auto get_trigger_particles(const std::vector<CLAS::Particle>& particles,
                               std::pair<double, double> status_range = {2000, 4000}) const
        -> std::vector<CLAS::Particle> {
        return filter(particles, [status_range](const CLAS::Particle& p) {
            const double absStatus = std::abs(p.status());
            return p.status() < 0.0 &&
                   p.p() != 0.0 &&
                   status_range.first <= absStatus &&
                   absStatus < status_range.second;
        });
    }

    /**
     * @brief Get forward detector particles (status 2000-4000)
     */
    auto get_forward_detector(const std::vector<CLAS::Particle>& particles) const
        -> std::vector<CLAS::Particle> {
        return get_trigger_particles(particles, {2000, 4000});
    }

    /**
     * @brief Get central detector particles (status 4000-6000)
     */
    auto get_central_detector(const std::vector<CLAS::Particle>& particles) const
        -> std::vector<CLAS::Particle> {
        return filter(particles, [](const CLAS::Particle& p) {
            const double absStatus = std::abs(p.status());
            return absStatus >= 4000 && absStatus < 6000;
        });
    }

private:
    std::vector<CLAS::PIDs> m_default_types = {
        CLAS::PIDs::ELECTRON, CLAS::PIDs::POSITRON,
        CLAS::PIDs::PHOTON,
        CLAS::PIDs::PROTON, CLAS::PIDs::NEUTRON,
        CLAS::PIDs::PIPLUS, CLAS::PIDs::PIMINUS,
        CLAS::PIDs::KPLUS, CLAS::PIDs::KMINUS
    };
};

}  // namespace Framework
