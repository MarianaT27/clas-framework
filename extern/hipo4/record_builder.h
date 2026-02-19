#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "constants.h"
#include "result.h"

namespace hipo {

/**
 * @brief Metadata describing a built record's position and size within a HIPO file.
 *
 * Used by the writer to construct the file trailer index table,
 * which maps record positions to their lengths and entry counts.
 */
struct record_info {
    int64_t position      = 0; /**< @brief Byte offset of the record within the file. */
    int     length        = 0; /**< @brief Record length in bytes (header + data). */
    int     entries       = 0; /**< @brief Number of events stored in the record. */
    int64_t user_word_one = 0; /**< @brief First user-defined 64-bit word. */
    int64_t user_word_two = 0; /**< @brief Second user-defined 64-bit word. */
};

/**
 * @brief Configuration options for the record_builder.
 *
 * Controls when a record is considered full and which compression
 * algorithm is used when building the record.
 */
struct record_builder_options {
    int              max_events       = 100000;          /**< @brief Maximum number of events per record. */
    int              max_record_bytes = 2 * 1024 * 1024; /**< @brief Maximum uncompressed record size in bytes. */
    compression_type compression      = compression_type::lz4; /**< @brief Compression algorithm to use. */
};

/**
 * @brief Buffers events and builds compressed HIPO records.
 *
 * Events are added one at a time via add_event(). When the record is full
 * (either max_events or max_record_bytes exceeded), the caller should
 * call build() to compress the buffered events into a complete record
 * with a 56-byte header, then reset() to prepare for the next record.
 *
 * The built record data is accessible via record_data() and record_size()
 * after a successful build().
 *
 * @note NOT thread-safe. Each thread should use its own record_builder instance.
 */
class record_builder {
public:
    /**
     * @brief Constructs a record_builder with the given options.
     * @param opts Configuration options controlling capacity and compression.
     */
    explicit record_builder(record_builder_options opts = {});
    ~record_builder() = default;

    record_builder(record_builder&&) = default;
    record_builder& operator=(record_builder&&) = default;
    record_builder(const record_builder&) = delete;
    record_builder& operator=(const record_builder&) = delete;

    /**
     * @brief Adds an event's raw bytes to the current record buffer.
     *
     * If adding this event would exceed max_events or max_record_bytes,
     * the event is not added and false is returned. The caller should
     * then call build(), reset(), and retry.
     *
     * @param event_data The raw event bytes to append.
     * @return true if the event was added, false if the record is full.
     */
    bool add_event(std::span<const char> event_data);

    /**
     * @brief Compresses buffered events and builds the final record.
     *
     * Constructs a complete HIPO record with a 56-byte header, an index
     * array of per-event lengths, and compressed event data. The result
     * is accessible via record_data() and record_size().
     *
     * @return result<void> Success, or an error if compression fails.
     */
    result<void> build();

    /**
     * @brief Resets the builder for the next record.
     *
     * Clears all buffered events and the index, but keeps internal
     * buffers allocated to avoid repeated allocations.
     */
    void reset();

    /**
     * @brief Returns a view of the built record data.
     *
     * Only valid after a successful call to build() and before the next reset().
     *
     * @return A span covering the complete record (header + compressed data).
     */
    std::span<const char> record_data() const noexcept;

    /**
     * @brief Returns the size of the built record in bytes.
     * @return Record size in bytes, or 0 if no record has been built.
     */
    int record_size() const noexcept;

    /**
     * @brief Returns the number of events currently buffered in this record.
     * @return The event count.
     */
    int entry_count() const noexcept { return m_index_count; }

    /**
     * @brief Sets the first user-defined word written into the record header.
     * @param w The 64-bit user word value.
     */
    void set_user_word_one(int64_t w) noexcept { m_user_word_one = w; }

    /**
     * @brief Sets the second user-defined word written into the record header.
     * @param w The 64-bit user word value.
     */
    void set_user_word_two(int64_t w) noexcept { m_user_word_two = w; }

    /**
     * @brief Builds a special record containing a user header payload.
     *
     * Used for writing dictionary and index records, where the payload
     * is placed in the user header section rather than as event data.
     *
     * @param user_header The raw bytes to embed as the user header.
     * @return result<void> Success, or an error if compression fails.
     */
    result<void> build_with_user_header(std::span<const char> user_header);

private:
    record_builder_options m_opts;
    std::vector<char>      m_index_buf;
    std::vector<char>      m_events_buf;
    std::vector<char>      m_data_buf;
    std::vector<char>      m_record_buf;
    int                    m_index_count  = 0;
    int                    m_events_pos   = 0;
    int                    m_record_size  = 0;  // built record size in bytes
    int64_t                m_user_word_one = 0;
    int64_t                m_user_word_two = 0;
};

} // namespace hipo
