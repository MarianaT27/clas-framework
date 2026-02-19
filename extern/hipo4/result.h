#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <fmt/format.h>

namespace hipo {

/**
 * @brief Categorized error codes for all HIPO operations.
 *
 * Error codes are grouped by subsystem in numeric ranges:
 * - 100s: I/O errors (file operations)
 * - 200s: Format validation errors (magic number, header, endianness)
 * - 300s: Record errors (read, decompress, index)
 * - 400s: Event errors (index, structure lookup, size)
 * - 500s: Schema and dictionary errors (parse, lookup, type mismatch)
 * - 600s: Bank errors (column/row bounds, type, binding)
 * - 700s: Chain errors (empty, schema mismatch, open)
 * - 800s: Writer errors (write, compression, state)
 *
 * @see error_message() for human-readable descriptions
 * @see hipo_error for the exception type that wraps an error_code
 * @see result for the primary error-propagation mechanism
 */
enum class error_code : int32_t {
    // I/O errors (100s)

    /** @brief The specified file does not exist */
    file_not_found           = 100,
    /** @brief Failed to open the file (permissions, etc.) */
    file_open_failed         = 101,
    /** @brief A read operation on the file failed */
    file_read_failed         = 102,
    /** @brief A seek operation on the file failed */
    file_seek_failed         = 103,
    /** @brief File is too small to contain a valid HIPO header */
    file_too_small           = 104,

    // Format validation errors (200s)

    /** @brief File does not begin with the expected HIPO magic number */
    invalid_magic_number     = 200,
    /** @brief Header length field has an invalid value */
    invalid_header_length    = 201,
    /** @brief Header type bits contain an unrecognized value */
    invalid_header_type      = 202,
    /** @brief File version is not supported */
    invalid_version          = 203,
    /** @brief Endianness marker is incorrect (wrong byte order) */
    invalid_endianness       = 204,
    /** @brief File header is structurally invalid */
    invalid_file_header      = 205,
    /** @brief Trailer record is missing or corrupted */
    invalid_trailer          = 206,
    /** @brief Padding bytes have unexpected values */
    invalid_padding          = 207,

    // Record errors (300s)

    /** @brief Failed to read a record from the file */
    record_read_failed       = 300,
    /** @brief Decompression of record data failed (LZ4/GZIP) */
    record_decompress_failed = 301,
    /** @brief Record index is out of the valid range */
    record_invalid_index     = 302,
    /** @brief Record data exceeds the declared bounds */
    record_bounds_exceeded   = 303,
    /** @brief Compression type is not supported by this build */
    record_unsupported_compression = 304,

    // Event errors (400s)

    /** @brief Requested event index is out of range */
    event_index_out_of_range = 400,
    /** @brief A named structure (bank) was not found in the event */
    event_structure_not_found = 401,
    /** @brief Event size field is invalid or inconsistent */
    event_invalid_size       = 402,

    // Schema/dictionary errors (500s)

    /** @brief Failed to parse a schema definition string */
    schema_parse_failed      = 500,
    /** @brief The requested schema name was not found in the dictionary */
    schema_not_found         = 501,
    /** @brief A column name was not found in the schema */
    schema_column_not_found  = 502,
    /** @brief Column type does not match the requested access type */
    schema_type_mismatch     = 503,
    /** @brief Schema definitions are inconsistent across files in a chain */
    schema_inconsistent      = 504,
    /** @brief Dictionary-level parse failure */
    dictionary_parse_failed  = 505,
    /** @brief Schema has no columns defined */
    schema_empty             = 506,
    /** @brief Two columns in a schema share the same name */
    schema_duplicate_column  = 507,
    /** @brief Two columns in a schema have overlapping byte offsets */
    schema_offset_overlap    = 508,

    // Bank errors (600s)

