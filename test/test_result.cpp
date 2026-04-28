#include <gtest/gtest.h>
#include <clas12/framework/core/Result.hpp>

using namespace framework;

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

TEST(ResultTest, ErrorWithErrorObject) {
    auto err = Error("detailed error");
    auto result = Result<int>::error(std::move(err));
    EXPECT_FALSE(result.is_ok());
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error().message(), "detailed error");
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

TEST(ResultTest, WithContext) {
    auto result = Result<int>::error("inner error");
    auto with_ctx = result.with_context("outer context");

    EXPECT_TRUE(with_ctx.is_error());
    EXPECT_EQ(with_ctx.error().message(), "outer context");
    EXPECT_TRUE(with_ctx.error().has_cause());
    EXPECT_EQ(with_ctx.error().cause()->message(), "inner error");
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

TEST(ResultVoidTest, WithContext) {
    auto result = Result<void>::error("inner error");
    auto with_ctx = result.with_context("outer context");

    EXPECT_TRUE(with_ctx.is_error());
    EXPECT_EQ(with_ctx.error().message(), "outer context");
    EXPECT_TRUE(with_ctx.error().has_cause());
}

// ============================================================================
// Error Tests
// ============================================================================

TEST(ErrorTest, BasicConstruction) {
    auto err = Error("test error");
    EXPECT_EQ(err.message(), "test error");
    EXPECT_FALSE(err.has_cause());
}

TEST(ErrorTest, SourceLocationCapture) {
    auto err = Error("test error");
    // Source location should be captured
    EXPECT_NE(err.location().line(), 0);
    EXPECT_NE(std::string(err.location().file_name()).find("test_result.cpp"), std::string::npos);
}

TEST(ErrorTest, ErrorChaining) {
    auto inner = Error("inner error");
    auto outer = Error("outer error");
    outer.with_cause(std::move(inner));

    EXPECT_TRUE(outer.has_cause());
    EXPECT_EQ(outer.cause()->message(), "inner error");
}

TEST(ErrorTest, FormatOutput) {
    auto err = Error("test error");
    auto formatted = err.format();

    // Should contain file, line, function, and message
    EXPECT_NE(formatted.find("test_result.cpp"), std::string::npos);
    EXPECT_NE(formatted.find("test error"), std::string::npos);
}

TEST(ErrorTest, FormatWithCause) {
    auto inner = Error("inner error");
    auto outer = Error("outer error");
    outer.with_cause(std::move(inner));

    auto formatted = outer.format();

    EXPECT_NE(formatted.find("outer error"), std::string::npos);
    EXPECT_NE(formatted.find("Caused by"), std::string::npos);
    EXPECT_NE(formatted.find("inner error"), std::string::npos);
}

TEST(ErrorTest, FormatShort) {
    auto inner = Error("inner");
    auto outer = Error("outer");
    outer.with_cause(std::move(inner));

    auto short_fmt = outer.format_short();
    EXPECT_EQ(short_fmt, "outer (caused by: inner)");
}

TEST(ErrorTest, CopyConstruction) {
    auto original = Error("original");
    original.with_cause(Error("cause"));

    auto copy = original;
    EXPECT_EQ(copy.message(), "original");
    EXPECT_TRUE(copy.has_cause());
    EXPECT_EQ(copy.cause()->message(), "cause");
}

TEST(ErrorTest, MoveConstruction) {
    auto original = Error("original");
    original.with_cause(Error("cause"));

    auto moved = std::move(original);
    EXPECT_EQ(moved.message(), "original");
    EXPECT_TRUE(moved.has_cause());
}


// ============================================================================
// TRY Macro Tests
// ============================================================================

namespace {

// Helper functions for testing TRY macro
auto succeed_with_value(int x) -> Result<int> {
    return Result<int>::ok(x);
}

auto fail_with_error(const std::string& msg) -> Result<int> {
    return Result<int>::error(msg);
}

auto function_using_try(bool should_succeed) -> Result<int> {
    auto result = should_succeed ? succeed_with_value(42) : fail_with_error("test error");
    auto value = FRAMEWORK_TRY(result);
    return Result<int>::ok(value * 2);
}

auto function_using_try_context(bool should_succeed) -> Result<int> {
    auto result = should_succeed ? succeed_with_value(42) : fail_with_error("inner error");
    auto value = FRAMEWORK_TRY_CONTEXT(result, "outer context");
    return Result<int>::ok(value * 2);
}

auto chained_try_operations() -> Result<int> {
    auto a = FRAMEWORK_TRY(succeed_with_value(10));
    auto b = FRAMEWORK_TRY(succeed_with_value(20));
    auto c = FRAMEWORK_TRY(succeed_with_value(30));
    return Result<int>::ok(a + b + c);
}

auto chained_try_with_failure() -> Result<int> {
    auto a = FRAMEWORK_TRY(succeed_with_value(10));
    auto b = FRAMEWORK_TRY(fail_with_error("middle failure"));
    auto c = FRAMEWORK_TRY(succeed_with_value(30));  // Should not be reached
    return Result<int>::ok(a + b + c);
}

}  // namespace

TEST(TryMacroTest, SuccessfulTry) {
    auto result = function_using_try(true);
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), 84);  // 42 * 2
}

TEST(TryMacroTest, FailedTry) {
    auto result = function_using_try(false);
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error_message(), "test error");
}

TEST(TryMacroTest, TryWithContextSuccess) {
    auto result = function_using_try_context(true);
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), 84);
}

TEST(TryMacroTest, TryWithContextFailure) {
    auto result = function_using_try_context(false);
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error().message(), "outer context");
    EXPECT_TRUE(result.error().has_cause());
    EXPECT_EQ(result.error().cause()->message(), "inner error");
}

TEST(TryMacroTest, ChainedTrySuccess) {
    auto result = chained_try_operations();
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), 60);  // 10 + 20 + 30
}

TEST(TryMacroTest, ChainedTryWithFailure) {
    auto result = chained_try_with_failure();
    EXPECT_TRUE(result.is_error());
    EXPECT_EQ(result.error_message(), "middle failure");
}

// Test short alias
TEST(TryMacroTest, ShortAliasWorks) {
    auto test_func = []() -> Result<int> {
        auto val = FW_TRY(succeed_with_value(100));
        return Result<int>::ok(val + 1);
    };

    auto result = test_func();
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(result.value(), 101);
}
