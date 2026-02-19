#pragma once

#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

#include "constants.h"
#include "dictionary.h"
#include "schema.h"

namespace hipo {

/**
 * @brief Min/max statistics for a single column within a single record.
 *
 * Tracks the minimum and maximum values seen in a particular bank column
 * across all events in a record. Used for record-level predicate pushdown:
 * if a query filter falls entirely outside [min_val, max_val], the entire
 * record can be skipped without decompressing event data.
 */
struct column_stats_entry {
    double min_val = std::numeric_limits<double>::infinity();  /**< @brief Minimum value seen in this column. */
    double max_val = -std::numeric_limits<double>::infinity(); /**< @brief Maximum value seen in this column. */

    /**
     * @brief Updates the min/max range with a new observed value.
     * @param v The value to incorporate into the statistics.
     */
    void update(double v) noexcept {
        if (v < min_val) min_val = v;
        if (v > max_val) max_val = v;
    }

    /**
     * @brief Tests whether a query range could overlap with this column's value range.
     *
     * Returns true if the interval [lo, hi] intersects [min_val, max_val].
     * Used for predicate pushdown to skip records that cannot contain matching rows.
     *
     * @param lo Lower bound of the query range (inclusive).
     * @param hi Upper bound of the query range (inclusive).
     * @return true if a value in [lo, hi] could exist in this column.
     */
    bool may_contain(double lo, double hi) const noexcept {
        return min_val <= hi && max_val >= lo;
    }
};

/**
 * @brief Per-record column statistics mapping (group, item, column_index) to min/max stats.
 *
 * Stores column_stats_entry values keyed by a packed 64-bit integer that encodes
 * the bank group, item, and column index. This allows efficient lookup of column
 * statistics for predicate pushdown during record scanning.
 */
struct record_column_stats {
    /** @brief Map from packed key to column statistics entry. */
    std::unordered_map<int64_t, column_stats_entry> columns;

    /**
     * @brief Packs a (group, item, col_idx) triple into a single 64-bit lookup key.
     * @param group The bank group identifier.
     * @param item The bank item identifier.
     * @param col_idx The zero-based column index within the bank schema.
     * @return A unique 64-bit key for this (group, item, column) combination.
     */
    static int64_t pack_key(int group, int item, int col_idx) noexcept {
        return (static_cast<int64_t>(group) << 24) | (static_cast<int64_t>(item) << 16) | col_idx;
    }

    /**
     * @brief Updates the statistics for a specific column with a new value.
     * @param group The bank group identifier.
     * @param item The bank item identifier.
     * @param col_idx The zero-based column index.
     * @param value The observed value to incorporate.
     */
    void update(int group, int item, int col_idx, double value) noexcept {
        columns[pack_key(group, item, col_idx)].update(value);
    }

    /**
     * @brief Looks up the statistics entry for a specific column.
     * @param group The bank group identifier.
     * @param item The bank item identifier.
     * @param col_idx The zero-based column index.
     * @return Pointer to the column_stats_entry if found, or nullptr if no
     *         statistics have been recorded for this column.
     */
    const column_stats_entry* find(int group, int item, int col_idx) const noexcept {
        auto it = columns.find(pack_key(group, item, col_idx));
        return it != columns.end() ? &it->second : nullptr;
    }
};

/**
 * @brief Tracks per-column statistics while building records.
 *
 * Parses raw event bytes using the dictionary to locate bank structures,
 * then iterates over each column to update min/max statistics. After all
 * events for a record have been processed, call finish_record() to
 * retrieve the accumulated statistics and reset for the next record.
 *
 * @note Requires a valid dictionary to be set via set_dictionary() before
 *       processing events. If no dictionary is set, process_event() is a no-op.
 */
class column_stats_tracker {
public:
    /** @brief Constructs a tracker with no dictionary set. */
    column_stats_tracker() = default;

    /**
     * @brief Sets the dictionary used to interpret bank structures in events.
     *
     * The dictionary must remain valid for the lifetime of processing calls.
     *
     * @param dict Reference to the dictionary containing bank schemas.
     */
    void set_dictionary(const dictionary& dict) { m_dict = &dict; }

