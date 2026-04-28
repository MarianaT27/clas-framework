#pragma once

#include <cstdint>
#include <type_traits>

#include "mh/accum/concepts.hpp"

namespace mh {

struct CountAccumulator {
    std::uint64_t n = 0;

    constexpr void fill() noexcept { ++n; }
    constexpr void fill(double) noexcept { ++n; }

    constexpr CountAccumulator operator+(CountAccumulator rhs) const noexcept {
        return {n + rhs.n};
    }
    constexpr CountAccumulator& operator+=(CountAccumulator rhs) noexcept {
        n += rhs.n;
        return *this;
    }
    constexpr bool operator==(const CountAccumulator&) const noexcept = default;

    [[nodiscard]] constexpr std::uint64_t value() const noexcept { return n; }
};

static_assert(std::is_trivially_copyable_v<CountAccumulator>);
static_assert(Accumulator<CountAccumulator>);

}
