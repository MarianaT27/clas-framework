#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "result.h"

namespace hipo {

/**
 * @brief POSIX file wrapper for reading HIPO files using pread.
 *
 * Provides position-independent reads via pread(), which is thread-safe
 * for concurrent calls on the same file descriptor. A single file_source
 * can be shared across multiple threads.
 *
 * Uses posix_fadvise for kernel read-ahead and prefetching.
 *
 * Instances are move-only and manage the underlying file descriptor lifetime.
 * Use the static factory method open() to create instances.
 *
 * @note Thread-safe for concurrent read_at() and prefetch() calls.
 */
class file_source {
public:
    /** @brief Constructs a closed (invalid) file_source. */
    file_source() = default;

    /** @brief Destructor. Closes the file descriptor if open. */
    ~file_source();

    /** @brief Move constructor. Transfers ownership of the file descriptor. */
    file_source(file_source&& other) noexcept;

    /** @brief Move assignment. Closes any currently open file and transfers ownership. */
    file_source& operator=(file_source&& other) noexcept;

    file_source(const file_source&) = delete;
    file_source& operator=(const file_source&) = delete;

    /**
     * @brief Opens a file for reading.
     *
     * Opens the file with O_RDONLY and enables kernel read-ahead
     * via posix_fadvise(SEQUENTIAL).
     *
     * @param path Filesystem path to the file to open.
     * @return result<file_source> The opened file source, or an error
     *         if the file cannot be opened or stat'd.
     */
    static result<file_source> open(std::string_view path);

    /**
     * @brief Reads bytes from an absolute file position using pread.
     *
     * Does not modify any internal seek state, making it safe for
     * concurrent calls from multiple threads on the same instance.
     *
     * @param pos Absolute byte offset within the file to read from.
     * @param dest Destination buffer (must have at least @p size bytes).
     * @param size Number of bytes to read.
     * @return result<int> The number of bytes actually read, or an error
     *         if the read fails.
     */
    result<int> read_at(int64_t pos, char* dest, int size);

    /**
     * @brief Advise the kernel to prefetch a region of the file.
     *
     * Uses posix_fadvise(WILLNEED) to trigger asynchronous read-ahead
     * into the page cache. Non-blocking — returns immediately.
     *
     * @param pos  Byte offset to start prefetching.
     * @param size Number of bytes to prefetch.
     */
    void prefetch(int64_t pos, int64_t size) const noexcept;

    /**
     * @brief Returns the total file size in bytes.
     * @return File size determined at open time.
     */
    int64_t     size() const noexcept { return m_size; }

    /**
     * @brief Returns the underlying POSIX file descriptor.
     * @return The file descriptor, or -1 if not open.
     */
    int         fd() const noexcept { return m_fd; }

    /**
     * @brief Checks whether the file is currently open.
     * @return true if the file descriptor is valid.
     */
    bool        is_open() const noexcept { return m_fd >= 0; }

    /**
     * @brief Closes the file descriptor.
     *
     * After calling close(), is_open() returns false and further
     * read_at() calls will fail.
     */
    void        close();

    /**
     * @brief Returns the filesystem path used to open this file.
     * @return A string_view of the stored path.
     */
    std::string_view path() const noexcept { return m_path; }

private:
    int         m_fd   = -1;
    std::string m_path;
    int64_t     m_size = 0;
};

} // namespace hipo
