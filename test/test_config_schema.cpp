/**
 * @file test_config_schema.cpp
 * @brief Unit tests for ConfigSchema compile-time configuration validation
 *
 * Tests the FixedString, ConfigKey, and ConfigSchema components.
 */

#include <gtest/gtest.h>

#include <clas12/framework/core/ConfigSchema.hpp>

#include <sstream>

using namespace clas12::framework;

// ============================================================================
// FixedString Tests
// ============================================================================

TEST(FixedStringTest, ConstructFromLiteral) {
    constexpr FixedString fs{"hello"};
    EXPECT_EQ(fs.view(), "hello");
    EXPECT_EQ(fs.size(), 5);
}

TEST(FixedStringTest, CStr) {
    constexpr FixedString fs{"test"};
    EXPECT_STREQ(fs.c_str(), "test");
}

TEST(FixedStringTest, ToStdString) {
    FixedString fs{"world"};
    std::string s = fs.str();
    EXPECT_EQ(s, "world");
}

TEST(FixedStringTest, EqualityWithSameSize) {
    constexpr FixedString a{"abc"};
    constexpr FixedString b{"abc"};
    constexpr FixedString c{"xyz"};

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
}

TEST(FixedStringTest, EqualityWithDifferentSize) {
    constexpr FixedString a{"abc"};
    constexpr FixedString b{"abcd"};

    EXPECT_FALSE(a == b);
}

TEST(FixedStringTest, EqualityWithStringView) {
    constexpr FixedString fs{"hello"};
    EXPECT_TRUE(fs == std::string_view("hello"));
    EXPECT_FALSE(fs == std::string_view("world"));
}

TEST(FixedStringTest, CompileTimeUsage) {
    // Verify FixedString can be used at compile time
    constexpr FixedString fs{"compile_time"};
    static_assert(fs.size() == 12);
    static_assert(fs.view() == "compile_time");
}

// ============================================================================
// ConfigKey Tests
// ============================================================================

TEST(ConfigKeyTest, KeyView) {
    using Key = ConfigKey<"application.name", std::string>;
    EXPECT_EQ(Key::key_view(), "application.name");
}

TEST(ConfigKeyTest, KeyString) {
    using Key = ConfigKey<"output.file", std::string>;
    EXPECT_EQ(Key::key_string(), "output.file");
}

TEST(ConfigKeyTest, ValueType) {
    using StringKey = ConfigKey<"name", std::string>;
    using IntKey = ConfigKey<"count", int>;
    using DoubleKey = ConfigKey<"value", double>;

    static_assert(std::is_same_v<StringKey::value_type, std::string>);
    static_assert(std::is_same_v<IntKey::value_type, int>);
    static_assert(std::is_same_v<DoubleKey::value_type, double>);
}

// ============================================================================
// ConfigSchema Validation Tests
// ============================================================================

class ConfigSchemaTest : public ::testing::Test {
protected:
    // Define a test schema
    using TestSchema = ConfigSchema<
        ConfigKey<"application.name", std::string>,
        ConfigKey<"application.threads", int>,
        ConfigKey<"output.file", std::string>,
        ConfigKey<"settings.enabled", bool>,
        ConfigKey<"settings.threshold", double>
    >;

    toml::table valid_config;
    toml::table partial_config;
    toml::table invalid_type_config;

    void SetUp() override {
        // Valid configuration with all keys
        valid_config = toml::parse(R"(
            [application]
            name = "TestApp"
            threads = 4

            [output]
            file = "output.root"

            [settings]
            enabled = true
            threshold = 0.5
        )");

        // Partial configuration (missing some keys)
        partial_config = toml::parse(R"(
            [application]
            name = "TestApp"
        )");

        // Configuration with wrong types
        invalid_type_config = toml::parse(R"(
            [application]
            name = 123
            threads = "not a number"

            [output]
            file = true

            [settings]
            enabled = "yes"
            threshold = "high"
        )");
    }
};

