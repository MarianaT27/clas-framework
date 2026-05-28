// SPDX-License-Identifier: BSD-3-Clause
//
// Lz4ByBank record reader (compression tag 5).
//
// The on-disk record's compressed payload is split into one LZ4 stream
// per bank type (descriptors are kept in a directory at the start). This
// reader parses the directory eagerly (cheap) and inflates each bank's
// stream lazily — banks the analysis never touches stay compressed for
// the record's lifetime.
//
// Wire-compatible with the hipo-rs port (`crates/hipo/src/wire/by_bank.rs`).
//
// Improvements over hipo-rs:
//   - `event_bank_sizes` stored as a flat vector indexed by `b * E + e`
//     (one allocation instead of B separate vectors).
//   - `bank_byte_prefix` is a precomputed prefix-sum table so
//     `bank_byte_range(event_idx, bank_idx)` is O(1) instead of O(E).

#ifndef HIPO_BYBANKRECORD_H
#define HIPO_BYBANKRECORD_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "record.h"

namespace hipo {

struct bank_descriptor_t {
    uint16_t group;
    uint8_t  item;
    uint8_t  data_type;
};

class bybankrecord {
  public:
    // Parse an on-disk record (header + compressed payload section already
    // sitting in `compressed`, length `compressed_bytes` including any
    // trailing alignment padding that the caller has *not yet* stripped).
    // The directory is parsed eagerly; no bank stream is decompressed.
    // This variant copies `compressed` into the bybankrecord's owned
    // buffer — use `parse_take` to avoid the copy on the hot reader path.
    static std::shared_ptr<bybankrecord> parse(const recordHeader_t& header,
                                               const char* compressed,
                                               int compressed_bytes,
                                               int compressed_padding);

    // Same as parse() but moves `compressed_owned` into the bybankrecord's
    // raw_ buffer (no copy). The caller's vector is left empty. This is
    // the hot path used by the reader to skip a 5-10 MB memcpy per record.
    static std::shared_ptr<bybankrecord> parse_take(recordHeader_t header,
                                                    std::vector<char>&& compressed_owned,
                                                    int compressed_bytes,
                                                    int compressed_padding);

    // Hot-path variant that takes ownership of a raw uninitialized buffer
    // (allocated with `new char[N]`). Avoids the std::vector::resize zero
    // init that would otherwise burn 500–700 µs per multi-MB record.
    static std::shared_ptr<bybankrecord> parse_take_raw(recordHeader_t header,
                                                        std::unique_ptr<char[]>&& buf,
                                                        int buf_capacity,
                                                        int compressed_bytes,
                                                        int compressed_padding);

    // Zero-copy variant: the bybankrecord holds a pointer into an
    // externally-owned buffer (typically an mmap'd file) and never copies
    // the section. The caller MUST guarantee the bytes outlive every
    // shared_ptr<bybankrecord>. Used by the reader's mmap-backed fast path
    // to break through the read() copy ceiling.
    static std::shared_ptr<bybankrecord> parse_view(recordHeader_t header,
                                                    const char* compressed,
                                                    int compressed_bytes,
                                                    int compressed_padding);

    // Counter incremented on every first-touch LZ4 inflate. Used by tests
    // to prove the partial-decompression contract; harmless otherwise.
    static std::atomic<uint64_t>& inflate_counter();

    const recordHeader_t& header() const { return hdr_; }
    uint32_t              event_count() const { return event_count_; }
    int                   num_banks() const { return static_cast<int>(descriptors_.size()); }
    uint32_t              event_tag(uint32_t event_idx) const;

    // Returns -1 if (group, item) is absent from this record.
    int      bank_index(uint16_t group, uint8_t item) const;

    // True iff event `event_idx` carries bank index `bank_idx`.
    bool     has(uint32_t event_idx, int bank_idx) const;

    // (group, item, data_type) of bank `bank_idx`.
    bank_descriptor_t descriptor(int bank_idx) const { return descriptors_[bank_idx]; }

    // Decompressed byte count of bank `bank_idx`'s event-`event_idx` slice.
    uint32_t bank_size(uint32_t event_idx, int bank_idx) const;

    // Lazily inflate (or return cached) decompressed stream for bank `bank_idx`.
    // First call triggers LZ4 decompression under std::call_once; subsequent
    // calls are lock-free reads. Thread-safe across many readers sharing one
    // shared_ptr<bybankrecord>.
    const std::vector<char>& bank_stream(int bank_idx);

    // O(1) byte range [start, start+len) of event `event_idx`'s data within
    // `bank_stream(bank_idx)`. Caller is responsible for checking `has` first.
    std::pair<uint32_t, uint32_t> bank_byte_range(uint32_t event_idx, int bank_idx) const;

    // True iff bank `bank_idx`'s stream has been decompressed already.
    // (For tests / instrumentation.)
    bool is_bank_inflated(int bank_idx) const;

  private:
    bybankrecord() = default;
    bybankrecord(const bybankrecord&) = delete;
    bybankrecord& operator=(const bybankrecord&) = delete;

    recordHeader_t                  hdr_{};
    std::vector<bank_descriptor_t>  descriptors_;
    uint32_t                        event_count_{0};
    std::vector<uint32_t>           event_tags_;
    std::vector<uint8_t>            presence_;      // bytes_per_row * event_count
    size_t                          bytes_per_row_{0};

    // Flat layout: event_bank_sizes_[b * event_count_ + e]
    std::vector<uint32_t>           event_bank_sizes_;
    // Precomputed prefix sums: bank_byte_prefix_[b * (event_count_ + 1) + e]
    // gives the byte offset within bank-b's decompressed stream where event-e
    // starts. (improvement over hipo-rs which does an O(E) scan per call)
    std::vector<uint32_t>           bank_byte_prefix_;

    // Owned copy of the compressed-payload section (pre-stripped of any
    // trailing alignment padding). Held for the record's lifetime so we
    // can decompress bank streams on demand after the source mmap goes away.
    // Owned compressed-section bytes. For parse() this is a vector with
    // its data() pointer; for parse_take_raw() this is a unique_ptr<char[]>.
    // We always read through raw_ptr_ (set to whichever owns the bytes).
    std::vector<char>                 raw_;
    std::unique_ptr<char[]>           raw_owned_;
    const char*                       raw_ptr_{nullptr};
    std::vector<std::pair<size_t, size_t>> compressed_streams_; // (off, len) into raw_ptr_
    std::vector<uint32_t>           decompressed_sizes_;

    // Lazy per-bank inflate cache. Sized at parse(); never resized.
    std::vector<std::vector<char>>  bank_data_;
    std::vector<std::once_flag>     bank_init_flags_;
    // Parallel "has this bank been inflated yet?" flags. Each byte is
    // independently addressable, so this is safe to read/write across
    // workers (one writer per bank, set inside the once_flag-guarded
    // closure). Used by is_bank_inflated() for tests/instrumentation.
    std::vector<char>               bank_inflated_;

    // Direct-index hash for bank_index() lookups. Keyed by (group<<8 | item).
    // Built once at parse, O(1) hot-path lookup vs the prior O(B) linear scan.
    std::unordered_map<uint32_t, int> bank_index_map_;

    // Internal: shared directory parser used by both parse() and parse_take().
    friend void hipo_parse_bybank_directory(bybankrecord*, const char*, int);
};

void hipo_parse_bybank_directory(bybankrecord* rec, const char* p, int section_bytes);

} // namespace hipo

#endif // HIPO_BYBANKRECORD_H