    /** @brief Column index is beyond the number of columns in the bank */
    bank_column_out_of_range = 600,
    /** @brief Row index is beyond the number of rows in the bank */
    bank_row_out_of_range    = 601,
    /** @brief Attempted to read a column with the wrong data type */
    bank_type_mismatch       = 602,
    /** @brief Bank is not bound to a schema (uninitialized) */
    bank_not_bound           = 603,

    // Chain errors (700s)

    /** @brief Chain was constructed with an empty file list */
    chain_no_files           = 700,
    /** @brief Schemas differ between files in the chain */
    chain_schema_mismatch    = 701,
    /** @brief Failed to open one of the files in the chain */
    chain_open_failed        = 702,

    // Writer errors (800s)

    /** @brief A write operation to the output file failed */
    write_failed             = 800,
    /** @brief Compression of record data failed during write */
    write_compression_failed = 801,
    /** @brief Attempted a write operation before the writer was opened */
    writer_not_open          = 802,
    /** @brief Attempted to open a writer that is already open */
    writer_already_open      = 803,
};

/**
 * @brief Map an error_code to its human-readable description string.
 *
 * Returns a compile-time string_view for each known error code.
 * Returns "unknown error" for any unrecognized value.
 *
 * @param ec The error code to describe.
 * @return A string_view containing the error description (never empty).
 */
constexpr std::string_view error_message(error_code ec) noexcept {
    switch (ec) {
    case error_code::file_not_found:            return "file not found";
    case error_code::file_open_failed:          return "failed to open file";
    case error_code::file_read_failed:          return "file read failed";
    case error_code::file_seek_failed:          return "file seek failed";
    case error_code::file_too_small:            return "file too small for valid header";
    case error_code::invalid_magic_number:      return "invalid magic number";
    case error_code::invalid_header_length:     return "invalid header length";
    case error_code::invalid_header_type:       return "invalid header type";
    case error_code::invalid_version:           return "invalid version";
    case error_code::invalid_endianness:        return "invalid endianness marker";
    case error_code::invalid_file_header:       return "invalid file header";
    case error_code::invalid_trailer:           return "invalid trailer";
    case error_code::invalid_padding:           return "invalid padding";
    case error_code::record_read_failed:        return "record read failed";
    case error_code::record_decompress_failed:  return "record decompression failed";
    case error_code::record_invalid_index:      return "invalid record index";
    case error_code::record_bounds_exceeded:    return "record data exceeds bounds";
    case error_code::record_unsupported_compression: return "unsupported compression type";
    case error_code::event_index_out_of_range:  return "event index out of range";
    case error_code::event_structure_not_found: return "structure not found in event";
    case error_code::event_invalid_size:        return "invalid event size";
    case error_code::schema_parse_failed:       return "schema parse failed";
    case error_code::schema_not_found:          return "schema not found";
    case error_code::schema_column_not_found:   return "column not found in schema";
    case error_code::schema_type_mismatch:      return "column type mismatch";
    case error_code::schema_inconsistent:       return "schema inconsistent across files";
    case error_code::dictionary_parse_failed:   return "dictionary parse failed";
    case error_code::schema_empty:              return "schema has no columns";
    case error_code::schema_duplicate_column:   return "duplicate column name in schema";
    case error_code::schema_offset_overlap:     return "overlapping column offsets in schema";
    case error_code::bank_column_out_of_range:  return "bank column index out of range";
    case error_code::bank_row_out_of_range:     return "bank row index out of range";
    case error_code::bank_type_mismatch:        return "bank type mismatch";
    case error_code::bank_not_bound:            return "bank not bound to schema";
    case error_code::chain_no_files:            return "chain has no files";
    case error_code::chain_schema_mismatch:     return "schema mismatch across chain files";
    case error_code::chain_open_failed:         return "chain open failed";
    case error_code::write_failed:              return "write failed";
    case error_code::write_compression_failed:  return "compression failed during write";
    case error_code::writer_not_open:           return "writer is not open";
    case error_code::writer_already_open:       return "writer is already open";
    }
    return "unknown error";
}

/**
 * @brief Exception type for HIPO errors.
 *
 * Wraps an error_code together with its human-readable message (via
 * std::runtime_error). Use this when converting result-based error handling
 * to exception-based error handling.
 *
 * @see result::value_or_throw() which throws hipo_error on failure
 */
struct hipo_error : std::runtime_error {
    /** @brief The error code that caused this exception. */
    error_code code;