    /**
     * @brief Processes one event's raw bytes and updates current record statistics.
     *
     * Parses the event structure to find bank headers, looks up each bank's
     * schema in the dictionary, and iterates over all columns to update
     * the min/max statistics for the current record.
     *
     * @param event_data The raw event bytes (including the 16-byte event header).
     */
    void process_event(std::span<const char> event_data) {
        if (!m_dict) return;

        // Event layout: [EVNT:4][total_size:4][tag:4][reserved:4] then structures
        const char* buf = event_data.data();
        int total = static_cast<int>(event_data.size());
        int pos = EVENT_HEADER_SIZE;

        while (pos + BANK_STRUCTURE_SIZE <= total) {
            int group = read_value<uint16_t>(buf + pos);
            int item  = read_value<uint8_t>(buf + pos + 2);
            int type  = read_value<uint8_t>(buf + pos + 3);
            auto size_word = read_value<int32_t>(buf + pos + 4);
            int data_length = size_word & 0x00FFFFFF;
            int header_size = (size_word >> 24) & 0xFF;
            int struct_total = BANK_STRUCTURE_SIZE + data_length;

            if (pos + struct_total > total) break;

            // Only process type 11 (composite bank) structures
            if (type == 11) {
                process_bank(buf + pos + BANK_STRUCTURE_SIZE + header_size,
                             data_length - header_size, group, item);
            }

            pos += struct_total;
        }
    }

    /**
     * @brief Retrieves accumulated statistics for the current record and resets.
     *
     * Returns the record_column_stats collected since the last reset or
     * finish_record() call, then clears the internal state for the next record.
     *
     * @return The column statistics for the completed record.
     */
    record_column_stats finish_record() {
        record_column_stats result = std::move(m_current);
        m_current = {};
        return result;
    }

    /**
     * @brief Resets accumulated statistics without returning them.
     *
     * Discards any statistics collected for the current record.
     */
    void reset() { m_current = {}; }

private:
    /**
     * @brief Processes a single bank's column data and updates statistics.
     * @param data Pointer to the bank's row data (after the bank header).
     * @param data_len Length of the row data in bytes.
     * @param group The bank group identifier.
     * @param item The bank item identifier.
     */
    void process_bank(const char* data, int data_len, int group, int item) {
        // Find schema for this (group, item)
        const schema* sch = find_schema(group, item);
        if (!sch) return;

        int row_len = sch->row_length();
        if (row_len <= 0) return;
        int nrows = data_len / row_len;
        if (nrows <= 0) return;

        // Iterate columns and update stats
        for (int c = 0; c < sch->entries(); ++c) {
            auto ct = sch->entry_type(c);
            int elem_size = type_size(ct);
            // Column data starts at: nrows * entry.offset
            // Each row element is at: column_start + row * elem_size
            int col_start = nrows * sch->entry(c).offset;

            for (int r = 0; r < nrows; ++r) {
                int off = col_start + r * elem_size;
                if (off + elem_size > data_len) break;
                double v = read_as_double(data + off, ct);
                m_current.update(group, item, c, v);
            }
        }
    }

    /**
     * @brief Looks up a schema by group and item identifiers in the dictionary.
     * @param group The bank group identifier.
     * @param item The bank item identifier.
     * @return Pointer to the matching schema, or nullptr if not found.
     */
    const schema* find_schema(int group, int item) const {
        // Simple linear scan through dictionary (cached in practice)
        for (auto const& s : *m_dict) {
            if (s.group() == group && s.item() == item)
                return &s;
        }
        return nullptr;
    }

    /**
     * @brief Reads a typed column value from raw bytes and converts to double.
     * @param ptr Pointer to the raw column value.
     * @param ct The column type indicating how to interpret the bytes.
     * @return The value converted to double.
     */
    static double read_as_double(const char* ptr, column_type ct) noexcept {
        switch (ct) {
        case column_type::byte_t:   return static_cast<double>(read_value<int8_t>(ptr));
        case column_type::short_t:  return static_cast<double>(read_value<int16_t>(ptr));
        case column_type::int_t:    return static_cast<double>(read_value<int32_t>(ptr));
        case column_type::float_t:  return static_cast<double>(read_value<float>(ptr));
        case column_type::double_t: return read_value<double>(ptr);
        case column_type::long_t:   return static_cast<double>(read_value<int64_t>(ptr));
        }
        return 0.0;
    }

    const dictionary* m_dict = nullptr;
    record_column_stats m_current;
};

/** @brief Schema group identifier for column statistics records in the file trailer. */
inline constexpr int STATS_GROUP = 32112;

/** @brief Schema item identifier for column statistics records in the file trailer. */
inline constexpr int STATS_ITEM  = 1;

} // namespace hipo
