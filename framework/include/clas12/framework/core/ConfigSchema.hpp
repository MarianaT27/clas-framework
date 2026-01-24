#pragma once

/**
 * @file ConfigSchema.hpp
 * @brief Compile-time configuration validation using C++20 features
 *
 * This file provides:
 * - FixedString: Compile-time string type for template parameters
 * - ConfigKey: Type-safe configuration key with associated value type
 * - ConfigSchema: Compile-time schema validation for TOML configuration
 *
 * @see Requirements 11.1, 11.2, 11.3, 11.4
 */

#include <algorithm>
#include <array>
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <fmt/format.h>
#include <toml++/toml.hpp>

#include <clas12/framework/core/StatusCode.hpp>

namespace clas12::framework {

// ============================================================================
// FixedString - Compile-time string for template parameters (C++20)
// ============================================================================

/**
 * @brief Compile-time fixed-length string for use as template parameters
 *
 * FixedString enables using string literals as non-type template parameters,
 * which is a C++20 feature. This allows compile-time validation of
 * configuration keys.
 *
 * Example usage:
 * @code
 * template <FixedString Key>
 * struct ConfigEntry {
 *     static constexpr auto key() { return Key.view(); }
 * };
 *
 * // Usage with string literal
 * using AppName = ConfigEntry<"application.name">;
 * static_assert(AppName::key() == "application.name");
 * @endcode
 *
 * @tparam N Size of the string including null terminator
 *
 * @see Requirement 11.1
 */
template <std::size_t N>
struct FixedString {
    std::array<char, N> data{};

    /**
     * @brief Construct from string literal
     * @param str String literal to copy
     */
    constexpr FixedString(const char (&str)[N]) {
        std::copy_n(str, N, data.begin());
    }

    /**
     * @brief Get the string as a null-terminated C string
     * @return Pointer to the string data
     */
    [[nodiscard]] constexpr auto c_str() const noexcept -> const char* {
        return data.data();
    }

    /**
     * @brief Get the string as a string_view (excludes null terminator)
     * @return String view of the content
     */
    [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
        return std::string_view(data.data(), N - 1);
    }

    /**
     * @brief Get the length of the string (excluding null terminator)
     * @return String length
     */
    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        return N - 1;
    }

    /**
     * @brief Convert to std::string (runtime operation)
     * @return String copy
     */
    [[nodiscard]] auto str() const -> std::string {
        return std::string(data.data(), N - 1);
    }

    /**
     * @brief Equality comparison with another FixedString
     */
    template <std::size_t M>
    [[nodiscard]] constexpr auto operator==(const FixedString<M>& other) const noexcept -> bool {
        if constexpr (N != M) {
            return false;
        } else {
            return data == other.data;
        }
    }

    /**
     * @brief Equality comparison with string_view
     */
    [[nodiscard]] constexpr auto operator==(std::string_view sv) const noexcept -> bool {
        return view() == sv;
    }

    /**
     * @brief Three-way comparison for ordering
     */
    [[nodiscard]] constexpr auto operator<=>(const FixedString& other) const noexcept = default;
};

// Deduction guide for FixedString
template <std::size_t N>
FixedString(const char (&)[N]) -> FixedString<N>;

// ============================================================================
// ConfigKey - Type-safe configuration key
// ============================================================================

/**
 * @brief Compile-time configuration key with associated value type
 *
 * ConfigKey associates a string key with a specific value type at compile time.
 * This enables type-safe configuration access and validation.
 *
 * Example usage:
 * @code
 * // Define configuration keys
 * using AppNameKey = ConfigKey<"application.name", std::string>;
 * using ThreadsKey = ConfigKey<"application.threads", int>;
 * using OutputKey = ConfigKey<"output.file", std::string>;
 *
 * // Use in schema
 * using MySchema = ConfigSchema<AppNameKey, ThreadsKey, OutputKey>;
 * @endcode
 *
 * @tparam Key The configuration key as a FixedString
 * @tparam T The expected value type
 *
 * @see Requirement 11.1
 */
template <FixedString Key, typename T>
struct ConfigKey {
    /// The configuration key string
    static constexpr auto key = Key;

    /// The expected value type
    using value_type = T;

    /**
     * @brief Get the key as a string_view
     * @return String view of the key
     */
    [[nodiscard]] static constexpr auto key_view() noexcept -> std::string_view {
        return Key.view();
    }

    /**
     * @brief Get the key as a std::string
     * @return String copy of the key
     */
    [[nodiscard]] static auto key_string() -> std::string {
        return Key.str();
    }
};

// ============================================================================
// Type traits for TOML type checking
// ============================================================================

