#pragma once

#include <functional>
#include <string>
#include <vector>

#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

/**
 * @brief Result of evaluating a cut, including pass/fail status and reason
 *
 * Provides detailed information about cut evaluation for debugging
 * and optimization purposes.
 */
struct CutResult {
    bool passed;
    std::string reason;

    /// Implicit conversion to bool for convenient use in conditionals
    constexpr operator bool() const noexcept { return passed; }

    /// Factory for passed result
    [[nodiscard]] static auto pass(std::string name = "") -> CutResult {
        return {true, std::move(name)};
    }

    /// Factory for failed result
    [[nodiscard]] static auto fail(std::string name = "") -> CutResult {
        return {false, std::move(name)};
    }
};

/**
 * @brief Individual cut with name and predicate
 *
 * Encapsulates a single named cut that can be evaluated.
 * Supports both simple predicates and predicates that capture context.
 */
class Cut {
   public:
    Cut(std::string name, std::function<bool()> predicate)
        : m_name(std::move(name)), m_predicate(std::move(predicate)) {}

    /// Evaluate the cut and return detailed result
    [[nodiscard]] auto evaluate() const -> CutResult {
        return {m_predicate(), m_name};
    }

    /// Get the cut name
    [[nodiscard]] auto name() const -> const std::string& { return m_name; }

   private:
    std::string m_name;
    std::function<bool()> m_predicate;
};

/**
 * @brief Fluent builder for constructing and evaluating cut sequences
 *
 * CutBuilder provides a chainable API for defining cuts and automatically
 * tracks cut efficiency through integration with Cutflow.
 *
 * Usage:
 * @code
 * Cutflow cutflow("Electron Selection");
 * cutflow.add_cut("All Events");
 * cutflow.add_cut("Momentum > 2 GeV");
 * cutflow.add_cut("Theta in range");
 *
 * double momentum = 2.5;
 * double theta = 15.0;
 *
 * auto result = CutBuilder(cutflow)
 *     .cut("All Events", []{ return true; })
 *     .cut("Momentum > 2 GeV", [&]{ return momentum > 2.0; })
 *     .cut("Theta in range", [&]{ return theta > 5.0 && theta < 35.0; })
 *     .evaluate();
 * @endcode
 *
 * C++ Core Guidelines:
 * - F.50: Use a lambda when a function won't do
 * - C.4: Make a function a member only if it needs direct access
 */
class CutBuilder {
   public:
    /// Construct a CutBuilder with a reference to a Cutflow for tracking
    explicit CutBuilder(Cutflow& cutflow) : m_cutflow(cutflow) {}

    /**
     * @brief Add a named cut with a predicate
     * @param name Display name for the cut (must match cutflow)
     * @param predicate Function returning true if cut passes
     * @return Reference to this builder for chaining
     */
    auto cut(std::string name, std::function<bool()> predicate) -> CutBuilder& {
        m_cuts.emplace_back(std::move(name), std::move(predicate));
        return *this;
    }

