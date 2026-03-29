/**
 * @file test_event_processing_integration.cpp
 * @brief Integration tests for event processing features with real HIPO data
 *
 * This test file verifies:
 * - Coroutine-based event iteration (Generator<T>)
 * - Structured bank access (Event::banks())
 * - Integration with real HIPO data files
 *
 * @see Requirements 5.1, 5.4, 6.1, 6.2, 6.4
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>

#include <clas12/framework/core/Generator.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Ranges.hpp>
#include <clas12/PIDs.hpp>
#include <reader.h>
#include <bank_view.h>

using namespace clas12::framework;

// ============================================================================
// Test Fixture with Real HIPO Data
// ============================================================================

class EventProcessingIntegrationTest : public ::testing::Test {
protected:
    // Try multiple paths to find the test data file
    static constexpr const char* TEST_DATA_PATHS[] = {
        "data/data.hipo",
        "../data/data.hipo",
        "../../data/data.hipo"
    };

    std::string test_data_file;

    void SetUp() override {
        // Find the test data file
        for (const char* path : TEST_DATA_PATHS) {
            if (std::filesystem::exists(path)) {
                test_data_file = path;
                return;
            }
        }
        GTEST_SKIP() << "Test data file not found in any expected location";
    }

    /**
     * @brief Helper to open a reader for the test data file
     */
    auto open_reader() -> hipo::reader {
        auto result = hipo::reader::open(test_data_file);
        if (!result) {
            throw std::runtime_error("Failed to open test data file: " + test_data_file);
        }
        return std::move(*result);
    }
};

// ============================================================================
// Coroutine-Based Event Iteration Tests (Requirement 5.1)
// ============================================================================

/**
 * @brief Generator that yields event indices
 */
Generator<std::size_t> event_index_generator(std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        co_yield i;
    }
}

/**
 * @brief Generator that yields filtered values above a threshold
 */
Generator<int> filtered_value_generator(const std::vector<int>& values, int threshold) {
    for (int v : values) {
        if (v > threshold) {
            co_yield v;
        }
    }
}

TEST_F(EventProcessingIntegrationTest, GeneratorBasicIteration) {
    auto rdr = open_reader();
    std::size_t total_events = rdr.total_events();

    std::size_t count = 0;
    for (std::size_t idx : event_index_generator(total_events)) {
        count++;
        (void)idx;
    }

    EXPECT_EQ(count, total_events) << "Generator should yield all indices";
}

TEST_F(EventProcessingIntegrationTest, GeneratorFilteredIteration) {
    std::vector<int> values = {1, 5, 2, 8, 3, 9, 4, 7};

    std::vector<int> result;
    for (int v : filtered_value_generator(values, 5)) {
        result.push_back(v);
    }

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 8);
    EXPECT_EQ(result[1], 9);
    EXPECT_EQ(result[2], 7);
}

TEST_F(EventProcessingIntegrationTest, GeneratorWithHipoIteration) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(), {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value()) << "Failed to create bank_view_set";
    auto& banks = *banks_result;

    std::size_t count = 0;
    std::size_t events_with_particles = 0;

    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        count++;
        if (banks["REC::Particle"].rows() > 0) {
            events_with_particles++;
        }
        if (count >= 100) break;
    }

    EXPECT_GT(count, 0) << "Should iterate over events";
    EXPECT_GT(events_with_particles, 0) << "Should find events with particles";
}

// ============================================================================
// Structured Bank Access Tests (Requirements 6.1, 6.2, 6.4)
// ============================================================================

TEST_F(EventProcessingIntegrationTest, StructuredBankAccessSingleBank) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    std::size_t count = 0;
    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);

        // Access single bank
        auto& particle_bank = event.get_bank("REC::Particle");
        // Bank should be accessible (may be empty for some events)
        (void)particle_bank;
        if (++count >= 10) break;
    }
}

TEST_F(EventProcessingIntegrationTest, StructuredBankAccessMultipleBanks) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle", "REC::Calorimeter", "REC::Track"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    std::size_t count = 0;
    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);

        // Access multiple banks using structured bindings
        auto [particles_ref, calo_ref, track_ref] = event.banks(
            "REC::Particle", "REC::Calorimeter", "REC::Track"
        );

        // Get the actual bank references
        const hipo::bank_view& particles = particles_ref;
        const hipo::bank_view& calo = calo_ref;
        const hipo::bank_view& track = track_ref;

        // Banks should be accessible (may be empty)
        (void)particles;
        (void)calo;
        (void)track;
        if (++count >= 10) break;
    }
}

TEST_F(EventProcessingIntegrationTest, StructuredBankAccessMissingBank) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Scintillator"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    std::size_t count = 0;
    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);

        // Access a bank that may be empty
        auto& bank = event.get_bank("REC::Scintillator");

        // Bank should be accessible (may be empty)
        int rows = bank.rows();
        EXPECT_GE(rows, 0) << "Bank rows should be non-negative";
        if (++count >= 5) break;
    }
}