namespace detail {

/**
 * @brief Check if a TOML node matches the expected type
 */
template <typename T>
[[nodiscard]] auto check_toml_type(const toml::node& node) -> bool {
    if constexpr (std::is_same_v<T, bool>) {
        return node.is_boolean();
    } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int64_t>) {
        return node.is_integer();
    } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
        return node.is_floating_point() || node.is_integer();
    } else if constexpr (std::is_same_v<T, std::string>) {
        return node.is_string();
    } else if constexpr (std::is_same_v<T, std::vector<int>> ||
                         std::is_same_v<T, std::vector<int64_t>>) {
        if (!node.is_array()) return false;
        const auto* arr = node.as_array();
        return arr && std::all_of(arr->begin(), arr->end(),
                                  [](const auto& elem) { return elem.is_integer(); });
    } else if constexpr (std::is_same_v<T, std::vector<double>> ||
                         std::is_same_v<T, std::vector<float>>) {
        if (!node.is_array()) return false;
        const auto* arr = node.as_array();
        return arr && std::all_of(arr->begin(), arr->end(),
                                  [](const auto& elem) {
                                      return elem.is_floating_point() || elem.is_integer();
                                  });
    } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        if (!node.is_array()) return false;
        const auto* arr = node.as_array();
        return arr && std::all_of(arr->begin(), arr->end(),
                                  [](const auto& elem) { return elem.is_string(); });
    } else {
        // For other types (tables, etc.), just check existence
        return true;
    }
}

/**
 * @brief Get the expected type name for error messages
 */
template <typename T>
[[nodiscard]] constexpr auto type_name() -> std::string_view {
    if constexpr (std::is_same_v<T, bool>) {
        return "boolean";
    } else if constexpr (std::is_same_v<T, int>) {
        return "integer";
    } else if constexpr (std::is_same_v<T, int64_t>) {
        return "integer (64-bit)";
    } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
        return "number";
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "string";
    } else if constexpr (std::is_same_v<T, std::vector<int>> ||
                         std::is_same_v<T, std::vector<int64_t>>) {
        return "array of integers";
    } else if constexpr (std::is_same_v<T, std::vector<double>> ||
                         std::is_same_v<T, std::vector<float>>) {
        return "array of numbers";
    } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
        return "array of strings";
    } else {
        return "unknown";
    }
}

/**
 * @brief Navigate to a nested key in a TOML table
 *
 * Supports dotted keys like "application.name" by traversing nested tables.
 *
 * @param table The root TOML table
 * @param key The dotted key path
 * @return Pointer to the node, or nullptr if not found
 */
[[nodiscard]] inline auto navigate_to_key(const toml::table& table,
                                          std::string_view key) -> const toml::node* {
    const toml::table* current = &table;
    std::string_view remaining = key;

    while (!remaining.empty()) {
        auto dot_pos = remaining.find('.');
        std::string_view segment;

        if (dot_pos == std::string_view::npos) {
            segment = remaining;
            remaining = {};
        } else {
            segment = remaining.substr(0, dot_pos);
            remaining = remaining.substr(dot_pos + 1);
        }

        auto node = current->get(segment);
        if (!node) {
            return nullptr;
        }

        if (remaining.empty()) {
            return node;
        }

        if (!node->is_table()) {
            return nullptr;
        }

        current = node->as_table();
    }

    return nullptr;
}

}  // namespace detail

// ============================================================================
// ValidationError - Detailed error information
// ============================================================================

/**
 * @brief Represents a configuration validation error
 */
struct ValidationError {
    std::string key;
    std::string message;
    std::string expected_type;
    std::string actual_type;

    /**
     * @brief Format the error for display
     * @return Formatted error message
     */
    [[nodiscard]] auto format() const -> std::string {
        if (!expected_type.empty() && !actual_type.empty()) {
            return fmt::format("Configuration key '{}': {} (expected {}, got {})",
                               key, message, expected_type, actual_type);
        }
        return fmt::format("Configuration key '{}': {}", key, message);
    }
};

// ============================================================================
// ConfigSchema - Compile-time schema validation
// ============================================================================

/**
 * @brief Compile-time configuration schema for validation
 *
 * ConfigSchema validates TOML configuration against a set of expected keys
 * and their types. It provides:
 * - Key existence checking
 * - Type validation
 * - Descriptive error messages
 *
 * Example usage:
 * @code
 * // Define the schema
 * using MySchema = ConfigSchema<
 *     ConfigKey<"application.name", std::string>,
 *     ConfigKey<"application.threads", int>,
 *     ConfigKey<"output.file", std::string>
 * >;
 *
 * // Validate configuration
 * auto config = toml::parse_file("config.toml");
 * auto result = MySchema::validate(config);
 * if (result.is_failure()) {
 *     std::cerr << result.message() << std::endl;
 * }
 * @endcode
 *
 * @tparam Keys ConfigKey types defining the expected schema
 *
 * @see Requirement 11.4
 */
template <typename... Keys>
struct ConfigSchema {
    /// Number of keys in the schema
    static constexpr std::size_t key_count = sizeof...(Keys);

    /**
     * @brief Validate a TOML table against this schema
     *
     * Checks that all required keys exist and have the correct types.
     *
     * @param config The TOML table to validate
     * @param require_all If true, all keys must be present; if false, only
     *                    validate keys that exist
     * @return StatusCode SUCCESS if valid, FAILURE with descriptive message otherwise
     */
    [[nodiscard]] static auto validate(const toml::table& config,
                                       bool require_all = false) -> StatusCode {
        std::vector<ValidationError> errors;

        // Validate each key
        (validate_key<Keys>(config, require_all, errors), ...);

        if (errors.empty()) {
            return StatusCode::SUCCESS();
        }

        // Format all errors into a single message
        std::string message = "Configuration validation failed:\n";
        for (const auto& error : errors) {
            message += "  - " + error.format() + "\n";
        }

        return StatusCode::failure(message);
    }

