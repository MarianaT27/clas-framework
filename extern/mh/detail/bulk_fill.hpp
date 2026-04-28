#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include "mh/axis/concepts.hpp"

namespace mh::detail {

// Resolve a single value through axis K and store the bin into `locals[K]`.
// For non-flow axes (Category) sets `valid = false` if the lookup misses.
// For flow axes never touches `valid` — the compiler can then drop the check
// entirely when every axis in the pack has flow.
template <std::size_t K, typename Ax, typename T, std::size_t Rank>
inline void resolve_into(const Ax& ax, T x,
                         std::array<std::ptrdiff_t, Rank>& locals,
                         bool& valid) noexcept
{
    if constexpr (axis_has_flow_v<Ax> &&
                  requires(const Ax& a, T v) { a.index_raw(v); }) {
        locals[K] = ax.index_raw(x);
    } else if constexpr (axis_has_flow_v<Ax>) {
        locals[K] = *ax.index(x);
    } else {
        const auto opt = ax.index(x);
        if (!opt) valid = false;
        locals[K] = opt.value_or(0);
    }
}

// Single-pass bulk fill. Per-axis index resolution, flat-index combine, and
// accumulator update fuse for each input row — `locals[K]` and `flat` stay
// in registers across the per-axis fold expression. The validity check is
// elided at compile time when every axis advertises has_flow.
template <typename Hist, std::size_t... Ks, typename... Spans>
std::size_t bulk_fill(Hist& h, std::index_sequence<Ks...>, Spans... cols) noexcept
{
    constexpr std::size_t Rank = Hist::rank;
    static_assert(sizeof...(Ks) == Rank);
    static_assert(sizeof...(Spans) == Rank);

    const std::array<std::size_t, Rank> sizes{ cols.size()... };
    const std::size_t n = sizes[0];
    for (std::size_t k = 1; k < Rank; ++k) {
        if (sizes[k] != n) return 0;
    }
    if (n == 0) return 0;

    using AxesTuple = std::tuple<
        std::remove_cvref_t<decltype(h.template axis<Ks>())>...>;
    constexpr bool any_can_discard =
        ((!axis_has_flow_v<std::tuple_element_t<Ks, AxesTuple>>) || ...);

    auto spans = std::tuple{cols...};
    const auto& strides   = h.strides();
    const auto& flow_offs = h.flow_offsets();

    for (std::size_t i = 0; i < n; ++i) {
        std::array<std::ptrdiff_t, Rank> locals;
        bool valid = true;
        (resolve_into<Ks>(h.template axis<Ks>(),
                          std::get<Ks>(spans)[i], locals, valid), ...);

        if constexpr (any_can_discard) {
            if (!valid) continue;
        }

        std::size_t flat = 0;
        for (std::size_t k = 0; k < Rank; ++k) {
            flat += static_cast<std::size_t>(
                        locals[k] + static_cast<std::ptrdiff_t>(flow_offs[k]))
                    * strides[k];
        }

        if constexpr (Hist::is_atomic) {
            h.storage().atomic_fill(flat);
        } else {
            h.storage()[flat].fill();
        }
    }
    return n;
}

template <typename Hist, std::size_t... Ks, typename... Spans>
std::size_t bulk_fill_weighted(Hist& h,
                               std::span<const double> w,
                               std::index_sequence<Ks...>,
                               Spans... cols) noexcept
{
    constexpr std::size_t Rank = Hist::rank;
    static_assert(sizeof...(Ks) == Rank);
    static_assert(sizeof...(Spans) == Rank);

    const std::array<std::size_t, Rank> sizes{ cols.size()... };
    const std::size_t n = sizes[0];
    for (std::size_t k = 1; k < Rank; ++k) {
        if (sizes[k] != n) return 0;
    }
    if (w.size() != n) return 0;
    if (n == 0) return 0;

    using AxesTuple = std::tuple<
        std::remove_cvref_t<decltype(h.template axis<Ks>())>...>;
    constexpr bool any_can_discard =
        ((!axis_has_flow_v<std::tuple_element_t<Ks, AxesTuple>>) || ...);

    auto spans = std::tuple{cols...};
    const auto& strides   = h.strides();
    const auto& flow_offs = h.flow_offsets();
    const double* wp = w.data();

    for (std::size_t i = 0; i < n; ++i) {
        std::array<std::ptrdiff_t, Rank> locals;
        bool valid = true;
        (resolve_into<Ks>(h.template axis<Ks>(),
                          std::get<Ks>(spans)[i], locals, valid), ...);

        if constexpr (any_can_discard) {
            if (!valid) continue;
        }

        std::size_t flat = 0;
        for (std::size_t k = 0; k < Rank; ++k) {
            flat += static_cast<std::size_t>(
                        locals[k] + static_cast<std::ptrdiff_t>(flow_offs[k]))
                    * strides[k];
        }

        if constexpr (Hist::is_atomic) {
            h.storage().atomic_fill(flat, wp[i]);
        } else {
            h.storage()[flat].fill(wp[i]);
        }
    }
    return n;
}

}
