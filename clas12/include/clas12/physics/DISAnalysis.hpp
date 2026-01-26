#pragma once

#include <optional>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/physics/Kinematics.hpp>

namespace clas12::physics {

/**
 * @brief Beam configuration for DIS analysis
 *
 * Encapsulates beam parameters for kinematic calculations.
 */
struct BeamConfig {
    double energy = 10.53;  // GeV (default CLAS12 RG-A)
    double px = 0.0;
    double py = 0.0;
    double pz = 10.53;

    /**
     * @brief Create a beam particle from this configuration
     */
    auto to_particle() const -> clas12::Particle {
        double E = std::sqrt(px * px + py * py + pz * pz + clas12::Constants::ELECTRONMASS * clas12::Constants::ELECTRONMASS);
        return clas12::Particle(11, 0, -1, -1, clas12::Constants::ELECTRONMASS,
                              px, py, pz, E, 0, 0, 0, 0, 1.0, 0);
    }
};

/**
 * @brief Higher-level DIS analysis tools
 *
 * Provides convenient methods for common DIS analysis tasks including
 * kinematic region selection, Q2-W binning, and DIS event classification.
 */
class DISAnalysis {
   public:
    /**
     * @brief DIS kinematic region definition
     */
    struct KinematicRegion {
        double q2_min = 1.0;   // GeV²
        double q2_max = 11.0;  // GeV²
        double w_min = 2.0;    // GeV
        double w_max = 5.0;    // GeV
        double y_min = 0.0;    // dimensionless
        double y_max = 0.85;   // dimensionless
        double xb_min = 0.0;   // dimensionless
        double xb_max = 1.0;   // dimensionless

        /**
         * @brief Check if kinematics are in this region
         */
        auto contains(double q2, double w, double y, double xb) const -> bool {
            return q2 >= q2_min && q2 <= q2_max &&
                   w >= w_min && w <= w_max &&
                   y >= y_min && y <= y_max &&
                   xb >= xb_min && xb <= xb_max;
        }
    };

    /**
     * @brief Full phase space region (No cuts)
     */
    static auto full_phase_space() -> KinematicRegion {
        return KinematicRegion{
            .q2_min = 0.0,
            .q2_max = 100.0,
            .w_min = 0.0,
            .w_max = 50.0,
            .y_min = 0.0,
            .y_max = 10.0};
    }

    /**
     * @brief Standard DIS region (Q² > 1 GeV², W > 2 GeV)
     */
    static auto standard_dis_region() -> KinematicRegion {
        return KinematicRegion{
            .q2_min = 1.0,
            .q2_max = 20.0,
            .w_min = 2.0,
            .w_max = 25.0,
            .y_min = 0.0,
            .y_max = 0.85};
    }

    /**
     * @brief Resonance region (W < 2 GeV)
     */
    static auto resonance_region() -> KinematicRegion {
        return KinematicRegion{
            .q2_min = 0.5,
            .q2_max = 5.0,
            .w_min = 1.08,  // Above pion threshold
            .w_max = 2.0,
            .y_min = 0.0,
            .y_max = 0.85};
    }

    /**
     * @brief High-Q² region for precision studies
     */
    static auto high_q2_region() -> KinematicRegion {
        return KinematicRegion{
            .q2_min = 4.0,
            .q2_max = 11.0,
            .w_min = 2.0,
            .w_max = 5.0,
            .y_min = 0.0,
            .y_max = 0.85};
    }

    /**
     * @brief Calculate all DIS kinematics for an electron
     */
    struct DISKinematics {
        double q2;       // GeV²
        double w;        // GeV
        double nu;       // GeV
        double xb;       // Bjorken x
        double y;        // Inelasticity
        double epsilon;  // Virtual photon polarization
        double gamma;    // Boost factor

        /**
         * @brief Check if in standard DIS region
         */
        auto is_dis() const -> bool {
            return q2 > 1.0 && w > 2.0 && y < 0.85;
        }

        /**
         * @brief Check if in resonance region
         */
        auto is_resonance() const -> bool {
            return w < 2.0 && w > 1.08;
        }

        /**
         * @brief Check if in specific region
         */
        auto in_region(const KinematicRegion& region) const -> bool {
            return region.contains(q2, w, y, xb);
        }
    };

    /**
     * @brief Calculate all DIS kinematics at once
     *
     * @param electron Scattered electron
     * @param beam Beam configuration (default: 10.53 GeV along z)
     * @return DISKinematics structure with all calculated values
     */
    static auto calculate_kinematics(
        const clas12::Particle& electron,
        const BeamConfig& beam = BeamConfig()) -> DISKinematics {
        // Create target proton at rest
        clas12::Particle target(2212, 0, -1, 1, clas12::Constants::PROTONMASS,
                              0, 0, 0, clas12::Constants::PROTONMASS, 0, 0, 0, 0, 0, 0);

        // Create beam particle
        clas12::Particle beam_particle = beam.to_particle();

        // Use Physics::KinematicCalculator for calculations
        double q2_val = KinematicCalculator::calculate_Q2(beam_particle, electron);
        double w_val = KinematicCalculator::calculate_W(beam_particle, electron, target);
        double nu_val = beam.energy - electron.E();
        double y_val = KinematicCalculator::calculate_y(beam_particle, electron);
        double xb_val = KinematicCalculator::calculate_x_bjorken(q2_val, nu_val, clas12::Constants::PROTONMASS);

        // Virtual photon polarization parameter
        double epsilon_val = calculate_epsilon(q2_val, nu_val, beam.energy);

        // Boost factor (γ = Q/M)
        double gamma_val = std::sqrt(q2_val) / clas12::Constants::PROTONMASS;

        return DISKinematics{
            .q2 = q2_val,
            .w = w_val,
            .nu = nu_val,
            .xb = xb_val,
            .y = y_val,
            .epsilon = epsilon_val,
            .gamma = gamma_val};
    }

