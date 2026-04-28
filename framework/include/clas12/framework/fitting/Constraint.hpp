#pragma once

#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <Math/Vector4D.h>

namespace framework::fitting {

/**
 * @brief Types of kinematic constraints
 */
enum class ConstraintType {
    MassConstraint,       ///< Fix invariant mass of particle combination
    VertexConstraint,     ///< Common vertex for particles
    MomentumConstraint,   ///< 3-momentum conservation
    EnergyConstraint,     ///< Energy conservation
    MissingMassConstraint ///< Fix missing mass
};

/**
 * @brief Base class for kinematic constraints
 *
 * Constraints define relationships between particle momenta that must
 * be satisfied. Each constraint provides:
 * - evaluate(): The constraint value (0 when satisfied)
 * - gradient(): Partial derivatives w.r.t. particle parameters
 *
 * The Lagrange multiplier method uses these to modify particle momenta
 * to satisfy all constraints while minimizing chi-squared.
 */
class Constraint {
   public:
    virtual ~Constraint() = default;

    /**
     * @brief Evaluate the constraint
     *
     * Returns a value that should be zero when the constraint is satisfied.
     * For a mass constraint: M_target^2 - (sum of 4-momenta)^2
     *
     * @param momenta Current particle 4-momenta
     * @return Constraint value (0 when satisfied)
     */
    virtual auto evaluate(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const -> double = 0;

    /**
     * @brief Get gradient (partial derivatives)
     *
     * Returns partial derivatives of the constraint w.r.t. each parameter.
     * For N particles with 4 parameters each (px, py, pz, E), returns
     * a vector of size 4*N.
     *
     * @param momenta Current particle 4-momenta
     * @return Vector of partial derivatives
     */
    virtual auto gradient(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const
        -> std::vector<double> = 0;

    /**
     * @brief Get the constraint type
     */
    [[nodiscard]] virtual auto type() const -> ConstraintType = 0;

    /**
     * @brief Get constraint name for logging/debugging
     */
    [[nodiscard]] virtual auto name() const -> std::string = 0;

    /**
     * @brief Get indices of particles involved in this constraint
     */
    [[nodiscard]] virtual auto particle_indices() const -> std::vector<int> = 0;
};

/**
 * @brief Mass constraint: Fix invariant mass of particle combination
 *
 * Constrains the invariant mass of particles [start_idx, end_idx) to
 * equal the target mass.
 *
 * Constraint: M^2 - (sum E)^2 + |sum p|^2 = 0
 */
class MassConstraint : public Constraint {
   public:
    /**
     * @brief Construct mass constraint for particles in range
     *
     * @param target_mass Target invariant mass [GeV/c^2]
     * @param indices Indices of particles to combine
     */
    MassConstraint(double target_mass, std::vector<int> indices)
        : m_target_mass(target_mass)
        , m_indices(std::move(indices)) {}

    /**
     * @brief Construct mass constraint for two particles
     */
    MassConstraint(double target_mass, int idx1, int idx2)
        : MassConstraint(target_mass, std::vector<int>{idx1, idx2}) {}

    auto evaluate(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const -> double override {
        // Sum 4-momenta of constrained particles
        ROOT::Math::PxPyPzEVector sum(0, 0, 0, 0);
        for (int idx : m_indices) {
            if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                sum += momenta[idx];
            }
        }

        // Constraint: M_target^2 - M_measured^2 = 0
        double m2_measured = sum.M2();
        double m2_target = m_target_mass * m_target_mass;

        return m2_target - m2_measured;
    }

    auto gradient(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const
        -> std::vector<double> override {
        // Gradient has 4 components per particle (px, py, pz, E)
        std::vector<double> grad(momenta.size() * 4, 0.0);

        // Sum 4-momenta
        ROOT::Math::PxPyPzEVector sum(0, 0, 0, 0);
        for (int idx : m_indices) {
            if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                sum += momenta[idx];
            }
        }

        // d(M^2)/d(px_i) = -2*sum_px for particles in constraint
        // d(M^2)/d(py_i) = -2*sum_py
        // d(M^2)/d(pz_i) = -2*sum_pz
        // d(M^2)/d(E_i)  = +2*sum_E
        // But we're computing d(target - measured)/d(params) = -d(measured)/d(params)
        for (int idx : m_indices) {
            if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                grad[4 * idx + 0] = 2.0 * sum.Px();    // d/dpx
                grad[4 * idx + 1] = 2.0 * sum.Py();    // d/dpy
                grad[4 * idx + 2] = 2.0 * sum.Pz();    // d/dpz
                grad[4 * idx + 3] = -2.0 * sum.E();    // d/dE
            }
        }

        return grad;
    }

    [[nodiscard]] auto type() const -> ConstraintType override {
        return ConstraintType::MassConstraint;
    }

    [[nodiscard]] auto name() const -> std::string override {
        return "MassConstraint(M=" + std::to_string(m_target_mass) + " GeV)";
    }

    [[nodiscard]] auto particle_indices() const -> std::vector<int> override {
        return m_indices;
    }

    [[nodiscard]] auto target_mass() const -> double { return m_target_mass; }

   private:
    double m_target_mass;
    std::vector<int> m_indices;
};

/**
 * @brief Momentum conservation constraint
 *
 * Constrains the sum of final state 3-momenta to equal the initial state.
 * Creates 3 constraints (px, py, pz conservation).
 */
class MomentumConstraint : public Constraint {
   public:
    /**
     * @brief Construct momentum conservation constraint
     *
     * @param initial Initial 4-momentum (e.g., beam + target)
     * @param final_indices Indices of final state particles
     */
    MomentumConstraint(const ROOT::Math::PxPyPzEVector& initial,
                       std::vector<int> final_indices,
                       int component)  // 0=px, 1=py, 2=pz
        : m_initial(initial)
        , m_final_indices(std::move(final_indices))
        , m_component(component) {}

