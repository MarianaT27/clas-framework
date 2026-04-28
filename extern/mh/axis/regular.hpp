#pragma once

#include <concepts>
#include <cstddef>
#include <optional>

#include "mh/axis/concepts.hpp"

namespace mh {

// Regular axis with under/overflow bins: index() always returns an engaged
// optional with a value in [-1, bins]. -1 = underflow, bins = overflow,
// [0, bins) = in-range. Storage layout puts the underflow at slot 0, in-range
// at slots [1, bins+1), overflow at slot bins+1; total extent = bins + 2.
template <std::floating_point T = double>
class RegularAxis {
public:
    using value_type = T;

    // Numeric axis: emits flow indices instead of discarding out-of-range.
    static constexpr bool has_flow = true;

    constexpr RegularAxis(T lo, T hi, std::size_t n) noexcept
        : lo_(lo),
          hi_(hi),
          inv_width_(static_cast<T>(n) / (hi - lo)),
          bins_(n) {}

    [[nodiscard]] constexpr std::optional<std::ptrdiff_t>
    index(T x) const noexcept {
        return index_raw(x);
    }

    // Branchless index helper used by bulk fill kernels — same value as
    // index() but without the optional wrapper. Always returns a valid
    // flow-aware ptrdiff_t in [-1, bins].
    [[nodiscard]] constexpr std::ptrdiff_t
    index_raw(T x) const noexcept {
        const T f = (x - lo_) * inv_width_;
        if (f < T{0}) return -1;
        const auto i = static_cast<std::size_t>(f);
        if (i >= bins_) return static_cast<std::ptrdiff_t>(bins_);
        return static_cast<std::ptrdiff_t>(i);
    }

    [[nodiscard]] constexpr std::size_t bins()   const noexcept { return bins_; }
    [[nodiscard]] constexpr std::size_t extent() const noexcept { return bins_ + 2; }

    [[nodiscard]] constexpr double lower(std::size_t i) const noexcept {
        return static_cast<double>(lo_) + static_cast<double>(i) / inv_width_;
    }
    [[nodiscard]] constexpr double upper(std::size_t i) const noexcept {
        return lower(i + 1);
    }

    [[nodiscard]] constexpr T min() const noexcept { return lo_; }
    [[nodiscard]] constexpr T max() const noexcept { return hi_; }

private:
    T lo_;
    T hi_;
    T inv_width_;
    std::size_t bins_;
};

static_assert(Axis<RegularAxis<double>>);
static_assert(Axis<RegularAxis<float>>);
static_assert(axis_has_flow_v<RegularAxis<double>>);

constexpr RegularAxis<double> regular(double lo, double hi, std::size_t n) noexcept {
    return {lo, hi, n};
}

}
