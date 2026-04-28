#pragma once

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <vector>

#include "mh/axis/concepts.hpp"

namespace mh {

// Variable-width axis with under/overflow bins. index() always returns an
// engaged optional with a value in [-1, bins]. -1 = underflow, bins = overflow.
class VariableAxis {
public:
    using value_type = double;

    // Numeric axis: emits flow indices instead of discarding out-of-range.
    static constexpr bool has_flow = true;

    VariableAxis(std::initializer_list<double> edges) : edges_(edges) {}

    template <std::input_iterator It>
    VariableAxis(It first, It last) : edges_(first, last) {}

    [[nodiscard]] std::optional<std::ptrdiff_t>
    index(double x) const noexcept {
        return index_raw(x);
    }

    [[nodiscard]] std::ptrdiff_t
    index_raw(double x) const noexcept {
        if (x < edges_.front()) return -1;
        if (x >= edges_.back())  return static_cast<std::ptrdiff_t>(edges_.size() - 1);
        const auto it = std::upper_bound(edges_.begin(), edges_.end(), x);
        return std::distance(edges_.begin(), it) - 1;
    }

    [[nodiscard]] std::size_t bins()   const noexcept { return edges_.size() - 1; }
    [[nodiscard]] std::size_t extent() const noexcept { return bins() + 2; }

    [[nodiscard]] double lower(std::size_t i) const noexcept { return edges_[i]; }
    [[nodiscard]] double upper(std::size_t i) const noexcept { return edges_[i + 1]; }

private:
    std::vector<double> edges_;
};

static_assert(Axis<VariableAxis>);
static_assert(axis_has_flow_v<VariableAxis>);

inline VariableAxis variable(std::initializer_list<double> edges) {
    return VariableAxis{edges};
}

}
