#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "bank_view.h"
#include "column_stats.h"
#include "constants.h"
#include "dictionary.h"
#include "io.h"
#include "record.h"
#include "result.h"

namespace hipo {

/**
 * @brief Options controlling how a reader opens and validates a HIPO file.
 *
 * @see reader::open
 */
struct reader_options {
    bool               validate       = true;   /**< @brief Whether to validate record headers on read. */
    std::vector<long>  tags           = {};     /**< @brief Event tags to accept (empty = accept all). */
};

/**
 * @brief Lightweight metadata obtained from a fast file scan.
 *
 * Returned by reader::scan_metadata without parsing the dictionary or
 * decompressing any data records.
 *
 * @see reader::scan_metadata
 */
struct file_metadata {
    int64_t total_events = 0;   /**< @brief Total number of events in the file. */
    int     num_records  = 0;   /**< @brief Number of data records in the file. */
};

/**
 * @brief Comprehensive information about an opened HIPO file.
 *
 * Populated during reader::open and accessible via reader::info().
 *
 * @see reader::info
 */
struct file_info {
    std::string path;                       /**< @brief Filesystem path of the opened file. */
    int64_t     file_size       = 0;        /**< @brief Total file size in bytes. */
    int         version         = 0;        /**< @brief HIPO format version extracted from the file header. */
    int         record_count    = 0;        /**< @brief Number of data records. */
    int         total_events    = 0;        /**< @brief Total event count across all records. */
    int64_t     trailer_position = 0;       /**< @brief Byte offset of the trailer record. */
    bool        has_trailer_index = false;   /**< @brief Whether the trailer contains an index table. */
};

/**
 * @brief Cumulative event index enabling O(log R) random access by event number.
 *
 * Maintains a cumulative event count per record together with file positions
 * and byte sizes so that the reader can jump to any event via binary search
 * on the record index.
 */
class file_event_index {
public:
    file_event_index() = default;

    /**
     * @brief Register a record in the index.
     * @param event_count  Number of events contained in the record.
     * @param position     Byte offset of the record within the file.
     * @param record_bytes Total byte size of the record (0 if unknown).
     */
    void add_record(int event_count, int64_t position, int record_bytes = 0);

    /** @brief Remove all entries and reset the index to its initial state. */
    void clear();

    /** @brief Rewind the cursor to the position before the first event. */
    void rewind();

    /**
     * @brief Check whether the cursor can advance to the next event.
     * @return True if there is at least one more event after the current position.
     */
    bool can_advance() const noexcept;

    /**
     * @brief Advance the cursor to the next event.
     * @return True if the advance succeeded, false if already at the end.
     */
    bool advance() noexcept;

    /**
     * @brief Jump the cursor to a specific event number.
     * @param event_number Zero-based global event index.
     * @return True if the event number is within range and the cursor was moved.
     */
    bool goto_event(int event_number) noexcept;

    /**
     * @brief Return the total number of events across all registered records.
     * @return Total event count, or 0 if no records have been added.
     */
    int  max_events() const noexcept;

    /** @brief Return the current global event number (zero-based). */
    int  current_event() const noexcept { return m_current_event; }

    /** @brief Return the index of the record containing the current event. */
    int  current_record() const noexcept { return m_current_record; }

    /** @brief Return the event offset within the current record. */
    int  current_record_event() const noexcept { return m_current_record_event; }

    /**
     * @brief Return the file byte offset of the record at the given index.
     * @param record_index Zero-based record index.
     * @return File position in bytes.
     */
    int64_t record_position(int record_index) const noexcept;

    /**
     * @brief Return the byte size of the record at the given index.
     * @param record_index Zero-based record index.
     * @return Record size in bytes, or 0 if unknown.
     */
    int  record_bytes(int record_index) const noexcept;