TEST_F(ConfigSchemaTest, ValidConfigPasses) {
    auto result = TestSchema::validate(valid_config);
    EXPECT_TRUE(result.is_success()) << result.message();
}

TEST_F(ConfigSchemaTest, PartialConfigPassesWithoutRequireAll) {
    auto result = TestSchema::validate(partial_config, false);
    EXPECT_TRUE(result.is_success()) << result.message();
}

TEST_F(ConfigSchemaTest, PartialConfigFailsWithRequireAll) {
    auto result = TestSchema::validate(partial_config, true);
    EXPECT_TRUE(result.is_failure());
    EXPECT_TRUE(result.message().find("required key not found") != std::string::npos);
}

TEST_F(ConfigSchemaTest, InvalidTypesFail) {
    auto result = TestSchema::validate(invalid_type_config);
    EXPECT_TRUE(result.is_failure());
    EXPECT_TRUE(result.message().find("type mismatch") != std::string::npos);
}

TEST_F(ConfigSchemaTest, DetailedValidationReturnsAllErrors) {
    auto errors = TestSchema::validate_detailed(invalid_type_config);
    EXPECT_GE(errors.size(), 1);

    // Check that errors have proper formatting
    for (const auto& error : errors) {
        EXPECT_FALSE(error.key.empty());
        EXPECT_FALSE(error.message.empty());
        auto formatted = error.format();
        EXPECT_FALSE(formatted.empty());
    }
}

TEST_F(ConfigSchemaTest, HasKey) {
    EXPECT_TRUE(TestSchema::has_key(valid_config, "application.name"));
    EXPECT_TRUE(TestSchema::has_key(valid_config, "settings.enabled"));
    EXPECT_FALSE(TestSchema::has_key(valid_config, "nonexistent.key"));
}

TEST_F(ConfigSchemaTest, KeyNames) {
    auto names = TestSchema::key_names();
    EXPECT_EQ(names.size(), 5);
    EXPECT_TRUE(std::find(names.begin(), names.end(), "application.name") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "application.threads") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "output.file") != names.end());
}

// ============================================================================
// RequiredConfigSchema Tests
// ============================================================================

TEST(RequiredConfigSchemaTest, AllKeysMustBePresent) {
    using Schema = RequiredConfigSchema<
        ConfigKey<"name", std::string>,
        ConfigKey<"value", int>
    >;

    auto complete = toml::parse(R"(
        name = "test"
        value = 42
    )");

    auto incomplete = toml::parse(R"(
        name = "test"
    )");

    EXPECT_TRUE(Schema::validate(complete).is_success());
    EXPECT_TRUE(Schema::validate(incomplete).is_failure());
}

// ============================================================================
// Array Type Validation Tests
// ============================================================================

TEST(ConfigSchemaArrayTest, IntegerArray) {
    using Schema = ConfigSchema<ConfigKey<"values", std::vector<int>>>;

    auto valid = toml::parse("values = [1, 2, 3, 4, 5]");
    auto invalid = toml::parse("values = [1, \"two\", 3]");

    EXPECT_TRUE(Schema::validate(valid).is_success());
    EXPECT_TRUE(Schema::validate(invalid).is_failure());
}

TEST(ConfigSchemaArrayTest, DoubleArray) {
    using Schema = ConfigSchema<ConfigKey<"values", std::vector<double>>>;

    auto valid = toml::parse("values = [1.0, 2.5, 3.14]");
    auto mixed = toml::parse("values = [1, 2.5, 3]");  // Integers should be accepted
    auto invalid = toml::parse("values = [1.0, \"two\", 3.0]");

    EXPECT_TRUE(Schema::validate(valid).is_success());
    EXPECT_TRUE(Schema::validate(mixed).is_success());
    EXPECT_TRUE(Schema::validate(invalid).is_failure());
}

