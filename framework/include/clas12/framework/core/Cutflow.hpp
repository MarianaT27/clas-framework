#pragma once

#include <fmt/format.h>
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <clas12/framework/services/LoggingService.hpp>

namespace framework {

/**
 * @brief Cutflow manager for tracking event selection efficiency
 *
 * The hot path is `pass(name)` — called several times per event from
 * every worker thread. Counters are stored in a flat
 * `vector<atomic<size_t>>` (one slot per cut, indexed by insertion order)
 * and an `unordered_map<string, size_t>` resolves the name to that index.
 * Compared to the previous `map<string, atomic<size_t>>`:
 *   - O(1) hashed lookup instead of O(log N) tree walk with string compares
 *   - No per-cut heap node; the counters live in one contiguous buffer
 *   - Heterogeneous lookup lets `pass(string_view)` and `pass(const char*)`
 *     skip the std::string allocation that the old `map` overloads forced
 */
class Cutflow {
   public:
    explicit Cutflow(std::string name) : m_name(std::move(name)) {}

    /// Add a cut to the cutflow. Cuts must be added before any `pass()` calls
    /// from worker threads — the underlying vector reserves the slot.
    auto add_cut(const std::string& cut_name) -> void {
        if (m_index.find(cut_name) != m_index.end()) return;  // duplicate add
        m_index.emplace(cut_name, m_cuts.size());
        m_cuts.push_back(cut_name);
        m_counters.push_back(std::make_unique<std::atomic<std::size_t>>(0));
    }

    /// Number of cuts
    [[nodiscard]] auto size() const -> std::size_t { return m_cuts.size(); }

    [[nodiscard]] auto num_cuts() const -> std::size_t { return size(); }

    /// Get all cut names in order
    [[nodiscard]] auto cuts() const -> const std::vector<std::string>& { return m_cuts; }

    [[nodiscard]] auto get_cut_names() const -> const std::vector<std::string>& { return m_cuts; }

    /// Increment counter for a cut (thread-safe). Hot path; resolves
    /// `cut_name` to an index via the heterogeneous unordered_map and
    /// performs a single relaxed atomic increment.
    auto pass(std::string_view cut_name) -> void {
        if (auto it = m_index.find(cut_name); it != m_index.end()) {
            m_counters[it->second]->fetch_add(1, std::memory_order_relaxed);
        }
    }

    auto pass(const std::string& cut_name) -> void {
        pass(std::string_view{cut_name});
    }

    auto pass(const char* cut_name) -> void {
        pass(std::string_view{cut_name});
    }

    /// Get count for a specific cut
    [[nodiscard]] auto count(std::string_view cut_name) const -> std::size_t {
        if (auto it = m_index.find(cut_name); it != m_index.end()) {
            return m_counters[it->second]->load(std::memory_order_relaxed);
        }
        return 0;
    }

    [[nodiscard]] auto count(const std::string& cut_name) const -> std::size_t {
        return count(std::string_view{cut_name});
    }

    [[nodiscard]] auto get_count(const std::string& cut_name) const -> std::size_t {
        return count(cut_name);
    }

    /// Get initial/final counts and efficiency
    [[nodiscard]] auto initial_count() const -> std::size_t {
        return m_cuts.empty() ? 0 : count(m_cuts.front());
    }

    [[nodiscard]] auto final_count() const -> std::size_t {
        return m_cuts.empty() ? 0 : count(m_cuts.back());
    }

    [[nodiscard]] auto efficiency() const -> double {
        auto init = initial_count();
        return init > 0 ? static_cast<double>(final_count()) / static_cast<double>(init) : 0.0;
    }

    /// Get cutflow name
    [[nodiscard]] auto name() const -> const std::string& { return m_name; }

