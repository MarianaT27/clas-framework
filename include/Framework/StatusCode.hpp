#pragma once

#include <string>
#include <fmt/format.h>

namespace Framework {

/**
 * @brief Status code for algorithm and service operations
 *
 * Similar to Gaudi/Athena StatusCode, this class provides a standardized
 * way to report success or failure of operations in the framework.
 * 
 * Enhanced to carry error context information when failures occur.
 */
class StatusCode {
public:
    enum class Code {
        SUCCESS = 0,
        FAILURE = 1
    };

    // Constructor from Code (backward compatible)
    StatusCode(Code code) : m_code(code), m_message() {}
    
    // Constructor with code and message
    StatusCode(Code code, std::string message) : m_code(code), m_message(std::move(message)) {}

    auto is_success() const -> bool { return m_code == Code::SUCCESS; }
    auto is_failure() const -> bool { return m_code == Code::FAILURE; }
    
    // Get the error message (empty for success)
    auto message() const -> const std::string& { return m_message; }

    operator bool() const { return is_success(); }

    // Factory methods
    static auto SUCCESS() -> StatusCode { return StatusCode(Code::SUCCESS); }
    
    // Backward compatible FAILURE() - no message
    static auto FAILURE() -> StatusCode { return StatusCode(Code::FAILURE); }
    
    // Enhanced failure() with error message
    static auto failure(std::string message) -> StatusCode { 
        return StatusCode(Code::FAILURE, std::move(message)); 
    }
    
    // Enhanced success() for consistency (lowercase)
    static auto success() -> StatusCode { return StatusCode(Code::SUCCESS); }

private:
    Code m_code;
    std::string m_message;
};

}  // namespace Framework
