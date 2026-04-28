#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>
#include <optional>

#include "mh/axis/concepts.hpp"

namespace mh {

template <std::floating_point T = double>
class CircularAxis {
public:
    using value_type = T;

    // Wrapping axis: out-of-range values are folded back into [0, bins). No
    // dedicated flow bins (extent == bins).
    static constexpr bool has_flow = false;

    constexpr CircularAxis(T lo, T hi, std::size_t n) noexcept
        : lo_(lo),
          range_(hi - lo),
          inv_width_(static_cast<T>(n) / (hi - lo)),
          bins_(n) {}

    [[nodiscard]] std::optional<std::ptrdiff_t>
    index(T x) const noexcept {
        const T shifted = x - lo_;
        const T wrapped = shifted - std::floor(shifted / range_) * range_;
        auto i = static_cast<std::ptrdiff_t>(wrapped * inv_width_);
        if (i == static_cast<std::ptrdiff_t>(bins_)) i = 0;
        return i;
    }

    [[nodiscard]] constexpr std::size_t bins()   const noexcept { return bins_; }
    [[nodiscard]] constexpr std::size_t extent() const noexcept { return bins_; }

    [[nodiscard]] constexpr double lower(std::size_t i) const noexcept {
        return static_cast<double>(lo_) + static_cast<double>(i) / inv_width_;
    }
    [[nodiscard]] constexpr double upper(std::size_t i) const noexcept {
        return lower(i + 1);
    }

private:
    T lo_;
    T range_;
    T inv_width_;
    std::size_t bins_;
};

static_assert(Axis<CircularAxis<double>>);

constexpr CircularAxis<double> circular(double lo, double hi, std::size_t n) noexcept {
    return {lo, hi, n};
}

}
