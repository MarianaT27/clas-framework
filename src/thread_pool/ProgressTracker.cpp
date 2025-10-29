#include "thread_pool/ProgressTracker.hpp"
#include <fmt/core.h>
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <stdexcept>

ProgressTracker::ProgressTracker(std::size_t total, Config config)
    : m_total{total}, m_start_time{std::chrono::steady_clock::now()}, m_config{config} {
    if (total == 0) throw std::invalid_argument("ProgressTracker: total must be greater than 0");
}

ProgressTracker::~ProgressTracker() noexcept {
    stop_update_thread();
    if (!m_finished) clear_line();
}

ProgressTracker::ProgressTracker(ProgressTracker&& other) noexcept
    : m_total{other.m_total}, m_processed{other.m_processed.load(std::memory_order_acquire)}, m_start_time{other.m_start_time}, m_config{other.m_config}, m_finished{other.m_finished} {
    other.stop_update_thread();
    other.m_finished = true;
}

ProgressTracker& ProgressTracker::operator=(ProgressTracker&& other) noexcept {
    if (this != &other) {
        stop_update_thread();
        if (!m_finished) clear_line();
        other.stop_update_thread();

        const_cast<std::size_t&>(m_total) = other.m_total;
        m_processed.store(other.m_processed.load(std::memory_order_acquire), std::memory_order_release);
        m_start_time = other.m_start_time;
        m_config = other.m_config;
        m_finished = other.m_finished;

        other.m_finished = true;
    }
    return *this;
}

auto ProgressTracker::increment() noexcept -> void {
    m_processed.fetch_add(1, std::memory_order_acq_rel);
}

auto ProgressTracker::add(std::size_t count) noexcept -> void {
    m_processed.fetch_add(count, std::memory_order_acq_rel);
}

auto ProgressTracker::get_progress() const noexcept -> double {
    const auto current = m_processed.load(std::memory_order_acquire);
    return m_total > 0 ? static_cast<double>(current) / static_cast<double>(m_total) : 0.0;
}

auto ProgressTracker::get_elapsed() const noexcept -> std::chrono::seconds {
    const auto elapsed = std::chrono::steady_clock::now() - m_start_time;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsed);
}

auto ProgressTracker::calculate_rate() const noexcept -> double {
    const auto elapsed = get_elapsed().count();
    const auto current = m_processed.load(std::memory_order_acquire);
    return elapsed > 0 ? static_cast<double>(current) / static_cast<double>(elapsed) : 0.0;
}

auto ProgressTracker::calculate_eta() const noexcept -> std::chrono::seconds {
    const auto current = m_processed.load(std::memory_order_acquire);
    const auto rate = calculate_rate();

    if (rate > 0.0 && current < m_total) {
        const auto remaining = m_total - current;
        return std::chrono::seconds{static_cast<long>(remaining / rate)};
    }
    return std::chrono::seconds{0};
}

auto ProgressTracker::format_duration(std::chrono::seconds duration) noexcept -> std::string {
    const auto m_totalseconds = duration.count();

    if (m_totalseconds < 60) return fmt::format("{}s", m_totalseconds);
    if (m_totalseconds < 3600) {
        const auto minutes = m_totalseconds / 60;
        const auto seconds = m_totalseconds % 60;
        return fmt::format("{}m {:02d}s", minutes, seconds);
    }

    const auto hours = m_totalseconds / 3600;
    const auto minutes = (m_totalseconds % 3600) / 60;
    return fmt::format("{}h {:02d}m", hours, minutes);
}

auto ProgressTracker::format_number(std::size_t num) noexcept -> std::string {
    if (num < 1000) return fmt::format("{}", num);
    if (num < 1000000) return fmt::format("{:.1f}k", num / 1000.0);
    return fmt::format("{:.1f}M", num / 1000000.0);
}

auto ProgressTracker::clear_line() noexcept -> void {
    try {
        fmt::print("{}", AnsiCodes::CLEAR_LINE);
        std::cout.flush();
    } catch (...) {
    }
}

