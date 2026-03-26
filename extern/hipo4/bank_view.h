#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstring>
#include <numeric>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <fmt/format.h>

#include "constants.h"
#include "dictionary.h"
#include "record.h"
#include "result.h"
#include "schema.h"

namespace hipo {

class row_filter;  // forward declaration for bank_view::filter()

/**
 * @brief Read-only, zero-copy view into a bank's column data within a record buffer.
 *
 * A bank_view provides typed access to column-major data stored in the HIPO binary
 * format without copying or owning the underlying memory. It supports single-cell
 * access, contiguous column spans (suitable for SIMD), type-dispatch visitors,
 * aggregates, predicates, filtering, sorting, and multi-column tuple iteration.
 *
 * The view is constructed from a schema pointer, a raw data span, and a row count.
 * All column offsets are computed on the fly from the schema.
 *
 * @note Thread safety: a bank_view is a non-owning read-only view into record
 *       memory. Safe for concurrent reads as long as the underlying record buffer
 *       is not being modified.
 *
 * @see bank       Mutable, owning counterpart that can create a bank_view via bank::view().
 * @see row_filter Zero-copy filtered row indices produced by bank_view::filter() and bank_view::sort_by().
 * @see bank_view_set Pre-configured collection of bank views for per-event batch updates.
 */
class bank_view {
public:
    /** @brief Default-constructs an invalid (empty) bank_view. */
    bank_view() = default;

    /**
     * @brief Construct a bank_view over existing record data.
     *
     * @param sch  Pointer to the schema describing the bank's column layout.
     * @param data Raw byte span covering all column data for this bank.
     * @param rows Number of rows (entries) in the bank.
     */
    bank_view(const schema* sch, std::span<const char> data, int rows) noexcept
        : m_schema(sch), m_data(data), m_rows(rows) {}

    /** @brief Return the raw column data span for this bank. */
    std::span<const char> data() const noexcept { return m_data; }

    /** @brief Return true if this view has a valid schema and non-negative row count. */
    bool valid() const noexcept { return m_schema != nullptr && m_rows >= 0; }
    /** @brief Return the number of rows in this bank. */
    int  rows()  const noexcept { return m_rows; }
    /** @brief Return a reference to the schema describing this bank's layout. */
    const schema& get_schema() const noexcept { return *m_schema; }

    /**
     * @brief Read a single typed value from the bank by column index and row.
     *
     * No bounds checking in release builds; asserts on out-of-range in debug builds.
     *
     * @tparam T Value type satisfying the BankValue concept (int8_t, int16_t, int32_t,
     *           int64_t, float, or double).
     * @param column Zero-based column index.
     * @param row    Zero-based row index.
     * @return The value at the specified cell, reinterpreted as type T.
     */
    template <BankValue T>
    T get(int column, int row) const noexcept {
        assert(column >= 0 && column < m_schema->entries());
        assert(row >= 0 && row < m_rows);
        auto offset = m_schema->offset(column, row, m_rows);
        return read_value<T>(m_data.data() + offset);
    }

    /**
     * @brief Read a single typed value using a pre-resolved column_handle.
     *
     * This overload avoids repeated name-to-index lookups by using a cached
     * column_handle, providing zero-overhead access after the initial resolve.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle.
     * @param row Zero-based row index.
     * @return The value at the specified cell.
     */
    template <BankValue T>
    T get(column_handle col, int row) const noexcept {
        assert(col.valid());
        return get<T>(col.index(), row);
    }

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
     * @brief Return a contiguous span over an entire column using a column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle.
     * @return A span of `rows()` elements of type T.
     */
    template <BankValue T>
    std::span<const T> column(column_handle col) const noexcept {
        assert(col.valid());
        return column<T>(col.index());
    }

    /**
     * @brief Return a contiguous span over an entire column, looked up by name.
     *
     * This is a convenience overload that performs an index lookup on each call.
     * For hot loops, prefer resolving a column_handle once and using the handle overload.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param name Column name as defined in the schema.
     * @return A span of `rows()` elements, or an empty span if the name is not found.
     */
    template <BankValue T>
    std::span<const T> column(std::string_view name) const noexcept {
        auto idx = m_schema->column_index(name);
        if (!idx) return {};
        return column<T>(*idx);
    }

