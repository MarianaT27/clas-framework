#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Framework/Services/ConfigService.hpp"

using namespace Framework;

namespace {
// Helper to create a temporary config file
class TempConfigFile {
   public:
    explicit TempConfigFile(const std::string& content) {
        // Create a unique temp file path
        m_path = std::filesystem::temp_directory_path() /
                 ("test_config_" + std::to_string(std::rand()) + ".toml");

        std::ofstream ofs(m_path);
        ofs << content;
        ofs.close();
    }

    ~TempConfigFile() {
        if (std::filesystem::exists(m_path)) {
            std::filesystem::remove(m_path);
        }
    }

    auto path() const -> std::string { return m_path.string(); }

   private:
    std::filesystem::path m_path;
};

// Generate valid TOML key (alphanumeric, starting with letter)
auto genValidKey() -> rc::Gen<std::string> {
    return rc::gen::map(
        rc::gen::pair(
            rc::gen::inRange('a', 'z'),  // First char must be a letter
            rc::gen::container<std::string>(rc::gen::inRange('a', 'z'))),
        [](const std::pair<char, std::string>& p) {
            return std::string(1, p.first) + p.second;
        });
}
}  // namespace

/**
 * **Feature: code-quality-improvements, Property 1: Configuration Round-Trip Consistency**
 * 
 * *For any* valid TOML configuration containing typed values (integers, doubles, strings, booleans),
 * storing the configuration and retrieving values by key SHALL return the original values with correct types.
 * 
 * **Validates: Requirements 1.1, 1.3, 1.4**
 */
RC_GTEST_PROP(ConfigServiceProperties, IntegerRoundTrip, ()) {
    // Generate a random key and integer value
    std::string key = *genValidKey();
    int64_t value = *rc::gen::inRange<int64_t>(-1000000, 1000000);

    // Create TOML content
    std::ostringstream toml;
    toml << key << " = " << value;

    TempConfigFile temp(toml.str());
    ConfigService config(temp.path());
    auto sc = config.initialize();
    RC_ASSERT(sc.is_success());

    // Round-trip: retrieve the value
    auto result = config.get<int64_t>(key);
    RC_ASSERT(result.is_ok());
    RC_ASSERT(result.value() == value);
}

RC_GTEST_PROP(ConfigServiceProperties, DoubleRoundTrip, ()) {
    // Generate a random key and double value
    std::string key = *genValidKey();
    // Use a reasonable range to avoid floating point edge cases
    double value = *rc::gen::map(
        rc::gen::inRange<int>(-10000, 10000),
        [](int i) { return static_cast<double>(i) / 100.0; });

    // Create TOML content with sufficient precision
    std::ostringstream toml;
    toml << std::setprecision(17) << key << " = " << value;

    TempConfigFile temp(toml.str());
    ConfigService config(temp.path());
    auto sc = config.initialize();
    RC_ASSERT(sc.is_success());

    // Round-trip: retrieve the value
    auto result = config.get<double>(key);
    RC_ASSERT(result.is_ok());
    // Use approximate comparison for floating point
    RC_ASSERT(std::abs(result.value() - value) < 1e-10);
}

RC_GTEST_PROP(ConfigServiceProperties, BooleanRoundTrip, ()) {
    // Generate a random key and boolean value
    std::string key = *genValidKey();
    bool value = *rc::gen::arbitrary<bool>();

    // Create TOML content
    std::ostringstream toml;
    toml << key << " = " << (value ? "true" : "false");

    TempConfigFile temp(toml.str());
    ConfigService config(temp.path());
    auto sc = config.initialize();
    RC_ASSERT(sc.is_success());

    // Round-trip: retrieve the value
    auto result = config.get<bool>(key);
    RC_ASSERT(result.is_ok());
    RC_ASSERT(result.value() == value);
}

RC_GTEST_PROP(ConfigServiceProperties, StringRoundTrip, ()) {
    // Generate a random key and string value (printable ASCII only for TOML compatibility)
    std::string key = *genValidKey();
    std::string value = *rc::gen::container<std::string>(
        rc::gen::suchThat(
            rc::gen::inRange<char>(32, 126),
            [](char c) {
                return c != '"' && c != '\\';
            }  // Avoid escape sequences
            ));

    // Create TOML content
    std::ostringstream toml;
    toml << key << " = \"" << value << "\"";

    TempConfigFile temp(toml.str());
    ConfigService config(temp.path());
    auto sc = config.initialize();
    RC_ASSERT(sc.is_success());

    // Round-trip: retrieve the value
    auto result = config.get<std::string>(key);
    RC_ASSERT(result.is_ok());
    RC_ASSERT(result.value() == value);
}

