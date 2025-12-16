#pragma once

#include <memory>
#include <vector>
#include "CLAS/PIDs.hpp"
#include "CLAS/Particle.hpp"

namespace Physics {

/**
 * @brief Composite particle created from decay products
 *
 * Represents a particle reconstructed from its decay products,
 * such as π⁰ → γγ, ρ → π⁺π⁻, etc.
 */
class CompositeParticle {
   public:
    /**
     * @brief Construct from daughter particles
     * @param daughters Vector of daughter particles
     * @param pid PDG ID of the composite particle
     */
    CompositeParticle(std::vector<CLAS::Particle> daughters, int pid = 0)
        : m_daughters(std::move(daughters)), m_pid(pid) {
        calculate_four_momentum();
    }

    /**
     * @brief Get invariant mass
     */
    auto mass() const -> double {
        return m_mass;
    }

    /**
     * @brief Get four-momentum components
     */
    auto px() const -> double { return m_px; }

    auto py() const -> double { return m_py; }

    auto pz() const -> double { return m_pz; }

    auto energy() const -> double { return m_energy; }

    /**
     * @brief Get total momentum
     */
    auto p() const -> double {
        return std::sqrt(m_px * m_px + m_py * m_py + m_pz * m_pz);
    }

    /**
     * @brief Get transverse momentum
     */
    auto pt() const -> double {
        return std::sqrt(m_px * m_px + m_py * m_py);
    }

    /**
     * @brief Get polar angle (degrees)
     */
    auto theta() const -> double {
        return std::atan2(pt(), m_pz) * 180.0 / M_PI;
    }

    /**
     * @brief Get azimuthal angle (degrees)
     */
    auto phi() const -> double {
        return std::atan2(m_py, m_px) * 180.0 / M_PI;
    }

    /**
     * @brief Get rapidity
     */
    auto rapidity() const -> double {
        return 0.5 * std::log((m_energy + m_pz) / (m_energy - m_pz));
    }

    /**
     * @brief Get daughter particles
     */
    auto daughters() const -> const std::vector<CLAS::Particle>& {
        return m_daughters;
    }

    /**
     * @brief Get number of daughters
     */
    auto n_daughters() const -> size_t {
        return m_daughters.size();
    }

    /**
     * @brief Get PDG ID
     */
    auto pid() const -> int {
        return m_pid;
    }

    /**
     * @brief Check if mass is within window
     */
    auto in_mass_window(double center, double width) const -> bool {
        return std::abs(m_mass - center) < width;
    }

   private:
    std::vector<CLAS::Particle> m_daughters;
    int m_pid;
    double m_px = 0.0;
    double m_py = 0.0;
    double m_pz = 0.0;
    double m_energy = 0.0;
    double m_mass = 0.0;

    auto calculate_four_momentum() -> void {
        m_px = 0.0;
        m_py = 0.0;
        m_pz = 0.0;
        m_energy = 0.0;

        for (const auto& daughter : m_daughters) {
            m_px += daughter.px();
            m_py += daughter.py();
            m_pz += daughter.pz();

            // Calculate energy from momentum and mass
            double p_mag = daughter.p();
            double mass = daughter.mass();
            double E = std::sqrt(p_mag * p_mag + mass * mass);
            m_energy += E;
        }

        // Calculate invariant mass
        double p_squared = m_px * m_px + m_py * m_py + m_pz * m_pz;
        double m_squared = m_energy * m_energy - p_squared;
        m_mass = (m_squared > 0) ? std::sqrt(m_squared) : 0.0;
    }
};

/**
 * @brief Builder for composite particles
 */
class CompositeParticleBuilder {
   public:
    /**
     * @brief Reconstruct π⁰ from two photons
     * @param photons Vector of photons
     * @return Vector of π⁰ candidates
     */
    static auto reconstruct_pi0(const std::vector<CLAS::Particle>& photons)
        -> std::vector<CompositeParticle> {
        std::vector<CompositeParticle> pi0_candidates;

        // Combine all photon pairs
        for (size_t i = 0; i < photons.size(); ++i) {
            for (size_t j = i + 1; j < photons.size(); ++j) {
                std::vector<CLAS::Particle> daughters = {photons[i], photons[j]};
                pi0_candidates.emplace_back(daughters, 111);  // PDG ID for π⁰
            }
        }

        return pi0_candidates;
    }

    /**
     * @brief Reconstruct ρ from π⁺π⁻
     * @param pions_plus Vector of π⁺
     * @param pions_minus Vector of π⁻
     * @return Vector of ρ candidates
     */
    static auto reconstruct_rho(const std::vector<CLAS::Particle>& pions_plus,
                                const std::vector<CLAS::Particle>& pions_minus)
        -> std::vector<CompositeParticle> {
        std::vector<CompositeParticle> rho_candidates;

        for (const auto& pip : pions_plus) {
            for (const auto& pim : pions_minus) {
                std::vector<CLAS::Particle> daughters = {pip, pim};
                rho_candidates.emplace_back(daughters, 113);  // PDG ID for ρ⁰
            }
        }

        return rho_candidates;
    }

    /**
     * @brief Reconstruct J/ψ from e⁺e⁻
     * @param electrons Vector of electrons
     * @param positrons Vector of positrons
     * @return Vector of J/ψ candidates
     */
    static auto reconstruct_jpsi(const std::vector<CLAS::Particle>& electrons,
                                 const std::vector<CLAS::Particle>& positrons)
        -> std::vector<CompositeParticle> {
        std::vector<CompositeParticle> jpsi_candidates;

        for (const auto& e_minus : electrons) {
            for (const auto& e_plus : positrons) {
                std::vector<CLAS::Particle> daughters = {e_minus, e_plus};
                jpsi_candidates.emplace_back(daughters, 443);  // PDG ID for J/ψ
            }
        }

        return jpsi_candidates;
    }

    /**
     * @brief Reconstruct generic two-body decay
     * @param particles1 First particle type
     * @param particles2 Second particle type
     * @param pid PDG ID of composite
     * @return Vector of candidates
     */
    static auto reconstruct_two_body(const std::vector<CLAS::Particle>& particles1,
                                     const std::vector<CLAS::Particle>& particles2,
                                     int pid = 0)
        -> std::vector<CompositeParticle> {
        std::vector<CompositeParticle> candidates;

        for (const auto& p1 : particles1) {
            for (const auto& p2 : particles2) {
                std::vector<CLAS::Particle> daughters = {p1, p2};
                candidates.emplace_back(daughters, pid);
            }
        }

        return candidates;
    }
};

}  // namespace Physics
