#pragma once

#include <hipoeventiterator.h>
#include <reader.h>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/physics/ParticleUtils.hpp>

#include <functional>
#include <map>
#include <vector>

namespace clas12::framework {

/**
 * @brief Type alias for storing particles grouped by their PID type
 */
using ParticleMap = std::map<clas12::PIDs, std::vector<clas12::Particle>>;

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
 * auto particle_svc = service<clas12::framework::ParticleSelectorService>("ParticleSelectorService");
 * auto particles = particle_svc->extract_particles(bank);
 * auto electrons = particle_svc->get_electrons(particles);
 * auto filtered = particle_svc->with_momentum_range(electrons, 1.0, 10.0);
 * @endcode
 */
class ParticleSelectorService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "ParticleSelectorService"; }

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
    void set_default_types(const std::vector<clas12::PIDs>& types) {
        m_default_types = types;
    }

    /**
     * @brief Get the default particle types
     */
    auto get_default_types() const -> const std::vector<clas12::PIDs>& {
        return m_default_types;
    }

    // =========================================================================
    // Particle Extraction
    // =========================================================================

    /**
     * @brief Check if a particle matches a specific PID type
     */
    static auto is_type(int pid, clas12::PIDs type) -> bool {
        return pid == static_cast<int>(type);
    }

    /**
     * @brief Extract all particles from a bank and group them by type
     *
     * @param REC_Particle The HIPO particle bank
     * @param types Vector of particle types to extract (empty = use defaults)
     * @return ParticleMap containing particles grouped by their PID
     */
    auto extract_particles(hipo::bank& REC_Particle, const std::vector<clas12::PIDs>& types = {}) const -> ParticleMap {
        ParticleMap particle_map;

        const auto& target_types = types.empty() ? m_default_types : types;

        for (const auto& type : target_types) {
            particle_map[type] = std::vector<clas12::Particle>();
        }

        for (int i = 0; i < REC_Particle.rows(); i++) {
            const int pid = REC_Particle.get<int>("pid", i);

            clas12::PIDs particle_type;
            bool found = false;
            for (const auto& type : target_types) {
                if (is_type(pid, type)) {
                    particle_type = type;
                    found = true;
                    break;
                }
            }

            if (!found) continue;

            const int status = REC_Particle.get<int>("status", i);
            const int charge = REC_Particle.get<int>("charge", i);
            const double px = REC_Particle.get<double>("px", i);
            const double py = REC_Particle.get<double>("py", i);
            const double pz = REC_Particle.get<double>("pz", i);
            const double vx = REC_Particle.get<double>("vx", i);
            const double vy = REC_Particle.get<double>("vy", i);
            const double vz = REC_Particle.get<double>("vz", i);
            const double vt = REC_Particle.get<double>("vt", i);
            const double beta = REC_Particle.get<double>("beta", i);
            const double chi2pid = REC_Particle.get<double>("chi2pid", i);

            const double mass = clas12::physics::ParticleUtils::get_mass(pid);
            const double E = clas12::physics::ParticleUtils::compute_energy(px, py, pz, pid);

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
    auto get_electrons(const ParticleMap& particles) const -> const std::vector<clas12::Particle>& {
        static const std::vector<clas12::Particle> empty;
        auto it = particles.find(clas12::PIDs::ELECTRON);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get photons from a particle map
     */
    auto get_photons(const ParticleMap& particles) const -> const std::vector<clas12::Particle>& {
        static const std::vector<clas12::Particle> empty;
        auto it = particles.find(clas12::PIDs::PHOTON);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get protons from a particle map
     */
    auto get_protons(const ParticleMap& particles) const -> const std::vector<clas12::Particle>& {
        static const std::vector<clas12::Particle> empty;
        auto it = particles.find(clas12::PIDs::PROTON);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get π+ from a particle map
     */
    auto get_pions_plus(const ParticleMap& particles) const -> const std::vector<clas12::Particle>& {
        static const std::vector<clas12::Particle> empty;
        auto it = particles.find(clas12::PIDs::PIPLUS);
        return (it != particles.end()) ? it->second : empty;
    }

    /**
     * @brief Get π- from a particle map
     */
    auto get_pions_minus(const ParticleMap& particles) const -> const std::vector<clas12::Particle>& {
        static const std::vector<clas12::Particle> empty;
        auto it = particles.find(clas12::PIDs::PIMINUS);
        return (it != particles.end()) ? it->second : empty;
    }

    auto get_particles_by_charge(hipo::bank& REC_Particle) const -> std::map<int, std::vector<clas12::Particle>> {
        std::map<int, std::vector<clas12::Particle>> charge_map;

        for (int i = 0; i < REC_Particle.rows(); i++) {
            const int charge = REC_Particle.get<int>("charge", i);
            const int pid = REC_Particle.get<int>("pid", i);
            const int status = REC_Particle.get<int>("status", i);
            const double px = REC_Particle.get<double>("px", i);
            const double py = REC_Particle.get<double>("py", i);
            const double pz = REC_Particle.get<double>("pz", i);
            const double vx = REC_Particle.get<double>("vx", i);
            const double vy = REC_Particle.get<double>("vy", i);
            const double vz = REC_Particle.get<double>("vz", i);
            const double vt = REC_Particle.get<double>("vt", i);
            const double beta = REC_Particle.get<double>("beta", i);
            const double chi2pid = REC_Particle.get<double>("chi2pid", i);

            const double mass = clas12::physics::ParticleUtils::get_mass(pid);
            const double E = clas12::physics::ParticleUtils::compute_energy(px, py, pz, pid);

            clas12::Particle particle(pid, status, i, charge, mass, px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);

            charge_map[charge].emplace_back(particle);
        }

        return charge_map;
    }

    // =========================================================================
    // Filtering Methods
    // =========================================================================

    /**
     * @brief Filter particles based on a custom predicate
     */
    auto filter(const std::vector<clas12::Particle>& particles,
                std::function<bool(const clas12::Particle&)> predicate) const
        -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> filtered;
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
    auto with_momentum_range(const std::vector<clas12::Particle>& particles,
                             double min_p, double max_p) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_p, max_p](const clas12::Particle& p) {
            double momentum = p.p();
            return momentum >= min_p && momentum <= max_p;
        });
    }

    /**
     * @brief Get particles with specific charge
     */
    auto with_charge(const std::vector<clas12::Particle>& particles,
                     int charge) const -> std::vector<clas12::Particle> {
        return filter(particles, [charge](const clas12::Particle& p) {
            return p.charge() == charge;
        });
    }

    /**
     * @brief Get particles with chi2pid quality in a specific range
     */
    auto with_chi2pid_range(const std::vector<clas12::Particle>& particles,
                            double min_chi2, double max_chi2) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_chi2, max_chi2](const clas12::Particle& p) {
            double chi2 = p.chi2pid();
            return chi2 >= min_chi2 && chi2 <= max_chi2;
        });
    }

    /**
     * @brief Get particles with vertex z-position in a specific range
     */
    auto with_vz_range(const std::vector<clas12::Particle>& particles,
                       double min_vz, double max_vz) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_vz, max_vz](const clas12::Particle& p) {
            double vz = p.vz();
            return vz >= min_vz && vz <= max_vz;
        });
    }

    /**
     * @brief Get particles with theta angle in a specific range (degrees)
     */
    auto with_theta_range(const std::vector<clas12::Particle>& particles,
                          double min_theta, double max_theta) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_theta, max_theta](const clas12::Particle& p) {
            double theta = p.theta();
            return theta >= min_theta && theta <= max_theta;
        });
    }

    /**
     * @brief Get trigger particles (negative status in specified range)
     */
    auto get_trigger_particles(const std::vector<clas12::Particle>& particles,
                               std::pair<double, double> status_range = {2000, 4000}) const
        -> std::vector<clas12::Particle> {
        return filter(particles, [status_range](const clas12::Particle& p) {
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
    auto get_forward_detector(const std::vector<clas12::Particle>& particles) const
        -> std::vector<clas12::Particle> {
        return get_trigger_particles(particles, {2000, 4000});
    }

    /**
     * @brief Get central detector particles (status 4000-6000)
     */
    auto get_central_detector(const std::vector<clas12::Particle>& particles) const
        -> std::vector<clas12::Particle> {
        return filter(particles, [](const clas12::Particle& p) {
            const double absStatus = std::abs(p.status());
            return absStatus >= 4000 && absStatus < 6000;
        });
    }

   private:
    std::vector<clas12::PIDs> m_default_types = {
        clas12::PIDs::ELECTRON, clas12::PIDs::POSITRON,
        clas12::PIDs::PHOTON,
        clas12::PIDs::PROTON, clas12::PIDs::NEUTRON,
        clas12::PIDs::PIPLUS, clas12::PIDs::PIMINUS,
        clas12::PIDs::KPLUS, clas12::PIDs::KMINUS};
};

}  // namespace clas12::framework
