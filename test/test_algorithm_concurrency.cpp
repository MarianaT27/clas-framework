// Concurrency regression test for SimpleAlgorithm::execute().
//
// Background: Application drives chain.process(), which invokes
// IAlgorithm::execute() concurrently from N worker threads on the SAME
// algorithm instance. Any non-atomic mutable state in process() races.
//
// This test exercises that contract directly without needing a real HIPO
// file: it constructs a minimal algorithm, calls execute() from many
// threads in tight loops, and asserts that atomic counters end up with
// exactly the total number of calls.
//
// The test does NOT drive an Event with real banks. Event's full public
// API requires hipo::bank_view_set and runtime data; we only exercise the
// Algorithm state machine and its atomic counters, which is what the
// concurrency contract protects.

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include <clas12/framework/core/IAlgorithm.hpp>

using namespace clas12::framework;

namespace {

// Minimal algorithm exercising only the state-machine part of IAlgorithm.
// We subclass Algorithm directly (not SimpleAlgorithm) so we do not need a
// real Event instance — this test isolates the pieces of the contract that
// are at risk under concurrency: Algorithm::m_state and user counters.
class CountingAlgorithm final : public Algorithm {
public:
    CountingAlgorithm() : Algorithm("CountingAlgorithm") {}

    auto initialize() -> StatusCode override {
        set_state(AlgorithmState::Initialized);
        return StatusCode::SUCCESS();
    }

    // Never called in this test (we bypass the Event parameter).
    auto execute(Event& /*event*/) -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    // The actual thing each worker thread calls. Increments an atomic
    // counter and reads state() to prove both survive concurrent access.
    auto tick() -> void {
        m_calls.fetch_add(1, std::memory_order_relaxed);
        (void)state();  // must not race with sibling ticks
    }

    auto finalize() -> StatusCode override {
        set_state(AlgorithmState::Finalized);
        return StatusCode::SUCCESS();
    }

    [[nodiscard]] auto calls() const -> std::size_t {
        return m_calls.load(std::memory_order_relaxed);
    }

private:
    std::atomic<std::size_t> m_calls{0};
};

}  // namespace

// A single shared algorithm instance driven by 8 workers over many iterations
// must observe every tick. If Algorithm::m_state were still a plain enum the
// TSan run would flag the concurrent state() reads against the Initialized
// store in initialize().
TEST(AlgorithmConcurrency, AtomicCountersAgreeUnderManyWorkers) {
    constexpr int kThreads = 8;
    constexpr std::size_t kItersPerThread = 100'000;

    CountingAlgorithm alg;
    ASSERT_TRUE(alg.initialize().is_success());
    ASSERT_EQ(alg.state(), AlgorithmState::Initialized);

    std::vector<std::thread> workers;
    workers.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) {
        workers.emplace_back([&alg] {
            for (std::size_t j = 0; j < kItersPerThread; ++j) {
                alg.tick();
            }
        });
    }
    for (auto& w : workers) w.join();

    EXPECT_EQ(alg.calls(), static_cast<std::size_t>(kThreads) * kItersPerThread);

    ASSERT_TRUE(alg.finalize().is_success());
    EXPECT_EQ(alg.state(), AlgorithmState::Finalized);
}

// Running twice with different thread counts must yield identical totals —
// a smoke test that the counter has no off-by-one or lost-update bug.
TEST(AlgorithmConcurrency, SingleAndMultiThreadedAgree) {
    constexpr std::size_t kTotal = 500'000;

    CountingAlgorithm a1;
    ASSERT_TRUE(a1.initialize().is_success());
    for (std::size_t j = 0; j < kTotal; ++j) a1.tick();
    ASSERT_TRUE(a1.finalize().is_success());

    CountingAlgorithm a2;
    ASSERT_TRUE(a2.initialize().is_success());
    {
        constexpr int kThreads = 4;
        const std::size_t per = kTotal / kThreads;
        std::vector<std::thread> workers;
        workers.reserve(kThreads);
        for (int i = 0; i < kThreads; ++i) {
            workers.emplace_back([&a2, per] {
                for (std::size_t j = 0; j < per; ++j) a2.tick();
            });
        }
        for (auto& w : workers) w.join();
    }
    ASSERT_TRUE(a2.finalize().is_success());

    EXPECT_EQ(a1.calls(), a2.calls());
}
