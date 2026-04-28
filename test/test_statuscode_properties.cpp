#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <clas12/framework/core/StatusCode.hpp>

using namespace framework;

/**
 * **Feature: code-quality-improvements, Property 2: StatusCode Error Message Preservation**
 * 
 * *For any* StatusCode created with a failure status and error message, 
 * the StatusCode SHALL preserve and return the exact error message when queried.
 * 
 * **Validates: Requirements 2.1, 2.2**
 */
RC_GTEST_PROP(StatusCodeProperties, ErrorMessagePreservation, ()) {
    // Generate a random non-empty error message
    std::string message = *rc::gen::nonEmpty<std::string>();

    // Create a failure StatusCode with the message
    StatusCode sc = StatusCode::failure(message);

    // Verify the StatusCode is a failure
    RC_ASSERT(sc.is_failure());
    RC_ASSERT(!sc.is_success());
    RC_ASSERT(!static_cast<bool>(sc));

    // Verify the exact error message is preserved
    RC_ASSERT(sc.message() == message);
}

/**
 * Additional property: Success StatusCode has empty message
 * 
 * *For any* StatusCode created with success status,
 * the message SHALL be empty.
 */
RC_GTEST_PROP(StatusCodeProperties, SuccessHasEmptyMessage, ()) {
    StatusCode sc = StatusCode::success();

    RC_ASSERT(sc.is_success());
    RC_ASSERT(!sc.is_failure());
    RC_ASSERT(static_cast<bool>(sc));
    RC_ASSERT(sc.message().empty());
}

/**
 * Additional property: Backward compatible FAILURE() has empty message
 */
RC_GTEST_PROP(StatusCodeProperties, BackwardCompatibleFailureEmptyMessage, ()) {
    StatusCode sc = StatusCode::FAILURE();

    RC_ASSERT(sc.is_failure());
    RC_ASSERT(!sc.is_success());
    RC_ASSERT(sc.message().empty());
}

// Unit tests for specific edge cases
TEST(StatusCodeTest, FailureWithEmptyMessage) {
    StatusCode sc = StatusCode::failure("");
    EXPECT_TRUE(sc.is_failure());
    EXPECT_EQ(sc.message(), "");
}

TEST(StatusCodeTest, FailureWithSpecialCharacters) {
    std::string msg = "Error: file 'test.txt' not found\n\tat line 42";
    StatusCode sc = StatusCode::failure(msg);
    EXPECT_TRUE(sc.is_failure());
    EXPECT_EQ(sc.message(), msg);
}

TEST(StatusCodeTest, BackwardCompatibility) {
    // Test that old-style SUCCESS() and FAILURE() still work
    StatusCode success = StatusCode::SUCCESS();
    StatusCode failure = StatusCode::FAILURE();

    EXPECT_TRUE(success.is_success());
    EXPECT_TRUE(failure.is_failure());

    // Old-style should have empty messages
    EXPECT_TRUE(success.message().empty());
    EXPECT_TRUE(failure.message().empty());
}

TEST(StatusCodeTest, BoolConversion) {
    EXPECT_TRUE(static_cast<bool>(StatusCode::SUCCESS()));
    EXPECT_FALSE(static_cast<bool>(StatusCode::FAILURE()));
    EXPECT_FALSE(static_cast<bool>(StatusCode::failure("error")));
}
