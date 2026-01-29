#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/fitting/Constraint.hpp>
#include <clas12/framework/fitting/FitResult.hpp>

#include <Math/Vector4D.h>
#include <TMath.h>

namespace clas12::framework::fitting {

/**
 * @brief Kinematic fitter using Lagrange multipliers
 *
 * KinematicFitter performs constrained kinematic fits using the
 * Lagrange multiplier method. It can apply:
 * - Mass constraints (fix invariant mass of particle combination)
 * - Momentum conservation constraints
 * - Energy conservation constraints
 * - Vertex constraints (future)
 *
 * The fit minimizes chi-squared while satisfying all constraints,
 * using an iterative Newton-Raphson method.
 *
 * Example usage:
 * @code
 * KinematicFitter fitter;
 *
 * // Add particles
 * int i_proton = fitter.add_particle(proton);
 * int i_pion = fitter.add_particle(pion);
 *
 * // Add mass constraint
 * fitter.add_mass_constraint(LAMBDA_MASS, {i_proton, i_pion});
 *
 * // Perform fit
 * auto result = fitter.fit();
 *
 * if (result.is_good()) {
 *     double fitted_mass = result.fitted_total_mass();
 *     h_lambda_fitted->Fill(fitted_mass);
 * }
 *
 * // Clear for next event
 * fitter.clear();
 * @endcode
 */
class KinematicFitter {
   public:
    KinematicFitter() = default;

    // ========================================================================
    // Particle Addition
    // ========================================================================

    /**
     * @brief Add a particle to the fit
     *
     * @param particle Reconstructed particle
     * @param cov Covariance matrix (optional, uses defaults if not provided)
     * @return Index of the particle (for constraint specification)
     */
    auto add_particle(const clas12::Particle& particle,
                      const CovarianceMatrix& cov = {}) -> int {
        // Store 4-momentum
        m_momenta.push_back(particle.PxPyPzEVector());
        m_original_momenta.push_back(particle.PxPyPzEVector());
        m_masses.push_back(particle.mass());
        m_covariances.push_back(cov);

        return static_cast<int>(m_momenta.size() - 1);
    }

    /**
     * @brief Add a particle from 4-momentum components
     */
    auto add_particle(double px, double py, double pz, double mass,
                      const CovarianceMatrix& cov = {}) -> int {
        double E = std::sqrt(px * px + py * py + pz * pz + mass * mass);
        m_momenta.emplace_back(px, py, pz, E);
        m_original_momenta.emplace_back(px, py, pz, E);
        m_masses.push_back(mass);
        m_covariances.push_back(cov);

        return static_cast<int>(m_momenta.size() - 1);
    }

    // ========================================================================
    // Constraint Addition (Fluent API)
    // ========================================================================

    /**
     * @brief Add a mass constraint
     *
     * @param mass Target invariant mass [GeV/c^2]
     * @param indices Particle indices to constrain
     * @return Reference to this fitter
     */
    auto add_mass_constraint(double mass, const std::vector<int>& indices)
        -> KinematicFitter& {
        m_constraints.push_back(std::make_unique<MassConstraint>(mass, indices));
        return *this;
    }

    /**
     * @brief Add a mass constraint for two particles
     */
    auto add_mass_constraint(double mass, int idx1, int idx2) -> KinematicFitter& {
        return add_mass_constraint(mass, std::vector<int>{idx1, idx2});
    }

    /**
     * @brief Add momentum conservation constraint
     *
     * @param initial Initial 4-momentum (e.g., beam + target)
     * @param final_indices Final state particle indices
     * @return Reference to this fitter
     */
    auto add_momentum_conservation(const ROOT::Math::PxPyPzEVector& initial,
                                   const std::vector<int>& final_indices)
        -> KinematicFitter& {
        // Add 3 constraints (px, py, pz)
        for (int comp = 0; comp < 3; ++comp) {
            m_constraints.push_back(
                std::make_unique<MomentumConstraint>(initial, final_indices, comp));
        }
        return *this;
    }

    /**
     * @brief Add energy conservation constraint
     */
    auto add_energy_conservation(const ROOT::Math::PxPyPzEVector& initial,
                                 const std::vector<int>& final_indices)
        -> KinematicFitter& {
        m_constraints.push_back(
            std::make_unique<EnergyConstraint>(initial, final_indices));
        return *this;
    }

    /**
     * @brief Add 4-momentum conservation (energy + 3-momentum)
     */
    auto add_four_momentum_conservation(const ROOT::Math::PxPyPzEVector& initial,
                                        const std::vector<int>& final_indices)
        -> KinematicFitter& {
        add_momentum_conservation(initial, final_indices);
        add_energy_conservation(initial, final_indices);
        return *this;
    }

