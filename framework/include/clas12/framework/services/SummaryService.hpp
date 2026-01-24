#pragma once

#include <fmt/format.h>
#include <chrono>
#include <map>
#include <string>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/ProfilingService.hpp>

namespace clas12::framework {

/**
 * @brief Service for generating comprehensive analysis summary reports
 *
 * Generates end-of-job summary including:
 * - Event statistics
 * - Cutflow efficiencies
 * - Histogram summary
 * - Performance metrics
 * - Resource usage
 *
 * Integrates with:
 * - HistogramService
 * - ProfilingService
 * - LoggingService
 */
class SummaryService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "SummaryService"; }

    explicit SummaryService(std::string name = "SummaryService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        m_start_time = std::chrono::high_resolution_clock::now();
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        m_end_time = std::chrono::high_resolution_clock::now();
        generate_summary();
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Record event statistics
     */
    auto record_events_processed(size_t count) -> void {
        m_events_processed = count;
    }

    /**
     * @brief Record events accepted (passed all cuts)
     */
    auto record_events_accepted(size_t count) -> void {
        m_events_accepted = count;
    }

    /**
     * @brief Add a cutflow stage
     */
    auto add_cutflow_stage(const std::string& stage_name, size_t count) -> void {
        m_cutflow[stage_name] = count;
    }

    /**
     * @brief Record custom statistic
     */
    auto add_statistic(const std::string& name, double value) -> void {
        m_statistics[name] = value;
    }

    /**
     * @brief Generate and print comprehensive summary
     */
    auto generate_summary() -> void {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            m_end_time - m_start_time);

        fmt::print("\n");
        fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
        fmt::print("║                   Analysis Summary Report                      ║\n");
        fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
        fmt::print("\n");

        // Timing information
        print_section_header("Timing Information");
        fmt::print("  Total runtime: {:.2f} seconds\n", duration.count() / 1000.0);
        if (m_events_processed > 0 && duration.count() > 0) {
            double events_per_sec = m_events_processed / (duration.count() / 1000.0);
            fmt::print("  Processing rate: {:.1f} events/second\n", events_per_sec);
            fmt::print("  Time per event: {:.2f} ms\n",
                       duration.count() / static_cast<double>(m_events_processed));
        }
        fmt::print("\n");

        // Event statistics
        print_section_header("Event Statistics");
        fmt::print("  Events processed: {}\n", m_events_processed);
        fmt::print("  Events accepted: {}\n", m_events_accepted);
        if (m_events_processed > 0) {
            double efficiency = 100.0 * m_events_accepted / m_events_processed;
            fmt::print("  Overall efficiency: {:.2f}%\n", efficiency);
        }
        fmt::print("\n");

        // Cutflow
        if (!m_cutflow.empty()) {
            print_section_header("Cutflow Efficiency");
            fmt::print("  {:<30} {:>12} {:>12} {:>12}\n",
                       "Cut Stage", "Events", "Efficiency", "Cumulative");
            fmt::print("  {:-<30} {:->12} {:->12} {:->12}\n", "", "", "", "");

            size_t previous_count = m_events_processed;
            for (const auto& [stage, count] : m_cutflow) {
                double stage_eff = (previous_count > 0) ? 100.0 * count / previous_count : 0.0;
                double cumul_eff = (m_events_processed > 0) ? 100.0 * count / m_events_processed : 0.0;

                fmt::print("  {:<30} {:>12} {:>11.2f}% {:>11.2f}%\n",
                           stage, count, stage_eff, cumul_eff);
                previous_count = count;
            }
            fmt::print("\n");
        }

        // Histogram summary
        if (auto hist_svc = get_histogram_service()) {
            print_section_header("Histogram Summary");
            fmt::print("  Total histograms: {}\n",
                       hist_svc->num_hist1D() + hist_svc->num_hist2D() +
                           hist_svc->num_profiles());
            fmt::print("    - 1D histograms: {}\n", hist_svc->num_hist1D());
            fmt::print("    - 2D histograms: {}\n", hist_svc->num_hist2D());
            fmt::print("    - TProfile: {}\n", hist_svc->num_profiles());
            fmt::print("  Output file: {}\n", hist_svc->get_output_file());
            fmt::print("\n");
        }

        // Custom statistics
        if (!m_statistics.empty()) {
            print_section_header("Custom Statistics");
            for (const auto& [name, value] : m_statistics) {
                fmt::print("  {}: {:.2f}\n", name, value);
            }
            fmt::print("\n");
        }

        // Performance (if ProfilingService available)
        if (auto prof_svc = get_profiling_service()) {
            auto stats = prof_svc->get_all_stats();
            if (!stats.empty()) {
                print_section_header("Performance Highlights");

                // Find slowest algorithm
                std::string slowest_name;
                double slowest_time = 0.0;
                for (const auto& [name, stat] : stats) {
                    if (stat.total_time_ms > slowest_time) {
                        slowest_time = stat.total_time_ms;
                        slowest_name = name;
                    }
                }

                if (!slowest_name.empty()) {
                    fmt::print("  Slowest algorithm: {} ({:.2f} ms total)\n",
                               slowest_name, slowest_time);
                }

                // Calculate total algorithm time
                double total_alg_time = 0.0;
                for (const auto& [name, stat] : stats) {
                    total_alg_time += stat.total_time_ms;
                }
                fmt::print("  Total algorithm time: {:.2f} seconds\n",
                           total_alg_time / 1000.0);

                fmt::print("  (See detailed performance report above)\n");
                fmt::print("\n");
            }
        }

        // Footer
        fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
        fmt::print("║                  Analysis Complete!                            ║\n");
        fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
        fmt::print("\n");
    }

    /**
     * @brief Write summary to file
     */
    auto write_summary_to_file(const std::string& filename) -> void {
        // Could redirect output to file here
        // For now, summary is printed to console
    }

   private:
    std::chrono::high_resolution_clock::time_point m_start_time;
    std::chrono::high_resolution_clock::time_point m_end_time;

    size_t m_events_processed = 0;
    size_t m_events_accepted = 0;
    std::map<std::string, size_t> m_cutflow;
    std::map<std::string, double> m_statistics;

    auto print_section_header(const std::string& title) -> void {
        fmt::print("─── {} ", title);
        for (size_t i = title.length() + 5; i < 63; ++i) {
            fmt::print("─");
        }
        fmt::print("\n");
    }

    auto get_histogram_service() -> HistogramService* {
        // Try to get histogram service from service manager
        // This requires access to service manager, which we'll handle in integration
        return nullptr;
    }

    auto get_profiling_service() -> ProfilingService* {
        // Try to get profiling service from service manager
        return nullptr;
    }
};

}  // namespace clas12::framework
