#pragma once

#include "Framework/IService.hpp"
#include <fmt/format.h>
#include <fmt/color.h>
#include <fstream>
#include <string>
#include <memory>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

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
    template<typename... Args>
    auto debug(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::DEBUG, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log info message
     */
    template<typename... Args>
    auto info(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::INFO, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log warning message
     */
    template<typename... Args>
    auto warning(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::WARNING, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log error message
     */
    template<typename... Args>
    auto error(const std::string& source, fmt::format_string<Args...> fmt, Args&&... args) -> void {
        log(LogLevel::ERROR, source, fmt, std::forward<Args>(args)...);
    }

    /**
     * @brief Log critical message
     */
    template<typename... Args>
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
    template<typename... Args>
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
            now.time_since_epoch()) % 1000;

        std::stringstream timestamp;
        timestamp << std::put_time(std::localtime(&time_t), "%H:%M:%S")
                  << '.' << std::setfill('0') << std::setw(3) << ms.count();

        // Get level string and color
        std::string level_str;
        fmt::color color;
        switch (level) {
            case LogLevel::DEBUG:
                level_str = "DEBUG";
                color = fmt::color::gray;
                break;
            case LogLevel::INFO:
                level_str = "INFO ";
                color = fmt::color::cyan;
                break;
            case LogLevel::WARNING:
                level_str = "WARN ";
                color = fmt::color::yellow;
                break;
            case LogLevel::ERROR:
                level_str = "ERROR";
                color = fmt::color::red;
                break;
            case LogLevel::CRITICAL:
                level_str = "CRIT ";
                color = fmt::color::magenta;
                break;
        }

        // Console output
        if (m_console_output) {
            if (m_colored_output) {
                fmt::print(fg(fmt::color::dim_gray), "[{}] ", timestamp.str());
                fmt::print(fg(color) | fmt::emphasis::bold, "{} ", level_str);
                fmt::print(fg(fmt::color::white) | fmt::emphasis::bold, "{:<20} ", source);
                fmt::print("{}\n", message);
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
 * @brief Helper macros for logging with automatic source name
 */
#define LOG_DEBUG(log_svc, ...) \
    if (log_svc) log_svc->debug(this->name(), __VA_ARGS__)

#define LOG_INFO(log_svc, ...) \
    if (log_svc) log_svc->info(this->name(), __VA_ARGS__)

#define LOG_WARNING(log_svc, ...) \
    if (log_svc) log_svc->warning(this->name(), __VA_ARGS__)

#define LOG_ERROR(log_svc, ...) \
    if (log_svc) log_svc->error(this->name(), __VA_ARGS__)

#define LOG_CRITICAL(log_svc, ...) \
    if (log_svc) log_svc->critical(this->name(), __VA_ARGS__)

}  // namespace Framework
