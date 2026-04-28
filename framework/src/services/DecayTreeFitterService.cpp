// Out-of-line definitions for DecayTreeFitterService's big Eigen-heavy
// members. The fit() body is ~125 LOC of iterative Lagrange-multiplier
// math; evaluate_constraints() and compute_jacobian() are its helpers.
// Moving them here keeps the Eigen matrix algebra out of every analysis TU.

#include <clas12/framework/services/DecayTreeFitterService.hpp>

#include <cmath>

namespace framework {

auto DTFFitter::fit() -> DTFResult {
        DTFResult result;

#if !CLAS12_HAS_EIGEN
        result.error_message = "DecayTreeFitter requires Eigen library (not available)";
        return result;
#else
        if (m_particles.empty()) {
            result.error_message = "No particles added";
            return result;
        }

        if (m_constraints.empty()) {
            result.error_message = "No constraints added";
            return result;
        }

        // Number of particles and constraints
        const int n_particles = static_cast<int>(m_particles.size());
        const int n_constraints = count_constraints();

        // Parameters: 3 momentum components per particle
        const int n_params = n_particles * 3;

        // Initialize parameter vector (px, py, pz for each particle)
        Eigen::VectorXd params(n_params);
        for (int i = 0; i < n_particles; ++i) {
            const auto& p4 = m_initial_momenta[i];
            params(3 * i + 0) = p4.Px();
            params(3 * i + 1) = p4.Py();
            params(3 * i + 2) = p4.Pz();
        }

        // Covariance matrix (diagonal, simplified)
        Eigen::MatrixXd V = Eigen::MatrixXd::Identity(n_params, n_params);
        for (int i = 0; i < n_particles; ++i) {
            double p = m_initial_momenta[i].P();
            double sigma_p = 0.01 * p;  // 1% momentum resolution
            V(3 * i + 0, 3 * i + 0) = sigma_p * sigma_p;
            V(3 * i + 1, 3 * i + 1) = sigma_p * sigma_p;
            V(3 * i + 2, 3 * i + 2) = sigma_p * sigma_p;
        }

        // Iterative fit
        double chi2_prev = 1e10;
        bool converged = false;

        for (int iter = 0; iter < m_max_iterations; ++iter) {
            result.n_iterations = iter + 1;

            // Evaluate constraints
            Eigen::VectorXd c = evaluate_constraints(params);

            // Check convergence
            double max_residual = c.cwiseAbs().maxCoeff();
            if (max_residual < m_tolerance) {
                converged = true;
                break;
            }

            // Compute Jacobian
            Eigen::MatrixXd D = compute_jacobian(params);

            // Update parameters using Lagrange multipliers
            // Solve: [V  D^T] [delta_alpha] = [0]
            //        [D  0  ] [lambda     ]   [c]

            Eigen::MatrixXd DV = D * V;
            Eigen::MatrixXd DV_Dt = DV * D.transpose();

            // Regularize if needed
            DV_Dt += Eigen::MatrixXd::Identity(n_constraints, n_constraints) * 1e-10;

            // Solve for lambda
            Eigen::VectorXd lambda = DV_Dt.ldlt().solve(-c);

            // Update parameters
            Eigen::VectorXd delta = V * D.transpose() * lambda;
            params += delta;

            // Check chi2 convergence
            double chi2 = lambda.transpose() * c;
            if (std::abs(chi2 - chi2_prev) < 1e-6) {
                converged = true;
            }
            chi2_prev = chi2;
        }

        // Store results
        result.converged = converged;

        // Compute final chi2
        Eigen::VectorXd c_final = evaluate_constraints(params);
        result.chi2 = 0.0;
        for (int i = 0; i < c_final.size(); ++i) {
            result.chi2 += c_final(i) * c_final(i);
            result.constraint_residuals.push_back(c_final(i));
        }

        result.ndf = n_constraints;
        if (result.ndf > 0 && result.chi2 >= 0) {
            result.probability = TMath::Prob(result.chi2, result.ndf);
        }

        // Store fitted momenta
        for (int i = 0; i < n_particles; ++i) {
            double px = params(3 * i + 0);
            double py = params(3 * i + 1);
            double pz = params(3 * i + 2);
            double p = std::sqrt(px * px + py * py + pz * pz);
            double E = std::sqrt(p * p + m_masses[i] * m_masses[i]);
            result.fitted_momenta[i] = ROOT::Math::PxPyPzEVector(px, py, pz, E);
        }

        // Compute decay vertex (average of track vertices)
        ROOT::Math::XYZVector decay_vertex(0, 0, 0);
        for (const auto& [role, particle] : m_particles) {
            decay_vertex += ROOT::Math::XYZVector(particle.vx(), particle.vy(), particle.vz());
        }
        decay_vertex /= static_cast<double>(m_particles.size());
        result.fitted_vertices[0] = decay_vertex;

        return result;
#endif  // CLAS12_HAS_EIGEN
    }

#if CLAS12_HAS_EIGEN
auto DTFFitter::evaluate_constraints(const Eigen::VectorXd& params) const
    -> Eigen::VectorXd {
        int n = count_constraints();
        Eigen::VectorXd c(n);
        c.setZero();

        int idx = 0;
        for (const auto& constraint : m_constraints) {
            switch (constraint.type) {
                case DTFConstraint::Type::Mass: {
                    // Compute invariant mass
                    ROOT::Math::PxPyPzEVector total;
                    for (int i : constraint.particle_indices) {
                        double px = params(3 * i + 0);
                        double py = params(3 * i + 1);
                        double pz = params(3 * i + 2);
                        double p = std::sqrt(px * px + py * py + pz * pz);
                        double E = std::sqrt(p * p + m_masses.at(i) * m_masses.at(i));
                        total += ROOT::Math::PxPyPzEVector(px, py, pz, E);
                    }
                    // Constraint: M^2 - m_target^2 = 0
                    c(idx++) = total.M2() - constraint.value * constraint.value;
                    break;
                }
                case DTFConstraint::Type::Vertex: {
                    // Vertex constraints simplified (differences in vertex z)
                    // In full implementation, would use track helix parameters
                    for (size_t i = 1; i < constraint.particle_indices.size(); ++i) {
                        // Placeholder: assume vertices match
                        c(idx++) = 0.0;
                        c(idx++) = 0.0;
                    }
                    break;
                }
                case DTFConstraint::Type::Pointing: {
                    // Pointing constraint: momentum direction matches flight direction
                    ROOT::Math::PxPyPzEVector total;
                    for (int i : constraint.particle_indices) {
                        double px = params(3 * i + 0);
                        double py = params(3 * i + 1);
                        double pz = params(3 * i + 2);
                        double p = std::sqrt(px * px + py * py + pz * pz);
                        double E = std::sqrt(p * p + m_masses.at(i) * m_masses.at(i));
                        total += ROOT::Math::PxPyPzEVector(px, py, pz, E);
                    }

                    // Get decay vertex (average)
                    ROOT::Math::XYZVector decay_vtx(0, 0, 0);
                    for (const auto& [role, particle] : m_particles) {
                        decay_vtx += ROOT::Math::XYZVector(
                            particle.vx(), particle.vy(), particle.vz());
                    }
                    decay_vtx /= static_cast<double>(m_particles.size());

                    // Flight direction
                    ROOT::Math::XYZVector flight = decay_vtx - constraint.origin;
                    double flight_r = flight.R();

                    if (flight_r > 1e-6 && total.P() > 1e-6) {
                        // Constraint: p_x/p_z = flight_x/flight_z
                        c(idx++) = total.Px() / total.P() - flight.X() / flight_r;
                        c(idx++) = total.Py() / total.P() - flight.Y() / flight_r;
                    } else {
                        c(idx++) = 0.0;
                        c(idx++) = 0.0;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        return c;
    }

auto DTFFitter::compute_jacobian(const Eigen::VectorXd& params) const -> Eigen::MatrixXd {
        int n_constraints = count_constraints();
        int n_params = static_cast<int>(params.size());

        Eigen::MatrixXd D(n_constraints, n_params);
        D.setZero();

        // Numerical differentiation
        double eps = 1e-6;
        Eigen::VectorXd c0 = evaluate_constraints(params);

        for (int j = 0; j < n_params; ++j) {
            Eigen::VectorXd params_plus = params;
            params_plus(j) += eps;
            Eigen::VectorXd c_plus = evaluate_constraints(params_plus);

            for (int i = 0; i < n_constraints; ++i) {
                D(i, j) = (c_plus(i) - c0(i)) / eps;
            }
        }

        return D;
    }
#endif

}  // namespace framework
