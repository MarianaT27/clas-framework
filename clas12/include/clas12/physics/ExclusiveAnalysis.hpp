#pragma once

#include <cmath>
#include <optional>
#include <vector>
#include <clas12/Particle.hpp>
#include <clas12/physics/Kinematics.hpp>

namespace physics {

/**
 * @brief Higher-level tools for exclusive reaction analysis
 *
 * Provides utilities for:
 * - Exclusivity checks (missing mass, energy, momentum)
 * - Topology classification
 * - Meson/baryon reconstruction
 * - Phase space cuts
 */
class ExclusiveAnalysis {
   public:
    /**
     * @brief Exclusivity criteria for exclusive reactions
     */
    struct ExclusivityCuts {
        double max_missing_energy = 0.5;    // GeV
        double max_missing_momentum = 0.3;  // GeV/c
        double missing_mass_center = 0.0;   // GeV (0 for exclusive)
        double missing_mass_width = 0.5;    // GeV

        /**
         * @brief Standard exclusive cuts (tight)
         */
        static auto tight() -> ExclusivityCuts {
            return ExclusivityCuts{
                .max_missing_energy = 0.3,
                .max_missing_momentum = 0.2,
                .missing_mass_center = 0.0,
                .missing_mass_width = 0.3};
        }

        /**
         * @brief Loose exclusive cuts
         */
        static auto loose() -> ExclusivityCuts {
            return ExclusivityCuts{
                .max_missing_energy = 0.8,
                .max_missing_momentum = 0.5,
                .missing_mass_center = 0.0,
                .missing_mass_width = 0.8};
        }

        /**
         * @brief Missing neutron cuts (e.g., ep → e'pπ⁺(n))
         */
        static auto missing_neutron() -> ExclusivityCuts {
            return ExclusivityCuts{
                .max_missing_energy = 0.5,
                .max_missing_momentum = 0.4,
                .missing_mass_center = 0.940,  // Neutron mass
                .missing_mass_width = 0.2};
        }
    };

    /**
     * @brief Result of exclusivity check
     */
    struct ExclusivityResult {
        double missing_energy;    // GeV
        double missing_momentum;  // GeV/c
        double missing_mass;      // GeV
        bool passes_energy;
        bool passes_momentum;
        bool passes_mass;

        /**
         * @brief Check if all exclusivity cuts pass
         */
        auto is_exclusive() const -> bool {
            return passes_energy && passes_momentum && passes_mass;
        }

        /**
         * @brief Calculate exclusivity confidence (0-1)
         */
        auto exclusivity_confidence() const -> double {
            // Simple metric: average of individual cut "goodness"
            return (passes_energy + passes_momentum + passes_mass) / 3.0;
        }
    };

    /**
     * @brief Check exclusivity for a reaction
     *
     * @param electron Scattered electron
     * @param detected_particles Final state hadrons that were detected
     * @param cuts Exclusivity criteria
     * @param beam Beam configuration
     * @return Exclusivity check results
     */
    static auto check_exclusivity(
        const Particle& electron,
        const std::vector<Particle>& detected_particles,
        const ExclusivityCuts& cuts = ExclusivityCuts(),
        const Kinematics::BeamConfig& beam = Kinematics::BeamConfig()) -> ExclusivityResult {
        using namespace Kinematics;

        double E_miss = missing_energy(electron, detected_particles, beam);
        double p_miss = missing_momentum(electron, detected_particles, beam);
        double mm = missing_mass(electron, detected_particles, beam);

        return ExclusivityResult{
            .missing_energy = E_miss,
            .missing_momentum = p_miss,
            .missing_mass = mm,
            .passes_energy = std::abs(E_miss) < cuts.max_missing_energy,
            .passes_momentum = p_miss < cuts.max_missing_momentum,
            .passes_mass = std::abs(mm - cuts.missing_mass_center) < cuts.missing_mass_width};
    }

    /**
     * @brief Event topology (particle multiplicities)
     */
    struct Topology {
        int n_electrons = 0;
        int n_protons = 0;
        int n_pi_plus = 0;
        int n_pi_minus = 0;
        int n_pi_zero = 0;
        int n_k_plus = 0;
        int n_k_minus = 0;
        int n_photons = 0;

        /**
         * @brief Get total charge
         */
        auto total_charge() const -> int {
            return -n_electrons + n_protons + n_pi_plus - n_pi_minus + n_k_plus - n_k_minus;
        }

        /**
         * @brief Get total particle count
         */
        auto total_particles() const -> int {
            return n_electrons + n_protons + n_pi_plus + n_pi_minus +
                   n_pi_zero + n_k_plus + n_k_minus + n_photons;
        }

