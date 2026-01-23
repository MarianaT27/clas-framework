#pragma once

#include <fmt/format.h>
#include <string>
#include <string_view>

namespace Framework {

/**
 * @brief Status code for algorithm and service operations
 *
 * Similar to Gaudi/Athena StatusCode, this class provides a standardized
 * way to report success or failure of operations in the framework.
 *
 * Status codes:
 * - SUCCESS: Operation completed successfully, event passes
 * - FAILURE: Fatal error, stops processing
 * - FILTERED: Event did not pass cuts, skip to next event (not an error)
 *
 * Enhanced to carry error context information when failures occur.
 *
 * C++ Core Guidelines:
 * - F.48: Don't return std::move(local) - compiler handles this
 * - C.44: Prefer default constructors to be simple and non-throwing
 * - ES.28: Use lambdas for complex initialization
 */
class StatusCode {
public:
    enum class Code : std::uint8_t {
        SUCCESS = 0,   ///< Operation succeeded, event passes
        FAILURE = 1,   ///< Fatal error, stops processing
        FILTERED = 2   ///< Event filtered out (cut not passed), continue to next event
    };

    // Constructors (C++ Core Guidelines: C.45 - Don't define default ctor that only initializes)
    constexpr StatusCode(Code code) noexcept : m_code(code) {}
    StatusCode(Code code, std::string message) : m_code(code), m_message(std::move(message)) {}

    // Observers (C++ Core Guidelines: C.12 - Don't make data members const or references)
    [[nodiscard]] constexpr auto is_success() const noexcept -> bool { return m_code == Code::SUCCESS; }
    [[nodiscard]] constexpr auto is_failure() const noexcept -> bool { return m_code == Code::FAILURE; }
    [[nodiscard]] constexpr auto is_filtered() const noexcept -> bool { return m_code == Code::FILTERED; }
    [[nodiscard]] constexpr auto should_continue() const noexcept -> bool { return m_code != Code::FAILURE; }
    [[nodiscard]] auto message() const noexcept -> const std::string& { return m_message; }
    [[nodiscard]] constexpr auto code() const noexcept -> Code { return m_code; }

    // Boolean conversion (explicit to avoid accidental conversions)
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return is_success(); }

    // Comparison operators (C++ Core Guidelines: C.161 - Use non-member functions for symmetric operators)
    [[nodiscard]] constexpr auto operator==(const StatusCode& other) const noexcept -> bool {
        return m_code == other.m_code;
    }
    [[nodiscard]] constexpr auto operator==(Code code) const noexcept -> bool {
        return m_code == code;
    }

    // Factory methods - uppercase for backward compatibility
    [[nodiscard]] static constexpr auto SUCCESS() noexcept -> StatusCode { return StatusCode(Code::SUCCESS); }
    [[nodiscard]] static constexpr auto FAILURE() noexcept -> StatusCode { return StatusCode(Code::FAILURE); }
    [[nodiscard]] static constexpr auto FILTERED() noexcept -> StatusCode { return StatusCode(Code::FILTERED); }

    // Factory methods - lowercase for modern style
    [[nodiscard]] static constexpr auto success() noexcept -> StatusCode { return StatusCode(Code::SUCCESS); }
    [[nodiscard]] static auto failure(std::string message) -> StatusCode {
        return StatusCode(Code::FAILURE, std::move(message));
    }
    [[nodiscard]] static auto filtered(std::string reason = "") -> StatusCode {
        return StatusCode(Code::FILTERED, std::move(reason));
    }

    // Utility: combine status codes (returns first failure)
    [[nodiscard]] auto and_then(const StatusCode& other) const -> StatusCode {
        return is_failure() ? *this : other;
    }

    // String representation for debugging
    [[nodiscard]] auto to_string() const -> std::string {
        switch (m_code) {
            case Code::SUCCESS: return "SUCCESS";
            case Code::FAILURE: return m_message.empty() ? "FAILURE" : fmt::format("FAILURE: {}", m_message);
            case Code::FILTERED: return m_message.empty() ? "FILTERED" : fmt::format("FILTERED: {}", m_message);
        }
        return "UNKNOWN";
    }

private:
    Code m_code;
    std::string m_message;
};

}  // namespace Framework
