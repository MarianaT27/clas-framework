#pragma once

#include <cassert>
#include <cstring>
#include <span>
#include <string_view>
#include <vector>

#include <fmt/format.h>

#include "constants.h"
#include "bank_view.h"
#include "result.h"
#include "schema.h"

namespace hipo {

/**
 * @brief Mutable, owning bank with column-major layout matching the HIPO binary format.
 *
 * A bank owns its data buffer and provides typed read/write access to individual
 * cells, bulk column writes, and column fills. The internal layout is column-major:
 * all values for column 0 are stored contiguously, followed by all values for
 * column 1, and so on. This matches the HIPO on-disk format and enables efficient
 * serialization without reordering.
 *
 * A bank can be converted to a read-only bank_view via the view() method, which
 * provides additional query operations (aggregates, filtering, sorting, etc.)
 * without copying data.
 *
 * @note Thread safety: NOT thread-safe. Use one bank per thread.
 *
 * @see bank_view Read-only, zero-copy view providing query operations over bank data.
 * @see schema    Describes the column layout (names, types, offsets) of a bank.
 */
class bank {
public:
    /** @brief Default-constructs an invalid (empty) bank with no schema. */
    bank() = default;

    /**
     * @brief Construct a bank with the given schema and initial row count.
     *
     * Allocates a zero-initialized data buffer sized for the specified number
     * of rows according to the schema's row length.
     *
     * @param sch  The schema describing this bank's column layout. Must outlive the bank.
     * @param rows Initial number of rows to allocate.
     */
    bank(const schema& sch, int rows)
        : m_schema(&sch), m_rows(rows),
          m_data(static_cast<size_t>(sch.size_for_rows(rows)), '\0') {}

    /** @brief Return true if this bank has a valid schema and non-negative row count. */
    bool valid() const noexcept { return m_schema != nullptr && m_rows >= 0; }
    /** @brief Return the number of rows in this bank. */
    int  rows()  const noexcept { return m_rows; }
    /** @brief Return a reference to the schema describing this bank's layout. */
    const schema& get_schema() const noexcept { return *m_schema; }

    /**
     * @brief Write a typed value into the bank at the specified column and row.
     *
     * No bounds checking in release builds; asserts on out-of-range in debug builds.
     *
     * @tparam T Value type satisfying the BankValue concept (int8_t, int16_t, int32_t,
     *           int64_t, float, or double).
     * @param column Zero-based column index.
     * @param row    Zero-based row index.
     * @param value  The value to write.
     */
    template <BankValue T>
    void put(int column, int row, T value) noexcept {
        assert(column >= 0 && column < m_schema->entries());
        assert(row >= 0 && row < m_rows);
        auto off = m_schema->offset(column, row, m_rows);
        write_value<T>(m_data.data() + off, value);
    }

    /**
     * @brief Write a typed value using a pre-resolved column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col   Pre-resolved column handle.
     * @param row   Zero-based row index.
     * @param value The value to write.
     */
    template <BankValue T>
    void put(column_handle col, int row, T value) noexcept { put<T>(col.index(), row, value); }

    /**
     * @brief Write an entire column of values from a span, by column index.
     *
     * The span must contain exactly `rows()` elements, and the element size
     * must match the column's type size. Performs a single memcpy for maximum
     * throughput.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param column Zero-based column index.
     * @param values Span of exactly `rows()` values to write.
     */
    template <BankValue T>
    void put_column(int column, std::span<const T> values) noexcept {
        assert(column >= 0 && column < m_schema->entries());
        assert(static_cast<int>(sizeof(T)) == m_schema->entry(column).type_size);
        assert(static_cast<int>(values.size()) == m_rows);
        auto off = m_schema->offset(column, 0, m_rows);
        std::memcpy(m_data.data() + off, values.data(), values.size_bytes());
    }

    /**
     * @brief Write an entire column of values using a column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col    Pre-resolved column handle.
     * @param values Span of exactly `rows()` values to write.
     */
    template <BankValue T>
    void put_column(column_handle col, std::span<const T> values) noexcept {
        put_column<T>(col.index(), values);
    }

    /**
     * @brief Write an entire column of values, looked up by name.
     *
     * This is a convenience overload that performs a name-to-index lookup.
     * If the column name is not found, the call is silently ignored.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col    Column name as defined in the schema.
     * @param values Span of exactly `rows()` values to write.
     */
    template <BankValue T>
    void put_column(std::string_view col, std::span<const T> values) noexcept {
        auto idx = m_schema->column_index(col);
        if (idx) put_column<T>(*idx, values);
    }