        /**
         * @brief Check if topology matches expected
         */
        auto matches(const Topology& expected) const -> bool {
            return n_electrons == expected.n_electrons &&
                   n_protons == expected.n_protons &&
                   n_pi_plus == expected.n_pi_plus &&
                   n_pi_minus == expected.n_pi_minus &&
                   n_pi_zero == expected.n_pi_zero &&
                   n_k_plus == expected.n_k_plus &&
                   n_k_minus == expected.n_k_minus &&
                   n_photons == expected.n_photons;
        }

        /**
         * @brief Get string representation
         */
        auto to_string() const -> std::string {
            std::string result;
            if (n_electrons > 0) result += std::to_string(n_electrons) + "e ";
            if (n_protons > 0) result += std::to_string(n_protons) + "p ";
            if (n_pi_plus > 0) result += std::to_string(n_pi_plus) + "π⁺ ";
            if (n_pi_minus > 0) result += std::to_string(n_pi_minus) + "π⁻ ";
            if (n_pi_zero > 0) result += std::to_string(n_pi_zero) + "π⁰ ";
            if (n_k_plus > 0) result += std::to_string(n_k_plus) + "K⁺ ";
            if (n_k_minus > 0) result += std::to_string(n_k_minus) + "K⁻ ";
            if (n_photons > 0) result += std::to_string(n_photons) + "γ ";
            return result.empty() ? "empty" : result;
        }

        /**
         * @brief Common topologies
         */
        static auto ep_to_epPi0() -> Topology {  // ep → e'p'π⁰
            return Topology{.n_electrons = 1, .n_protons = 1, .n_pi_zero = 1};
        }

        static auto ep_to_epPiPlus() -> Topology {  // ep → e'p'π⁺
            return Topology{.n_electrons = 1, .n_protons = 1, .n_pi_plus = 1};
        }

        static auto ep_to_eppPiMinus() -> Topology {  // ep → e'p'p'π⁻
            return Topology{.n_electrons = 1, .n_protons = 2, .n_pi_minus = 1};
        }

        static auto ep_to_epPiPlusPiMinus() -> Topology {  // ep → e'p'π⁺π⁻
            return Topology{.n_electrons = 1, .n_protons = 1, .n_pi_plus = 1, .n_pi_minus = 1};
        }

        static auto ep_to_enPiPlus() -> Topology {  // ep → e'(n)π⁺ (missing neutron)
            return Topology{.n_electrons = 1, .n_pi_plus = 1};
        }
    };

    /**
     * @brief Mandelstam t calculation for meson production
     *
     * For ep → e'p'X: t = (q - p_X)²
     */
    static auto calculate_t(
        const Particle& electron,
        const Particle& meson,
        const Kinematics::BeamConfig& beam = Kinematics::BeamConfig()) -> double {
        return Kinematics::momentum_transfer_t(electron, meson, beam);
    }

    /**
     * @brief Check if event is in valid t range for DVCS/exclusive meson production
     */
    static auto is_valid_t_range(double t, double t_min = -2.0, double t_max = 0.0) -> bool {
        return t >= t_min && t <= t_max;
    }

    /**
     * @brief Angle calculations for exclusive reactions
     */
    struct ExclusiveAngles {
        double theta_gamma_cm;  // Meson/photon angle in CM frame
        double phi_h;           // Hadron plane angle
        double phi_gamma;       // Photon/meson plane angle
        double delta_phi;       // Relative azimuthal angle

        /**
         * @brief Calculate angles for DVCS/meson production
         * Simplified calculation - full implementation requires proper Lorentz boosts
         */
        static auto calculate(
            const Particle& electron,
            const Particle& meson_or_photon,
            const Particle& recoil_proton) -> ExclusiveAngles {
            using namespace Kinematics;

            // Calculate relative phi angle
            double dphi = delta_phi(meson_or_photon, recoil_proton);

            // Simplified CM angle calculation
            double theta_cm = angle_between(meson_or_photon, recoil_proton);

            return ExclusiveAngles{
                .theta_gamma_cm = theta_cm,
                .phi_h = recoil_proton.phi(),
                .phi_gamma = meson_or_photon.phi(),
                .delta_phi = dphi};
        }
    };

    /**
     * @brief Phase space boundaries for exclusive reactions
     */
    struct PhaseSpaceCuts {
        double t_min = -2.0;   // GeV²
        double t_max = 0.0;    // GeV²
        double w_min = 2.0;    // GeV
        double w_max = 5.0;    // GeV
        double q2_min = 1.0;   // GeV²
        double q2_max = 10.0;  // GeV²
        double xb_min = 0.0;
        double xb_max = 1.0;

        /**
         * @brief Standard exclusive meson production cuts
         */
        static auto exclusive_meson() -> PhaseSpaceCuts {
            return PhaseSpaceCuts{
                .t_min = -1.5,
                .t_max = -0.1,
                .w_min = 2.0,
                .w_max = 4.0,
                .q2_min = 1.0,
                .q2_max = 10.0};
        }