    /**
     * @brief Return a contiguous span over an entire column by column index.
     *
     * The returned span points directly into the record buffer with no copies,
     * making it suitable for SIMD-friendly vectorized access.
     *
     * @tparam T Value type satisfying the BankValue concept. Must match the
     *           column's actual type size; mismatches cause undefined behavior.
     * @param col_index Zero-based column index.
     * @return A span of `rows()` elements of type T.
     */
    template <BankValue T>
    std::span<const T> column(int col_index) const noexcept {
        assert(col_index >= 0 && col_index < m_schema->entries());
        auto offset = m_schema->offset(col_index, 0, m_rows);
        auto ptr = reinterpret_cast<const T*>(m_data.data() + offset);
        return std::span<const T>(ptr, m_rows);
    }

    /**
     * @brief Type-dispatch visitor over a column's data.
     *
     * Calls the visitor with a `std::span<const T>` where T is the column's
     * runtime type. The visitor must be a generic callable accepting any span type
     * (e.g., a lambda with an `auto` parameter).
     *
     * @tparam F Callable type. Must be invocable with std::span<const T> for all
     *           BankValue types T.
     * @param col_index Zero-based column index.
     * @param visitor   Generic callable to invoke with the typed column span.
     * @return The value returned by the visitor.
     */
    template <typename F>
    decltype(auto) visit_column(int col_index, F&& visitor) const {
        assert(col_index >= 0 && col_index < m_schema->entries());
        switch (m_schema->entry_type(col_index)) {
        case column_type::byte_t:   return visitor(column<int8_t>(col_index));
        case column_type::short_t:  return visitor(column<int16_t>(col_index));
        case column_type::int_t:    return visitor(column<int32_t>(col_index));
        case column_type::float_t:  return visitor(column<float>(col_index));
        case column_type::double_t: return visitor(column<double>(col_index));
        case column_type::long_t:   return visitor(column<int64_t>(col_index));
        }
        __builtin_unreachable();
    }

    /**
     * @brief Type-dispatch visitor over a column's data, using a column_handle.
     *
     * @tparam F Callable type, as described in the column-index overload.
     * @param col     Pre-resolved column handle.
     * @param visitor Generic callable to invoke with the typed column span.
     * @return The value returned by the visitor.
     */
    template <typename F>
    decltype(auto) visit_column(column_handle col, F&& visitor) const {
        assert(col.valid());
        return visit_column(col.index(), std::forward<F>(visitor));
    }

    /**
     * @brief Bounds-checked typed access returning a result.
     *
     * Unlike get(), this method validates column and row indices at runtime
     * and returns an error code instead of asserting on out-of-range access.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param column Zero-based column index.
     * @param row    Zero-based row index.
     * @return result<T> containing the value on success, or an error_code
     *         (bank_column_out_of_range or bank_row_out_of_range) on failure.
     */
    template <BankValue T>
    result<T> get_checked(int column, int row) const noexcept {
        if (column < 0 || column >= m_schema->entries())
            return unexpected(error_code::bank_column_out_of_range);
        if (row < 0 || row >= m_rows)
            return unexpected(error_code::bank_row_out_of_range);
        return get<T>(column, row);
    }

    /**
     * @brief Type-erased numeric access returning any column value as double.
     *
     * Dispatches on the column's runtime type and converts the result to double.
     * Useful for generic processing where the column type is not known at compile time.
     *
     * @param column Zero-based column index.
     * @param row    Zero-based row index.
     * @return The cell value converted to double.
     */
    double get_numeric(int column, int row) const noexcept {
        assert(column >= 0 && column < m_schema->entries());
        assert(row >= 0 && row < m_rows);
        auto offset = m_schema->offset(column, row, m_rows);
        switch (m_schema->entry_type(column)) {
        case column_type::byte_t:   return static_cast<double>(read_value<int8_t>(m_data.data() + offset));
        case column_type::short_t:  return static_cast<double>(read_value<int16_t>(m_data.data() + offset));
        case column_type::int_t:    return static_cast<double>(read_value<int32_t>(m_data.data() + offset));
        case column_type::float_t:  return static_cast<double>(read_value<float>(m_data.data() + offset));
        case column_type::double_t: return read_value<double>(m_data.data() + offset);
        case column_type::long_t:   return static_cast<double>(read_value<int64_t>(m_data.data() + offset));
        }
        return 0.0;
    }

