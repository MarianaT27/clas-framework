#pragma once

#include <fmt/format.h>
#include <memory>
#include <optional>
#include <source_location>
#include <string>
#include <string_view>

namespace clas12::framework {

/**
 * @brief Error with source location and optional cause chain
 *
 * This class provides comprehensive error handling with:
 * - Automatic source location capture (file, line, function)
 * - Error chaining for nested operations
 * - Formatted error messages with full context
 *
 * C++ Core Guidelines:
 * - E.27: If you can't throw exceptions, use error codes systematically
 * - C.49: Prefer initialization to assignment in constructors
 *
 * Usage:
 * @code
 * auto err = Error("Failed to open file");
 * // Automatically captures source location
 *
 * // Chain errors for context
 * auto outer = Error("Configuration failed")
 *     .with_cause(Error("Invalid TOML syntax"));
 *
 * // Format for logging
 * std::cerr << err.format() << std::endl;
 * @endcode
 */
class Error {
public:
    /**
     * @brief Construct an error with message and automatic source location
     * @param msg Error message describing what went wrong
     * @param loc Source location (automatically captured by default)
     */
    explicit Error(
        std::string msg,
        std::source_location loc = std::source_location::current())
        : m_message(std::move(msg)), m_location(loc) {}

    // Copy and move operations
    Error(const Error& other)
        : m_message(other.m_message),
          m_location(other.m_location),
          m_cause(other.m_cause ? std::make_unique<Error>(*other.m_cause) : nullptr) {}

    Error(Error&&) noexcept = default;

    auto operator=(const Error& other) -> Error& {
        if (this != &other) {
            m_message = other.m_message;
            m_location = other.m_location;
            m_cause = other.m_cause ? std::make_unique<Error>(*other.m_cause) : nullptr;
        }
        return *this;
    }

    auto operator=(Error&&) noexcept -> Error& = default;

    ~Error() = default;

    /**
     * @brief Add a cause to this error for error chaining
     * @param cause The underlying error that caused this error
     * @return Reference to this error for method chaining
     *
     * Example:
     * @code
     * auto err = Error("Failed to load config")
     *     .with_cause(Error("File not found"));
     * @endcode
     */
    auto with_cause(Error cause) -> Error& {
        m_cause = std::make_unique<Error>(std::move(cause));
        return *this;
    }

    /**
     * @brief Get the error message
     * @return The error message string
     */
    [[nodiscard]] auto message() const noexcept -> const std::string& {
        return m_message;
    }

    /**
     * @brief Get the source location where the error was created
     * @return The source location
     */
    [[nodiscard]] auto location() const noexcept -> const std::source_location& {
        return m_location;
    }

    /**
     * @brief Check if this error has a cause
     * @return true if there is a chained cause error
     */
    [[nodiscard]] auto has_cause() const noexcept -> bool {
        return m_cause != nullptr;
    }

    /**
     * @brief Get the cause of this error
     * @return Pointer to the cause error, or nullptr if none
     */
    [[nodiscard]] auto cause() const noexcept -> const Error* {
        return m_cause.get();
    }

    /**
     * @brief Format the error with full context including source location
     * @return Formatted error string with file:line and function info
     *
     * Output format:
     * "file.cpp:42 in function_name(): Error message"
     * "  Caused by: other_file.cpp:10 in other_func(): Underlying error"
     */
    [[nodiscard]] auto format() const -> std::string {
        auto result = fmt::format("{}:{} in {}(): {}",
                                  m_location.file_name(),
                                  m_location.line(),
                                  m_location.function_name(),
                                  m_message);

        if (m_cause) {
            result += fmt::format("\n  Caused by: {}", m_cause->format());
        }

        return result;
    }

    /**
     * @brief Format error with indentation for nested display
     * @param indent Number of spaces to indent
     * @return Formatted error string with indentation
     */
    [[nodiscard]] auto format_indented(int indent = 0) const -> std::string {
        std::string prefix(indent, ' ');
        auto result = fmt::format("{}{}:{} in {}(): {}",
                                  prefix,
                                  m_location.file_name(),
                                  m_location.line(),
                                  m_location.function_name(),
                                  m_message);

        if (m_cause) {
            result += fmt::format("\n{}Caused by: {}", prefix, m_cause->format_indented(indent + 2));
        }

        return result;
    }

    /**
     * @brief Get a short format without source location details
     * @return Just the error message and cause chain
     */
    [[nodiscard]] auto format_short() const -> std::string {
        auto result = m_message;
        if (m_cause) {
            result += fmt::format(" (caused by: {})", m_cause->format_short());
        }
        return result;
    }

private:
    std::string m_message;
    std::source_location m_location;
    std::unique_ptr<Error> m_cause;
};

}  // namespace clas12::framework
