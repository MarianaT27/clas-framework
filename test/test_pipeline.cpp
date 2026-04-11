#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include <bank_view.h>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Pipeline.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>

using clas12::Particle;
using clas12::framework::Cutflow;
using clas12::framework::Event;
using clas12::framework::EventPipeline;
using clas12::framework::ParticlePipeline;
using clas12::framework::ThreadLocalHistogram1D;
using clas12::framework::particles;
using clas12::framework::pipeline;

namespace {

// Minimal Particle factory for tests. We only care about a handful
// of getters the pipeline's filter/tap lambdas touch: E(), beta(),
// charge(), index().
auto make_particle(int index, int charge, double mass,
                   double px, double py, double pz,
                   double E, double beta) -> Particle {
    return Particle(/*pdg*/   0,
                    /*status*/0,
                    /*index*/ index,
                    /*charge*/charge,
                    /*mass*/  mass,
                    /*px*/    px,
                    /*py*/    py,
                    /*pz*/    pz,
                    /*E*/     E,
                    /*vx*/    0.0,
                    /*vy*/    0.0,
                    /*vz*/    0.0,
                    /*vt*/    0.0,
                    /*beta*/  beta,
                    /*chi2pid*/0.0);
}

}  // namespace

// =============================================================================
// ParticlePipeline
// =============================================================================

TEST(ParticlePipeline, CountBumpsCutflowPerInputParticle) {
    std::vector<Particle> input{
        make_particle(0, 0, 0.0, 0, 0, 1.0, 1.0, 1.0),
        make_particle(1, 0, 0.0, 0, 0, 2.0, 2.0, 1.0),
        make_particle(2, 0, 0.0, 0, 0, 3.0, 3.0, 1.0),
    };

    Cutflow cf("test");
    cf.add_cut("All");

    auto n = particles(input, cf)
        .count("All")
        .run();

    EXPECT_EQ(n, 3U);
    EXPECT_EQ(cf.count("All"), 3U);
}

TEST(ParticlePipeline, FilterDropsParticlesAndCountsSurvivors) {
    std::vector<Particle> input{
        make_particle(0, 0, 0.0, 0, 0, 1.0, 1.0, 1.0),
        make_particle(1, 0, 0.0, 0, 0, 2.0, 2.0, 1.0),
        make_particle(2, 0, 0.0, 0, 0, 3.0, 3.0, 1.0),
        make_particle(3, 0, 0.0, 0, 0, 4.0, 4.0, 1.0),
    };

    Cutflow cf("test");
    cf.add_cut("All");
    cf.add_cut("E > 2.5");

    auto survivors = particles(input, cf)
        .count("All")
        .filter("E > 2.5", [](const Particle& p) { return p.E() > 2.5; })
        .collect();

    EXPECT_EQ(survivors.size(), 2U);
    EXPECT_EQ(cf.count("All"), 4U);
    EXPECT_EQ(cf.count("E > 2.5"), 2U);
}

TEST(ParticlePipeline, ChainedFiltersIncrementEachCutflowIndependently) {
    std::vector<Particle> input{
        make_particle(0, +1, 0.0, 0, 0, 1.0, 1.0, 0.95),
        make_particle(1,  0, 0.0, 0, 0, 2.0, 2.0, 1.00),
        make_particle(2, +1, 0.0, 0, 0, 3.0, 3.0, 0.50),  // bad beta
        make_particle(3, +1, 0.0, 0, 0, 4.0, 4.0, 1.00),
    };

    Cutflow cf("test");
    cf.add_cut("Charged");
    cf.add_cut("Good Beta");

    auto selected = particles(input, cf)
        .filter("Charged",  [](const Particle& p) { return p.charge() != 0; })
        .filter("Good Beta", [](const Particle& p) { return p.beta() > 0.9; })
        .collect();

    EXPECT_EQ(selected.size(), 2U);
    EXPECT_EQ(cf.count("Charged"), 3U);    // indices 0, 2, 3 pass charge
    EXPECT_EQ(cf.count("Good Beta"), 2U);  // indices 0 and 3 pass beta
}

TEST(ParticlePipeline, TapRunsPerSurvivingParticle) {
    std::vector<Particle> input{
        make_particle(0, 0, 0.0, 0, 0, 1.0, 1.0, 1.0),
        make_particle(1, 0, 0.0, 0, 0, 2.0, 2.0, 1.0),
        make_particle(2, 0, 0.0, 0, 0, 3.0, 3.0, 1.0),
    };

    Cutflow cf("test");
    cf.add_cut("E > 1.5");

    ThreadLocalHistogram1D h(10, 0.0, 10.0);

    particles(input, cf)
        .filter("E > 1.5", [](const Particle& p) { return p.E() > 1.5; })
        .tap([&](const Particle& p) { h.fill(p.E()); })
        .run();

    EXPECT_EQ(h.entries(), 2U);
    EXPECT_EQ(cf.count("E > 1.5"), 2U);
}

