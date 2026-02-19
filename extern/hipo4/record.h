#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

#include "constants.h"
#include "io.h"
#include "result.h"

namespace hipo {

/**
 * @brief Position and length of a bank structure within an event's data.
 *
 * Used by record::find_structure and record::find_structures_in_event to
 * report where a specific (group, item) structure resides inside a
 * decompressed event buffer.
 */
struct structure_position {
    int offset = -1;  /**< @brief Byte offset within the event data, or -1 if not found. */
    int length = 0;   /**< @brief Structure data length in bytes (excluding the 8-byte structure header). */

    /**
     * @brief Check whether the structure was found.
     * @return True if offset >= 0.
     */
    bool found() const noexcept { return offset >= 0; }
};

/**
 * @brief Parsed 56-byte HIPO record header.
 *
 * Contains both the raw header words read from the file and derived
 * fields computed during parsing (compression type, padding sizes, etc.).
 *
 * @see record::header
 */
struct record_header {
    int32_t  record_length        = 0; /**< @brief Total record length in bytes (header word 1 * 4). */
    int32_t  record_number        = 0; /**< @brief Sequential record number (header word 2). */
    int32_t  header_length        = 0; /**< @brief Header length in 32-bit words (header word 3). */
    int32_t  event_count          = 0; /**< @brief Number of events in this record (header word 4). */
    int32_t  index_data_length    = 0; /**< @brief Index data length in bytes (header word 5, = event_count * 4). */
    int32_t  bit_info             = 0; /**< @brief Bit-info word encoding version and padding (header word 6). */
    int32_t  user_header_length   = 0; /**< @brief User header length in bytes (header word 7). */
    int32_t  magic_number         = 0; /**< @brief Magic number for endianness detection (header word 8, expected 0xc0da0100). */
    int32_t  data_length          = 0; /**< @brief Uncompressed data length in bytes (header word 9). */
    int32_t  compression_word     = 0; /**< @brief Compression word (header word 10): type in bits [28-31], compressed length in words in bits [0-27]. */

    // Derived fields
    compression_type comp_type    = compression_type::none; /**< @brief Compression type extracted from compression_word. */
    int32_t  compressed_length    = 0; /**< @brief Compressed data length in bytes (derived from compression_word). */
    int32_t  user_header_padding  = 0; /**< @brief User header padding bytes (pad1 from bit-info). */
    int32_t  data_padding         = 0; /**< @brief Data padding bytes (pad2 from bit-info). */
    int32_t  comp_data_padding    = 0; /**< @brief Compressed data padding bytes (pad3 from bit-info). */
    bool     byte_swapped         = false; /**< @brief Whether the record requires byte-swapping (opposite endianness). */
};

/**
 * @brief A decompressed HIPO data record providing access to individual events.
 *
 * Reads a record from a file_source, decompresses it (LZ4 or GZIP),
 * converts the event-length index to cumulative offsets, and provides
 * zero-copy access to individual events and their bank structures.
 *
 * After a successful read_from(), the internal buffer layout is:
 * @code
 * [0 .. index_data_length)                    = cumulative offset index
 * [index_data_length .. events_offset)        = user header + padding
 * [events_offset .. data_length)              = event data
 * @endcode
 *
 * @note NOT thread-safe. A record instance must not be shared between
 *       threads. The chain class allocates one record per worker thread.
 *
 * @see reader, chain, structure_position
 */
class record {
public:
    record() = default;
    ~record() = default;
    record(record&&) = default;
    record& operator=(record&&) = default;
    record(const record&) = delete;
    record& operator=(const record&) = delete;

    /**
     * @brief Read and decompress a record from a file source.
     *
     * Reads the record header, then the compressed payload, decompresses
     * it, and converts the event-length index to cumulative offsets.
     *
     * @param source    Open file source for pread access.
     * @param position  Byte offset of the record within the file.
     * @param file_size Total file size (for bounds checking).
     * @return result<void> indicating success or a parse/I/O error.
     */
    result<void> read_from(file_source& source, int64_t position, int64_t file_size);

