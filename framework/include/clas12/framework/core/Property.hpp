#pragma once

/**
 * @file Property.hpp
 * @brief Declarative property system for algorithm configuration
 *
 * This file provides a Property<T> template class for declaring configurable
 * parameters with automatic validation, default values, and TOML integration.
 *
 * @see Requirements 2.1, 2.2, 2.3, 2.4, 2.5
 */

#include <concepts>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <toml++/toml.hpp>

#include <clas12/framework/core/Field.hpp>
#include <clas12/framework/core/StatusCode.hpp>

namespace framework {

/**
 * @brief Supported property value types
 *
 * This variant defines all types that can be stored in properties
 * and read from TOML configuration files.
 */
using PropertyValue = std::variant<
    bool,
    int,
    int64_t,
    double,
    std::string,
    std::vector<int>,
    std::vector<double>,
    std::vector<std::string>
>;

/**
 * @brief Property validation constraint function type
 *
 * A validator returns true if the value is valid, false otherwise.
 *
 * @tparam T The property value type
 */
template <typename T>
using Validator = std::function<bool(const T&)>;

/**
 * @brief Concept for numeric types that support range validation
 */
template <typename T>
concept NumericProperty = std::integral<T> || std::floating_point<T>;

/**
 * @brief A configurable property with validation and metadata
 *
 * Property<T> provides a type-safe way to declare algorithm configuration
 * parameters with:
 * - Default values
 * - Validation constraints (range, custom validators)
 * - Documentation strings
 * - Automatic TOML configuration support
 *
 * Example usage:
 * @code
 * Property<double> m_momentum_cut{"momentum_cut", 2.0, "Minimum momentum [GeV]"};
 * m_momentum_cut.with_range(0.0, 10.0);
 *
 * // Set value with validation
 * auto sc = m_momentum_cut.set(3.5);  // Returns SUCCESS
 * auto sc2 = m_momentum_cut.set(-1.0); // Returns FAILURE with message
 *
 * // Get value
 * double cut = m_momentum_cut.get();  // or just m_momentum_cut (implicit conversion)
 * @endcode
 *
 * @tparam T The property value type
 *
 * @see Requirement 2.1
 */
template <typename T>
class Property {
public:
    /**
     * @brief Construct a property with name, default value, and optional description
     *
     * @param name Property name (used for configuration lookup)
     * @param default_value Default value if not configured
     * @param description Human-readable description for documentation
     */
    Property(std::string name, T default_value, std::string description = "")
        : m_name(std::move(name))
        , m_value(std::move(default_value))
        , m_default_value(m_value)
        , m_description(std::move(description)) {}

    /**
     * @brief Add a range constraint for numeric properties
     *
     * Values outside [min, max] will be rejected by set().
     *
     * @param min Minimum allowed value (inclusive)
     * @param max Maximum allowed value (inclusive)
     * @return Reference to this property for chaining
     *
     * @see Requirement 2.3
     */
    auto with_range(T min, T max) -> Property& requires NumericProperty<T> {
        m_min = min;
        m_max = max;
        m_validators.push_back([min, max](const T& v) {
            return v >= min && v <= max;
        });
        return *this;
    }

    /**
     * @brief Add a custom validation function
     *
     * The validator should return true if the value is valid.
     *
     * @param validator Validation function
     * @return Reference to this property for chaining
     *
     * @see Requirement 2.3
     */
    auto with_validator(Validator<T> validator) -> Property& {
        m_validators.push_back(std::move(validator));
        return *this;
    }

    /**
     * @brief Mark this property as required (must be explicitly set)
     *
     * @return Reference to this property for chaining
     */
    auto required() -> Property& {
        m_required = true;
        return *this;
    }

    /**
     * @brief Get the current property value
     *
     * @return const T& Reference to the current value
     */
    [[nodiscard]] auto get() const -> const T& { return m_value; }

