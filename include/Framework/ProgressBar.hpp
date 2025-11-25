#pragma once

#include <fmt/format.h>
#include <fmt/color.h>
#include <chrono>
#include <string>
#include <cmath>

namespace Framework {

/**
 * @brief Real-time progress bar with ETA
 *
 * Features:
 * - Visual progress indicator
 * - Estimated time remaining
 * - Processing rate (events/second)
 * - Percentage complete
 * - Terminal-friendly (updates in place)
 *
 * Example:
 * @code
 * ProgressBar progress(total_events, "Processing");
 * for (int i = 0; i < total_events; ++i) {
 *     // ... process event ...
 *     progress.update(i + 1);
 * }
 * progress.finish();
 * @endcode
 */
class ProgressBar {
public:
    /**
     * @brief Construct progress bar
     * @param total Total number of items to process
     * @param description Description to display
     * @param bar_width Width of progress bar (default 50)
     */
    explicit ProgressBar(size_t total, std::string description = "Progress",
                        int bar_width = 50)
        : m_total(total)
        , m_description(std::move(description))
        , m_bar_width(bar_width)
        , m_current(0)
        , m_start_time(std::chrono::high_resolution_clock::now())
        , m_last_update_time(m_start_time)
        , m_update_interval_ms(100) // Update every 100ms
    {
        if (m_total > 0) {
            display();
        }
    }

    /**
     * @brief Update progress
     * @param current Current progress count
     */
    auto update(size_t current) -> void {
        m_current = current;

        // Throttle updates
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_last_update_time).count();

        if (elapsed_ms >= m_update_interval_ms || m_current >= m_total) {
            display();
            m_last_update_time = now;
        }
    }

    /**
     * @brief Increment progress by one
     */
    auto increment() -> void {
        update(m_current + 1);
    }

    /**
     * @brief Finish progress bar (print final state)
     */
    auto finish() -> void {
        m_current = m_total;
        display();
        fmt::print("\n");
    }

    /**
     * @brief Set update interval
     * @param ms Milliseconds between updates
     */
    auto set_update_interval(int ms) -> void {
        m_update_interval_ms = ms;
    }

private:
    size_t m_total;
    std::string m_description;
    int m_bar_width;
    size_t m_current;
    std::chrono::high_resolution_clock::time_point m_start_time;
    std::chrono::high_resolution_clock::time_point m_last_update_time;
    int m_update_interval_ms;

    auto display() -> void {
        if (m_total == 0) return;

        // Calculate progress
        double fraction = static_cast<double>(m_current) / m_total;
        int percent = static_cast<int>(fraction * 100.0);

        // Calculate time statistics
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_start_time);
        double elapsed_sec = elapsed.count();

        // Calculate rate and ETA
        double rate = (elapsed_sec > 0) ? m_current / elapsed_sec : 0.0;
        double remaining_sec = (rate > 0) ? (m_total - m_current) / rate : 0.0;

        // Build progress bar
        int filled = static_cast<int>(fraction * m_bar_width);
        std::string bar;
        for (int i = 0; i < m_bar_width; ++i) {
            if (i < filled) {
                bar += "█";
            } else if (i == filled) {
                bar += "▌";
            } else {
                bar += " ";
            }
        }

        // Format time strings
        std::string elapsed_str = format_time(elapsed_sec);
        std::string eta_str = format_time(remaining_sec);
        std::string rate_str = format_rate(rate);

        // Clear line and print progress
        fmt::print("\r\033[K");  // Clear line

        if (m_current >= m_total) {
            // Finished - use green
            fmt::print(fg(fmt::color::green) | fmt::emphasis::bold,
                      "✓ {}: ", m_description);
            fmt::print(fg(fmt::color::green), "[{}] ", bar);
            fmt::print(fg(fmt::color::green) | fmt::emphasis::bold, "{}% ", percent);
            fmt::print(fg(fmt::color::green),
                      "({}/{}) ", m_current, m_total);
            fmt::print(fg(fmt::color::dim_gray),
                      "[{}] ", elapsed_str);
            fmt::print(fg(fmt::color::cyan),
                      "{}", rate_str);
        } else {
            // In progress - use cyan
            fmt::print(fg(fmt::color::white) | fmt::emphasis::bold,
                      "{}: ", m_description);
            fmt::print(fg(fmt::color::cyan), "[{}] ", bar);
            fmt::print(fg(fmt::color::cyan) | fmt::emphasis::bold, "{}% ", percent);
            fmt::print(fg(fmt::color::white),
                      "({}/{}) ", m_current, m_total);
            fmt::print(fg(fmt::color::dim_gray),
                      "[{}<{}] ", elapsed_str, eta_str);
            fmt::print(fg(fmt::color::yellow),
                      "{}", rate_str);
        }

        std::fflush(stdout);
    }

    auto format_time(double seconds) -> std::string {
        if (std::isnan(seconds) || std::isinf(seconds) || seconds < 0) {
            return "??:??:??";
        }

        int hours = static_cast<int>(seconds) / 3600;
        int minutes = (static_cast<int>(seconds) % 3600) / 60;
        int secs = static_cast<int>(seconds) % 60;

        if (hours > 0) {
            return fmt::format("{:02d}:{:02d}:{:02d}", hours, minutes, secs);
        } else {
            return fmt::format("{:02d}:{:02d}", minutes, secs);
        }
    }

    auto format_rate(double rate) -> std::string {
        if (rate < 0.01) {
            return "?.? evt/s";
        } else if (rate < 10) {
            return fmt::format("{:.1f} evt/s", rate);
        } else if (rate < 1000) {
            return fmt::format("{:.0f} evt/s", rate);
        } else {
            return fmt::format("{:.1f}k evt/s", rate / 1000.0);
        }
    }
};

/**
 * @brief Simple percentage progress (no bar, just percent)
 */
class SimpleProgress {
public:
    explicit SimpleProgress(size_t total, std::string description = "Progress")
        : m_total(total), m_description(std::move(description)), m_current(0) {}

    auto update(size_t current) -> void {
        m_current = current;
        if (m_total > 0) {
            int percent = static_cast<int>(100.0 * m_current / m_total);
            if (percent != m_last_percent) {
                fmt::print("\r{}: {}% ({}/{})", m_description, percent,
                          m_current, m_total);
                std::fflush(stdout);
                m_last_percent = percent;
            }
        }
    }

    auto finish() -> void {
        m_current = m_total;
        if (m_total > 0) {
            fmt::print("\r{}: 100% ({}/{})\n", m_description, m_total, m_total);
        }
    }

private:
    size_t m_total;
    std::string m_description;
    size_t m_current;
    int m_last_percent = -1;
};

}  // namespace Framework