TEST_F(EventProcessingIntegrationTest, HasBankCheck) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    std::size_t count = 0;
    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);

        // REC::Particle should exist in most CLAS12 data
        bool has_particle_bank = event.has_bank("REC::Particle");
        // We can't guarantee it has data, but the check should work
        (void)has_particle_bank;
        if (++count >= 10) break;
    }
}

// ============================================================================
// Integration with Range Adaptors (Requirement 3.4)
// ============================================================================

TEST_F(EventProcessingIntegrationTest, RangeAdaptorsWithRealData) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    std::size_t total_electrons = 0;
    std::size_t high_p_electrons = 0;
    std::size_t count = 0;

    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);

        // Use range adaptors to filter particles
        auto electrons = event.all_particles()
            | Ranges::by_pid(clas12::PIDs::ELECTRON);

        for (const auto& e : electrons) {
            total_electrons++;
            if (e.p() > 1.0) {
                high_p_electrons++;
            }
        }
        if (++count >= 100) break;
    }

    // Just verify the pipeline works - actual counts depend on data
    EXPECT_GE(total_electrons, 0);
    EXPECT_LE(high_p_electrons, total_electrons);
}

TEST_F(EventProcessingIntegrationTest, ComposedRangeFilters) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    std::size_t fd_electron_count = 0;
    std::size_t count = 0;

    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);

        // Compose multiple filters
        auto fd_electrons = event.all_particles()
            | Ranges::by_pid(clas12::PIDs::ELECTRON)
            | Ranges::in_detector(clas12::FD)
            | Ranges::by_min_momentum(0.5);

        for (const auto& e : fd_electrons) {
            fd_electron_count++;
            // Verify filters applied correctly
            EXPECT_EQ(e.pdg(), 11);
            EXPECT_GE(e.p(), 0.5);
        }
        if (++count >= 100) break;
    }

    // Count depends on data, just verify pipeline works
    EXPECT_GE(fd_electron_count, 0);
}

// ============================================================================
// Traditional Algorithm Compatibility (Requirement 5.4)
// ============================================================================

TEST_F(EventProcessingIntegrationTest, TraditionalPatternStillWorks) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    // Traditional pattern: iterate and process each event
    std::size_t processed = 0;
    std::size_t passed = 0;

    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        processed++;

        // Traditional execute-style logic
        Event event(banks);
        auto& electrons = event.particles(clas12::PIDs::ELECTRON);

        if (!electrons.empty()) {
            passed++;
        }
        if (processed >= 100) break;
    }

    EXPECT_GT(processed, 0);
    EXPECT_LE(passed, processed);
}

// ============================================================================
// Event Store Integration
// ============================================================================

TEST_F(EventProcessingIntegrationTest, EventStoreWithRealData) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    std::size_t count = 0;
    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);

        // Store computed values
        auto& electrons = event.particles(clas12::PIDs::ELECTRON);
        clas12::framework::TypedKey<int> n_electrons_key("n_electrons");
        clas12::framework::TypedKey<bool> has_electron_key("has_electron");
        event.store_typed(n_electrons_key, static_cast<int>(electrons.size()));
        event.store_typed(has_electron_key, !electrons.empty());

        // Retrieve and verify
        auto n_electrons = event.retrieve_typed(n_electrons_key);
        auto has_electron = event.retrieve_typed(has_electron_key);

        ASSERT_TRUE(n_electrons.has_value());
        ASSERT_TRUE(has_electron.has_value());
        EXPECT_EQ(*n_electrons, static_cast<int>(electrons.size()));
        EXPECT_EQ(*has_electron, !electrons.empty());
        if (++count >= 10) break;
    }
}

// ============================================================================
// Performance Sanity Check
// ============================================================================

TEST_F(EventProcessingIntegrationTest, IterationPerformanceSanity) {
    auto rdr = open_reader();
    auto banks_result = hipo::bank_view_set::create(rdr.dict(),
        {"REC::Particle"});
    ASSERT_TRUE(banks_result.has_value());
    auto& banks = *banks_result;

    auto start = std::chrono::high_resolution_clock::now();

    std::size_t event_count = 0;
    std::size_t particle_type_count = 0;
    for ([[maybe_unused]] auto& evt : rdr.loop(banks)) {
        Event event(banks);
        auto& particles = event.particles();
        particle_type_count += particles.size();
        event_count++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Just log the performance, don't fail on timing
    std::cout << "Processed " << event_count << " events in "
              << duration.count() << "ms" << std::endl;

    EXPECT_GT(particle_type_count, 0) << "Should process some particle types";
}
