#pragma once

/**
 * @file PerformanceMonitor.hpp
 * @brief Opt-in per-step (per-algorithm) wall + thread-CPU timing, to tell
 *        whether each step is CPU-bound or wait/IO-bound and which is slowest.
 *
 * The diagnostic signal is **wall time vs thread-CPU time** per step:
 *   - cpu ≈ wall  → the thread was actually computing            → CPU-bound
 *   - cpu ≪ wall  → the thread was blocked (disk read, network/DB call, or
 *                   lock contention)                              → wait/IO-bound
 *
 * A synthetic "Input (read+decode)" step measures the gap between consecutive
 * events on a worker thread (i.e. the time hipo::chain spent reading +
 * decompressing the next record), so an input-bound job is visible too.
 *
 * Header-only and dependency-light (only <chrono>/<ctime> + fmt) on purpose:
 * tools/local_batch does NOT link the framework library but still needs to
 * parse the per-slice `PERF` lines and render the identical aggregated table,
 * so the wire helpers (format_perf_line/parse_perf_line) and the renderer
 * (render_table) live here and are usable from both sides.
 *
 * Enabled by `--profile` (CLI) / `[analysis].profile = true` (TOML). When off,
 * the event loop takes its existing untimed path — none of this is touched.
 */

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/format.h>

namespace framework::perf {

/// One accumulator per step: total wall-clock + thread-CPU nanoseconds summed
/// across all worker threads, and the number of times the step ran.
struct StepAccum {
    std::uint64_t wall_ns = 0;
    std::uint64_t cpu_ns  = 0;
    std::uint64_t count   = 0;
};

// ─── clocks ────────────────────────────────────────────────────────────────

/// Monotonic wall-clock nanoseconds (vDSO-fast on Linux/macOS).
[[nodiscard]] inline auto now_wall_ns() -> std::uint64_t {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());
}

/// Per-thread CPU-time nanoseconds (user+system the calling thread consumed).
/// May be a real syscall on some kernels — that is exactly why profiling is
/// opt-in. The gap (wall − cpu) over a step is its wait/IO time.
[[nodiscard]] inline auto now_cpu_ns() -> std::uint64_t {
    std::timespec ts{};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return static_cast<std::uint64_t>(ts.tv_sec) * 1'000'000'000ull +
           static_cast<std::uint64_t>(ts.tv_nsec);
}

// ─── wire format (per-slice → local_batch over the progress-fd pipe) ─────────

/// "PERF <slice> <idx> <wall_ns> <cpu_ns> <count> <name…>\n". The leading
/// "PERF" makes the existing numeric progress parser (sscanf "%d %ld %ld")
/// skip these lines, so the channel stays backward compatible.
[[nodiscard]] inline auto format_perf_line(int slice_id, int idx,
                                           const StepAccum& a,
                                           std::string_view name) -> std::string {
    return fmt::format("PERF {} {} {} {} {} {}\n", slice_id, idx, a.wall_ns,
                       a.cpu_ns, a.count, name);
}

/// Parse one PERF line. `name` is the remainder of the line (it may contain
/// spaces, e.g. "Input (read+decode)"). Returns false if the line is not a
/// well-formed PERF record.
[[nodiscard]] inline auto parse_perf_line(const std::string& line, int& slice_id,
                                          int& idx, StepAccum& a,
                                          std::string& name) -> bool {
    std::istringstream is(line);
    std::string tag;
    if (!(is >> tag) || tag != "PERF") return false;
    if (!(is >> slice_id >> idx >> a.wall_ns >> a.cpu_ns >> a.count)) return false;
    std::getline(is, name);
    // trim one leading space left by getline after the count
    const auto first = name.find_first_not_of(' ');
    name = (first == std::string::npos) ? std::string{} : name.substr(first);
    return true;
}

// ─── classification + rendering (shared by framework + local_batch) ──────────

