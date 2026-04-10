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
#include <clas12/framework/core/IService.hpp>

namespace clas12::framework {

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

    auto initialize() -> StatusCode override;

    auto finalize() -> StatusCode override;

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
    auto set_source_level(const std::string& source, LogLevel level) -> void;

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
    auto set_log_file(const std::string& filename) -> void;

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
    auto section(const std::string& title) -> void;

    /**
     * @brief Print a subsection header
     */
    auto subsection(const std::string& title) -> void;

    /**
     * @brief Print a divider line
     */
    auto divider(char c = '-', int width = 60) -> void;

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
    auto start(const std::string& source, const std::string& operation) -> void;

    /**
     * @brief Log end of an operation with duration
     */
    auto done(const std::string& source, const std::string& operation, double duration_ms) -> void;

    /**
     * @brief Log a progress update
     */
    auto progress(const std::string& source, size_t current, size_t total,
                  const std::string& item = "items") -> void;

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
    auto blank() -> void;

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
    auto print_table(const TableBuilder& tb) -> void;

    /**
     * @brief Print a simple cutflow-style table
     * @param title Table title
     * @param cuts Vector of (cut_name, count, abs_eff, rel_eff) tuples
     * @param efficiency Overall efficiency as percentage
     */
    auto print_cutflow_table(
        const std::string& title,
        const std::vector<std::tuple<std::string, size_t, double, double>>& cuts,
        double efficiency) -> void;

    /**
     * @brief Print a fit results table
     * @param title Table title (e.g., "Fit Results: LC_1.0_1.5")
     * @param rows Vector of (label, value, unit) tuples
     */
    auto print_fit_results_table(
        const std::string& title,
        const std::vector<std::tuple<std::string, std::string, std::string>>& rows) -> void;

    /**
     * @brief Print a yield summary table
     * @param title Table title
     * @param columns Column headers
     * @param rows Data rows - each row is a vector of strings
     * @param footer Optional footer text
     */
    auto print_yield_summary_table(
        const std::string& title,
        const std::vector<std::string>& columns,
        const std::vector<std::vector<std::string>>& rows,
        const std::string& footer = "") -> void;

    /**
     * @brief Print a run metadata summary
     * @param title Table title
     * @param total_runs Number of runs
     * @param runs Vector of run numbers
     */
    auto print_run_metadata(
        const std::string& title,
        size_t total_runs,
        const std::vector<int>& runs) -> void;

    // ─────────────────────────────────────────────────────────────────────────
    // Timer Utilities
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * @brief RAII scoped timer that logs duration on destruction
     */
    class ScopedLog {
       public:
        ScopedLog(LoggingService* svc, std::string source, std::string operation);

        ~ScopedLog();

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
    auto format_timestamp() const -> std::string;

    /**
     * @brief Format duration in human-readable form
     */
    static auto format_duration(double ms) -> std::string;

    /**
     * @brief Get effective log level for a source
     */
    auto effective_level(const std::string& source) const -> LogLevel;

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
                       const std::string& message) -> void;

    /**
     * @brief Write raw text to log file
     */
    auto write_to_file_raw(const std::string& text) -> void;
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

    /**
     * @brief Print a fit results table
     */
    static auto print_fit_results_table(
        const std::string& title,
        const std::vector<std::tuple<std::string, std::string, std::string>>& rows
    ) -> void {
        if (s_instance) s_instance->print_fit_results_table(title, rows);
    }

    /**
     * @brief Print a yield summary table
     */
    static auto print_yield_summary_table(
        const std::string& title,
        const std::vector<std::string>& columns,
        const std::vector<std::vector<std::string>>& rows,
        const std::string& footer = ""
    ) -> void {
        if (s_instance) s_instance->print_yield_summary_table(title, columns, rows, footer);
    }

    /**
     * @brief Print run metadata summary
     */
    static auto print_run_metadata(
        const std::string& title,
        size_t total_runs,
        const std::vector<int>& runs
    ) -> void {
        if (s_instance) s_instance->print_run_metadata(title, total_runs, runs);
    }

   private:
    static inline LoggingService* s_instance = nullptr;
};

}  // namespace clas12::framework