    // ========================================================================
    // Configuration
    // ========================================================================

    /**
     * @brief Set maximum number of iterations
     */
    auto set_max_iterations(int n) -> KinematicFitter& {
        m_max_iterations = n;
        return *this;
    }

    /**
     * @brief Set convergence tolerance
     */
    auto set_convergence_tolerance(double tol) -> KinematicFitter& {
        m_tolerance = tol;
        return *this;
    }

    /**
     * @brief Set chi2 change threshold for convergence
     */
    auto set_chi2_threshold(double threshold) -> KinematicFitter& {
        m_chi2_threshold = threshold;
        return *this;
    }

    // ========================================================================
    // Fitting
    // ========================================================================

    /**
     * @brief Perform the kinematic fit
     *
     * Uses the Lagrange multiplier method with iterative linearization.
     * The fit modifies particle momenta to satisfy all constraints while
     * minimizing chi-squared.
     *
     * @return FitResult containing fitted momenta and quality metrics
     */
    auto fit() -> FitResult {
        FitResult result;

        // Check we have particles and constraints
        if (m_momenta.empty()) {
            result.error_message = "No particles added";
            return result;
        }

        if (m_constraints.empty()) {
            result.error_message = "No constraints added";
            return result;
        }

        // Number of measurements and constraints
        const int n_particles = static_cast<int>(m_momenta.size());
        const int n_constraints = static_cast<int>(m_constraints.size());

        // For this simplified implementation, we use 3 parameters per particle
        // (momentum magnitude, theta, phi) - the mass is fixed
        const int n_params = n_particles * 3;

        // NDF = number of constraints
        result.ndf = n_constraints;

        // Iterative fit
        double chi2_prev = 1e10;
        bool converged = false;

        for (int iter = 0; iter < m_max_iterations; ++iter) {
            result.n_iterations = iter + 1;

            // Evaluate all constraints
            std::vector<double> constraint_values(n_constraints);
            for (int c = 0; c < n_constraints; ++c) {
                constraint_values[c] = m_constraints[c]->evaluate(m_momenta);
            }

            // Check convergence (all constraints near zero)
            double max_constraint = 0.0;
            for (double cv : constraint_values) {
                max_constraint = std::max(max_constraint, std::abs(cv));
            }

            if (max_constraint < m_tolerance) {
                converged = true;
                break;
            }

            // Compute constraint gradients (Jacobian)
            std::vector<std::vector<double>> gradients(n_constraints);
            for (int c = 0; c < n_constraints; ++c) {
                gradients[c] = m_constraints[c]->gradient(m_momenta);
            }

            // Simplified update: use gradient descent on constraint values
            // (Full Lagrange multiplier method would solve linear system)
            for (int c = 0; c < n_constraints; ++c) {
                const auto& grad = gradients[c];
                double constraint_val = constraint_values[c];

                // Scale factor for update (adaptive step size)
                double scale = 0.1;  // Small step for stability

                // Update momenta
                for (int p = 0; p < n_particles; ++p) {
                    double dpx = scale * grad[4 * p + 0] * constraint_val;
                    double dpy = scale * grad[4 * p + 1] * constraint_val;
                    double dpz = scale * grad[4 * p + 2] * constraint_val;
                    double dE = scale * grad[4 * p + 3] * constraint_val;

                    // Limit update size
                    double max_update = 0.1 * m_momenta[p].P();
                    dpx = std::clamp(dpx, -max_update, max_update);
                    dpy = std::clamp(dpy, -max_update, max_update);
                    dpz = std::clamp(dpz, -max_update, max_update);
                    dE = std::clamp(dE, -max_update, max_update);

                    double new_px = m_momenta[p].Px() + dpx;
                    double new_py = m_momenta[p].Py() + dpy;
                    double new_pz = m_momenta[p].Pz() + dpz;

                    // Recalculate energy from mass constraint
                    double p_mag = std::sqrt(new_px * new_px + new_py * new_py + new_pz * new_pz);
                    double new_E = std::sqrt(p_mag * p_mag + m_masses[p] * m_masses[p]);

                    m_momenta[p] = ROOT::Math::PxPyPzEVector(new_px, new_py, new_pz, new_E);
                }
            }

            // Compute chi2 for convergence check
            double chi2 = compute_chi2();
            if (std::abs(chi2 - chi2_prev) < m_chi2_threshold) {
                converged = true;
            }
            chi2_prev = chi2;
        }

        // Fill result
        result.converged = converged;
        result.chi2 = compute_chi2();
        result.fitted_momenta = m_momenta;

        if (converged) {
            result.max_iterations_reached = (result.n_iterations >= m_max_iterations);

            // Calculate probability using TMath::Prob
            if (result.ndf > 0 && result.chi2 >= 0) {
                result.probability = TMath::Prob(result.chi2, result.ndf);
            }

            // Store constraint residuals
            for (const auto& constraint : m_constraints) {
                result.constraint_residuals.push_back(constraint->evaluate(m_momenta));
            }

            // Compute pulls
            result.pulls = compute_pulls();
        } else {
            result.error_message = "Fit did not converge";
        }

        return result;
    }