    /**
     * @brief Set the property value with validation
     *
     * Runs all registered validators. If any validator fails,
     * returns FAILURE with a descriptive message.
     *
     * @param value New value to set
     * @return StatusCode SUCCESS if valid, FAILURE with message otherwise
     *
     * @see Requirement 2.4
     */
    auto set(T value) -> StatusCode {
        // Run all validators
        for (const auto& validator : m_validators) {
            if (!validator(value)) {
                return StatusCode::failure(format_validation_error(value));
            }
        }
        m_value = std::move(value);
        m_was_set = true;
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Implicit conversion to the value type
     *
     * Allows using the property directly where T is expected.
     */
    [[nodiscard]] operator const T&() const { return m_value; }

    /**
     * @brief Get the property name
     * @return const std::string& Property name
     */
    [[nodiscard]] auto name() const -> const std::string& { return m_name; }

    /**
     * @brief Get the property description
     * @return const std::string& Property description
     */
    [[nodiscard]] auto description() const -> const std::string& { return m_description; }

    /**
     * @brief Get the default value
     * @return const T& Default value
     */
    [[nodiscard]] auto default_value() const -> const T& { return m_default_value; }

    /**
     * @brief Check if this property is required
     * @return bool True if required
     */
    [[nodiscard]] auto is_required() const -> bool { return m_required; }

    /**
     * @brief Check if this property was explicitly set
     * @return bool True if set() was called successfully
     */
    [[nodiscard]] auto was_set() const -> bool { return m_was_set; }

    /**
     * @brief Reset to default value
     */
    auto reset() -> void {
        m_value = m_default_value;
        m_was_set = false;
    }

    /**
     * @brief Get the minimum value (for numeric properties with range)
     * @return std::optional<T> Minimum value if range was set
     */
    [[nodiscard]] auto min_value() const -> std::optional<T> requires NumericProperty<T> {
        return m_min;
    }

    /**
     * @brief Get the maximum value (for numeric properties with range)
     * @return std::optional<T> Maximum value if range was set
     */
    [[nodiscard]] auto max_value() const -> std::optional<T> requires NumericProperty<T> {
        return m_max;
    }

private:
    std::string m_name;
    T m_value;
    T m_default_value;
    std::string m_description;
    std::vector<Validator<T>> m_validators;
    bool m_required = false;
    bool m_was_set = false;

    // Range bounds for numeric types (stored separately for error messages)
    std::optional<T> m_min;
    std::optional<T> m_max;

    /**
     * @brief Format a validation error message
     */
    [[nodiscard]] auto format_validation_error(const T& value) const -> std::string {
        if constexpr (NumericProperty<T>) {
            if (m_min.has_value() && m_max.has_value()) {
                return fmt::format(
                    "Property '{}': value {} is outside valid range [{}, {}]",
                    m_name, value, *m_min, *m_max
                );
            }
        }
        return fmt::format("Property '{}': validation failed for value", m_name);
    }
};

// Specialization for string to provide better error messages
template <>
inline auto Property<std::string>::format_validation_error(const std::string& value) const -> std::string {
    return fmt::format("Property '{}': validation failed for value '{}'", m_name, value);
}

}  // namespace framework


// Forward declaration for PropertyBase
namespace framework {

/**
 * @brief Base interface for type-erased property access
 *
 * This interface allows PropertyMixin to store and configure properties
 * of different types uniformly.
 */
class IPropertyBase {
public:
    virtual ~IPropertyBase() = default;
    
    [[nodiscard]] virtual auto name() const -> const std::string& = 0;
    [[nodiscard]] virtual auto description() const -> const std::string& = 0;
    [[nodiscard]] virtual auto is_required() const -> bool = 0;
    [[nodiscard]] virtual auto was_set() const -> bool = 0;
    
    /**
     * @brief Configure this property from a TOML node
     * @param node TOML node containing the value
     * @return StatusCode SUCCESS if configured, FAILURE with message otherwise
     */
    virtual auto configure_from_toml(const toml::node& node) -> StatusCode = 0;
};

/**
 * @brief Type-erased wrapper for Property<T>
 *
 * Allows storing properties of different types in a container
 * while preserving the ability to configure them from TOML.
 */
template <typename T>
class PropertyWrapper : public IPropertyBase {
public:
    explicit PropertyWrapper(Property<T>& prop) : m_property(prop) {}
    
    [[nodiscard]] auto name() const -> const std::string& override {
        return m_property.name();
    }
    
    [[nodiscard]] auto description() const -> const std::string& override {
        return m_property.description();
    }
    