TEST(ParticlePipeline, AnyShortCircuitsAtFirstSurvivor) {
    std::vector<Particle> input{
        make_particle(0, 0, 0.0, 0, 0, 1.0, 1.0, 1.0),
        make_particle(1, 0, 0.0, 0, 0, 2.0, 2.0, 1.0),
    };

    Cutflow cf("test");
    cf.add_cut("always");

    auto ok = particles(input, cf)
        .count("always")
        .any();

    EXPECT_TRUE(ok);
}

TEST(ParticlePipeline, EmptyInputYieldsEmptyResult) {
    std::vector<Particle> input;
    Cutflow cf("test");
    cf.add_cut("never");

    auto selected = particles(input, cf)
        .count("never")
        .collect();

    EXPECT_TRUE(selected.empty());
    EXPECT_EQ(cf.count("never"), 0U);
}

TEST(ParticlePipeline, WorksWithoutCutflow) {
    std::vector<Particle> input{
        make_particle(0, 0, 0.0, 0, 0, 1.0, 1.0, 1.0),
        make_particle(1, 0, 0.0, 0, 0, 2.0, 2.0, 1.0),
    };

    auto selected = clas12::framework::particles(input)
        .filter("any", [](const Particle& p) { return p.E() > 1.5; })
        .collect();

    EXPECT_EQ(selected.size(), 1U);
}

// =============================================================================
// EventPipeline::fill — variadic-args wrapper around std::invoke
// =============================================================================

namespace {

// Fresh Event per test. Default-constructed bank_view_set is empty but
// valid; the .fill tests never touch its banks.
class EventPipelineFillTest : public ::testing::Test {
   protected:
    hipo::bank_view_set m_banks;
    std::unique_ptr<Event> m_event = std::make_unique<Event>(m_banks);
};

struct FillCtx {
    int    id;
    double value;
};

// A toy histogram-like sink that records every fill call. Used to
// verify the pipeline calls `.fill` in the correct order and skips
// when the pipeline is filtered.
struct Recorder {
    std::vector<std::pair<std::string, double>> calls;

    auto push(std::string tag, double v) -> void { calls.emplace_back(std::move(tag), v); }

    auto push_member(const FillCtx& c) -> void {
        calls.emplace_back("member", c.value);
    }
};

}  // namespace

TEST_F(EventPipelineFillTest, FillInvokesLambda) {
    Recorder rec;
    auto sc = pipeline(*m_event)
        .fill([&] { rec.push("a", 1.0); })
        .fill([&] { rec.push("b", 2.0); })
        .status();

    EXPECT_TRUE(sc.is_success());
    ASSERT_EQ(rec.calls.size(), 2U);
    EXPECT_EQ(rec.calls[0].first, "a");
    EXPECT_DOUBLE_EQ(rec.calls[1].second, 2.0);
}

TEST_F(EventPipelineFillTest, FillForwardsArgumentsToLambda) {
    Recorder rec;
    FillCtx ctx{42, 3.14};

    auto sink = [&](std::string tag, const FillCtx& c) {
        rec.push(std::move(tag), c.value);
    };

    (void)pipeline(*m_event)
        .fill(sink, "before", ctx)
        .fill(sink, "after",  ctx)
        .status();

    ASSERT_EQ(rec.calls.size(), 2U);
    EXPECT_EQ(rec.calls[0].first, "before");
    EXPECT_EQ(rec.calls[1].first, "after");
    EXPECT_DOUBLE_EQ(rec.calls[0].second, 3.14);
}

TEST_F(EventPipelineFillTest, FillSupportsMemberFunctionPointer) {
    Recorder rec;
    FillCtx ctx{7, 2.718};

    (void)pipeline(*m_event)
        .fill(&Recorder::push_member, &rec, ctx)
        .status();

    ASSERT_EQ(rec.calls.size(), 1U);
    EXPECT_EQ(rec.calls[0].first, "member");
    EXPECT_DOUBLE_EQ(rec.calls[0].second, 2.718);
}

TEST_F(EventPipelineFillTest, FillSkipsWhenFiltered) {
    Recorder rec;

    (void)pipeline(*m_event)
        .filter("always fails", false)
        .fill([&] { rec.push("never", 0.0); })
        .status();

    EXPECT_TRUE(rec.calls.empty());
}

TEST_F(EventPipelineFillTest, FillAfterPassingFilterStillRuns) {
    Recorder rec;

    auto sc = pipeline(*m_event)
        .filter("passes", true)
        .fill([&] { rec.push("ran", 1.0); })
        .status();

    EXPECT_TRUE(sc.is_success());
    ASSERT_EQ(rec.calls.size(), 1U);
    EXPECT_EQ(rec.calls[0].first, "ran");
}
