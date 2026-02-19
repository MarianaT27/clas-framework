#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "column_stats.h"
#include "dictionary.h"
#include "event_builder.h"
#include "file_sink.h"
#include "record_builder.h"
#include "result.h"

namespace hipo {

/**
 * @brief Options controlling how a writer creates a HIPO file.
 *
 * @see writer::open
 */
struct writer_options {
    compression_type compression        = compression_type::lz4;   /**< @brief Compression algorithm for data records. */
    int              max_events         = 100000;                  /**< @brief Maximum events per record before automatic flush. */
    int              max_record_bytes   = 2 * 1024 * 1024;        /**< @brief Maximum uncompressed record size in bytes before automatic flush. */
    bool             collect_statistics = false;                   /**< @brief Opt-in per-column min/max statistics collection. */
};

/**
 * @brief Callback signature invoked after each record is flushed to disk.
 *
 * Parameters: (record_index, file_position, record_bytes, event_count).
 */
using record_callback = std::function<void(int, int64_t, int, int)>;

/**
 * @brief Sequential writer for HIPO files.
 *
 * Accumulates events into records using a record_builder, compresses and
 * flushes them to disk via a file_sink, and writes a trailer with the
 * record index table on close. Optionally collects per-column min/max
 * statistics that are stored in the trailer for downstream pruning.
 *
 * Typical usage:
 * @code
 * hipo::writer w;
 * w.set_dictionary(dict);
 * auto res = w.open("output.hipo");
 * // ... add events ...
 * w.close();
 * @endcode
 *
 * @note NOT thread-safe. A single writer must not be accessed from
 *       multiple threads concurrently.
 *
 * @see event_builder, record_builder, file_sink
 */
class writer {
public:
    writer() = default;
    ~writer();

    writer(writer&&) = default;
    writer& operator=(writer&&) = default;
    writer(const writer&) = delete;
    writer& operator=(const writer&) = delete;

    /**
     * @brief Set the schema dictionary to be written into the file header.
     *
     * Must be called before open(). The dictionary is serialised into the
     * first record's user header.
     *
     * @param dict The dictionary containing all schemas for this file.
     */
    void set_dictionary(const dictionary& dict);

    /**
     * @brief Return a mutable reference to the internal dictionary.
     * @return Reference to the dictionary.
     */
    dictionary& get_dictionary() noexcept { return m_dict; }

    /**
     * @brief Open a file for writing.
     *
     * Writes the file header and the dictionary record. The writer is
     * ready to accept events after a successful open.
     *
     * @param path Filesystem path of the output file (created or truncated).
     * @param opts Writer options (compression, flush thresholds, statistics).
     * @return result<void> indicating success or an I/O error.
     */
    result<void> open(std::string_view path, writer_options opts = {});

    /**
     * @brief Add a pre-built event to the current record.
     *
     * If the record exceeds its size or event-count thresholds, it is
     * automatically flushed to disk before the new event is added.
     *
     * @param ev The event_builder holding the serialised event data.
     * @return result<void> indicating success or an I/O error on flush.
     */
    result<void> add_event(const event_builder& ev);

    /**
     * @brief Add a raw event buffer to the current record.
     *
     * Overload accepting a pre-serialised event as a span of bytes.
     *
     * @param event_data Raw serialised event bytes.
     * @return result<void> indicating success or an I/O error on flush.
     */
    result<void> add_event(std::span<const char> event_data);

    /**
     * @brief Flush the current record to disk immediately.
     * @return result<void> indicating success or an I/O error.
     */
    result<void> flush();

    /**
     * @brief Close the file.
     *
     * Flushes remaining events, writes the trailer containing the record
     * index table (and optional statistics), and patches the file header
     * with the trailer position.
     *
     * @return result<void> indicating success or an I/O error.
     */
    result<void> close();

    /**
     * @brief Set user word one in the file header.
     * @param w The 64-bit value to store.
     */
    void set_user_word_one(int64_t w) noexcept;

    /**
     * @brief Set user word two in the file header.
     * @param w The 64-bit value to store.
     */
    void set_user_word_two(int64_t w) noexcept;

    /**
     * @brief Register a callback invoked after each record is flushed to disk.
     * @param cb Callback with signature void(int record_index, int64_t file_position, int record_bytes, int event_count).
     */
    void on_record_written(record_callback cb) { m_on_record_written = std::move(cb); }

    /**
     * @brief Check whether the writer currently has a file open.
     * @return True if open() succeeded and close() has not been called.
     */
    bool is_open() const noexcept { return m_is_open; }

    /**
     * @brief Access per-record column statistics.
     *
     * Only populated when writer_options::collect_statistics is true.
     *
     * @return A const reference to the vector of record_column_stats.
     */
    const std::vector<record_column_stats>& record_stats() const noexcept { return m_record_stats; }

private:
    result<void> write_record();
    result<void> write_index_table();
    result<void> write_stats_table();

    dictionary                          m_dict;
    file_sink                           m_sink;
    record_builder                      m_builder;
    writer_options                      m_opts;
    std::vector<record_info>            m_record_infos;
    record_callback                     m_on_record_written;
    bool                                m_is_open = false;

    // Per-column statistics (opt-in)
    column_stats_tracker                m_stats_tracker;
    std::vector<record_column_stats>    m_record_stats;
};

} // namespace hipo