    /**
     * @brief Type-erased numeric access using a column_handle.
     *
     * @param col Pre-resolved column handle.
     * @param row Zero-based row index.
     * @return The cell value converted to double.
     */
    double get_numeric(column_handle col, int row) const noexcept {
        assert(col.valid());
        return get_numeric(col.index(), row);
    }

    /**
     * @brief Bulk-convert an entire column to doubles.
     *
     * Performs a single type dispatch and then iterates all rows, writing the
     * converted values into the caller-provided output buffer. More efficient
     * than calling get_numeric() per row.
     *
     * @param col Zero-based column index.
     * @param out Pointer to a caller-allocated array of at least `rows()` doubles.
     */
    void column_to_double(int col, double* out) const noexcept {
        assert(col >= 0 && col < m_schema->entries());
        auto offset = m_schema->offset(col, 0, m_rows);
        const char* base = m_data.data() + offset;
        switch (m_schema->entry_type(col)) {
        case column_type::byte_t:
            for (int r = 0; r < m_rows; ++r)
                out[r] = static_cast<double>(read_value<int8_t>(base + r * sizeof(int8_t)));
            break;
        case column_type::short_t:
            for (int r = 0; r < m_rows; ++r)
                out[r] = static_cast<double>(read_value<int16_t>(base + r * sizeof(int16_t)));
            break;
        case column_type::int_t:
            for (int r = 0; r < m_rows; ++r)
                out[r] = static_cast<double>(read_value<int32_t>(base + r * sizeof(int32_t)));
            break;
        case column_type::float_t:
            for (int r = 0; r < m_rows; ++r)
                out[r] = static_cast<double>(read_value<float>(base + r * sizeof(float)));
            break;
        case column_type::double_t:
            for (int r = 0; r < m_rows; ++r)
                out[r] = read_value<double>(base + r * sizeof(double));
            break;
        case column_type::long_t:
            for (int r = 0; r < m_rows; ++r)
                out[r] = static_cast<double>(read_value<int64_t>(base + r * sizeof(int64_t)));
            break;
        }
    }

    /**
     * @brief Compute the sum of all values in a column.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Zero-based column index.
     * @return The sum of all row values, starting from a zero-initialized T.
     */
    template <BankValue T>
    T sum(int col) const noexcept {
        auto span = column<T>(col);
        return std::accumulate(span.begin(), span.end(), T{});
    }

    /**
     * @brief Compute the sum of all values in a column using a column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle.
     * @return The sum of all row values.
     */
    template <BankValue T>
    T sum(column_handle col) const noexcept { return sum<T>(col.index()); }

    /**
     * @brief Find the minimum value in a column.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Zero-based column index.
     * @return The minimum value, or a zero-initialized T if the column is empty.
     */
    template <BankValue T>
    T min(int col) const noexcept {
        auto span = column<T>(col);
        if (span.empty()) return T{};
        return *std::min_element(span.begin(), span.end());
    }

    /**
     * @brief Find the minimum value in a column using a column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle.
     * @return The minimum value.
     */
    template <BankValue T>
    T min(column_handle col) const noexcept { return min<T>(col.index()); }

    /**
     * @brief Find the maximum value in a column.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Zero-based column index.
     * @return The maximum value, or a zero-initialized T if the column is empty.
     */
    template <BankValue T>
    T max(int col) const noexcept {
        auto span = column<T>(col);
        if (span.empty()) return T{};
        return *std::max_element(span.begin(), span.end());
    }

    /**
     * @brief Find the maximum value in a column using a column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle.
     * @return The maximum value.
     */
    template <BankValue T>
    T max(column_handle col) const noexcept { return max<T>(col.index()); }

