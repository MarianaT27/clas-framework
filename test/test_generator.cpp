/**
 * @file test_generator.cpp
 * @brief Unit tests for Generator<T> coroutine type
 *
 * Tests the C++20 coroutine-based Generator implementation.
 *
 * @see Requirements 5.1
 */

#include <gtest/gtest.h>
#include <vector>
#include <string>

#include <clas12/framework/core/Generator.hpp>

using namespace framework;

// ============================================================================
// Basic Generator Tests
// ============================================================================

/**
 * @brief Simple generator that yields integers from start to end
 */
Generator<int> range(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}

/**
 * @brief Generator that yields filtered values
 */
Generator<int> even_numbers(int start, int end) {
    for (int i = start; i < end; ++i) {
        if (i % 2 == 0) {
            co_yield i;
        }
    }
}

/**
 * @brief Generator that yields strings
 */
Generator<std::string> string_generator() {
    co_yield "hello";
    co_yield "world";
    co_yield "coroutines";
}

/**
 * @brief Empty generator
 */
Generator<int> empty_generator() {
    co_return;
}

TEST(GeneratorTest, BasicIteration) {
    std::vector<int> result;
    for (int value : range(0, 5)) {
        result.push_back(value);
    }
    
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 0);
    EXPECT_EQ(result[1], 1);
    EXPECT_EQ(result[2], 2);
    EXPECT_EQ(result[3], 3);
    EXPECT_EQ(result[4], 4);
}

TEST(GeneratorTest, FilteredIteration) {
    std::vector<int> result;
    for (int value : even_numbers(0, 10)) {
        result.push_back(value);
    }
    
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 0);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 4);
    EXPECT_EQ(result[3], 6);
    EXPECT_EQ(result[4], 8);
}

TEST(GeneratorTest, StringGenerator) {
    std::vector<std::string> result;
    for (const auto& value : string_generator()) {
        result.push_back(value);
    }
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "hello");
    EXPECT_EQ(result[1], "world");
    EXPECT_EQ(result[2], "coroutines");
}

TEST(GeneratorTest, EmptyGenerator) {
    std::vector<int> result;
    for (int value : empty_generator()) {
        result.push_back(value);
    }
    
    EXPECT_TRUE(result.empty());
}

TEST(GeneratorTest, NextMethod) {
    auto gen = range(0, 3);
    
    auto v1 = gen.next();
    ASSERT_TRUE(v1.has_value());
    EXPECT_EQ(*v1, 0);
    
    auto v2 = gen.next();
    ASSERT_TRUE(v2.has_value());
    EXPECT_EQ(*v2, 1);
    
    auto v3 = gen.next();
    ASSERT_TRUE(v3.has_value());
    EXPECT_EQ(*v3, 2);
    
    auto v4 = gen.next();
    EXPECT_FALSE(v4.has_value());
}

TEST(GeneratorTest, DoneCheck) {
    auto gen = range(0, 2);
    
    EXPECT_FALSE(gen.done());
    
    gen.next();  // 0
    EXPECT_FALSE(gen.done());
    
    gen.next();  // 1
    EXPECT_FALSE(gen.done());
    
    gen.next();  // exhausted
    EXPECT_TRUE(gen.done());
}

TEST(GeneratorTest, MoveSemantics) {
    auto gen1 = range(0, 3);
    auto gen2 = std::move(gen1);
    
    // gen1 should be empty after move
    EXPECT_FALSE(static_cast<bool>(gen1));
    
    // gen2 should work
    std::vector<int> result;
    for (int value : gen2) {
        result.push_back(value);
    }
    
    ASSERT_EQ(result.size(), 3);
}

// ============================================================================
// Reference Generator Tests
// ============================================================================

/**
 * @brief Generator that yields references to elements in a vector
 */
Generator<int&> ref_generator(std::vector<int>& vec) {
    for (auto& elem : vec) {
        co_yield elem;
    }
}

TEST(GeneratorRefTest, ReferenceIteration) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    
    // Modify through references
    for (int& value : ref_generator(data)) {
        value *= 2;
    }
    
    EXPECT_EQ(data[0], 2);
    EXPECT_EQ(data[1], 4);
    EXPECT_EQ(data[2], 6);
    EXPECT_EQ(data[3], 8);
    EXPECT_EQ(data[4], 10);
}

// ============================================================================
// Nested Generator Tests
// ============================================================================

/**
 * @brief Generator that composes another generator
 */
Generator<int> doubled_range(int start, int end) {
    for (int value : range(start, end)) {
        co_yield value * 2;
    }
}

TEST(GeneratorTest, NestedGenerators) {
    std::vector<int> result;
    for (int value : doubled_range(0, 5)) {
        result.push_back(value);
    }
    
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], 0);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 4);
    EXPECT_EQ(result[3], 6);
    EXPECT_EQ(result[4], 8);
}

// ============================================================================
// Traditional Algorithm Compatibility Tests
// ============================================================================

/**
 * @brief Test that demonstrates traditional execute() pattern still works
 *
 * This test verifies that the coroutine support doesn't break the
 * traditional algorithm pattern where execute() is called for each event.
 *
 * @see Requirement 5.4
 */
TEST(CompatibilityTest, TraditionalPatternWorks) {
    // Simulate traditional algorithm pattern
    struct MockEvent {
        int id;
    };
    
    // Traditional execute() pattern
    auto traditional_execute = [](MockEvent& event) -> bool {
        return event.id > 0;
    };
    
    // Process events traditionally
    std::vector<MockEvent> events = {{1}, {-1}, {2}, {0}, {3}};
    int passed = 0;
    
    for (auto& event : events) {
        if (traditional_execute(event)) {
            passed++;
        }
    }
    
    EXPECT_EQ(passed, 3);  // Events with id > 0: 1, 2, 3
}

/**
 * @brief Test that demonstrates coroutine pattern as alternative
 *
 * Shows how coroutines provide an alternative to traditional execute()
 * without replacing it.
 */
Generator<int> filter_positive(const std::vector<int>& values) {
    for (int v : values) {
        if (v > 0) {
            co_yield v;
        }
    }
}

TEST(CompatibilityTest, CoroutinePatternAsAlternative) {
    std::vector<int> values = {1, -1, 2, 0, 3};
    
    std::vector<int> result;
    for (int v : filter_positive(values)) {
        result.push_back(v);
    }
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
}
