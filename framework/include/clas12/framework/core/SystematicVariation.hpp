#pragma once

#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <clas12/Particle.hpp>

namespace clas12::framework {

/**
 * @brief Types of systematic variations
 */
enum class VariationType {
    EnergyScale,        // Energy scale up/down
    EnergyResolution,   // Energy resolution smearing
    MomentumScale,      // Momentum scale up/down
    AngularResolution,  // Angular resolution
    SamplingFraction,   // SF cut variation
    Efficiency,         // Selection efficiency
    Custom              // User-defined
};

/**
 * @brief Systematic variation descriptor
 */
struct SystematicVariation {
    std::string name;  // e.g., "EnergyScale_Up"
    VariationType type;
    double magnitude;  // Variation magnitude (e.g., +1 sigma)

    SystematicVariation(std::string name, VariationType type, double mag = 1.0)
        : name(std::move(name)), type(type), magnitude(mag) {}
};

/**
 * @brief Manager for systematic uncertainty variations
 *
 * This class manages systematic variations for physics analyses,
 * following the pattern used in ATLAS/CMS frameworks.
 *
 * Usage:
 * @code
 * SystematicManager sys_mgr;
 * sys_mgr.add_variation("EnergyScale_Up", VariationType::EnergyScale, +1.0);
 * sys_mgr.add_variation("EnergyScale_Down", VariationType::EnergyScale, -1.0);
 *
 * // In event loop:
 * for (const auto& sys : sys_mgr.variations()) {
 *     auto varied_electron = sys_mgr.vary(electron, sys);
 *     // Fill histograms for this variation
 * }
 * @endcode
 */
class SystematicManager {
   public:
    /**
     * @brief Add a systematic variation
     */
    auto add_variation(const std::string& name, VariationType type, double magnitude = 1.0) -> void {
        m_variations.emplace_back(name, type, magnitude);
    }

    /**
     * @brief Get all registered variations
     */
    auto variations() const -> const std::vector<SystematicVariation>& {
        return m_variations;
    }

    /**
     * @brief Apply variation to a particle
     * @param particle Original particle
     * @param variation Variation to apply
     * @return Modified particle
     *
     * Note: Creates a new Particle with varied momentum.
     * For now, uses simple scaling. In production, would use proper
     * smearing with random number generator for resolution effects.
     */
    auto vary(const clas12::Particle& particle, const SystematicVariation& variation) const -> clas12::Particle {
        double scale_factor = 1.0;

        switch (variation.type) {
            case VariationType::EnergyScale:
                scale_factor = 1.0 + (0.01 * variation.magnitude);  // 1% per sigma
                break;

            case VariationType::MomentumScale:
                scale_factor = 1.0 + (0.005 * variation.magnitude);  // 0.5% per sigma
                break;

            case VariationType::EnergyResolution:
            case VariationType::AngularResolution:
                // Would need random number generator for smearing
                // For now, return unchanged
                return particle;

            default:
                return particle;
        }

        // Create new particle with scaled momentum
        // Since Particle is immutable, we create a new one
        return clas12::Particle(
            particle.pdg(),
            particle.status(),
            particle.index(),
            particle.charge(),
            particle.mass(),
            particle.px() * scale_factor,  // Scaled px
            particle.py() * scale_factor,  // Scaled py
            particle.pz() * scale_factor,  // Scaled pz
            particle.E() * scale_factor,   // Scaled E (approximately)
            particle.vx(),
            particle.vy(),
            particle.vz(),
            particle.vt(),
            particle.beta(),
            particle.chi2pid());
    }

    /**
     * @brief Register custom variation function
     */
    auto register_custom_variation(const std::string& name,
                                   std::function<clas12::Particle(const clas12::Particle&)> func) -> void {
        m_custom_variations[name] = std::move(func);
    }

    /**
     * @brief Apply custom variation
     */
    auto apply_custom(const clas12::Particle& particle, const std::string& name) const -> clas12::Particle {
        auto it = m_custom_variations.find(name);
        if (it != m_custom_variations.end()) {
            return it->second(particle);
        }
        return particle;
    }

    /**
     * @brief Check if running nominal (no variations)
     */
    auto is_nominal() const -> bool {
        return m_current_variation.empty();
    }

    /**
     * @brief Set current variation being processed
     */
    auto set_current_variation(const std::string& name) -> void {
        m_current_variation = name;
    }

    /**
     * @brief Get current variation name
     */
    auto current_variation() const -> const std::string& {
        return m_current_variation;
    }

   private:
    std::vector<SystematicVariation> m_variations;
    std::map<std::string, std::function<clas12::Particle(const clas12::Particle&)>> m_custom_variations;
    std::string m_current_variation;  // Empty string means nominal
};

}  // namespace clas12::framework