    /**
     * @brief Construct a hipo_error from an error code.
     * @param ec The error code. The what() message is derived from error_message(ec).
     */
    hipo_error(error_code ec)
        : std::runtime_error(std::string(error_message(ec))), code(ec) {}
};

/**
 * @brief Wrapper for an error value, used to construct a result in the error state.
 *
 * Analogous to std::unexpected (C++23). Wrap an error_code in unexpected to
 * pass it to a result constructor, making it unambiguous that the result
 * should hold an error rather than a value.
 *
 * Example:
 * @code
 *   result<int> load_value() {
 *       return unexpected(error_code::file_not_found);
 *   }
 * @endcode
 *
 * @tparam T The error type (typically hipo::error_code).
 * @see result
 */
template <typename T>
class [[nodiscard]] unexpected {
    T m_value;
public:
    /** @brief Construct an unexpected from an error value. */
    constexpr explicit unexpected(T value) noexcept : m_value(std::move(value)) {}

    /** @brief Access the stored error value (const lvalue). */
    constexpr const T& value() const & noexcept { return m_value; }

    /** @brief Access the stored error value (rvalue). */
    constexpr T&& value() && noexcept { return std::move(m_value); }
};

/** @brief Deduction guide: unexpected(error_code) deduces to unexpected<error_code>. */
unexpected(error_code) -> unexpected<error_code>;

namespace detail {

/** @brief Placeholder type used internally to represent void in result<void>. */
struct void_placeholder {};

/**
 * @brief Tagged-union storage for result<T>.
 *
 * Holds either a value of type T or an error_code, tracked by m_has_value.
 * Manages lifetime of the value via placement new / explicit destructor call.
 *
 * @tparam T The value type to store.
 */
template <typename T>
struct result_storage {
    union {
        T m_value;
        error_code m_error;
    };
    bool m_has_value;