    /// Fold another cutflow into this one (Accumulator<Cutflow>).
    ///
    /// Both cutflows must share the same list of cut names; mismatched
    /// entries in `other` are silently skipped. Intended for the
    /// per-worker accumulator pattern where the partial cutflow from a
    /// local processor is merged into the master cutflow at finalize.
    auto merge(const Cutflow& other) -> void {
        for (std::size_t oi = 0; oi < other.m_cuts.size(); ++oi) {
            auto it = m_index.find(other.m_cuts[oi]);
            if (it == m_index.end()) continue;
            const auto delta = other.m_counters[oi]->load(std::memory_order_relaxed);
            m_counters[it->second]->fetch_add(delta, std::memory_order_relaxed);
        }
    }

    /// Zero every counter (Accumulator<Cutflow>).
    auto reset() -> void {
        for (auto& counter : m_counters) {
            counter->store(0, std::memory_order_relaxed);
        }
    }

    /// Print cutflow table (uses Log singleton if available, otherwise fmt::print)
    auto print() const -> void {
        if (m_cuts.empty()) return;

        const std::size_t init = count(m_cuts.front());
        std::size_t prev = init;

        // Build cut data for table
        std::vector<std::tuple<std::string, std::size_t, double, double>> cut_data;
        cut_data.reserve(m_cuts.size());

        for (std::size_t i = 0; i < m_cuts.size(); ++i) {
            const auto& cut = m_cuts[i];
            const std::size_t cnt = count(cut);
            const double abs_eff = init > 0 ? 100.0 * cnt / init : 0.0;
            const double rel_eff = prev > 0 ? 100.0 * cnt / prev : 0.0;
            cut_data.emplace_back(cut, cnt, abs_eff, rel_eff);
            prev = cnt;
        }

        // Use Log's table printer if available
        if (Log::is_available()) {
            Log::print_cutflow_table(m_name, cut_data, efficiency() * 100.0);
        } else {
            // Fallback to simple text output
            fmt::print("\n");
            fmt::print("+------------------------------------------------------------------------+\n");
            fmt::print("|{:^72}|\n", "Cutflow: " + m_name);
            fmt::print("+-----------------------------------+------------+----------+----------+\n");
            fmt::print("| {:<33} | {:>10} | {:>8} | {:>8} |\n", "Cut", "Events", "Abs Eff", "Rel Eff");
            fmt::print("+-----------------------------------+------------+----------+----------+\n");

            for (const auto& [cut_name, cnt, abs_eff, rel_eff] : cut_data) {
                fmt::print("| {:<33} | {:>10} | {:>6.1f}%  | {:>6.1f}%  |\n",
                          cut_name, cnt, abs_eff, rel_eff);
            }

            fmt::print("+-----------------------------------+------------+----------+----------+\n");
            fmt::print("| {:<70} |\n", fmt::format("Overall efficiency: {:.2f}%", efficiency() * 100.0));
            fmt::print("+------------------------------------------------------------------------+\n");
        }
    }

   private:
    // Heterogeneous lookup: pass(string_view) and pass(const char*) hit
    // the unordered_map without allocating a temporary std::string. Requires
    // C++20 (transparent hashing for unordered associative containers).
    struct StringHash {
        using is_transparent = void;
        std::size_t operator()(std::string_view s) const noexcept {
            return std::hash<std::string_view>{}(s);
        }
        std::size_t operator()(const std::string& s) const noexcept {
            return std::hash<std::string_view>{}(s);
        }
        std::size_t operator()(const char* s) const noexcept {
            return std::hash<std::string_view>{}(std::string_view{s});
        }
    };
    struct StringEq {
        using is_transparent = void;
        bool operator()(std::string_view a, std::string_view b) const noexcept { return a == b; }
    };

    std::string m_name;
    std::vector<std::string> m_cuts;
    // unique_ptr because std::atomic is neither copyable nor movable, so a
    // bare std::vector<std::atomic<...>> cannot be reallocated when push_back
    // grows it. The double-indirection cost is paid once per `pass()` call,
    // dominated by the atomic RMW that follows.
    std::vector<std::unique_ptr<std::atomic<std::size_t>>> m_counters;
    std::unordered_map<std::string, std::size_t, StringHash, StringEq> m_index;
};

}  // namespace framework