namespace detail {
constexpr const char* RESET  = "\033[0m";
constexpr const char* GREEN  = "\033[32m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* RED    = "\033[31m";
constexpr const char* GRAY   = "\033[90m";
constexpr const char* BOLD   = "\033[1m";

/// CPU/wall ratio → label + color. ≥0.8 CPU-bound, ≤0.5 wait/IO, else mixed.
struct Bound { const char* label; const char* color; };
[[nodiscard]] inline auto classify(double cpu_over_wall) -> Bound {
    if (cpu_over_wall >= 0.80) return {"CPU-bound", GREEN};
    if (cpu_over_wall <= 0.50) return {"wait/IO",   RED};
    return {"mixed", YELLOW};
}
}  // namespace detail

/// Render an aggregated per-step timing table (slowest-first) + a one-line
/// verdict. Pure std+fmt so both the framework and local_batch can call it.
[[nodiscard]] inline auto render_table(
    std::string_view title,
    std::vector<std::pair<std::string, StepAccum>> rows) -> std::string {
    using namespace detail;
    if (rows.empty()) return {};

    std::sort(rows.begin(), rows.end(),
              [](const auto& x, const auto& y) { return x.second.wall_ns > y.second.wall_ns; });

    std::uint64_t total_wall = 0;
    for (const auto& [n, a] : rows) total_wall += a.wall_ns;
    if (total_wall == 0) total_wall = 1;  // avoid /0; everything will read 0%

    std::size_t namew = std::string_view("Step").size();
    for (const auto& [n, a] : rows) namew = std::max(namew, n.size());

    auto ms       = [](std::uint64_t ns) { return static_cast<double>(ns) / 1e6; };
    auto us_per   = [](const StepAccum& a) {
        return a.count ? static_cast<double>(a.wall_ns) / static_cast<double>(a.count) / 1e3 : 0.0;
    };

    std::string out;
    out += fmt::format("\n{}{}{}\n", BOLD, title, RESET);
    out += fmt::format("{}  {:<{}}  {:>11}  {:>6}  {:>8}  {:<10}  {:>12}  {:>10}{}\n",
                       GRAY, "Step", namew, "Wall(ms)", "Wall%", "CPU/Wall",
                       "Bound", "Calls", "us/call", RESET);
    out += fmt::format("{}  {:-<{}}  {:->11}  {:->6}  {:->8}  {:-<10}  {:->12}  {:->10}{}\n",
                       GRAY, "", namew, "", "", "", "", "", "", RESET);

    for (const auto& [name, a] : rows) {
        const double wall_pct = 100.0 * static_cast<double>(a.wall_ns) / static_cast<double>(total_wall);
        const double ratio    = a.wall_ns ? static_cast<double>(a.cpu_ns) / static_cast<double>(a.wall_ns) : 0.0;
        const Bound  b        = classify(ratio);
        out += fmt::format(
            "  {:<{}}  {:>11.1f}  {:>5.1f}%  {:>8.2f}  {}{:<10}{}  {:>12}  {:>10.2f}\n",
            name, namew, ms(a.wall_ns), wall_pct, ratio, b.color, b.label, RESET,
            a.count, us_per(a));
    }

    // Verdict: the slowest step drives it.
    const auto& [top_name, top] = rows.front();
    const double top_pct   = 100.0 * static_cast<double>(top.wall_ns) / static_cast<double>(total_wall);
    const double top_ratio = top.wall_ns ? static_cast<double>(top.cpu_ns) / static_cast<double>(top.wall_ns) : 0.0;
    const Bound  top_b     = classify(top_ratio);
    const bool   is_input  = top_name.rfind("Input", 0) == 0;
    out += fmt::format("{}  -> Bottleneck: {} ({:.0f}% wall, {}{}{}){}\n",
                       GRAY, top_name, top_pct, top_b.color, top_b.label, GRAY, RESET);
    if (top_ratio <= 0.50) {  // genuinely off-CPU
        out += fmt::format("{}     mostly OFF-CPU: waiting on disk read, network/DB, or lock contention{}\n",
                           GRAY, RESET);
    } else if (is_input) {    // input dominates but is on-CPU = decompression
        out += fmt::format("{}     Input is CPU-bound (record decompression), not disk-limited{}\n",
                           GRAY, RESET);
    }
    out += fmt::format("{}  (\"wait/IO\" = not on CPU: disk read, network/DB, or lock contention){}\n",
                       GRAY, RESET);
    return out;
}

// ─── per-process accumulator (framework only) ────────────────────────────────

/**
 * @brief Thread-safe per-step accumulator. Each worker thread writes to its own
 *        heap-owned ThreadState (registered once under a mutex, mirroring
 *        Cutflow); the monitor OWNS those heap states so they outlive the chain
 *        worker threads (which are destroyed at the end of run(), before the
 *        report is produced in finalize()). The hot path is lock-free.
 */
class PerformanceMonitor {
   public:
    struct ThreadState {
        std::vector<StepAccum> steps;       ///< one per step (algorithms + "Input")
        std::uint64_t          last_wall = 0;
        std::uint64_t          last_cpu  = 0;
        bool                   have_last = false;  ///< false until this thread's 1st event
        explicit ThreadState(std::size_t n) : steps(n) {}
    };