    /**
     * @brief Count the number of rows matching a predicate.
     *
     * @tparam Pred Callable type invocable as `bool(const bank_view&, int row)`.
     * @param pred Predicate applied to each row. Receives a reference to this
     *             bank_view and the current row index.
     * @return The number of rows for which the predicate returned true.
     */
    template <typename Pred>
        requires std::invocable<Pred, const bank_view&, int>
    int count(Pred&& pred) const noexcept {
        int n = 0;
        for (int r = 0; r < m_rows; ++r)
            if (pred(*this, r)) ++n;
        return n;
    }

    /**
     * @brief Find the first row matching a predicate.
     *
     * @tparam Pred Callable type invocable as `bool(const bank_view&, int row)`.
     * @param pred Predicate applied to each row.
     * @return The zero-based index of the first matching row, or -1 if no row matches.
     */
    template <typename Pred>
        requires std::invocable<Pred, const bank_view&, int>
    int find_first(Pred&& pred) const noexcept {
        for (int r = 0; r < m_rows; ++r)
            if (pred(*this, r)) return r;
        return -1;
    }

    /**
     * @brief Extract a column's data into an owning vector.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Zero-based column index.
     * @return A std::vector<T> containing a copy of the column's data.
     */
    template <BankValue T>
    std::vector<T> to_vector(int col) const {
        auto span = column<T>(col);
        return std::vector<T>(span.begin(), span.end());
    }

    /**
     * @brief Extract a column's data into an owning vector using a column_handle.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle.
     * @return A std::vector<T> containing a copy of the column's data.
     */
    template <BankValue T>
    std::vector<T> to_vector(column_handle col) const { return to_vector<T>(col.index()); }

    /**
     * @brief Extract multiple column values from a single row as a tuple.
     *
     * @tparam Ts Value types satisfying the BankValue concept, one per column.
     * @param cols Array of column handles, one per type in Ts.
     * @param row  Zero-based row index.
     * @return A std::tuple<Ts...> containing the values from the specified columns.
     */
    template <BankValue... Ts>
    std::tuple<Ts...> row_as(std::array<column_handle, sizeof...(Ts)> cols, int row) const noexcept {
        return row_as_impl<Ts...>(cols, row, std::index_sequence_for<Ts...>{});
    }

    /**
     * @brief Ranges-compatible multi-column row iterator.
     *
     * Provides begin()/end() iterators that yield std::tuple<Ts...> for each row,
     * enabling structured bindings in range-based for loops.
     *
     * Usage:
     * @code
     * for (auto [pid, px, py] : bv.rows_as<int16_t, float, float>({h_pid, h_px, h_py})) {
     *     // process each row
     * }
     * @endcode
     *
     * @tparam Ts Value types satisfying the BankValue concept, one per column.
     */
    template <BankValue... Ts>
    class rows_view {
    public:
        /**
         * @brief Construct a rows_view over the given bank_view and column handles.
         *
         * @param bv      The bank_view to iterate over.
         * @param handles Array of column handles, one per type in Ts.
         */
        rows_view(const bank_view& bv, std::array<column_handle, sizeof...(Ts)> handles)
            : m_bank(&bv), m_handles(handles) {}

        /** @brief Input iterator that dereferences to std::tuple<Ts...>. */
        struct iterator {
            using iterator_category = std::input_iterator_tag;
            using value_type = std::tuple<Ts...>;
            using difference_type = std::ptrdiff_t;

            /** @brief Dereference to extract the current row as a tuple. */
            value_type operator*() const {
                return m_view->m_bank->row_as<Ts...>(m_view->m_handles, m_row);
            }
            /** @brief Advance to the next row (pre-increment). */
            iterator& operator++() { ++m_row; return *this; }
            /** @brief Advance to the next row (post-increment). */
            iterator  operator++(int) { auto tmp = *this; ++m_row; return tmp; }
            /** @brief Equality comparison. */
            bool operator==(const iterator& o) const noexcept { return m_row == o.m_row; }
            /** @brief Inequality comparison. */
            bool operator!=(const iterator& o) const noexcept { return m_row != o.m_row; }

