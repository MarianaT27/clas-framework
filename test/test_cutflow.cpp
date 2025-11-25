#include <gtest/gtest.h>
#include "Framework/Cutflow.hpp"

using namespace Framework;

TEST(CutflowTest, BasicFunctionality) {
    Cutflow cutflow("Test Cutflow");

    // Add cuts
    cutflow.add_cut("All Events");
    cutflow.add_cut("Cut 1");
    cutflow.add_cut("Cut 2");

    // Simulate event processing
    for (int i = 0; i < 100; ++i) {
        cutflow.pass("All Events");
        if (i % 2 == 0) {  // 50% pass rate
            cutflow.pass("Cut 1");
            if (i % 4 == 0) {  // 50% of those pass
                cutflow.pass("Cut 2");
            }
        }
    }

    // Check counts
    EXPECT_EQ(cutflow.get_count("All Events"), 100);
    EXPECT_EQ(cutflow.get_count("Cut 1"), 50);
    EXPECT_EQ(cutflow.get_count("Cut 2"), 25);
}

TEST(CutflowTest, CutNames) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Cut A");
    cutflow.add_cut("Cut B");
    cutflow.add_cut("Cut C");

    auto names = cutflow.get_cut_names();
    ASSERT_EQ(names.size(), 3);
    EXPECT_EQ(names[0], "Cut A");
    EXPECT_EQ(names[1], "Cut B");
    EXPECT_EQ(names[2], "Cut C");
}

TEST(CutflowTest, EmptyCutflow) {
    Cutflow cutflow("Empty");
    EXPECT_EQ(cutflow.get_count("NonExistent"), 0);
}

TEST(CutflowTest, ThreadSafety) {
    Cutflow cutflow("Thread Test");
    cutflow.add_cut("Counter");

    // Simulate concurrent increments
    #pragma omp parallel for
    for (int i = 0; i < 10000; ++i) {
        cutflow.pass("Counter");
    }

    EXPECT_EQ(cutflow.get_count("Counter"), 10000);
}
