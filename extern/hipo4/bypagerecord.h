// SPDX-License-Identifier: BSD-3-Clause
//
// Lz4ByPage record reader (compression tag 6).
//
// The on-disk record's compressed payload is split into one LZ4 stream
// per (bank, column, page) — RNTuple-style finer granularity than
// bybankrecord (which has one stream per bank). Pages partition events
// into fixed-size groups (events_per_page); each page of each column of
// each bank is independently compressed.
//
// Lazy inflate via std::call_once: streams the analysis never asks for
// stay compressed for the record's lifetime. Partial-column workloads
// (e.g. touching px,py,pz of REC::Particle) inflate just those streams.

#ifndef HIPO_BYPAGERECORD_H
#define HIPO_BYPAGERECORD_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bybankrecord.h"   // bank_descriptor_t
#include "dictionary.h"
#include "record.h"

namespace hipo {

class bypagerecord {
  public:
    // Three constructors mirroring bybankrecord. parse() copies the
    // compressed section, parse_take_raw() moves a unique_ptr<char[]>
    // for the writer→reader hot path, parse_view() borrows a pointer
    // (typically into an mmap region) for true zero-copy reads.
    static std::shared_ptr<bypagerecord> parse(const recordHeader_t& header,
                                               const char* compressed,
                                               int compressed_bytes,
                                               int compressed_padding,
                                               const hipo::dictionary& dict);

    static std::shared_ptr<bypagerecord> parse_take_raw(recordHeader_t header,
                                                        std::unique_ptr<char[]>&& buf,
                                                        int buf_capacity,
                                                        int compressed_bytes,
                                                        int compressed_padding,
                                                        const hipo::dictionary& dict);

    static std::shared_ptr<bypagerecord> parse_view(recordHeader_t header,
                                                    const char* compressed,
                                                    int compressed_bytes,
                                                    int compressed_padding,
                                                    const hipo::dictionary& dict);

    // Counter incremented on every first-touch LZ4 inflate (one per
    // unique (bank, col, page) stream). Tests use this to prove the
    // partial-column-decompression contract.
    static std::atomic<uint64_t>& inflate_counter();

    const recordHeader_t& header()      const { return hdr_; }
    uint32_t              event_count() const { return event_count_; }
    int                   num_banks()   const { return static_cast<int>(descriptors_.size()); }
    uint32_t              events_per_page() const { return events_per_page_; }
    int                   num_pages_per_column() const { return num_pages_per_column_; }
    int                   total_streams() const { return static_cast<int>(stream_comp_sizes_.size()); }

    // Returns -1 if (group, item) is absent from this record.
    int                   bank_index(uint16_t group, uint8_t item) const;
    bool                  has(uint32_t event_idx, int bank_idx) const;
    bank_descriptor_t     descriptor(int bank_idx) const { return descriptors_[bank_idx]; }
    int                   num_columns(int bank_idx) const { return static_cast<int>(num_columns_[bank_idx]); }
    uint32_t              event_row_count(uint32_t event_idx, int bank_idx) const;

    // Lazily inflate (or return cached) decompressed bytes for the
    // (bank_idx, col_idx, page_idx) stream. First call decompresses;
    // subsequent calls are lock-free. Thread-safe.
    const std::vector<char>& stream(int bank_idx, int col_idx, int page_idx);

    // O(1) view of event_idx's data for (bank_idx, col_idx):
    // returns the page's decompressed bytes, the byte range covered by
    // this single event within the page, and the event's row count.
    // Caller is responsible for has(event, bank) first.
    struct event_column_view {
        const char* data;       // pointer into the decompressed page
        uint32_t    byte_len;   // row_count * type_size * array_length
        uint32_t    row_count;
    };
    event_column_view column_for_event(uint32_t event_idx, int bank_idx, int col_idx);

    // For tests / instrumentation.
    bool is_stream_inflated(int bank_idx, int col_idx, int page_idx) const;

    // Per-column type metadata (resolved from the schema at parse time).
    struct column_info {
        uint8_t  type_size;     // 1/2/4/8
        uint8_t  array_length;  // 1 for scalar, N for /T#N arrays
        uint16_t bytes_per_row; // = type_size * array_length
    };
    column_info column(int bank_idx, int col_idx) const { return bank_columns_[bank_idx][col_idx]; }

    // Flat stream index = bank_stream_offset_[bank_idx] + col_idx * num_pages_per_column_ + page_idx
    int stream_index(int bank_idx, int col_idx, int page_idx) const;

    // Compute (or return cached) per-bank prefix sums of row counts for
    // bank_idx. Lazy: paid only when at least one column of this bank is
    // accessed. Returns a pointer to (event_count_ + 1) uint64_t entries
    // where entry[e] = sum of row_counts[bank_idx][0..e).
    const uint64_t* ensure_row_count_prefix(int bank_idx) const;

  private:
    bypagerecord() = default;
    bypagerecord(const bypagerecord&) = delete;
    bypagerecord& operator=(const bypagerecord&) = delete;

    recordHeader_t                    hdr_{};
    std::vector<bank_descriptor_t>    descriptors_;
    std::vector<uint32_t>             num_columns_;          // per bank
    std::vector<uint32_t>             bank_stream_offset_;   // [b] -> first stream idx
    uint32_t                          event_count_{0};
    uint32_t                          events_per_page_{0};
    uint32_t                          num_pages_per_column_{0};

    std::unordered_map<uint32_t, int> bank_index_map_;

    // Per-bank column metadata (type_size, array_length) from schema.
    std::vector<std::vector<column_info>> bank_columns_;

    // Presence + event_row_counts point straight into raw_ptr_ (zero copy).
    // Both are LE u8/u32 arrays in the on-disk format; we never need to
    // rewrite them, only read.
    const uint8_t*                    presence_ptr_{nullptr};
    size_t                            bytes_per_row_{0};
    const uint32_t*                   event_row_counts_ptr_{nullptr}; // [b * E + e]

    // Per-bank cumulative row count, computed lazily (on first column access
    // for that bank). row_count_prefix_data_[b] holds (E+1) cumulative
    // uint64_t entries; row_count_prefix_flags_[b] guards initialization.
    // Banks the analysis never touches NEVER trigger this O(E) prefix sum.
    mutable std::vector<std::vector<uint64_t>> row_count_prefix_data_;
    mutable std::vector<std::once_flag>        row_count_prefix_flags_;

    // Per-stream metadata.
    std::vector<uint32_t>             stream_comp_sizes_;
    std::vector<uint32_t>             stream_decomp_sizes_;
    std::vector<size_t>               stream_offsets_;       // into raw_ptr_

    // Lazy inflate cache.
    std::vector<std::vector<char>>    stream_data_;
    std::vector<std::once_flag>       stream_flags_;
    std::vector<char>                 stream_inflated_;       // 1 after inflate

    // Owned compressed-section bytes (one of these holds it; raw_ptr_ points
    // at whichever owns; nullptr-owned means parse_view borrowed mode).
    std::vector<char>                 raw_;
    std::unique_ptr<char[]>           raw_owned_;
    const char*                       raw_ptr_{nullptr};

    friend void hipo_parse_bypage_directory(bypagerecord*, const char*, int, const hipo::dictionary&);
};

void hipo_parse_bypage_directory(bypagerecord* rec, const char* p, int section_bytes,
                                 const hipo::dictionary& dict);

} // namespace hipo

#endif // HIPO_BYPAGERECORD_H