    constexpr result_storage(T value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_value(std::move(value)), m_has_value(true) {}

    constexpr result_storage(unexpected<error_code> err) noexcept
        : m_error(err.value()), m_has_value(false) {}

    constexpr ~result_storage() {
        if (m_has_value)
            m_value.~T();
    }

    result_storage(const result_storage& other) : m_has_value(other.m_has_value) {
        if (m_has_value)
            new (&m_value) T(other.m_value);
        else
            m_error = other.m_error;
    }

    result_storage(result_storage&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : m_has_value(other.m_has_value) {
        if (m_has_value)
            new (&m_value) T(std::move(other.m_value));
        else
            m_error = other.m_error;
    }

    result_storage& operator=(const result_storage& other) {
        if (this != &other) {
            if (m_has_value) m_value.~T();
            m_has_value = other.m_has_value;
            if (m_has_value)
                new (&m_value) T(other.m_value);
            else
                m_error = other.m_error;
        }
        return *this;
    }

    result_storage& operator=(result_storage&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (this != &other) {
            if (m_has_value) m_value.~T();
            m_has_value = other.m_has_value;
            if (m_has_value)
                new (&m_value) T(std::move(other.m_value));
            else
                m_error = other.m_error;
        }
        return *this;
    }
};

/**
 * @brief Specialization of result_storage for void results.
 *
 * Uses void_placeholder in place of an actual value. Trivially copyable
 * and movable since the placeholder has no resources to manage.
 */
template <>
struct result_storage<void_placeholder> {
    union {
        void_placeholder m_value;
        error_code m_error;
    };
    bool m_has_value;

    constexpr result_storage() noexcept : m_value{}, m_has_value(true) {}
    constexpr result_storage(void_placeholder) noexcept : m_value{}, m_has_value(true) {}
    constexpr result_storage(unexpected<error_code> err) noexcept : m_error(err.value()), m_has_value(false) {}
    constexpr ~result_storage() = default;
    result_storage(const result_storage&) = default;
    result_storage(result_storage&&) noexcept = default;
    result_storage& operator=(const result_storage&) = default;
    result_storage& operator=(result_storage&&) noexcept = default;
};

} // namespace detail

/**
 * @brief A result type that holds either a value of type T or an error_code.
 *
 * Inspired by std::expected (C++23) and Rust's Result<T, E>. This is the
 * primary error-propagation mechanism in the HIPO library -- functions return
 * result<T> instead of throwing exceptions, allowing callers to handle errors
 * explicitly.
 *
 * A result is contextually convertible to bool: true means it holds a value,
 * false means it holds an error. The class is marked [[nodiscard]] so that
 * callers cannot silently ignore errors.
 *
 * Monadic operations (and_then, map, or_else, map_error) enable functional
 * chaining of operations that may fail.
 *
 * The void specialization (result<void>, aliased as status) represents
 * operations that succeed with no return value or fail with an error_code.
 *
 * Example:
 * @code
 *   result<schema> s = schema::parse("particle", 100, 1, "pid/S,px/F,py/F,pz/F");
 *   if (!s) {
 *       fmt::print("Error: {}\n", s.error());
 *       return;
 *   }
 *   fmt::print("Schema: {}\n", s->name());
 * @endcode
 *
 * @tparam T The value type on success. Use void for operations with no return value.
 *
 * @see unexpected for constructing error results
 * @see status for the result<void> alias
 * @see ok() for constructing a successful status
 * @see hipo_error for converting to exceptions via value_or_throw()
 */
template <typename T>
class [[nodiscard]] result {
    using storage_type = std::conditional_t<std::is_void_v<T>, detail::void_placeholder, T>;
    detail::result_storage<storage_type> m_storage;

public:
    // Value constructors

    /**
     * @brief Construct a result holding a value (non-void specialization).
     * @param value The value to store.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr result(U value) noexcept(std::is_nothrow_move_constructible_v<U>)
        : m_storage(std::move(value)) {}

    /**
     * @brief Construct a successful void result.
     */
    template <typename U = T, std::enable_if_t<std::is_void_v<U>, int> = 0>
    constexpr result() noexcept : m_storage() {}

    /**
     * @brief Construct a result in the error state.
     * @param err The unexpected-wrapped error code.
     */
    constexpr result(unexpected<error_code> err) noexcept : m_storage(err) {}

    /** @brief Contextual conversion to bool. Returns true if holding a value. */
    constexpr explicit operator bool() const noexcept { return m_storage.m_has_value; }

    /** @brief Check whether this result holds a value (true) or an error (false). */
    constexpr bool has_value() const noexcept { return m_storage.m_has_value; }

    /**
     * @brief Access the stored value (const lvalue reference).
     * @return Const reference to the value.
     * @note Undefined behavior if has_value() is false.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr const U& value() const & noexcept { return m_storage.m_value; }

    /**
     * @brief Access the stored value (mutable lvalue reference).
     * @return Mutable reference to the value.
     * @note Undefined behavior if has_value() is false.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr U& value() & noexcept { return m_storage.m_value; }

    /**
     * @brief Move the stored value out (rvalue reference).
     * @return Rvalue reference to the value.
     * @note Undefined behavior if has_value() is false.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr U&& value() && noexcept { return std::move(m_storage.m_value); }

    /**
     * @brief Dereference operator (const). Equivalent to value().
     * @return Const reference to the value.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr const U& operator*() const & noexcept { return m_storage.m_value; }

    /**
     * @brief Dereference operator (mutable). Equivalent to value().
     * @return Mutable reference to the value.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr U& operator*() & noexcept { return m_storage.m_value; }

    /**
     * @brief Arrow operator (const). Provides pointer-like access to the value.
     * @return Const pointer to the value.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr const U* operator->() const noexcept { return &m_storage.m_value; }

    /**
     * @brief Arrow operator (mutable). Provides pointer-like access to the value.
     * @return Mutable pointer to the value.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr U* operator->() noexcept { return &m_storage.m_value; }

    /**
     * @brief Access the stored error code.
     * @return The error_code.
     * @note Undefined behavior if has_value() is true.
     */
    constexpr error_code error() const noexcept { return m_storage.m_error; }