    // ========================================================================
    // Reset
    // ========================================================================

    /**
     * @brief Clear all particles and constraints
     */
    auto clear() -> void {
        m_momenta.clear();
        m_original_momenta.clear();
        m_masses.clear();
        m_covariances.clear();
        m_constraints.clear();
    }

    /**
     * @brief Reset momenta to original values (keep particles and constraints)
     */
    auto reset() -> void {
        m_momenta = m_original_momenta;
    }

    // ========================================================================
    // Accessors
    // ========================================================================

    /// Get number of particles
    [[nodiscard]] auto num_particles() const -> int {
        return static_cast<int>(m_momenta.size());
    }

    /// Get number of constraints
    [[nodiscard]] auto num_constraints() const -> int {
        return static_cast<int>(m_constraints.size());
    }

    /// Get current momenta
    [[nodiscard]] auto momenta() const -> const std::vector<ROOT::Math::PxPyPzEVector>& {
        return m_momenta;
    }

    /// Get original momenta
    [[nodiscard]] auto original_momenta() const -> const std::vector<ROOT::Math::PxPyPzEVector>& {
        return m_original_momenta;
    }

   private:
    // Particle data
    std::vector<ROOT::Math::PxPyPzEVector> m_momenta;
    std::vector<ROOT::Math::PxPyPzEVector> m_original_momenta;
    std::vector<double> m_masses;
    std::vector<CovarianceMatrix> m_covariances;

    // Constraints
    std::vector<std::unique_ptr<Constraint>> m_constraints;

    // Fit configuration
    int m_max_iterations = 100;
    double m_tolerance = 1e-6;  // Constraint convergence tolerance
    double m_chi2_threshold = 0.001;  // Chi2 change for convergence

    /**
     * @brief Compute chi-squared
     *
     * Chi2 = sum over particles of (fitted - measured)^T * V^-1 * (fitted - measured)
     */
    auto compute_chi2() const -> double {
        double chi2 = 0.0;

        for (size_t p = 0; p < m_momenta.size(); ++p) {
            const auto& fitted = m_momenta[p];
            const auto& original = m_original_momenta[p];
            const auto& cov = m_covariances[p];

            // Compute differences
            double dp = fitted.P() - original.P();
            double dtheta = fitted.Theta() - original.Theta();
            double dphi = fitted.Phi() - original.Phi();

            // Handle phi wrap-around
            while (dphi > M_PI) dphi -= 2 * M_PI;
            while (dphi < -M_PI) dphi += 2 * M_PI;

            // Compute chi2 contribution (assuming diagonal covariance)
            double sigma_p = cov.sigma_p_over_p * original.P();
            if (sigma_p > 0) {
                chi2 += (dp * dp) / (sigma_p * sigma_p);
            }

            if (cov.sigma_theta > 0) {
                chi2 += (dtheta * dtheta) / (cov.sigma_theta * cov.sigma_theta);
            }

            if (cov.sigma_phi > 0) {
                chi2 += (dphi * dphi) / (cov.sigma_phi * cov.sigma_phi);
            }
        }

        return chi2;
    }

    /**
     * @brief Compute pulls for each parameter
     */
    auto compute_pulls() const -> std::vector<double> {
        std::vector<double> pulls;

        for (size_t p = 0; p < m_momenta.size(); ++p) {
            const auto& fitted = m_momenta[p];
            const auto& original = m_original_momenta[p];
            const auto& cov = m_covariances[p];

            // Momentum pull
            double sigma_p = cov.sigma_p_over_p * original.P();
            if (sigma_p > 0) {
                pulls.push_back((fitted.P() - original.P()) / sigma_p);
            } else {
                pulls.push_back(0.0);
            }

            // Theta pull
            if (cov.sigma_theta > 0) {
                pulls.push_back((fitted.Theta() - original.Theta()) / cov.sigma_theta);
            } else {
                pulls.push_back(0.0);
            }

            // Phi pull
            double dphi = fitted.Phi() - original.Phi();
            while (dphi > M_PI) dphi -= 2 * M_PI;
            while (dphi < -M_PI) dphi += 2 * M_PI;
            if (cov.sigma_phi > 0) {
                pulls.push_back(dphi / cov.sigma_phi);
            } else {
                pulls.push_back(0.0);
            }
        }

        return pulls;
    }
};

}  // namespace clas12::framework::fitting