    /**
     * @brief Fill an entire column with the same value, by column index.
     *
     * Writes the given value to every row of the specified column.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param column Zero-based column index.
     * @param value  The value to write to every row.
     */
    template <BankValue T>
    void fill_column(int column, T value) noexcept {
        assert(column >= 0 && column < m_schema->entries());
        assert(static_cast<int>(sizeof(T)) == m_schema->entry(column).type_size);
        auto off = m_schema->offset(column, 0, m_rows);
        auto* ptr = m_data.data() + off;
        for (int r = 0; r < m_rows; ++r)
            write_value<T>(ptr + r * static_cast<int>(sizeof(T)), value);
    }

    /**
     * @brief Fill an entire column with the same value using a column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col   Pre-resolved column handle.
     * @param value The value to write to every row.
     */
    template <BankValue T>
    void fill_column(column_handle col, T value) noexcept {
        fill_column<T>(col.index(), value);
    }

    /**
     * @brief Fill an entire column with the same value, looked up by name.
     *
     * If the column name is not found, the call is silently ignored.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col   Column name as defined in the schema.
     * @param value The value to write to every row.
     */
    template <BankValue T>
    void fill_column(std::string_view col, T value) noexcept {
        auto idx = m_schema->column_index(col);
        if (idx) fill_column<T>(*idx, value);
    }

    /**
     * @brief Read a typed value from the bank by column index and row.
     *
     * Provides the same read interface as bank_view::get() for API compatibility.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param column Zero-based column index.
     * @param row    Zero-based row index.
     * @return The value at the specified cell.
     */
    template <BankValue T>
    T get(int column, int row) const noexcept {
        assert(column >= 0 && column < m_schema->entries());
        assert(row >= 0 && row < m_rows);
        auto off = m_schema->offset(column, row, m_rows);
        return read_value<T>(m_data.data() + off);
    }

    /**
     * @brief Read a typed value using a pre-resolved column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle.
     * @param row Zero-based row index.
     * @return The value at the specified cell.
     */
    template <BankValue T>
    T get(column_handle col, int row) const noexcept { return get<T>(col.index(), row); }

    /**
     * @brief Read a typed value by column name (convenience overload).
     *
     * Performs a name-to-index lookup on each call. For hot loops, prefer
     * resolving a column_handle once and using the handle-based overload.
     */
    template <BankValue T>
    T get(std::string_view col, int row) const noexcept {
        auto idx = m_schema->column_index(col);
        assert(idx && "column name not found");
        return get<T>(*idx, row);
    }

    /**
     * @brief Resize the bank to the specified number of rows.
     *
     * Reallocates the internal data buffer to fit the new row count. Existing
     * data is preserved for rows that still exist; new rows are zero-initialized.
     *
     * @param rows The new row count.
     */
    void set_rows(int rows) {
        m_rows = rows;
        m_data.resize(static_cast<size_t>(m_schema->size_for_rows(rows)), '\0');
    }

    /**
     * @brief Reset the bank to zero rows and release the data buffer.
     *
     * After calling reset(), the bank is empty but retains its schema reference.
     */
    void reset() {
        m_rows = 0;
        m_data.clear();
    }

    /** @brief Return a read-only span over the raw data buffer for serialization. */
    std::span<const char> data() const noexcept { return {m_data.data(), static_cast<size_t>(data_size())}; }
    /** @brief Return the size in bytes of the raw data buffer. */
    int data_size() const noexcept { return m_schema ? m_schema->size_for_rows(m_rows) : 0; }

    /**
     * @brief Create a read-only bank_view over this bank's data.
     *
     * The returned view references this bank's internal buffer and schema.
     * It remains valid as long as this bank is not modified or destroyed.
     *
     * @return A bank_view providing read-only access to this bank's data.
     */
    bank_view view() const noexcept {
        return bank_view(m_schema, {m_data.data(), static_cast<size_t>(data_size())}, m_rows);
    }

    /**
     * @brief Print the bank contents to stdout in a formatted table.
     *
     * Delegates to bank_view::show() via a temporary view.
     */
    void show() const { view().show(); }

private:
    const schema*     m_schema = nullptr;
    int               m_rows = 0;
    std::vector<char> m_data;
};

} // namespace hipo
