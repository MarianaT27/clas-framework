#pragma once

#include "Core/Particle.hpp"
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace Physics {

/**
 * @brief General physics helper utilities
 */
class PhysicsHelpers {
public:
    /**
     * @brief Particle pair combination generator
     * Generates all unique pairs of particles (without replacement)
     */
    template<typename ParticleType>
    static auto generate_pairs(const std::vector<ParticleType>& particles)
        -> std::vector<std::pair<ParticleType, ParticleType>> {
        std::vector<std::pair<ParticleType, ParticleType>> pairs;

        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                pairs.emplace_back(particles[i], particles[j]);
            }
        }

        return pairs;
    }

    /**
     * @brief Find best pair based on criterion
     * Example: Find pair with invariant mass closest to target
     */
    template<typename ParticleType, typename Criterion>
    static auto find_best_pair(
        const std::vector<ParticleType>& particles,
        Criterion criterion
    ) -> std::optional<std::pair<ParticleType, ParticleType>> {
        auto pairs = generate_pairs(particles);
        if (pairs.empty()) return std::nullopt;

        auto best = std::min_element(
            pairs.begin(), pairs.end(),
            [&criterion](const auto& a, const auto& b) {
                return criterion(a) < criterion(b);
            }
        );

        return *best;
    }

    /**
     * @brief Rapidity calculation
     * y = 0.5 * ln[(E + p_z)/(E - p_z)]
     */
    static auto rapidity(const Core::Particle& p) -> double {
        double E = p.E();
        double pz = p.pz();
        return 0.5 * std::log((E + pz) / (E - pz));
    }

    /**
     * @brief Pseudorapidity calculation
     * η = -ln[tan(θ/2)]
     */
    static auto pseudorapidity(const Core::Particle& p) -> double {
        double theta_rad = p.theta() * M_PI / 180.0;
        return -std::log(std::tan(theta_rad / 2.0));
    }

    /**
     * @brief Transverse momentum
     */
    static auto pt(const Core::Particle& p) -> double {
        return std::sqrt(p.px() * p.px() + p.py() * p.py());
    }

    /**
     * @brief Transverse mass
     * m_T = sqrt(m² + p_T²)
     */
    static auto transverse_mass(const Core::Particle& p) -> double {
        double pt_val = pt(p);
        double m = p.mass();
        return std::sqrt(m * m + pt_val * pt_val);
    }

    /**
     * @brief Sort particles by momentum
     */
    static auto sort_by_momentum(std::vector<Core::Particle> particles, bool descending = true)
        -> std::vector<Core::Particle> {
        std::sort(particles.begin(), particles.end(),
            [descending](const Core::Particle& a, const Core::Particle& b) {
                return descending ? a.p() > b.p() : a.p() < b.p();
            });
        return particles;
    }

    /**
     * @brief Sort particles by energy
     */
    static auto sort_by_energy(std::vector<Core::Particle> particles, bool descending = true)
        -> std::vector<Core::Particle> {
        std::sort(particles.begin(), particles.end(),
            [descending](const Core::Particle& a, const Core::Particle& b) {
                return descending ? a.E() > b.E() : a.E() < b.E();
            });
        return particles;
    }

    /**
     * @brief Calculate total 4-momentum of particle collection
     */
    struct FourMomentum {
        double px, py, pz, E;

        auto invariant_mass() const -> double {
            return std::sqrt(E * E - px * px - py * py - pz * pz);
        }

        auto momentum() const -> double {
            return std::sqrt(px * px + py * py + pz * pz);
        }
    };

    static auto total_four_momentum(const std::vector<Core::Particle>& particles) -> FourMomentum {
        FourMomentum total{0, 0, 0, 0};

        for (const auto& p : particles) {
            total.px += p.px();
            total.py += p.py();
            total.pz += p.pz();
            total.E += p.E();
        }

        return total;
    }

    /**
     * @brief Boost particle to center-of-mass frame
     * Simplified boost along z-axis
     */
    static auto boost_to_cm(
        const Core::Particle& p,
        double beta_z
    ) -> FourMomentum {
        double gamma = 1.0 / std::sqrt(1.0 - beta_z * beta_z);

        double E_cm = gamma * (p.E() - beta_z * p.pz());
        double pz_cm = gamma * (p.pz() - beta_z * p.E());

        return FourMomentum{
            .px = p.px(),
            .py = p.py(),
            .pz = pz_cm,
            .E = E_cm
        };
    }

    /**
     * @brief Calculate Lorentz invariant phase space factor
     * For two-body decay: Φ = p_cm / (8πM²)
     */
    static auto phase_space_two_body(
        double M,           // Parent mass
        double m1,          // Daughter 1 mass
        double m2           // Daughter 2 mass
    ) -> double {
        // CM momentum
        double E_cm = M / 2.0;
        double p_cm = std::sqrt(E_cm * E_cm - m1 * m1);

        return p_cm / (8.0 * M_PI * M * M);
    }

    /**
     * @brief Check if decay is kinematically allowed
     */
    static auto is_decay_allowed(
        double M_parent,
        const std::vector<double>& daughter_masses
    ) -> bool {
        double sum_masses = 0.0;
        for (double m : daughter_masses) {
            sum_masses += m;
        }
        return M_parent > sum_masses;
    }

    /**
     * @brief Calculate decay angle in rest frame
     * For A → B + C, calculate angle of B in A rest frame
     */
    static auto decay_angle_cm(
        const Core::Particle& parent,
        const Core::Particle& daughter
    ) -> double {
        // Get parent momentum direction
        double px_p = parent.px();
        double py_p = parent.py();
        double pz_p = parent.pz();
        double p_p = parent.p();

        // Get daughter momentum
        double px_d = daughter.px();
        double py_d = daughter.py();
        double pz_d = daughter.pz();

        // Dot product
        double dot = (px_p * px_d + py_p * py_d + pz_p * pz_d);

        // Angle in degrees
        double cos_theta = dot / (p_p * daughter.p());
        return std::acos(cos_theta) * 180.0 / M_PI;
    }

    /**
     * @brief Particle multiplicity counter
     */
    struct Multiplicity {
        int charged = 0;
        int neutral = 0;
        int positive = 0;
        int negative = 0;

        auto total() const -> int { return charged + neutral; }

        static auto calculate(const std::vector<Core::Particle>& particles) -> Multiplicity {
            Multiplicity mult;

            for (const auto& p : particles) {
                if (p.charge() == 0) {
                    mult.neutral++;
                } else {
                    mult.charged++;
                    if (p.charge() > 0) mult.positive++;
                    else mult.negative++;
                }
            }

            return mult;
        }
    };

    /**
     * @brief Energy-momentum conservation check
     * Returns sum of relative deviations
     */
    static auto check_conservation(
        const FourMomentum& initial,
        const FourMomentum& final
    ) -> double {
        double dE = std::abs(initial.E - final.E) / initial.E;
        double dpx = std::abs(initial.px - final.px) / (std::abs(initial.px) + 1e-9);
        double dpy = std::abs(initial.py - final.py) / (std::abs(initial.py) + 1e-9);
        double dpz = std::abs(initial.pz - final.pz) / (std::abs(initial.pz) + 1e-9);

        return dE + dpx + dpy + dpz;
    }

    /**
     * @brief Sphericity tensor calculation for event shape
     * Sphericity: S = 3/2 (λ2 + λ3) where λi are eigenvalues
     */
    static auto calculate_sphericity(const std::vector<Core::Particle>& particles) -> double {
        // Build momentum tensor
        double S_xx = 0, S_yy = 0, S_zz = 0;
        double S_xy = 0, S_xz = 0, S_yz = 0;
        double p_tot = 0;

        for (const auto& p : particles) {
            double px = p.px();
            double py = p.py();
            double pz = p.pz();
            double p_mag = p.p();

            S_xx += px * px;
            S_yy += py * py;
            S_zz += pz * pz;
            S_xy += px * py;
            S_xz += px * pz;
            S_yz += py * pz;
            p_tot += p_mag * p_mag;
        }

        // Normalize
        S_xx /= p_tot;
        S_yy /= p_tot;
        S_zz /= p_tot;
        S_xy /= p_tot;
        S_xz /= p_tot;
        S_yz /= p_tot;

        // Simplified: return trace/3 as approximation
        // Full calculation requires eigenvalue solver
        return (S_xx + S_yy + S_zz) / 3.0;
    }

    /**
     * @brief Thrust calculation for event shape
     * Thrust: max Σ|p·n| / Σ|p|
     */
    static auto calculate_thrust(const std::vector<Core::Particle>& particles) -> double {
        // Simplified calculation along z-axis
        // Full calculation requires optimization over all directions
        double thrust_z = 0;
        double p_tot = 0;

        for (const auto& p : particles) {
            thrust_z += std::abs(p.pz());
            p_tot += p.p();
        }

        return thrust_z / p_tot;
    }
};

