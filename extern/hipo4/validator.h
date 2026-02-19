#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "constants.h"
#include "dictionary.h"
#include "record.h"
#include "result.h"

namespace hipo {

/**
 * @brief Severity level for validation issues found during file inspection.
 */
enum class severity {
    warning, /**< @brief Non-fatal issue that may indicate a problem. */
    error    /**< @brief Fatal issue that indicates corruption or invalid structure. */
};

/**
 * @brief Describes a single validation issue found during file inspection.
 *
 * Contains the severity, a human-readable message, and optionally
 * the byte offset within the file where the issue was detected.
 */
struct validation_issue {
    severity    level;                /**< @brief Severity of this issue. */
    std::string message;             /**< @brief Human-readable description of the issue. */
    int64_t     file_offset = -1;    /**< @brief Byte offset in the file, or -1 if not applicable. */
};

/**
 * @brief Validates HIPO file structure and consistency.
 *
 * Provides static methods to check individual headers, full files,
 * and dictionary consistency. All methods return result<void> on
 * success or an error describing the validation failure.
 *
 * @note All methods are static; no instance state is required.
 */
class validator {
public:
    /**
     * @brief Validates a HIPO file header from a raw byte buffer.
     *
     * Checks the unique word, magic number, header length, bit-info version,
     * and trailer position for consistency.
     *
     * @param buf Pointer to a buffer containing at least FILE_HEADER_SIZE bytes.
     * @param file_size Total size of the file in bytes (used for bounds checks).
     * @return result<void> Success if the header is valid, or an error describing
     *         the first validation failure encountered.
     */
    static result<void> validate_file_header(const char* buf, int64_t file_size);

    /**
     * @brief Validates a parsed record header for structural correctness.
     *
     * Checks that record length, data length, and compression fields
     * are consistent and within file bounds.
     *
     * @param hdr The parsed record header to validate.
     * @param position Byte offset of this record within the file.
     * @param file_size Total file size in bytes (used for bounds checks).
     * @return result<void> Success if the header is valid, or an error describing
     *         the first validation failure encountered.
     */
    static result<void> validate_record_header(const record_header& hdr, int64_t position, int64_t file_size);

    /**
     * @brief Validates a HIPO file trailer record.
     *
     * Checks trailer-specific fields such as the last-record flag
     * and index array consistency.
     *
     * @param hdr The parsed trailer record header to validate.
     * @param position Byte offset of the trailer within the file.
     * @param file_size Total file size in bytes.
     * @return result<void> Success if the trailer is valid, or an error.
     */
    static result<void> validate_trailer(const record_header& hdr, int64_t position, int64_t file_size);

    /**
     * @brief Performs a full validation scan of a HIPO file on disk.
     *
     * Opens the file, validates the file header, iterates through all
     * records checking each header, and validates the trailer. Collects
     * all issues (warnings and errors) encountered during the scan.
     *
     * @param path Filesystem path to the HIPO file to validate.
     * @return result<std::vector<validation_issue>> A list of all issues
     *         found, or an error if the file cannot be opened.
     */
    static result<std::vector<validation_issue>> validate_file(std::string_view path);

    /**
     * @brief Validates that two dictionaries have consistent schema definitions.
     *
     * Checks that schemas with the same (group, item) pair have identical
     * column layouts across both dictionaries.
     *
     * @param d1 The first dictionary to compare.
     * @param d2 The second dictionary to compare.
     * @return result<void> Success if consistent, or an error describing
     *         the first inconsistency found.
     */
    static result<void> validate_schema_consistency(const dictionary& d1, const dictionary& d2);

    /**
     * @brief Validates the internal consistency of a single dictionary.
     *
     * Delegates to dictionary::validate() to check for duplicate schemas,
     * invalid column types, and other structural issues.
     *
     * @param d The dictionary to validate.
     * @return result<void> Success if valid, or an error describing the issue.
     */
    static result<void> validate_dictionary(const dictionary& d) { return d.validate(); }
};

} // namespace hipo
