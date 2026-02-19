#include "record.h"

#include <cstring>

#include <lz4.h>
#include <zlib.h>

// In-place LZ4 decompression requires LZ4 >= 1.8.2
#ifdef LZ4_DECOMPRESS_INPLACE_MARGIN
#define HIPO_LZ4_INPLACE 1
#else
#define HIPO_LZ4_INPLACE 0
#endif

namespace hipo {

namespace {

// Find a bank structure within event data (scanning the event's structure list)
// Event layout: 16-byte header, then structures: [group:2][item:1][type:1][length:4][data...]
std::pair<int, int> find_structure_in_event(const char* event_buf, int event_size,
                                            int group, int item) noexcept {
    if (event_size < 16) return {-1, 0};

    int position = 16; // skip 16-byte event header
    int total = read_value<int32_t>(event_buf + 4); // event size from header
    if (total > event_size) total = event_size;

    while (position + 8 <= total) {
        auto gid    = read_value<uint16_t>(event_buf + position);
        auto iid    = read_value<uint8_t>(event_buf + position + 2);
        auto length = read_value<int32_t>(event_buf + position + 4);
        if (length < 0) break;

        if (static_cast<int>(gid) == group && static_cast<int>(iid) == item)
            return {position, length};

        position += (length + 8);
    }
    return {-1, 0};
}

} // anonymous namespace

result<void> record::parse_header(const char* buf) {
    m_header.record_length      = read_value<int32_t>(buf + 0);
    m_header.record_number      = read_value<int32_t>(buf + 4);
    m_header.header_length      = read_value<int32_t>(buf + 8);
    m_header.event_count        = read_value<int32_t>(buf + 12);
    m_header.index_data_length  = read_value<int32_t>(buf + 16);
    m_header.bit_info           = read_value<int32_t>(buf + 20);
    m_header.user_header_length = read_value<int32_t>(buf + 24);
    m_header.magic_number       = read_value<int32_t>(buf + 28);
    m_header.data_length        = read_value<int32_t>(buf + 32);
    m_header.compression_word   = read_value<int32_t>(buf + 36);

    // Endianness detection
    m_header.byte_swapped = (m_header.magic_number == static_cast<int32_t>(0x0001dac0));
    bool s = m_header.byte_swapped;
    m_header.record_length      = maybe_swap32(m_header.record_length, s);
    m_header.record_number      = maybe_swap32(m_header.record_number, s);
    m_header.header_length      = maybe_swap32(m_header.header_length, s);
    m_header.event_count        = maybe_swap32(m_header.event_count, s);
    m_header.index_data_length  = maybe_swap32(m_header.index_data_length, s);
    m_header.bit_info           = maybe_swap32(m_header.bit_info, s);
    m_header.user_header_length = maybe_swap32(m_header.user_header_length, s);
    m_header.data_length        = maybe_swap32(m_header.data_length, s);
    m_header.compression_word   = maybe_swap32(m_header.compression_word, s);
    if (s) m_header.magic_number = static_cast<int32_t>(HEADER_MAGIC);

    if (m_header.magic_number != static_cast<int32_t>(HEADER_MAGIC))
        return unexpected(error_code::invalid_magic_number);

    // Derive compression info
    m_header.comp_type = static_cast<compression_type>(
        (m_header.compression_word >> COMP_TYPE_SHIFT) & 0xF);
    m_header.compressed_length = (m_header.compression_word & COMP_LENGTH_MASK) * 4;

    // Derive padding from bit-info
    m_header.user_header_padding = (m_header.bit_info >> BITINFO_PAD1_SHIFT) & BITINFO_PAD_MASK;
    m_header.data_padding        = (m_header.bit_info >> BITINFO_PAD2_SHIFT) & BITINFO_PAD_MASK;
    m_header.comp_data_padding   = (m_header.bit_info >> BITINFO_PAD3_SHIFT) & BITINFO_PAD_MASK;

    // Compute events offset within decompressed data
    m_events_offset = m_header.index_data_length
                    + m_header.user_header_length
                    + m_header.user_header_padding;

    return ok();
}

result<void> record::decompress_data(const char* compressed, int compressed_len, int decompressed_len) {
    grow_buffer(m_decompressed_buf, decompressed_len);

    if (m_header.comp_type == compression_type::none) {
        std::memcpy(m_decompressed_buf.data(), compressed, decompressed_len);
        return ok();
    }

    if (m_header.comp_type == compression_type::lz4 ||
        m_header.comp_type == compression_type::lz4_best) {
        int result = LZ4_decompress_safe(compressed, m_decompressed_buf.data(),
                                         compressed_len, decompressed_len);
        if (result < 0)
            return unexpected(error_code::record_decompress_failed);
        return ok();
    }

    if (m_header.comp_type == compression_type::gzip) {
        unsigned long dest_len = static_cast<unsigned long>(decompressed_len);
        int zresult = ::uncompress(
            reinterpret_cast<Bytef*>(m_decompressed_buf.data()),
            &dest_len,
            reinterpret_cast<const Bytef*>(compressed),
            static_cast<unsigned long>(compressed_len));
        if (zresult != Z_OK)
            return unexpected(error_code::record_decompress_failed);
        return ok();
    }

    return unexpected(error_code::record_unsupported_compression);
}

void record::convert_index() {
    // Convert event lengths in index array to cumulative offsets
    // (from RecordInputStream.java)
    int event_pos = 0;
    char* base = m_decompressed_buf.data();
    bool s = m_header.byte_swapped;
    for (int i = 0; i < m_header.event_count; ++i) {
        int32_t size = maybe_swap32(read_value<int32_t>(base + i * 4), s);
        event_pos += size;
        write_value(base + i * 4, event_pos);
    }
}

result<void> record::read_and_decompress(file_source& source, int64_t data_position,
                                          int compressed_input_len, int decompressed_len) {
#if HIPO_LZ4_INPLACE
    // In-place LZ4: read compressed data into tail of decompressed buffer
    if ((m_header.comp_type == compression_type::lz4 ||
         m_header.comp_type == compression_type::lz4_best) &&
        compressed_input_len < decompressed_len) {

        int margin = LZ4_DECOMPRESS_INPLACE_MARGIN(compressed_input_len);
        int total_buf = decompressed_len + margin;

        grow_buffer(m_decompressed_buf, total_buf);

        char* compressed_start = m_decompressed_buf.data() + total_buf - compressed_input_len;

        auto read_data = source.read_at(data_position, compressed_start, compressed_input_len);
        if (!read_data) return unexpected(read_data.error());

        int result = LZ4_decompress_safe(compressed_start, m_decompressed_buf.data(),
                                          compressed_input_len, decompressed_len);
        if (result < 0)
            return unexpected(error_code::record_decompress_failed);

        convert_index();
        return ok();
    }
#endif

    // Uncompressed: read directly into decompressed buffer
    if (m_header.comp_type == compression_type::none) {
        grow_buffer(m_decompressed_buf, decompressed_len);

        auto read_data = source.read_at(data_position, m_decompressed_buf.data(), decompressed_len);
        if (!read_data) return unexpected(read_data.error());

        convert_index();
        return ok();
    }

    // Fallback (gzip, or LZ4 with compressed >= decompressed): two-buffer path
    grow_buffer(m_compressed_buf, compressed_input_len);

    auto read_data = source.read_at(data_position, m_compressed_buf.data(), compressed_input_len);
    if (!read_data) return unexpected(read_data.error());

    auto decomp_res = decompress_data(m_compressed_buf.data(), compressed_input_len, decompressed_len);
    if (!decomp_res) return unexpected(decomp_res.error());

    convert_index();
    return ok();
}

result<void> record::read_from(file_source& source, int64_t position, int64_t file_size) {
    if (position + RECORD_HEADER_SIZE > file_size)
        return unexpected(error_code::record_bounds_exceeded);

    char header_buf[RECORD_HEADER_SIZE];
    auto read_res = source.read_at(position, header_buf, RECORD_HEADER_SIZE);
    if (!read_res) return unexpected(read_res.error());

    auto parse_res = parse_header(header_buf);
    if (!parse_res) return unexpected(parse_res.error());

    int header_bytes = m_header.header_length * 4;

    if (position + m_header.record_length * 4LL > file_size)
        return unexpected(error_code::record_bounds_exceeded);

    int data_buffer_bytes = m_header.record_length * 4 - header_bytes;
    int decompressed_len = m_header.index_data_length
                         + m_header.user_header_length
                         + m_header.user_header_padding
                         + m_header.data_length;
    int compressed_input_len = data_buffer_bytes - m_header.comp_data_padding;

    return read_and_decompress(source, position + header_bytes,
                                compressed_input_len, decompressed_len);
}

result<void> record::read_from(file_source& source, int64_t position, int64_t file_size, int record_bytes) {
    if (position + record_bytes > file_size)
        return unexpected(error_code::record_bounds_exceeded);

    // Delegate to 2-pread path (in-place decompression makes 1-pread unnecessary)
    return read_from(source, position, file_size);
}

result<std::span<const char>> record::event_data(int index) const noexcept {
    if (index < 0 || index >= m_header.event_count)
        return unexpected(error_code::event_index_out_of_range);

    int start_offset = 0;
    if (index > 0)
        start_offset = read_value<int32_t>(m_decompressed_buf.data() + (index - 1) * 4);

    int end_offset = read_value<int32_t>(m_decompressed_buf.data() + index * 4);
    int event_start = m_events_offset + start_offset;
    int event_size = end_offset - start_offset;

    return std::span<const char>(m_decompressed_buf.data() + event_start, event_size);
}

result<std::span<const char>> record::find_structure(int event_index, int group, int item) const noexcept {
    auto ev = event_data(event_index);
    if (!ev) return unexpected(ev.error());

    auto [pos, length] = find_structure_in_event(ev->data(), static_cast<int>(ev->size()), group, item);
    if (pos < 0)
        return unexpected(error_code::event_structure_not_found);

    // Return the full structure: 8-byte header + data
    return std::span<const char>(ev->data() + pos, length + 8);
}

void record::find_structures_in_event(int event_index,
                                      std::span<const std::pair<int,int>> group_items,
                                      std::span<structure_position> results) const noexcept {
    auto ev = event_data(event_index);
    if (!ev) {
        for (auto& r : results) r = {-1, 0};
        return;
    }
    find_structures_in_event(*ev, group_items, results);
}

void record::find_structures_in_event(std::span<const char> event,
                                      std::span<const std::pair<int,int>> group_items,
                                      std::span<structure_position> results) noexcept {
    for (auto& r : results) r = {-1, 0};

    int event_size = static_cast<int>(event.size());
    if (event_size < 16) return;

    const char* buf = event.data();
    int total = read_value<int32_t>(buf + 4);
    if (total > event_size) total = event_size;

    int found = 0;
    int n = static_cast<int>(group_items.size());
    int position = 16; // skip 16-byte event header
    while (position + 8 <= total && found < n) {
        auto gid    = static_cast<int>(read_value<uint16_t>(buf + position));
        auto iid    = static_cast<int>(read_value<uint8_t>(buf + position + 2));
        auto length = read_value<int32_t>(buf + position + 4);
        if (length < 0) break;

        for (int i = 0; i < n; ++i) {
            if (!results[i].found() &&
                gid == group_items[i].first && iid == group_items[i].second) {
                results[i] = {position, length};
                ++found;
                break;
            }
        }
        position += (length + 8);
    }
}

std::span<const char> record::user_header() const noexcept {
    if (m_header.user_header_length <= 0)
        return {};
    int uh_start = m_header.index_data_length;
    return std::span<const char>(m_decompressed_buf.data() + uh_start,
                                 m_header.user_header_length);
}

void record::reserve_buffers(int compressed_bytes, int decompressed_bytes) {
    if (static_cast<int>(m_compressed_buf.size()) < compressed_bytes)
        m_compressed_buf.resize(compressed_bytes);
#if HIPO_LZ4_INPLACE
    int margin = LZ4_DECOMPRESS_INPLACE_MARGIN(compressed_bytes);
    int total = decompressed_bytes + margin;
#else
    int total = decompressed_bytes;
#endif
    if (static_cast<int>(m_decompressed_buf.size()) < total)
        m_decompressed_buf.resize(total);
}

} // namespace hipo