/**
 * @brief Statistical helper functions
 */
class StatisticsHelper {
public:
    /**
     * @brief Calculate mean of a vector
     */
    template<typename T>
    static auto mean(const std::vector<T>& values) -> double {
        if (values.empty()) return 0.0;
        double sum = 0.0;
        for (const auto& v : values) sum += v;
        return sum / values.size();
    }

    /**
     * @brief Calculate standard deviation
     */
    template<typename T>
    static auto stddev(const std::vector<T>& values) -> double {
        if (values.size() < 2) return 0.0;

        double m = mean(values);
        double sum_sq = 0.0;
        for (const auto& v : values) {
            double diff = v - m;
            sum_sq += diff * diff;
        }

        return std::sqrt(sum_sq / (values.size() - 1));
    }

    /**
     * @brief Calculate median
     */
    template<typename T>
    static auto median(std::vector<T> values) -> double {
        if (values.empty()) return 0.0;

        std::sort(values.begin(), values.end());
        size_t n = values.size();

        if (n % 2 == 0) {
            return (values[n / 2 - 1] + values[n / 2]) / 2.0;
        } else {
            return values[n / 2];
        }
    }

    /**
     * @brief Calculate weighted mean
     */
    template<typename T>
    static auto weighted_mean(
        const std::vector<T>& values,
        const std::vector<double>& weights
    ) -> double {
        if (values.size() != weights.size() || values.empty()) return 0.0;

        double sum = 0.0;
        double weight_sum = 0.0;

        for (size_t i = 0; i < values.size(); ++i) {
            sum += values[i] * weights[i];
            weight_sum += weights[i];
        }

        return weight_sum > 0 ? sum / weight_sum : 0.0;
    }

    /**
     * @brief Calculate Pearson correlation coefficient
     */
    template<typename T>
    static auto correlation(
        const std::vector<T>& x,
        const std::vector<T>& y
    ) -> double {
        if (x.size() != y.size() || x.size() < 2) return 0.0;

        double mean_x = mean(x);
        double mean_y = mean(y);

        double sum_xy = 0.0;
        double sum_xx = 0.0;
        double sum_yy = 0.0;

        for (size_t i = 0; i < x.size(); ++i) {
            double dx = x[i] - mean_x;
            double dy = y[i] - mean_y;
            sum_xy += dx * dy;
            sum_xx += dx * dx;
            sum_yy += dy * dy;
        }

        double denom = std::sqrt(sum_xx * sum_yy);
        return denom > 0 ? sum_xy / denom : 0.0;
    }
};

}  // namespace Physics
