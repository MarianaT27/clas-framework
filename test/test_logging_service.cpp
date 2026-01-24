#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <clas12/framework/services/LoggingService.hpp>

using namespace clas12::framework;

namespace {

// Helper to read file contents
auto read_file_contents(const std::string& path) -> std::string {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
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

// Test: LoggingService initialization and finalization
TEST(LoggingServiceTest, InitializeAndFinalize) {
    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);

    auto init_status = log_svc.initialize();
    EXPECT_TRUE(init_status.is_success());

    auto final_status = log_svc.finalize();
    EXPECT_TRUE(final_status.is_success());
}

// Test: Log level filtering - messages below threshold are suppressed
TEST(LoggingServiceTest, LevelFiltering) {
    TempLogFile temp_file;

    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);
    log_svc.set_log_level(LogLevel::WARNING);
    log_svc.set_log_file(temp_file.path());

    // These should be suppressed
    log_svc.debug("source", "debug message");
    log_svc.info("source", "info message");

    // These should appear
    log_svc.warning("source", "warning message");
    log_svc.error("source", "error message");
    log_svc.critical("source", "critical message");

    std::string content = read_file_contents(temp_file.path());

    // Suppressed messages should not appear
    EXPECT_EQ(content.find("debug message"), std::string::npos);
    EXPECT_EQ(content.find("info message"), std::string::npos);

    // These should appear
    EXPECT_NE(content.find("warning message"), std::string::npos);
    EXPECT_NE(content.find("error message"), std::string::npos);
    EXPECT_NE(content.find("critical message"), std::string::npos);
}

// Test: File output writes formatted messages
TEST(LoggingServiceTest, FileOutput) {
    TempLogFile temp_file;

    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);
    log_svc.set_log_level(LogLevel::DEBUG);
    log_svc.set_log_file(temp_file.path());

    log_svc.info("TestSource", "Test message with value {}", 42);

    std::string content = read_file_contents(temp_file.path());

    // Verify file contains the message
    EXPECT_NE(content.find("Test message with value 42"), std::string::npos);
    EXPECT_NE(content.find("TestSource"), std::string::npos);
    EXPECT_NE(content.find("INFO"), std::string::npos);
}

// Test: Log message format includes timestamp, level, and source
TEST(LoggingServiceTest, MessageFormat) {
    TempLogFile temp_file;

    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);
    log_svc.set_log_level(LogLevel::DEBUG);
    log_svc.set_log_file(temp_file.path());

    log_svc.error("MyAlgorithm", "Something went wrong");

    std::string content = read_file_contents(temp_file.path());

    // Verify timestamp pattern [HH:MM:SS.mmm]
    std::regex timestamp_pattern(R"(\[\d{2}:\d{2}:\d{2}\.\d{3}\])");
    EXPECT_TRUE(std::regex_search(content, timestamp_pattern));

    // Verify level string
    EXPECT_NE(content.find("ERROR"), std::string::npos);

    // Verify source
    EXPECT_NE(content.find("MyAlgorithm"), std::string::npos);

    // Verify message
    EXPECT_NE(content.find("Something went wrong"), std::string::npos);
}

// Test: All log levels produce correct level strings
TEST(LoggingServiceTest, AllLogLevels) {
    TempLogFile temp_file;

    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);
    log_svc.set_log_level(LogLevel::DEBUG);
    log_svc.set_log_file(temp_file.path());

    log_svc.debug("src", "debug");
    log_svc.info("src", "info");
    log_svc.warning("src", "warning");
    log_svc.error("src", "error");
    log_svc.critical("src", "critical");

    std::string content = read_file_contents(temp_file.path());

    EXPECT_NE(content.find("DEBUG"), std::string::npos);
    EXPECT_NE(content.find("INFO"), std::string::npos);
    EXPECT_NE(content.find("WARN"), std::string::npos);
    EXPECT_NE(content.find("ERROR"), std::string::npos);
    EXPECT_NE(content.find("CRIT"), std::string::npos);
}

// Test: Get and set log level
TEST(LoggingServiceTest, GetSetLogLevel) {
    LoggingService log_svc("TestLogging");

    // Default level is INFO
    EXPECT_EQ(log_svc.get_log_level(), LogLevel::INFO);

    log_svc.set_log_level(LogLevel::ERROR);
    EXPECT_EQ(log_svc.get_log_level(), LogLevel::ERROR);

    log_svc.set_log_level(LogLevel::DEBUG);
    EXPECT_EQ(log_svc.get_log_level(), LogLevel::DEBUG);
}

// Test: Format string with multiple arguments
TEST(LoggingServiceTest, FormatStringWithArgs) {
    TempLogFile temp_file;

    LoggingService log_svc("TestLogging");
    log_svc.set_console_output(false);
    log_svc.set_log_level(LogLevel::DEBUG);
    log_svc.set_log_file(temp_file.path());

    log_svc.info("TestSource", "Processing event {} of {} with value {:.2f}",
                 5, 100, 3.14159);

    std::string content = read_file_contents(temp_file.path());

    EXPECT_NE(content.find("Processing event 5 of 100 with value 3.14"), std::string::npos);
}