    // Monadic operations

    /**
     * @brief Chain a function that returns a result, forwarding the value.
     *
     * If this result holds a value, calls f with the value and returns the
     * result of f. If this result holds an error, propagates the error
     * without calling f.
     *
     * @tparam F Callable taking const T& (or no args for void) and returning a result type.
     * @param f The function to apply to the value.
     * @return The result of f(value), or this result's error propagated into f's return type.
     */
    template <typename F>
    constexpr auto and_then(F&& f) const & {
        using R = std::invoke_result_t<F, const storage_type&>;
        if (m_storage.m_has_value) {
            if constexpr (std::is_void_v<T>)
                return f();
            else
                return f(m_storage.m_value);
        }
        return R(unexpected(m_storage.m_error));
    }

    /**
     * @brief Transform the value with a function, wrapping the result.
     *
     * If this result holds a value, calls f with the value and wraps the
     * return in a new result. If this result holds an error, propagates it.
     * Unlike and_then(), f returns a plain value rather than a result.
     *
     * @tparam F Callable taking const T& (or no args for void) and returning a plain value.
     * @param f The transformation function.
     * @return result<R> where R is the return type of f, holding f(value) or the propagated error.
     */
    template <typename F>
    constexpr auto map(F&& f) const & {
        if constexpr (std::is_void_v<T>) {
            using R = std::invoke_result_t<F>;
            if (m_storage.m_has_value) return result<R>(f());
            return result<R>(unexpected(m_storage.m_error));
        } else {
            using R = std::invoke_result_t<F, const storage_type&>;
            if (m_storage.m_has_value) return result<R>(f(m_storage.m_value));
            return result<R>(unexpected(m_storage.m_error));
        }
    }

    /**
     * @brief Handle an error by calling a recovery function.
     *
     * If this result holds an error, calls f with the error_code and returns
     * f's result. If this result holds a value, returns *this unchanged.
     *
     * @tparam F Callable taking error_code and returning result<T>.
     * @param f The error-handling function.
     * @return Either *this (on success) or the result of f(error()).
     */
    template <typename F>
    constexpr auto or_else(F&& f) const & {
        if (!m_storage.m_has_value)
            return f(m_storage.m_error);
        return *this;
    }

    /**
     * @brief Transform the error code, preserving the value unchanged.
     *
     * If this result holds a value, returns it unchanged. If it holds an
     * error, applies f to the error_code and wraps the result as a new error.
     *
     * @tparam F Callable taking error_code and returning error_code.
     * @param f The error transformation function.
     * @return result<T> with either the original value or the transformed error.
     */
    template <typename F>
    constexpr auto map_error(F&& f) const & -> result<T> {
        if (m_storage.m_has_value) {
            if constexpr (std::is_void_v<T>)
                return result<T>();
            else
                return result<T>(m_storage.m_value);
        }
        return result<T>(unexpected(f(m_storage.m_error)));
    }

