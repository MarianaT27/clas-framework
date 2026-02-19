#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "constants.h"
#include "result.h"

namespace hipo {

/**
 * @brief Metadata for a single column in a schema.
 *
 * Each entry describes one column's name, data type, the size of that type
 * in bytes, and its byte offset within a single row of the column-major
 * bank layout.
 *
 * @see schema for the class that holds a collection of schema_entry objects
 * @see column_type (defined in constants.h) for the supported data types
 */
struct schema_entry {
    /** @brief Column name (e.g. "pid", "px"). */
    std::string name;
    /** @brief Data type of the column (e.g. FLOAT, SHORT, INT). */
    column_type type;
    /** @brief Size of the data type in bytes (e.g. 4 for FLOAT, 2 for SHORT). */
    int         type_size;
    /** @brief Byte offset of this column within a single row. */
    int         offset;     // byte offset within a single row
};

/**
 * @brief Cached column index handle for efficient repeated column access.
 *
 * A column_handle is a lightweight wrapper around an integer column index.
 * Resolve a column name to a handle once via schema::get_handle(), then
 * use the handle for repeated accesses without paying the cost of a
 * name lookup each time.
 *
 * A default-constructed handle is invalid (index == -1). Validity is tied
 * to the schema that created the handle -- using a handle from one schema
 * with a different schema is undefined behavior.
 *
 * @note This class is trivially copyable and costs only sizeof(int).
 * @see schema::get_handle()
 * @see schema::get_handles() for batch resolution
 */
class column_handle {
public:
    /** @brief Default-construct an invalid handle. */
    constexpr column_handle() noexcept = default;

    /** @brief Check whether this handle was successfully resolved. */
    constexpr bool valid() const noexcept { return m_index >= 0; }

    /** @brief Return the underlying column index. */
    constexpr int  index() const noexcept { return m_index; }
private:
    friend class schema;
    constexpr explicit column_handle(int idx) noexcept : m_index(idx) {}

    /** @brief The column index, or -1 if invalid. */
    int m_index = -1;
};

/**
 * @brief Defines the structure (columns, types, layout) of a HIPO bank.
 *
 * A schema describes the column-major layout of a bank: each column has a
 * name, a data type, and a byte offset within a row. Schemas are identified
 * by a (group, item) pair and a human-readable name (e.g. "event::particle").
 *
 * Schemas are typically parsed from a definition string of the form
 * "name/type,name/type,..." where type is a single-character type code
 * (S=short, I=int, F=float, D=double, L=long, B=byte).
 *
 * Column lookups by name use a sorted lookup table for O(log N) binary
 * search. For hot-path access, resolve names to column_handle objects once
 * and reuse them.
 *
 * Example:
 * @code
 *   auto s = schema::parse("event::particle", 100, 1, "pid/S,px/F,py/F,pz/F")
 *                .expect("parse particle schema");
 *   auto [h_pid, h_px] = s.get_handles("pid", "px").value();
 *   int offset = s.offset(h_pid.index(), row, nrows);
 * @endcode
 *
 * @see schema_entry for individual column metadata
 * @see column_handle for cached column indices
 * @see dictionary for a collection of schemas
 */
class schema {
public:
    /** @brief Default-construct an empty schema. */
    schema() = default;

    /**
     * @brief Parse a schema definition string into a schema object.
     *
     * The columns string uses the format "name/type,name/type,..." where
     * type is a single character: S (short), I (int), F (float), D (double),
     * L (long), B (byte).
     *
     * @param name   Schema name (e.g. "event::particle").
     * @param group  Group number for the (group, item) identifier.
     * @param item   Item number for the (group, item) identifier.
     * @param columns Column definitions in "name/type,..." format.
     * @return The parsed schema on success, or an error_code on failure.
     */
    static result<schema> parse(std::string_view name, int group, int item,
                                std::string_view columns);

    /** @brief Return the schema name (e.g. "event::particle"). */
    std::string_view name()  const noexcept { return m_name; }

    /** @brief Return the group number of this schema. */
    int              group() const noexcept { return m_group; }

    /** @brief Return the item number of this schema. */
    int              item()  const noexcept { return m_item; }

    /** @brief Return the number of columns in this schema. */
    int              entries() const noexcept { return static_cast<int>(m_entries.size()); }

    /**
     * @brief Return the byte length of a single row.
     *
     * Computed from the last column's offset plus its type size.
     *
     * @return Row length in bytes, or 0 if the schema is empty.
     */
    int row_length() const noexcept {
        if (m_entries.empty()) return 0;
        auto const& last = m_entries.back();
        return last.offset + last.type_size;
    }

    /**
     * @brief Return the total byte size needed to store the given number of rows.
     * @param rows Number of rows.
     * @return rows * row_length().
     */
    int size_for_rows(int rows) const noexcept {
        return rows * row_length();
    }

    /**
     * @brief Look up a column index by name.
     * @param col_name The column name to search for.
     * @return The zero-based column index, or error_code::schema_column_not_found.
     */
    result<int> column_index(std::string_view col_name) const noexcept;

    /**
     * @brief Resolve a column name to a cached column_handle.
     * @param col_name The column name to resolve.
     * @return A valid column_handle, or error_code::schema_column_not_found.
     * @see column_handle
     */
    result<column_handle> get_handle(std::string_view col_name) const noexcept;

