#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include "mh/accum/concepts.hpp"

#ifndef MH_NO_MDSPAN
#  if __has_include(<mdspan>)
#    include <mdspan>
#    define MH_HAS_MDSPAN 1
#  endif
#endif

namespace mh {

// StaticStorage is a DenseStorage variant that additionally exposes an
// std::mdspan view for N-D access without manual stride math. Use when
// you want to iterate over the storage as a multidimensional array
// (e.g. for custom printing, serialization to NumPy) without recomputing
// strides via detail::linear_index.
//
// On compilers without <mdspan> (pre-GCC 13 / libc++ < 18), define
// MH_NO_MDSPAN to fall back to a simple linear-indexing view.
template <Accumulator Acc, std::size_t Rank>
class StaticStorage {
public:
    using value_type = Acc;
    static constexpr std::size_t rank = Rank;

    StaticStorage() { shape_.fill(0); }

    explicit StaticStorage(std::size_t n) : buffer_(n) {
        shape_.fill(0);
        if constexpr (Rank == 1) shape_[0] = n;
    }

    void resize(std::size_t n) {
        buffer_.assign(n, Acc{});
        shape_.fill(0);
        if constexpr (Rank == 1) shape_[0] = n;
    }

    template <typename... Dims>
        requires (sizeof...(Dims) == Rank)
    void reshape(Dims... dims) {
        const std::array<std::size_t, Rank> s{static_cast<std::size_t>(dims)...};
        std::size_t total = 1;
        for (auto d : s) total *= d;
        buffer_.assign(total, Acc{});
        shape_ = s;
    }

    void reset() { std::fill(buffer_.begin(), buffer_.end(), Acc{}); }

    [[nodiscard]] Acc&       operator[](std::size_t i)       noexcept { return buffer_[i]; }
    [[nodiscard]] const Acc& operator[](std::size_t i) const noexcept { return buffer_[i]; }

    [[nodiscard]] std::size_t size() const noexcept { return buffer_.size(); }

    [[nodiscard]] Acc*       data()       noexcept { return buffer_.data(); }
    [[nodiscard]] const Acc* data() const noexcept { return buffer_.data(); }

    [[nodiscard]] const std::array<std::size_t, Rank>& shape() const noexcept { return shape_; }

#ifdef MH_HAS_MDSPAN
    using extents_type = std::dextents<std::size_t, Rank>;
    using mdspan_type  = std::mdspan<Acc, extents_type>;
    using const_mdspan_type = std::mdspan<const Acc, extents_type>;

    [[nodiscard]] mdspan_type view() noexcept {
        return make_view_(std::make_index_sequence<Rank>{});
    }
    [[nodiscard]] const_mdspan_type view() const noexcept {
        return make_const_view_(std::make_index_sequence<Rank>{});
    }
#endif

    StaticStorage& operator+=(const StaticStorage& rhs) noexcept {
        const auto n = size();
        for (std::size_t i = 0; i < n; ++i) buffer_[i] += rhs.buffer_[i];
        return *this;
    }

private:
#ifdef MH_HAS_MDSPAN
    template <std::size_t... Is>
    mdspan_type make_view_(std::index_sequence<Is...>) noexcept {
        return mdspan_type{buffer_.data(), extents_type{shape_[Is]...}};
    }
    template <std::size_t... Is>
    const_mdspan_type make_const_view_(std::index_sequence<Is...>) const noexcept {
        return const_mdspan_type{buffer_.data(), extents_type{shape_[Is]...}};
    }
#endif

    std::vector<Acc>               buffer_;
    std::array<std::size_t, Rank>  shape_{};
};

}
