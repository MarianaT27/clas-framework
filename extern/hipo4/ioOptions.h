#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace hipo {

///
/// @brief Tunable I/O options for hipo::chain on remote/low-bandwidth storage.
///
/// Defaults preserve the legacy (pre-prefetch, pre-cache) behavior. Setting any
/// non-zero field opts in to the RNTuple-style read pipeline: a small pool of
/// dedicated I/O threads asynchronously fetches and decompresses records that
/// worker threads will need next, with a byte-bounded shared LRU on top.
///
/// For ifarm-style links (~5 ms RTT, ~1 Gbit/s), a good starting point is
///   prefetch_depth = 4, record_cache_bytes = 128 MB, io_threads = 2.
///
struct IOOptions {
    /// Number of records the prefetcher tries to keep "in flight" ahead of the
    /// consumers. 0 leaves prefetch disabled; values > 0 enable it.
    int prefetch_depth = 0;

    /// Byte ceiling on the shared decompressed-record LRU cache. 0 disables it.
    /// Counts decompressed sizes. A value of 0 also disables prefetch
    /// retention (records still flow through but aren't kept).
    std::size_t record_cache_bytes = 0;

    /// Worker-thread count for the I/O pool. 0 lets the chain pick: 1 for
    /// local file:// URLs, 2 for remote (root://, http://, https://, davix://).
    int io_threads = 0;

    /// Optional whitelist of bank names. When non-empty, event/bank parsing
    /// short-circuits for unlisted banks. Reduces CPU on the consumer side;
    /// has no effect on network bytes (records bundle every bank on disk).
    std::vector<std::string> bank_filter;

    /// Speculative coalesced read size when fetching a record's header+data
    /// in a single syscall. 0 lets the backend pick a sensible default
    /// (16 KB local, 256 KB remote).
    int network_read_chunk = 0;
};

} // namespace hipo
