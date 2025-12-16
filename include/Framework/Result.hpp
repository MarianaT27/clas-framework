#pragma once

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace Framework {

namespace detail {
/// @brief Wrapper to distinguish error strings from value strings
struct ErrorWrapper {
    std::string message;
    explicit ErrorWrapper(std::string msg) : message(std::move(msg)) {}
};
}  // namespace detail

/**
 * @brief A Result type for operations that can fail
 *
 * Result<T> provides a type-safe way to handle operations that may succeed
 * with a value of type T, or fail with an error message. This is similar
 * to Rust's Result type or C++23's std::expected.
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
 *     log_error(result.error());
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
    using error_type = std::string;

    // Factory methods (C++ Core Guidelines: C.50 - Use factory function if you need "virtual behavior")
    [[nodiscard]] static auto ok(T value) -> Result<T> {
        return Result<T>(std::in_place_index<0>, std::move(value));
    }

    [[nodiscard]] static auto error(std::string message) -> Result<T> {
        return Result<T>(std::in_place_index<1>, detail::ErrorWrapper{std::move(message)});
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
            throw std::runtime_error("Result::value() called on error: " + error());
        }
        return std::get<T>(m_data);
    }

    [[nodiscard]] auto value() & -> T& {
        if (is_error()) {
            throw std::runtime_error("Result::value() called on error: " + error());
        }
        return std::get<T>(m_data);
    }

    [[nodiscard]] auto value() && -> T&& {
        if (is_error()) {
            throw std::runtime_error("Result::value() called on error: " + error());
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
            throw std::runtime_error(std::string(msg) + ": " + error());
        }
        return std::get<T>(m_data);
    }

    [[nodiscard]] auto expect(std::string_view msg) && -> T&& {
        if (is_error()) {
            throw std::runtime_error(std::string(msg) + ": " + error());
        }
        return std::get<T>(std::move(m_data));
    }

    // Error access
    [[nodiscard]] auto error() const -> const std::string& {
        static const std::string empty;
        return is_error() ? std::get<detail::ErrorWrapper>(m_data).message : empty;
    }

    // Alias for backward compatibility
    [[nodiscard]] auto error_message() const -> const std::string& { return error(); }

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
        return Result<U>::error(error());
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
        return ResultU::error(error());
    }

    /// @brief Transform error message if error, pass through value otherwise
    template <typename F>
    [[nodiscard]] auto map_error(F&& func) const -> Result<T> {
        if (is_error()) {
            return Result<T>::error(std::invoke(std::forward<F>(func), error()));
        }
        return *this;
    }

private:
    template <std::size_t I, typename... Args>
    explicit Result(std::in_place_index_t<I> tag, Args&&... args)
        : m_data(tag, std::forward<Args>(args)...) {}

    std::variant<T, detail::ErrorWrapper> m_data;
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
    using error_type = std::string;

    // Factory methods
    [[nodiscard]] static auto ok() -> Result<void> {
        return Result<void>(true, {});
    }

    [[nodiscard]] static auto error(std::string message) -> Result<void> {
        return Result<void>(false, std::move(message));
    }

    // State queries
    [[nodiscard]] auto is_ok() const noexcept -> bool { return m_is_ok; }
    [[nodiscard]] auto is_error() const noexcept -> bool { return !m_is_ok; }
    [[nodiscard]] explicit operator bool() const noexcept { return m_is_ok; }

    // Error access
    [[nodiscard]] auto error() const noexcept -> const std::string& { return m_message; }
    [[nodiscard]] auto error_message() const noexcept -> const std::string& { return m_message; }

    /// @brief Throw if error with custom message
    auto expect(std::string_view msg) const -> void {
        if (is_error()) {
            throw std::runtime_error(std::string(msg) + ": " + m_message);
        }
    }

    /// @brief Chain operations that return Result<void>
    template <typename F>
    [[nodiscard]] auto and_then(F&& func) const -> std::invoke_result_t<F> {
        using ResultU = std::invoke_result_t<F>;
        if (is_ok()) {
            return std::invoke(std::forward<F>(func));
        }
        return ResultU::error(m_message);
    }

    /// @brief Transform error message
    template <typename F>
    [[nodiscard]] auto map_error(F&& func) const -> Result<void> {
        if (is_error()) {
            return Result<void>::error(std::invoke(std::forward<F>(func), m_message));
        }
        return *this;
    }

private:
    explicit Result(bool ok, std::string message)
        : m_is_ok(ok), m_message(std::move(message)) {}

    bool m_is_ok;
    std::string m_message;
};

}  // namespace Framework
