#pragma once

/**
 * @file Generator.hpp
 * @brief C++20 coroutine-based generator for lazy event iteration
 *
 * This file provides a Generator<T> coroutine type that enables elegant
 * event filtering and lazy evaluation using C++20 coroutines.
 *
 * ## Migration Guide: Traditional vs Coroutine Patterns
 *
 * The coroutine support is **complementary** to the traditional execute() pattern.
 * Existing algorithms continue to work unchanged. Coroutines provide an alternative
 * for cases where lazy evaluation and elegant filtering are beneficial.
 *
 * ### Traditional Pattern (Still Fully Supported)
 *
 * The traditional pattern uses execute() called by the Application event loop:
 *
 * @code
 * class MyAlgorithm : public SimpleAlgorithm<MyAlgorithm> {
 * public:
 *     auto process(Event& event) -> StatusCode override {
 *         // Process each event
 *         if (should_filter(event)) {
 *             return StatusCode::FILTERED();
 *         }
 *         // Do analysis...
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 *
 * ### Coroutine Pattern (New Alternative)
 *
 * Coroutines enable elegant filtering with sequential-looking code:
 *
 * @code
 * Generator<int> range(int start, int end) {
 *     for (int i = start; i < end; ++i) {
 *         co_yield i;
 *     }
 * }
 *
 * // Usage
 * for (int value : range(0, 10)) {
 *     std::cout << value << "\n";
 * }
 * @endcode
 *
 * ### When to Use Each Pattern
 *
 * **Use Traditional execute() when:**
 * - Working within the Application framework
 * - Need service injection and property system
 * - Processing all events in sequence
 * - Integrating with existing algorithm chains
 *
 * **Use Coroutines when:**
 * - Writing standalone analysis scripts
 * - Need complex multi-stage filtering
 * - Want lazy evaluation for efficiency
 * - Composing multiple filter stages
 *
 * ### Backward Compatibility
 *
 * - All existing algorithms using execute() continue to work unchanged
 * - The Application class event loop is unchanged
 * - SimpleAlgorithm works as before
 * - Coroutines are opt-in and don't affect existing code
 *
 * @see Requirements 5.1, 5.2, 5.4
 */

#include <coroutine>
#include <exception>
#include <iterator>
#include <optional>
#include <utility>

namespace clas12::framework {

/**
 * @brief A lazy generator using C++20 coroutines
 *
 * Generator<T> is a coroutine type that produces values lazily on demand.
 * It supports range-based for loops and can be used to implement elegant
 * event filtering pipelines.
 *
 * The generator suspends after each yield and resumes when the next value
 * is requested, enabling efficient lazy evaluation.
 *
 * Example usage:
 * @code
 * // Define a generator function
 * Generator<int> range(int start, int end) {
 *     for (int i = start; i < end; ++i) {
 *         co_yield i;
 *     }
 * }
 *
 * // Use with range-based for loop
 * for (int value : range(0, 10)) {
 *     std::cout << value << "\n";
 * }
 * @endcode
 *
 * @tparam T The type of values produced by the generator
 *
 * @see Requirement 5.1
 */
template <typename T>
class Generator {
public:
    /**
     * @brief Promise type required by the coroutine machinery
     *
     * The promise_type defines how the coroutine behaves:
     * - initial_suspend: suspend immediately (lazy start)
     * - final_suspend: suspend at end (allows cleanup)
     * - yield_value: store the yielded value and suspend
     */
    struct promise_type {
        /// The current yielded value (stored as optional for safety)
        std::optional<T> current_value;
        
        /// Exception pointer for propagating exceptions
        std::exception_ptr exception;

        /**
         * @brief Create the Generator object from the promise
         * @return Generator The generator wrapping this coroutine
         */
        auto get_return_object() -> Generator {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        /**
         * @brief Suspend immediately at coroutine start (lazy evaluation)
         * @return std::suspend_always Always suspend
         */
        auto initial_suspend() noexcept -> std::suspend_always {
            return {};
        }

        /**
         * @brief Suspend at coroutine end (allows proper cleanup)
         * @return std::suspend_always Always suspend
         */
        auto final_suspend() noexcept -> std::suspend_always {
            return {};
        }

        /**
         * @brief Handle a yielded value
         *
         * Stores the value and suspends the coroutine until the next
         * iteration is requested.
         *
         * @param value The value being yielded
         * @return std::suspend_always Always suspend after yield
         */
        auto yield_value(T value) -> std::suspend_always {
            current_value = std::move(value);
            return {};
        }

        /**
         * @brief Handle coroutine completion (no return value)
         */
        auto return_void() noexcept -> void {}

        /**
         * @brief Handle exceptions thrown in the coroutine
         */
        auto unhandled_exception() -> void {
            exception = std::current_exception();
        }
    };