    /** @brief Return the number of records in the index. */
    int  num_records() const noexcept { return static_cast<int>(m_positions.size()); }

private:
    std::vector<int>     m_cumulative_events; // cumulative: m_cumulative_events[i] = total events through record i
    std::vector<int64_t> m_positions;         // file position of each record
    std::vector<int>     m_record_bytes;      // byte size of each record (for single-pread)
    int m_current_event        = -1;
    int m_current_record       = 0;
    int m_current_record_event = -1;
};

/**
 * @brief Sequential reader for HIPO files.
 *
 * Provides forward iteration over events, random access by event number,
 * bank extraction into zero-copy views, and optional per-column statistics
 * queries.
 *
 * @note NOT thread-safe. The underlying file_source uses POSIX pread which
 *       is OS-level thread-safe, but the reader's internal state (index
 *       cursor, loaded record) is not synchronised. For parallel access
 *       use the chain class or create one reader per thread.
 *
 * @see chain, bank_view_set
 */
class reader {
public:
    reader() = default;
    ~reader() = default;

    reader(reader&&) = default;
    reader& operator=(reader&&) = default;

    /**
     * @brief Open a HIPO file and return a fully initialised reader.
     *
     * Reads the file header, scans the record index, and parses the
     * dictionary from the first record's user header.
     *
     * @param path Filesystem path to the HIPO file.
     * @param opts Reader options (validation, tag filtering).
     * @return A result containing the reader on success, or an error.
     */
    static result<reader> open(std::string_view path, reader_options opts = {});

    /**
     * @brief Perform a lightweight scan of the file to obtain event and record counts.
     *
     * Reads only the file header and trailer index without decompressing
     * data records or parsing the dictionary.
     *
     * @param path Filesystem path to the HIPO file.
     * @param opts Reader options (validation, tag filtering).
     * @return A result containing file_metadata on success, or an error.
     */
    static result<file_metadata> scan_metadata(std::string_view path, reader_options opts = {});

    /** @brief Return a const reference to the file information struct. */
    const file_info&  info() const noexcept { return m_info; }

    /** @brief Return a const reference to the schema dictionary. */
    const dictionary& dict() const noexcept { return m_dict; }

    /**
     * @brief Advance to the next event.
     * @return True if an event is available, false when all events are exhausted.
     */
    bool next();

    /**
     * @brief Check whether more events remain without advancing.
     * @return True if next() would succeed.
     */
    bool has_next() const noexcept;

    /** @brief Reset the read position to the beginning of the file. */
    void rewind();

    /**
     * @brief Jump to a specific event by its global event number.
     * @param event_number Zero-based event index.
     * @return result<void> indicating success or an out-of-range error.
     */
    result<void> goto_event(int event_number);

    /**
     * @brief Skip forward by a number of events.
     * @param n Number of events to skip.
     * @return result<void> indicating success or an error if the skip goes past the end.
     */
    result<void> skip(int n);

    /**
     * @brief Fill a bank_view_set from the current event.
     *
     * Provides zero-copy views into the decompressed record buffer.
     *
     * @param banks The bank_view_set to populate; must have been bound to a dictionary.
     */
    void read_bank(bank_view_set& banks);

    /** @brief Return the current global event number (zero-based). */
    int current_event() const noexcept { return m_index.current_event(); }

    /** @brief Return the total number of events in the file. */
    int total_events() const noexcept { return m_index.max_events(); }

    /** @brief Return the number of data records in the file. */
    int num_records() const noexcept { return m_index.num_records(); }

    /**
     * @brief Check whether per-column statistics are available.
     * @return True if the file was written with collect_statistics=true.
     */
    bool has_column_stats() const noexcept { return !m_column_stats.empty(); }

    /**
     * @brief Return the per-record column statistics vector.
     * @return A const reference to the vector of record_column_stats.
     */
    const std::vector<record_column_stats>& column_stats() const noexcept { return m_column_stats; }

