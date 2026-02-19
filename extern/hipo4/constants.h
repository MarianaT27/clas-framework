#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

/**
 * @brief Core namespace for the HIPO I/O library.
 *
 * Provides types, constants, and utilities for reading and writing files
 * in the HIPO (Hall-B I/O Performance Optimized) binary format.
 */
namespace hipo {

/** @brief HIPO file unique identifier word ("HIPO" in little-endian). */
inline constexpr uint32_t HIPO_FILE_UNIQUE_WORD = 0x4F504948; // "HIPO" in LE

/** @brief Endianness marker written into every file and record header. */
inline constexpr uint32_t HEADER_MAGIC          = 0xc0da0100; // endianness marker

/** @brief Number of 32-bit words in a HIPO file header. */
inline constexpr int FILE_HEADER_WORDS   = 14;

/** @brief Number of 32-bit words in a HIPO record header. */
inline constexpr int RECORD_HEADER_WORDS = 14;

/** @brief File header size in bytes (FILE_HEADER_WORDS * 4 = 56). */
inline constexpr int FILE_HEADER_SIZE    = FILE_HEADER_WORDS * 4;     // 56 bytes

/** @brief Record header size in bytes (RECORD_HEADER_WORDS * 4 = 56). */
inline constexpr int RECORD_HEADER_SIZE  = RECORD_HEADER_WORDS * 4;   // 56 bytes

/** @brief Size of the event header in bytes (tag + total size + tag + reserved). */
inline constexpr int EVENT_HEADER_SIZE   = 16;

/** @brief Size of a bank structure header in bytes (group + item + type + size word). */
inline constexpr int BANK_STRUCTURE_SIZE = 8;

/**
 * @brief Byte offset of the unique word field in the file header.
 *
 * Word 1: must equal HIPO_FILE_UNIQUE_WORD to identify a valid HIPO file.
 */
inline constexpr int FH_UNIQUE_WORD_OFFSET     = 0;   // word 1

/** @brief Byte offset of the file number field in the file header (word 2). */
inline constexpr int FH_FILE_NUMBER_OFFSET     = 4;   // word 2

/** @brief Byte offset of the header length field in the file header (word 3, value in 32-bit words). */
inline constexpr int FH_HEADER_LENGTH_OFFSET   = 8;   // word 3 (in words)

/** @brief Byte offset of the record count field in the file header (word 4). */
inline constexpr int FH_RECORD_COUNT_OFFSET    = 12;  // word 4

/** @brief Byte offset of the index array length field in the file header (word 5, value in bytes). */
inline constexpr int FH_INDEX_ARRAY_LEN_OFFSET = 16;  // word 5 (in bytes)

/** @brief Byte offset of the bit-info/version field in the file header (word 6). */
inline constexpr int FH_BIT_INFO_OFFSET        = 20;  // word 6

/** @brief Byte offset of the user header length field in the file header (word 7, value in bytes). */
inline constexpr int FH_USER_HEADER_LEN_OFFSET = 24;  // word 7 (in bytes)

/** @brief Byte offset of the magic number field in the file header (word 8). */
inline constexpr int FH_MAGIC_NUMBER_OFFSET    = 28;  // word 8

/** @brief Byte offset of the user register field in the file header (words 9-10, 64-bit). */
inline constexpr int FH_USER_REGISTER_OFFSET   = 32;  // words 9-10 (64-bit)

/** @brief Byte offset of the trailer position field in the file header (words 11-12, 64-bit). */
inline constexpr int FH_TRAILER_POS_OFFSET     = 40;  // words 11-12 (64-bit)

/** @brief Byte offset of the first user integer field in the file header (word 13). */
inline constexpr int FH_USER_INT1_OFFSET       = 48;  // word 13

/** @brief Byte offset of the second user integer field in the file header (word 14). */
inline constexpr int FH_USER_INT2_OFFSET       = 52;  // word 14

/** @brief Schema group identifier used for dictionary records. */
inline constexpr int DICT_GROUP  = 120;

/** @brief Schema item identifier used for dictionary records. */
inline constexpr int DICT_ITEM   = 2;

/** @brief Group identifier for configuration records. */
inline constexpr int CONFIG_GROUP = 32555;

/**
 * @brief Identifies the type of a HIPO/EVIO header.
 *
 * Encoded in bits[28-31] of the bit-info word in file headers,
 * and bits[8-11] in record headers.
 */
enum class header_type : uint8_t {
    evio_record      = 0, /**< @brief EVIO record header. */
    evio_file        = 1, /**< @brief EVIO file header. */
    evio_ext_file    = 2, /**< @brief EVIO extended file header. */
    hipo_record      = 4, /**< @brief HIPO record header. */
    hipo_file        = 5, /**< @brief HIPO file header. */
    hipo_ext_file    = 6, /**< @brief HIPO extended file header. */
    hipo_trailer     = 7, /**< @brief HIPO file trailer record. */
};

/**
 * @brief Compression algorithm used for record data.
 *
 * Encoded in bits[28-31] of the compression word in record headers.
 */
enum class compression_type : uint8_t {
    none     = 0, /**< @brief No compression. */
    lz4      = 1, /**< @brief LZ4 fast compression. */
    lz4_best = 2, /**< @brief LZ4 high-compression mode. */
    gzip     = 3, /**< @brief GZIP compression. */
};

/**
 * @brief Bit-info word layout documentation.
 *
 * **File header bit-info:**
 * - bits[0-7]:   version number
 * - bit[8]:      has dictionary (first record)
 * - bit[9]:      has "first event" (in every split file)
 * - bit[10]:     file trailer with index exists
 * - bits[11-19]: reserved
 * - bits[20-21]: pad1 (user header padding bytes)
 * - bits[22-23]: pad2 (data padding bytes)
 * - bits[24-25]: pad3 (compressed data padding bytes)
 * - bits[26-27]: reserved
 * - bits[28-31]: general header type (see header_type)
 *
 * **Record header bit-info:**
 * - bits[0-7]:   version number
 * - bit[8]:      last record flag
 * - bit[9]:      reserved
 * - bit[10]:     has-dictionary (record has dictionary in user header)
 * - bit[11]:     has-first-event
 * - bits[20-21]: pad1
 * - bits[22-23]: pad2
 * - bits[24-25]: pad3
 */

/** @brief Mask to extract the version number (bits[0-7]) from the bit-info word. */
inline constexpr int BITINFO_VERSION_MASK          = 0x000000FF;

/** @brief Bit position of the has-dictionary flag in the bit-info word. */
inline constexpr int BITINFO_HAS_DICTIONARY_BIT    = 8;

/** @brief Bit position of the has-first-event flag in the bit-info word. */
inline constexpr int BITINFO_HAS_FIRST_EVENT_BIT   = 9;

/** @brief Bit position of the trailer-with-index flag in the file header bit-info word. */
inline constexpr int BITINFO_TRAILER_WITH_INDEX_BIT = 10;

/** @brief Bit shift for pad1 (user header padding) in the bit-info word. */
inline constexpr int BITINFO_PAD1_SHIFT            = 20;

/** @brief Bit shift for pad2 (data padding) in the bit-info word. */
inline constexpr int BITINFO_PAD2_SHIFT            = 22;

/** @brief Bit shift for pad3 (compressed data padding) in the bit-info word. */
inline constexpr int BITINFO_PAD3_SHIFT            = 24;

/** @brief Mask for extracting a 2-bit padding value after shifting. */
inline constexpr int BITINFO_PAD_MASK              = 0x3;

/** @brief Bit shift for the header type field (bits[28-31]) in the bit-info word. */
inline constexpr int BITINFO_HEADER_TYPE_SHIFT     = 28;

/**
 * @brief Compression word layout constants.
 *
 * The compression word in a record header encodes two fields:
 * - bits[28-31]: compression type (see compression_type)
 * - bits[0-27]:  compressed data length in 32-bit words (not bytes)
 *
 * To obtain the compressed size in bytes, multiply the extracted length by 4.
 */

/** @brief Mask to extract the compression type (bits[28-31]) from the compression word. */
inline constexpr uint32_t COMP_TYPE_MASK   = 0xF0000000;

/** @brief Bit shift for the compression type field in the compression word. */
inline constexpr int      COMP_TYPE_SHIFT  = 28;

/** @brief Mask to extract the compressed data length in words (bits[0-27]) from the compression word. */
inline constexpr uint32_t COMP_LENGTH_MASK = 0x0FFFFFFF;

/**
 * @brief HIPO bank column data types.
 *
 * Each enumerator value matches the type code stored in the HIPO binary format.
 * Use type_size() to obtain the byte width for a given column type.
 */
enum class column_type : uint8_t {
    byte_t   = 1, /**< @brief 8-bit signed integer (int8_t). */
    short_t  = 2, /**< @brief 16-bit signed integer (int16_t). */
    int_t    = 3, /**< @brief 32-bit signed integer (int32_t). */
    float_t  = 4, /**< @brief 32-bit IEEE 754 floating point (float). */
    double_t = 5, /**< @brief 64-bit IEEE 754 floating point (double). */
    long_t   = 8, /**< @brief 64-bit signed integer (int64_t). */
};

/**
 * @brief Returns the size in bytes of a given column data type.
 * @param ct The column type to query.
 * @return Size in bytes (1, 2, 4, or 8), or 0 for an unrecognized type.
 */
constexpr int type_size(column_type ct) noexcept {
    switch (ct) {
    case column_type::byte_t:   return 1;
    case column_type::short_t:  return 2;
    case column_type::int_t:    return 4;
    case column_type::float_t:  return 4;
    case column_type::double_t: return 8;
    case column_type::long_t:   return 8;
    }
    return 0;
}

/**
 * @brief Maps a column_type enumerator to its corresponding C++ type at compile time.
 * @tparam CT The column_type value to map.
 *
 * Specializations define a nested `type` alias for each column_type:
 * - byte_t   -> int8_t
 * - short_t  -> int16_t
 * - int_t    -> int32_t
 * - float_t  -> float
 * - double_t -> double
 * - long_t   -> int64_t
 */
template <column_type CT> struct column_cpp_type_impl;
template <> struct column_cpp_type_impl<column_type::byte_t>   { using type = int8_t; };
template <> struct column_cpp_type_impl<column_type::short_t>  { using type = int16_t; };
template <> struct column_cpp_type_impl<column_type::int_t>    { using type = int32_t; };
template <> struct column_cpp_type_impl<column_type::float_t>  { using type = float; };
template <> struct column_cpp_type_impl<column_type::double_t> { using type = double; };
template <> struct column_cpp_type_impl<column_type::long_t>   { using type = int64_t; };

/**
 * @brief Convenience alias that extracts the C++ type for a given column_type.
 * @tparam CT The column_type value to map.
 *
 * Example: `column_cpp_type<column_type::float_t>` resolves to `float`.
 */
template <column_type CT>
using column_cpp_type = typename column_cpp_type_impl<CT>::type;

/**
 * @brief Constrains a type to be one of the valid HIPO bank column C++ types.
 * @tparam T The type to check.
 *
 * Satisfied by: int8_t, int16_t, int32_t, float, double, int64_t.
 */
template <typename T>
concept BankValue = std::is_same_v<T, int8_t>   ||
                    std::is_same_v<T, int16_t>  ||
                    std::is_same_v<T, int32_t>  ||
                    std::is_same_v<T, float>    ||
                    std::is_same_v<T, double>   ||
                    std::is_same_v<T, int64_t>;

/**
 * @brief Lookup table for computing 4-byte alignment padding.
 *
 * `PAD_VALUES[length % 4]` gives the number of padding bytes needed
 * to reach the next 4-byte boundary.
 */
inline constexpr int PAD_VALUES[4] = {0, 3, 2, 1};

/**
 * @brief Computes the number of padding bytes needed for 4-byte alignment.
 * @param length The current byte length.
 * @return Number of padding bytes (0-3) to add for 4-byte alignment.
 */
constexpr int pad4(int length) noexcept {
    return PAD_VALUES[length & 3];
}

/**
 * @brief Reads a value of type T from a raw byte buffer using memcpy.
 *
 * Provides a safe alternative to reinterpret_cast that avoids
 * undefined behavior from strict aliasing violations.
 *
 * @tparam T The type to read (must be trivially copyable).
 * @param ptr Pointer to the source bytes (must have at least sizeof(T) readable bytes).
 * @return The value of type T read from the buffer.
 */
template <typename T>
inline T read_value(const char* ptr) noexcept {
    T value;
    std::memcpy(&value, ptr, sizeof(T));
    return value;
}

/**
 * @brief Writes a value of type T into a raw byte buffer using memcpy.
 *
 * Provides a safe alternative to reinterpret_cast that avoids
 * undefined behavior from strict aliasing violations.
 *
 * @tparam T The type to write (must be trivially copyable).
 * @param ptr Pointer to the destination buffer (must have at least sizeof(T) writable bytes).
 * @param value The value to write.
 */
template <typename T>
inline void write_value(char* ptr, T value) noexcept {
    std::memcpy(ptr, &value, sizeof(T));
}

/**
 * @brief Conditionally byte-swaps a 32-bit integer.
 * @param v The value to potentially swap.
 * @param swap If true, performs a byte-order reversal; otherwise returns v unchanged.
 * @return The (possibly swapped) 32-bit value.
 */
inline int32_t maybe_swap32(int32_t v, bool swap) noexcept {
    return swap ? static_cast<int32_t>(__builtin_bswap32(static_cast<uint32_t>(v))) : v;
}

/**
 * @brief Conditionally byte-swaps a 64-bit integer.
 * @param v The value to potentially swap.
 * @param swap If true, performs a byte-order reversal; otherwise returns v unchanged.
 * @return The (possibly swapped) 64-bit value.
 */
inline int64_t maybe_swap64(int64_t v, bool swap) noexcept {
    return swap ? static_cast<int64_t>(__builtin_bswap64(static_cast<uint64_t>(v))) : v;
}

/**
 * @brief Grows a buffer to at least the requested size, doubling capacity on growth.
 *
 * If the buffer is already large enough, this is a no-op. Otherwise, the buffer
 * is resized to `max(needed, current_size * 2)` to amortize allocation costs.
 *
 * @param buf The buffer to grow.
 * @param needed The minimum required size in bytes.
 */
inline void grow_buffer(std::vector<char>& buf, int needed) {
    if (static_cast<int>(buf.size()) < needed)
        buf.resize(std::max(needed, static_cast<int>(buf.size()) * 2));
}

} // namespace hipo
