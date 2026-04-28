#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include "mh/accum/concepts.hpp"
#include "mh/axis/concepts.hpp"
#include "mh/detail/bulk_fill.hpp"
#include "mh/index.hpp"
#include "mh/storage/concepts.hpp"
#include "mh/storage/dense.hpp"

namespace mh {

template <Accumulator Acc,
          Storage     St = DenseStorage<Acc>,
          Axis...     Axes>
class Histogram {
    static_assert((Axis<Axes> && ...),
                  "mh: all axis template arguments must satisfy mh::Axis.");
    static_assert(Accumulator<Acc>,
                  "mh: the Accumulator template argument must satisfy mh::Accumulator.");
    static_assert(std::is_same_v<typename St::value_type, Acc>,
                  "mh: Storage::value_type must match the Accumulator type.");

public:
    using accumulator_type = Acc;
    using storage_type     = St;
    static constexpr std::size_t rank = sizeof...(Axes);
    static constexpr bool        is_atomic = HasAtomicFill<St>;

    explicit Histogram(Axes... axes)
        : axes_(std::move(axes)...) {
        init_layout_();
        init_storage_(std::index_sequence_for<Axes...>{});
    }

    // Scalar fill. Each axis's index() lookup may discard (CategoryAxis miss);
    // numeric (flow) axes always succeed and route into under/overflow when
    // out-of-range.
    template <typename... Xs>
        requires (sizeof...(Xs) == rank)
    void fill(Xs... xs) noexcept {
        const auto idx = resolve_index_(std::index_sequence_for<Xs...>{}, xs...);
        if (!idx) return;
        if constexpr (is_atomic) data_.atomic_fill(*idx);
        else                     data_[*idx].fill();
    }

    template <typename... Xs>
        requires (sizeof...(Xs) == rank)
    void fill_weighted(double w, Xs... xs) noexcept {
        const auto idx = resolve_index_(std::index_sequence_for<Xs...>{}, xs...);
        if (!idx) return;
        if constexpr (is_atomic) data_.atomic_fill(*idx, w);
        else                     data_[*idx].fill(w);
    }

    // Profile fill: y is the profiled quantity at bin (xs...). Same dispatch
    // as fill_weighted but routes through Accumulator::fill(y).
    template <typename... Xs>
        requires (sizeof...(Xs) == rank)
    void fill_profile(double y, Xs... xs) noexcept {
        const auto idx = resolve_index_(std::index_sequence_for<Xs...>{}, xs...);
        if (!idx) return;
        data_[*idx].fill(y);
    }

    // Bulk N-D fill: each Span is std::span<const T_k> over the k-th axis's
    // input column. All spans must have equal length (debug assert). Returns
    // the number of points consumed.
    template <typename... Spans>
        requires (sizeof...(Spans) == rank)
    std::size_t fill_n(Spans... cols) noexcept {
        return detail::bulk_fill(*this, std::index_sequence_for<Spans...>{}, cols...);
    }

    template <typename... Spans>
        requires (sizeof...(Spans) == rank)
    std::size_t fill_n_weighted(std::span<const double> w, Spans... cols) noexcept {
        return detail::bulk_fill_weighted(*this, w,
                                          std::index_sequence_for<Spans...>{},
                                          cols...);
    }

    [[nodiscard]] const Acc& at(std::size_t i) const noexcept { return data_[i]; }
    [[nodiscard]] Acc&       at(std::size_t i)       noexcept { return data_[i]; }

    // at_nd(is...) — `is...` are signed; `-1` selects the underflow slot,
    // `bins(k)` selects the overflow slot, `[0, bins(k))` selects in-range.
    template <typename... Is>
        requires (sizeof...(Is) == rank)
    [[nodiscard]] const Acc& at_nd(Is... is) const noexcept {
        const std::array<std::ptrdiff_t, rank> local{
            static_cast<std::ptrdiff_t>(is)...};
        return data_[detail::linear_index(strides_, flow_offsets_, local)];
    }

    template <typename... Is>
        requires (sizeof...(Is) == rank)
    [[nodiscard]] Acc& at_nd(Is... is) noexcept {
        const std::array<std::ptrdiff_t, rank> local{
            static_cast<std::ptrdiff_t>(is)...};
        return data_[detail::linear_index(strides_, flow_offsets_, local)];
    }

    // Access the i-th in-range bin in row-major order, skipping flow bins.
    // i ∈ [0, nbins()). Use this for serialization and "iterate over the
    // observable histogram" walks.
    [[nodiscard]] const Acc& at_in_range(std::size_t i) const noexcept {
        return data_[in_range_to_flat_(i)];
    }
    [[nodiscard]] Acc& at_in_range(std::size_t i) noexcept {
        return data_[in_range_to_flat_(i)];
    }