    auto evaluate(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const -> double override {
        double p_initial = 0.0;
        double p_final = 0.0;

        switch (m_component) {
            case 0:
                p_initial = m_initial.Px();
                for (int idx : m_final_indices) {
                    if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                        p_final += momenta[idx].Px();
                    }
                }
                break;
            case 1:
                p_initial = m_initial.Py();
                for (int idx : m_final_indices) {
                    if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                        p_final += momenta[idx].Py();
                    }
                }
                break;
            case 2:
                p_initial = m_initial.Pz();
                for (int idx : m_final_indices) {
                    if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                        p_final += momenta[idx].Pz();
                    }
                }
                break;
        }

        return p_initial - p_final;
    }

    auto gradient(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const
        -> std::vector<double> override {
        std::vector<double> grad(momenta.size() * 4, 0.0);

        // d(p_initial - p_final)/d(p_i) = -1 for particles in final state
        for (int idx : m_final_indices) {
            if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                grad[4 * idx + m_component] = -1.0;
            }
        }

        return grad;
    }

    [[nodiscard]] auto type() const -> ConstraintType override {
        return ConstraintType::MomentumConstraint;
    }

    [[nodiscard]] auto name() const -> std::string override {
        static const char* comp_names[] = {"Px", "Py", "Pz"};
        return std::string("MomentumConstraint(") + comp_names[m_component] + ")";
    }

    [[nodiscard]] auto particle_indices() const -> std::vector<int> override {
        return m_final_indices;
    }

   private:
    ROOT::Math::PxPyPzEVector m_initial;
    std::vector<int> m_final_indices;
    int m_component;  // 0=px, 1=py, 2=pz
};

/**
 * @brief Energy conservation constraint
 */
class EnergyConstraint : public Constraint {
   public:
    EnergyConstraint(const ROOT::Math::PxPyPzEVector& initial,
                     std::vector<int> final_indices)
        : m_initial(initial)
        , m_final_indices(std::move(final_indices)) {}

    auto evaluate(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const -> double override {
        double E_final = 0.0;
        for (int idx : m_final_indices) {
            if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                E_final += momenta[idx].E();
            }
        }
        return m_initial.E() - E_final;
    }

    auto gradient(const std::vector<ROOT::Math::PxPyPzEVector>& momenta) const
        -> std::vector<double> override {
        std::vector<double> grad(momenta.size() * 4, 0.0);

        for (int idx : m_final_indices) {
            if (idx >= 0 && static_cast<size_t>(idx) < momenta.size()) {
                grad[4 * idx + 3] = -1.0;  // d/dE
            }
        }

        return grad;
    }

    [[nodiscard]] auto type() const -> ConstraintType override {
        return ConstraintType::EnergyConstraint;
    }

    [[nodiscard]] auto name() const -> std::string override {
        return "EnergyConstraint";
    }

    [[nodiscard]] auto particle_indices() const -> std::vector<int> override {
        return m_final_indices;
    }

   private:
    ROOT::Math::PxPyPzEVector m_initial;
    std::vector<int> m_final_indices;
};

}  // namespace framework::fitting
