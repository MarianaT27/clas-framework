#pragma once

#include <string>
#include <vector>

#include <fmt/core.h>


/**
 * @brief String manipulation utility functions
 */
namespace StringUtils {

/**
 * @brief Formats a value into a string with specified precision.
 *
 * @tparam T The type of the value to format
 * @param value The value to format
 * @param precision Number of decimal places (default 2)
 * @return Formatted string representation
 */
template <typename T>
std::string format_number(const T& value, int precision = 2) {
    return fmt::format("{:.{}f}", value, precision);
}

/**
 * @brief Replaces all occurrences of a substring.
 *
 * @param str The input string
 * @param from The substring to find
 * @param to The replacement substring
 * @return String with all replacements made
 */
inline auto replace_all(std::string str, const std::string& from, const std::string& to) -> std::string {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

/**
 * @brief Splits a string by a delimiter character.
 *
 * @param str The input string
 * @param delimiter The character to split on
 * @return Vector of substrings
 */
inline auto split(const std::string& str, char delimiter) -> std::vector<std::string> {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        std::string token = str.substr(start, end - start);
        if (!token.empty()) {
            result.push_back(token);
        }
        start = end + 1;
        end = str.find(delimiter, start);
    }

    std::string last = str.substr(start);
    if (!last.empty()) {
        result.push_back(last);
    }

    return result;
}

}  // namespace StringUtils