        /**
         * @brief DVCS phase space
         */
        static auto dvcs() -> PhaseSpaceCuts {
            return PhaseSpaceCuts{
                .t_min = -2.0,
                .t_max = -0.1,
                .w_min = 2.0,
                .w_max = 5.0,
                .q2_min = 1.0,
                .q2_max = 10.0,
                .xb_min = 0.05,
                .xb_max = 0.5};
        }

        /**
         * @brief Check if kinematics are in phase space
         */
        auto in_phase_space(double t, double w, double q2, double xb) const -> bool {
            return t >= t_min && t <= t_max &&
                   w >= w_min && w <= w_max &&
                   q2 >= q2_min && q2 <= q2_max &&
                   xb >= xb_min && xb <= xb_max;
        }
    };

    /**
     * @brief Reconstruct π⁰ from two photons
     */
    static auto reconstruct_pi0(
        const Particle& gamma1,
        const Particle& gamma2) -> std::optional<Particle> {
        using namespace Kinematics;

        double m_pipi = invariant_mass(gamma1, gamma2);

        // Check if mass is consistent with π⁰ (135 MeV ± 20 MeV)
        if (std::abs(m_pipi - 0.135) > 0.020) {
            return std::nullopt;
        }

        // Create π⁰ particle (combine 4-momenta)
        double px = gamma1.px() + gamma2.px();
        double py = gamma1.py() + gamma2.py();
        double pz = gamma1.pz() + gamma2.pz();
        double E = gamma1.E() + gamma2.E();

        // Create particle with π⁰ properties
        // Note: This is simplified - proper implementation would need full Particle constructor
        return std::nullopt;  // Placeholder - implement when needed
    }

    /**
     * @brief Coplanarity check for exclusive reactions
     * Checks if particles are coplanar (φ difference ~ 180°)
     */
    static auto check_coplanarity(
        const Particle& p1,
        const Particle& p2,
        double tolerance = 20.0  // degrees
        ) -> bool {
        double dphi = std::abs(Kinematics::delta_phi(p1, p2));
        return std::abs(dphi - 180.0) < tolerance;
    }

    /**
     * @brief Calculate momentum balance in transverse plane
     * For exclusive reactions, p_T should sum to ~0
     */
    static auto transverse_momentum_balance(
        const std::vector<Particle>& particles) -> double {
        double px_sum = 0.0;
        double py_sum = 0.0;

        for (const auto& p : particles) {
            px_sum += p.px();
            py_sum += p.py();
        }

        return std::sqrt(px_sum * px_sum + py_sum * py_sum);
    }
};

/**
 * @brief Helper for resonance identification
 */
class ResonanceHelper {
   public:
    /**
     * @brief Known resonances
     */
    struct Resonance {
        std::string name;
        double mass;   // GeV
        double width;  // GeV
        int charge;
        int strangeness;

        /**
         * @brief Check if invariant mass is consistent with this resonance
         */
        auto matches(double inv_mass, double n_sigma = 2.0) const -> bool {
            return std::abs(inv_mass - mass) < n_sigma * width;
        }
    };

    // Baryon resonances
    static constexpr Resonance DELTA_1232{.name = "Δ(1232)", .mass = 1.232, .width = 0.120, .charge = 1, .strangeness = 0};
    static constexpr Resonance N_1440{.name = "N(1440)", .mass = 1.440, .width = 0.350, .charge = 0, .strangeness = 0};
    static constexpr Resonance N_1520{.name = "N(1520)", .mass = 1.520, .width = 0.120, .charge = 0, .strangeness = 0};
    static constexpr Resonance N_1535{.name = "N(1535)", .mass = 1.535, .width = 0.150, .charge = 0, .strangeness = 0};

    // Meson resonances
    static constexpr Resonance RHO_770{.name = "ρ(770)", .mass = 0.775, .width = 0.149, .charge = 0, .strangeness = 0};
    static constexpr Resonance OMEGA_782{.name = "ω(782)", .mass = 0.782, .width = 0.008, .charge = 0, .strangeness = 0};
    static constexpr Resonance PHI_1020{.name = "φ(1020)", .mass = 1.020, .width = 0.004, .charge = 0, .strangeness = 0};
    static constexpr Resonance K_STAR_892{.name = "K*(892)", .mass = 0.892, .width = 0.051, .charge = 1, .strangeness = 1};

    /**
     * @brief Identify potential resonance from invariant mass
     */
    static auto identify_resonance(
        double inv_mass,
        int charge = 0,
        double n_sigma = 2.0) -> std::vector<Resonance> {
        std::vector<Resonance> candidates;

        // Check all known resonances
        std::vector<Resonance> resonances = {
            DELTA_1232, N_1440, N_1520, N_1535,
            RHO_770, OMEGA_782, PHI_1020, K_STAR_892};

        for (const auto& res : resonances) {
            if (res.charge == charge && res.matches(inv_mass, n_sigma)) {
                candidates.push_back(res);
            }
        }

        return candidates;
    }
};

}  // namespace physics
