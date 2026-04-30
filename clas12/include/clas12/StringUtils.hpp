#pragma once

#include <cstddef>
#include <string>
#include <string_view>
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

/**
 * @brief Brace-expansion in shell-glob style.
 *
 * Expands `{a,b,c}` groups into the cartesian product of all comma-separated
 * options. Mirrors a useful subset of bash's brace expansion so analysts can
 * write paths like:
 *
 *   /mss/clas12/rg-d/recon/{018944,018945,018946}
 *
 * which expand to three sibling paths. Multiple groups in one string compose
 * cartesian-product:
 *
 *   /run/{a,b}/{x,y}  ->  /run/a/x, /run/a/y, /run/b/x, /run/b/y
 *
 * Nested groups (`{a,{b,c}}`) work via simple recursion. Numeric ranges
 * (`{1..3}`) are NOT supported — write them out explicitly.
 *
 * Strings without `{` are returned as a one-element vector (cheap no-op).
 * Unmatched braces are left in place — the caller will see an obviously
 * broken path and fail at the filesystem layer rather than silently
 * dropping characters.
 */
inline auto expand_braces(std::string_view s) -> std::vector<std::string> {
    auto open = s.find('{');
    if (open == std::string_view::npos) return {std::string(s)};

    // Find the matching `}` at the same nesting depth.
    int depth = 1;
    auto close = std::string_view::npos;
    for (auto i = open + 1; i < s.size(); ++i) {
        if (s[i] == '{') ++depth;
        else if (s[i] == '}' && --depth == 0) { close = i; break; }
    }
    if (close == std::string_view::npos) return {std::string(s)};

    // Split the brace contents on commas at nesting depth 0.
    auto contents = s.substr(open + 1, close - open - 1);
    std::vector<std::string_view> options;
    int        d    = 0;
    std::size_t last = 0;
    for (std::size_t j = 0; j < contents.size(); ++j) {
        if      (contents[j] == '{') ++d;
        else if (contents[j] == '}') --d;
        else if (contents[j] == ',' && d == 0) {
            options.push_back(contents.substr(last, j - last));
            last = j + 1;
        }
    }
    options.push_back(contents.substr(last));

    const std::string prefix(s.substr(0, open));
    const std::string suffix(s.substr(close + 1));

    std::vector<std::string> out;
    for (const auto& opt : options) {
        // Recurse into the recombined string so we pick up further groups
        // in either the option itself or the suffix.
        for (auto& expanded : expand_braces(prefix + std::string(opt) + suffix)) {
            out.push_back(std::move(expanded));
        }
    }
    return out;
}

}  // namespace StringUtils

