#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/DecayDescriptor.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/fitting/FitResult.hpp>
#include <clas12/framework/services/LoggingService.hpp>

#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TMath.h>

// DecayTreeFitter requires Eigen for matrix operations
#if __has_include(<Eigen/Dense>)
#define CLAS12_HAS_EIGEN 1
#include <Eigen/Dense>
#else
#define CLAS12_HAS_EIGEN 0
#endif

namespace clas12::framework {

// Forward declaration
class DTFFitter;

/**
 * @brief Constraint types for DecayTreeFitter
 */
struct DTFConstraint {
    enum class Type {
        Mass,           ///< Fix invariant mass of particle combination
        Vertex,         ///< Daughters share common vertex
        OriginVertex,   ///< Mother originates from specific vertex
        FourMomentum,   ///< Four-momentum conservation
        Pointing        ///< Mother momentum points to origin
    };

    Type type;
    double value = 0.0;                    ///< For mass constraint [GeV/c^2]
    std::vector<int> particle_indices;     ///< Particles involved
    ROOT::Math::XYZVector origin{0, 0, 0}; ///< For origin/pointing constraints
    std::string resonance_role;            ///< For intermediate resonance mass

    DTFConstraint(Type t) : type(t) {}

    static auto mass(double m, const std::vector<int>& indices) -> DTFConstraint {
        DTFConstraint c(Type::Mass);
        c.value = m;
        c.particle_indices = indices;
        return c;
    }

    static auto vertex(const std::vector<int>& indices) -> DTFConstraint {
        DTFConstraint c(Type::Vertex);
        c.particle_indices = indices;
        return c;
    }

    static auto pointing(const ROOT::Math::XYZVector& origin,
                        const std::vector<int>& indices) -> DTFConstraint {
        DTFConstraint c(Type::Pointing);
        c.origin = origin;
        c.particle_indices = indices;
        return c;
    }
};

/**
 * @brief Result of DecayTreeFitter
 */
struct DTFResult {
    bool converged = false;               ///< Fit converged
    double chi2 = -1.0;                   ///< Fit chi-squared
    int ndf = 0;                          ///< Degrees of freedom
    double probability = 0.0;             ///< Fit probability
    int n_iterations = 0;                 ///< Number of iterations
    std::string error_message;            ///< Error message if failed

    /// Fitted momenta by particle index
    std::map<int, ROOT::Math::PxPyPzEVector> fitted_momenta;

    /// Fitted vertices by particle index (for decay vertices)
    std::map<int, ROOT::Math::XYZVector> fitted_vertices;

    /// Constraint residuals
    std::vector<double> constraint_residuals;

    /// Pulls for each parameter
    std::vector<double> pulls;

    // ========================================================================
    // Quality Methods
    // ========================================================================

    /**
     * @brief Check if fit is good
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
     * @brief Get fitted momentum for a particle
     */
    [[nodiscard]] auto get_momentum(int particle_idx) const
        -> ROOT::Math::PxPyPzEVector {
        auto it = fitted_momenta.find(particle_idx);
        if (it != fitted_momenta.end()) {
            return it->second;
        }
        return ROOT::Math::PxPyPzEVector(0, 0, 0, 0);
    }

    /**
     * @brief Get invariant mass of particle combination
     */
    [[nodiscard]] auto get_invariant_mass(const std::vector<int>& indices) const
        -> double {
        ROOT::Math::PxPyPzEVector total;
        for (int idx : indices) {
            auto it = fitted_momenta.find(idx);
            if (it != fitted_momenta.end()) {
                total += it->second;
            }
        }
        return total.M();
    }

    /**
     * @brief Get fitted vertex for mother particle
     */
    [[nodiscard]] auto get_vertex(int particle_idx = 0) const
        -> ROOT::Math::XYZVector {
        auto it = fitted_vertices.find(particle_idx);
        if (it != fitted_vertices.end()) {
            return it->second;
        }
        return ROOT::Math::XYZVector(0, 0, 0);
    }

    /**
     * @brief Get total fitted 4-momentum
     */
    [[nodiscard]] auto total_momentum() const -> ROOT::Math::PxPyPzEVector {
        ROOT::Math::PxPyPzEVector total;
        for (const auto& [idx, p4] : fitted_momenta) {
            total += p4;
        }
        return total;
    }