    /**
     * @brief Add a cut with a value-based predicate
     * @tparam T Type of value to test
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param predicate Function taking value and returning bool
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto cut(std::string name, const T& value,
             std::function<bool(const T&)> predicate) -> CutBuilder& {
        return cut(std::move(name), [&value, pred = std::move(predicate)]() {
            return pred(value);
        });
    }

    /**
     * @brief Require value to be greater than threshold
     * @tparam T Comparable type
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param threshold Minimum value (exclusive)
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto require_gt(std::string name, const T& value, T threshold) -> CutBuilder& {
        return cut(std::move(name), [&value, threshold]() {
            return value > threshold;
        });
    }

    /**
     * @brief Require value to be greater than or equal to threshold
     * @tparam T Comparable type
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param threshold Minimum value (inclusive)
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto require_gte(std::string name, const T& value, T threshold) -> CutBuilder& {
        return cut(std::move(name), [&value, threshold]() {
            return value >= threshold;
        });
    }

    /**
     * @brief Require value to be less than threshold
     * @tparam T Comparable type
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param threshold Maximum value (exclusive)
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto require_lt(std::string name, const T& value, T threshold) -> CutBuilder& {
        return cut(std::move(name), [&value, threshold]() {
            return value < threshold;
        });
    }

    /**
     * @brief Require value to be less than or equal to threshold
     * @tparam T Comparable type
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param threshold Maximum value (inclusive)
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto require_lte(std::string name, const T& value, T threshold) -> CutBuilder& {
        return cut(std::move(name), [&value, threshold]() {
            return value <= threshold;
        });
    }

    /**
     * @brief Require value to be within range [min, max]
     * @tparam T Comparable type
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param min Minimum value (inclusive)
     * @param max Maximum value (inclusive)
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto require_in_range(std::string name, const T& value, T min, T max) -> CutBuilder& {
        return cut(std::move(name), [&value, min, max]() {
            return value >= min && value <= max;
        });
    }

    /**
     * @brief Require value to equal expected value
     * @tparam T Equality-comparable type
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param expected Expected value
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto require_eq(std::string name, const T& value, T expected) -> CutBuilder& {
        return cut(std::move(name), [&value, expected]() {
            return value == expected;
        });
    }

    /**
     * @brief Require value to not equal a specific value
     * @tparam T Equality-comparable type
     * @param name Display name for the cut
     * @param value Reference to value to test
     * @param excluded Value that should not match
     * @return Reference to this builder for chaining
     */
    template <typename T>
    auto require_ne(std::string name, const T& value, T excluded) -> CutBuilder& {
        return cut(std::move(name), [&value, excluded]() {
            return value != excluded;
        });
    }

    auto require(std::string name, bool pass) -> CutBuilder& {
        return cut(std::move(name), [pass]() { return pass; });
    }

    /**
     * @brief Evaluate all cuts in sequence, updating cutflow
     * @return true if all cuts pass, false on first failure
     *
     * Cuts are evaluated in order. On each passing cut, the cutflow
     * counter is incremented. Evaluation stops at the first failing cut.
     */
    [[nodiscard]] auto evaluate() -> bool {
        for (const auto& c : m_cuts) {
            auto result = c.evaluate();
            if (!result) {
                return false;
            }
            m_cutflow.pass(c.name());
        }
        return true;
    }

    /**
     * @brief Evaluate all cuts and return StatusCode
     * @return SUCCESS if all cuts pass, FILTERED on first failure
     *
     * Convenience method for use in algorithm execute() methods.
     */
    [[nodiscard]] auto apply() -> StatusCode {
        return evaluate() ? StatusCode::SUCCESS() : StatusCode::FILTERED();
    }

    /**
     * @brief Get the number of cuts in the builder
     * @return Number of cuts added
     */
    [[nodiscard]] auto size() const -> std::size_t { return m_cuts.size(); }

    /**
     * @brief Check if any cuts have been added
     * @return true if no cuts have been added
     */
    [[nodiscard]] auto empty() const -> bool { return m_cuts.empty(); }

    /**
     * @brief Get names of all cuts in order
     * @return Vector of cut names
     */
    [[nodiscard]] auto cut_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_cuts.size());
        for (const auto& c : m_cuts) {
            names.push_back(c.name());
        }
        return names;
    }

   private:
    Cutflow& m_cutflow;
    std::vector<Cut> m_cuts;
};

/**
 * @brief Factory function to create a CutBuilder
 * @param cutflow Reference to cutflow for tracking
 * @return New CutBuilder instance
 *
 * Provides a more readable entry point:
 * @code
 * auto result = cuts(cutflow)
 *     .cut("All Events", []{ return true; })
 *     .require_gt("Momentum", p, 2.0)
 *     .apply();
 * @endcode
 */
inline auto cuts(Cutflow& cutflow) -> CutBuilder {
    return CutBuilder(cutflow);
}

}  // namespace clas12::framework
