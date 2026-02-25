#pragma once

#include <fmt/format.h>
#include <atomic>
#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

/**
 * @brief Cutflow manager for tracking event selection efficiency
 */
class Cutflow {
   public:
    explicit Cutflow(std::string name) : m_name(std::move(name)) {}

    /// Add a cut to the cutflow
    auto add_cut(const std::string& cut_name) -> void {
        m_cuts.push_back(cut_name);
        m_counters[cut_name] = 0;
    }

    /// Number of cuts
    [[nodiscard]] auto size() const -> std::size_t { return m_cuts.size(); }

    [[nodiscard]] auto num_cuts() const -> std::size_t { return size(); }

    /// Get all cut names in order
    [[nodiscard]] auto cuts() const -> const std::vector<std::string>& { return m_cuts; }

    [[nodiscard]] auto get_cut_names() const -> const std::vector<std::string>& { return m_cuts; }

    /// Increment counter for a cut (thread-safe)
    auto pass(const std::string& cut_name) -> void {
        if (auto it = m_counters.find(cut_name); it != m_counters.end()) {
            ++it->second;
        }
    }

    /// Increment counter for a cut (thread-safe)
    auto pass(const std::string_view cut_name) -> void {
        if (auto it = m_counters.find(cut_name.data()); it != m_counters.end()) {
            ++it->second;
        }
    }

    /// Increment counter for a cut (thread-safe) - overload for string literals
    auto pass(const char* cut_name) -> void {
        if (auto it = m_counters.find(cut_name); it != m_counters.end()) {
            ++it->second;
        }
    }

    /// Get count for a specific cut
    [[nodiscard]] auto count(const std::string& cut_name) const -> std::size_t {
        if (auto it = m_counters.find(cut_name); it != m_counters.end()) {
            return it->second.load();
        }
        return 0;
    }

    [[nodiscard]] auto get_count(const std::string& cut_name) const -> std::size_t {
        return count(cut_name);
    }

    /// Get initial/final counts and efficiency
    [[nodiscard]] auto initial_count() const -> std::size_t {
        return m_cuts.empty() ? 0 : count(m_cuts.front());
    }

    [[nodiscard]] auto final_count() const -> std::size_t {
        return m_cuts.empty() ? 0 : count(m_cuts.back());
    }

    [[nodiscard]] auto efficiency() const -> double {
        auto init = initial_count();
        return init > 0 ? static_cast<double>(final_count()) / static_cast<double>(init) : 0.0;
    }

    /// Get cutflow name
    [[nodiscard]] auto name() const -> const std::string& { return m_name; }

    /// Print cutflow table (uses Log singleton if available, otherwise fmt::print)
    auto print() const -> void {
        if (m_cuts.empty()) return;

        const std::size_t init = count(m_cuts.front());
        std::size_t prev = init;

        // Build cut data for table
        std::vector<std::tuple<std::string, std::size_t, double, double>> cut_data;
        cut_data.reserve(m_cuts.size());

        for (std::size_t i = 0; i < m_cuts.size(); ++i) {
            const auto& cut = m_cuts[i];
            const std::size_t cnt = count(cut);
            const double abs_eff = init > 0 ? 100.0 * cnt / init : 0.0;
            const double rel_eff = prev > 0 ? 100.0 * cnt / prev : 0.0;
            cut_data.emplace_back(cut, cnt, abs_eff, rel_eff);
            prev = cnt;
        }

        // Use Log's table printer if available
        if (Log::is_available()) {
            Log::print_cutflow_table(m_name, cut_data, efficiency() * 100.0);
        } else {
            // Fallback to simple text output
            fmt::print("\n");
            fmt::print("+------------------------------------------------------------------------+\n");
            fmt::print("|{:^72}|\n", "Cutflow: " + m_name);
            fmt::print("+-----------------------------------+------------+----------+----------+\n");
            fmt::print("| {:<33} | {:>10} | {:>8} | {:>8} |\n", "Cut", "Events", "Abs Eff", "Rel Eff");
            fmt::print("+-----------------------------------+------------+----------+----------+\n");

            for (const auto& [cut_name, cnt, abs_eff, rel_eff] : cut_data) {
                fmt::print("| {:<33} | {:>10} | {:>6.1f}%  | {:>6.1f}%  |\n",
                          cut_name, cnt, abs_eff, rel_eff);
            }

            fmt::print("+-----------------------------------+------------+----------+----------+\n");
            fmt::print("| {:<70} |\n", fmt::format("Overall efficiency: {:.2f}%", efficiency() * 100.0));
            fmt::print("+------------------------------------------------------------------------+\n");
        }
    }

   private:
    std::string m_name;
    std::vector<std::string> m_cuts;
    std::map<std::string, std::atomic<std::size_t>> m_counters;
};

}  // namespace clas12::framework