    /**
     * @brief Get fitted mother mass
     */
    [[nodiscard]] auto fitted_mass() const -> double {
        return total_momentum().M();
    }
};

/**
 * @brief Fitter instance for a specific decay topology
 *
 * DTFFitter performs a global kinematic fit of the entire decay tree,
 * propagating constraints through the decay chain.
 */
class DTFFitter {
   public:
    /**
     * @brief Construct fitter from decay descriptor
     */
    explicit DTFFitter(const DecayDescriptor& descriptor)
        : m_descriptor(descriptor) {}

    /**
     * @brief Add particle to the fit
     *
     * @param role Role in decay (e.g., "p+", "pi-")
     * @param particle Reconstructed particle
     * @return Reference to this fitter for chaining
     */
    auto add_particle(const std::string& role, const clas12::Particle& particle)
        -> DTFFitter& {
        int idx = static_cast<int>(m_particles.size());
        m_particles[role] = particle;
        m_role_to_index[role] = idx;
        m_index_to_role[idx] = role;

        // Store initial momentum
        m_initial_momenta[idx] = particle.PxPyPzEVector();
        m_masses[idx] = particle.mass();

        return *this;
    }

    /**
     * @brief Add mass constraint for mother particle
     */
    auto constrain_mass(double mass) -> DTFFitter& {
        // Constrain all particles
        std::vector<int> indices;
        for (const auto& [role, idx] : m_role_to_index) {
            indices.push_back(idx);
        }
        m_constraints.push_back(DTFConstraint::mass(mass, indices));
        return *this;
    }

    /**
     * @brief Add mass constraint for intermediate resonance
     */
    auto constrain_mass(const std::string& resonance_role, double mass)
        -> DTFFitter& {
        // TODO: Handle intermediate resonances
        // For now, treat as final state mass constraint
        auto constraint = DTFConstraint(DTFConstraint::Type::Mass);
        constraint.value = mass;
        constraint.resonance_role = resonance_role;
        m_constraints.push_back(constraint);
        return *this;
    }

    /**
     * @brief Add vertex constraint (daughters share common vertex)
     */
    auto constrain_vertex() -> DTFFitter& {
        std::vector<int> indices;
        for (const auto& [role, idx] : m_role_to_index) {
            indices.push_back(idx);
        }
        m_constraints.push_back(DTFConstraint::vertex(indices));
        return *this;
    }

    /**
     * @brief Add pointing constraint (mother points to origin)
     */
    auto constrain_pointing(const ROOT::Math::XYZVector& origin) -> DTFFitter& {
        std::vector<int> indices;
        for (const auto& [role, idx] : m_role_to_index) {
            indices.push_back(idx);
        }
        m_constraints.push_back(DTFConstraint::pointing(origin, indices));
        m_origin_vertex = origin;
        return *this;
    }

    /**
     * @brief Set maximum iterations
     */
    auto set_max_iterations(int n) -> DTFFitter& {
        m_max_iterations = n;
        return *this;
    }

    /**
     * @brief Set convergence tolerance
     */
    auto set_tolerance(double tol) -> DTFFitter& {
        m_tolerance = tol;
        return *this;
    }

    /**
     * @brief Perform the fit
     */
    auto fit() -> DTFResult;

   private:
    DecayDescriptor m_descriptor;
    std::map<std::string, clas12::Particle> m_particles;
    std::map<std::string, int> m_role_to_index;
    std::map<int, std::string> m_index_to_role;
    std::map<int, ROOT::Math::PxPyPzEVector> m_initial_momenta;
    std::map<int, double> m_masses;
    std::vector<DTFConstraint> m_constraints;
    ROOT::Math::XYZVector m_origin_vertex{0, 0, 0};

    int m_max_iterations = 100;
    double m_tolerance = 1e-6;

