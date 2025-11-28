#pragma once

#include <string>
#include <variant>
#include <type_traits>
#include <stdexcept>

namespace Framework {

namespace detail {
    // Wrapper to distinguish error strings from value strings
    struct ErrorWrapper {
        std::string message;
        
        explicit ErrorWrapper(std::string msg) : message(std::move(msg)) {}
    };
}

/**
 * @brief A Result type for operations that can fail
 *
 * Result<T> provides a type-safe way to handle operations that may succeed
 * with a value of type T, or fail with an error message. This is similar
 * to Rust's Result type or Haskell's Either.
 *
 * Usage:
 *   auto result = some_operation();
 *   if (result.is_ok()) {
 *       auto value = result.value();
 *   } else {
 *       std::cerr << result.error_message() << std::endl;
 *   }
 */
template<typename T>
class Result {
public:
    // Factory methods
    static auto ok(T value) -> Result<T> {
        return Result<T>(std::move(value));
    }

    static auto error(std::string message) -> Result<T> {
        return Result<T>(detail::ErrorWrapper{std::move(message)});
    }

    // Accessors
    auto is_ok() const -> bool {
        return std::holds_alternative<T>(m_data);
    }

    auto is_error() const -> bool {
        return std::holds_alternative<detail::ErrorWrapper>(m_data);
    }

    auto value() const -> const T& {
        if (is_error()) {
            throw std::runtime_error("Attempted to access value of error Result: " + error_message());
        }
        return std::get<T>(m_data);
    }

    auto value() -> T& {
        if (is_error()) {
            throw std::runtime_error("Attempted to access value of error Result: " + error_message());
        }
        return std::get<T>(m_data);
    }

    auto value_or(T default_val) const -> T {
        if (is_ok()) {
            return std::get<T>(m_data);
        }
        return default_val;
    }

    auto error_message() const -> const std::string& {
        if (is_ok()) {
            static const std::string empty;
            return empty;
        }
        return std::get<detail::ErrorWrapper>(m_data).message;
    }

    // Monadic operations

    /**
     * @brief Transform the value if ok, propagate error otherwise
     *
     * If this Result is ok, applies func to the value and returns a new Result
     * containing the transformed value. If this Result is an error, returns
     * a new Result with the same error message.
     */
    template<typename F>
    auto map(F&& func) const -> Result<std::invoke_result_t<F, const T&>> {
        using U = std::invoke_result_t<F, const T&>;
        if (is_ok()) {
            return Result<U>::ok(func(std::get<T>(m_data)));
        }
        return Result<U>::error(error_message());
    }

    /**
     * @brief Chain operations that return Result
     *
     * If this Result is ok, applies func to the value (func must return a Result).
     * If this Result is an error, returns a new Result with the same error message.
     */
    template<typename F>
    auto and_then(F&& func) const -> std::invoke_result_t<F, const T&> {
        using ResultU = std::invoke_result_t<F, const T&>;
        if (is_ok()) {
            return func(std::get<T>(m_data));
        }
        return ResultU::error(error_message());
    }

private:
    // Constructor for ok value
    explicit Result(T value) : m_data(std::move(value)) {}

    // Constructor for error
    explicit Result(detail::ErrorWrapper err) : m_data(std::move(err)) {}

    std::variant<T, detail::ErrorWrapper> m_data;
};


/**
 * @brief Specialization of Result for void return type
 *
 * Used for operations that either succeed (with no value) or fail with an error.
 * This is useful for operations like validation or side-effect-only functions.
 */
template<>
class Result<void> {
public:
    // Factory methods
    static auto ok() -> Result<void> {
        return Result<void>(true);
    }

    static auto error(std::string message) -> Result<void> {
        return Result<void>(std::move(message));
    }

    // Accessors
    auto is_ok() const -> bool {
        return m_is_ok;
    }

    auto is_error() const -> bool {
        return !m_is_ok;
    }

    auto error_message() const -> const std::string& {
        return m_message;
    }

    /**
     * @brief Chain operations that return Result<void>
     *
     * If this Result is ok, calls func and returns its result.
     * If this Result is an error, returns a new Result with the same error message.
     */
    template<typename F>
    auto and_then(F&& func) const -> std::invoke_result_t<F> {
        using ResultU = std::invoke_result_t<F>;
        if (is_ok()) {
            return func();
        }
        return ResultU::error(error_message());
    }

private:
    // Constructor for ok
    explicit Result(bool) : m_is_ok(true), m_message() {}

    // Constructor for error
    explicit Result(std::string message) : m_is_ok(false), m_message(std::move(message)) {}

    bool m_is_ok;
    std::string m_message;
};

}  // namespace Framework
