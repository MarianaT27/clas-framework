#pragma once

#include <fmt/format.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
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
 * @brief Centralized logging service
 *
 * Provides thread-safe logging with:
 * - Multiple log levels (DEBUG, INFO, WARNING, ERROR, CRITICAL)
 * - Console output with colors
 * - Optional file output
 * - Timestamps
 * - Source information (algorithm/service name)
 *
 * Example:
 * @code
 * auto log = service<LoggingService>("LoggingService");
 * log->info("Processing event {}", event_number);
 * log->warning("No trigger electron found");
 * log->error("Failed to read bank: {}", bank_name);
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

    /**
     * @brief Set minimum log level
     */
    auto set_log_level(LogLevel level) -> void {
        m_min_level = level;
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

    /**
     * @brief Get current log level
     */
    auto get_log_level() const -> LogLevel {
        return m_min_level;
    }

   private:
    LogLevel m_min_level = LogLevel::INFO;
    bool m_console_output = true;
    bool m_colored_output = true;
    std::string m_log_file;
    std::ofstream m_file_stream;
    mutable std::mutex m_mutex;

    /**
     * @brief Core logging function
     */
    template <typename... Args>
    auto log(LogLevel level, const std::string& source,
             fmt::format_string<Args...> fmt_str, Args&&... args) -> void {
        if (level < m_min_level) return;

        std::lock_guard<std::mutex> lock(m_mutex);

        // Format the message
        std::string message = fmt::format(fmt_str, std::forward<Args>(args)...);

        // Get timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        std::stringstream timestamp;
        timestamp << std::put_time(std::localtime(&time_t), "%H:%M:%S")
                  << '.' << std::setfill('0') << std::setw(3) << ms.count();

        // Get level string and ANSI color code
        std::string level_str;
        std::string color_code;
        switch (level) {
            case LogLevel::DEBUG:
                level_str = "DEBUG";
                color_code = "\033[90m";  // Gray
                break;
            case LogLevel::INFO:
                level_str = "INFO ";
                color_code = "\033[36m";  // Cyan
                break;
            case LogLevel::WARNING:
                level_str = "WARN ";
                color_code = "\033[33m";  // Yellow
                break;
            case LogLevel::ERROR:
                level_str = "ERROR";
                color_code = "\033[31m";  // Red
                break;
            case LogLevel::CRITICAL:
                level_str = "CRIT ";
                color_code = "\033[35m";  // Magenta
                break;
        }

        // Console output
        if (m_console_output) {
            if (m_colored_output) {
                // Use ANSI escape codes for colors
                fmt::print("\033[90m[{}]\033[0m {}{}  \033[0m\033[1m{:<20}\033[0m {}\n",
                           timestamp.str(), color_code, level_str, source, message);
            } else {
                fmt::print("[{}] {} {:<20} {}\n",
                           timestamp.str(), level_str, source, message);
            }
        }

        // File output
        if (m_file_stream.is_open()) {
            m_file_stream << "[" << timestamp.str() << "] "
                          << level_str << " "
                          << std::setw(20) << std::left << source << " "
                          << message << "\n";
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

    /**
     * @brief Log debug message
     */
    template <typename... Args>
    static auto debug(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) s_instance->debug(source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log info message
     */
    template <typename... Args>
    static auto info(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) s_instance->info(source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log warning message
     */
    template <typename... Args>
    static auto warning(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) s_instance->warning(source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log error message
     */
    template <typename... Args>
    static auto error(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) s_instance->error(source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log critical message
     */
    template <typename... Args>
    static auto critical(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        if (s_instance) s_instance->critical(source, fmt, std::forward<Args>(args)...);
    }

   private:
    static inline LoggingService* s_instance = nullptr;
};

}  // namespace Framework
