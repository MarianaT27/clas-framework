#pragma once

#include "Framework/Error.hpp"

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace Framework {

/**
 * @brief A Result type for operations that can fail
 *
 * Result<T> provides a type-safe way to handle operations that may succeed
 * with a value of type T, or fail with an Error containing source location
 * and optional cause chain. This is similar to Rust's Result type or
 * C++23's std::expected.
 *
 * C++ Core Guidelines:
 * - E.27: If you can't throw exceptions, use error codes systematically
 * - F.21: To return multiple "out" values, prefer returning a struct or tuple
 * - C.49: Prefer initialization to assignment in constructors
 *
 * Usage:
 * @code
 * auto result = some_operation();
 * if (result) {  // or result.is_ok()
 *     process(result.value());
 * } else {
 *     log_error(result.error().format());
 * }
 *
 * // Or use expect() for assertions
 * auto value = result.expect("Operation should succeed");
 *
 * // Or use value_or() for defaults
 * auto value = result.value_or(default_value);
 * @endcode
 */
template <typename T>
class [[nodiscard]] Result {
public:
    using value_type = T;
    using error_type = Error;

    // Factory methods (C++ Core Guidelines: C.50 - Use factory function if you need "virtual behavior")
    [[nodiscard]] static auto ok(T value) -> Result<T> {
        return Result<T>(std::in_place_index<0>, std::move(value));
    }

    [[nodiscard]] static auto error(
        std::string message,
        std::source_location loc = std::source_location::current()) -> Result<T> {
        return Result<T>(std::in_place_index<1>, Error(std::move(message), loc));
    }

    [[nodiscard]] static auto error(Error err) -> Result<T> {
        return Result<T>(std::in_place_index<1>, std::move(err));
    }

    // State queries
    [[nodiscard]] auto is_ok() const noexcept -> bool {
        return std::holds_alternative<T>(m_data);
    }

    [[nodiscard]] auto is_error() const noexcept -> bool {
        return !is_ok();
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return is_ok();
    }

    // Value access (C++ Core Guidelines: F.23 - Use not_null<T> to indicate "null" is not valid)
    [[nodiscard]] auto value() const& -> const T& {
        if (is_error()) {
            throw std::runtime_error("Result::value() called on error: " + error().format());
        }
        return std::get<T>(m_data);
    }

    [[nodiscard]] auto value() & -> T& {
        if (is_error()) {
            throw std::runtime_error("Result::value() called on error: " + error().format());
        }
        return std::get<T>(m_data);
    }

    [[nodiscard]] auto value() && -> T&& {
        if (is_error()) {
            throw std::runtime_error("Result::value() called on error: " + error().format());
        }
        return std::get<T>(std::move(m_data));
    }

    /// @brief Get value or return default (no exception)
    [[nodiscard]] auto value_or(T default_val) const& -> T {
        return is_ok() ? std::get<T>(m_data) : std::move(default_val);
    }

    [[nodiscard]] auto value_or(T default_val) && -> T {
        return is_ok() ? std::get<T>(std::move(m_data)) : std::move(default_val);
    }

    /// @brief Get value or throw with custom message (Rust-style expect)
    [[nodiscard]] auto expect(std::string_view msg) const& -> const T& {
        if (is_error()) {
            throw std::runtime_error(std::string(msg) + ": " + error().format());
        }
        return std::get<T>(m_data);
    }

    [[nodiscard]] auto expect(std::string_view msg) && -> T&& {
        if (is_error()) {
            throw std::runtime_error(std::string(msg) + ": " + error().format());
        }
        return std::get<T>(std::move(m_data));
    }

    // Error access - returns the Error object
    [[nodiscard]] auto error() const& -> const Error& {
        static const Error empty_error("");
        return is_error() ? std::get<Error>(m_data) : empty_error;
    }

    [[nodiscard]] auto error() & -> Error& {
        if (!is_error()) {
            throw std::runtime_error("Result::error() called on ok value");
        }
        return std::get<Error>(m_data);
    }

    [[nodiscard]] auto error() && -> Error&& {
        if (!is_error()) {
            throw std::runtime_error("Result::error() called on ok value");
        }
        return std::get<Error>(std::move(m_data));
    }

    // Convenience: get error message string directly
    [[nodiscard]] auto error_message() const -> std::string {
        return is_error() ? error().message() : std::string{};
    }

    // Monadic operations (C++ Core Guidelines: T.40 - Use function objects for operations)

    /// @brief Transform value if ok, propagate error otherwise
    template <typename F>
    [[nodiscard]] auto map(F&& func) const& -> Result<std::invoke_result_t<F, const T&>> {
        using U = std::invoke_result_t<F, const T&>;
        if (is_ok()) {
            return Result<U>::ok(std::invoke(std::forward<F>(func), std::get<T>(m_data)));
        }
        return Result<U>::error(error());
    }

    template <typename F>
    [[nodiscard]] auto map(F&& func) && -> Result<std::invoke_result_t<F, T&&>> {
        using U = std::invoke_result_t<F, T&&>;
        if (is_ok()) {
            return Result<U>::ok(std::invoke(std::forward<F>(func), std::get<T>(std::move(m_data))));
        }
        return Result<U>::error(std::get<Error>(std::move(m_data)));
    }

    /// @brief Chain operations that return Result (flatMap/bind)
    template <typename F>
    [[nodiscard]] auto and_then(F&& func) const& -> std::invoke_result_t<F, const T&> {
        using ResultU = std::invoke_result_t<F, const T&>;
        if (is_ok()) {
            return std::invoke(std::forward<F>(func), std::get<T>(m_data));
        }
        return ResultU::error(error());
    }

    template <typename F>
    [[nodiscard]] auto and_then(F&& func) && -> std::invoke_result_t<F, T&&> {
        using ResultU = std::invoke_result_t<F, T&&>;
        if (is_ok()) {
            return std::invoke(std::forward<F>(func), std::get<T>(std::move(m_data)));
        }
        return ResultU::error(std::get<Error>(std::move(m_data)));
    }

    /// @brief Transform error if error, pass through value otherwise
    template <typename F>
    [[nodiscard]] auto map_error(F&& func) const& -> Result<T> {
        if (is_error()) {
            return Result<T>::error(std::invoke(std::forward<F>(func), error()));
        }
        return *this;
    }

    template <typename F>
    [[nodiscard]] auto map_error(F&& func) && -> Result<T> {
        if (is_error()) {
            return Result<T>::error(std::invoke(std::forward<F>(func), std::get<Error>(std::move(m_data))));
        }
        return std::move(*this);
    }

    /// @brief Add context to error by wrapping it as a cause
    [[nodiscard]] auto with_context(
        std::string context_msg,
        std::source_location loc = std::source_location::current()) const& -> Result<T> {
        if (is_error()) {
            auto new_error = Error(std::move(context_msg), loc);
            new_error.with_cause(error());
            return Result<T>::error(std::move(new_error));
        }
        return *this;
    }

    [[nodiscard]] auto with_context(
        std::string context_msg,
        std::source_location loc = std::source_location::current()) && -> Result<T> {
        if (is_error()) {
            auto new_error = Error(std::move(context_msg), loc);
            new_error.with_cause(std::get<Error>(std::move(m_data)));
            return Result<T>::error(std::move(new_error));
        }
        return std::move(*this);
    }

private:
    template <std::size_t I, typename... Args>
    explicit Result(std::in_place_index_t<I> tag, Args&&... args)
        : m_data(tag, std::forward<Args>(args)...) {}

    std::variant<T, Error> m_data;
};

/**
 * @brief Specialization of Result for void return type
 *
 * Used for operations that either succeed (with no value) or fail with an error.
 * This is useful for operations like validation or side-effect-only functions.
 */
template <>
class [[nodiscard]] Result<void> {
public:
    using value_type = void;
    using error_type = Error;

    // Factory methods
    [[nodiscard]] static auto ok() -> Result<void> {
        return Result<void>(true, std::nullopt);
    }

    [[nodiscard]] static auto error(
        std::string message,
        std::source_location loc = std::source_location::current()) -> Result<void> {
        return Result<void>(false, Error(std::move(message), loc));
    }

    [[nodiscard]] static auto error(Error err) -> Result<void> {
        return Result<void>(false, std::move(err));
    }

    // State queries
    [[nodiscard]] auto is_ok() const noexcept -> bool { return m_is_ok; }
    [[nodiscard]] auto is_error() const noexcept -> bool { return !m_is_ok; }
    [[nodiscard]] explicit operator bool() const noexcept { return m_is_ok; }

    // Error access
    [[nodiscard]] auto error() const noexcept -> const Error& {
        static const Error empty_error("");
        return m_error.has_value() ? *m_error : empty_error;
    }

    [[nodiscard]] auto error_message() const -> std::string {
        return m_error.has_value() ? m_error->message() : std::string{};
    }

    /// @brief Throw if error with custom message
    auto expect(std::string_view msg) const -> void {
        if (is_error()) {
            throw std::runtime_error(std::string(msg) + ": " + error().format());
        }
    }

    /// @brief Chain operations that return Result<void>
    template <typename F>
    [[nodiscard]] auto and_then(F&& func) const -> std::invoke_result_t<F> {
        using ResultU = std::invoke_result_t<F>;
        if (is_ok()) {
            return std::invoke(std::forward<F>(func));
        }
        return ResultU::error(error());
    }

    /// @brief Transform error
    template <typename F>
    [[nodiscard]] auto map_error(F&& func) const -> Result<void> {
        if (is_error()) {
            return Result<void>::error(std::invoke(std::forward<F>(func), error()));
        }
        return *this;
    }

    /// @brief Add context to error
    [[nodiscard]] auto with_context(
        std::string context_msg,
        std::source_location loc = std::source_location::current()) const -> Result<void> {
        if (is_error()) {
            auto new_error = Error(std::move(context_msg), loc);
            new_error.with_cause(error());
            return Result<void>::error(std::move(new_error));
        }
        return *this;
    }

private:
    explicit Result(bool ok, std::optional<Error> err)
        : m_is_ok(ok), m_error(std::move(err)) {}

    bool m_is_ok;
    std::optional<Error> m_error;
};

// ============================================================================
// TRY Macro for early return on error
// ============================================================================

/**
 * @brief Macro for early return on error with context preservation
 *
 * Usage:
 * @code
 * auto load_config() -> Result<Config> {
 *     auto file_result = open_file("config.toml");
 *     auto file = FRAMEWORK_TRY(file_result);  // Returns error if failed
 *
 *     auto parse_result = parse_toml(file);
 *     auto config = FRAMEWORK_TRY(parse_result);  // Returns error if failed
 *
 *     return Result<Config>::ok(config);
 * }
 * @endcode
 *
 * Note: This macro uses a statement expression (GCC/Clang extension).
 * For portable code, use the explicit if-check pattern instead.
 */
#if defined(__GNUC__) || defined(__clang__)
#define FRAMEWORK_TRY(expr)                                                    \
    ({                                                                         \
        auto _result = (expr);                                                 \
        if (_result.is_error()) {                                              \
            return decltype(_result)::error(std::move(_result).error());       \
        }                                                                      \
        std::move(_result).value();                                            \
    })

/**
 * @brief TRY macro that adds context on error
 *
 * Usage:
 * @code
 * auto process() -> Result<Data> {
 *     auto data = FRAMEWORK_TRY_CONTEXT(load_data(), "Failed to load data");
 *     return Result<Data>::ok(data);
 * }
 * @endcode
 */
#define FRAMEWORK_TRY_CONTEXT(expr, context_msg)                               \
    ({                                                                         \
        auto _result = (expr);                                                 \
        if (_result.is_error()) {                                              \
            auto _new_error = ::Framework::Error(context_msg);                 \
            _new_error.with_cause(std::move(_result).error());                 \
            return decltype(_result)::error(std::move(_new_error));            \
        }                                                                      \
        std::move(_result).value();                                            \
    })
#else
// MSVC-compatible version using lambda
#define FRAMEWORK_TRY(expr)                                                    \
    [&]() {                                                                    \
        auto _result = (expr);                                                 \
        if (_result.is_error()) {                                              \
            return decltype(_result)::error(std::move(_result).error());       \
        }                                                                      \
        return std::move(_result).value();                                     \
    }()

#define FRAMEWORK_TRY_CONTEXT(expr, context_msg)                               \
    [&]() {                                                                    \
        auto _result = (expr);                                                 \
        if (_result.is_error()) {                                              \
            auto _new_error = ::Framework::Error(context_msg);                 \
            _new_error.with_cause(std::move(_result).error());                 \
            return decltype(_result)::error(std::move(_new_error));            \
        }                                                                      \
        return std::move(_result).value();                                     \
    }()
#endif

// Shorter alias for convenience
#define FW_TRY FRAMEWORK_TRY
#define FW_TRY_CTX FRAMEWORK_TRY_CONTEXT

}  // namespace Framework
