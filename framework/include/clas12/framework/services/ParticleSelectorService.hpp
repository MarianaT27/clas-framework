#pragma once

#include <clas12/Detectors.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/physics/ParticleUtils.hpp>

#include <cmath>
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

namespace clas12::framework {

/**
 * @brief Find the first trigger electron from a vector of electrons.
 *
 * Picks the first electron with negative status whose |status| falls in
 * @p status_range and whose momentum is non-zero. Default range covers the
 * forward detector band. Free-function form so callers that already have a
 * `std::vector<Particle>` (e.g. via `Event::particles()`) can reuse the same
 * implementation as EventParticles::find_trigger_electron().
 */
[[nodiscard]] inline auto find_trigger_electron(
    const std::vector<clas12::Particle>& electrons,
    std::pair<double, double> status_range = {clas12::FD, clas12::CD}
) -> std::optional<clas12::Particle> {
    for (const auto& e : electrons) {
        const double abs_status = std::abs(e.status());
        if (e.status() < 0.0 && e.p() != 0.0 &&
            status_range.first <= abs_status && abs_status < status_range.second) {
            return e;
        }
    }
    return std::nullopt;
}

/**
 * @brief Per-event particle view: one REC::Particle read, indexed by pid and charge.
 *
 * Returned by ParticleSelectorService::extract_particles(). Owns the particle
 * vector and two lookup maps (pid -> particles, charge -> particles), all
 * populated in the same single-pass loop over the bank. Post-read lookups
 * are O(1).
 *
 * Missing keys return a reference to a static empty vector — safe to bind
 * with `const auto&` without worrying about lifetime.
 */
class EventParticles {
   public:
    /// Every extracted particle in bank-row order.
    [[nodiscard]] auto all() const -> const std::vector<clas12::Particle>& { return m_all; }

    /// Particles with the given PDG id. Empty vector if none matched.
    [[nodiscard]] auto by_pid(int pid) const -> const std::vector<clas12::Particle>& {
        auto it = m_by_pid.find(pid);
        return (it != m_by_pid.end()) ? it->second : empty();
    }

    [[nodiscard]] auto by_pid(clas12::PIDs pid) const -> const std::vector<clas12::Particle>& {
        return by_pid(static_cast<int>(pid));
    }

    /// Particles with the given charge. Empty vector if none matched.
    [[nodiscard]] auto by_charge(int charge) const -> const std::vector<clas12::Particle>& {
        auto it = m_by_charge.find(charge);
        return (it != m_by_charge.end()) ? it->second : empty();
    }

    /// All pid → particles groupings, for range-for iteration over types.
    [[nodiscard]] auto pid_groups() const -> const std::unordered_map<int, std::vector<clas12::Particle>>& {
        return m_by_pid;
    }

    /// All charge → particles groupings.
    [[nodiscard]] auto charge_groups() const -> const std::unordered_map<int, std::vector<clas12::Particle>>& {
        return m_by_charge;
    }

    [[nodiscard]] auto electrons()   const -> const std::vector<clas12::Particle>& { return by_pid(clas12::PIDs::ELECTRON); }
    [[nodiscard]] auto photons()     const -> const std::vector<clas12::Particle>& { return by_pid(clas12::PIDs::PHOTON); }
    [[nodiscard]] auto protons()     const -> const std::vector<clas12::Particle>& { return by_pid(clas12::PIDs::PROTON); }
    [[nodiscard]] auto pions_plus()  const -> const std::vector<clas12::Particle>& { return by_pid(clas12::PIDs::PIPLUS); }
    [[nodiscard]] auto pions_minus() const -> const std::vector<clas12::Particle>& { return by_pid(clas12::PIDs::PIMINUS); }

    /**
     * @brief Find the first trigger electron in this event's electrons vector.
     *
     * Thin wrapper around the free-function `find_trigger_electron` so
     * `particles.find_trigger_electron()` matches the old ParticleUtils idiom.
     */
    [[nodiscard]] auto find_trigger_electron(
        std::pair<double, double> status_range = {clas12::FD, clas12::CD}
    ) const -> std::optional<clas12::Particle> {
        return clas12::framework::find_trigger_electron(electrons(), status_range);
    }

   private:
    friend class ParticleSelectorService;

    static auto empty() -> const std::vector<clas12::Particle>& {
        static const std::vector<clas12::Particle> s_empty;
        return s_empty;
    }

    std::vector<clas12::Particle> m_all;
    std::unordered_map<int, std::vector<clas12::Particle>> m_by_pid;
    std::unordered_map<int, std::vector<clas12::Particle>> m_by_charge;
};

/**
 * @brief Service that reads REC::Particle once and serves up particles by pid/charge.
 *
 * Call extract_particles() at the top of process() to read the bank. The
 * returned EventParticles owns the data for the rest of the event — every
 * downstream access is O(1).
 *
 * Stateless filter helpers (with_momentum_range, with_charge, etc.) operate
 * on already-extracted std::vector<Particle> values and perform zero bank
 * reads.
 */
class ParticleSelectorService : public Service {
   public:
    static constexpr std::string_view service_name() { return "ParticleSelectorService"; }