    /// Set the step names (algorithm name()s, with a trailing synthetic
    /// "Input (read+decode)"). Call once before the event loop.
    auto init(std::vector<std::string> step_names) -> void {
        m_names = std::move(step_names);
    }

    [[nodiscard]] auto n_steps() const -> std::size_t { return m_names.size(); }
    [[nodiscard]] auto input_index() const -> std::size_t {
        return m_names.empty() ? 0 : m_names.size() - 1;  // "Input" is last
    }

    /// Borrowed pointer to the calling thread's accumulator (heap-owned). The
    /// per-thread cache keyed by `this` resolves it after the first event;
    /// only that first touch locks the registry.
    [[nodiscard]] auto thread_state() -> ThreadState* {
        thread_local std::unordered_map<PerformanceMonitor*, ThreadState*> cache;
        if (auto it = cache.find(this); it != cache.end()) return it->second;
        ThreadState* ts = nullptr;
        {
            std::lock_guard<std::mutex> lock(m_mu);
            m_states.push_back(std::make_unique<ThreadState>(m_names.size()));
            ts = m_states.back().get();
        }
        cache.emplace(this, ts);
        return ts;
    }

    /// Sum every thread's accumulators into one vector (called after the worker
    /// threads are gone — the heap states are still alive).
    [[nodiscard]] auto reduce() const -> std::vector<StepAccum> {
        std::vector<StepAccum> total(m_names.size());
        std::lock_guard<std::mutex> lock(m_mu);
        for (const auto& st : m_states) {
            for (std::size_t i = 0; i < total.size() && i < st->steps.size(); ++i) {
                total[i].wall_ns += st->steps[i].wall_ns;
                total[i].cpu_ns  += st->steps[i].cpu_ns;
                total[i].count   += st->steps[i].count;
            }
        }
        return total;
    }

    [[nodiscard]] auto rows() const -> std::vector<std::pair<std::string, StepAccum>> {
        auto totals = reduce();
        std::vector<std::pair<std::string, StepAccum>> r;
        r.reserve(totals.size());
        for (std::size_t i = 0; i < m_names.size(); ++i) r.emplace_back(m_names[i], totals[i]);
        return r;
    }

    /// Print the per-run table to stdout (lands in the per-slice log under
    /// local_batch). Self-contained — no LoggingService dependency.
    auto report(std::string_view title = "Per-step performance (wall vs CPU)") const -> void {
        fmt::print("{}", render_table(title, rows()));
    }

    /// PERF lines for every step, for emission to local_batch over the pipe.
    [[nodiscard]] auto perf_lines(int slice_id) const -> std::string {
        auto totals = reduce();
        std::string out;
        for (std::size_t i = 0; i < m_names.size(); ++i)
            out += format_perf_line(slice_id, static_cast<int>(i), totals[i], m_names[i]);
        return out;
    }

   private:
    std::vector<std::string>                  m_names;
    mutable std::mutex                        m_mu;
    std::vector<std::unique_ptr<ThreadState>> m_states;  ///< owned; outlive workers
};

}  // namespace framework::perf