    /**
     * @brief Read and decompress a record with a known byte size.
     *
     * Optimised overload that reads the entire record in a single pread
     * when the record size is known from the trailer index.
     *
     * @param source       Open file source for pread access.
     * @param position     Byte offset of the record within the file.
     * @param file_size    Total file size (for bounds checking).
     * @param record_bytes Known total size of the record in bytes.
     * @return result<void> indicating success or a parse/I/O error.
     */
    result<void> read_from(file_source& source, int64_t position, int64_t file_size, int record_bytes);

    /**
     * @brief Return the number of events in this record.
     * @return Event count from the parsed record header.
     */
    int event_count() const noexcept { return m_header.event_count; }

    /**
     * @brief Return a read-only span over the raw bytes of an event.
     * @param index Zero-based event index within this record.
     * @return A result containing the event data span, or an error if the index is out of range.
     */
    result<std::span<const char>> event_data(int index) const noexcept;

    /**
     * @brief Find a specific bank structure within an event (zero-copy).
     *
     * Scans the event for a structure matching the given group and item
     * identifiers and returns a span over its data payload.
     *
     * @param event_index Zero-based event index within this record.
     * @param group       Bank group number.
     * @param item        Bank item number.
     * @return A result containing the structure data span, or an error if not found.
     */
    result<std::span<const char>> find_structure(int event_index, int group, int item) const noexcept;

    /**
     * @brief Batch-find multiple bank structures within an event.
     *
     * Scans the event once to locate all requested (group, item) pairs,
     * writing results into the provided output span.
     *
     * @param event_index Zero-based event index within this record.
     * @param group_items Span of (group, item) pairs to search for.
     * @param results     Output span of structure_position (same length as group_items).
     */
    void find_structures_in_event(int event_index,
                                  std::span<const std::pair<int,int>> group_items,
                                  std::span<structure_position> results) const noexcept;

    /**
     * @brief Batch-find multiple bank structures within a pre-extracted event span.
     *
     * Static overload that operates on raw event data, avoiding a
     * duplicate event_data() lookup when the caller already holds
     * the event span.
     *
     * @param event       Raw event data span (as returned by event_data()).
     * @param group_items Span of (group, item) pairs to search for.
     * @param results     Output span of structure_position (same length as group_items).
     */
    static void find_structures_in_event(std::span<const char> event,
                                         std::span<const std::pair<int,int>> group_items,
                                         std::span<structure_position> results) noexcept;

    /**
     * @brief Return a read-only span over the user header.
     *
     * The user header is used for the dictionary in the first data record.
     *
     * @return Span over the user header bytes (may be empty).
     */
    std::span<const char> user_header() const noexcept;

    /**
     * @brief Return a const reference to the parsed record header.
     * @return The record_header struct.
     */
    const record_header& header() const noexcept { return m_header; }

    /**
     * @brief Pre-allocate internal buffers to avoid reallocation during the first records.
     * @param compressed_bytes   Expected compressed buffer size in bytes.
     * @param decompressed_bytes Expected decompressed buffer size in bytes.
     */
    void reserve_buffers(int compressed_bytes, int decompressed_bytes);

private:
    result<void> parse_header(const char* header_buf);
    result<void> decompress_data(const char* compressed, int compressed_len, int decompressed_len);
    result<void> read_and_decompress(file_source& source, int64_t data_position,
                                      int compressed_input_len, int decompressed_len);
    void         convert_index();

    record_header       m_header;
    std::vector<char>   m_compressed_buf;
    std::vector<char>   m_decompressed_buf;
    // After convert_index: m_decompressed_buf layout:
    //   [0 .. index_data_length)                     = cumulative offset index
    //   [index_data_length .. events_offset)          = user header + padding
    //   [events_offset .. data_length)                = event data
    int                 m_events_offset = 0;
};

} // namespace hipo