    [[nodiscard]] auto is_required() const -> bool override {
        return m_property.is_required();
    }
    
    [[nodiscard]] auto was_set() const -> bool override {
        return m_property.was_set();
    }
    
    auto configure_from_toml(const toml::node& node) -> StatusCode override {
        return configure_property_from_toml(m_property, node);
    }

private:
    Property<T>& m_property;
    
    /**
     * @brief Configure a property from a TOML node
     *
     * Handles type conversion from TOML types to property types.
     */
    static auto configure_property_from_toml(Property<T>& prop, const toml::node& node) -> StatusCode {
        if constexpr (std::is_same_v<T, bool>) {
            if (auto val = node.as_boolean()) {
                return prop.set(val->get());
            }
        } else if constexpr (std::is_same_v<T, int>) {
            if (auto val = node.as_integer()) {
                return prop.set(static_cast<int>(val->get()));
            }
        } else if constexpr (std::is_same_v<T, int64_t>) {
            if (auto val = node.as_integer()) {
                return prop.set(val->get());
            }
        } else if constexpr (std::is_same_v<T, double>) {
            if (auto val = node.as_floating_point()) {
                return prop.set(val->get());
            }
            // Also accept integers as doubles
            if (auto val = node.as_integer()) {
                return prop.set(static_cast<double>(val->get()));
            }
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (auto val = node.as_string()) {
                return prop.set(std::string(val->get()));
            }
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            if (auto arr = node.as_array()) {
                std::vector<int> values;
                for (const auto& elem : *arr) {
                    if (auto val = elem.as_integer()) {
                        values.push_back(static_cast<int>(val->get()));
                    } else {
                        return StatusCode::failure(
                            fmt::format("Property '{}': array element is not an integer", prop.name())
                        );
                    }
                }
                return prop.set(std::move(values));
            }
        } else if constexpr (std::is_same_v<T, std::vector<double>>) {
            if (auto arr = node.as_array()) {
                std::vector<double> values;
                for (const auto& elem : *arr) {
                    if (auto val = elem.as_floating_point()) {
                        values.push_back(val->get());
                    } else if (auto ival = elem.as_integer()) {
                        values.push_back(static_cast<double>(ival->get()));
                    } else {
                        return StatusCode::failure(
                            fmt::format("Property '{}': array element is not a number", prop.name())
                        );
                    }
                }
                return prop.set(std::move(values));
            }
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            if (auto arr = node.as_array()) {
                std::vector<std::string> values;
                for (const auto& elem : *arr) {
                    if (auto val = elem.as_string()) {
                        values.push_back(std::string(val->get()));
                    } else {
                        return StatusCode::failure(
                            fmt::format("Property '{}': array element is not a string", prop.name())
                        );
                    }
                }
                return prop.set(std::move(values));
            }
        }
        
        return StatusCode::failure(
            fmt::format("Property '{}': type mismatch in TOML configuration", prop.name())
        );
    }
};

/**
 * @brief Type-erased binding from a TOML key to a struct field.
 *
 * `declare_config(my_cfg)` walks `Config::config_fields()` and
 * registers one of these per field. It writes through the pointer
 * directly when the algorithm's TOML sub-table is loaded — no parallel
 * `Property<T>` needed.
 */
template <typename T>
class ConfigFieldBinding : public IPropertyBase {
public:
    ConfigFieldBinding(std::string name, T* target) noexcept
        : m_name(std::move(name)), m_target(target) {}

    [[nodiscard]] auto name() const -> const std::string& override { return m_name; }
    [[nodiscard]] auto description() const -> const std::string& override {
        static const std::string empty;
        return empty;
    }
    [[nodiscard]] auto is_required() const -> bool override { return false; }
    [[nodiscard]] auto was_set() const -> bool override { return m_was_set; }

