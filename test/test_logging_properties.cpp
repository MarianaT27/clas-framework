#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include "Framework/Services/LoggingService.hpp"

using namespace Framework;

namespace {

// Helper to read file contents
auto read_file_contents(const std::string& path) -> std::string {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper to generate a valid LogLevel
auto gen_log_level() -> rc::Gen<LogLevel> {
    return rc::gen::element(
        LogLevel::DEBUG,
        LogLevel::INFO,
        LogLevel::WARNING,
        LogLevel::ERROR,
        LogLevel::CRITICAL);
}

// Helper to get numeric value of log level
auto level_value(LogLevel level) -> int {
    return static_cast<int>(level);
}

// Temporary file RAII wrapper
class TempLogFile {
   public:
    TempLogFile() {
        m_path = std::filesystem::temp_directory_path() /
                 ("test_log_" + std::to_string(std::rand()) + ".log");
    }

    ~TempLogFile() {
        if (std::filesystem::exists(m_path)) {
            std::filesystem::remove(m_path);
        }
    }

    auto path() const -> std::string { return m_path.string(); }

   private:
    std::filesystem::path m_path;
};

}  // anonymous namespace

/**
 * **Feature: code-quality-improvements, Property 5: Log Level Filtering**
 * 
 * *For any* log message emitted at a given level, when the LoggingService 
 * minimum level is set higher than the message level, the message SHALL 
 * be suppressed (not output).
 * 
 * **Validates: Requirements 5.2**
 */
RC_GTEST_PROP(LoggingServiceProperties, LogLevelFiltering, ()) {
    // Generate random log levels for threshold and message
    LogLevel threshold_level = *gen_log_level();
    LogLevel message_level = *gen_log_level();

    // Generate a random message
    std::string message = *rc::gen::nonEmpty<std::string>();
    std::string source = *rc::gen::nonEmpty(rc::gen::container<std::string>(
        rc::gen::inRange('a', 'z')));

    // Create a temporary log file
    TempLogFile temp_file;

    // Create and configure logging service
    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);  // Disable console for testing
    log_svc.set_log_level(threshold_level);
    log_svc.set_log_file(temp_file.path());

    // Log the message at the specified level
    switch (message_level) {
        case LogLevel::DEBUG:
            log_svc.debug(source, "{}", message);
            break;
        case LogLevel::INFO:
            log_svc.info(source, "{}", message);
            break;
        case LogLevel::WARNING:
            log_svc.warning(source, "{}", message);
            break;
        case LogLevel::ERROR:
            log_svc.error(source, "{}", message);
            break;
        case LogLevel::CRITICAL:
            log_svc.critical(source, "{}", message);
            break;
    }

    // Read the log file
    std::string log_content = read_file_contents(temp_file.path());

    // Property: message should appear if and only if message_level >= threshold_level
    bool should_appear = level_value(message_level) >= level_value(threshold_level);
    bool does_appear = !log_content.empty();

    RC_ASSERT(should_appear == does_appear);

    // If message appears, verify it contains the expected content
    if (does_appear) {
        RC_ASSERT(log_content.find(message) != std::string::npos);
        RC_ASSERT(log_content.find(source) != std::string::npos);
    }
}

/**
 * **Feature: code-quality-improvements, Property 6: Log Message Format Completeness**
 * 
 * *For any* log message emitted through the LoggingService, the formatted 
 * output SHALL contain the timestamp, log level string, and source component name.
 * 
 * **Validates: Requirements 5.1, 5.3**
 */
RC_GTEST_PROP(LoggingServiceProperties, LogMessageFormatCompleteness, ()) {
    // Generate random log level (excluding DEBUG to ensure message is logged at INFO default)
    LogLevel message_level = *rc::gen::element(
        LogLevel::INFO,
        LogLevel::WARNING,
        LogLevel::ERROR,
        LogLevel::CRITICAL);

    // Generate a random message and source
    std::string message = *rc::gen::nonEmpty(rc::gen::container<std::string>(
        rc::gen::inRange('a', 'z')));
    std::string source = *rc::gen::nonEmpty(rc::gen::container<std::string>(
        rc::gen::inRange('a', 'z')));

    // Create a temporary log file
    TempLogFile temp_file;

    // Create and configure logging service
    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);       // Disable console for testing
    log_svc.set_log_level(LogLevel::DEBUG);  // Allow all messages
    log_svc.set_log_file(temp_file.path());

    // Log the message at the specified level
    switch (message_level) {
        case LogLevel::DEBUG:
            log_svc.debug(source, "{}", message);
            break;
        case LogLevel::INFO:
            log_svc.info(source, "{}", message);
            break;
        case LogLevel::WARNING:
            log_svc.warning(source, "{}", message);
            break;
        case LogLevel::ERROR:
            log_svc.error(source, "{}", message);
            break;
        case LogLevel::CRITICAL:
            log_svc.critical(source, "{}", message);
            break;
    }

    // Read the log file
    std::string log_content = read_file_contents(temp_file.path());

    // Property 1: Log output should not be empty
    RC_ASSERT(!log_content.empty());

    // Property 2: Log output should contain timestamp pattern [HH:MM:SS.mmm]
    std::regex timestamp_pattern(R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\])");
    RC_ASSERT(std::regex_search(log_content, timestamp_pattern));

    // Property 3: Log output should contain the log level string
    std::string level_str;
    switch (message_level) {
        case LogLevel::DEBUG:
            level_str = "DEBUG";
            break;
        case LogLevel::INFO:
            level_str = "INFO";
            break;
        case LogLevel::WARNING:
            level_str = "WARN";
            break;
        case LogLevel::ERROR:
            level_str = "ERROR";
            break;
        case LogLevel::CRITICAL:
            level_str = "CRIT";
            break;
    }
    RC_ASSERT(log_content.find(level_str) != std::string::npos);

    // Property 4: Log output should contain the source component name
    RC_ASSERT(log_content.find(source) != std::string::npos);

    // Property 5: Log output should contain the message
    RC_ASSERT(log_content.find(message) != std::string::npos);
}
