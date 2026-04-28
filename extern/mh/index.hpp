#pragma once

#include <array>
#include <cstddef>
#include <tuple>
#include <utility>

#include "mh/axis/concepts.hpp"

namespace mh::detail {

// Product of bins() across axes — the "in-range" cell count, excluding flow.
template <Axis... Axes>
constexpr std::size_t total_bins(const std::tuple<Axes...>& axes) noexcept {
    return std::apply(
        [](const auto&... a) { return (a.bins() * ... * std::size_t{1}); },
        axes);
}

// Product of extent() across axes — the on-disk storage cell count, including
// any per-axis under/overflow slots.
template <Axis... Axes>
constexpr std::size_t total_extent(const std::tuple<Axes...>& axes) noexcept {
    return std::apply(
        [](const auto&... a) { return (a.extent() * ... * std::size_t{1}); },
        axes);
}

// Compute row-major strides over per-axis extent() — the *storage* layout.
// strides[N-1] = 1, strides[k-1] = strides[k] * extent(axis[k]).
template <Axis... Axes, std::size_t N = sizeof...(Axes)>
constexpr std::array<std::size_t, N>
extent_strides(const std::tuple<Axes...>& axes) noexcept {
    std::array<std::size_t, N> strides{};
    std::array<std::size_t, N> extents{};
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ((extents[Is] = std::get<Is>(axes).extent()), ...);
    }(std::make_index_sequence<N>{});
    std::size_t s = 1;
    for (std::size_t i = N; i-- > 0; ) {
        strides[i] = s;
        s *= extents[i];
    }
    return strides;
}

// Per-axis flow offset: 1 if the axis records under/overflow inline (so the
// signed local index `-1` maps to storage slot 0), 0 otherwise.
template <Axis... Axes, std::size_t N = sizeof...(Axes)>
constexpr std::array<std::size_t, N>
flow_offsets(const std::tuple<Axes...>&) noexcept {
    std::array<std::size_t, N> offs{};
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ((offs[Is] = axis_has_flow_v<std::tuple_element_t<Is, std::tuple<Axes...>>> ? 1 : 0),
         ...);
    }(std::make_index_sequence<N>{});
    return offs;
}

// Combine signed per-axis local indices (in [-flow_offset, extent-flow_offset))
// into a flat storage slot using precomputed strides and flow offsets. Hot
// path; intentionally kept branch-free.
template <std::size_t N>
constexpr std::size_t
linear_index(const std::array<std::size_t, N>&  strides,
             const std::array<std::size_t, N>&  flow_offs,
             const std::array<std::ptrdiff_t, N>& local) noexcept {
    std::size_t idx = 0;
    for (std::size_t k = 0; k < N; ++k) {
        idx += static_cast<std::size_t>(
                   local[k] + static_cast<std::ptrdiff_t>(flow_offs[k]))
               * strides[k];
    }
    return idx;
}

// Convenience overload that walks the axis tuple to compute extent-based
// strides on the fly. Use when the histogram's cached strides aren't handy.
template <Axis... Axes, std::size_t N = sizeof...(Axes)>
constexpr std::size_t
linear_index(const std::tuple<Axes...>& axes,
             const std::array<std::ptrdiff_t, N>& local) noexcept {
    return linear_index(extent_strides(axes), flow_offsets(axes), local);
}

}