    auto configure_from_toml(const toml::node& node) -> StatusCode override {
        if constexpr (std::is_same_v<T, bool>) {
            if (auto val = node.as_boolean()) { *m_target = val->get(); m_was_set = true; return StatusCode::SUCCESS(); }
        } else if constexpr (std::is_same_v<T, int>) {
            if (auto val = node.as_integer()) { *m_target = static_cast<int>(val->get()); m_was_set = true; return StatusCode::SUCCESS(); }
        } else if constexpr (std::is_same_v<T, int64_t>) {
            if (auto val = node.as_integer()) { *m_target = val->get(); m_was_set = true; return StatusCode::SUCCESS(); }
        } else if constexpr (std::is_same_v<T, double>) {
            if (auto val = node.as_floating_point()) { *m_target = val->get(); m_was_set = true; return StatusCode::SUCCESS(); }
            if (auto val = node.as_integer())        { *m_target = static_cast<double>(val->get()); m_was_set = true; return StatusCode::SUCCESS(); }
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (auto val = node.as_string()) { *m_target = std::string(val->get()); m_was_set = true; return StatusCode::SUCCESS(); }
        } else if constexpr (std::is_same_v<T, std::vector<int>>) {
            if (auto arr = node.as_array()) {
                std::vector<int> values;
                for (const auto& elem : *arr) {
                    if (auto val = elem.as_integer()) values.push_back(static_cast<int>(val->get()));
                    else return StatusCode::failure(fmt::format("Config field '{}': array element is not an integer", m_name));
                }
                *m_target = std::move(values); m_was_set = true; return StatusCode::SUCCESS();
            }
        } else if constexpr (std::is_same_v<T, std::vector<double>>) {
            if (auto arr = node.as_array()) {
                std::vector<double> values;
                for (const auto& elem : *arr) {
                    if (auto val = elem.as_floating_point()) values.push_back(val->get());
                    else if (auto ival = elem.as_integer())  values.push_back(static_cast<double>(ival->get()));
                    else return StatusCode::failure(fmt::format("Config field '{}': array element is not a number", m_name));
                }
                *m_target = std::move(values); m_was_set = true; return StatusCode::SUCCESS();
            }
        } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            if (auto arr = node.as_array()) {
                std::vector<std::string> values;
                for (const auto& elem : *arr) {
                    if (auto val = elem.as_string()) values.push_back(std::string(val->get()));
                    else return StatusCode::failure(fmt::format("Config field '{}': array element is not a string", m_name));
                }
                *m_target = std::move(values); m_was_set = true; return StatusCode::SUCCESS();
            }
        } else if constexpr (std::is_enum_v<T>) {
            // Look up `parse_config_value(string_view, T&) -> bool` via ADL.
            // Define one in the same namespace as the enum to opt in (see
            // clas12/Target.hpp for examples covering Target/Polarity/RunGroup).
            if (auto val = node.as_string()) {
                T parsed{};
                if (parse_config_value(std::string_view(val->get()), parsed)) {
                    *m_target = parsed; m_was_set = true; return StatusCode::SUCCESS();
                }
                return StatusCode::failure(fmt::format("Config field '{}': value '{}' is not a valid enum entry", m_name, val->get()));
            }
        }
        return StatusCode::failure(fmt::format("Config field '{}': type mismatch in TOML configuration", m_name));
    }

private:
    std::string m_name;
    T* m_target;
    bool m_was_set = false;
};

/**
 * @brief Mixin for property declaration in algorithms
 *
 * PropertyMixin provides automatic property registration and TOML
 * configuration support for algorithms. Algorithms inherit from this
 * mixin to gain declarative property support.
 *
 * Example usage:
 * @code
 * class MyAlgorithm : public Algorithm, public PropertyMixin<MyAlgorithm> {
 * public:
 *     MyAlgorithm() : Algorithm("MyAlgorithm") {
 *         declare_property(m_momentum_cut);
 *         declare_property(m_theta_range);
 *     }
 *
 * private:
 *     Property<double> m_momentum_cut{"momentum_cut", 2.0, "Minimum momentum"};
 *     Property<std::vector<double>> m_theta_range{"theta_range", {5.0, 35.0}, "Theta range"};
 * };
 * @endcode
 *
 * @see Requirement 2.2
 */
class PropertyMixin {
public:
    /**
     * @brief Declare a property for automatic registration
     *
     * Registered properties can be configured from TOML files
     * using configure_from_toml().
     *
     * @tparam T Property value type
     * @param property Reference to the property to register
     * @return Property<T>& Reference to the property for chaining
     */
    template <typename T>
    auto declare_property(Property<T>& property) -> Property<T>& {
        m_properties[property.name()] = std::make_unique<PropertyWrapper<T>>(property);
        return property;
    }

