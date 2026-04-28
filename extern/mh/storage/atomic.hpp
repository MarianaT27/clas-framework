#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <vector>

#include "mh/accum/concepts.hpp"
#include "mh/accum/count.hpp"
#include "mh/accum/weighted.hpp"

// 16-byte atomic CAS via CMPXCHG16B on x86-64. On AArch64 the same operation
// is only lock-free with LSE2 (ARMv8.4+); without that, GCC emits a libatomic
// call that's not provided on every platform (notably macOS). Restrict the
// path to x86-64 and fall back to two single-CAS loops elsewhere.
#if defined(__SIZEOF_INT128__) && defined(__x86_64__)
#  define MH_HAS_ATOMIC_INT128 1
#else
#  define MH_HAS_ATOMIC_INT128 0
#endif

namespace mh {

namespace detail {

inline void atomic_add_relaxed(double& target, double delta) noexcept {
    std::atomic_ref<double> ref(target);
    double expected = ref.load(std::memory_order_relaxed);
    while (!ref.compare_exchange_weak(expected, expected + delta,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed)) {
        // expected was updated by CAS on failure — retry with new value.
    }
}

#if MH_HAS_ATOMIC_INT128
// Single-CAS update of (sum_w, sum_w2) viewed as a 128-bit word. This keeps
// the pair internally consistent (no torn reads) and roughly halves the
// contended-bin nanosecond cost vs. two independent CAS loops.
inline void atomic_add_weighted_relaxed(WeightedAccumulator& target,
                                        double dw, double dw2) noexcept {
    static_assert(sizeof(WeightedAccumulator) == 16);
    static_assert(alignof(WeightedAccumulator) == 16);

    using u128 = unsigned __int128;
    auto* word = reinterpret_cast<u128*>(&target);
    u128 expected;
    __atomic_load(word, &expected, __ATOMIC_RELAXED);
    while (true) {
        WeightedAccumulator cur;
        std::memcpy(&cur, &expected, sizeof(cur));
        WeightedAccumulator next{cur.sum_w + dw, cur.sum_w2 + dw2};
        u128 desired;
        std::memcpy(&desired, &next, sizeof(desired));
        if (__atomic_compare_exchange(word, &expected, &desired,
                                      true,
                                      __ATOMIC_RELAXED,
                                      __ATOMIC_RELAXED)) {
            return;
        }
        // expected updated by failed CAS — retry with new value.
    }
}
#endif

}

// AtomicStorage routes fills through lock-free atomics. Specialized for
// CountAccumulator (single fetch_add) and WeightedAccumulator (single 128-bit
// CAS where supported, falling back to a paired CAS loop otherwise).
//
// Non-atomic operator[] is provided for post-join readout — it must NOT be
// called while other threads are still filling.
template <Accumulator Acc>
class AtomicStorage {
    static_assert(std::is_same_v<Acc, CountAccumulator> ||
                  std::is_same_v<Acc, WeightedAccumulator>,
                  "mh: AtomicStorage is only specialized for CountAccumulator "
                  "and WeightedAccumulator. Use thread_pool for other types.");

public:
    using value_type = Acc;

    AtomicStorage() = default;
    explicit AtomicStorage(std::size_t n) : data_(n) {}

    void resize(std::size_t n) { data_.assign(n, Acc{}); }
    void reset() { std::fill(data_.begin(), data_.end(), Acc{}); }

    [[nodiscard]] Acc&       operator[](std::size_t i)       noexcept { return data_[i]; }
    [[nodiscard]] const Acc& operator[](std::size_t i) const noexcept { return data_[i]; }

    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

    [[nodiscard]] Acc*       data()       noexcept { return data_.data(); }
    [[nodiscard]] const Acc* data() const noexcept { return data_.data(); }

    [[nodiscard]] auto begin()       noexcept { return data_.begin(); }
    [[nodiscard]] auto end()         noexcept { return data_.end(); }
    [[nodiscard]] auto begin() const noexcept { return data_.begin(); }
    [[nodiscard]] auto end()   const noexcept { return data_.end(); }

    void atomic_fill(std::size_t i) noexcept {
        if constexpr (std::is_same_v<Acc, CountAccumulator>) {
            std::atomic_ref<std::uint64_t> ref(data_[i].n);
            ref.fetch_add(1, std::memory_order_relaxed);
        } else {
#if MH_HAS_ATOMIC_INT128
            detail::atomic_add_weighted_relaxed(data_[i], 1.0, 1.0);
#else
            detail::atomic_add_relaxed(data_[i].sum_w,  1.0);
            detail::atomic_add_relaxed(data_[i].sum_w2, 1.0);
#endif
        }
    }

    void atomic_fill(std::size_t i, double w) noexcept {
        if constexpr (std::is_same_v<Acc, CountAccumulator>) {
            (void)w;
            atomic_fill(i);
        } else {
#if MH_HAS_ATOMIC_INT128
            detail::atomic_add_weighted_relaxed(data_[i], w, w * w);
#else
            detail::atomic_add_relaxed(data_[i].sum_w,  w);
            detail::atomic_add_relaxed(data_[i].sum_w2, w * w);
#endif
        }
    }

    AtomicStorage& operator+=(const AtomicStorage& rhs) noexcept {
        const auto n = size();
        for (std::size_t i = 0; i < n; ++i) data_[i] += rhs.data_[i];
        return *this;
    }

private:
    std::vector<Acc> data_;
};

}
