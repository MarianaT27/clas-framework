#include <gtest/gtest.h>

#include <string>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Pipeline.hpp>

using clas12::Particle;
using clas12::framework::Cutflow;
using clas12::framework::PairPipeline;
using clas12::framework::particle_pairs;
using clas12::framework::unique_pairs;

namespace {

// Minimal Particle factory; tests only touch index() + E().
auto make_particle(int index, double E) -> Particle {
    return Particle(/*pdg*/    0,
                    /*status*/ 0,
                    /*index*/  index,
                    /*charge*/ 0,
                    /*mass*/   0.0,
                    /*px*/     0.0, /*py*/ 0.0, /*pz*/ 0.0,
                    /*E*/      E,
                    /*vx*/     0.0, /*vy*/ 0.0, /*vz*/ 0.0, /*vt*/ 0.0,
                    /*beta*/   1.0,
                    /*chi2pid*/0.0);
}

}  // namespace

// =============================================================================
// Cartesian product walk order and count
// =============================================================================

TEST(PairPipeline, DistinctCollectionsWalkRowMajor) {
    std::vector<Particle> lhs{
        make_particle(10, 1.0),
        make_particle(11, 2.0),
    };
    std::vector<Particle> rhs{
        make_particle(20, 3.0),
        make_particle(21, 4.0),
        make_particle(22, 5.0),
    };

    Cutflow cf("test");
    cf.add_cut("visited");

    std::vector<std::pair<int, int>> seen;
    particle_pairs(lhs, rhs, cf)
        .count("visited")
        .tap([&](const Particle& a, const Particle& b) {
            seen.emplace_back(a.index(), b.index());
        })
        .run();

    ASSERT_EQ(seen.size(), 6U);
    EXPECT_EQ(cf.count("visited"), 6U);

    // Row-major: (10, 20), (10, 21), (10, 22), (11, 20), (11, 21), (11, 22).
    EXPECT_EQ(seen[0], std::make_pair(10, 20));
    EXPECT_EQ(seen[1], std::make_pair(10, 21));
    EXPECT_EQ(seen[2], std::make_pair(10, 22));
    EXPECT_EQ(seen[3], std::make_pair(11, 20));
    EXPECT_EQ(seen[4], std::make_pair(11, 21));
    EXPECT_EQ(seen[5], std::make_pair(11, 22));
}

// =============================================================================
// Upper-triangle iteration for same-collection pairs
// =============================================================================

TEST(PairPipeline, UniquePairsWalksUpperTriangle) {
    std::vector<Particle> xs{
        make_particle(0, 0.0),
        make_particle(1, 0.0),
        make_particle(2, 0.0),
        make_particle(3, 0.0),
    };

    Cutflow cf("test");
    cf.add_cut("pair");

    std::vector<std::pair<int, int>> seen;
    unique_pairs(xs, cf)
        .count("pair")
        .tap([&](const Particle& a, const Particle& b) {
            seen.emplace_back(a.index(), b.index());
        })
        .run();

    // C(4, 2) = 6 unordered pairs.
    ASSERT_EQ(seen.size(), 6U);
    EXPECT_EQ(cf.count("pair"), 6U);

    // Each a.index() < b.index() (upper triangle).
    for (const auto& [a, b] : seen) {
        EXPECT_LT(a, b);
    }
}

// =============================================================================
// Filter drops pairs and counts survivors only
// =============================================================================

TEST(PairPipeline, FilterDropsPairsAndBumpsOnlySurvivors) {
    std::vector<Particle> lhs{make_particle(0, 1.0), make_particle(1, 5.0)};
    std::vector<Particle> rhs{make_particle(2, 1.0), make_particle(3, 5.0)};

    Cutflow cf("test");
    cf.add_cut("visited");
    cf.add_cut("energy sum > 5");

    auto surviving = particle_pairs(lhs, rhs, cf)
        .count("visited")
        .filter("energy sum > 5",
                [](const Particle& a, const Particle& b) { return a.E() + b.E() > 5.0; })
        .run();

    // 4 pairs total; sums are 2, 6, 6, 10 -> three survive.
    EXPECT_EQ(surviving, 3U);
    EXPECT_EQ(cf.count("visited"), 4U);
    EXPECT_EQ(cf.count("energy sum > 5"), 3U);
}

// =============================================================================
// build<T>(fn) terminal with deduced T
// =============================================================================

namespace {
struct Sum {
    double value;
};
}  // namespace

TEST(PairPipeline, BuildDeducesOutputTypeFromLambda) {
    std::vector<Particle> xs{
        make_particle(0, 1.0),
        make_particle(1, 2.0),
        make_particle(2, 3.0),
    };

    auto sums = unique_pairs(xs)
        .build([](const Particle& a, const Particle& b) {
            return Sum{a.E() + b.E()};
        });

    ASSERT_EQ(sums.size(), 3U);
    EXPECT_DOUBLE_EQ(sums[0].value, 3.0);  // 1 + 2
    EXPECT_DOUBLE_EQ(sums[1].value, 4.0);  // 1 + 3
    EXPECT_DOUBLE_EQ(sums[2].value, 5.0);  // 2 + 3
}

// =============================================================================
// Empty-input edge cases
// =============================================================================

TEST(PairPipeline, EmptyBothSidesProducesNothing) {
    std::vector<Particle> empty;
    Cutflow cf("test");
    cf.add_cut("ignored");

    auto n = particle_pairs(empty, empty, cf).count("ignored").run();

    EXPECT_EQ(n, 0U);
    EXPECT_EQ(cf.count("ignored"), 0U);
}

TEST(PairPipeline, OneEmptySideProducesNothing) {
    std::vector<Particle> xs{make_particle(0, 1.0), make_particle(1, 2.0)};
    std::vector<Particle> empty;
    Cutflow cf("test");
    cf.add_cut("ignored");

    EXPECT_EQ(particle_pairs(xs, empty, cf).count("ignored").run(), 0U);
    EXPECT_EQ(particle_pairs(empty, xs, cf).count("ignored").run(), 0U);
    EXPECT_EQ(cf.count("ignored"), 0U);
}

TEST(PairPipeline, UniquePairsOnSingleton) {
    std::vector<Particle> xs{make_particle(0, 0.0)};
    Cutflow cf("test");
    cf.add_cut("pair");

    auto n = unique_pairs(xs, cf).count("pair").run();

    EXPECT_EQ(n, 0U);
    EXPECT_EQ(cf.count("pair"), 0U);
}

// =============================================================================
// Works without a cutflow
// =============================================================================

TEST(PairPipeline, WorksWithoutCutflow) {
    std::vector<Particle> xs{
        make_particle(0, 1.0),
        make_particle(1, 2.0),
        make_particle(2, 3.0),
    };

    auto n = unique_pairs(xs)
        .filter("sum > 3", [](const Particle& a, const Particle& b) {
            return a.E() + b.E() > 3.0;
        })
        .run();

    // Pairs: (1+2)=3, (1+3)=4, (2+3)=5 -> two survive.
    EXPECT_EQ(n, 2U);
}