    /**
     * @brief Return the value if present, otherwise return a default.
     * @param default_value The value to return if this result holds an error.
     * @return The stored value, or default_value.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    constexpr U value_or(U default_value) const & noexcept {
        return m_storage.m_has_value ? m_storage.m_value : std::move(default_value);
    }

    /**
     * @brief Return the value or throw hipo_error if this result holds an error.
     *
     * Use this to bridge from result-based to exception-based error handling.
     *
     * @return Mutable reference to the stored value.
     * @throws hipo_error if has_value() is false.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    U& value_or_throw() & {
        if (!m_storage.m_has_value) throw hipo_error(m_storage.m_error);
        return m_storage.m_value;
    }

    /**
     * @brief Return the value (moved) or throw hipo_error if this result holds an error.
     * @return Rvalue reference to the stored value.
     * @throws hipo_error if has_value() is false.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    U&& value_or_throw() && {
        if (!m_storage.m_has_value) throw hipo_error(m_storage.m_error);
        return std::move(m_storage.m_value);
    }

    /**
     * @brief Throw hipo_error if this void result holds an error.
     * @throws hipo_error if has_value() is false.
     */
    template <typename U = T, std::enable_if_t<std::is_void_v<U>, int> = 0>
    void value_or_throw() const {
        if (!m_storage.m_has_value) throw hipo_error(m_storage.m_error);
    }

    /**
     * @brief Return the value or abort with a message if this result holds an error.
     *
     * Rust-style expect(): intended for hard preconditions where an error
     * is a programming bug rather than a recoverable condition. Prints the
     * error message to stderr and calls std::abort().
     *
     * @param msg Context message printed before the error description on failure.
     * @return Mutable reference to the stored value.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    U& expect(const char* msg) & {
        if (!m_storage.m_has_value) {
            auto sv = error_message(m_storage.m_error);
            std::fprintf(stderr, "FATAL: %s: %.*s\n", msg,
                         static_cast<int>(sv.size()), sv.data());
            std::abort();
        }
        return m_storage.m_value;
    }

    /**
     * @brief Return the value (moved) or abort with a message.
     * @param msg Context message printed before the error description on failure.
     * @return Rvalue reference to the stored value.
     * @see expect(const char*) & for details.
     */
    template <typename U = T, std::enable_if_t<!std::is_void_v<U>, int> = 0>
    U&& expect(const char* msg) && {
        if (!m_storage.m_has_value) {
            auto sv = error_message(m_storage.m_error);
            std::fprintf(stderr, "FATAL: %s: %.*s\n", msg,
                         static_cast<int>(sv.size()), sv.data());
            std::abort();
        }
        return std::move(m_storage.m_value);
    }

    /**
     * @brief Abort with a message if this void result holds an error.
     * @param msg Context message printed before the error description on failure.
     * @see expect(const char*) & for details.
     */
    template <typename U = T, std::enable_if_t<std::is_void_v<U>, int> = 0>
    void expect(const char* msg) const {
        if (!m_storage.m_has_value) {
            auto sv = error_message(m_storage.m_error);
            std::fprintf(stderr, "FATAL: %s: %.*s\n", msg,
                         static_cast<int>(sv.size()), sv.data());
            std::abort();
        }
    }
};

/**
 * @brief Alias for result<void>, representing success-or-error with no return value.
 *
 * Use status as the return type for operations that either succeed (with no
 * meaningful value) or fail with an error_code.
 *
 * @see ok() for constructing a successful status
 */
using status = result<void>;

/**
 * @brief Construct a successful status (result<void>).
 * @return A status in the success state.
 */
inline constexpr status ok() noexcept { return status(); }

} // namespace hipo

/**
 * @brief fmt formatter specialization for hipo::error_code.
 *
 * Formats an error_code as its human-readable message string, enabling
 * direct use with fmt::format() and fmt::print().
 */
template <>
struct fmt::formatter<hipo::error_code> : fmt::formatter<std::string_view> {
    auto format(hipo::error_code ec, fmt::format_context& ctx) const {
        return fmt::formatter<std::string_view>::format(hipo::error_message(ec), ctx);
    }
};