    /**
     * @brief Query whether a record might contain values in a given range for a bank column.
     *
     * Uses per-column min/max statistics to prune records that cannot
     * contain matching values. Returns true (assumes yes) when statistics
     * are unavailable for the column or record.
     *
     * @param record_index Zero-based record index.
     * @param group        Bank group number.
     * @param item         Bank item number.
     * @param col_idx      Column index within the bank schema.
     * @param lo           Lower bound of the query range (inclusive).
     * @param hi           Upper bound of the query range (inclusive).
     * @return True if the record may contain matching values, false if it definitely does not.
     */
    bool record_may_contain(int record_index, int group, int item, int col_idx,
                            double lo, double hi) const noexcept {
        if (record_index < 0 || record_index >= static_cast<int>(m_column_stats.size()))
            return true;  // no stats => assume yes
        auto* entry = m_column_stats[record_index].find(group, item, col_idx);
        if (!entry) return true;  // column not tracked => assume yes
        return entry->may_contain(lo, hi);
    }

    /**
     * @brief Range adapter that skips events not matching a predicate.
     *
     * Returned by event_loop::filter(). Provides input iterators suitable
     * for range-based for loops.
     *
     * @tparam Pred Callable with signature compatible with bool(const bank_view_set&).
     *
     * @see event_loop::filter
     */
    template <typename Pred>
    struct filtered_event_loop {
        /** @brief Input iterator over matching events. */
        struct iterator {
            using iterator_category = std::input_iterator_tag;
            using value_type = reader;
            using difference_type = std::ptrdiff_t;
            using reference = reader&;
            using pointer = reader*;

            iterator() noexcept = default;

            /** @brief Advance to the next event that satisfies the predicate. */
            iterator& operator++() {
                advance_to_next_match();
                return *this;
            }

            reader& operator*()  const noexcept { return *m_reader; }
            reader* operator->() const noexcept { return m_reader; }

            bool operator==(const iterator& o) const noexcept { return m_at_end == o.m_at_end; }
            bool operator!=(const iterator& o) const noexcept { return m_at_end != o.m_at_end; }

        private:
            friend struct filtered_event_loop;
            iterator(reader* r, bank_view_set* b, Pred* p, bool at_end) noexcept
                : m_reader(r), m_banks(b), m_pred(p), m_at_end(at_end) {}

            void advance_to_next_match() {
                while (m_reader->next()) {
                    m_reader->read_bank(*m_banks);
                    if ((*m_pred)(*m_banks)) return;
                }
                m_at_end = true;
            }

            reader*        m_reader = nullptr;
            bank_view_set* m_banks  = nullptr;
            Pred*          m_pred   = nullptr;
            bool           m_at_end = true;
        };

        /** @brief Return an iterator positioned at the first matching event (rewinds the reader). */
        iterator begin() {
            m_reader->rewind();
            iterator it(m_reader, m_banks, &m_pred, false);
            it.advance_to_next_match();
            return it;
        }

        /** @brief Return the past-the-end sentinel iterator. */
        iterator end() { return iterator(); }

    private:
        template <typename> friend struct filtered_event_loop;
        friend struct event_loop;
        filtered_event_loop(reader* r, bank_view_set* b, Pred pred) noexcept
            : m_reader(r), m_banks(b), m_pred(std::move(pred)) {}
        reader*        m_reader;
        bank_view_set* m_banks;
        Pred           m_pred;
    };

    /**
     * @brief Range adapter for iterating over all events with automatic bank reading.
     *
     * Usage:
     * @code
     * for (auto& event : rdr.loop(banks)) { ... }
     * @endcode
     *
     * Automatically calls read_bank() on each iteration so the bank_view_set
     * is populated before the loop body executes.
     *
     * @see reader::loop, filtered_event_loop
     */
    struct event_loop {
        /** @brief Input iterator over events. */
        struct iterator {
            using iterator_category = std::input_iterator_tag;
            using value_type = reader;
            using difference_type = std::ptrdiff_t;
            using reference = reader&;
            using pointer = reader*;

            iterator() noexcept = default;

            /** @brief Advance to the next event and read its banks. */
            iterator& operator++() {
                if (!m_reader || !m_reader->next()) {
                    m_at_end = true;
                } else {
                    m_reader->read_bank(*m_banks);
                }
                return *this;
            }