RC_GTEST_PROP(ConfigServiceProperties, NestedKeyRoundTrip, ()) {
    // Generate nested keys and integer value
    std::string section = *genValidKey();
    std::string key = *genValidKey();
    int64_t value = *rc::gen::inRange<int64_t>(-1000, 1000);

    // Create TOML content with section
    std::ostringstream toml;
    toml << "[" << section << "]\n";
    toml << key << " = " << value;

    TempConfigFile temp(toml.str());
    ConfigService config(temp.path());
    auto sc = config.initialize();
    RC_ASSERT(sc.is_success());

    // Round-trip: retrieve the value using dot notation
    std::string full_key = section + "." + key;
    auto result = config.get<int64_t>(full_key);
    RC_ASSERT(result.is_ok());
    RC_ASSERT(result.value() == value);
}

RC_GTEST_PROP(ConfigServiceProperties, HasKeyConsistency, ()) {
    // Generate a key and value
    std::string key = *genValidKey();
    int64_t value = *rc::gen::inRange<int64_t>(-1000, 1000);

    // Create TOML content
    std::ostringstream toml;
    toml << key << " = " << value;

    TempConfigFile temp(toml.str());
    ConfigService config(temp.path());
    auto sc = config.initialize();
    RC_ASSERT(sc.is_success());

    // has() should return true for existing key
    RC_ASSERT(config.has(key));

    // has() should return false for non-existing key
    RC_ASSERT(!config.has("nonexistent_key_xyz"));
}

RC_GTEST_PROP(ConfigServiceProperties, DefaultValueFallback, ()) {
    // Generate a key and default value
    std::string key = *genValidKey();
    int64_t default_value = *rc::gen::inRange<int64_t>(-1000, 1000);

    // Create empty TOML content
    TempConfigFile temp("");
    ConfigService config(temp.path());
    auto sc = config.initialize();
    RC_ASSERT(sc.is_success());

    // get with default should return the default for missing key
    int64_t result = config.get<int64_t>(key, default_value);
    RC_ASSERT(result == default_value);
}

// Unit tests for specific edge cases and error conditions
TEST(ConfigServiceTest, FileNotFound) {
    ConfigService config("/nonexistent/path/config.toml");
    auto sc = config.initialize();

    EXPECT_TRUE(sc.is_failure());
    EXPECT_TRUE(sc.message().find("not found") != std::string::npos);
    EXPECT_TRUE(sc.message().find("/nonexistent/path/config.toml") != std::string::npos);
}

TEST(ConfigServiceTest, ParseError) {
    // Create invalid TOML - use a syntax error that tomlplusplus will catch
    TempConfigFile temp("[section\nkey = value");  // Missing closing bracket
    ConfigService config(temp.path());
    auto sc = config.initialize();

    EXPECT_TRUE(sc.is_failure());
    // The error message should contain information about the parse failure
    EXPECT_FALSE(sc.message().empty());
    EXPECT_TRUE(sc.message().find("parse") != std::string::npos ||
                sc.message().find("Parse") != std::string::npos ||
                sc.message().find("Error") != std::string::npos);
}

TEST(ConfigServiceTest, TypeMismatch) {
    TempConfigFile temp("value = \"string\"");
    ConfigService config(temp.path());
    config.initialize();

    // Try to get string as int
    auto result = config.get<int>("value");
    EXPECT_TRUE(result.is_error());
    EXPECT_TRUE(result.error_message().find("Type mismatch") != std::string::npos);
}

TEST(ConfigServiceTest, ValidationMissingKeys) {
    TempConfigFile temp("existing = 42");
    ConfigService config(temp.path());
    config.initialize();

    auto result = config.validate({"existing", "missing1", "missing2"});
    EXPECT_TRUE(result.is_error());
    EXPECT_TRUE(result.error_message().find("missing1") != std::string::npos);
    EXPECT_TRUE(result.error_message().find("missing2") != std::string::npos);
}

TEST(ConfigServiceTest, ValidationAllPresent) {
    TempConfigFile temp("key1 = 1\nkey2 = 2");
    ConfigService config(temp.path());
    config.initialize();

    auto result = config.validate({"key1", "key2"});
    EXPECT_TRUE(result.is_ok());
}

TEST(ConfigServiceTest, DeeplyNestedKeys) {
    TempConfigFile temp("[a]\n[a.b]\n[a.b.c]\nvalue = 123");
    ConfigService config(temp.path());
    config.initialize();

    auto result = config.get<int64_t>("a.b.c.value");
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), 123);
}
