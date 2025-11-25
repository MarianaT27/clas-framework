#pragma once

#include <string>
#include <fmt/format.h>

namespace Framework {

/**
 * @brief Status code for algorithm and service operations
 *
 * Similar to Gaudi/Athena StatusCode, this class provides a standardized
 * way to report success or failure of operations in the framework.
 */
class StatusCode {
public:
    enum class Code {
        SUCCESS = 0,
        FAILURE = 1
    };

    StatusCode(Code code) : m_code(code) {}

    auto is_success() const -> bool { return m_code == Code::SUCCESS; }
    auto is_failure() const -> bool { return m_code == Code::FAILURE; }

    operator bool() const { return is_success(); }

    static auto SUCCESS() -> StatusCode { return StatusCode(Code::SUCCESS); }
    static auto FAILURE() -> StatusCode { return StatusCode(Code::FAILURE); }

private:
    Code m_code;
};

}  // namespace Framework