        private:
            friend class rows_view;
            iterator(const rows_view* v, int row) : m_view(v), m_row(row) {}
            const rows_view* m_view;
            int m_row;
        };

        /** @brief Return an iterator to the first row. */
        iterator begin() const { return {this, 0}; }
        /** @brief Return a past-the-end iterator. */
        iterator end()   const { return {this, m_bank->rows()}; }
        /** @brief Return the number of rows. */
        int      size()  const noexcept { return m_bank->rows(); }

    private:
        const bank_view* m_bank;
        std::array<column_handle, sizeof...(Ts)> m_handles;
    };

    /**
     * @brief Create a rows_view for range-based multi-column iteration.
     *
     * @tparam Ts Value types satisfying the BankValue concept, one per column.
     * @param handles Array of column handles, one per type in Ts.
     * @return A rows_view<Ts...> iterable with range-based for loops.
     */
    template <BankValue... Ts>
    rows_view<Ts...> rows_as(std::array<column_handle, sizeof...(Ts)> handles) const {
        return rows_view<Ts...>(*this, handles);
    }

    /**
     * @brief Apply a function across multiple columns, producing a per-row result vector.
     *
     * For each row, reads one value from each specified column and passes them
     * all to the function. The function's return value is cast to R and stored.
     *
     * Usage:
     * @code
     * auto pt = bv.apply<float, float, float>(std::hypot, {h_px, h_py});
     * @endcode
     *
     * @tparam R       Return type for each row's computed value.
     * @tparam Ts      Value types satisfying the BankValue concept, one per input column.
     * @tparam F       Callable type invocable as `R(Ts...)`.
     * @param func     Function to apply to each row's column values.
     * @param handles  Array of column handles, one per type in Ts.
     * @return A std::vector<R> of size `rows()` with the computed results.
     */
    template <typename R, BankValue... Ts, typename F>
    std::vector<R> apply(F&& func, std::array<column_handle, sizeof...(Ts)> handles) const {
        std::vector<R> out(m_rows);
        for (int r = 0; r < m_rows; ++r)
            out[r] = static_cast<R>(apply_impl<Ts...>(std::forward<F>(func), handles, r,
                                    std::index_sequence_for<Ts...>{}));
        return out;
    }

    /**
     * @brief Filter rows by a predicate, returning a row_filter with accepted row indices.
     *
     * @tparam Pred Callable type invocable as `bool(const bank_view&, int row)`.
     * @param pred Predicate applied to each row.
     * @return A row_filter containing the indices of rows for which the predicate returned true.
     *
     * @see row_filter
     */
    template <typename Pred>
        requires std::invocable<Pred, const bank_view&, int>
    row_filter filter(Pred&& pred) const;

    /**
     * @brief Return row indices sorted by a column's values in ascending order.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param column Zero-based column index to sort by.
     * @return A row_filter with row indices in sorted order.
     *
     * @see row_filter
     */
    template <BankValue T>
    row_filter sort_by(int column) const;

    /**
     * @brief Return row indices sorted by a column's values in ascending order.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col Pre-resolved column handle to sort by.
     * @return A row_filter with row indices in sorted order.
     */
    template <BankValue T>
    row_filter sort_by(column_handle col) const;

    /**
     * @brief Print the bank contents to stdout in a formatted table.
     *
     * Displays column names as the header row, followed by all data rows
     * with type-appropriate formatting (integers as %d, floats as %.6f).
     */
    void show() const;

private:
    template <BankValue... Ts, std::size_t... Is>
    std::tuple<Ts...> row_as_impl(std::array<column_handle, sizeof...(Ts)> const& cols,
                                   int row, std::index_sequence<Is...>) const noexcept {
        return std::tuple<Ts...>{get<Ts>(cols[Is], row)...};
    }

