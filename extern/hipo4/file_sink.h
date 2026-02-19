#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "result.h"

namespace hipo {

/**
 * @brief POSIX file wrapper for writing HIPO files.
 *
 * Provides sequential append writes via write() and random-access
 * writes via write_at() (used for patching header fields such as
 * the trailer position after all records have been written).
 *
 * Instances are move-only and manage the underlying file descriptor lifetime.
 * Use the static factory method open() to create instances.
 *
 * @note NOT thread-safe. All writes to a single file_sink must be serialized.
 */
class file_sink {
public:
    /** @brief Constructs a closed (invalid) file_sink. */
    file_sink() = default;

    /** @brief Destructor. Closes the file descriptor if open. */
    ~file_sink();

    /** @brief Move constructor. Transfers ownership of the file descriptor. */
    file_sink(file_sink&& other) noexcept;

    /** @brief Move assignment. Closes any currently open file and transfers ownership. */
    file_sink& operator=(file_sink&& other) noexcept;

    file_sink(const file_sink&) = delete;
    file_sink& operator=(const file_sink&) = delete;

    /**
     * @brief Opens (or creates) a file for writing.
     *
     * Creates a new file_sink by opening the file at the given path
     * with O_WRONLY | O_CREAT | O_TRUNC. Any existing file is truncated.
     *
     * @param path Filesystem path to the file to create or overwrite.
     * @return result<file_sink> The opened file sink, or an error
     *         if the file cannot be opened.
     */
    static result<file_sink> open(std::string_view path);

    /**
     * @brief Writes data at the current file position (append mode).
     *
     * Advances the internal position tracker by the number of bytes written.
     *
     * @param data Pointer to the data to write.
     * @param size Number of bytes to write.
     * @return result<void> Success, or an error if the write fails.
     */
    result<void> write(const char* data, int size);

    /**
     * @brief Writes data at a specific file position using pwrite.
     *
     * Used for patching previously written header fields (e.g., writing
     * the trailer position back into the file header at offset 40).
     * Does not change the current append position.
     *
     * @param pos Absolute byte offset within the file to write at.
     * @param data Pointer to the data to write.
     * @param size Number of bytes to write.
     * @return result<void> Success, or an error if the write fails.
     */
    result<void> write_at(int64_t pos, const char* data, int size);

    /**
     * @brief Returns the current write position (total bytes written so far).
     * @return The current byte offset for the next append write.
     */
    int64_t position() const noexcept { return m_position; }

    /**
     * @brief Checks whether the file is currently open.
     * @return true if the file descriptor is valid.
     */
    bool    is_open()  const noexcept { return m_fd >= 0; }

    /**
     * @brief Closes the file descriptor.
     *
     * After calling close(), is_open() returns false and further
     * write operations will fail.
     */
    void    close();

    /**
     * @brief Returns the filesystem path used to open this file.
     * @return A string_view of the stored path.
     */
    std::string_view path() const noexcept { return m_path; }

private:
    int         m_fd       = -1;
    std::string m_path;
    int64_t     m_position = 0;
};

} // namespace hipo
