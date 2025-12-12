#pragma once

#include <atomic>
#include <cstddef>
#include <fmt/format.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "Framework/Services/LoggingService.hpp"

namespace Framework {

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

    /// Set logger for structured output (optional)
    auto set_logger(LoggingService* log) -> void { m_log = log; }

    /// Print cutflow table (uses Log singleton if available, otherwise fmt::print)
    auto print() const -> void {
        if (m_cuts.empty()) return;

        const std::size_t init = count(m_cuts.front());
        std::size_t prev = init;

        // Header
        if (Log::is_available() || m_log) {
            log_info("────────────────────────────────────────────────────────────");
            log_info("Cutflow: {}", m_name);
            log_info("────────────────────────────────────────────────────────────");
        } else {
            fmt::print("\n");
            fmt::print("════════════════════════════════════════════════════════════\n");
            fmt::print("  Cutflow: {}\n", m_name);
            fmt::print("════════════════════════════════════════════════════════════\n");
            fmt::print("{:<35} {:>10} {:>8} {:>8}\n", "Cut", "Events", "Abs Eff", "Rel Eff");
            fmt::print("────────────────────────────────────────────────────────────\n");
        }

        // Cuts
        for (std::size_t i = 0; i < m_cuts.size(); ++i) {
            const auto& cut = m_cuts[i];
            const std::size_t cnt = count(cut);
            const double abs_eff = init > 0 ? 100.0 * cnt / init : 0.0;
            const double rel_eff = prev > 0 ? 100.0 * cnt / prev : 0.0;

            if (Log::is_available() || m_log) {
                if (i == 0) {
                    log_info("{:<35} {:>10}   Abs: 100.0%   Rel: 100.0%", cut, cnt);
                } else {
                    log_info("{:<35} {:>10}   Abs: {:>5.1f}%   Rel: {:>5.1f}%", cut, cnt, abs_eff, rel_eff);
                }
            } else {
                if (i == 0) {
                    fmt::print("{:<35} {:>10} {:>8} {:>8}\n", cut, cnt, "100.0%", "100.0%");
                } else {
                    fmt::print("{:<35} {:>10} {:>7.1f}% {:>7.1f}%\n", cut, cnt, abs_eff, rel_eff);
                }
            }
            prev = cnt;
        }

        // Footer
        if (Log::is_available() || m_log) {
            log_info("────────────────────────────────────────────────────────────");
            log_info("Overall efficiency: {:.2f}%", efficiency() * 100.0);
        } else {
            fmt::print("════════════════════════════════════════════════════════════\n");
            fmt::print("  Overall efficiency: {:.2f}%\n\n", efficiency() * 100.0);
        }
    }

   private:
    std::string m_name;
    std::vector<std::string> m_cuts;
    std::map<std::string, std::atomic<std::size_t>> m_counters;
    LoggingService* m_log = nullptr;

    template <typename... Args>
    auto log_info(fmt::format_string<Args...> fmt, Args&&... args) const -> void {
        if (m_log) {
            m_log->info(m_name, fmt, std::forward<Args>(args)...);
        } else if (Log::is_available()) {
            Log::info(m_name, fmt, std::forward<Args>(args)...);
        }
    }
};


/**
 * @brief Extended cutflow with histogram filling capabilities
 */
template <typename Particle>
class CutflowWithHistograms : public Cutflow {
   public:
    using HistogramFiller = std::function<void(const Particle&, const std::string&)>;

    explicit CutflowWithHistograms(std::string name, HistogramFiller filler = nullptr)
        : Cutflow(std::move(name)), m_filler(std::move(filler)) {}

    /// Pass a cut and optionally fill histograms
    auto pass(const std::string& cut_name, const Particle& particle, bool fill = true) -> void {
        Cutflow::pass(cut_name);
        if (fill && m_filler) {
            m_filler(particle, cut_name);
        }
    }

    /// Set the histogram filler function
    auto set_histogram_filler(HistogramFiller filler) -> void {
        m_filler = std::move(filler);
    }

   private:
    HistogramFiller m_filler;
};

}  // namespace Framework
