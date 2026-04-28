#pragma once

#include <cmath>
#include <cstdint>
#include <type_traits>

#include "mh/accum/concepts.hpp"

namespace mh {

// Tracks the mean and variance of a second variable y per bin.
// Use via Histogram::fill_profile(y, xs...).
//
// `fill()` (no-arg) inserts a y=0 sample — only used to satisfy the
// Accumulator concept and is semantically meaningless for profiles; prefer
// fill(y) or fill_profile on the histogram.
struct ProfileAccumulator {
    double        sum_wy  = 0.0;
    double        sum_wy2 = 0.0;
    double        sum_w   = 0.0;
    std::uint64_t n       = 0;

    constexpr void fill() noexcept { ++n; }

    constexpr void fill(double y) noexcept {
        sum_wy  += y;
        sum_wy2 += y * y;
        sum_w   += 1.0;
        ++n;
    }

    constexpr void fill(double y, double w) noexcept {
        sum_wy  += w * y;
        sum_wy2 += w * y * y;
        sum_w   += w;
        ++n;
    }

    constexpr ProfileAccumulator operator+(ProfileAccumulator rhs) const noexcept {
        return {sum_wy  + rhs.sum_wy,
                sum_wy2 + rhs.sum_wy2,
                sum_w   + rhs.sum_w,
                n       + rhs.n};
    }
    constexpr ProfileAccumulator& operator+=(ProfileAccumulator rhs) noexcept {
        sum_wy  += rhs.sum_wy;
        sum_wy2 += rhs.sum_wy2;
        sum_w   += rhs.sum_w;
        n       += rhs.n;
        return *this;
    }
    constexpr bool operator==(const ProfileAccumulator&) const noexcept = default;

    [[nodiscard]] constexpr double count() const noexcept { return sum_w; }

    [[nodiscard]] double mean() const noexcept {
        return sum_w > 0.0 ? sum_wy / sum_w : 0.0;
    }
    [[nodiscard]] double variance() const noexcept {
        if (sum_w <= 0.0) return 0.0;
        const double m = mean();
        return sum_wy2 / sum_w - m * m;
    }
    [[nodiscard]] double sigma() const noexcept { return std::sqrt(variance()); }
};

static_assert(std::is_trivially_copyable_v<ProfileAccumulator>);
static_assert(Accumulator<ProfileAccumulator>);

}