    template <BankValue... Ts, typename F, std::size_t... Is>
    auto apply_impl(F&& func, std::array<column_handle, sizeof...(Ts)> const& cols,
                    int row, std::index_sequence<Is...>) const {
        return func(get<Ts>(cols[Is], row)...);
    }

private:
    const schema*        m_schema = nullptr;
    std::span<const char> m_data;
    int                  m_rows = 0;
};

/**
 * @brief Zero-copy row filter storing accepted row indices into a bank_view.
 *
 * A row_filter is produced by bank_view::filter() or bank_view::sort_by(). It
 * stores a vector of original row indices that passed the filter predicate (or
 * that represent the sorted order). Data is accessed through the original
 * bank_view via read-through accessors -- no data is copied.
 *
 * Supports iteration over accepted row indices, making it usable in
 * range-based for loops.
 *
 * Usage:
 * @code
 * auto pos = particles.filter([&](auto& bv, int r) {
 *     return bv.template get<int32_t>(h_charge, r) > 0;
 * });
 * for (int r : pos) {
 *     // r is an original row index of a positively-charged particle
 * }
 * @endcode
 *
 * @see bank_view::filter()
 * @see bank_view::sort_by()
 */
class row_filter {
public:
    /** @brief Default-constructs an empty row_filter with no accepted rows. */
    row_filter() = default;

    /**
     * @brief Create a row_filter by applying a predicate to every row of a bank_view.
     *
     * @tparam Pred Callable type invocable as `bool(const bank_view&, int row)`.
     * @param bv   The bank_view to filter.
     * @param pred Predicate applied to each row. Rows for which the predicate
     *             returns true are included in the filter.
     * @return A row_filter containing the accepted row indices.
     */
    template <typename Pred>
        requires std::invocable<Pred, const bank_view&, int>
    static row_filter create(const bank_view& bv, Pred&& pred) {
        row_filter rf;
        rf.m_bank = &bv;
        rf.m_rows.reserve(bv.rows());
        for (int r = 0; r < bv.rows(); ++r) {
            if (pred(bv, r))
                rf.m_rows.push_back(r);
        }
        return rf;
    }

    /** @brief Return the number of accepted rows. */
    int  size()  const noexcept { return static_cast<int>(m_rows.size()); }
    /** @brief Return true if no rows were accepted. */
    bool empty() const noexcept { return m_rows.empty(); }

    /**
     * @brief Return the original row index for the i-th accepted row.
     *
     * @param i Zero-based index into the filtered row list.
     * @return The original row index in the underlying bank_view.
     */
    int row(int i) const noexcept { return m_rows[i]; }

    /**
     * @brief Read a typed value from the underlying bank_view at a filtered row position.
     *
     * @tparam T Value type satisfying the BankValue concept.
     * @param col          Pre-resolved column handle.
     * @param filtered_row Zero-based index into the filtered row list (not the original row index).
     * @return The value at the specified column and original row.
     */
    template <BankValue T>
    T get(column_handle col, int filtered_row) const noexcept {
        return m_bank->get<T>(col, m_rows[filtered_row]);
    }

    /** @brief Return an iterator to the first accepted row index. */
    auto begin() const noexcept { return m_rows.begin(); }
    /** @brief Return a past-the-end iterator over accepted row indices. */
    auto end()   const noexcept { return m_rows.end(); }

private:
    friend class bank_view;  // for sort_by()
    const bank_view*  m_bank = nullptr;
    std::vector<int>  m_rows;
};

// bank_view::filter() convenience -- defined here since row_filter must be complete.
template <typename Pred>
    requires std::invocable<Pred, const bank_view&, int>
inline row_filter bank_view::filter(Pred&& pred) const {
    return row_filter::create(*this, std::forward<Pred>(pred));
}

// bank_view::sort_by() -- defined here since row_filter must be complete.
template <BankValue T>
inline row_filter bank_view::sort_by(int col) const {
    row_filter rf;
    rf.m_bank = this;
    rf.m_rows.resize(m_rows);
    std::iota(rf.m_rows.begin(), rf.m_rows.end(), 0);
    auto span = column<T>(col);
    std::sort(rf.m_rows.begin(), rf.m_rows.end(),
              [&span](int a, int b) { return span[a] < span[b]; });
    return rf;
}

template <BankValue T>
inline row_filter bank_view::sort_by(column_handle col) const {
    return sort_by<T>(col.index());
}

/**
 * @brief Pre-configured collection of bank views, registered once and updated per event.
 *
 * A bank_view_set allows you to declare a set of required bank names, bind them
 * to a dictionary (resolving schemas), and then efficiently update all views
 * from a record in a single pass per event. This avoids repeated per-bank
 * lookups during event processing.
 *
 * Usage:
 * @code
 * auto banks = hipo::bank_view_set::create(dict, {"event::particle", "event::detector"}).value();
 * banks.update_from_record(rec, event_index);
 * auto& particles = banks["event::particle"];
 * @endcode
 *
 * @note Thread safety: NOT thread-safe. Owns mutable scratch buffers reused
 *       across update_from_record() calls. Use one bank_view_set per thread.
 *
 * @see bank_view
 * @see dictionary
 */
class bank_view_set {
public:
    /** @brief Default-constructs an empty bank_view_set with no required banks. */
    bank_view_set() = default;