    /// Coroutine handle type
    using handle_type = std::coroutine_handle<promise_type>;

    /**
     * @brief Iterator for range-based for loop support
     *
     * This iterator allows Generator to be used with range-based for loops
     * and standard algorithms that work with input iterators.
     */
    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        /// Default constructor creates an end iterator
        iterator() noexcept = default;

        /**
         * @brief Construct an iterator from a coroutine handle
         * @param handle The coroutine handle
         */
        explicit iterator(handle_type handle) noexcept : m_handle(handle) {}

        /**
         * @brief Advance to the next value
         * @return iterator& Reference to this iterator
         */
        auto operator++() -> iterator& {
            if (m_handle) {
                m_handle.resume();
                if (m_handle.done()) {
                    // Check for exceptions
                    if (m_handle.promise().exception) {
                        std::rethrow_exception(m_handle.promise().exception);
                    }
                    m_handle = nullptr;
                }
            }
            return *this;
        }

        /**
         * @brief Post-increment operator
         * @return void (input iterator only requires pre-increment to return reference)
         */
        auto operator++(int) -> void {
            ++(*this);
        }

        /**
         * @brief Dereference to get the current value
         * @return const T& Reference to the current value
         */
        auto operator*() const -> const T& {
            return *m_handle.promise().current_value;
        }

        /**
         * @brief Arrow operator for member access
         * @return const T* Pointer to the current value
         */
        auto operator->() const -> const T* {
            return &(*m_handle.promise().current_value);
        }

        /**
         * @brief Compare with another iterator
         * @param other The other iterator
         * @return bool True if both are at the same position
         */
        auto operator==(const iterator& other) const noexcept -> bool {
            return m_handle == other.m_handle;
        }

        /**
         * @brief Compare with sentinel (end iterator)
         * @param sentinel The sentinel value
         * @return bool True if at end
         */
        auto operator==(std::default_sentinel_t) const noexcept -> bool {
            return !m_handle || m_handle.done();
        }

    private:
        handle_type m_handle = nullptr;
    };

    // ========================================================================
    // Constructors and destructor
    // ========================================================================

    /// Default constructor creates an empty generator
    Generator() noexcept = default;

    /**
     * @brief Construct from a coroutine handle
     * @param handle The coroutine handle
     */
    explicit Generator(handle_type handle) noexcept : m_handle(handle) {}

    /// Generators are move-only (no copying)
    Generator(const Generator&) = delete;
    auto operator=(const Generator&) -> Generator& = delete;

    /**
     * @brief Move constructor
     * @param other The generator to move from
     */
    Generator(Generator&& other) noexcept : m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

