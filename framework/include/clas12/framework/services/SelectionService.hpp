#pragma once

#include <fmt/format.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>

namespace clas12::framework {

/**
 * @brief Centralized event selection service
 *
 * Provides:
 * - Named selection criteria
 * - Boolean combinations (AND/OR/NOT)
 * - Selection statistics
 * - Easy reuse across algorithms
 * - Cutflow integration
 *
 * Example:
 * @code
 * auto selection = service<SelectionService>("SelectionService");
 *
 * // Define selections
 * selection->define("has_electron", [](const Event& e) {
 *     return e.passes_decision("ElectronFilter");
 * });
 *
 * selection->define("dis_cut", [](const Event& e) {
 *     auto Q2 = e.retrieve<double>("Q2");
 *     auto W = e.retrieve<double>("W");
 *     return Q2.has_value() && Q2.value() > 1.0 &&
 *            W.has_value() && W.value() > 2.0;
 * });
 *
 * // Use selections
 * if (selection->passes("has_electron", event)) {
 *     // ...
 * }
 *
 * // Combine selections
 * if (selection->passes_all({"has_electron", "dis_cut"}, event)) {
 *     // ...
 * }
 * @endcode
 */
class SelectionService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "SelectionService"; }

    using SelectionFunction = std::function<bool(const Event&)>;

    explicit SelectionService(std::string name = "SelectionService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        print_statistics();
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Define a named selection
     */
    auto define(const std::string& name, SelectionFunction func) -> void {
        m_selections[name] = func;
        m_statistics[name] = {0, 0};  // {passed, total}
    }

    /**
     * @brief Check if event passes selection
     */
    auto passes(const std::string& name, const Event& event) -> bool {
        auto it = m_selections.find(name);
        if (it == m_selections.end()) {
            fmt::print(stderr, "WARNING: Selection '{}' not defined\n", name);
            return false;
        }

        bool result = it->second(event);

        // Update statistics
        m_statistics[name].total++;
        if (result) {
            m_statistics[name].passed++;
        }

        return result;
    }

    /**
     * @brief Check if event passes ALL selections (AND)
     */
    auto passes_all(const std::vector<std::string>& selections,
                    const Event& event) -> bool {
        for (const auto& sel : selections) {
            if (!passes(sel, event)) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Check if event passes ANY selection (OR)
     */
    auto passes_any(const std::vector<std::string>& selections,
                    const Event& event) -> bool {
        for (const auto& sel : selections) {
            if (passes(sel, event)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Check if event does NOT pass selection (NOT)
     */
    auto passes_not(const std::string& name, const Event& event) -> bool {
        return !passes(name, event);
    }

    /**
     * @brief Define composite selection (AND)
     */
    auto define_and(const std::string& name,
                    const std::vector<std::string>& selections) -> void {
        define(name, [this, selections](const Event& event) {
            for (const auto& sel : selections) {
                auto it = m_selections.find(sel);
                if (it == m_selections.end() || !it->second(event)) {
                    return false;
                }
            }
            return true;
        });
    }

    /**
     * @brief Define composite selection (OR)
     */
    auto define_or(const std::string& name,
                   const std::vector<std::string>& selections) -> void {
        define(name, [this, selections](const Event& event) {
            for (const auto& sel : selections) {
                auto it = m_selections.find(sel);
                if (it != m_selections.end() && it->second(event)) {
                    return true;
                }
            }
            return false;
        });
    }

    /**
     * @brief Get efficiency for a selection
     */
    auto get_efficiency(const std::string& name) const -> double {
        auto it = m_statistics.find(name);
        if (it == m_statistics.end() || it->second.total == 0) {
            return 0.0;
        }
        return 100.0 * it->second.passed / it->second.total;
    }

    /**
     * @brief Get statistics for a selection
     */
    auto get_statistics(const std::string& name) const
        -> std::pair<size_t, size_t> {
        auto it = m_statistics.find(name);
        if (it != m_statistics.end()) {
            return {it->second.passed, it->second.total};
        }
        return {0, 0};
    }

    /**
     * @brief Print selection statistics
     */
    auto print_statistics() const -> void {
        if (m_statistics.empty()) {
            return;
        }

        fmt::print("\n");
        fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
        fmt::print("║                  Selection Statistics                          ║\n");
        fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
        fmt::print("\n");

        fmt::print("{:<30} {:>12} {:>12} {:>12}\n",
                   "Selection", "Passed", "Total", "Efficiency");
        fmt::print("{:-<30} {:->12} {:->12} {:->12}\n",
                   "", "", "", "");

        for (const auto& [name, stats] : m_statistics) {
            double efficiency = (stats.total > 0) ? 100.0 * stats.passed / stats.total : 0.0;

            fmt::print("{:<30} {:>12} {:>12} {:>11.2f}%\n",
                       name, stats.passed, stats.total, efficiency);
        }

        fmt::print("\n");
    }

    /**
     * @brief Reset statistics
     */
    auto reset_statistics() -> void {
        for (auto& [name, stats] : m_statistics) {
            stats.passed = 0;
            stats.total = 0;
        }
    }

    /**
     * @brief Get all selection names
     */
    auto get_selection_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        for (const auto& [name, func] : m_selections) {
            names.push_back(name);
        }
        return names;
    }

   private:
    struct SelectionStats {
        size_t passed = 0;
        size_t total = 0;
    };

    std::map<std::string, SelectionFunction> m_selections;
    std::map<std::string, SelectionStats> m_statistics;
};

/**
 * @brief Predefined common selections
 */
namespace CommonSelections {

/**
 * @brief DIS event selection
 */
inline auto DIS_Selection(double min_Q2 = 1.0, double min_W = 2.0) -> SelectionService::SelectionFunction {
    return [min_Q2, min_W](const Event& event) {
        auto Q2 = event.retrieve<double>("Q2");
        auto W = event.retrieve<double>("W");

        return Q2.has_value() && Q2.value() > min_Q2 &&
               W.has_value() && W.value() > min_W;
    };
}

/**
 * @brief Has trigger electron
 */
inline auto HasTriggerElectron() -> SelectionService::SelectionFunction {
    return [](const Event& event) {
        return event.passes_decision("TriggerElectronFilter") ||
               event.passes_decision("ElectronFilter");
    };
}

/**
 * @brief Exclusive event (missing mass near target mass)
 */
inline auto ExclusiveEvent(double target_mass = 0.938, double window = 0.3)
    -> SelectionService::SelectionFunction {
    return [target_mass, window](const Event& event) {
        auto mm = event.retrieve<double>("MissingMass");
        if (!mm.has_value()) return false;

        double diff = std::abs(mm.value() - target_mass);
        return diff < window;
    };
}

}  // namespace CommonSelections

}  // namespace clas12::framework
