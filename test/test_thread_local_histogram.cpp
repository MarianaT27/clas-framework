#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include <clas12/framework/core/ThreadLocalHistogram.hpp>

using clas12::framework::ThreadLocalHistogram1D;
using clas12::framework::ThreadLocalHistogram2D;

// =============================================================================
// Correctness under contention
// =============================================================================
//
// Multiple threads fill the same histogram. The final per-bin sum and
// the entry count must match a single-threaded reference.

TEST(ThreadLocalHistogram, ContendedFillsMatchSerialReference) {
    constexpr int    kNBins  = 50;
    constexpr double kXmin   = 0.0;
    constexpr double kXmax   = 50.0;
    constexpr int    kNWorkers = 8;
    constexpr int    kNFillsPerWorker = 50'000;

    // Reference: single-threaded fills of the same sequence.
    ThreadLocalHistogram1D ref(kNBins, kXmin, kXmax);
    for (int w = 0; w < kNWorkers; ++w) {
        std::mt19937 rng(static_cast<unsigned>(w) * 9973U);
        std::uniform_real_distribution<double> dist(kXmin, kXmax);
        for (int i = 0; i < kNFillsPerWorker; ++i) ref.fill(dist(rng));
    }

    // Parallel: N worker threads fill the same histogram concurrently.
    ThreadLocalHistogram1D par(kNBins, kXmin, kXmax);
    std::vector<std::thread> workers;
    workers.reserve(kNWorkers);
    for (int w = 0; w < kNWorkers; ++w) {
        workers.emplace_back([&par, w]() {
            std::mt19937 rng(static_cast<unsigned>(w) * 9973U);
            std::uniform_real_distribution<double> dist(kXmin, kXmax);
            for (int i = 0; i < kNFillsPerWorker; ++i) par.fill(dist(rng));
        });
    }
    for (auto& t : workers) t.join();

    EXPECT_EQ(ref.entries(), par.entries());
    for (int i = 0; i <= kNBins + 1; ++i) {
        EXPECT_DOUBLE_EQ(ref.get_bin_content(i), par.get_bin_content(i))
            << "bin " << i;
    }
}

// =============================================================================
// Concurrent weighted fills stay consistent
// =============================================================================

TEST(ThreadLocalHistogram, WeightedContendedFills) {
    constexpr int kNWorkers = 4;
    constexpr int kNFills   = 10'000;

    ThreadLocalHistogram1D h(10, 0.0, 10.0);
    std::vector<std::thread> workers;
    workers.reserve(kNWorkers);
    for (int w = 0; w < kNWorkers; ++w) {
        workers.emplace_back([&h]() {
            for (int i = 0; i < kNFills; ++i) {
                h.fill(3.5, 0.5);  // deterministic bin + weight
            }
        });
    }
    for (auto& t : workers) t.join();

    EXPECT_EQ(h.entries(),
              static_cast<std::size_t>(kNWorkers) * static_cast<std::size_t>(kNFills));
    // 3.5 falls in bin 4 (1-indexed, bin width 1.0). Content = 0.5 * total fills.
    EXPECT_DOUBLE_EQ(h.get_bin_content(4),
                     0.5 * static_cast<double>(kNWorkers) * static_cast<double>(kNFills));
}

// =============================================================================
// Accumulator<T> concept: merge two histograms
// =============================================================================

TEST(ThreadLocalHistogram, MergeFoldsAcrossHistograms) {
    ThreadLocalHistogram1D a(5, 0.0, 5.0);
    ThreadLocalHistogram1D b(5, 0.0, 5.0);

    a.fill(0.5);
    a.fill(1.5);
    b.fill(2.5);
    b.fill(3.5);
    b.fill(4.5);

    a.merge(b);

    EXPECT_EQ(a.entries(), 5U);
    EXPECT_DOUBLE_EQ(a.get_bin_content(1), 1.0);  // 0.5 -> bin 1
    EXPECT_DOUBLE_EQ(a.get_bin_content(2), 1.0);  // 1.5 -> bin 2
    EXPECT_DOUBLE_EQ(a.get_bin_content(3), 1.0);  // 2.5 -> bin 3
    EXPECT_DOUBLE_EQ(a.get_bin_content(4), 1.0);  // 3.5 -> bin 4
    EXPECT_DOUBLE_EQ(a.get_bin_content(5), 1.0);  // 4.5 -> bin 5
}

TEST(ThreadLocalHistogram, MergeIgnoresIncompatibleBinning) {
    ThreadLocalHistogram1D a(5, 0.0, 5.0);
    ThreadLocalHistogram1D b(10, 0.0, 5.0);  // different nbins

    a.fill(2.5);
    b.fill(2.5);
    a.merge(b);

    // b's fill silently dropped; only a's original 1 entry remains.
    EXPECT_EQ(a.entries(), 1U);
}

// =============================================================================
// 2D concurrent fills
// =============================================================================

TEST(ThreadLocalHistogram2D, ContendedFills) {
    constexpr int kNWorkers = 4;
    constexpr int kNFills   = 10'000;

    ThreadLocalHistogram2D h(5, 0.0, 5.0, 5, 0.0, 5.0);
    std::vector<std::thread> workers;
    workers.reserve(kNWorkers);
    for (int w = 0; w < kNWorkers; ++w) {
        workers.emplace_back([&h]() {
            for (int i = 0; i < kNFills; ++i) h.fill(2.5, 2.5);
        });
    }
    for (auto& t : workers) t.join();

    EXPECT_EQ(h.entries(),
              static_cast<std::size_t>(kNWorkers) * static_cast<std::size_t>(kNFills));
    EXPECT_DOUBLE_EQ(h.get_bin_content(3, 3),
                     static_cast<double>(kNWorkers) * static_cast<double>(kNFills));
}

// =============================================================================
// Throughput smoke test (not a hard perf assertion - just sanity)
// =============================================================================
//
// Reports fills/sec under contention so regressions show up in the test
// log. No throughput threshold is asserted because CI machines vary wildly;
// the correctness tests above are the real gate.

TEST(ThreadLocalHistogram, ThroughputSmoke) {
    constexpr int kNWorkers = 4;
    constexpr int kNFillsPerWorker = 200'000;

    ThreadLocalHistogram1D h(100, 0.0, 100.0);
    const auto t0 = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;
    workers.reserve(kNWorkers);
    for (int w = 0; w < kNWorkers; ++w) {
        workers.emplace_back([&h, w]() {
            std::mt19937 rng(static_cast<unsigned>(w) * 7919U);
            std::uniform_real_distribution<double> dist(0.0, 100.0);
            for (int i = 0; i < kNFillsPerWorker; ++i) h.fill(dist(rng));
        });
    }
    for (auto& t : workers) t.join();

    const auto t1 = std::chrono::steady_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    const auto total_fills = static_cast<long>(kNWorkers) * kNFillsPerWorker;

    ::testing::Test::RecordProperty("total_fills", std::to_string(total_fills));
    ::testing::Test::RecordProperty("elapsed_ms", std::to_string(ms));

    EXPECT_EQ(h.entries(), static_cast<std::size_t>(total_fills));
}