TEST(ConfigSchemaArrayTest, StringArray) {
    using Schema = ConfigSchema<ConfigKey<"names", std::vector<std::string>>>;

    auto valid = toml::parse(R"(names = ["alice", "bob", "charlie"])");
    auto invalid = toml::parse("names = [\"alice\", 123, \"charlie\"]");

    EXPECT_TRUE(Schema::validate(valid).is_success());
    EXPECT_TRUE(Schema::validate(invalid).is_failure());
}

// ============================================================================
// Convenience Type Alias Tests
// ============================================================================

TEST(ConvenienceAliasTest, StringKey) {
    using Key = StringKey<"name">;
    static_assert(std::is_same_v<Key::value_type, std::string>);
    EXPECT_EQ(Key::key_view(), "name");
}

TEST(ConvenienceAliasTest, IntKey) {
    using Key = IntKey<"count">;
    static_assert(std::is_same_v<Key::value_type, int>);
    EXPECT_EQ(Key::key_view(), "count");
}

TEST(ConvenienceAliasTest, DoubleKey) {
    using Key = DoubleKey<"threshold">;
    static_assert(std::is_same_v<Key::value_type, double>);
    EXPECT_EQ(Key::key_view(), "threshold");
}

TEST(ConvenienceAliasTest, BoolKey) {
    using Key = BoolKey<"enabled">;
    static_assert(std::is_same_v<Key::value_type, bool>);
    EXPECT_EQ(Key::key_view(), "enabled");
}

TEST(ConvenienceAliasTest, ArrayKeys) {
    using SKey = StringArrayKey<"files">;
    using IKey = IntArrayKey<"ids">;
    using DKey = DoubleArrayKey<"values">;

    static_assert(std::is_same_v<SKey::value_type, std::vector<std::string>>);
    static_assert(std::is_same_v<IKey::value_type, std::vector<int>>);
    static_assert(std::is_same_v<DKey::value_type, std::vector<double>>);
}

// ============================================================================
// Nested Key Navigation Tests
// ============================================================================

TEST(NestedKeyTest, DeepNesting) {
    using Schema = ConfigSchema<
        ConfigKey<"level1.level2.level3.value", int>
    >;

    auto valid = toml::parse(R"(
        [level1.level2.level3]
        value = 42
    )");

    auto missing = toml::parse(R"(
        [level1.level2]
        other = 42
    )");

    EXPECT_TRUE(Schema::validate(valid).is_success());
    // Missing key should pass without require_all
    EXPECT_TRUE(Schema::validate(missing, false).is_success());
    // But fail with require_all
    EXPECT_TRUE(Schema::validate(missing, true).is_failure());
}

// ============================================================================
// ValidationError Formatting Tests
// ============================================================================

TEST(ValidationErrorTest, FormatWithTypes) {
    ValidationError error{
        .key = "application.threads",
        .message = "type mismatch",
        .expected_type = "integer",
        .actual_type = "string"
    };

    auto formatted = error.format();
    EXPECT_TRUE(formatted.find("application.threads") != std::string::npos);
    EXPECT_TRUE(formatted.find("type mismatch") != std::string::npos);
    EXPECT_TRUE(formatted.find("integer") != std::string::npos);
    EXPECT_TRUE(formatted.find("string") != std::string::npos);
}

TEST(ValidationErrorTest, FormatWithoutTypes) {
    ValidationError error{
        .key = "missing.key",
        .message = "required key not found",
        .expected_type = "",
        .actual_type = ""
    };

    auto formatted = error.format();
    EXPECT_TRUE(formatted.find("missing.key") != std::string::npos);
    EXPECT_TRUE(formatted.find("required key not found") != std::string::npos);
}

// ============================================================================
// Empty Schema Tests
// ============================================================================

TEST(EmptySchemaTest, EmptySchemaAlwaysPasses) {
    using EmptySchema = ConfigSchema<>;

    auto any_config = toml::parse(R"(
        [anything]
        goes = "here"
    )");

    EXPECT_TRUE(EmptySchema::validate(any_config).is_success());
    EXPECT_TRUE(EmptySchema::validate(any_config, true).is_success());
    EXPECT_EQ(EmptySchema::key_count, 0);
}