    /**
     * @brief Q²-W binning scheme for 2D histograms
     */
    struct Q2WBinning {
        std::vector<double> q2_edges;
        std::vector<double> w_edges;

        /**
         * @brief Standard RG-A binning
         */
        static auto standard_binning() -> Q2WBinning {
            return Q2WBinning{
                .q2_edges = {1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 7.0, 11.0},
                .w_edges = {2.0, 2.2, 2.4, 2.6, 2.8, 3.0, 3.5, 4.0, 5.0}};
        }

        /**
         * @brief Fine binning for detailed studies
         */
        static auto fine_binning() -> Q2WBinning {
            Q2WBinning bins;
            // Q²: 1.0 to 11.0 in 0.5 GeV² steps
            for (double q2 = 1.0; q2 <= 11.0; q2 += 0.5) {
                bins.q2_edges.push_back(q2);
            }
            // W: 2.0 to 5.0 in 0.2 GeV steps
            for (double w = 2.0; w <= 5.0; w += 0.2) {
                bins.w_edges.push_back(w);
            }
            return bins;
        }

        /**
         * @brief Get bin indices for Q² and W
         */
        auto get_bin(double q2, double w) const -> std::optional<std::pair<int, int>> {
            // Find Q² bin
            auto q2_it = std::upper_bound(q2_edges.begin(), q2_edges.end(), q2);
            if (q2_it == q2_edges.begin() || q2_it == q2_edges.end()) {
                return std::nullopt;
            }
            int q2_bin = std::distance(q2_edges.begin(), q2_it) - 1;

            // Find W bin
            auto w_it = std::upper_bound(w_edges.begin(), w_edges.end(), w);
            if (w_it == w_edges.begin() || w_it == w_edges.end()) {
                return std::nullopt;
            }
            int w_bin = std::distance(w_edges.begin(), w_it) - 1;

            return std::make_pair(q2_bin, w_bin);
        }
    };

    /**
     * @brief DIS event classification
     */
    enum class EventType {
        Elastic,       // W ~ M_p, Q² > 0
        QuasiElastic,  // W < 1.5 GeV
        Resonance,     // 1.08 < W < 2.0 GeV
        DIS,           // W > 2.0 GeV, Q² > 1.0 GeV²
        HighQ2DIS,     // W > 2.0 GeV, Q² > 4.0 GeV²
        LowQ2,         // Q² < 1.0 GeV²
        HighY,         // y > 0.85 (radiative tail)
        Unknown
    };

    /**
     * @brief Classify DIS event based on kinematics
     */
    static auto classify_event(const DISKinematics& kin) -> EventType {
        // Check for radiative tail
        if (kin.y > 0.85) return EventType::HighY;

        // Check W regions
        if (kin.w < 1.08) return EventType::Elastic;
        if (kin.w < 1.5) return EventType::QuasiElastic;
        if (kin.w < 2.0) return EventType::Resonance;

        // DIS region
        if (kin.q2 < 1.0) return EventType::LowQ2;
        if (kin.q2 > 4.0) return EventType::HighQ2DIS;
        return EventType::DIS;
    }

    /**
     * @brief Get string name for event type
     */
    static auto event_type_name(EventType type) -> std::string {
        switch (type) {
            case EventType::Elastic:
                return "Elastic";
            case EventType::QuasiElastic:
                return "QuasiElastic";
            case EventType::Resonance:
                return "Resonance";
            case EventType::DIS:
                return "DIS";
            case EventType::HighQ2DIS:
                return "HighQ2DIS";
            case EventType::LowQ2:
                return "LowQ2";
            case EventType::HighY:
                return "HighY";
            default:
                return "Unknown";
        }
    }

   private:
    /**
     * @brief Calculate virtual photon polarization parameter
     * ε = [1 + 2(1 + ν²/Q²)tan²(θ_e/2)]^(-1)
     */
    static auto calculate_epsilon(double q2, double nu, double beam_energy) -> double {
        double tan_half_theta_sq = q2 / (4.0 * beam_energy * (beam_energy - nu));
        return 1.0 / (1.0 + 2.0 * (1.0 + nu * nu / q2) * tan_half_theta_sq);
    }
};

/**
 * @brief Helper for structure function extraction
 */
class StructureFunctionHelper {
   public:
    /**
     * @brief Calculate F2 structure function from cross section
     * Simplified formula for quick estimates
     */
    static auto estimate_F2(double sigma_r, double /*xb*/) -> double {
        // σ_r ≈ F2 - y²/(1+(1-y)²) FL
        // Approximate: F2 ≈ σ_r (neglecting FL contribution)
        return sigma_r;
    }

    /**
     * @brief Calculate reduced cross section
     * σ_r = F2 - y²/(1+(1-y)²) FL
     */
    static auto reduced_cross_section(
        double F2,
        double FL,
        double y) -> double {
        double y_factor = y * y / (1.0 + (1.0 - y) * (1.0 - y));
        return F2 - y_factor * FL;
    }

    /**
     * @brief R = σ_L/σ_T ratio estimation (SLAC parameterization)
     */
    static auto estimate_R(double q2, double xb) -> double {
        // SLAC R1998 parameterization
        double theta = 1.0 + 12.0 * q2 / (1.0 + q2) * 0.125 * 0.125 / (0.125 * 0.125 + xb * xb);
        double R = 0.0635 / std::log(q2 / 0.04) * theta;
        return std::max(0.0, R);
    }
};

}  // namespace clas12::physics
