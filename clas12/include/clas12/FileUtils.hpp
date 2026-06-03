#pragma once

#include <glob.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <fmt/core.h>


/**
 * @brief File and directory utility functions
 *
 * This namespace contains general-purpose file I/O utilities that were
 * previously in Helpers.
 */
namespace FileUtils {

/**
 * @brief Reads all HIPO files recursively in a directory with optional percentage filtering.
 *
 * @param directory The root directory to search
 * @param percentage Percentage of files to include (0-100, default 100)
 * @return Vector of file paths
 * @throws std::runtime_error if directory doesn't exist
 * @throws std::out_of_range if percentage is invalid
 */
inline auto read_recursive_hipo_files(const std::filesystem::path& directory,
                                      const double percentage = 100) -> std::vector<std::string> {
    if (!std::filesystem::exists(directory)) {
        throw std::runtime_error(fmt::format("Directory '{}' does not exist", directory.string()));
    }
    if (!std::filesystem::is_directory(directory)) {
        throw std::runtime_error(fmt::format("Path '{}' is not a directory", directory.string()));
    }
    if (percentage < 0.0 || percentage > 100.0) {
        throw std::out_of_range("Percentage must be between 0 and 100. Given: " + std::to_string(percentage));
    }

    std::vector<std::string> files;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".hipo") {
            files.push_back(entry.path().string());
        }
    }

    std::sort(files.begin(), files.end());

    const std::size_t total = files.size();
    const std::size_t num_to_keep = static_cast<std::size_t>(std::round((percentage / 100.0) * total));
    if (num_to_keep < total) {
        files.resize(num_to_keep);
    }

    return files;
}

/**
 * @brief True if @p s contains shell glob metacharacters (`*`, `?`, `[`).
 */
inline auto is_glob(const std::string& s) -> bool {
    return s.find_first_of("*?[") != std::string::npos;
}

/**
 * @brief Expand a shell glob pattern (`*`, `?`, `[...]`, leading `~`) into the
 *        sorted list of matching paths.
 *
 * Uses POSIX glob(3). Returns an empty vector when the pattern matches nothing
 * (the caller decides whether that is an error) — note glob only ever returns
 * paths that actually exist, so callers can treat each result as real.
 */
inline auto expand_glob(const std::string& pattern) -> std::vector<std::string> {
    std::vector<std::string> matches;
    glob_t g{};
    const int rc = ::glob(pattern.c_str(), GLOB_TILDE | GLOB_NOSORT, nullptr, &g);
    if (rc == 0) {
        matches.reserve(g.gl_pathc);
        for (std::size_t i = 0; i < g.gl_pathc; ++i) {
            matches.emplace_back(g.gl_pathv[i]);
        }
    }
    ::globfree(&g);
    std::sort(matches.begin(), matches.end());
    return matches;
}

/**
 * @brief Reads all subdirectories in a directory.
 *
 * @param directory The directory to scan
 * @return Vector of subdirectory paths
 * @throws std::runtime_error if directory doesn't exist
 */
inline auto read_subdirectories(const std::filesystem::path& directory) -> std::vector<std::string> {
    if (!std::filesystem::exists(directory)) {
        throw std::runtime_error(fmt::format("Directory '{}' does not exist", directory.string()));
    }
    if (!std::filesystem::is_directory(directory)) {
        throw std::runtime_error(fmt::format("Path '{}' is not a directory", directory.string()));
    }

    std::vector<std::string> dirs;

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_directory()) {
            dirs.push_back(entry.path().string());
        }
    }

    std::sort(dirs.begin(), dirs.end());
    return dirs;
}

/**
 * @brief Filters file paths by run numbers.
 *
 * @param files Vector of file paths to filter
 * @param run_numbers Vector of run number strings to match
 * @return Filtered vector of file paths containing any of the run numbers
 */
inline auto filter_by_run_numbers(const std::vector<std::string>& files,
                                  const std::vector<std::string>& run_numbers) -> std::vector<std::string> {
    std::vector<std::string> filtered;
    filtered.reserve(files.size());

    for (const auto& file_name : files) {
        bool matches = std::any_of(run_numbers.begin(), run_numbers.end(),
                                   [&file_name](const std::string& run_num) {
                                       return file_name.find(run_num) != std::string::npos;
                                   });
        if (matches) {
            filtered.push_back(file_name);
        }
    }

    return filtered;
}

}  // namespace FileUtils

