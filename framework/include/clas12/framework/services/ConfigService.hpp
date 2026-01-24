#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <toml++/toml.hpp>
#include <vector>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Result.hpp>

namespace clas12::framework {

/**
 * @brief Configuration service for loading and accessing TOML configuration files
 *
 * Provides centralized configuration management with:
 * - Type-safe access to configuration values
 * - Default value support
 * - Validation of required fields
 * - Clear error messages for file and parse errors
 *
 * Example:
 * @code
 * ConfigService config("config.toml");
 * auto sc = config.initialize();
 * if (sc.is_failure()) {
 *     std::cerr << sc.message() << std::endl;
 *     return;
 * }
 * 
 * auto max_events = config.get<int>("application.max_events");
 * if (max_events.is_ok()) {
 *     process(max_events.value());
 * }
 * 
 * // With default value
 * int threads = config.get<int>("application.num_threads", 4);
 * @endcode
 */
class ConfigService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "ConfigService"; }

    /**
     * @brief Construct ConfigService with path to TOML config file
     * @param config_path Path to the TOML configuration file
     * @param name Service name (default: "ConfigService")
     */
    explicit ConfigService(std::string config_path, std::string name = "ConfigService")
        : Service(std::move(name)), m_config_path(std::move(config_path)), m_initialized(false) {}

    /**
     * @brief Initialize the service by loading the configuration file
     * @return StatusCode indicating success or failure with error message
     */
    auto initialize() -> StatusCode override {
        auto result = load_config();
        if (result.is_error()) {
            return StatusCode::failure(result.error_message());
        }
        m_initialized = true;
        return StatusCode::success();
    }

    auto finalize() -> StatusCode override {
        m_initialized = false;
        return StatusCode::success();
    }

    /**
     * @brief Get a configuration value by key with Result return type
     * @tparam T The expected type of the configuration value
     * @param key Dot-separated key path (e.g., "application.max_events")
     * @return Result<T> containing the value or an error message
     */
    template <typename T>
    auto get(const std::string& key) const -> Result<T> {
        if (!m_initialized) {
            return Result<T>::error("ConfigService not initialized");
        }

        auto node = navigate_to_key(key);
        if (!node) {
            return Result<T>::error("Configuration key not found: " + key);
        }

        auto value = node.value<T>();
        if (!value) {
            return Result<T>::error("Type mismatch for configuration key: " + key);
        }

        return Result<T>::ok(*value);
    }

    /**
     * @brief Get a configuration value with a default fallback
     * @tparam T The expected type of the configuration value
     * @param key Dot-separated key path
     * @param default_value Value to return if key is missing or type mismatch
     * @return The configuration value or the default
     */
    template <typename T>
    auto get(const std::string& key, T default_value) const -> T {
        auto result = get<T>(key);
        return result.value_or(std::move(default_value));
    }

    /**
     * @brief Check if a configuration key exists
     * @param key Dot-separated key path
     * @return true if the key exists, false otherwise
     */
    auto has(const std::string& key) const -> bool {
        if (!m_initialized) {
            return false;
        }
        auto node = navigate_to_key(key);
        return static_cast<bool>(node);
    }

    /**
     * @brief Validate that all required keys are present
     * @param required_keys List of required key paths
     * @return Result<void> indicating success or listing missing keys
     */
    auto validate(const std::vector<std::string>& required_keys) const -> Result<void> {
        if (!m_initialized) {
            return Result<void>::error("ConfigService not initialized");
        }

        std::vector<std::string> missing;
        for (const auto& key : required_keys) {
            if (!has(key)) {
                missing.push_back(key);
            }
        }

        if (!missing.empty()) {
            std::ostringstream oss;
            oss << "Missing required configuration keys: ";
            for (size_t i = 0; i < missing.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << missing[i];
            }
            return Result<void>::error(oss.str());
        }

        return Result<void>::ok();
    }

    /**
     * @brief Get the path to the configuration file
     * @return const std::string& The configuration file path
     */
    auto config_path() const -> const std::string& {
        return m_config_path;
    }

    /**
     * @brief Get direct access to the underlying TOML table (for advanced use)
     * @return const toml::table& The parsed TOML table
     */
    auto table() const -> const toml::table& {
        return m_config;
    }

   private:
    std::string m_config_path;
    toml::table m_config;
    bool m_initialized;

    /**
     * @brief Load and parse the configuration file
     * @return Result<void> indicating success or error with details
     */
    auto load_config() -> Result<void> {
        // Check if file exists
        if (!std::filesystem::exists(m_config_path)) {
            return Result<void>::error(
                "Configuration file not found: " + m_config_path);
        }

        // Try to parse the file
        try {
            m_config = toml::parse_file(m_config_path);
            return Result<void>::ok();
        } catch (const toml::parse_error& err) {
            std::ostringstream oss;
            oss << "Failed to parse configuration file '" << m_config_path << "': ";

            // Include source location if available
            if (err.source().begin.line > 0) {
                oss << "line " << err.source().begin.line;
                if (err.source().begin.column > 0) {
                    oss << ", column " << err.source().begin.column;
                }
                oss << ": ";
            }
            oss << err.description();

            return Result<void>::error(oss.str());
        } catch (const std::exception& err) {
            return Result<void>::error(
                "Failed to parse configuration file '" + m_config_path + "': " + err.what());
        }
    }

    /**
     * @brief Navigate to a node using dot-separated key path
     * @param key Dot-separated key path (e.g., "section.subsection.value")
     * @return toml::node_view pointing to the node, or empty if not found
     */
    auto navigate_to_key(const std::string& key) const -> toml::node_view<const toml::node> {
        // Split key by dots
        std::vector<std::string> parts;
        std::istringstream iss(key);
        std::string part;
        while (std::getline(iss, part, '.')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }

        if (parts.empty()) {
            return toml::node_view<const toml::node>{};
        }

        // Navigate through the table
        toml::node_view<const toml::node> current = m_config[parts[0]];
        for (size_t i = 1; i < parts.size() && current; ++i) {
            current = current[parts[i]];
        }

        return current;
    }
};

}  // namespace clas12::framework