auto ProgressTracker::get_progress_color() const noexcept -> std::string_view {
    if (!m_config.use_colors || !m_config.use_gradient) return AnsiCodes::GREEN;
    return AnsiCodes::BRIGHT_GREEN;
}

auto ProgressTracker::render_bar(double progress) const noexcept -> void {
    try {
        const auto& colors = m_config.use_colors;
        progress = std::clamp(progress, 0.0, 1.0);

        const auto filled_width = m_config.bar_width * progress;
        const auto full_blocks = static_cast<std::size_t>(filled_width);
        const auto remainder = filled_width - static_cast<double>(full_blocks);
        const auto partial_block_idx = static_cast<std::size_t>(remainder * 8.0);

        // Left bracket with color
        if (colors) fmt::print("{}", AnsiCodes::DIM);
        fmt::print("╢");
        if (colors) fmt::print("{}", AnsiCodes::RESET);

        // Render progress bar with gradient
        for (std::size_t i = 0; i < m_config.bar_width; ++i) {
            if (i < full_blocks) {
                if (colors) {
                    fmt::print("{}", get_progress_color());
                }
                fmt::print("{}", BLOCK_CHARS[8]);
            } else if (i == full_blocks) {
                if (colors) {
                    fmt::print("{}", get_progress_color());
                }
                fmt::print("{}", BLOCK_CHARS[partial_block_idx]);
            } else {
                if (colors) fmt::print("{}", AnsiCodes::DIM);
                fmt::print("░");
            }
        }

        if (colors) fmt::print("{}", AnsiCodes::RESET);

        // Right bracket
        if (colors) fmt::print("{}", AnsiCodes::DIM);
        fmt::print("╟");
        if (colors) fmt::print("{}", AnsiCodes::RESET);
        fmt::print(" ");

    } catch (...) {
    }
}

auto ProgressTracker::update_display() noexcept -> void {
    std::lock_guard lock{m_display_mutex};

    if (m_finished) return;

    try {
        const auto current = m_processed.load(std::memory_order_acquire);
        const auto progress = get_progress();
        const auto elapsed = get_elapsed();
        const auto& colors = m_config.use_colors;

        clear_line();

        // Spinner animation
        if (progress < 1.0) {
            if (colors) fmt::print("{}", AnsiCodes::BRIGHT_CYAN);
            fmt::print("{}", SPINNER_CHARS[m_spinner_index % SPINNER_CHARS.size()]);
            m_spinner_index++;
            if (colors) fmt::print("{}", AnsiCodes::RESET);
            fmt::print(" ");
        } else {
            if (colors) fmt::print("{}", AnsiCodes::BRIGHT_GREEN);
            fmt::print("✓");
            if (colors) fmt::print("{}", AnsiCodes::RESET);
            fmt::print(" ");
        }

        // Label
        if (!m_config.label.empty()) {
            if (colors) fmt::print("{}", AnsiCodes::BOLD);
            fmt::print("{}", m_config.label);
            if (colors) fmt::print("{}", AnsiCodes::RESET);
            fmt::print(" ");
        }

        render_bar(progress);

        // Percentage with enhanced styling
        if (progress >= 1.0) {
            if (colors) fmt::print("{}{}", AnsiCodes::BRIGHT_GREEN, AnsiCodes::BOLD);
            fmt::print("100%");
            if (colors) fmt::print("{}", AnsiCodes::RESET);
        } else {
            if (colors) fmt::print("{}", get_progress_color());
            fmt::print("{:>3.0f}%", progress * 100.0);
            if (colors) fmt::print("{}", AnsiCodes::RESET);
        }

        // Count with dividers
        fmt::print(" ");
        if (colors) fmt::print("{}", AnsiCodes::DIM);
        fmt::print("│");
        if (colors) fmt::print("{}", AnsiCodes::RESET);
        fmt::print(" ");

        if (colors) fmt::print("{}", AnsiCodes::BRIGHT_WHITE);
        fmt::print("{}", format_number(current));
        if (colors) fmt::print("{}", AnsiCodes::RESET);

        if (colors) fmt::print("{}", AnsiCodes::DIM);
        fmt::print("/");
        if (colors) fmt::print("{}", AnsiCodes::RESET);

        if (colors) fmt::print("{}", AnsiCodes::BRIGHT_CYAN);
        fmt::print("{}", format_number(m_total));
        if (colors) fmt::print("{}", AnsiCodes::RESET);

        // Elapsed time
        fmt::print(" ");
        if (colors) fmt::print("{}", AnsiCodes::DIM);
        fmt::print("│ ⏱");
        if (colors) fmt::print("{}", AnsiCodes::RESET);
        fmt::print(" ");
        if (colors) fmt::print("{}", AnsiCodes::MAGENTA);
        fmt::print("{}", format_duration(elapsed));
        if (colors) fmt::print("{}", AnsiCodes::RESET);

        // ETA
        if (m_config.show_eta && current < m_total) {
            const auto eta = calculate_eta();
            if (eta.count() > 0) {
                fmt::print(" ");
                if (colors) fmt::print("{}", AnsiCodes::DIM);
                fmt::print("│ ⏳");
                if (colors) fmt::print("{}", AnsiCodes::RESET);
                fmt::print(" ");
                if (colors) fmt::print("{}", AnsiCodes::YELLOW);
                fmt::print("{}", format_duration(eta));
                if (colors) fmt::print("{}", AnsiCodes::RESET);
            }
        }

        // Rate
        if (m_config.show_rate) {
            const auto rate = calculate_rate();
            if (rate > 0.0) {
                fmt::print(" ");
                if (colors) fmt::print("{}", AnsiCodes::DIM);
                fmt::print("│ ⚡");
                if (colors) fmt::print("{}", AnsiCodes::RESET);
                fmt::print(" ");
                if (colors) fmt::print("{}", AnsiCodes::BRIGHT_MAGENTA);
                fmt::print("{:.1f}/s", rate);
                if (colors) fmt::print("{}", AnsiCodes::RESET);
            }
        }

        std::cout.flush();

    } catch (...) {
    }
}

