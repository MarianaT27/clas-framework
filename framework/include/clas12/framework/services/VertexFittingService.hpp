#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMath.h>

// Eigen for matrix operations
#include <Eigen/Dense>

namespace clas12::framework {

/**
 * @brief Vertex constraint type
 */
enum class VertexConstraint {
    None,           ///< Free vertex
    BeamSpot,       ///< Constrain to beam spot
    PrimaryVertex,  ///< Constrain to primary vertex
    Target          ///< Constrain to target region
};

/**
 * @brief Result of a vertex fit
 */
struct VertexFitResult {
    bool converged = false;                           ///< Fit converged
    ROOT::Math::XYZVector position{0, 0, 0};         ///< Fitted vertex position [cm]
    Eigen::Matrix3d covariance = Eigen::Matrix3d::Identity(); ///< Position covariance
    double chi2 = -1.0;                               ///< Fit chi-squared
    int ndf = 0;                                      ///< Degrees of freedom
    double probability = 0.0;                         ///< Fit probability
    int n_iterations = 0;                             ///< Number of iterations used
    std::string error_message;                        ///< Error message if failed

    /// Refitted track momenta at the vertex
    std::vector<ROOT::Math::PxPyPzEVector> refitted_momenta;

    /// Chi2 contribution from each track
    std::vector<double> track_chi2_contributions;

    // ========================================================================
    // Quality Methods
    // ========================================================================

    /**
     * @brief Check if fit is good
     *
     * @param min_prob Minimum probability required
     */
    [[nodiscard]] auto is_good(double min_prob = 0.01) const -> bool {
        return converged && probability > min_prob;
    }

    /**
     * @brief Get chi2/ndf
     */
    [[nodiscard]] auto chi2_ndf() const -> double {
        return (ndf > 0) ? chi2 / ndf : -1.0;
    }

    /**
     * @brief Calculate distance from a point
     *
     * @param point Reference point
     * @return Distance in cm
     */
    [[nodiscard]] auto distance_from(const ROOT::Math::XYZVector& point) const -> double {
        return (position - point).R();
    }

    /**
     * @brief Calculate significance of distance from a point
     *
     * Uses the covariance matrix to compute the distance significance.
     *
     * @param point Reference point
     * @return Distance significance (distance / error)
     */
    [[nodiscard]] auto significance_from(const ROOT::Math::XYZVector& point) const -> double {
        Eigen::Vector3d d;
        d << (position.X() - point.X()),
             (position.Y() - point.Y()),
             (position.Z() - point.Z());

        // Compute d^T * V^-1 * d
        double chi2_dist = d.transpose() * covariance.inverse() * d;
        return std::sqrt(chi2_dist);
    }

    /**
     * @brief Get position error in X
     */
    [[nodiscard]] auto error_x() const -> double { return std::sqrt(covariance(0, 0)); }

    /**
     * @brief Get position error in Y
     */
    [[nodiscard]] auto error_y() const -> double { return std::sqrt(covariance(1, 1)); }

    /**
     * @brief Get position error in Z
     */
    [[nodiscard]] auto error_z() const -> double { return std::sqrt(covariance(2, 2)); }
};

/**
 * @brief Track parameters for vertex fitting
 */
struct TrackParameters {
    ROOT::Math::XYZVector point;      ///< Reference point on track
    ROOT::Math::XYZVector direction;  ///< Track direction (unit vector)
    Eigen::Matrix3d covariance;       ///< Position covariance at reference
    double momentum = 0.0;            ///< Track momentum magnitude
    double mass = 0.0;                ///< Assumed particle mass

