#include <gtest/gtest.h>

#include <clas12/framework/core/CutBuilder.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/StatusCode.hpp>

using namespace clas12::framework;

// =============================================================================
// CutResult Tests
// =============================================================================

TEST(CutResultTest, PassedResult) {
    auto result = CutResult::pass("Test Cut");
    EXPECT_TRUE(result.passed);
    EXPECT_TRUE(result);  // implicit conversion
    EXPECT_EQ(result.reason, "Test Cut");
}

TEST(CutResultTest, FailedResult) {
    auto result = CutResult::fail("Failed Cut");
    EXPECT_FALSE(result.passed);
    EXPECT_FALSE(result);  // implicit conversion
    EXPECT_EQ(result.reason, "Failed Cut");
}

// =============================================================================
// Cut Tests
// =============================================================================

TEST(CutTest, EvaluatePassingCut) {
    Cut cut("Always Pass", []() { return true; });
    auto result = cut.evaluate();
    EXPECT_TRUE(result);
    EXPECT_EQ(cut.name(), "Always Pass");
}

TEST(CutTest, EvaluateFailingCut) {
    Cut cut("Always Fail", []() { return false; });
    auto result = cut.evaluate();
    EXPECT_FALSE(result);
}

TEST(CutTest, CutWithCapture) {
    int value = 5;
    Cut cut("Value > 3", [&value]() { return value > 3; });
    EXPECT_TRUE(cut.evaluate());

    value = 2;
    EXPECT_FALSE(cut.evaluate());
}

// =============================================================================
// CutBuilder Basic Tests
// =============================================================================

TEST(CutBuilderTest, EmptyBuilder) {
    Cutflow cutflow("Test");
    CutBuilder builder(cutflow);

    EXPECT_TRUE(builder.empty());
    EXPECT_EQ(builder.size(), 0);
    EXPECT_TRUE(builder.evaluate());  // Empty builder passes
}

TEST(CutBuilderTest, SinglePassingCut) {
    Cutflow cutflow("Test");
    cutflow.add_cut("All Events");

    auto result = CutBuilder(cutflow)
                      .cut("All Events", []() { return true; })
                      .evaluate();

    EXPECT_TRUE(result);
    EXPECT_EQ(cutflow.count("All Events"), 1);
}

TEST(CutBuilderTest, SingleFailingCut) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Never Pass");

    auto result = CutBuilder(cutflow)
                      .cut("Never Pass", []() { return false; })
                      .evaluate();

    EXPECT_FALSE(result);
    EXPECT_EQ(cutflow.count("Never Pass"), 0);
}

TEST(CutBuilderTest, ChainedCuts) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Cut 1");
    cutflow.add_cut("Cut 2");
    cutflow.add_cut("Cut 3");

    auto result = CutBuilder(cutflow)
                      .cut("Cut 1", []() { return true; })
                      .cut("Cut 2", []() { return true; })
                      .cut("Cut 3", []() { return true; })
                      .evaluate();

    EXPECT_TRUE(result);
    EXPECT_EQ(cutflow.count("Cut 1"), 1);
    EXPECT_EQ(cutflow.count("Cut 2"), 1);
    EXPECT_EQ(cutflow.count("Cut 3"), 1);
}

TEST(CutBuilderTest, EarlyFailure) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Cut 1");
    cutflow.add_cut("Cut 2");
    cutflow.add_cut("Cut 3");

    auto result = CutBuilder(cutflow)
                      .cut("Cut 1", []() { return true; })
                      .cut("Cut 2", []() { return false; })  // Fails here
                      .cut("Cut 3", []() { return true; })
                      .evaluate();

    EXPECT_FALSE(result);
    EXPECT_EQ(cutflow.count("Cut 1"), 1);
    EXPECT_EQ(cutflow.count("Cut 2"), 0);  // Failed, not counted
    EXPECT_EQ(cutflow.count("Cut 3"), 0);  // Never reached
}

// =============================================================================
// CutBuilder Convenience Methods Tests
// =============================================================================

TEST(CutBuilderTest, RequireGt) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Value > 5");

    double value = 6.0;
    auto result = CutBuilder(cutflow)
                      .require_gt("Value > 5", value, 5.0)
                      .evaluate();

    EXPECT_TRUE(result);
    EXPECT_EQ(cutflow.count("Value > 5"), 1);
}

TEST(CutBuilderTest, RequireGtFails) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Value > 5");

    double value = 4.0;
    auto result = CutBuilder(cutflow)
                      .require_gt("Value > 5", value, 5.0)
                      .evaluate();

    EXPECT_FALSE(result);
}

TEST(CutBuilderTest, RequireGte) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Value >= 5");

    double value = 5.0;
    auto result = CutBuilder(cutflow)
                      .require_gte("Value >= 5", value, 5.0)
                      .evaluate();

    EXPECT_TRUE(result);
}

TEST(CutBuilderTest, RequireLt) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Value < 10");

    double value = 8.0;
    auto result = CutBuilder(cutflow)
                      .require_lt("Value < 10", value, 10.0)
                      .evaluate();

    EXPECT_TRUE(result);
}

TEST(CutBuilderTest, RequireLte) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Value <= 10");

    double value = 10.0;
    auto result = CutBuilder(cutflow)
                      .require_lte("Value <= 10", value, 10.0)
                      .evaluate();

    EXPECT_TRUE(result);
}

TEST(CutBuilderTest, RequireInRange) {
    Cutflow cutflow("Test");
    cutflow.add_cut("5 <= Value <= 15");

    double value = 10.0;
    auto result = CutBuilder(cutflow)
                      .require_in_range("5 <= Value <= 15", value, 5.0, 15.0)
                      .evaluate();

    EXPECT_TRUE(result);
}

