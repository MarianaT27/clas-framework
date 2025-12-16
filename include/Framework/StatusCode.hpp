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
 * Status codes:
 * - SUCCESS: Operation completed successfully, event passes
 * - FAILURE: Fatal error, stops processing
 * - FILTERED: Event did not pass cuts, skip to next event (not an error)
 * 
 * Enhanced to carry error context information when failures occur.
 */
class StatusCode {
public:
    enum class Code {
        SUCCESS = 0,   ///< Operation succeeded, event passes
        FAILURE = 1,   ///< Fatal error, stops processing
        FILTERED = 2   ///< Event filtered out (cut not passed), continue to next event
    };

    // Constructor from Code (backward compatible)
    StatusCode(Code code) : m_code(code), m_message() {}
    
    // Constructor with code and message
    StatusCode(Code code, std::string message) : m_code(code), m_message(std::move(message)) {}

    auto is_success() const -> bool { return m_code == Code::SUCCESS; }
    auto is_failure() const -> bool { return m_code == Code::FAILURE; }
    auto is_filtered() const -> bool { return m_code == Code::FILTERED; }
    
    /// @brief Check if processing should continue (SUCCESS or FILTERED)
    auto should_continue() const -> bool { return m_code != Code::FAILURE; }
    
    // Get the error message (empty for success/filtered)
    auto message() const -> const std::string& { return m_message; }
    
    // Get the underlying code
    auto code() const -> Code { return m_code; }

    operator bool() const { return is_success(); }

    // Factory methods (uppercase for backward compatibility)
    static auto SUCCESS() -> StatusCode { return StatusCode(Code::SUCCESS); }
    static auto FAILURE() -> StatusCode { return StatusCode(Code::FAILURE); }
    static auto FILTERED() -> StatusCode { return StatusCode(Code::FILTERED); }
    
    // Enhanced failure() with error message
    static auto failure(std::string message) -> StatusCode { 
        return StatusCode(Code::FAILURE, std::move(message)); 
    }
    
    // Filtered with optional reason
    static auto filtered(std::string reason = "") -> StatusCode {
        return StatusCode(Code::FILTERED, std::move(reason));
    }
    
    // Enhanced success() for consistency (lowercase)
    static auto success() -> StatusCode { return StatusCode(Code::SUCCESS); }

private:
    Code m_code;
    std::string m_message;
};

}  // namespace Framework
