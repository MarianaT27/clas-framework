#pragma once

#include <concepts>
#include <cstddef>
#include <optional>
#include <type_traits>

namespace mh {

template <typename A>
concept Axis = requires(const A& a, typename A::value_type x) {
    typename A::value_type;
    { a.index(x)  } noexcept -> std::same_as<std::optional<std::ptrdiff_t>>;
    { a.bins()    } noexcept -> std::convertible_to<std::size_t>;
    { a.extent()  } noexcept -> std::convertible_to<std::size_t>;
    { a.lower(std::size_t{}) } -> std::convertible_to<double>;
    { a.upper(std::size_t{}) } -> std::convertible_to<double>;
};

// Per-axis flow trait. Numeric axes (Regular, Variable) advertise has_flow=true:
// extent() == bins() + 2 and index() returns values in [-1, bins] (always engaged
// — never nullopt). Wrapping or discrete axes (Circular, Category) advertise
// has_flow=false: extent() == bins() and index() may return nullopt to discard.
template <typename A>
inline constexpr bool axis_has_flow_v = []{
    if constexpr (requires { A::has_flow; }) return bool(A::has_flow);
    else return false;
}();

}
