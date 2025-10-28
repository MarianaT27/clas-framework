#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

/**
 * @brief Thread-safe progress tracker with visual progress bar.
 * 
 * Features:
 * - Lock-free increment operations for performance
 * - Background thread for periodic display updates
 * - Real-time ETA calculation
 * - Smooth sub-character progress rendering with color gradients
 * - Automatic time formatting
 * - ANSI color-coded output with gradient effects
 * - Move-only semantics for safety
 */
class ProgressTracker {
   public:
    struct Config {
        std::size_t bar_width;
        bool show_eta;
        bool show_rate;
        bool use_colors;
        bool use_gradient;
        std::string label;
        std::chrono::milliseconds update_interval;

        Config()
            : bar_width(50), show_eta(true), show_rate(true), 
              use_colors(true), use_gradient(true), label("Processing"),
              update_interval(100) {}
    };

    /**
     * @brief Constructs a progress tracker.
     * @param total Total number of items to track
     * @param config Display configuration
     * @throws std::invalid_argument if total is 0
     */
    explicit ProgressTracker(std::size_t total, Config config = {});

    ~ProgressTracker() noexcept;

    // Non-copyable, move-only
    ProgressTracker(const ProgressTracker&) = delete;
    ProgressTracker& operator=(const ProgressTracker&) = delete;
    ProgressTracker(ProgressTracker&&) noexcept;
    ProgressTracker& operator=(ProgressTracker&&) noexcept;

    /**
     * @brief Increments the progress counter (thread-safe, no immediate display update).
     */
    auto increment() noexcept -> void;

    /**
     * @brief Adds multiple items to progress (thread-safe, no immediate display update).
     */
    auto add(std::size_t count) noexcept -> void;

    /**
     * @brief Gets current progress count (thread-safe).
     */
    [[nodiscard]] auto get_processed() const noexcept -> std::size_t{
        return m_processed.load(std::memory_order_acquire);
    }

    /**
     * @brief Gets total item count (thread-safe).
     */
    [[nodiscard]] auto get_total() const noexcept -> std::size_t {
        return m_total;
    }

    /**
     * @brief Gets current progress ratio [0.0, 1.0] (thread-safe).
     */
    [[nodiscard]] auto get_progress() const noexcept -> double;

    /**
     * @brief Starts the background update thread and shows initial display.
     */
    auto start() noexcept -> void;

    /**
     * @brief Stops the update thread and displays final completion message.
     */
    auto finish() noexcept -> void;

    /**
     * @brief Clears the current progress line.
     */
    static auto clear_line() noexcept -> void;

    /**
     * @brief Formats a duration given in seconds into a human-readable string.
     *
     * Converts the provided std::chrono::seconds duration into a string representation,
     * typically in the format "HH:MM:SS" or similar, for display purposes.
     *
     * @param duration The duration to format, expressed as std::chrono::seconds.
     * @return A std::string containing the formatted duration.
     *
     * @note This function does not throw exceptions.
     */
    [[nodiscard]] static auto format_duration(std::chrono::seconds duration) noexcept -> std::string;


   private:
    // ANSI escape codes
    struct AnsiCodes {
        static constexpr std::string_view CLEAR_LINE = "\r\033[K";
        static constexpr std::string_view RESET = "\033[0m";
        static constexpr std::string_view BOLD = "\033[1m";
        static constexpr std::string_view DIM = "\033[2m";
        
        // Standard colors
        static constexpr std::string_view GREEN = "\033[32m";
        static constexpr std::string_view CYAN = "\033[36m";
        static constexpr std::string_view YELLOW = "\033[33m";
        static constexpr std::string_view MAGENTA = "\033[35m";
        static constexpr std::string_view WHITE = "\033[37m";
        static constexpr std::string_view BLUE = "\033[34m";
        static constexpr std::string_view RED = "\033[31m";
        
        // Bright colors
        static constexpr std::string_view BRIGHT_GREEN = "\033[92m";
        static constexpr std::string_view BRIGHT_CYAN = "\033[96m";
        static constexpr std::string_view BRIGHT_YELLOW = "\033[93m";
        static constexpr std::string_view BRIGHT_MAGENTA = "\033[95m";
        static constexpr std::string_view BRIGHT_WHITE = "\033[97m";
        
        // Background colors for gradient effect
        static constexpr std::string_view BG_CYAN = "\033[46m";
        static constexpr std::string_view BG_BLUE = "\033[44m";
        static constexpr std::string_view BG_MAGENTA = "\033[45m";
    };

    // Progress bar block characters (8 levels of granularity)
    static constexpr std::array<std::string_view, 9> BLOCK_CHARS = {
        " ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};
    
    // Spinner animation characters
    static constexpr std::array<std::string_view, 10> SPINNER_CHARS = {
        "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

    auto update_display() noexcept -> void;
    auto render_bar(double progress) const noexcept -> void;
    auto update_loop() noexcept -> void;
    auto stop_update_thread() noexcept -> void;

    [[nodiscard]] static auto format_number(std::size_t num) noexcept -> std::string;
    [[nodiscard]] auto get_elapsed() const noexcept -> std::chrono::seconds;
    [[nodiscard]] auto calculate_eta() const noexcept -> std::chrono::seconds;
    [[nodiscard]] auto calculate_rate() const noexcept -> double;
    [[nodiscard]] auto get_progress_color() const noexcept -> std::string_view;

    const std::size_t m_total;
    std::atomic<std::size_t> m_processed{0};
    std::chrono::steady_clock::time_point m_start_time;
    Config m_config;
    mutable std::mutex m_display_mutex;
    bool m_finished{false};
    mutable std::size_t m_spinner_index{0};

    // Background update thread
    std::atomic<bool> m_should_stop{false};
    std::thread m_update_thread;
};