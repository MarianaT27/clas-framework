#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include <clas12/framework/core/ThreadLocalHistogram.hpp>

using clas12::framework::ThreadLocalHistogram1D;
using clas12::framework::ThreadLocalHistogram2D;
using clas12::framework::ThreadLocalHistogram3D;

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

// =============================================================================
// ThreadLocalHistogram3D — basic fill + serialization to TH3D
// =============================================================================

TEST(ThreadLocalHistogram, Hist3DSingleThreadFill) {
    ThreadLocalHistogram3D h(10, 0.0, 10.0,   // x: 1-wide bins
                             20, 0.0, 2.0,    // y: 0.1-wide bins
                             4,  -2.0, 2.0);  // z: 1-wide bins

    // Fill bin (x∈[5,6), y∈[1.0,1.1), z∈[0,1)) five times.
    for (int i = 0; i < 5; ++i) h.fill(5.5, 1.05, 0.5);
    // And a single fill into bin (x∈[1,2), y∈[0,0.1), z∈[-1,0)).
    h.fill(1.5, 0.05, -0.5);
    // Underflow on z should be routed to bin 0, not dropped.
    h.fill(5.5, 1.05, -10.0);

    EXPECT_EQ(h.entries(), 7u);

    auto root_hist = h.to_root("h3_test", "h3;x;y;z");
    ASSERT_NE(root_hist, nullptr);
    EXPECT_EQ(root_hist->GetEntries(), 7.0);

    // The 5-fold-filled bin should have content 5.
    const int bx = root_hist->GetXaxis()->FindBin(5.5);
    const int by = root_hist->GetYaxis()->FindBin(1.05);
    const int bz = root_hist->GetZaxis()->FindBin(0.5);
    EXPECT_DOUBLE_EQ(root_hist->GetBinContent(bx, by, bz), 5.0);

    // The single-fill bin.
    const int bx2 = root_hist->GetXaxis()->FindBin(1.5);
    const int by2 = root_hist->GetYaxis()->FindBin(0.05);
    const int bz2 = root_hist->GetZaxis()->FindBin(-0.5);
    EXPECT_DOUBLE_EQ(root_hist->GetBinContent(bx2, by2, bz2), 1.0);

    // Z-underflow bin (z index 0) with the same x/y as the 5-count bin.
    EXPECT_DOUBLE_EQ(root_hist->GetBinContent(bx, by, 0), 1.0);
}

TEST(ThreadLocalHistogram, Hist3DContendedFillsMatchSerialReference) {
    constexpr int kNWorkers         = 8;
    constexpr int kNFillsPerWorker  = 20'000;

    auto fill_stream = [](auto& hist, int worker_seed) {
        std::mt19937 rng(static_cast<unsigned>(worker_seed) * 6619U);
        std::uniform_real_distribution<double> xd(0.0, 10.0);
        std::uniform_real_distribution<double> yd(0.0, 5.0);
        std::uniform_real_distribution<double> zd(-2.0, 2.0);
        for (int i = 0; i < kNFillsPerWorker; ++i) {
            hist.fill(xd(rng), yd(rng), zd(rng));
        }
    };

    ThreadLocalHistogram3D ref(10, 0, 10, 10, 0, 5, 8, -2, 2);
    for (int w = 0; w < kNWorkers; ++w) fill_stream(ref, w);

    ThreadLocalHistogram3D par(10, 0, 10, 10, 0, 5, 8, -2, 2);
    std::vector<std::thread> workers;
    workers.reserve(kNWorkers);
    for (int w = 0; w < kNWorkers; ++w) {
        workers.emplace_back([&par, w, &fill_stream]() { fill_stream(par, w); });
    }
    for (auto& t : workers) t.join();

    EXPECT_EQ(ref.entries(), par.entries());

    // The merge path into TH3D should preserve bin contents under contention.
    auto ref_th3 = ref.to_root("ref", "");
    auto par_th3 = par.to_root("par", "");
    for (int ix = 1; ix <= 10; ++ix) {
        for (int iy = 1; iy <= 10; ++iy) {
            for (int iz = 1; iz <= 8; ++iz) {
                EXPECT_DOUBLE_EQ(ref_th3->GetBinContent(ix, iy, iz),
                                 par_th3->GetBinContent(ix, iy, iz))
                    << "bin (" << ix << "," << iy << "," << iz << ")";
            }
        }
    }
}