            reader& operator*()  const noexcept { return *m_reader; }
            reader* operator->() const noexcept { return m_reader; }

            bool operator==(const iterator& o) const noexcept { return m_at_end == o.m_at_end; }
            bool operator!=(const iterator& o) const noexcept { return m_at_end != o.m_at_end; }

        private:
            friend struct event_loop;
            iterator(reader* r, bank_view_set* b, bool at_end) noexcept
                : m_reader(r), m_banks(b), m_at_end(at_end) {}
            reader*        m_reader = nullptr;
            bank_view_set* m_banks  = nullptr;
            bool           m_at_end = true;
        };

        /** @brief Return an iterator positioned at the first event (rewinds the reader). */
        iterator begin() {
            m_reader->rewind();
            if (!m_reader->next()) return iterator();
            m_reader->read_bank(*m_banks);
            return iterator(m_reader, m_banks, false);
        }

        /** @brief Return the past-the-end sentinel iterator. */
        iterator end() { return iterator(); }

        /**
         * @brief Chain a filter predicate to produce a filtered_event_loop.
         *
         * Usage:
         * @code
         * for (auto& ev : rdr.loop(banks).filter([&](auto& b) { return b["X"].rows() > 0; })) { ... }
         * @endcode
         *
         * @tparam Pred Callable invocable with `const bank_view_set&`.
         * @param pred  Predicate returning true for events to keep.
         * @return A filtered_event_loop adapter.
         */
        template <typename Pred>
            requires std::invocable<Pred, const bank_view_set&>
        filtered_event_loop<std::decay_t<Pred>> filter(Pred&& pred) {
            return filtered_event_loop<std::decay_t<Pred>>(m_reader, m_banks, std::forward<Pred>(pred));
        }

    private:
        friend class reader;
        event_loop(reader* r, bank_view_set& b) noexcept : m_reader(r), m_banks(&b) {}
        reader*        m_reader;
        bank_view_set* m_banks;
    };

    /**
     * @brief Create an event_loop range adapter for range-based for iteration.
     * @param banks A bank_view_set bound to the reader's dictionary.
     * @return An event_loop that iterates over all events, calling read_bank() automatically.
     */
    event_loop loop(bank_view_set& banks) { return event_loop(this, banks); }

    /**
     * @brief Load a specific record by index into the provided record object.
     *
     * Used by the chain class for parallel record-level access.
     *
     * @param rec          Record object to populate.
     * @param record_index Zero-based record index.
     * @return result<void> indicating success or an I/O error.
     */
    result<void> load_record(record& rec, int record_index);

    /**
     * @brief Return the file byte offset of a record.
     * @param record_index Zero-based record index.
     * @return File position in bytes.
     */
    int64_t record_position(int record_index) const noexcept { return m_index.record_position(record_index); }

    /** @brief Return the total file size in bytes. */
    int64_t file_size() const noexcept { return m_info.file_size; }

private:
    result<void> read_file_header();
    result<void> scan_index();
    result<void> force_scan();
    result<file_metadata> scan_counts();
    result<void> read_dictionary();
    result<void> ensure_record_loaded();
    void parse_trailer_stats(std::span<const char> trailer_event);

    file_info        m_info;
    dictionary       m_dict;
    file_event_index m_index;
    reader_options   m_opts;

    // I/O
    file_source      m_file_source;

    // Current record
    record           m_record;
    int              m_loaded_record = -1;

    // File header parsed fields
    int     m_header_length_words = 0;
    int     m_index_array_length  = 0;
    int     m_user_header_length  = 0;
    int     m_bit_info            = 0;
    int64_t m_first_record_pos    = 0;
    bool    m_byte_swapped        = false;

    // Tags converted to set for O(1) lookup (populated from m_opts.tags)
    std::unordered_set<long> m_tag_set;

    // Per-column statistics (parsed from trailer if present)
    std::vector<record_column_stats> m_column_stats;
};

} // namespace hipo
