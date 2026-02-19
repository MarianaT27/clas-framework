#include "record_builder.h"

#include <cstring>

#include <lz4.h>

namespace hipo {

record_builder::record_builder(record_builder_options opts)
    : m_opts(opts) {
    m_index_buf.resize(4 * opts.max_events);
    m_events_buf.resize(opts.max_record_bytes);
    m_data_buf.resize(opts.max_record_bytes + 4 * opts.max_events);
    m_record_buf.resize(opts.max_record_bytes + 4 * opts.max_events + 512 * 1024);
}

bool record_builder::add_event(std::span<const char> event_data) {
    int len = static_cast<int>(event_data.size());
    if (m_events_pos + len >= static_cast<int>(m_events_buf.size()))
        return false;
    if ((m_index_count + 1) * 4 >= static_cast<int>(m_index_buf.size()))
        return false;

    // Write event length to index array
    write_value<int32_t>(m_index_buf.data() + m_index_count * 4, len);
    m_index_count++;

    // Copy event data
    std::memcpy(m_events_buf.data() + m_events_pos, event_data.data(), len);
    m_events_pos += len;
    return true;
}

result<void> record_builder::build() {
    int index_size  = m_index_count * 4;
    int events_size = m_events_pos;

    // Concatenate index + events into data buffer
    int uncompressed_size = index_size + events_size;
    if (static_cast<int>(m_data_buf.size()) < uncompressed_size)
        m_data_buf.resize(uncompressed_size);
    std::memcpy(m_data_buf.data(), m_index_buf.data(), index_size);
    std::memcpy(m_data_buf.data() + index_size, m_events_buf.data(), events_size);

    // Compress
    int max_dst = LZ4_compressBound(uncompressed_size);
    if (static_cast<int>(m_record_buf.size()) < RECORD_HEADER_SIZE + max_dst)
        m_record_buf.resize(RECORD_HEADER_SIZE + max_dst + 256);

    int compressed_size = 0;
    if (m_opts.compression == compression_type::lz4 ||
        m_opts.compression == compression_type::lz4_best) {
        compressed_size = LZ4_compress_fast(
            m_data_buf.data(),
            m_record_buf.data() + RECORD_HEADER_SIZE,
            uncompressed_size,
            static_cast<int>(m_record_buf.size()) - RECORD_HEADER_SIZE,
            1);
        if (compressed_size <= 0)
            return unexpected(error_code::write_compression_failed);
    } else {
        // No compression: copy directly
        std::memcpy(m_record_buf.data() + RECORD_HEADER_SIZE,
                     m_data_buf.data(), uncompressed_size);
        compressed_size = uncompressed_size;
    }

    // Compute padding to align to 4-byte boundary
    int rounding = pad4(compressed_size);
    int compressed_with_pad = compressed_size + rounding;
    int compressed_words    = compressed_with_pad / 4;
    int record_length_words = compressed_words + RECORD_HEADER_WORDS;

    // Write 14-word record header
    char* h = m_record_buf.data();
    write_value<int32_t>(h + 0,  record_length_words);                                // word 1: record length in words
    write_value<int32_t>(h + 4,  0);                                                   // word 2: record number
    write_value<int32_t>(h + 8,  RECORD_HEADER_WORDS);                                // word 3: header length in words
    write_value<int32_t>(h + 12, m_index_count);                                       // word 4: event count
    write_value<int32_t>(h + 16, index_size);                                          // word 5: index data length (bytes)
    int version_word = (rounding << BITINFO_PAD3_SHIFT) | 6;                           // pad3 + version 6
    write_value<int32_t>(h + 20, version_word);                                        // word 6: bit-info
    write_value<int32_t>(h + 24, 0);                                                   // word 7: user header length
    write_value<int32_t>(h + 28, static_cast<int32_t>(HEADER_MAGIC));                 // word 8: magic number
    write_value<int32_t>(h + 32, events_size);                                         // word 9: data length (uncompressed events only)
    int comp_type_bits = static_cast<int>(m_opts.compression) << COMP_TYPE_SHIFT;
    int comp_word = comp_type_bits | (compressed_words & static_cast<int>(COMP_LENGTH_MASK));
    write_value<int32_t>(h + 36, comp_word);                                           // word 10: compression word
    write_value<int64_t>(h + 40, m_user_word_one);                                     // words 11-12: user word one
    write_value<int64_t>(h + 48, m_user_word_two);                                     // words 13-14: user word two

    m_record_size = record_length_words * 4;
    return ok();
}

result<void> record_builder::build_with_user_header(std::span<const char> user_header) {
    // Build a record with no events, only a user header (for dictionary/trailer records)
    int uh_len = static_cast<int>(user_header.size());
    int uh_padding = pad4(uh_len);
    int uncompressed_size = uh_len + uh_padding;

    if (static_cast<int>(m_data_buf.size()) < uncompressed_size)
        m_data_buf.resize(uncompressed_size);
    std::memcpy(m_data_buf.data(), user_header.data(), uh_len);
    // Zero-fill padding
    std::memset(m_data_buf.data() + uh_len, 0, uh_padding);

    // Compress
    int max_dst = LZ4_compressBound(uncompressed_size);
    if (static_cast<int>(m_record_buf.size()) < RECORD_HEADER_SIZE + max_dst)
        m_record_buf.resize(RECORD_HEADER_SIZE + max_dst + 256);

    int compressed_size = 0;
    if (m_opts.compression == compression_type::lz4 ||
        m_opts.compression == compression_type::lz4_best) {
        compressed_size = LZ4_compress_fast(
            m_data_buf.data(),
            m_record_buf.data() + RECORD_HEADER_SIZE,
            uncompressed_size,
            static_cast<int>(m_record_buf.size()) - RECORD_HEADER_SIZE,
            1);
        if (compressed_size <= 0)
            return unexpected(error_code::write_compression_failed);
    } else {
        std::memcpy(m_record_buf.data() + RECORD_HEADER_SIZE,
                     m_data_buf.data(), uncompressed_size);
        compressed_size = uncompressed_size;
    }

    int rounding = pad4(compressed_size);
    int compressed_with_pad = compressed_size + rounding;
    int compressed_words    = compressed_with_pad / 4;
    int record_length_words = compressed_words + RECORD_HEADER_WORDS;

    char* h = m_record_buf.data();
    write_value<int32_t>(h + 0,  record_length_words);
    write_value<int32_t>(h + 4,  0);
    write_value<int32_t>(h + 8,  RECORD_HEADER_WORDS);
    write_value<int32_t>(h + 12, 0);                                                   // no events
    write_value<int32_t>(h + 16, 0);                                                   // no index
    int version_word = (uh_padding << BITINFO_PAD1_SHIFT)                              // pad1 = user header padding
                     | (rounding << BITINFO_PAD3_SHIFT)                                // pad3 = compressed data padding
                     | 6;                                                              // version 6
    write_value<int32_t>(h + 20, version_word);
    write_value<int32_t>(h + 24, uh_len);                                              // user header length (bytes)
    write_value<int32_t>(h + 28, static_cast<int32_t>(HEADER_MAGIC));
    write_value<int32_t>(h + 32, 0);                                                   // data length = 0 (no events)
    int comp_type_bits = static_cast<int>(m_opts.compression) << COMP_TYPE_SHIFT;
    int comp_word = comp_type_bits | (compressed_words & static_cast<int>(COMP_LENGTH_MASK));
    write_value<int32_t>(h + 36, comp_word);
    write_value<int64_t>(h + 40, m_user_word_one);
    write_value<int64_t>(h + 48, m_user_word_two);

    m_record_size = record_length_words * 4;
    return ok();
}

void record_builder::reset() {
    m_index_count = 0;
    m_events_pos  = 0;
    m_record_size = 0;
}

std::span<const char> record_builder::record_data() const noexcept {
    return {m_record_buf.data(), static_cast<size_t>(m_record_size)};
}

int record_builder::record_size() const noexcept {
    return m_record_size;
}

} // namespace hipo
