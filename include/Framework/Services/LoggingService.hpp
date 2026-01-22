#pragma once

#include <fmt/format.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include "Framework/IService.hpp"

namespace Framework {

/**
 * @brief Log level enumeration
 */
enum class LogLevel {
    DEBUG = 0,    // Detailed debugging information
    INFO = 1,     // General informational messages
    WARNING = 2,  // Warning messages
    ERROR = 3,    // Error messages
    CRITICAL = 4  // Critical errors
};

/**
 * @brief RAII timer for measuring operation duration
 */
class ScopedTimer {
   public:
    using Clock = std::chrono::high_resolution_clock;

    ScopedTimer() : m_start(Clock::now()) {}

    auto elapsed_ms() const -> double {
        auto now = Clock::now();
        return std::chrono::duration<double, std::milli>(now - m_start).count();
    }

    auto elapsed_sec() const -> double {
        return elapsed_ms() / 1000.0;
    }

    auto reset() -> void {
        m_start = Clock::now();
    }

   private:
    Clock::time_point m_start;
};

/**
 * @brief Centralized logging service with enhanced visuals
 *
 * Provides thread-safe logging with:
 * - Multiple log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
 * - Beautiful console output with icons and colors
 * - Section headers and dividers
 * - Timer utilities for performance measurement
 * - Progress indicators
 * - Structured key-value logging
 * - Optional file output
 *
 * Example:
 * @code
 * auto log = service<LoggingService>("LoggingService");
 * log->info("Processing event {}", event_number);
 * log->warning("No trigger electron found");
 * log->error("Failed to read bank: {}", bank_name);
 *
 * // Section headers
 * log->section("Initialization");
 *
 * // Key-value pairs
 * log->config("MyAlg", "threshold", 0.5);
 * log->config("MyAlg", "max_events", 1000);
 *
 * // Timing
 * {
 *     auto timer = log->start_timer("MyAlg", "Processing");
 *     // ... do work ...
 * } // auto-logs duration on scope exit
 * @endcode
 */
class LoggingService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "LoggingService"; }

    explicit LoggingService(std::string name = "LoggingService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        info("LoggingService", "Initialized");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        info("LoggingService", "Finalized");
        if (m_file_stream.is_open()) {
            m_file_stream.close();
        }
        return StatusCode::SUCCESS();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Configuration
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief Set minimum log level
     */
    auto set_log_level(LogLevel level) -> void {
        m_min_level = level;
    }

    /**
     * @brief Set log level for a specific source
     */
    auto set_source_level(const std::string& source, LogLevel level) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_source_levels[source] = level;
    }

    /**
     * @brief Enable/disable console output
     */
    auto set_console_output(bool enable) -> void {
        m_console_output = enable;
    }

    /**
     * @brief Enable/disable colored console output
     */
    auto set_colored_output(bool enable) -> void {
        m_colored_output = enable;
    }

    /**
     * @brief Enable/disable timestamps
     */
    auto set_show_timestamps(bool enable) -> void {
        m_show_timestamps = enable;
    }

    /**
     * @brief Enable/disable icons
     */
    auto set_show_icons(bool enable) -> void {
        m_show_icons = enable;
    }

    /**
     * @brief Set log file output
     */
    auto set_log_file(const std::string& filename) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_file_stream.is_open()) {
            m_file_stream.close();
        }
        m_log_file = filename;
        m_file_stream.open(filename, std::ios::out | std::ios::trunc);
        if (!m_file_stream.is_open()) {
            fmt::print(stderr, "WARNING: Failed to open log file: {}\n", filename);
        }
    }

    /**
     * @brief Get current log level
     */
    auto get_log_level() const -> LogLevel {
        return m_min_level;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Basic Logging Methods
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief Log debug message
     */
    template <typename... Args>
    auto debug(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::DEBUG, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log info message
     */
    template <typename... Args>
    auto info(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::INFO, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log warning message
     */
    template <typename... Args>
    auto warning(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::WARNING, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log error message
     */
    template <typename... Args>
    auto error(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::ERROR, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log critical message
     */
    template <typename... Args>
    auto critical(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::CRITICAL, source, fmt, std::forward<Args>(args)...);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Enhanced Logging Methods
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief Log a success message
     */
    template <typename... Args>
    auto success(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (LogLevel::INFO < m_min_level) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        std::string message = fmt::format(fmt, std::forward<Args>(args)...);

        if (m_console_output) {
            if (m_colored_output) {
                std::string icon = m_show_icons ? "\033[32m\342\234\223\033[0m " : "";
                fmt::print("{}{}\033[32m{:<16}\033[0m {}\n",
                           format_timestamp(), icon, source, message);
            } else {
                fmt::print("{}[OK] {:<16} {}\n", format_timestamp(), source, message);
            }
        }
        write_to_file("SUCCESS", source, message);
    }

    /**
     * @brief Print a section header
     */
    auto section(const std::string& title) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_console_output) {
            if (m_colored_output) {
                fmt::print("\n\033[1;36m{}\033[0m\n", std::string(60, '\342'));
                fmt::print("\033[1;36m  {}\033[0m\n", title);
                fmt::print("\033[1;36m{}\033[0m\n\n", std::string(60, '\342'));
            } else {
                fmt::print("\n{}\n", std::string(60, '='));
                fmt::print("  {}\n", title);
                fmt::print("{}\n\n", std::string(60, '='));
            }
        }
        write_to_file_raw("\n" + std::string(60, '=') + "\n  " + title + "\n" + std::string(60, '=') + "\n");
    }

    /**
     * @brief Print a subsection header
     */
    auto subsection(const std::string& title) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_console_output) {
            if (m_colored_output) {
                fmt::print("\n\033[1;37m  {}\033[0m\n", title);
                fmt::print("\033[90m  {}\033[0m\n", std::string(title.length() + 4, '-'));
            } else {
                fmt::print("\n  {}\n", title);
                fmt::print("  {}\n", std::string(title.length(), '-'));
            }
        }
        write_to_file_raw("\n  " + title + "\n  " + std::string(title.length(), '-') + "\n");
    }

    /**
     * @brief Print a divider line
     */
    auto divider(char c = '-', int width = 60) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_console_output) {
            if (m_colored_output) {
                fmt::print("\033[90m{}\033[0m\n", std::string(width, c));
            } else {
                fmt::print("{}\n", std::string(width, c));
            }
        }
        write_to_file_raw(std::string(width, c) + "\n");
    }

    /**
     * @brief Log a configuration value (key-value pair)
     */
    template <typename T>
    auto config(const std::string& source, const std::string& key, const T& value) -> void {
        if (LogLevel::INFO < m_min_level) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_console_output) {
            if (m_colored_output) {
                fmt::print("{}\033[90m{:<16}\033[0m \033[37m{:<20}\033[0m = \033[33m{}\033[0m\n",
                           format_timestamp(), source, key, value);
            } else {
                fmt::print("{}{:<16} {:<20} = {}\n", format_timestamp(), source, key, value);
            }
        }
        write_to_file("CONFIG", source, fmt::format("{} = {}", key, value));
    }

    /**
     * @brief Log a statistic (numeric value with optional unit)
     */
    template <typename T>
    auto stat(const std::string& source, const std::string& name, const T& value,
              const std::string& unit = "") -> void {
        if (LogLevel::INFO < m_min_level) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        std::string value_str = unit.empty() ? fmt::format("{}", value)
                                             : fmt::format("{} {}", value, unit);

        if (m_console_output) {
            if (m_colored_output) {
                std::string icon = m_show_icons ? "\033[34m\342\227\217\033[0m " : "";
                fmt::print("{}{}\033[90m{:<16}\033[0m \033[37m{:<24}\033[0m \033[1;34m{}\033[0m\n",
                           format_timestamp(), icon, source, name, value_str);
            } else {
                fmt::print("{}{:<16} {:<24} {}\n", format_timestamp(), source, name, value_str);
            }
        }
        write_to_file("STAT", source, fmt::format("{}: {}", name, value_str));
    }

    /**
     * @brief Log start of an operation
     */
    auto start(const std::string& source, const std::string& operation) -> void {
        if (LogLevel::INFO < m_min_level) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_console_output) {
            if (m_colored_output) {
                std::string icon = m_show_icons ? "\033[33m\342\226\266\033[0m " : "";
                fmt::print("{}{}\033[33m{:<16}\033[0m Starting {}...\n",
                           format_timestamp(), icon, source, operation);
            } else {
                fmt::print("{}[>>] {:<16} Starting {}...\n", format_timestamp(), source, operation);
            }
        }
        write_to_file("START", source, "Starting " + operation);
    }

    /**
     * @brief Log end of an operation with duration
     */
    auto done(const std::string& source, const std::string& operation, double duration_ms) -> void {
        if (LogLevel::INFO < m_min_level) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        std::string duration_str = format_duration(duration_ms);

        if (m_console_output) {
            if (m_colored_output) {
                std::string icon = m_show_icons ? "\033[32m\342\234\223\033[0m " : "";
                fmt::print("{}{}\033[32m{:<16}\033[0m {} completed \033[90m({})\033[0m\n",
                           format_timestamp(), icon, source, operation, duration_str);
            } else {
                fmt::print("{}[OK] {:<16} {} completed ({})\n",
                           format_timestamp(), source, operation, duration_str);
            }
        }
        write_to_file("DONE", source, fmt::format("{} completed ({})", operation, duration_str));
    }

    /**
     * @brief Log a progress update
     */
    auto progress(const std::string& source, size_t current, size_t total,
                  const std::string& item = "items") -> void {
        if (LogLevel::INFO < m_min_level) return;
        if (total == 0) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        double percent = (static_cast<double>(current) / total) * 100.0;
        int bar_width = 30;
        int filled = static_cast<int>((percent / 100.0) * bar_width);

        if (m_console_output) {
            std::string bar(filled, '\342');  // Use box characters
            std::string empty(bar_width - filled, ' ');

            if (m_colored_output) {
                fmt::print("\r{}\033[90m{:<16}\033[0m [\033[32m{}\033[90m{}\033[0m] "
                           "\033[1m{:5.1f}%\033[0m ({}/{} {})",
                           format_timestamp(), source, bar, empty, percent, current, total, item);
            } else {
                std::string simple_bar(filled, '#');
                std::string simple_empty(bar_width - filled, '-');
                fmt::print("\r{}{:<16} [{}{}] {:5.1f}% ({}/{} {})",
                           format_timestamp(), source, simple_bar, simple_empty, percent, current, total, item);
            }

            if (current == total) {
                fmt::print("\n");
            } else {
                std::fflush(stdout);
            }
        }
    }

    /**
     * @brief Log a table row (for tabular data)
     */
    template <typename... Args>
    auto table_row(const std::string& format_str, Args&&... args) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string row = fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...);

        if (m_console_output) {
            if (m_colored_output) {
                fmt::print("  \033[37m{}\033[0m\n", row);
            } else {
                fmt::print("  {}\n", row);
            }
        }
        write_to_file_raw("  " + row + "\n");
    }

    /**
     * @brief Print a blank line
     */
    auto blank() -> void {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_console_output) {
            fmt::print("\n");
        }
        write_to_file_raw("\n");
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Table Formatting Utilities
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief Table builder for creating formatted tables
     *
     * Example usage:
     * @code
     * log->table("My Table")
     *     .columns({"Name", "Value", "Unit"})
     *     .widths({20, 10, 8})
     *     .row("Temperature", 25.5, "°C")
     *     .row("Pressure", 1013, "hPa")
     *     .footer("Total measurements: 2")
     *     .print();
     * @endcode
     */
    class TableBuilder {
       public:
        TableBuilder(LoggingService* svc, std::string title)
            : m_svc(svc), m_title(std::move(title)) {}

        auto columns(std::initializer_list<std::string> cols) -> TableBuilder& {
            m_columns.assign(cols);
            return *this;
        }

        auto widths(std::initializer_list<int> w) -> TableBuilder& {
            m_widths.assign(w);
            return *this;
        }

        template <typename... Args>
        auto row(Args&&... args) -> TableBuilder& {
            std::vector<std::string> cells;
            (cells.push_back(fmt::format("{}", std::forward<Args>(args))), ...);
            m_rows.push_back(std::move(cells));
            return *this;
        }

        auto footer(const std::string& text) -> TableBuilder& {
            m_footer = text;
            return *this;
        }

        auto print() -> void {
            if (!m_svc) return;
            m_svc->print_table(*this);
        }

       private:
        friend class LoggingService;
        LoggingService* m_svc;
        std::string m_title;
        std::vector<std::string> m_columns;
        std::vector<int> m_widths;
        std::vector<std::vector<std::string>> m_rows;
        std::string m_footer;
    };

    /**
     * @brief Create a new table builder
     */
    [[nodiscard]] auto table(const std::string& title) -> TableBuilder {
        return TableBuilder(this, title);
    }

    /**
     * @brief Print a formatted table
     */
    auto print_table(const TableBuilder& tb) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Calculate total width
        int total_width = 1;  // Left border
        for (size_t i = 0; i < tb.m_widths.size(); ++i) {
            total_width += tb.m_widths[i] + 3;  // width + " | "
        }
        total_width -= 1;  // Last column doesn't need trailing space

        // Box drawing characters
        const char* TL = "\342\224\214";  // ┌
        const char* TR = "\342\224\220";  // ┐
        const char* BL = "\342\224\224";  // └
        const char* BR = "\342\224\230";  // ┘
        const char* H = "\342\224\200";   // ─
        const char* V = "\342\224\202";   // │
        const char* TM = "\342\224\254";  // ┬
        const char* BM = "\342\224\264";  // ┴
        const char* LM = "\342\224\234";  // ├
        const char* RM = "\342\224\244";  // ┤
        const char* X = "\342\224\274";   // ┼

        auto make_horizontal = [&](const char* left, const char* mid, const char* right) {
            std::string line = left;
            for (size_t i = 0; i < tb.m_widths.size(); ++i) {
                for (int j = 0; j < tb.m_widths[i] + 2; ++j) {
                    line += H;
                }
                if (i < tb.m_widths.size() - 1) {
                    line += mid;
                }
            }
            line += right;
            return line;
        };

        auto make_row = [&](const std::vector<std::string>& cells, [[maybe_unused]] bool header = false) {
            std::string line = V;
            for (size_t i = 0; i < tb.m_widths.size() && i < cells.size(); ++i) {
                std::string cell = cells[i];
                int width = tb.m_widths[i];

                // Truncate if too long
                if (static_cast<int>(cell.length()) > width) {
                    cell = cell.substr(0, width - 2) + "..";
                }

                // Right-align numbers, left-align text
                bool is_number = !cell.empty() && (std::isdigit(cell[0]) || cell[0] == '-' || cell[0] == '+');
                if (is_number) {
                    line += fmt::format(" {:>{}} ", cell, width);
                } else {
                    line += fmt::format(" {:<{}} ", cell, width);
                }

                line += V;
            }
            return line;
        };

        // Print table
        if (m_console_output) {
            // Title row
            std::string top_line = make_horizontal(TL, TM, TR);
            int content_width = total_width - 2;  // Minus borders
            std::string title_centered = tb.m_title;
            int padding = (content_width - static_cast<int>(title_centered.length())) / 2;
            if (padding < 0) padding = 0;

            if (m_colored_output) {
                fmt::print("\n\033[36m{}\033[0m\n", top_line);
                fmt::print("\033[36m{}\033[0m\033[1;37m{:^{}}\033[0m\033[36m{}\033[0m\n",
                           V, tb.m_title, content_width, V);
            } else {
                fmt::print("\n{}\n", top_line);
                fmt::print("{}{:^{}}{}\n", V, tb.m_title, content_width, V);
            }

            // Header separator
            std::string header_sep = make_horizontal(LM, X, RM);
            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m\n", header_sep);
            } else {
                fmt::print("{}\n", header_sep);
            }

            // Column headers
            if (!tb.m_columns.empty()) {
                std::string header_row = make_row(tb.m_columns, true);
                if (m_colored_output) {
                    fmt::print("\033[36m{}\033[0m\n", V);
                    // Print header with bold text
                    fmt::print("\033[36m{}\033[0m", V);
                    for (size_t i = 0; i < tb.m_widths.size() && i < tb.m_columns.size(); ++i) {
                        std::string cell = tb.m_columns[i];
                        int width = tb.m_widths[i];
                        if (static_cast<int>(cell.length()) > width) {
                            cell = cell.substr(0, width - 2) + "..";
                        }
                        fmt::print("\033[1;37m {:<{}} \033[0m\033[36m{}\033[0m", cell, width, V);
                    }
                    fmt::print("\n");
                } else {
                    fmt::print("{}\n", make_row(tb.m_columns));
                }

                // Header-data separator
                if (m_colored_output) {
                    fmt::print("\033[36m{}\033[0m\n", header_sep);
                } else {
                    fmt::print("{}\n", header_sep);
                }
            }

            // Data rows
            for (const auto& row : tb.m_rows) {
                if (m_colored_output) {
                    fmt::print("\033[36m{}\033[0m", V);
                    for (size_t i = 0; i < tb.m_widths.size() && i < row.size(); ++i) {
                        std::string cell = row[i];
                        int width = tb.m_widths[i];
                        if (static_cast<int>(cell.length()) > width) {
                            cell = cell.substr(0, width - 2) + "..";
                        }
                        // Right-align numbers
                        bool is_number = !cell.empty() && (std::isdigit(cell[0]) || cell[0] == '-' || cell[0] == '+');
                        if (is_number) {
                            fmt::print("\033[33m {:>{}} \033[0m\033[36m{}\033[0m", cell, width, V);
                        } else {
                            fmt::print("\033[37m {:<{}} \033[0m\033[36m{}\033[0m", cell, width, V);
                        }
                    }
                    fmt::print("\n");
                } else {
                    fmt::print("{}\n", make_row(row));
                }
            }

            // Footer
            if (!tb.m_footer.empty()) {
                std::string footer_sep = make_horizontal(LM, BM, RM);
                if (m_colored_output) {
                    fmt::print("\033[36m{}\033[0m\n", footer_sep);
                    fmt::print("\033[36m{}\033[0m\033[1;32m {:<{}} \033[0m\033[36m{}\033[0m\n",
                               V, tb.m_footer, content_width - 1, V);
                } else {
                    fmt::print("{}\n", footer_sep);
                    fmt::print("{} {:<{}} {}\n", V, tb.m_footer, content_width - 1, V);
                }
            }

            // Bottom border
            std::string bottom_line = make_horizontal(BL, BM, BR);
            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m\n", bottom_line);
            } else {
                fmt::print("{}\n", bottom_line);
            }
        }

        // Write to file (simplified ASCII version)
        if (m_file_stream.is_open()) {
            write_to_file_raw("\n");
            write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
            int content_width = total_width - 2;
            write_to_file_raw("|" + fmt::format("{:^{}}", tb.m_title, content_width) + "|\n");
            write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");

            // Headers
            if (!tb.m_columns.empty()) {
                std::string header_line = "|";
                for (size_t i = 0; i < tb.m_widths.size() && i < tb.m_columns.size(); ++i) {
                    header_line += fmt::format(" {:<{}} |", tb.m_columns[i], tb.m_widths[i]);
                }
                write_to_file_raw(header_line + "\n");
                write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
            }

            // Data
            for (const auto& row : tb.m_rows) {
                std::string line = "|";
                for (size_t i = 0; i < tb.m_widths.size() && i < row.size(); ++i) {
                    bool is_number = !row[i].empty() && (std::isdigit(row[i][0]) || row[i][0] == '-');
                    if (is_number) {
                        line += fmt::format(" {:>{}} |", row[i], tb.m_widths[i]);
                    } else {
                        line += fmt::format(" {:<{}} |", row[i], tb.m_widths[i]);
                    }
                }
                write_to_file_raw(line + "\n");
            }

            if (!tb.m_footer.empty()) {
                write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
                write_to_file_raw("| " + fmt::format("{:<{}}", tb.m_footer, content_width - 2) + " |\n");
            }
            write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
        }
    }

    /**
     * @brief Print a simple cutflow-style table
     * @param title Table title
     * @param cuts Vector of (cut_name, count, abs_eff, rel_eff) tuples
     * @param efficiency Overall efficiency as percentage
     */
    auto print_cutflow_table(
        const std::string& title,
        const std::vector<std::tuple<std::string, size_t, double, double>>& cuts,
        double efficiency
    ) -> void {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Column widths
        constexpr int CUT_WIDTH = 35;
        constexpr int COUNT_WIDTH = 12;
        constexpr int EFF_WIDTH = 10;
        // Total = 4 columns + 4*2 spaces + 5 borders = 67 + 8 + 5 = 80
        constexpr int TOTAL_WIDTH = CUT_WIDTH + COUNT_WIDTH + EFF_WIDTH + EFF_WIDTH + 13;

        // Box drawing characters
        const char* TL = "\342\224\214";  // ┌
        const char* TR = "\342\224\220";  // ┐
        const char* BL = "\342\224\224";  // └
        const char* BR = "\342\224\230";  // ┘
        const char* H = "\342\224\200";   // ─
        const char* V = "\342\224\202";   // │
        const char* TM = "\342\224\254";  // ┬
        const char* BM = "\342\224\264";  // ┴
        const char* LM = "\342\224\234";  // ├
        const char* RM = "\342\224\244";  // ┤

        auto repeat = [](const char* ch, int n) {
            std::string result;
            for (int i = 0; i < n; ++i) result += ch;
            return result;
        };

        // Box drawing: ┼ for cross intersections
        const char* X = "\342\224\274";   // ┼

        // Build horizontal lines (WIDTH + 2 to account for space before and after content)
        std::string top = std::string(TL) + repeat(H, TOTAL_WIDTH - 2) + TR;
        std::string title_sep = std::string(LM) + repeat(H, CUT_WIDTH + 2) + TM +
                                repeat(H, COUNT_WIDTH + 2) + TM +
                                repeat(H, EFF_WIDTH + 2) + TM +
                                repeat(H, EFF_WIDTH + 2) + RM;
        std::string header_sep = std::string(LM) + repeat(H, CUT_WIDTH + 2) + X +
                                 repeat(H, COUNT_WIDTH + 2) + X +
                                 repeat(H, EFF_WIDTH + 2) + X +
                                 repeat(H, EFF_WIDTH + 2) + RM;
        std::string footer_sep = std::string(LM) + repeat(H, CUT_WIDTH + 2) + BM +
                                 repeat(H, COUNT_WIDTH + 2) + BM +
                                 repeat(H, EFF_WIDTH + 2) + BM +
                                 repeat(H, EFF_WIDTH + 2) + RM;
        std::string bottom = std::string(BL) + repeat(H, TOTAL_WIDTH - 2) + BR;

        if (m_console_output) {
            // Top border + title
            if (m_colored_output) {
                fmt::print("\n\033[36m{}\033[0m\n", top);
                fmt::print("\033[36m{}\033[0m\033[1;37m{:^{}}\033[0m\033[36m{}\033[0m\n",
                           V, "Cutflow: " + title, TOTAL_WIDTH - 2, V);
                fmt::print("\033[36m{}\033[0m\n", title_sep);
            } else {
                fmt::print("\n{}\n", top);
                fmt::print("{}{:^{}}{}\n", V, "Cutflow: " + title, TOTAL_WIDTH - 2, V);
                fmt::print("{}\n", title_sep);
            }

            // Column headers
            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m\033[1;37m {:<{}} \033[0m\033[36m{}\033[0m"
                           "\033[1;37m {:>{}} \033[0m\033[36m{}\033[0m"
                           "\033[1;37m {:>{}} \033[0m\033[36m{}\033[0m"
                           "\033[1;37m {:>{}} \033[0m\033[36m{}\033[0m\n",
                           V, "Cut", CUT_WIDTH, V,
                           "Events", COUNT_WIDTH, V,
                           "Abs Eff", EFF_WIDTH, V,
                           "Rel Eff", EFF_WIDTH, V);
                fmt::print("\033[36m{}\033[0m\n", header_sep);
            } else {
                fmt::print("{} {:<{}} {} {:>{}} {} {:>{}} {} {:>{}} {}\n",
                           V, "Cut", CUT_WIDTH, V,
                           "Events", COUNT_WIDTH, V,
                           "Abs Eff", EFF_WIDTH, V,
                           "Rel Eff", EFF_WIDTH, V);
                fmt::print("{}\n", header_sep);
            }

            // Data rows
            for (const auto& [cut_name, count, abs_eff, rel_eff] : cuts) {
                std::string count_str = fmt::format("{}", count);
                std::string abs_str = fmt::format("{:>6.1f}%", abs_eff);
                std::string rel_str = fmt::format("{:>6.1f}%", rel_eff);

                if (m_colored_output) {
                    // Color code efficiency (green for high, yellow for medium, red for low)
                    std::string abs_color = abs_eff >= 90 ? "\033[32m" : (abs_eff >= 50 ? "\033[33m" : "\033[31m");
                    std::string rel_color = rel_eff >= 95 ? "\033[32m" : (rel_eff >= 80 ? "\033[33m" : "\033[31m");

                    fmt::print("\033[36m{}\033[0m\033[37m {:<{}} \033[0m\033[36m{}\033[0m"
                               "\033[1;33m {:>{}} \033[0m\033[36m{}\033[0m"
                               "{} {:>{}} \033[0m\033[36m{}\033[0m"
                               "{} {:>{}} \033[0m\033[36m{}\033[0m\n",
                               V, cut_name, CUT_WIDTH, V,
                               count_str, COUNT_WIDTH, V,
                               abs_color, abs_str, EFF_WIDTH, V,
                               rel_color, rel_str, EFF_WIDTH, V);
                } else {
                    fmt::print("{} {:<{}} {} {:>{}} {} {:>{}} {} {:>{}} {}\n",
                               V, cut_name, CUT_WIDTH, V,
                               count_str, COUNT_WIDTH, V,
                               abs_str, EFF_WIDTH, V,
                               rel_str, EFF_WIDTH, V);
                }
            }

            // Footer with overall efficiency
            std::string eff_str = fmt::format("Overall efficiency: {:.2f}%", efficiency);
            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m\n", footer_sep);
                std::string eff_color = efficiency >= 90 ? "\033[1;32m" : (efficiency >= 50 ? "\033[1;33m" : "\033[1;31m");
                fmt::print("\033[36m{}\033[0m{} {:<{}} \033[0m\033[36m{}\033[0m\n",
                           V, eff_color, eff_str, TOTAL_WIDTH - 3, V);
                fmt::print("\033[36m{}\033[0m\n", bottom);
            } else {
                fmt::print("{}\n", footer_sep);
                fmt::print("{} {:<{}} {}\n", V, eff_str, TOTAL_WIDTH - 3, V);
                fmt::print("{}\n", bottom);
            }
        }

        // Write to log file (ASCII version)
        if (m_file_stream.is_open()) {
            std::string eff_str_file = fmt::format("Overall efficiency: {:.2f}%", efficiency);
            write_to_file_raw("\n+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
            write_to_file_raw(fmt::format("|{:^{}}|\n", "Cutflow: " + title, TOTAL_WIDTH - 2));
            write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
            write_to_file_raw(fmt::format("| {:<{}} | {:>{}} | {:>{}} | {:>{}} |\n",
                                         "Cut", CUT_WIDTH, "Events", COUNT_WIDTH - 1,
                                         "Abs Eff", EFF_WIDTH - 1, "Rel Eff", EFF_WIDTH - 1));
            write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");

            for (const auto& [cut_name, count, abs_eff, rel_eff] : cuts) {
                write_to_file_raw(fmt::format("| {:<{}} | {:>{}} | {:>6.1f}%   | {:>6.1f}%   |\n",
                                             cut_name, CUT_WIDTH, count, COUNT_WIDTH - 1,
                                             abs_eff, rel_eff));
            }

            write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
            write_to_file_raw(fmt::format("| {:<{}} |\n", eff_str_file, TOTAL_WIDTH - 3));
            write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Timer Utilities
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief RAII scoped timer that logs duration on destruction
     */
    class ScopedLog {
       public:
        ScopedLog(LoggingService* svc, std::string source, std::string operation)
            : m_svc(svc), m_source(std::move(source)), m_operation(std::move(operation)) {
            if (m_svc) m_svc->start(m_source, m_operation);
        }

        ~ScopedLog() {
            if (m_svc) m_svc->done(m_source, m_operation, m_timer.elapsed_ms());
        }

        // Non-copyable, non-movable
        ScopedLog(const ScopedLog&) = delete;
        ScopedLog& operator=(const ScopedLog&) = delete;
        ScopedLog(ScopedLog&&) = delete;
        ScopedLog& operator=(ScopedLog&&) = delete;

        auto elapsed_ms() const -> double { return m_timer.elapsed_ms(); }

       private:
        LoggingService* m_svc;
        std::string m_source;
        std::string m_operation;
        ScopedTimer m_timer;
    };

    /**
     * @brief Start a scoped timer that auto-logs on destruction
     */
    [[nodiscard]] auto scoped(const std::string& source, const std::string& operation)
        -> std::unique_ptr<ScopedLog> {
        return std::make_unique<ScopedLog>(this, source, operation);
    }

   private:
    LogLevel m_min_level = LogLevel::INFO;
    bool m_console_output = true;
    bool m_colored_output = true;
    bool m_show_timestamps = true;
    bool m_show_icons = true;
    std::string m_log_file;
    std::ofstream m_file_stream;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, LogLevel> m_source_levels;

    /**
     * @brief Format timestamp string
     */
    auto format_timestamp() const -> std::string {
        if (!m_show_timestamps) return "";

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();

        if (m_colored_output) {
            return fmt::format("\033[90m[{}]\033[0m ", ss.str());
        }
        return fmt::format("[{}] ", ss.str());
    }

    /**
     * @brief Format duration in human-readable form
     */
    static auto format_duration(double ms) -> std::string {
        if (ms < 1.0) {
            return fmt::format("{:.1f} \302\265s", ms * 1000);
        } else if (ms < 1000.0) {
            return fmt::format("{:.1f} ms", ms);
        } else if (ms < 60000.0) {
            return fmt::format("{:.2f} s", ms / 1000.0);
        } else {
            double minutes = ms / 60000.0;
            return fmt::format("{:.1f} min", minutes);
        }
    }

    /**
     * @brief Get effective log level for a source
     */
    auto effective_level(const std::string& source) const -> LogLevel {
        auto it = m_source_levels.find(source);
        if (it != m_source_levels.end()) {
            return it->second;
        }
        return m_min_level;
    }

    /**
     * @brief Core logging function
     */
    template <typename... Args>
    auto log(LogLevel level, const std::string& source,
             fmt::format_string<Args...> fmt_str, Args&&... args) -> void {
        if (level < effective_level(source)) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        // Format the message
        std::string message = fmt::format(fmt_str, std::forward<Args>(args)...);

        // Get level string, icon, and ANSI color code
        std::string level_str;
        std::string icon;
        std::string color_code;

        switch (level) {
            case LogLevel::DEBUG:
                level_str = "DEBUG";
                icon = "\342\227\213";  // white circle
                color_code = "\033[90m";  // Gray
                break;
            case LogLevel::INFO:
                level_str = "INFO";
                icon = "\342\227\217";  // black circle
                color_code = "\033[36m";  // Cyan
                break;
            case LogLevel::WARNING:
                level_str = "WARN";
                icon = "\342\232\240";  // warning sign
                color_code = "\033[33m";  // Yellow
                break;
            case LogLevel::ERROR:
                level_str = "ERROR";
                icon = "\342\234\227";  // x mark
                color_code = "\033[31m";  // Red
                break;
            case LogLevel::CRITICAL:
                level_str = "CRIT";
                icon = "\342\234\227";  // x mark (bold)
                color_code = "\033[1;31m";  // Bold Red
                break;
        }

        // Console output
        if (m_console_output) {
            if (m_colored_output) {
                std::string icon_str = m_show_icons ? fmt::format("{}{}\033[0m ", color_code, icon) : "";
                fmt::print("{}{}{}{:<16}\033[0m {}\n",
                           format_timestamp(), icon_str, color_code, source, message);
            } else {
                fmt::print("{}[{:5}] {:<16} {}\n",
                           format_timestamp(), level_str, source, message);
            }
        }

        // File output
        write_to_file(level_str, source, message);
    }

    /**
     * @brief Write to log file
     */
    auto write_to_file(const std::string& level, const std::string& source,
                       const std::string& message) -> void {
        if (m_file_stream.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) % 1000;

            m_file_stream << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S")
                          << '.' << std::setfill('0') << std::setw(3) << ms.count() << "] "
                          << std::setw(5) << level << " "
                          << std::setw(16) << std::left << source << " "
                          << message << "\n";
            m_file_stream.flush();
        }
    }

    /**
     * @brief Write raw text to log file
     */
    auto write_to_file_raw(const std::string& text) -> void {
        if (m_file_stream.is_open()) {
            m_file_stream << text;
            m_file_stream.flush();
        }
    }
};

/**
 * @brief Global logger singleton for easy access without injection
 *
 * This provides a global access point to the logging service.
 * The LoggingService instance is set by the Application during initialization.
 *
 * Usage:
 * @code
 * // In any algorithm or service:
 * Log::info("MyAlgorithm", "Processing event {}", event_num);
 * Log::warning("MyService", "Value out of range: {}", value);
 *
 * // With scoped timing:
 * {
 *     auto timer = Log::scoped("MyAlg", "Heavy computation");
 *     // ... work ...
 * } // auto-logs duration
 *
 * // Section headers:
 * Log::section("Initialization");
 *
 * // Key-value config:
 * Log::config("MyAlg", "threshold", 0.5);
 * @endcode
 */
class Log {
   public:
    /**
     * @brief Set the global logger instance
     * Called by Application during initialization
     */
    static auto set_instance(LoggingService* logger) -> void {
        s_instance = logger;
    }

    /**
     * @brief Get the global logger instance
     */
    static auto instance() -> LoggingService* {
        return s_instance;
    }

    /**
     * @brief Check if logger is available
     */
    static auto is_available() -> bool {
        return s_instance != nullptr;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Basic Logging (with fallback to stderr)
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief Log debug message
     */
    template <typename... Args>
    static auto debug(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) {
            s_instance->debug(source, fmt, std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Log info message
     */
    template <typename... Args>
    static auto info(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) {
            s_instance->info(source, fmt, std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Log warning message
     */
    template <typename... Args>
    static auto warning(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) {
            s_instance->warning(source, fmt, std::forward<Args>(args)...);
        } else {
            // Fallback to stderr for warnings even without logger
            fmt::print(stderr, "\033[33m[WARN] {}: {}\033[0m\n", source,
                       fmt::format(fmt, std::forward<Args>(args)...));
        }
    }

    /**
     * @brief Log error message
     */
    template <typename... Args>
    static auto error(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) {
            s_instance->error(source, fmt, std::forward<Args>(args)...);
        } else {
            // Fallback to stderr for errors even without logger
            fmt::print(stderr, "\033[31m[ERROR] {}: {}\033[0m\n", source,
                       fmt::format(fmt, std::forward<Args>(args)...));
        }
    }

    /**
     * @brief Log critical message
     */
    template <typename... Args>
    static auto critical(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) {
            s_instance->critical(source, fmt, std::forward<Args>(args)...);
        } else {
            // Fallback to stderr for critical errors even without logger
            fmt::print(stderr, "\033[1;31m[CRITICAL] {}: {}\033[0m\n", source,
                       fmt::format(fmt, std::forward<Args>(args)...));
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Enhanced Logging
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief Log success message
     */
    template <typename... Args>
    static auto success(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) s_instance->success(source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Print section header
     */
    static auto section(const std::string& title) -> void {
        if (s_instance) s_instance->section(title);
    }

    /**
     * @brief Print subsection header
     */
    static auto subsection(const std::string& title) -> void {
        if (s_instance) s_instance->subsection(title);
    }

    /**
     * @brief Print divider
     */
    static auto divider(char c = '-', int width = 60) -> void {
        if (s_instance) s_instance->divider(c, width);
    }

    /**
     * @brief Log configuration value
     */
    template <typename T>
    static auto config(const std::string& source, const std::string& key, const T& value) -> void {
        if (s_instance) s_instance->config(source, key, value);
    }

    /**
     * @brief Log statistic
     */
    template <typename T>
    static auto stat(const std::string& source, const std::string& name, const T& value,
                     const std::string& unit = "") -> void {
        if (s_instance) s_instance->stat(source, name, value, unit);
    }

    /**
     * @brief Log start of operation
     */
    static auto start(const std::string& source, const std::string& operation) -> void {
        if (s_instance) s_instance->start(source, operation);
    }

    /**
     * @brief Log end of operation with duration
     */
    static auto done(const std::string& source, const std::string& operation, double duration_ms) -> void {
        if (s_instance) s_instance->done(source, operation, duration_ms);
    }

    /**
     * @brief Log progress
     */
    static auto progress(const std::string& source, size_t current, size_t total,
                         const std::string& item = "items") -> void {
        if (s_instance) s_instance->progress(source, current, total, item);
    }

    /**
     * @brief Print table row
     */
    template <typename... Args>
    static auto table_row(const std::string& format_str, Args&&... args) -> void {
        if (s_instance) s_instance->table_row(format_str, std::forward<Args>(args)...);
    }

    /**
     * @brief Print blank line
     */
    static auto blank() -> void {
        if (s_instance) s_instance->blank();
    }

    /**
     * @brief Create scoped timer
     */
    [[nodiscard]] static auto scoped(const std::string& source, const std::string& operation)
        -> std::unique_ptr<LoggingService::ScopedLog> {
        if (s_instance) return s_instance->scoped(source, operation);
        return nullptr;
    }

    /**
     * @brief Print a cutflow table
     */
    static auto print_cutflow_table(
        const std::string& title,
        const std::vector<std::tuple<std::string, size_t, double, double>>& cuts,
        double efficiency
    ) -> void {
        if (s_instance) s_instance->print_cutflow_table(title, cuts, efficiency);
    }

   private:
    static inline LoggingService* s_instance = nullptr;
};

}  // namespace Framework