auto ProgressTracker::update_loop() noexcept -> void {
    while (!m_should_stop.load(std::memory_order_acquire)) {
        update_display();
        if (m_processed.load(std::memory_order_acquire) >= m_total) break;
        std::this_thread::sleep_for(m_config.update_interval);
    }

    update_display();
}

auto ProgressTracker::start() noexcept -> void {
    if (!m_update_thread.joinable() && !m_finished) {
        m_should_stop.store(false, std::memory_order_release);
        update_display();
        m_update_thread = std::thread(&ProgressTracker::update_loop, this);
    }
}

auto ProgressTracker::stop_update_thread() noexcept -> void {
    if (m_update_thread.joinable()) {
        m_should_stop.store(true, std::memory_order_release);
        m_update_thread.join();
    }
}

auto ProgressTracker::finish() noexcept -> void {
    stop_update_thread();

    std::lock_guard lock{m_display_mutex};

    if (m_finished) return;
    m_finished = true;

    try {
        const auto elapsed = get_elapsed();
        const auto& colors = m_config.use_colors;

        clear_line();

        // Completion line
        if (colors) fmt::print("{}", AnsiCodes::BRIGHT_GREEN);
        fmt::print(" {} Complete!   │   ", m_config.label);
        if (colors) fmt::print("{}", AnsiCodes::RESET);

        // Files processed
        fmt::print(" Items: ");
        if (colors) fmt::print("{}", AnsiCodes::BRIGHT_CYAN);
        fmt::print("{}", format_number(m_total));
        if (colors) fmt::print("{}", AnsiCodes::RESET);
        fmt::print("  │  ");

        // Time elapsed
        fmt::print("⏱  Time: ");
        if (colors) fmt::print("{}", AnsiCodes::MAGENTA);
        fmt::print("{}", format_duration(elapsed));
        if (colors) fmt::print("{}", AnsiCodes::RESET);

        fmt::print("\n");
        std::cout.flush();

    } catch (...) {
    }
}