    explicit ParticleSelectorService(std::string name = "ParticleSelectorService")
        : Service(std::move(name)) {}

    auto finalize() -> StatusCode override { return StatusCode::SUCCESS(); }

    // =========================================================================
    // Particle extraction (one bank read, indexed by pid and charge)
    // =========================================================================

    /**
     * @brief Read REC::Particle once and build pid- and charge-indexed views.
     *
     * @param rec_particle Bound REC_Particle bankdef wrapper.
     * @param types        Optional PID whitelist. Empty = keep every row.
     * @return EventParticles view: `by_pid`, `by_charge`, and convenience
     *         accessors are served from the single pass done here.
     */
    [[nodiscard]] auto extract_particles(
        const clas12::bankdefs::REC_Particle& rec_particle,
        const std::vector<clas12::PIDs>& types = {}
    ) const -> EventParticles {
        EventParticles result;
        if (!rec_particle.valid()) return result;

        const int n_rows = rec_particle.rows();
        result.m_all.reserve(n_rows);

        for (int i = 0; i < n_rows; ++i) {
            const int pid = rec_particle.pid(i);

            if (!types.empty()) {
                bool keep = false;
                for (auto t : types) {
                    if (pid == static_cast<int>(t)) { keep = true; break; }
                }
                if (!keep) continue;
            }

            const int status    = rec_particle.status(i);
            const int charge    = rec_particle.charge(i);
            const double px     = rec_particle.px(i);
            const double py     = rec_particle.py(i);
            const double pz     = rec_particle.pz(i);
            const double vx     = rec_particle.vx(i);
            const double vy     = rec_particle.vy(i);
            const double vz     = rec_particle.vz(i);
            const double vt     = rec_particle.vt(i);
            const double beta   = rec_particle.beta(i);
            const double chi2pid = rec_particle.chi2pid(i);

            const double mass = clas12::physics::ParticleUtils::get_mass(pid);
            const double E    = clas12::physics::ParticleUtils::compute_energy(px, py, pz, pid);

            result.m_all.emplace_back(
                pid, status, i, charge, mass,
                px, py, pz, E, vx, vy, vz, vt, beta, chi2pid);

            const auto& p = result.m_all.back();
            result.m_by_pid[pid].push_back(p);
            result.m_by_charge[charge].push_back(p);
        }

        return result;
    }

    // =========================================================================
    // Stateless filters (operate on already-extracted vectors, zero bank reads)
    // =========================================================================

    auto filter(const std::vector<clas12::Particle>& particles,
                std::function<bool(const clas12::Particle&)> predicate) const
        -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> filtered;
        filtered.reserve(particles.size());
        for (const auto& p : particles) {
            if (predicate(p)) filtered.push_back(p);
        }
        return filtered;
    }

    auto with_momentum_range(const std::vector<clas12::Particle>& particles,
                             double min_p, double max_p) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_p, max_p](const clas12::Particle& p) {
            const double m = p.p();
            return m >= min_p && m <= max_p;
        });
    }

    auto with_charge(const std::vector<clas12::Particle>& particles,
                     int charge) const -> std::vector<clas12::Particle> {
        return filter(particles, [charge](const clas12::Particle& p) {
            return p.charge() == charge;
        });
    }

    auto with_chi2pid_range(const std::vector<clas12::Particle>& particles,
                            double min_chi2, double max_chi2) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_chi2, max_chi2](const clas12::Particle& p) {
            const double c = p.chi2pid();
            return c >= min_chi2 && c <= max_chi2;
        });
    }

    auto with_vz_range(const std::vector<clas12::Particle>& particles,
                       double min_vz, double max_vz) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_vz, max_vz](const clas12::Particle& p) {
            const double v = p.vz();
            return v >= min_vz && v <= max_vz;
        });
    }

    auto with_theta_range(const std::vector<clas12::Particle>& particles,
                          double min_theta, double max_theta) const -> std::vector<clas12::Particle> {
        return filter(particles, [min_theta, max_theta](const clas12::Particle& p) {
            const double t = p.theta();
            return t >= min_theta && t <= max_theta;
        });
    }

    auto get_trigger_particles(const std::vector<clas12::Particle>& particles,
                               std::pair<double, double> status_range = {2000, 4000}) const
        -> std::vector<clas12::Particle> {
        return filter(particles, [status_range](const clas12::Particle& p) {
            const double abs_status = std::abs(p.status());
            return p.status() < 0.0 && p.p() != 0.0 &&
                   status_range.first <= abs_status && abs_status < status_range.second;
        });
    }

    auto get_forward_detector(const std::vector<clas12::Particle>& particles) const
        -> std::vector<clas12::Particle> {
        return get_trigger_particles(particles, {2000, 4000});
    }

    auto get_central_detector(const std::vector<clas12::Particle>& particles) const
        -> std::vector<clas12::Particle> {
        return filter(particles, [](const clas12::Particle& p) {
            const double abs_status = std::abs(p.status());
            return abs_status >= 4000 && abs_status < 6000;
        });
    }
};

}  // namespace clas12::framework