    /// @brief Variadic version to declare multiple properties at once
    template<typename... Args>
    auto declare_properties(Property<Args>&... properties) -> void {
        (declare_property(properties), ...);
    }

    /**
     * @brief Bind a Config struct's fields directly to TOML keys.
     *
     * The struct must expose a `static constexpr auto config_fields()`
     * returning a tuple of `framework::field("name", &T::member)` entries.
     * Each listed field is registered as if it were a Property — when the
     * algorithm's TOML sub-table is applied, the value is written straight
     * into the struct member with no intermediate Property<T>.
     *
     * Compile-time-only fields (e.g. enum settings) can simply be omitted
     * from `config_fields()`.
     */
    template <typename Config>
        requires HasStaticConfigFields<Config>
    auto declare_config(Config& cfg) -> Config& {
        std::apply(
            [this, &cfg](const auto&... f) {
                (this->register_config_field_(cfg, f), ...);
            },
            Config::config_fields());
        return cfg;
    }

private:
    template <typename Config, typename Member>
    auto register_config_field_(Config& cfg, const Field<Config, Member>& f) -> void {
        auto* target = &(cfg.*f.ptr);
        m_properties[std::string(f.name)] =
            std::make_unique<ConfigFieldBinding<Member>>(std::string(f.name), target);
    }

public:

    /**
     * @brief Configure all registered properties from a TOML table
     *
     * Looks up each registered property by name in the TOML table
     * and configures it if found. Properties not in the table keep
     * their default values.
     *
     * @param config TOML table containing property values
     * @return StatusCode SUCCESS if all properties configured successfully,
     *         FAILURE with message if any validation fails or required
     *         properties are missing
     *
     * @see Requirement 2.5
     */
    auto configure_from_toml(const toml::table& config) -> StatusCode {
        for (auto& [name, prop] : m_properties) {
            if (auto node = config.get(name)) {
                auto sc = prop->configure_from_toml(*node);
                if (sc.is_failure()) {
                    return sc;
                }
            } else if (prop->is_required()) {
                return StatusCode::failure(
                    fmt::format("Required property '{}' not found in configuration", name)
                );
            }
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Configure properties from a TOML file
     *
     * @param filepath Path to the TOML configuration file
     * @return StatusCode SUCCESS if configured, FAILURE with message otherwise
     */
    auto configure_from_file(const std::string& filepath) -> StatusCode {
        try {
            auto config = toml::parse_file(filepath);
            return configure_from_toml(config);
        } catch (const toml::parse_error& e) {
            return StatusCode::failure(
                fmt::format("Failed to parse TOML file '{}': {}", filepath, e.what())
            );
        }
    }

    /**
     * @brief Get all registered property names
     *
     * @return std::vector<std::string> List of property names
     */
    [[nodiscard]] auto property_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_properties.size());
        for (const auto& [name, _] : m_properties) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * @brief Check if a property with the given name is registered
     *
     * @param name Property name
     * @return bool True if registered
     */
    [[nodiscard]] auto has_property(const std::string& name) const -> bool {
        return m_properties.contains(name);
    }

    /**
     * @brief Get the number of registered properties
     *
     * @return std::size_t Number of properties
     */
    [[nodiscard]] auto property_count() const -> std::size_t {
        return m_properties.size();
    }

    /**
     * @brief Validate that all required properties have been set
     *
     * @return StatusCode SUCCESS if all required properties are set,
     *         FAILURE with message listing missing properties
     */
    [[nodiscard]] auto validate_required_properties() const -> StatusCode {
        std::vector<std::string> missing;
        for (const auto& [name, prop] : m_properties) {
            if (prop->is_required() && !prop->was_set()) {
                missing.push_back(name);
            }
        }
        
        if (!missing.empty()) {
            return StatusCode::failure(
                fmt::format("Missing required properties: {}", fmt::join(missing, ", "))
            );
        }
        return StatusCode::SUCCESS();
    }

protected:
    ~PropertyMixin() = default;

private:
    std::map<std::string, std::unique_ptr<IPropertyBase>> m_properties;
};

}  // namespace framework