TEST(CutBuilderTest, RequireInRangeBoundary) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Min Boundary");
    cutflow.add_cut("Max Boundary");

    double min_val = 5.0;
    double max_val = 15.0;

    // Test min boundary (inclusive)
    auto result1 = CutBuilder(cutflow)
                       .require_in_range("Min Boundary", min_val, 5.0, 15.0)
                       .evaluate();
    EXPECT_TRUE(result1);

    // Test max boundary (inclusive)
    auto result2 = CutBuilder(cutflow)
                       .require_in_range("Max Boundary", max_val, 5.0, 15.0)
                       .evaluate();
    EXPECT_TRUE(result2);
}

TEST(CutBuilderTest, RequireInRangeFails) {
    Cutflow cutflow("Test");
    cutflow.add_cut("In Range");

    double value = 20.0;
    auto result = CutBuilder(cutflow)
                      .require_in_range("In Range", value, 5.0, 15.0)
                      .evaluate();

    EXPECT_FALSE(result);
}

TEST(CutBuilderTest, RequireEq) {
    Cutflow cutflow("Test");
    cutflow.add_cut("PID == 11");

    int pid = 11;
    auto result = CutBuilder(cutflow)
                      .require_eq("PID == 11", pid, 11)
                      .evaluate();

    EXPECT_TRUE(result);
}

TEST(CutBuilderTest, RequireNe) {
    Cutflow cutflow("Test");
    cutflow.add_cut("PID != 0");

    int pid = 11;
    auto result = CutBuilder(cutflow)
                      .require_ne("PID != 0", pid, 0)
                      .evaluate();

    EXPECT_TRUE(result);
}

// =============================================================================
// CutBuilder StatusCode Integration Tests
// =============================================================================

TEST(CutBuilderTest, ApplyReturnsSuccess) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Pass");

    auto status = CutBuilder(cutflow)
                      .cut("Pass", []() { return true; })
                      .apply();

    EXPECT_TRUE(status.is_success());
}

TEST(CutBuilderTest, ApplyReturnsFiltered) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Fail");

    auto status = CutBuilder(cutflow)
                      .cut("Fail", []() { return false; })
                      .apply();

    EXPECT_TRUE(status.is_filtered());
}

// =============================================================================
// CutBuilder Factory Function Tests
// =============================================================================

TEST(CutBuilderTest, CutsFactoryFunction) {
    Cutflow cutflow("Test");
    cutflow.add_cut("All Events");

    auto result = cuts(cutflow)
                      .cut("All Events", []() { return true; })
                      .evaluate();

    EXPECT_TRUE(result);
}

// =============================================================================
// CutBuilder Metadata Tests
// =============================================================================

TEST(CutBuilderTest, CutNames) {
    Cutflow cutflow("Test");
    cutflow.add_cut("Cut A");
    cutflow.add_cut("Cut B");
    cutflow.add_cut("Cut C");

    CutBuilder builder(cutflow);
    builder.cut("Cut A", []() { return true; })
        .cut("Cut B", []() { return true; })
        .cut("Cut C", []() { return true; });

    auto names = builder.cut_names();
    ASSERT_EQ(names.size(), 3);
    EXPECT_EQ(names[0], "Cut A");
    EXPECT_EQ(names[1], "Cut B");
    EXPECT_EQ(names[2], "Cut C");
}

TEST(CutBuilderTest, Size) {
    Cutflow cutflow("Test");
    CutBuilder builder(cutflow);

    EXPECT_EQ(builder.size(), 0);

    builder.cut("Cut 1", []() { return true; });
    EXPECT_EQ(builder.size(), 1);

    builder.cut("Cut 2", []() { return true; });
    EXPECT_EQ(builder.size(), 2);
}

// =============================================================================
// CutBuilder Real-World Usage Pattern Tests
// =============================================================================

TEST(CutBuilderTest, ElectronSelectionPattern) {
    // Simulate electron selection cuts
    Cutflow cutflow("Electron Selection");
    cutflow.add_cut("All Events");
    cutflow.add_cut("Has Electron");
    cutflow.add_cut("Momentum > 2 GeV");
    cutflow.add_cut("Theta in FD");

    // Simulated particle properties
    bool has_electron = true;
    double momentum = 3.5;
    double theta = 15.0;

    auto status = cuts(cutflow)
                      .cut("All Events", []() { return true; })
                      .cut("Has Electron", [&]() { return has_electron; })
                      .require_gt("Momentum > 2 GeV", momentum, 2.0)
                      .require_in_range("Theta in FD", theta, 5.0, 35.0)
                      .apply();

    EXPECT_TRUE(status.is_success());
    EXPECT_EQ(cutflow.count("All Events"), 1);
    EXPECT_EQ(cutflow.count("Has Electron"), 1);
    EXPECT_EQ(cutflow.count("Momentum > 2 GeV"), 1);
    EXPECT_EQ(cutflow.count("Theta in FD"), 1);
}

TEST(CutBuilderTest, MultipleEventsPattern) {
    Cutflow cutflow("Multi-Event");
    cutflow.add_cut("All");
    cutflow.add_cut("Pass Cut");

    // Process multiple "events"
    std::vector<double> values = {1.0, 3.0, 5.0, 2.0, 4.0};
    int passed = 0;

    for (const auto& val : values) {
        auto result = cuts(cutflow)
                          .cut("All", []() { return true; })
                          .require_gt("Pass Cut", val, 2.5)
                          .evaluate();
        if (result) ++passed;
    }

    EXPECT_EQ(cutflow.count("All"), 5);
    EXPECT_EQ(cutflow.count("Pass Cut"), 3);  // 3.0, 5.0, 4.0 > 2.5
    EXPECT_EQ(passed, 3);
}