    /**
     * @brief Count total number of constraint equations
     */
    auto count_constraints() const -> int {
        int n = 0;
        for (const auto& c : m_constraints) {
            switch (c.type) {
                case DTFConstraint::Type::Mass:
                    n += 1;  // One mass constraint
                    break;
                case DTFConstraint::Type::Vertex:
                    // 2*(n-1) constraints for n particles sharing vertex
                    n += 2 * (static_cast<int>(c.particle_indices.size()) - 1);
                    break;
                case DTFConstraint::Type::Pointing:
                    n += 2;  // Two angular constraints (theta_x, theta_y)
                    break;
                default:
                    break;
            }
        }
        return n;
    }

#if CLAS12_HAS_EIGEN
    /**
     * @brief Evaluate constraint functions
     */
    auto evaluate_constraints(const Eigen::VectorXd& params) const
        -> Eigen::VectorXd;

    /**
     * @brief Compute Jacobian matrix of constraints
     */
    auto compute_jacobian(const Eigen::VectorXd& params) const -> Eigen::MatrixXd;
#endif  // CLAS12_HAS_EIGEN
};

/**
 * @brief Service for LHCb-style DecayTreeFitter
 *
 * DecayTreeFitterService provides global topological fits of decay chains,
 * propagating constraints through the entire decay tree. This enables:
 *
 * - Mass constraints on mother and intermediate particles
 * - Vertex constraints (common decay vertex)
 * - Pointing constraints (momentum direction matches flight)
 * - Improved resolution on all kinematic quantities
 *
 * Example usage:
 * @code
 * // Create fitter for Lambda -> p+ pi-
 * auto result = m_dtf_svc->create_fitter(DecayDescriptor::parse("Lambda0 -> p+ pi-"))
 *     ->add_particle("p+", proton)
 *     ->add_particle("pi-", pion)
 *     ->constrain_mass(1.115683)
 *     ->constrain_vertex()
 *     ->constrain_pointing(primary_vertex)
 *     ->fit();
 *
 * if (result.is_good(0.01)) {
 *     auto p_fitted = result.get_momentum(0);
 *     auto pi_fitted = result.get_momentum(1);
 *     // Use fitted momenta for analysis
 * }
 * @endcode
 */
class DecayTreeFitterService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "DecayTreeFitterService"; }

    explicit DecayTreeFitterService(std::string name = "DecayTreeFitterService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "DecayTreeFitterService initialized");
        Log::info(this->name(), "  Max iterations: {}", m_max_iterations.get());
        Log::info(this->name(), "  Convergence tolerance: {}", m_tolerance.get());
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "DecayTreeFitterService finalized");
        Log::info(this->name(), "  Total fits: {}", m_total_fits);
        Log::info(this->name(), "  Successful fits: {}", m_successful_fits);
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Maximum fit iterations
    Property<int> m_max_iterations{"max_iterations", 100, "Maximum iterations"};

    /// Convergence tolerance
    Property<double> m_tolerance{"convergence_tolerance", 1e-6,
                                 "Convergence threshold"};

    /// Apply vertex constraints by default
    Property<bool> m_use_vertex_constraints{"use_vertex_constraints", true,
                                            "Apply vertex constraints by default"};

    // ========================================================================
    // Fitter Creation
    // ========================================================================

    /**
     * @brief Create a DTF fitter from decay descriptor
     *
     * @param descriptor Parsed decay descriptor
     * @return Unique pointer to DTFFitter instance
     */
    auto create_fitter(const DecayDescriptor& descriptor) -> std::unique_ptr<DTFFitter> {
        auto fitter = std::make_unique<DTFFitter>(descriptor);
        fitter->set_max_iterations(m_max_iterations.get());
        fitter->set_tolerance(m_tolerance.get());
        return fitter;
    }

    // ========================================================================
    // Convenience Methods
    // ========================================================================

    /**
     * @brief Fit two-body decay with mass constraint
     *
     * @param daughter1 First decay product
     * @param daughter2 Second decay product
     * @param mother_mass Target mother mass [GeV/c^2]
     * @param origin Origin vertex (for pointing constraint)
     * @return DTFResult with fitted quantities
     */
    auto fit_two_body(const clas12::Particle& daughter1,
                      const clas12::Particle& daughter2,
                      double mother_mass,
                      const ROOT::Math::XYZVector& origin = {0, 0, 0}) -> DTFResult {
        ++m_total_fits;

        auto fitter = std::make_unique<DTFFitter>(DecayDescriptor());
        fitter->add_particle("d1", daughter1)
              .add_particle("d2", daughter2)
              .constrain_mass(mother_mass)
              .set_max_iterations(m_max_iterations.get())
              .set_tolerance(m_tolerance.get());

        if (m_use_vertex_constraints.get()) {
            fitter->constrain_vertex();
        }

        if (origin.R() > 1e-6) {
            fitter->constrain_pointing(origin);
        }

        auto result = fitter->fit();

        if (result.converged) {
            ++m_successful_fits;
        }

        return result;
    }

    /**
     * @brief Fit two-body decay with vertex/pointing constraints only (no mass constraint)
     *
     * This method applies vertex and pointing constraints to improve momentum resolution
     * but does NOT constrain the invariant mass, allowing the fitted mass to be measured.
     * Use this when you want to study mass distributions with improved resolution.
     *
     * @param daughter1 First decay product
     * @param daughter2 Second decay product
     * @param origin Origin vertex (for pointing constraint)
     * @return DTFResult with fitted quantities
     */
    auto fit_two_body_vertex_only(const clas12::Particle& daughter1,
                                  const clas12::Particle& daughter2,
                                  const ROOT::Math::XYZVector& origin = {0, 0, 0}) -> DTFResult {
        ++m_total_fits;

        auto fitter = std::make_unique<DTFFitter>(DecayDescriptor());
        fitter->add_particle("d1", daughter1)
              .add_particle("d2", daughter2)
              .set_max_iterations(m_max_iterations.get())
              .set_tolerance(m_tolerance.get());

        if (m_use_vertex_constraints.get()) {
            fitter->constrain_vertex();
        }

        if (origin.R() > 1e-6) {
            fitter->constrain_pointing(origin);
        }

        auto result = fitter->fit();

        if (result.converged) {
            ++m_successful_fits;
        }

        return result;
    }

    /**
     * @brief Fit with mass and pointing constraints
     *
     * Common for displaced decays like Lambda -> p pi
     *
     * @param daughters Decay products
     * @param mother_mass Target mother mass
     * @param origin Origin vertex (e.g., primary vertex)
     * @return DTFResult
     */
    auto fit_with_pointing(const std::vector<clas12::Particle>& daughters,
                           double mother_mass,
                           const ROOT::Math::XYZVector& origin) -> DTFResult {
        ++m_total_fits;

        auto fitter = std::make_unique<DTFFitter>(DecayDescriptor());

        for (size_t i = 0; i < daughters.size(); ++i) {
            fitter->add_particle("d" + std::to_string(i), daughters[i]);
        }

        fitter->constrain_mass(mother_mass)
              .constrain_vertex()
              .constrain_pointing(origin)
              .set_max_iterations(m_max_iterations.get())
              .set_tolerance(m_tolerance.get());

        auto result = fitter->fit();

        if (result.converged) {
            ++m_successful_fits;
        }

        return result;
    }

    /**
     * @brief Fit with explicit constraint list
     *
     * @param particles Map of role to particle
     * @param constraints List of constraints
     * @return DTFResult
     */
    auto fit_with_constraints(const std::map<std::string, clas12::Particle>& particles,
                              const std::vector<DTFConstraint>& constraints) -> DTFResult {
        ++m_total_fits;

        auto fitter = std::make_unique<DTFFitter>(DecayDescriptor());

        for (const auto& [role, particle] : particles) {
            fitter->add_particle(role, particle);
        }

        fitter->set_max_iterations(m_max_iterations.get())
              .set_tolerance(m_tolerance.get());

        // Add constraints
        for (const auto& c : constraints) {
            switch (c.type) {
                case DTFConstraint::Type::Mass:
                    fitter->constrain_mass(c.value);
                    break;
                case DTFConstraint::Type::Vertex:
                    fitter->constrain_vertex();
                    break;
                case DTFConstraint::Type::Pointing:
                    fitter->constrain_pointing(c.origin);
                    break;
                default:
                    break;
            }
        }

        auto result = fitter->fit();

        if (result.converged) {
            ++m_successful_fits;
        }

        return result;
    }

   private:
    mutable size_t m_total_fits = 0;
    mutable size_t m_successful_fits = 0;
};

}  // namespace clas12::framework