    /**
     * @brief Create a fully-bound bank_view_set from a dictionary and bank names.
     *
     * This static factory method requires the specified bank names, binds them
     * against the dictionary (resolving schemas), and returns the ready-to-use set.
     *
     * @param dict       The dictionary containing schema definitions.
     * @param bank_names List of bank names to include in the set.
     * @return result<bank_view_set> containing the configured set on success,
     *         or error_code::schema_not_found if any bank name is not in the dictionary.
     */
    static result<bank_view_set> create(const dictionary& dict,
                                        std::initializer_list<std::string_view> bank_names) {
        bank_view_set bs;
        for (auto name : bank_names)
            bs.require(name);
        auto res = bs.bind(dict);
        if (!res) return unexpected(res.error());
        return bs;
    }

    /**
     * @brief Register a bank name to be included in this set.
     *
     * Must be called before bind(). Names are stored in registration order,
     * which determines the integer index for operator[](size_t).
     *
     * @param bank_name The schema name of the bank to require.
     */
    void require(std::string_view bank_name) {
        m_required.emplace_back(std::string(bank_name));
    }

    /**
     * @brief Bind all required bank names to schemas in the given dictionary.
     *
     * Resolves each required bank name to its schema. Must be called after
     * all require() calls and before update_from_record().
     *
     * @param dict The dictionary containing schema definitions.
     * @return result<void> indicating success, or error_code::schema_not_found
     *         if any required bank name is not in the dictionary.
     */
    result<void> bind(const dictionary& dict) {
        m_schemas.clear();
        m_views.resize(m_required.size());
        m_schemas.reserve(m_required.size());

        for (size_t i = 0; i < m_required.size(); ++i) {
            auto sc = dict.find(m_required[i]);
            if (!sc)
                return unexpected(error_code::schema_not_found);
            m_schemas.push_back(&sc->get());
        }
        return ok();
    }

    /**
     * @brief Update all bank views from a record at a given event index.
     *
     * Performs a single scan of the event data to locate all required banks,
     * then constructs a bank_view for each. Banks not found in the event
     * receive an empty view (zero rows).
     *
     * @param rec         The record containing the event data.
     * @param event_index Zero-based index of the event within the record.
     * @return result<void> indicating success, or an error if event data
     *         could not be retrieved.
     */
    result<void> update_from_record(const record& rec, int event_index) {
        size_t n = m_required.size();

        // Get event data once (single call)
        auto ev = rec.event_data(event_index);
        if (!ev) return unexpected(ev.error());

        // Cache the full raw event bytes (all banks, not just required ones)
        m_raw_event.assign(ev->begin(), ev->end());
        const char* ev_buf = m_raw_event.data();

        // Build (group, item) pairs for batch lookup (reuses scratch buffers)
        m_group_items.resize(n);
        m_positions.resize(n);
        for (size_t i = 0; i < n; ++i)
            m_group_items[i] = {m_schemas[i]->group(), m_schemas[i]->item()};

        // Single scan of event data for all banks (using pre-computed span)
        record::find_structures_in_event(*ev, m_group_items, m_positions);

        // Distribute results to views
        for (size_t i = 0; i < n; ++i) {
            if (m_positions[i].found()) {
                auto* sch = m_schemas[i];
                const char* str = ev_buf + m_positions[i].offset;
                // Structure layout: [group:2][item:1][type:1][size_word:4][data...]
                auto size_word = read_value<int32_t>(str + 4);
                int data_length = size_word & 0x00FFFFFF;
                int header_size = (size_word >> 24) & 0xFF;
                int actual_data = data_length - header_size;
                int row_length = sch->row_length();
                int nrows = (row_length > 0) ? (actual_data / row_length) : 0;
                auto data = std::span<const char>(str + 8 + header_size, actual_data);
                m_views[i] = bank_view(sch, data, nrows);
            } else {
                m_views[i] = bank_view(m_schemas[i], {}, 0);
            }
        }
        return ok();
    }

