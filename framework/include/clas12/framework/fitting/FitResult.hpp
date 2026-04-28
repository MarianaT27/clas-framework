#pragma once

#include <cmath>
#include <string>
#include <vector>

#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMath.h>

namespace framework::fitting {

/**
 * @brief Result of a kinematic fit
 *
 * Contains the fitted particle momenta, chi-squared, convergence status,
 * and other fit quality metrics.
 */
struct FitResult {
    /// Whether the fit converged
    bool converged = false;

    /// Chi-squared of the fit
    double chi2 = -1.0;

    /// Number of degrees of freedom
    int ndf = 0;

    /// Fit probability (P(chi2, ndf))
    double probability = 0.0;

    /// Number of iterations used
    int n_iterations = 0;

    /// Maximum number of iterations reached
    bool max_iterations_reached = false;

    /// Fitted 4-momenta of all particles
    std::vector<ROOT::Math::PxPyPzEVector> fitted_momenta;

    /// Fitted common vertex (if vertex constraint applied)
    ROOT::Math::XYZVector fitted_vertex{0, 0, 0};

    /// Whether vertex was fitted
    bool vertex_fitted = false;

    /// Pulls for each measured parameter
    std::vector<double> pulls;

    /// Constraint residuals after fit
    std::vector<double> constraint_residuals;

    /// Error message (if failed)
    std::string error_message;

    // ========================================================================
    // Quality Checks
    // ========================================================================

    /**
     * @brief Get chi2/ndf
     * @return chi2/ndf, or -1 if ndf <= 0
     */
    [[nodiscard]] auto chi2_ndf() const -> double {
        return ndf > 0 ? chi2 / ndf : -1.0;
    }

    /**
     * @brief Check if fit is good
     *
     * @param min_prob Minimum probability threshold (default 0.01 = 1%)
     * @return True if converged and probability above threshold
     */
    [[nodiscard]] auto is_good(double min_prob = 0.01) const -> bool {
        return converged && probability > min_prob;
    }

    /**
     * @brief Check if fit passed basic quality
     *
     * Requires convergence and reasonable chi2
     */
    [[nodiscard]] auto is_valid() const -> bool {
        return converged && chi2 >= 0.0;
    }

    /**
     * @brief Get fitted invariant mass of particle combination
     *
     * @param indices Particle indices to combine
     * @return Invariant mass [GeV/c^2]
     */
    [[nodiscard]] auto fitted_mass(const std::vector<int>& indices) const -> double {
        ROOT::Math::PxPyPzEVector sum(0, 0, 0, 0);
        for (int idx : indices) {
            if (idx >= 0 && static_cast<size_t>(idx) < fitted_momenta.size()) {
                sum += fitted_momenta[idx];
            }
        }
        return sum.M();
    }

    /**
     * @brief Get fitted mass of all particles combined
     */
    [[nodiscard]] auto fitted_total_mass() const -> double {
        ROOT::Math::PxPyPzEVector sum(0, 0, 0, 0);
        for (const auto& p : fitted_momenta) {
            sum += p;
        }
        return sum.M();
    }

    /**
     * @brief Get momentum of fitted particle
     *
     * @param idx Particle index
     * @return 4-momentum, or zero vector if invalid
     */
    [[nodiscard]] auto momentum(int idx) const -> ROOT::Math::PxPyPzEVector {
        if (idx >= 0 && static_cast<size_t>(idx) < fitted_momenta.size()) {
            return fitted_momenta[idx];
        }
        return ROOT::Math::PxPyPzEVector(0, 0, 0, 0);
    }

    /**
     * @brief Calculate confidence level
     */
    [[nodiscard]] auto confidence_level() const -> double {
        return probability;
    }
};

/**
 * @brief Covariance matrix for a particle (simplified)
 *
 * For a full kinematic fit, we need covariance matrices for the
 * measured parameters. This simplified version assumes diagonal
 * covariance with momentum and angular resolutions.
 */
struct CovarianceMatrix {
    /// Momentum resolution (sigma_p / p)
    double sigma_p_over_p = 0.01;  // 1% default

    /// Polar angle resolution [radians]
    double sigma_theta = 0.001;  // ~0.06 degrees

    /// Azimuthal angle resolution [radians]
    double sigma_phi = 0.001;

    /// Energy resolution (for calorimeter particles)
    double sigma_E_over_E = 0.0;  // 0 = not used

    /// Check if this is a default (unset) covariance
    [[nodiscard]] auto is_default() const -> bool {
        return sigma_p_over_p == 0.01 && sigma_theta == 0.001 && sigma_phi == 0.001;
    }

    /**
     * @brief Create from particle type
     *
     * Sets typical resolutions for CLAS12.
     */
    static auto for_charged_track() -> CovarianceMatrix {
        return CovarianceMatrix{0.01, 0.001, 0.001, 0.0};
    }

    static auto for_photon() -> CovarianceMatrix {
        return CovarianceMatrix{0.0, 0.003, 0.003, 0.05};  // 5% energy resolution
    }

    static auto for_neutral() -> CovarianceMatrix {
        return CovarianceMatrix{0.0, 0.005, 0.005, 0.10};  // Worse for neutrals
    }
};

}  // namespace framework::fitting
