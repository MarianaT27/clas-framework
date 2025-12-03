#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <fmt/format.h>
#include "Framework/LoggingService.hpp"

namespace Framework {

// Forward declaration
class LoggingService;

/**
 * @brief Cutflow manager for tracking event selection efficiency
 *
 * This class implements a cutflow system similar to those used in ATLAS/CMS
 * analyses. It tracks the number of events passing each cut in a sequential
 * selection chain and provides methods to print cutflow tables and calculate
 * efficiencies.
 *
 * Example usage:
 * @code
 * Cutflow cutflow("Electron Selection");
 * cutflow.add_cut("All Events");
 * cutflow.add_cut("Trigger Electron");
 * cutflow.add_cut("Chi2 PID");
 * cutflow.add_cut("Momentum > 0.8 GeV");
 * cutflow.add_cut("Sampling Fraction");
 * cutflow.add_cut("PCAL Fiducial");
 *
 * // In event loop:
 * cutflow.pass("All Events");
 * if (!has_trigger_electron) return;
 * cutflow.pass("Trigger Electron");
 * if (!pass_chi2pid) return;
 * cutflow.pass("Chi2 PID");
 * // ... etc
 *
 * // At end:
 * cutflow.print();
 * @endcode
 */
class Cutflow {
public:
    /**
     * @brief Construct a new Cutflow object
     * @param name Name of this cutflow (e.g., "Electron Selection")
     */
    explicit Cutflow(std::string name) : m_name(std::move(name)) {}

    /**
     * @brief Add a cut to the cutflow
     * @param cut_name Name of the cut
     */
    auto add_cut(const std::string& cut_name) -> void {
        m_cut_names.push_back(cut_name);
        m_counters[cut_name] = 0;
        m_cut_order.push_back(cut_name);
    }

    /**
     * @brief Increment the counter for a given cut
     * @param cut_name Name of the cut that was passed
     */
    auto pass(const std::string& cut_name) -> void {
        m_counters[cut_name]++;
    }

    /**
     * @brief Get the count for a specific cut
     * @param cut_name Name of the cut
     * @return size_t Number of events passing this cut
     */
    auto get_count(const std::string& cut_name) const -> size_t {
        auto it = m_counters.find(cut_name);
        if (it != m_counters.end()) {
            return it->second.load();
        }
        return 0;
    }

    /**
     * @brief Get all cut names in order
     * @return const std::vector<std::string>& Vector of cut names
     */
    auto get_cut_names() const -> const std::vector<std::string>& {
        return m_cut_order;
    }

    /**
     * @brief Set logging service for structured output
     */
    auto set_logger(LoggingService* log) -> void {
        m_log = log;
    }

    /**
     * @brief Print the cutflow table to stdout or logger
     */
    auto print() const -> void {
        if (m_log) {
            print_with_logger();
        } else {
            print_legacy();
        }
    }

private:
    /**
     * @brief Print cutflow using logger (one line per cut)
     */
    auto print_with_logger() const -> void {
        if (m_cut_order.empty()) return;

        // Calculate column widths for alignment
        size_t initial_count = get_count(m_cut_order[0]);
        size_t previous_count = initial_count;

        // Print each cut as a single INFO line
        for (size_t i = 0; i < m_cut_order.size(); ++i) {
            const auto& cut_name = m_cut_order[i];
            size_t count = get_count(cut_name);

            if (i == 0) {
                m_log->info(m_name, "{:<40} │ {:>10} events │ Abs: {:>6} │ Rel: {:>6}",
                           cut_name, count, "100.0%", "100.0%");
                previous_count = count;
            } else {
                double abs_eff = initial_count > 0 ? 100.0 * count / initial_count : 0.0;
                double rel_eff = previous_count > 0 ? 100.0 * count / previous_count : 0.0;
                m_log->info(m_name, "{:<40} │ {:>10} events │ Abs: {:>5.2f}% │ Rel: {:>5.2f}%",
                           cut_name, count, abs_eff, rel_eff);
                previous_count = count;
            }
        }
    }

    /**
     * @brief Legacy print method (fallback when no logger)
     */
    auto print_legacy() const -> void {
        fmt::print("\n");
        fmt::print("═══════════════════════════════════════════════════════════════\n");
        fmt::print("  Cutflow: {}\n", m_name);
        fmt::print("═══════════════════════════════════════════════════════════════\n");
        fmt::print("{:<35} {:>10} {:>8} {:>8}\n", "Cut", "Events", "Abs Eff", "Rel Eff");
        fmt::print("───────────────────────────────────────────────────────────────\n");

        size_t initial_count = 0;
        size_t previous_count = 0;

        for (size_t i = 0; i < m_cut_order.size(); ++i) {
            const auto& cut_name = m_cut_order[i];
            size_t count = get_count(cut_name);

            if (i == 0) {
                initial_count = count;
                previous_count = count;
                fmt::print("{:<35} {:>10} {:>8} {:>8}\n",
                          cut_name, count, "100.0%", "100.0%");
            } else {
                double abs_eff = initial_count > 0 ? 100.0 * count / initial_count : 0.0;
                double rel_eff = previous_count > 0 ? 100.0 * count / previous_count : 0.0;
                fmt::print("{:<35} {:>10} {:>7.2f}% {:>7.2f}%\n",
                          cut_name, count, abs_eff, rel_eff);
                previous_count = count;
            }
        }

        fmt::print("═══════════════════════════════════════════════════════════════\n");
        fmt::print("\n");
    }

public:

    /**
     * @brief Get the cutflow name
     * @return const std::string& Name of this cutflow
     */
    auto name() const -> const std::string& {
        return m_name;
    }

private:
    std::string m_name;
    std::vector<std::string> m_cut_names;
    std::vector<std::string> m_cut_order;  // Preserves order of cuts
    std::map<std::string, std::atomic<size_t>> m_counters;
    LoggingService* m_log = nullptr;
};

/**
 * @brief Extended cutflow with histogram filling capabilities
 *
 * This class extends the basic Cutflow to automatically fill histograms
 * after each cut, enabling N-1 plots and per-cut distributions.
 *
 * @tparam HistogramFiller Function type for filling histograms
 */
template<typename Particle>
class CutflowWithHistograms : public Cutflow {
public:
    using HistogramFiller = std::function<void(const Particle&, const std::string&)>;

    explicit CutflowWithHistograms(std::string name, HistogramFiller filler = nullptr)
        : Cutflow(std::move(name)), m_histogram_filler(std::move(filler)) {}

    /**
     * @brief Pass a cut and optionally fill histograms
     * @param cut_name Name of the cut
     * @param particle Particle object to fill histograms with
     * @param fill_histograms Whether to fill histograms for this event
     */
    auto pass(const std::string& cut_name, const Particle& particle, bool fill_histograms = true) -> void {
        Cutflow::pass(cut_name);
        if (fill_histograms && m_histogram_filler) {
            m_histogram_filler(particle, cut_name);
        }
    }

    /**
     * @brief Set the histogram filler function
     * @param filler Function to call for filling histograms
     */
    auto set_histogram_filler(HistogramFiller filler) -> void {
        m_histogram_filler = std::move(filler);
    }

private:
    HistogramFiller m_histogram_filler;
};

}  // namespace Framework