    /**
     * @brief Move assignment operator
     * @param other The generator to move from
     * @return Generator& Reference to this
     */
    auto operator=(Generator&& other) noexcept -> Generator& {
        if (this != &other) {
            if (m_handle) {
                m_handle.destroy();
            }
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }

    /**
     * @brief Destructor - destroys the coroutine
     */
    ~Generator() {
        if (m_handle) {
            m_handle.destroy();
        }
    }

    // ========================================================================
    // Range interface
    // ========================================================================

    /**
     * @brief Get an iterator to the beginning
     *
     * This resumes the coroutine to get the first value.
     *
     * @return iterator Iterator pointing to the first value
     */
    auto begin() -> iterator {
        if (m_handle) {
            m_handle.resume();
            if (m_handle.done()) {
                // Check for exceptions
                if (m_handle.promise().exception) {
                    std::rethrow_exception(m_handle.promise().exception);
                }
                return iterator{};
            }
        }
        return iterator{m_handle};
    }

    /**
     * @brief Get the end sentinel
     * @return std::default_sentinel_t The end sentinel
     */
    auto end() const noexcept -> std::default_sentinel_t {
        return {};
    }

    // ========================================================================
    // Additional utilities
    // ========================================================================

    /**
     * @brief Check if the generator is valid (has a coroutine)
     * @return bool True if the generator has a valid coroutine
     */
    explicit operator bool() const noexcept {
        return m_handle != nullptr;
    }

    /**
     * @brief Check if the generator is exhausted
     * @return bool True if the coroutine has completed
     */
    [[nodiscard]] auto done() const noexcept -> bool {
        return !m_handle || m_handle.done();
    }

    /**
     * @brief Get the next value (alternative to iteration)
     *
     * Resumes the coroutine and returns the next value, or std::nullopt
     * if the generator is exhausted.
     *
     * @return std::optional<T> The next value, or nullopt if done
     */
    auto next() -> std::optional<T> {
        if (!m_handle || m_handle.done()) {
            return std::nullopt;
        }
        
        m_handle.resume();
        
        if (m_handle.done()) {
            if (m_handle.promise().exception) {
                std::rethrow_exception(m_handle.promise().exception);
            }
            return std::nullopt;
        }
        
        return m_handle.promise().current_value;
    }

private:
    handle_type m_handle = nullptr;
};

/**
 * @brief Generator specialization for reference types
 *
 * This specialization handles Generator<T&> to allow yielding references
 * without copying. Useful for event streams where copying events is expensive.
 *
 * @tparam T The referenced type
 */
template <typename T>
class Generator<T&> {
public:
    struct promise_type {
        T* current_value = nullptr;
        std::exception_ptr exception;

        auto get_return_object() -> Generator {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto initial_suspend() noexcept -> std::suspend_always { return {}; }
        auto final_suspend() noexcept -> std::suspend_always { return {}; }

        auto yield_value(T& value) -> std::suspend_always {
            current_value = std::addressof(value);
            return {};
        }

        auto return_void() noexcept -> void {}

        auto unhandled_exception() -> void {
            exception = std::current_exception();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;

        iterator() noexcept = default;
        explicit iterator(handle_type handle) noexcept : m_handle(handle) {}

        auto operator++() -> iterator& {
            if (m_handle) {
                m_handle.resume();
                if (m_handle.done()) {
                    if (m_handle.promise().exception) {
                        std::rethrow_exception(m_handle.promise().exception);
                    }
                    m_handle = nullptr;
                }
            }
            return *this;
        }

        auto operator++(int) -> void { ++(*this); }

        auto operator*() const -> T& {
            return *m_handle.promise().current_value;
        }

        auto operator->() const -> T* {
            return m_handle.promise().current_value;
        }

        auto operator==(const iterator& other) const noexcept -> bool {
            return m_handle == other.m_handle;
        }

        auto operator==(std::default_sentinel_t) const noexcept -> bool {
            return !m_handle || m_handle.done();
        }

    private:
        handle_type m_handle = nullptr;
    };

    Generator() noexcept = default;
    explicit Generator(handle_type handle) noexcept : m_handle(handle) {}
    Generator(const Generator&) = delete;
    auto operator=(const Generator&) -> Generator& = delete;
    Generator(Generator&& other) noexcept : m_handle(other.m_handle) { other.m_handle = nullptr; }
    auto operator=(Generator&& other) noexcept -> Generator& {
        if (this != &other) {
            if (m_handle) m_handle.destroy();
            m_handle = other.m_handle;
            other.m_handle = nullptr;
        }
        return *this;
    }
    ~Generator() { if (m_handle) m_handle.destroy(); }

    auto begin() -> iterator {
        if (m_handle) {
            m_handle.resume();
            if (m_handle.done()) {
                if (m_handle.promise().exception) {
                    std::rethrow_exception(m_handle.promise().exception);
                }
                return iterator{};
            }
        }
        return iterator{m_handle};
    }

    auto end() const noexcept -> std::default_sentinel_t { return {}; }

    explicit operator bool() const noexcept { return m_handle != nullptr; }
    [[nodiscard]] auto done() const noexcept -> bool { return !m_handle || m_handle.done(); }

private:
    handle_type m_handle = nullptr;
};

}  // namespace clas12::framework