    /**
     * @brief Access a bank_view by its registration index.
     *
     * @param index Zero-based index in registration order.
     * @return A const reference to the bank_view at that index.
     */
    const bank_view& operator[](size_t index) const noexcept {
        return m_views[index];
    }

    /**
     * @brief Access a bank_view by name (unchecked).
     *
     * The bank_name must have been previously passed to require(). In debug
     * builds, asserts on unknown names. In release builds, returns an empty
     * bank_view for unknown names.
     *
     * @param name The bank name to look up.
     * @return A const reference to the matching bank_view.
     *
     * @see get() For checked access returning a result.
     */
    const bank_view& operator[](std::string_view name) const noexcept {
        for (size_t i = 0; i < m_required.size(); ++i) {
            if (m_required[i] == name)
                return m_views[i];
        }
        assert(false && "bank name not found in bank_view_set");
        static const bank_view empty;
        return empty;
    }

    /**
     * @brief Access a bank_view by name with error checking.
     *
     * @param name The bank name to look up.
     * @return result containing a reference to the bank_view on success,
     *         or error_code::schema_not_found if the name was not registered.
     */
    result<std::reference_wrapper<const bank_view>> get(std::string_view name) const noexcept {
        for (size_t i = 0; i < m_required.size(); ++i) {
            if (m_required[i] == name)
                return std::cref(m_views[i]);
        }
        return unexpected(error_code::schema_not_found);
    }

    /** @brief Return the number of registered banks. */
    size_t size() const noexcept { return m_required.size(); }

    /**
     * @brief Return the raw event bytes from the last update_from_record() call.
     *
     * Contains ALL banks (not just the required ones). Valid until the next
     * update_from_record() call. Useful for writing complete events to output files.
     *
     * @return Span over the cached raw event data, or empty if not available.
     */
    std::span<const char> raw_event_data() const noexcept { return m_raw_event; }

private:
    std::vector<std::string>              m_required;
    std::vector<const schema*>            m_schemas;
    std::vector<bank_view>                m_views;
    std::vector<std::pair<int,int>>       m_group_items;  // scratch: (group, item) pairs
    std::vector<structure_position>       m_positions;    // scratch: batch lookup results
    std::vector<char>                     m_raw_event;    // cached raw event bytes
};

// --- Inline implementation ---

inline void bank_view::show() const {
    if (!m_schema) return;
    auto const& sch = *m_schema;
    // Print header
    for (int c = 0; c < sch.entries(); ++c) {
        auto const& e = sch.entry(c);
        fmt::print("{:>14s} ", e.name);
    }
    fmt::print("\n");
    // Print rows
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < sch.entries(); ++c) {
            auto const& e = sch.entry(c);
            switch (e.type) {
            case column_type::byte_t:   fmt::print("{:>14d} ", get<int8_t>(c, r)); break;
            case column_type::short_t:  fmt::print("{:>14d} ", get<int16_t>(c, r)); break;
            case column_type::int_t:    fmt::print("{:>14d} ", get<int32_t>(c, r)); break;
            case column_type::float_t:  fmt::print("{:>14.6f} ", get<float>(c, r)); break;
            case column_type::double_t: fmt::print("{:>14.6f} ", get<double>(c, r)); break;
            case column_type::long_t:   fmt::print("{:>14d} ", get<int64_t>(c, r)); break;
            }
        }
        fmt::print("\n");
    }
}

} // namespace hipo
