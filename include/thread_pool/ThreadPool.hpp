#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <vector>

/**
 * @brief A ThreadPool implementation.
 * 
 * Features:
 * - RAII compliant with proper resource management
 * - Move-only semantics for efficiency
 * - Strong exception safety guarantees
 * - Comprehensive statistics and monitoring
 * - Graceful shutdown with configurable timeout
 * - Cross-platform thread naming support
 */
class ThreadPool {
   public:
    enum class ShutdownResult {
        Graceful,       ///< All tasks completed within timeout
        Forced,         ///< Timeout exceeded, threads detached
        AlreadyStopped  ///< Pool was already stopped
    };

    struct Statistics {
        std::atomic<std::size_t> tasks_submitted{0};
        std::atomic<std::size_t> tasks_completed{0};
        std::atomic<std::size_t> tasks_failed{0};
        std::atomic<std::size_t> tasks_pending{0};
        std::chrono::steady_clock::time_point start_time{std::chrono::steady_clock::now()};

        // Custom move operations for atomic members
        Statistics() = default;
        Statistics(const Statistics&) = delete;
        Statistics& operator=(const Statistics&) = delete;
        Statistics(Statistics&& other) noexcept;
        Statistics& operator=(Statistics&& other) noexcept;

        [[nodiscard]] auto get_throughput() const noexcept -> double;
        [[nodiscard]] auto get_uptime() const noexcept -> std::chrono::duration<double>;
    };

    /**
     * @brief Constructs a ThreadPool with specified number of worker threads.
     * @param num_threads Number of threads (0 = hardware_concurrency)
     * @param thread_name_prefix Prefix for thread names
     * @throws std::system_error if thread creation fails
     * @throws std::invalid_argument if num_threads > hardware limit
     */
    explicit ThreadPool(std::size_t num_threads = 0, std::string_view thread_name_prefix = "ThreadPool");

    /**
     * @brief Destructor ensures graceful shutdown with reasonable timeout.
     */
    ~ThreadPool() noexcept;

    // Non-copyable, move-only
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) noexcept;
    ThreadPool& operator=(ThreadPool&&) noexcept;

    /**
     * @brief Submits a task for asynchronous execution.
     * @param callable Function/callable object to execute
     * @param args Arguments to pass to the callable
     * @return Future for the result
     * @throws std::runtime_error if pool is stopped
     */
    template <typename Callable, typename... Args>
    [[nodiscard]] auto submit(Callable&& callable, Args&&... args) -> std::future<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>>;

    /**
     * @brief Initiates graceful shutdown with specified timeout.
     * @param timeout Maximum time to wait for completion
     */
    auto shutdown(std::chrono::milliseconds timeout = std::chrono::seconds{5}) noexcept -> void;

    /**
     * @brief Waits for all pending tasks to complete.
     * @param timeout Maximum time to wait
     * @return true if all tasks completed within timeout
     */
    [[nodiscard]] auto wait_for_idle(std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) const -> bool;

    /**
     * @brief Gets the number of worker threads.
     */
    [[nodiscard]] auto thread_count() const noexcept -> std::size_t { return m_workers.size(); }

    /**
     * @brief Gets the number of pending tasks.
     */
    [[nodiscard]] auto pending_tasks() const noexcept -> std::size_t { return m_stats.tasks_pending.load(std::memory_order_acquire); }

    /**
     * @brief Checks if the pool is running (not stopped).
     */
    [[nodiscard]] auto is_running() const noexcept -> bool { return !m_stop_requested.load(std::memory_order_acquire); }

    /**
     * @brief Gets current statistics snapshot.
     */
    [[nodiscard]] auto get_statistics() const noexcept -> const Statistics& { return m_stats; }

   private:
    auto worker_loop(std::size_t worker_id) noexcept -> void;
    auto set_thread_name(std::string_view name) const noexcept -> void;

    mutable std::mutex m_queue_mutex;
    std::condition_variable m_task_available;
    mutable std::condition_variable m_all_idle;
    std::queue<std::function<void()>> m_task_queue;

    std::vector<std::thread> m_workers;
    std::atomic<bool> m_stop_requested{false};
    std::string m_thread_name_prefix;
    mutable Statistics m_stats;
};

// Template implementation
template <typename Callable, typename... Args>
auto ThreadPool::submit(Callable&& callable, Args&&... args)
    -> std::future<std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>> {

    using ReturnType = std::invoke_result_t<std::decay_t<Callable>, std::decay_t<Args>...>;

    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        [callable = std::forward<Callable>(callable),
         args_tuple = std::make_tuple(std::forward<Args>(args)...)]() mutable -> ReturnType {
            return std::apply(std::move(callable), std::move(args_tuple));
        });

    auto future = task->get_future();

    std::lock_guard lock{m_queue_mutex};
    if (m_stop_requested.load(std::memory_order_acquire)) throw std::runtime_error("Cannot submit task to stopped ThreadPool");
    m_task_queue.emplace([task = std::move(task)]() noexcept {try {(*task)();} catch (...) {} });

    m_stats.tasks_submitted.fetch_add(1, std::memory_order_acq_rel);
    m_stats.tasks_pending.fetch_add(1, std::memory_order_acq_rel);

    m_task_available.notify_one();
    return future;
}