    // Total in-range bin count: product of axis<K>().bins() over all axes.
    [[nodiscard]] std::size_t nbins() const noexcept {
        return detail::total_bins(axes_);
    }

    // Total storage cell count, including any inline flow bins.
    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

    // Cached row-major strides over the storage layout (extent-based).
    [[nodiscard]] const std::array<std::size_t, rank>& strides() const noexcept {
        return strides_;
    }
    [[nodiscard]] const std::array<std::size_t, rank>& flow_offsets() const noexcept {
        return flow_offsets_;
    }

    template <std::size_t K>
    [[nodiscard]] const auto& axis() const noexcept { return std::get<K>(axes_); }

    [[nodiscard]] const std::tuple<Axes...>& axes() const noexcept { return axes_; }

    [[nodiscard]] const storage_type& storage() const noexcept { return data_; }
    [[nodiscard]] storage_type&       storage()       noexcept { return data_; }

    Histogram& operator+=(const Histogram& rhs) noexcept {
        data_ += rhs.data_;
        return *this;
    }

private:
    void init_layout_() noexcept {
        strides_      = detail::extent_strides(axes_);
        flow_offsets_ = detail::flow_offsets(axes_);
    }

    template <std::size_t... Is>
    void init_storage_(std::index_sequence<Is...>) {
        if constexpr (requires(storage_type& s) {
                          s.reshape(std::get<Is>(axes_).extent()...);
                      }) {
            data_.reshape(std::get<Is>(axes_).extent()...);
        } else {
            data_.resize(detail::total_extent(axes_));
        }
    }

    // For a given axis K and value x, returns the engaged signed bin index
    // (with flow encoding for numeric axes), or nullopt if the axis discarded.
    template <std::size_t K, typename X>
    [[nodiscard]] std::optional<std::ptrdiff_t>
    query_one_(X x) const noexcept {
        using Ax = std::tuple_element_t<K, std::tuple<Axes...>>;
        return std::get<K>(axes_).index(static_cast<typename Ax::value_type>(x));
    }

    template <std::size_t... Ks, typename... Xs>
    [[nodiscard]] std::optional<std::size_t>
    resolve_index_(std::index_sequence<Ks...>, Xs... xs) const noexcept {
        const std::array<std::optional<std::ptrdiff_t>, rank> maybe{
            query_one_<Ks, Xs>(xs)...};
        for (std::size_t k = 0; k < rank; ++k) {
            if (!maybe[k].has_value()) return std::nullopt;
        }
        std::array<std::ptrdiff_t, rank> local{};
        for (std::size_t k = 0; k < rank; ++k) local[k] = *maybe[k];
        return detail::linear_index(strides_, flow_offsets_, local);
    }

    // Decompose a row-major in-range index i ∈ [0, nbins()) into per-axis
    // [0, bins(k)) coords, then map to the flat storage slot via the
    // flow-aware linear_index.
    [[nodiscard]] std::size_t in_range_to_flat_(std::size_t i) const noexcept {
        std::array<std::ptrdiff_t, rank> local{};
        std::array<std::size_t, rank>    bins{};
        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((bins[Is] = std::get<Is>(axes_).bins()), ...);
        }(std::index_sequence_for<Axes...>{});
        for (std::size_t k = rank; k-- > 0; ) {
            local[k] = static_cast<std::ptrdiff_t>(i % bins[k]);
            i /= bins[k];
        }
        return detail::linear_index(strides_, flow_offsets_, local);
    }

    std::tuple<Axes...>             axes_;
    storage_type                    data_;
    std::array<std::size_t, rank>   strides_{};
    std::array<std::size_t, rank>   flow_offsets_{};
};

// Default factory: DenseStorage is chosen implicitly.
template <Accumulator Acc, Axis... Axes>
Histogram<Acc, DenseStorage<Acc>, Axes...> make_histogram(Axes... axes) {
    return Histogram<Acc, DenseStorage<Acc>, Axes...>{std::move(axes)...};
}

// Explicit-storage factory: user passes Storage as a second template arg.
// Use via: mh::make_histogram_with<Acc, AtomicStorage<Acc>>(regular(...))
template <Accumulator Acc, Storage St, Axis... Axes>
Histogram<Acc, St, Axes...> make_histogram_with(Axes... axes) {
    return Histogram<Acc, St, Axes...>{std::move(axes)...};
}

}