    /**
     * @brief Check whether this schema contains a column with the given name.
     * @param col_name The column name to check.
     * @return True if the column exists, false otherwise.
     */
    bool        contains(std::string_view col_name) const noexcept;

    /**
     * @brief Batch-resolve multiple column names to handles.
     *
     * Resolves all column names in the array and returns them as a
     * fixed-size array of handles. Returns an error on the first column
     * name that is not found.
     *
     * @tparam N Number of column names to resolve.
     * @param col_names Array of column name string_views.
     * @return Array of N column_handle objects, or the error from the first failed lookup.
     */
    template <std::size_t N>
    result<std::array<column_handle, N>> get_handles(
            const std::array<std::string_view, N>& col_names) const noexcept {
        std::array<column_handle, N> handles;
        for (std::size_t i = 0; i < N; ++i) {
            auto h = get_handle(col_names[i]);
            if (!h) return unexpected(h.error());
            handles[i] = *h;
        }
        return handles;
    }

    /**
     * @brief Batch-resolve column names to handles (variadic convenience overload).
     *
     * Example:
     * @code
     *   auto [h_pid, h_px, h_py] = schema.get_handles("pid", "px", "py").value();
     * @endcode
     *
     * @tparam Names String-like types convertible to std::string_view.
     * @param names Column names to resolve.
     * @return Array of column_handle objects, or the error from the first failed lookup.
     */
    template <typename... Names>
        requires (sizeof...(Names) > 0 && (std::convertible_to<Names, std::string_view> && ...))
    result<std::array<column_handle, sizeof...(Names)>> get_handles(Names&&... names) const noexcept {
        return get_handles<sizeof...(Names)>(
            {std::string_view(std::forward<Names>(names))...});
    }

    /**
     * @brief Access a column's schema_entry by index.
     * @param index Zero-based column index.
     * @return Const reference to the schema_entry.
     * @note No bounds checking. Passing an out-of-range index is undefined behavior.
     */
    const schema_entry& entry(int index) const noexcept { return m_entries[index]; }

    /**
     * @brief Return a read-only span over all column entries.
     * @return A span of schema_entry covering every column in the schema.
     */
    std::span<const schema_entry> columns() const noexcept {
        return {m_entries.data(), m_entries.size()};
    }

    /**
     * @brief Compute the byte offset for a cell in the column-major bank layout.
     *
     * In HIPO's column-major layout, all values for column 0 come first,
     * then all values for column 1, etc. The offset for cell (col, row) with
     * nrows total rows is: nrows * column_offset + row * type_size.
     *
     * @param col   Zero-based column index.
     * @param row   Zero-based row index.
     * @param nrows Total number of rows in the bank.
     * @return Byte offset from the start of the bank data buffer.
     */
    int offset(int col, int row, int nrows) const noexcept {
        assert(col >= 0 && col < static_cast<int>(m_entries.size()));
        auto const& e = m_entries[col];
        return nrows * e.offset + row * e.type_size;
    }

    /**
     * @brief Return the data type of a column by index.
     * @param col Zero-based column index.
     * @return The column_type of the specified column.
     */
    column_type entry_type(int col) const noexcept { return m_entries[col].type; }

    /**
     * @brief Validate the structural integrity of this schema.
     *
     * Checks the following invariants:
     * - The schema has at least one column (not empty).
     * - No two columns share the same name.
     * - No two columns have overlapping byte offsets.
     * - Each column's type_size matches its declared column_type.
     *
     * @return ok() on success, or the first error_code encountered.
     */
    result<void> validate() const noexcept;

    /**
     * @brief Serialize this schema back into the "name/type,..." definition string.
     * @return The schema definition string suitable for parse().
     */
    std::string to_schema_string() const;

    /**
     * @brief Three-way comparison by (group, item).
     *
     * Schemas are ordered first by group number, then by item number.
     */
    auto operator<=>(const schema& other) const noexcept {
        if (auto cmp = m_group <=> other.m_group; cmp != 0) return cmp;
        return m_item <=> other.m_item;
    }

    /**
     * @brief Equality comparison by (group, item).
     *
     * Two schemas are considered equal if they share the same group and item
     * numbers, regardless of name or column definitions.
     */
    bool operator==(const schema& other) const noexcept {
        return m_group == other.m_group && m_item == other.m_item;
    }

private:
    schema(std::string name, int group, int item,
           std::vector<schema_entry> entries,
           std::vector<std::pair<std::string, int>> lookup)
        : m_name(std::move(name)), m_group(group), m_item(item),
          m_entries(std::move(entries)), m_lookup(std::move(lookup)) {}

    /** @brief Human-readable schema name (e.g. "event::particle"). */
    std::string                            m_name;
    /** @brief Group number of the (group, item) identifier. */
    int                                    m_group = 0;
    /** @brief Item number of the (group, item) identifier. */
    int                                    m_item  = 0;
    /** @brief Ordered list of column definitions. */
    std::vector<schema_entry>              m_entries;
    /** @brief Sorted (name, index) pairs for O(log N) column lookup by name. */
    std::vector<std::pair<std::string,int>> m_lookup; // sorted by name for binary search
};

} // namespace hipo