    TrackParameters() : covariance(Eigen::Matrix3d::Identity() * 0.01) {}
};

/**
 * @brief Service for vertex reconstruction and fitting
 *
 * VertexFittingService provides vertex fitting capabilities for:
 * - Primary vertex reconstruction from multiple tracks
 * - Secondary vertex fitting for decay analyses
 * - Flight distance and significance calculations
 *
 * The service uses a least-squares approach with Kalman filter updates
 * for robust vertex estimation.
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_vertex_svc = &require_service<VertexFittingService>("VertexFittingService");
 *
 * // Fit primary vertex
 * auto tracks = event.all_particles_vector();
 * auto pv = m_vertex_svc->fit_primary_vertex_robust(tracks, event);
 *
 * // Fit secondary vertex for Lambda decay
 * auto sv = m_vertex_svc->fit_secondary_vertex(proton, pion, event);
 *
 * if (sv.is_good()) {
 *     auto [dist, sig] = m_vertex_svc->flight_distance(pv, sv);
 *     h_flight_significance->Fill(sig);
 * }
 * @endcode
 */
class VertexFittingService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "VertexFittingService"; }

    explicit VertexFittingService(std::string name = "VertexFittingService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "VertexFittingService initialized");
        Log::info(this->name(), "  Max iterations: {}", m_max_iterations.get());
        Log::info(this->name(), "  Convergence tolerance: {}", m_convergence_tolerance.get());
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "VertexFittingService finalized");
        Log::info(this->name(), "  Total fits: {}", m_total_fits);
        Log::info(this->name(), "  Successful fits: {}", m_successful_fits);
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Convergence tolerance for iterative fit
    Property<double> m_convergence_tolerance{"convergence_tolerance", 1e-6,
                                             "Fit convergence threshold"};

    /// Maximum iterations
    Property<int> m_max_iterations{"max_iterations", 100, "Maximum fit iterations"};

    /// Chi2/ndf cut for good vertex
    Property<double> m_chi2_cut{"chi2_cut", 50.0, "Maximum chi2/ndf for good vertex"};

    /// Use beam spot constraint
    Property<bool> m_use_beam_constraint{"use_beam_constraint", false,
                                         "Apply beam spot constraint"};

    /// Default track position resolution [cm]
    Property<double> m_default_position_resolution{"default_position_resolution", 0.1,
                                                   "Default track position resolution [cm]"};

    // ========================================================================
    // Primary Vertex Fitting
    // ========================================================================

    /**
     * @brief Fit primary vertex from tracks
     *
     * Uses a weighted least-squares approach to find the vertex
     * that minimizes the sum of squared distances to all tracks.
     *
     * @param tracks Vector of particles to fit
     * @param event Event for bank access
     * @param constraint Optional constraint type
     * @return VertexFitResult with fitted vertex position
     */
    auto fit_primary_vertex(const std::vector<clas12::Particle>& tracks,
                            Event& event,
                            VertexConstraint constraint = VertexConstraint::None)
        -> VertexFitResult {
        ++m_total_fits;
        VertexFitResult result;

        if (tracks.size() < 2) {
            result.error_message = "Need at least 2 tracks for vertex fit";
            return result;
        }

        // Convert particles to track parameters
        std::vector<TrackParameters> track_params;
        track_params.reserve(tracks.size());

        for (const auto& track : tracks) {
            track_params.push_back(extract_track_parameters(track, event));
        }

        // Perform fit
        result = fit_vertex_least_squares(track_params);

        // Apply constraint if requested
        if (result.converged && constraint != VertexConstraint::None) {
            apply_constraint(result, constraint);
        }

        if (result.converged) {
            ++m_successful_fits;
        }

        return result;
    }

    /**
     * @brief Fit primary vertex with outlier rejection
     *
     * Iteratively removes tracks with large chi2 contribution
     * until the fit stabilizes.
     *
     * @param tracks Vector of particles
     * @param event Event for bank access
     * @param outlier_chi2_cut Chi2 cut for outlier removal
     * @return VertexFitResult with robust vertex
     */
    auto fit_primary_vertex_robust(const std::vector<clas12::Particle>& tracks,
                                   Event& event,
                                   double outlier_chi2_cut = 9.0) -> VertexFitResult {
        ++m_total_fits;
        VertexFitResult result;

        if (tracks.size() < 2) {
            result.error_message = "Need at least 2 tracks for vertex fit";
            return result;
        }

        // Convert particles to track parameters
        std::vector<TrackParameters> track_params;
        track_params.reserve(tracks.size());

        for (const auto& track : tracks) {
            track_params.push_back(extract_track_parameters(track, event));
        }

        // Iterative outlier removal
        const int max_outlier_iterations = 5;
        for (int iter = 0; iter < max_outlier_iterations; ++iter) {
            if (track_params.size() < 2) break;

            result = fit_vertex_least_squares(track_params);
            if (!result.converged) break;

            // Find and remove outliers
            bool removed_outlier = false;
            for (size_t i = 0; i < result.track_chi2_contributions.size(); ++i) {
                if (result.track_chi2_contributions[i] > outlier_chi2_cut) {
                    track_params.erase(track_params.begin() + i);
                    removed_outlier = true;
                    break;  // Re-fit after each removal
                }
            }

            if (!removed_outlier) break;
        }

        if (result.converged) {
            ++m_successful_fits;
        }

        return result;
    }

    // ========================================================================
    // Secondary Vertex Fitting
    // ========================================================================

    /**
     * @brief Fit secondary vertex (2-body decay)
     *
     * @param track1 First decay product
     * @param track2 Second decay product
     * @param event Event for bank access
     * @return VertexFitResult with decay vertex
     */
    auto fit_secondary_vertex(const clas12::Particle& track1,
                              const clas12::Particle& track2,
                              Event& event) -> VertexFitResult {
        return fit_primary_vertex({track1, track2}, event, VertexConstraint::None);
    }

    /**
     * @brief Fit secondary vertex with mass constraint
     *
     * Fits the vertex while constraining the invariant mass of
     * the decay products.
     *
     * @param tracks Decay products
     * @param mass_constraint Target invariant mass [GeV/c^2]
     * @param event Event for bank access
     * @return VertexFitResult with constrained vertex
     */
    auto fit_secondary_vertex_with_mass(const std::vector<clas12::Particle>& tracks,
                                        double mass_constraint,
                                        Event& event) -> VertexFitResult {
        ++m_total_fits;
        VertexFitResult result;

        if (tracks.size() < 2) {
            result.error_message = "Need at least 2 tracks for vertex fit";
            return result;
        }

        // First fit vertex without mass constraint
        result = fit_primary_vertex(tracks, event, VertexConstraint::None);

        if (!result.converged) {
            return result;
        }

        // Iteratively adjust momenta to satisfy mass constraint
        // This is a simplified approach - full DTF does this properly
        double current_mass = 0.0;
        ROOT::Math::PxPyPzEVector total_p4;
        for (const auto& track : tracks) {
            total_p4 += track.PxPyPzEVector();
        }
        current_mass = total_p4.M();

        // Store refitted momenta (scaled to match mass constraint)
        double scale = mass_constraint / current_mass;
        result.refitted_momenta.clear();
        for (const auto& track : tracks) {
            auto p4 = track.PxPyPzEVector();
            // Simple scaling (proper implementation would use Lagrange multipliers)
            double E = std::sqrt(p4.P2() * scale * scale + track.mass() * track.mass());
            result.refitted_momenta.emplace_back(
                p4.Px() * scale, p4.Py() * scale, p4.Pz() * scale, E);
        }

        if (result.converged) {
            ++m_successful_fits;
        }

        return result;
    }

    /**
     * @brief Fit secondary vertex for N tracks
     */
    auto fit_secondary_vertex(const std::vector<clas12::Particle>& tracks,
                              Event& event) -> VertexFitResult {
        return fit_primary_vertex(tracks, event, VertexConstraint::None);
    }

    // ========================================================================
    // Flight Distance Calculations
    // ========================================================================

    /**
     * @brief Calculate flight distance from PV to SV
     *
     * @param primary Primary vertex result
     * @param secondary Secondary vertex result
     * @return Pair of (distance [cm], significance)
     */
    [[nodiscard]] auto flight_distance(const VertexFitResult& primary,
                                       const VertexFitResult& secondary) const
        -> std::pair<double, double> {
        // Calculate distance
        double dist = secondary.distance_from(primary.position);

        // Calculate significance using combined covariance
        Eigen::Matrix3d total_cov = primary.covariance + secondary.covariance;

        Eigen::Vector3d d;
        d << (secondary.position.X() - primary.position.X()),
             (secondary.position.Y() - primary.position.Y()),
             (secondary.position.Z() - primary.position.Z());

        double chi2_dist = d.transpose() * total_cov.inverse() * d;
        double significance = std::sqrt(chi2_dist);

        return {dist, significance};
    }

    /**
     * @brief Calculate decay length significance
     *
     * Projects the flight distance onto the momentum direction
     * for proper decay length measurement.
     *
     * @param primary Primary vertex
     * @param secondary Secondary vertex
     * @param momentum Mother particle momentum
     * @return Decay length significance
     */
    [[nodiscard]] auto decay_length_significance(
        const VertexFitResult& primary,
        const VertexFitResult& secondary,
        const ROOT::Math::XYZVector& momentum) const -> double {

        // Flight vector
        ROOT::Math::XYZVector flight = secondary.position - primary.position;

        // Project onto momentum direction
        double p_mag = momentum.R();
        if (p_mag < 1e-10) return 0.0;

        ROOT::Math::XYZVector p_unit = momentum / p_mag;
        double decay_length = flight.Dot(p_unit);

        // Calculate error on decay length
        Eigen::Matrix3d total_cov = primary.covariance + secondary.covariance;

        Eigen::Vector3d p_vec;
        p_vec << p_unit.X(), p_unit.Y(), p_unit.Z();

        double decay_length_var = p_vec.transpose() * total_cov * p_vec;
        double decay_length_err = std::sqrt(decay_length_var);

        if (decay_length_err < 1e-10) return 0.0;
        return decay_length / decay_length_err;
    }

    /**
     * @brief Calculate impact parameter of a track to a vertex
     *
     * @param track Track particle
     * @param vertex Vertex position
     * @param event Event for bank access
     * @return Impact parameter [cm]
     */
    [[nodiscard]] auto impact_parameter(const clas12::Particle& track,
                                        const ROOT::Math::XYZVector& vertex,
                                        Event& event) const -> double {
        auto params = extract_track_parameters(track, event);

        // Vector from vertex to track reference point
        ROOT::Math::XYZVector diff = params.point - vertex;

        // Impact parameter is perpendicular distance
        double proj = diff.Dot(params.direction);
        ROOT::Math::XYZVector perp = diff - proj * params.direction;

        return perp.R();
    }

    // ========================================================================
    // Beam Spot Configuration
    // ========================================================================

    /**
     * @brief Set beam spot position and uncertainty
     */
    auto set_beam_spot(const ROOT::Math::XYZVector& position,
                       const Eigen::Matrix3d& covariance) -> void {
        m_beam_spot = position;
        m_beam_spot_cov = covariance;
    }

    /**
     * @brief Set target position for constraint
     */
    auto set_target_position(double z_center, double z_width) -> void {
        m_target_z = z_center;
        m_target_width = z_width;
    }

    /**
     * @brief Get beam spot position
     */
    [[nodiscard]] auto beam_spot() const -> const ROOT::Math::XYZVector& {
        return m_beam_spot;
    }

   private:
    // Beam spot configuration
    ROOT::Math::XYZVector m_beam_spot{0.0, 0.0, 0.0};
    Eigen::Matrix3d m_beam_spot_cov = Eigen::Matrix3d::Identity() * 0.01;  // 1mm default
    double m_target_z = -5.0;
    double m_target_width = 5.0;

    // Statistics
    mutable size_t m_total_fits = 0;
    mutable size_t m_successful_fits = 0;

    /**
     * @brief Extract track parameters from particle
     */
    auto extract_track_parameters(const clas12::Particle& particle,
                                  Event& /*event*/) const -> TrackParameters {
        TrackParameters params;

        // Reference point is the particle vertex
        params.point = ROOT::Math::XYZVector(particle.vx(), particle.vy(), particle.vz());

        // Direction from momentum
        double p = particle.p();
        if (p > 1e-10) {
            params.direction = ROOT::Math::XYZVector(
                particle.px() / p, particle.py() / p, particle.pz() / p);
        } else {
            params.direction = ROOT::Math::XYZVector(0, 0, 1);
        }

        params.momentum = p;
        params.mass = particle.mass();

        // Set covariance (use default resolution)
        double sigma = m_default_position_resolution.get();
        params.covariance = Eigen::Matrix3d::Identity() * sigma * sigma;

        return params;
    }

    /**
     * @brief Perform least-squares vertex fit
     */
    auto fit_vertex_least_squares(const std::vector<TrackParameters>& tracks)
        -> VertexFitResult {
        VertexFitResult result;

        if (tracks.size() < 2) {
            result.error_message = "Need at least 2 tracks";
            return result;
        }

        // Initialize vertex at average of track reference points
        Eigen::Vector3d vertex = Eigen::Vector3d::Zero();
        for (const auto& track : tracks) {
            vertex(0) += track.point.X();
            vertex(1) += track.point.Y();
            vertex(2) += track.point.Z();
        }
        vertex /= static_cast<double>(tracks.size());

        // Iterative weighted least squares
        const double tolerance = m_convergence_tolerance.get();
        const int max_iter = m_max_iterations.get();

        for (int iter = 0; iter < max_iter; ++iter) {
            result.n_iterations = iter + 1;

            // Accumulate weighted sum
            Eigen::Matrix3d W = Eigen::Matrix3d::Zero();  // Weight matrix
            Eigen::Vector3d b = Eigen::Vector3d::Zero();  // Weighted residuals

            for (const auto& track : tracks) {
                // Direction matrix: I - d*d^T
                Eigen::Vector3d d;
                d << track.direction.X(), track.direction.Y(), track.direction.Z();
                Eigen::Matrix3d D = Eigen::Matrix3d::Identity() - d * d.transpose();

                // Weight by inverse covariance projected perpendicular to track
                Eigen::Matrix3d cov_inv = track.covariance.inverse();
                Eigen::Matrix3d Wi = D * cov_inv * D;

                // Reference point
                Eigen::Vector3d pi;
                pi << track.point.X(), track.point.Y(), track.point.Z();

                W += Wi;
                b += Wi * pi;
            }

            // Solve for vertex
            Eigen::Vector3d new_vertex = W.ldlt().solve(b);

            // Check convergence
            double delta = (new_vertex - vertex).norm();
            vertex = new_vertex;

            if (delta < tolerance) {
                result.converged = true;
                break;
            }
        }

        // Store result
        result.position = ROOT::Math::XYZVector(vertex(0), vertex(1), vertex(2));

        // Compute covariance (inverse of weight matrix)
        Eigen::Matrix3d W = Eigen::Matrix3d::Zero();
        for (const auto& track : tracks) {
            Eigen::Vector3d d;
            d << track.direction.X(), track.direction.Y(), track.direction.Z();
            Eigen::Matrix3d D = Eigen::Matrix3d::Identity() - d * d.transpose();
            Eigen::Matrix3d cov_inv = track.covariance.inverse();
            W += D * cov_inv * D;
        }
        result.covariance = W.inverse();

        // Compute chi2 and track contributions
        result.chi2 = 0.0;
        result.track_chi2_contributions.clear();

        for (const auto& track : tracks) {
            // Distance from vertex to track
            Eigen::Vector3d v;
            v << vertex(0) - track.point.X(),
                 vertex(1) - track.point.Y(),
                 vertex(2) - track.point.Z();

            Eigen::Vector3d d;
            d << track.direction.X(), track.direction.Y(), track.direction.Z();

            // Perpendicular distance
            Eigen::Vector3d perp = v - (v.dot(d)) * d;

            // Chi2 contribution
            double track_chi2 = perp.transpose() * track.covariance.inverse() * perp;
            result.track_chi2_contributions.push_back(track_chi2);
            result.chi2 += track_chi2;
        }

        // NDF = 2 * n_tracks - 3 (vertex has 3 parameters)
        result.ndf = static_cast<int>(2 * tracks.size() - 3);
        if (result.ndf > 0 && result.chi2 >= 0) {
            result.probability = TMath::Prob(result.chi2, result.ndf);
        }

        return result;
    }

    /**
     * @brief Apply vertex constraint
     */
    auto apply_constraint(VertexFitResult& result, VertexConstraint constraint) -> void {
        switch (constraint) {
            case VertexConstraint::BeamSpot: {
                // Combine beam spot with fitted vertex
                Eigen::Matrix3d V_inv = result.covariance.inverse();
                Eigen::Matrix3d B_inv = m_beam_spot_cov.inverse();

                Eigen::Vector3d v;
                v << result.position.X(), result.position.Y(), result.position.Z();

                Eigen::Vector3d b;
                b << m_beam_spot.X(), m_beam_spot.Y(), m_beam_spot.Z();

                Eigen::Matrix3d new_cov = (V_inv + B_inv).inverse();
                Eigen::Vector3d new_pos = new_cov * (V_inv * v + B_inv * b);

                result.position = ROOT::Math::XYZVector(new_pos(0), new_pos(1), new_pos(2));
                result.covariance = new_cov;
                break;
            }
            case VertexConstraint::Target: {
                // Constrain Z to target region
                double z = result.position.Z();
                if (z < m_target_z - m_target_width / 2) {
                    result.position.SetZ(m_target_z - m_target_width / 2);
                } else if (z > m_target_z + m_target_width / 2) {
                    result.position.SetZ(m_target_z + m_target_width / 2);
                }
                break;
            }
            case VertexConstraint::PrimaryVertex:
            case VertexConstraint::None:
            default:
                break;
        }
    }
};

}  // namespace clas12::framework
