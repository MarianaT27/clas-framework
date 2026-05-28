// SPDX-License-Identifier: BSD-3-Clause

#include "bybankrecord.h"

#include "constants.h"
#include "hipoexceptions.h"

#include <atomic>
#include <cstring>
#include <stdexcept>

#ifdef __LZ4__
#include <lz4.h>
#endif

namespace hipo {

std::atomic<uint64_t>& bybankrecord::inflate_counter() {
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
} // namespace

// (the shared directory parser is the friend function
// `hipo_parse_bybank_directory`, defined below)

std::shared_ptr<bybankrecord> bybankrecord::parse(const recordHeader_t& header,
                                                  const char* compressed,
                                                  int compressed_bytes,
                                                  int compressed_padding) {
    if (header.dataEndianness == 1) {
        throw hipo::record_error(
            "bybankrecord: big-endian Lz4ByBank records are not supported");
    }

    const int section_bytes = compressed_bytes - compressed_padding;
    if (section_bytes < 8) {
        throw hipo::record_error("bybankrecord: section truncated (header)");
    }

    auto rec = std::shared_ptr<bybankrecord>(new bybankrecord{});
    rec->hdr_ = header;

    // Copy variant: own a snapshot of the section.
    rec->raw_.assign(compressed, compressed + section_bytes);
    rec->raw_ptr_ = rec->raw_.data();
    hipo_parse_bybank_directory(rec.get(), rec->raw_ptr_, section_bytes);
    return rec;
}

std::shared_ptr<bybankrecord> bybankrecord::parse_take(recordHeader_t header,
                                                       std::vector<char>&& compressed_owned,
                                                       int compressed_bytes,
                                                       int compressed_padding) {
    if (header.dataEndianness == 1) {
        throw hipo::record_error(
            "bybankrecord: big-endian Lz4ByBank records are not supported");
    }
    const int section_bytes = compressed_bytes - compressed_padding;
    if (section_bytes < 8) {
        throw hipo::record_error("bybankrecord: section truncated (header)");
    }
    if (static_cast<int>(compressed_owned.size()) < compressed_bytes) {
        throw hipo::record_error("bybankrecord: take buffer smaller than compressed_bytes");
    }

    auto rec = std::shared_ptr<bybankrecord>(new bybankrecord{});
    rec->hdr_ = header;
    rec->raw_ = std::move(compressed_owned);
    rec->raw_ptr_ = rec->raw_.data();
    hipo_parse_bybank_directory(rec.get(), rec->raw_ptr_, section_bytes);
    return rec;
}

std::shared_ptr<bybankrecord> bybankrecord::parse_view(recordHeader_t header,
                                                       const char* compressed,
                                                       int compressed_bytes,
                                                       int compressed_padding) {
    if (header.dataEndianness == 1) {
        throw hipo::record_error(
            "bybankrecord: big-endian Lz4ByBank records are not supported");
    }
    const int section_bytes = compressed_bytes - compressed_padding;
    if (section_bytes < 8) {
        throw hipo::record_error("bybankrecord: section truncated (header)");
    }
    auto rec = std::shared_ptr<bybankrecord>(new bybankrecord{});
    rec->hdr_     = header;
    rec->raw_ptr_ = compressed;  // borrowed — caller guarantees lifetime
    hipo_parse_bybank_directory(rec.get(), rec->raw_ptr_, section_bytes);
    return rec;
}

std::shared_ptr<bybankrecord> bybankrecord::parse_take_raw(recordHeader_t header,
                                                            std::unique_ptr<char[]>&& buf,
                                                            int buf_capacity,
                                                            int compressed_bytes,
                                                            int compressed_padding) {
    if (header.dataEndianness == 1) {
        throw hipo::record_error(
            "bybankrecord: big-endian Lz4ByBank records are not supported");
    }
    const int section_bytes = compressed_bytes - compressed_padding;
    if (section_bytes < 8) {
        throw hipo::record_error("bybankrecord: section truncated (header)");
    }
    if (buf_capacity < compressed_bytes) {
        throw hipo::record_error("bybankrecord: take_raw buffer smaller than compressed_bytes");
    }
    auto rec = std::shared_ptr<bybankrecord>(new bybankrecord{});
    rec->hdr_     = header;
    rec->raw_owned_ = std::move(buf);
    rec->raw_ptr_   = rec->raw_owned_.get();
    hipo_parse_bybank_directory(rec.get(), rec->raw_ptr_, section_bytes);
    return rec;
}

void hipo_parse_bybank_directory(bybankrecord* rec, const char* p, int section_bytes) {
    const uint32_t num_banks   = read_u32_le(p, 0);
    const uint32_t event_count = read_u32_le(p, 4);
    if (event_count != static_cast<uint32_t>(rec->hdr_.numberOfEvents)) {
        throw hipo::record_error("bybankrecord: event_count mismatch with record header");
    }
    rec->event_count_ = event_count;

    // Compute directory offsets.
    const size_t desc_off          = 8;
    const size_t desc_bytes        = 4 * num_banks;
    const size_t comp_sizes_off    = desc_off + desc_bytes;
    const size_t comp_sizes_bytes  = 4 * num_banks;
    const size_t decomp_sizes_off  = comp_sizes_off + comp_sizes_bytes;
    const size_t decomp_sizes_bytes = 4 * num_banks;
    const size_t tags_off          = decomp_sizes_off + decomp_sizes_bytes;
    const size_t tags_bytes        = 4 * event_count;
    const size_t presence_off      = tags_off + tags_bytes;
    const size_t bytes_per_row     = (num_banks + 7) / 8;
    const size_t presence_bytes    = event_count * bytes_per_row;
    const size_t ebs_off           = presence_off + presence_bytes;
    const size_t ebs_bytes         = static_cast<size_t>(4) * num_banks * event_count;
    const size_t streams_off       = ebs_off + ebs_bytes;

    if (static_cast<size_t>(section_bytes) < streams_off) {
        throw hipo::record_error("bybankrecord: directory truncated");
    }

    rec->descriptors_.reserve(num_banks);
    rec->bank_index_map_.reserve(num_banks);
    for (uint32_t b = 0; b < num_banks; ++b) {
        size_t off = desc_off + b * 4;
        bank_descriptor_t d{};
        d.group     = read_u16_le(p, off);
        d.item      = static_cast<uint8_t>(p[off + 2]);
        d.data_type = static_cast<uint8_t>(p[off + 3]);
        rec->descriptors_.push_back(d);
        rec->bank_index_map_[(static_cast<uint32_t>(d.group) << 8) | d.item] = static_cast<int>(b);
    }

    std::vector<uint32_t> comp_sizes(num_banks);
    rec->decompressed_sizes_.resize(num_banks);
    for (uint32_t b = 0; b < num_banks; ++b) {
        comp_sizes[b]                 = read_u32_le(p, comp_sizes_off   + b * 4);
        rec->decompressed_sizes_[b]   = read_u32_le(p, decomp_sizes_off + b * 4);
    }

    rec->event_tags_.resize(event_count);
    for (uint32_t e = 0; e < event_count; ++e) {
        rec->event_tags_[e] = read_u32_le(p, tags_off + e * 4);
    }

    rec->bytes_per_row_ = bytes_per_row;
    rec->presence_.assign(p + presence_off, p + presence_off + presence_bytes);

    // Flat event_bank_sizes layout: [b * event_count + e].
    rec->event_bank_sizes_.resize(num_banks * event_count);
    for (uint32_t b = 0; b < num_banks; ++b) {
        const size_t row_off = ebs_off + b * 4 * event_count;
        for (uint32_t e = 0; e < event_count; ++e) {
            rec->event_bank_sizes_[b * event_count + e] = read_u32_le(p, row_off + e * 4);
        }
    }

    // Precomputed prefix sums: [b * (event_count + 1) + e] = byte offset
    // of event-e's data within bank-b's decompressed stream. O(B*E) once,
    // O(1) per lookup forever after.
    rec->bank_byte_prefix_.resize(num_banks * (event_count + 1));
    for (uint32_t b = 0; b < num_banks; ++b) {
        const size_t prefix_off = b * (event_count + 1);
        const size_t sizes_off  = b * event_count;
        uint32_t cum = 0;
        rec->bank_byte_prefix_[prefix_off + 0] = 0;
        for (uint32_t e = 0; e < event_count; ++e) {
            cum += rec->event_bank_sizes_[sizes_off + e];
            rec->bank_byte_prefix_[prefix_off + e + 1] = cum;
        }
    }

    rec->compressed_streams_.reserve(num_banks);
    size_t off = streams_off;
    for (uint32_t b = 0; b < num_banks; ++b) {
        const size_t cs = comp_sizes[b];
        rec->compressed_streams_.emplace_back(off, cs);
        off += cs;
    }
    if (off > static_cast<size_t>(section_bytes)) {
        throw hipo::record_error("bybankrecord: bank streams run past section end");
    }

    // raw_ is set up by the caller (parse copies, parse_take moves) before
    // dispatching to this helper.

    rec->bank_data_.resize(num_banks);
    rec->bank_init_flags_ = std::vector<std::once_flag>(num_banks);
    rec->bank_inflated_.assign(num_banks, 0);
}

uint32_t bybankrecord::event_tag(uint32_t event_idx) const {
    return event_tags_.at(event_idx);
}

int bybankrecord::bank_index(uint16_t group, uint8_t item) const {
    // O(1) lookup via the (group<<8 | item) → idx hash built at parse time.
    auto it = bank_index_map_.find((static_cast<uint32_t>(group) << 8) | item);
    return (it == bank_index_map_.end()) ? -1 : it->second;
}

bool bybankrecord::has(uint32_t event_idx, int bank_idx) const {
    const size_t byte = static_cast<size_t>(event_idx) * bytes_per_row_ + bank_idx / 8;
    const uint8_t bit = static_cast<uint8_t>(bank_idx % 8);
    return ((presence_[byte] >> bit) & 1u) != 0;
}

uint32_t bybankrecord::bank_size(uint32_t event_idx, int bank_idx) const {
    return event_bank_sizes_[static_cast<size_t>(bank_idx) * event_count_ + event_idx];
}

const std::vector<char>& bybankrecord::bank_stream(int bank_idx) {
    std::call_once(bank_init_flags_[bank_idx], [this, bank_idx]() {
        const auto& range = compressed_streams_[bank_idx];
        const uint32_t expected = decompressed_sizes_[bank_idx];
        auto& dst = bank_data_[bank_idx];
        if (expected == 0) {
            bank_inflated_[bank_idx] = 1;
            return;
        }
        inflate_counter().fetch_add(1, std::memory_order_relaxed);
#ifdef __LZ4__
        dst.resize(expected);
        int r = LZ4_decompress_safe(raw_ptr_ + range.first, dst.data(),
                                    static_cast<int>(range.second),
                                    static_cast<int>(expected));
        if (r != static_cast<int>(expected)) {
            throw hipo::record_error(
                "bybankrecord: LZ4 decompress failed for bank " + std::to_string(bank_idx));
        }
        bank_inflated_[bank_idx] = 1;
#else
        (void)range;
        throw hipo::record_error("bybankrecord: LZ4 not available");
#endif
    });
    return bank_data_[bank_idx];
}

std::pair<uint32_t, uint32_t> bybankrecord::bank_byte_range(uint32_t event_idx, int bank_idx) const {
    const size_t prefix_off = static_cast<size_t>(bank_idx) * (event_count_ + 1);
    const uint32_t start = bank_byte_prefix_[prefix_off + event_idx];
    const uint32_t end   = bank_byte_prefix_[prefix_off + event_idx + 1];
    return {start, end - start};
}

bool bybankrecord::is_bank_inflated(int bank_idx) const {
    return bank_inflated_[bank_idx] != 0;
}

} // namespace hipo