    /**
     * @brief Validate and return detailed errors
     *
     * @param config The TOML table to validate
     * @param require_all If true, all keys must be present
     * @return Vector of validation errors (empty if valid)
     */
    [[nodiscard]] static auto validate_detailed(const toml::table& config,
                                                bool require_all = false)
        -> std::vector<ValidationError> {
        std::vector<ValidationError> errors;
        (validate_key<Keys>(config, require_all, errors), ...);
        return errors;
    }

    /**
     * @brief Check if a specific key exists in the configuration
     *
     * @param config The TOML table to check
     * @param key The key to look for
     * @return true if the key exists
     */
    [[nodiscard]] static auto has_key(const toml::table& config,
                                      std::string_view key) -> bool {
        return detail::navigate_to_key(config, key) != nullptr;
    }

    /**
     * @brief Get all key names in this schema
     *
     * @return Vector of key names
     */
    [[nodiscard]] static auto key_names() -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(key_count);
        (names.push_back(Keys::key_string()), ...);
        return names;
    }

private:
    /**
     * @brief Validate a single key
     */
    template <typename Key>
    static auto validate_key(const toml::table& config,
                             bool require_all,
                             std::vector<ValidationError>& errors) -> void {
        auto key_str = Key::key_view();
        auto* node = detail::navigate_to_key(config, key_str);

        if (!node) {
            if (require_all) {
                errors.push_back({
                    .key = std::string(key_str),
                    .message = "required key not found",
                    .expected_type = std::string(detail::type_name<typename Key::value_type>()),
                    .actual_type = ""
                });
            }
            return;
        }

        // Check type
        if (!detail::check_toml_type<typename Key::value_type>(*node)) {
            errors.push_back({
                .key = std::string(key_str),
                .message = "type mismatch",
                .expected_type = std::string(detail::type_name<typename Key::value_type>()),
                .actual_type = get_actual_type(*node)
            });
        }
    }

    /**
     * @brief Get the actual TOML type name for error messages
     */
    [[nodiscard]] static auto get_actual_type(const toml::node& node) -> std::string {
        if (node.is_boolean()) return "boolean";
        if (node.is_integer()) return "integer";
        if (node.is_floating_point()) return "floating-point";
        if (node.is_string()) return "string";
        if (node.is_array()) return "array";
        if (node.is_table()) return "table";
        if (node.is_date()) return "date";
        if (node.is_time()) return "time";
        if (node.is_date_time()) return "datetime";
        return "unknown";
    }
};

// ============================================================================
// RequiredConfigSchema - Schema where all keys are required
// ============================================================================

/**
 * @brief Configuration schema where all keys are required
 *
 * This is a convenience wrapper around ConfigSchema that always requires
 * all keys to be present.
 *
 * @tparam Keys ConfigKey types defining the required schema
 */
template <typename... Keys>
struct RequiredConfigSchema {
    /**
     * @brief Validate that all keys exist and have correct types
     *
     * @param config The TOML table to validate
     * @return StatusCode SUCCESS if valid, FAILURE with message otherwise
     */
    [[nodiscard]] static auto validate(const toml::table& config) -> StatusCode {
        return ConfigSchema<Keys...>::validate(config, true);
    }

    /**
     * @brief Validate and return detailed errors
     */
    [[nodiscard]] static auto validate_detailed(const toml::table& config)
        -> std::vector<ValidationError> {
        return ConfigSchema<Keys...>::validate_detailed(config, true);
    }
};

// ============================================================================
// Convenience type aliases
// ============================================================================

/**
 * @brief Create a string configuration key
 */
template <FixedString Key>
using StringKey = ConfigKey<Key, std::string>;

/**
 * @brief Create an integer configuration key
 */
template <FixedString Key>
using IntKey = ConfigKey<Key, int>;

/**
 * @brief Create a 64-bit integer configuration key
 */
template <FixedString Key>
using Int64Key = ConfigKey<Key, int64_t>;

/**
 * @brief Create a double configuration key
 */
template <FixedString Key>
using DoubleKey = ConfigKey<Key, double>;

/**
 * @brief Create a boolean configuration key
 */
template <FixedString Key>
using BoolKey = ConfigKey<Key, bool>;

/**
 * @brief Create a string array configuration key
 */
template <FixedString Key>
using StringArrayKey = ConfigKey<Key, std::vector<std::string>>;

/**
 * @brief Create an integer array configuration key
 */
template <FixedString Key>
using IntArrayKey = ConfigKey<Key, std::vector<int>>;

/**
 * @brief Create a double array configuration key
 */
template <FixedString Key>
using DoubleArrayKey = ConfigKey<Key, std::vector<double>>;

}  // namespace clas12::framework
