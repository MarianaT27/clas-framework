#pragma once

#include <cmath>
#include <type_traits>

#include "mh/accum/concepts.hpp"

namespace mh {

// 16-byte alignment lets AtomicStorage<WeightedAccumulator> issue a single
// 128-bit CAS (CMPXCHG16B / CASP) per fill on platforms with 16-byte atomic
// support, instead of two independent CAS loops over sum_w and sum_w2.
struct alignas(16) WeightedAccumulator {
    double sum_w  = 0.0;
    double sum_w2 = 0.0;

    constexpr void fill() noexcept {
        sum_w  += 1.0;
        sum_w2 += 1.0;
    }
    constexpr void fill(double w) noexcept {
        sum_w  += w;
        sum_w2 += w * w;
    }

    constexpr WeightedAccumulator operator+(WeightedAccumulator rhs) const noexcept {
        return {sum_w + rhs.sum_w, sum_w2 + rhs.sum_w2};
    }
    constexpr WeightedAccumulator& operator+=(WeightedAccumulator rhs) noexcept {
        sum_w  += rhs.sum_w;
        sum_w2 += rhs.sum_w2;
        return *this;
    }
    constexpr bool operator==(const WeightedAccumulator&) const noexcept = default;

    [[nodiscard]] constexpr double value()    const noexcept { return sum_w; }
    [[nodiscard]] constexpr double variance() const noexcept { return sum_w2; }
    [[nodiscard]] double error() const noexcept { return std::sqrt(sum_w2); }
};

static_assert(std::is_trivially_copyable_v<WeightedAccumulator>);
static_assert(sizeof(WeightedAccumulator)  == 16);
static_assert(alignof(WeightedAccumulator) == 16);
static_assert(Accumulator<WeightedAccumulator>);

}
