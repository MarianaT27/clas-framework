#include <gtest/gtest.h>
#include "Framework/Result.hpp"

using namespace Framework;

// ============================================================================
// Result<T> Tests
// ============================================================================

TEST(ResultTest, OkCreation) {
    auto result = Result<int>::ok(42);
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.is_error());
    EXPECT_EQ(result.value(), 42);
}

TEST(ResultTest, ErrorCreation) {
    auto result = Result<int>::error("something went wrong");
    EXPECT_FALSE(result.is_ok());
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error_message(), "something went wrong");
}

TEST(ResultTest, ValueAccess) {
    auto result = Result<std::string>::ok("hello");
    EXPECT_EQ(result.value(), "hello");
}

TEST(ResultTest, ValueAccessOnErrorThrows) {
    auto result = Result<int>::error("error");
    EXPECT_THROW(result.value(), std::runtime_error);
}

TEST(ResultTest, ValueOr) {
    auto ok_result = Result<int>::ok(42);
    auto err_result = Result<int>::error("error");

    EXPECT_EQ(ok_result.value_or(0), 42);
    EXPECT_EQ(err_result.value_or(0), 0);
}

TEST(ResultTest, ErrorMessageOnOkIsEmpty) {
    auto result = Result<int>::ok(42);
    EXPECT_TRUE(result.error_message().empty());
}

// ============================================================================
// Monadic Operations Tests
// ============================================================================

TEST(ResultTest, MapOnOk) {
    auto result = Result<int>::ok(5);
    auto mapped = result.map([](int x) { return x * 2; });

    EXPECT_TRUE(mapped.is_ok());
    EXPECT_EQ(mapped.value(), 10);
}

TEST(ResultTest, MapOnError) {
    auto result = Result<int>::error("original error");
    auto mapped = result.map([](int x) { return x * 2; });

    EXPECT_TRUE(mapped.is_error());
    EXPECT_EQ(mapped.error_message(), "original error");
}

TEST(ResultTest, AndThenOnOk) {
    auto result = Result<int>::ok(5);
    auto chained = result.and_then([](int x) -> Result<std::string> {
        return Result<std::string>::ok(std::to_string(x * 2));
    });

    EXPECT_TRUE(chained.is_ok());
    EXPECT_EQ(chained.value(), "10");
}

TEST(ResultTest, AndThenOnError) {
    auto result = Result<int>::error("original error");
    auto chained = result.and_then([](int x) -> Result<std::string> {
        return Result<std::string>::ok(std::to_string(x));
    });

    EXPECT_TRUE(chained.is_error());
    EXPECT_EQ(chained.error_message(), "original error");
}

TEST(ResultTest, AndThenChaining) {
    auto result = Result<int>::ok(10)
                      .and_then([](int x) { return Result<int>::ok(x + 5); })
                      .and_then([](int x) { return Result<int>::ok(x * 2); });

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), 30);
}

TEST(ResultTest, AndThenChainingWithError) {
    auto result = Result<int>::ok(10)
                      .and_then([](int) -> Result<int> { return Result<int>::error("failed"); })
                      .and_then([](int x) { return Result<int>::ok(x * 2); });

    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error_message(), "failed");
}

// ============================================================================
// Result<void> Tests
// ============================================================================

TEST(ResultVoidTest, OkCreation) {
    auto result = Result<void>::ok();
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.is_error());
}

TEST(ResultVoidTest, ErrorCreation) {
    auto result = Result<void>::error("void error");
    EXPECT_FALSE(result.is_ok());
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error_message(), "void error");
}

TEST(ResultVoidTest, ErrorMessageOnOkIsEmpty) {
    auto result = Result<void>::ok();
    EXPECT_TRUE(result.error_message().empty());
}

TEST(ResultVoidTest, AndThenOnOk) {
    auto result = Result<void>::ok();
    auto chained = result.and_then([]() -> Result<void> {
        return Result<void>::ok();
    });

    EXPECT_TRUE(chained.is_ok());
}

TEST(ResultVoidTest, AndThenOnError) {
    auto result = Result<void>::error("original error");
    auto chained = result.and_then([]() -> Result<void> {
        return Result<void>::ok();
    });

    EXPECT_TRUE(chained.is_error());
    EXPECT_EQ(chained.error_message(), "original error");
}
