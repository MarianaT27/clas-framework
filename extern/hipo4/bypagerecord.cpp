// SPDX-License-Identifier: BSD-3-Clause

#include "bypagerecord.h"

#include "constants.h"
#include "hipoexceptions.h"

#include <atomic>
#include <cstring>
#include <stdexcept>
#include <string>

#ifdef __LZ4__
#include <lz4.h>
#endif

namespace hipo {

std::atomic<uint64_t>& bypagerecord::inflate_counter() {
    static std::atomic<uint64_t> counter{0};
    return counter;
}

namespace {
inline uint32_t read_u32_le(const char* p, size_t off) {
    uint32_t v;
    std::memcpy(&v, p + off, sizeof(v));
    return v;
}
inline uint16_t read_u16_le(const char* p, size_t off) {
    uint16_t v;
    std::memcpy(&v, p + off, sizeof(v));
    return v;
}

// O(1) lookup by (group, item) via the dictionary's (group<<8|item) hash.
inline const hipo::schema* find_schema(const hipo::dictionary& dict, uint16_t group, uint8_t item) {
    return dict.findByGroupItem(group, item);
}
} // namespace

std::shared_ptr<bypagerecord> bypagerecord::parse(const recordHeader_t& header,
                                                  const char* compressed,
                                                  int compressed_bytes,
                                                  int compressed_padding,
                                                  const hipo::dictionary& dict) {
    if (header.dataEndianness == 1) {
        throw hipo::record_error(
            "bypagerecord: big-endian Lz4ByPage records are not supported");
    }
    const int section_bytes = compressed_bytes - compressed_padding;
    if (section_bytes < 12) {
        throw hipo::record_error("bypagerecord: section truncated (header)");
    }
    auto rec = std::shared_ptr<bypagerecord>(new bypagerecord{});
    rec->hdr_ = header;
    rec->raw_.assign(compressed, compressed + section_bytes);
    rec->raw_ptr_ = rec->raw_.data();
    hipo_parse_bypage_directory(rec.get(), rec->raw_ptr_, section_bytes, dict);
    return rec;
}

std::shared_ptr<bypagerecord> bypagerecord::parse_take_raw(recordHeader_t header,
                                                            std::unique_ptr<char[]>&& buf,
                                                            int buf_capacity,
                                                            int compressed_bytes,
                                                            int compressed_padding,
                                                            const hipo::dictionary& dict) {
    if (header.dataEndianness == 1) {
        throw hipo::record_error(
            "bypagerecord: big-endian Lz4ByPage records are not supported");
    }
    const int section_bytes = compressed_bytes - compressed_padding;
    if (section_bytes < 12) {
        throw hipo::record_error("bypagerecord: section truncated (header)");
    }
    if (buf_capacity < compressed_bytes) {
        throw hipo::record_error("bypagerecord: take_raw buffer smaller than compressed_bytes");
    }
    auto rec = std::shared_ptr<bypagerecord>(new bypagerecord{});
    rec->hdr_       = header;
    rec->raw_owned_ = std::move(buf);
    rec->raw_ptr_   = rec->raw_owned_.get();
    hipo_parse_bypage_directory(rec.get(), rec->raw_ptr_, section_bytes, dict);
    return rec;
}

std::shared_ptr<bypagerecord> bypagerecord::parse_view(recordHeader_t header,
                                                       const char* compressed,
                                                       int compressed_bytes,
                                                       int compressed_padding,
                                                       const hipo::dictionary& dict) {
    if (header.dataEndianness == 1) {
        throw hipo::record_error(
            "bypagerecord: big-endian Lz4ByPage records are not supported");
    }
    const int section_bytes = compressed_bytes - compressed_padding;
    if (section_bytes < 12) {
        throw hipo::record_error("bypagerecord: section truncated (header)");
    }
    auto rec = std::shared_ptr<bypagerecord>(new bypagerecord{});
    rec->hdr_     = header;
    rec->raw_ptr_ = compressed;   // borrowed — caller guarantees lifetime
    hipo_parse_bypage_directory(rec.get(), rec->raw_ptr_, section_bytes, dict);
    return rec;
}

void hipo_parse_bypage_directory(bypagerecord* rec, const char* p, int section_bytes,
                                 const hipo::dictionary& dict) {
    const uint32_t num_banks       = read_u32_le(p, 0);
    const uint32_t event_count     = read_u32_le(p, 4);
    const uint32_t events_per_page = read_u32_le(p, 8);
    if (event_count != static_cast<uint32_t>(rec->hdr_.numberOfEvents)) {
        throw hipo::record_error("bypagerecord: event_count mismatch with record header");
    }
    if (events_per_page < 1) {
        throw hipo::record_error("bypagerecord: events_per_page must be >= 1");
    }
    rec->event_count_          = event_count;
    rec->events_per_page_      = events_per_page;
    rec->num_pages_per_column_ = (event_count + events_per_page - 1) / events_per_page;

    // Bank descriptors: 8 bytes each (u16 group, u8 item, u8 type, u32 num_columns).
    const size_t desc_off          = 12;
    const size_t desc_bytes        = 8 * num_banks;
    const size_t presence_off      = desc_off + desc_bytes;
    const size_t bytes_per_row     = (num_banks + 7) / 8;
    const size_t presence_bytes    = event_count * bytes_per_row;
    const size_t rowcounts_off     = presence_off + presence_bytes;
    const size_t rowcounts_bytes   = static_cast<size_t>(4) * num_banks * event_count;
    const size_t streams_hdr_off   = rowcounts_off + rowcounts_bytes;

    if (static_cast<size_t>(section_bytes) < streams_hdr_off + 4) {
        throw hipo::record_error("bypagerecord: directory truncated");
    }

    // Bank descriptors + column info from dictionary.
    rec->descriptors_.reserve(num_banks);
    rec->num_columns_.reserve(num_banks);
    rec->bank_columns_.reserve(num_banks);
    rec->bank_index_map_.reserve(num_banks);
    rec->bank_stream_offset_.reserve(num_banks + 1);
    rec->bank_stream_offset_.push_back(0);
    uint32_t stream_cursor = 0;

    for (uint32_t b = 0; b < num_banks; ++b) {
        size_t off = desc_off + b * 8;
        bank_descriptor_t d{};
        d.group     = read_u16_le(p, off);
        d.item      = static_cast<uint8_t>(p[off + 2]);
        d.data_type = static_cast<uint8_t>(p[off + 3]);
        uint32_t ncols = read_u32_le(p, off + 4);
        rec->descriptors_.push_back(d);
        rec->num_columns_.push_back(ncols);
        rec->bank_index_map_[(static_cast<uint32_t>(d.group) << 8) | d.item] = static_cast<int>(b);

        // Resolve column types from the schema dictionary.
        const hipo::schema* sch = find_schema(dict, d.group, d.item);
        if (!sch) {
            throw hipo::record_error(
                "bypagerecord: schema (" + std::to_string(d.group) + "," +
                std::to_string(d.item) + ") not in dictionary");
        }
        if (static_cast<int>(ncols) != const_cast<hipo::schema*>(sch)->getEntries()) {
            throw hipo::record_error(
                "bypagerecord: column count mismatch for schema " +
                std::string(const_cast<hipo::schema*>(sch)->getName()));
        }
        std::vector<bypagerecord::column_info> cols;
        cols.reserve(ncols);
        for (uint32_t c = 0; c < ncols; ++c) {
            const auto& e = const_cast<hipo::schema*>(sch)->getEntry(static_cast<int>(c));
            bypagerecord::column_info ci{};
            ci.type_size     = static_cast<uint8_t>(e.typeSize);
            ci.array_length  = static_cast<uint8_t>(e.arrayLength);
            ci.bytes_per_row = static_cast<uint16_t>(e.typeSize * e.arrayLength);
            cols.push_back(ci);
        }
        rec->bank_columns_.push_back(std::move(cols));

        stream_cursor += ncols * rec->num_pages_per_column_;
        rec->bank_stream_offset_.push_back(stream_cursor);
    }
    const uint32_t expected_streams = stream_cursor;

    // Zero-copy: presence + event_row_counts point straight into raw_ptr_.
    // Both are LE arrays in the on-disk format; we never need to rewrite,
    // only read. raw_ptr_ stays valid for the bypagerecord's lifetime.
    // (Suppress unused-variable for presence_bytes.)
    (void)presence_bytes;
    rec->bytes_per_row_         = bytes_per_row;
    rec->presence_ptr_          = reinterpret_cast<const uint8_t*>(p + presence_off);
    rec->event_row_counts_ptr_  = reinterpret_cast<const uint32_t*>(p + rowcounts_off);

    // Per-bank row-count prefix sums are DEFERRED until first column
    // access for each bank (see ensure_row_count_prefix). For records
    // where the analysis touches few/no banks, we skip the O(B*E) sum
    // entirely. On rec_clas this is ~300 KB of writes per record avoided.
    rec->row_count_prefix_data_.assign(num_banks, {});
    rec->row_count_prefix_flags_ = std::vector<std::once_flag>(num_banks);

    // Stream directory: read total_streams + T descriptors.
    const uint32_t total_streams = read_u32_le(p, streams_hdr_off);
    if (total_streams != expected_streams) {
        throw hipo::record_error(
            "bypagerecord: total_streams mismatch (expected " + std::to_string(expected_streams) +
            ", got " + std::to_string(total_streams) + ")");
    }
    const size_t stream_desc_off   = streams_hdr_off + 4;
    const size_t stream_desc_bytes = static_cast<size_t>(20) * total_streams;
    const size_t streams_off       = stream_desc_off + stream_desc_bytes;
    if (static_cast<size_t>(section_bytes) < streams_off) {
        throw hipo::record_error("bypagerecord: stream descriptor table truncated");
    }

    rec->stream_comp_sizes_.resize(total_streams);
    rec->stream_decomp_sizes_.resize(total_streams);
    rec->stream_offsets_.resize(total_streams);

    size_t cur_stream_off = streams_off;
    for (uint32_t s = 0; s < total_streams; ++s) {
        size_t off = stream_desc_off + s * 20;
        // We don't actually need bank_idx, col_idx, first_event, page_event_count
        // from the descriptor at runtime — they're implicit in the flat index.
        // We do verify the comp/decomp sizes are present.
        uint32_t comp   = read_u32_le(p, off + 12);
        uint32_t decomp = read_u32_le(p, off + 16);
        rec->stream_comp_sizes_[s]   = comp;
        rec->stream_decomp_sizes_[s] = decomp;
        rec->stream_offsets_[s]      = cur_stream_off;
        cur_stream_off += comp;
    }
    if (cur_stream_off > static_cast<size_t>(section_bytes)) {
        throw hipo::record_error("bypagerecord: stream payloads run past section end");
    }

    rec->stream_data_.resize(total_streams);
    rec->stream_flags_ = std::vector<std::once_flag>(total_streams);
    rec->stream_inflated_.assign(total_streams, 0);
}

int bypagerecord::bank_index(uint16_t group, uint8_t item) const {
    auto it = bank_index_map_.find((static_cast<uint32_t>(group) << 8) | item);
    return (it == bank_index_map_.end()) ? -1 : it->second;
}

bool bypagerecord::has(uint32_t event_idx, int bank_idx) const {
    const size_t byte = static_cast<size_t>(event_idx) * bytes_per_row_ + bank_idx / 8;
    const uint8_t bit = static_cast<uint8_t>(bank_idx % 8);
    return ((presence_ptr_[byte] >> bit) & 1u) != 0;
}

uint32_t bypagerecord::event_row_count(uint32_t event_idx, int bank_idx) const {
    return event_row_counts_ptr_[static_cast<size_t>(bank_idx) * event_count_ + event_idx];
}

const uint64_t* bypagerecord::ensure_row_count_prefix(int bank_idx) const {
    std::call_once(row_count_prefix_flags_[bank_idx], [this, bank_idx]() {
        auto& prefix = row_count_prefix_data_[bank_idx];
        prefix.resize(event_count_ + 1);
        const uint32_t* src = event_row_counts_ptr_ + static_cast<size_t>(bank_idx) * event_count_;
        uint64_t cum = 0;
        prefix[0] = 0;
        for (uint32_t e = 0; e < event_count_; ++e) {
            cum += src[e];
            prefix[e + 1] = cum;
        }
    });
    return row_count_prefix_data_[bank_idx].data();
}

int bypagerecord::stream_index(int bank_idx, int col_idx, int page_idx) const {
    return static_cast<int>(bank_stream_offset_[bank_idx])
         + col_idx * static_cast<int>(num_pages_per_column_)
         + page_idx;
}

const std::vector<char>& bypagerecord::stream(int bank_idx, int col_idx, int page_idx) {
    const int s = stream_index(bank_idx, col_idx, page_idx);
    std::call_once(stream_flags_[s], [this, s]() {
        const uint32_t expected = stream_decomp_sizes_[s];
        auto& dst = stream_data_[s];
        if (expected == 0) {
            stream_inflated_[s] = 1;
            return;
        }
        inflate_counter().fetch_add(1, std::memory_order_relaxed);
#ifdef __LZ4__
        dst.resize(expected);
        int r = LZ4_decompress_safe(raw_ptr_ + stream_offsets_[s], dst.data(),
                                    static_cast<int>(stream_comp_sizes_[s]),
                                    static_cast<int>(expected));
        if (r != static_cast<int>(expected)) {
            throw hipo::record_error(
                "bypagerecord: LZ4 decompress failed for stream " + std::to_string(s));
        }
        stream_inflated_[s] = 1;
#else
        throw hipo::record_error("bypagerecord: LZ4 not available");
#endif
    });
    return stream_data_[s];
}

bypagerecord::event_column_view
bypagerecord::column_for_event(uint32_t event_idx, int bank_idx, int col_idx) {
    const int page_idx       = static_cast<int>(event_idx / events_per_page_);
    const uint32_t page_first = static_cast<uint32_t>(page_idx) * events_per_page_;
    const auto& blob          = stream(bank_idx, col_idx, page_idx);  // lazy inflate
    const auto bytes_per_row  = bank_columns_[bank_idx][col_idx].bytes_per_row;
    // Lazy per-bank prefix sum: paid only on first column access of this
    // bank in this record. Subsequent accesses are O(1) loads.
    const uint64_t* prefix    = ensure_row_count_prefix(bank_idx);
    const uint64_t row_start  = prefix[page_first];
    const uint64_t row_event  = prefix[event_idx];
    const uint64_t row_count  = event_row_counts_ptr_[
        static_cast<size_t>(bank_idx) * event_count_ + event_idx];
    const uint64_t byte_off   = (row_event - row_start) * bytes_per_row;
    const uint64_t byte_len   = row_count * bytes_per_row;
    return event_column_view{ blob.data() + byte_off,
                              static_cast<uint32_t>(byte_len),
                              static_cast<uint32_t>(row_count) };
}

bool bypagerecord::is_stream_inflated(int bank_idx, int col_idx, int page_idx) const {
    return stream_inflated_[stream_index(bank_idx, col_idx, page_idx)] != 0;
}

} // namespace hipo
