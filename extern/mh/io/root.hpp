#pragma once

// Convert mh::Histogram (rank 1-3) into a CERN ROOT TH1D / TH2D / TH3D.
//
// This header is opt-in: include the relevant ROOT headers before this one
// (or just keep them visible on the include path). The mh core has no ROOT
// dependency — including <mh/mh.hpp> alone never pulls ROOT in.
//
// Usage:
//
//   #include <TFile.h>
//   #include <TH1.h>
//   #include <TH2.h>
//   #include <TH3.h>
//   #include <mh/mh.hpp>
//   #include <mh/io/root.hpp>
//
//   auto h = mh::make_histogram<mh::WeightedAccumulator>(mh::regular(0., 1., 50));
//   // ... fill h ...
//   auto* th = mh::to_root(h, "myhist", "My histogram");   // TH1D*
//   TFile f("out.root", "RECREATE");
//   th->Write();
//
// Caller owns the returned pointer. Variable, Regular, and Circular axes are
// handled (Circular flow slots are just zero in ROOT). CategoryAxis is
// rejected at compile time since ROOT axes don't map to discrete keys.

#include <cmath>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include "mh/accum/count.hpp"
#include "mh/accum/profile.hpp"
#include "mh/accum/weighted.hpp"
#include "mh/axis/category.hpp"
#include "mh/axis/circular.hpp"
#include "mh/axis/concepts.hpp"
#include "mh/axis/regular.hpp"
#include "mh/axis/variable.hpp"
#include "mh/histogram.hpp"

#if !__has_include("TH1.h")
# error "mh/io/root.hpp requires ROOT — add ROOT's include path or omit this header."
#endif

#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

namespace mh {

namespace detail {

// Build a vector of N+1 edges from a Regular, Variable, or Circular axis.
template <typename Ax>
std::vector<double> axis_edges(const Ax& ax) {
    std::vector<double> e;
    e.reserve(ax.bins() + 1);
    for (std::size_t i = 0; i < ax.bins(); ++i) e.push_back(ax.lower(i));
    e.push_back(ax.upper(ax.bins() - 1));
    return e;
}

template <typename Acc>
double acc_value(const Acc& a) noexcept {
    if constexpr (std::is_same_v<Acc, CountAccumulator>) {
        return static_cast<double>(a.n);
    } else if constexpr (std::is_same_v<Acc, WeightedAccumulator>) {
        return a.sum_w;
    } else if constexpr (std::is_same_v<Acc, ProfileAccumulator>) {
        return a.mean();
    } else {
        return 0.0;
    }
}

template <typename Acc>
double acc_variance(const Acc& a) noexcept {
    if constexpr (std::is_same_v<Acc, CountAccumulator>) {
        return static_cast<double>(a.n);          // Poisson: σ² = N
    } else if constexpr (std::is_same_v<Acc, WeightedAccumulator>) {
        return a.sum_w2;
    } else if constexpr (std::is_same_v<Acc, ProfileAccumulator>) {
        return a.variance();
    } else {
        return 0.0;
    }
}

// Map a ROOT 1-based bin index (with 0 / N+1 reserved for flow) to the mh
// signed local coordinate. Returns (valid, local). valid == false means the
// ROOT bin has no mh counterpart (e.g. flow on a no-flow axis) and should
// be left at zero.
template <typename Ax>
inline std::pair<bool, std::ptrdiff_t>
local_for_root_bin(const Ax& ax, int root_bin) noexcept {
    const int N = static_cast<int>(ax.bins());
    if (root_bin >= 1 && root_bin <= N) {
        return {true, static_cast<std::ptrdiff_t>(root_bin - 1)};
    }
    if constexpr (axis_has_flow_v<Ax>) {
        if (root_bin == 0)     return {true, -1};
        if (root_bin == N + 1) return {true, static_cast<std::ptrdiff_t>(N)};
    }
    (void)ax;
    return {false, 0};
}

// Compile-time guard: ROOT can't represent discrete-key axes, so reject
// CategoryAxis explicitly with a readable diagnostic.
template <typename Ax>
struct is_category_axis : std::false_type {};
template <typename K>
struct is_category_axis<CategoryAxis<K>> : std::true_type {};

template <typename... Axes>
inline constexpr bool any_category_axis_v =
    (is_category_axis<std::remove_cvref_t<Axes>>::value || ...);

}

// 1-D conversion. Returns a heap-allocated TH1D; caller takes ownership.
template <typename Acc, typename St, typename Ax>
TH1D* to_root(const Histogram<Acc, St, Ax>& h,
              const char* name = "mh_h1",
              const char* title = "") {
    static_assert(!detail::any_category_axis_v<Ax>,
                  "mh::to_root: CategoryAxis cannot be represented as a ROOT TH axis.");
    const auto& ax = h.template axis<0>();
    const auto edges = detail::axis_edges(ax);
    auto* th = new TH1D(name, title, static_cast<int>(ax.bins()), edges.data());
    th->Sumw2();

    const int N = static_cast<int>(ax.bins());
    for (int i = 0; i <= N + 1; ++i) {
        const auto [ok, li] = detail::local_for_root_bin(ax, i);
        if (!ok) continue;
        const auto& acc = h.at_nd(li);
        th->SetBinContent(i, detail::acc_value(acc));
        th->SetBinError(i,   std::sqrt(detail::acc_variance(acc)));
    }
    th->SetEntries(static_cast<double>(h.size()));  // best-effort; ROOT uses this for stats
    return th;
}

// 2-D conversion. Returns a heap-allocated TH2D; caller takes ownership.
template <typename Acc, typename St, typename Ax0, typename Ax1>
TH2D* to_root(const Histogram<Acc, St, Ax0, Ax1>& h,
              const char* name = "mh_h2",
              const char* title = "") {
    static_assert(!detail::any_category_axis_v<Ax0, Ax1>,
                  "mh::to_root: CategoryAxis cannot be represented as a ROOT TH axis.");
    const auto& ax0 = h.template axis<0>();
    const auto& ax1 = h.template axis<1>();
    const auto e0 = detail::axis_edges(ax0);
    const auto e1 = detail::axis_edges(ax1);
    auto* th = new TH2D(name, title,
                        static_cast<int>(ax0.bins()), e0.data(),
                        static_cast<int>(ax1.bins()), e1.data());
    th->Sumw2();

    const int N0 = static_cast<int>(ax0.bins());
    const int N1 = static_cast<int>(ax1.bins());
    for (int i = 0; i <= N0 + 1; ++i) {
        const auto [okI, li] = detail::local_for_root_bin(ax0, i);
        if (!okI) continue;
        for (int j = 0; j <= N1 + 1; ++j) {
            const auto [okJ, lj] = detail::local_for_root_bin(ax1, j);
            if (!okJ) continue;
            const auto& acc = h.at_nd(li, lj);
            th->SetBinContent(i, j, detail::acc_value(acc));
            th->SetBinError(i, j,   std::sqrt(detail::acc_variance(acc)));
        }
    }
    return th;
}

// 3-D conversion. Returns a heap-allocated TH3D; caller takes ownership.
template <typename Acc, typename St, typename Ax0, typename Ax1, typename Ax2>
TH3D* to_root(const Histogram<Acc, St, Ax0, Ax1, Ax2>& h,
              const char* name = "mh_h3",
              const char* title = "") {
    static_assert(!detail::any_category_axis_v<Ax0, Ax1, Ax2>,
                  "mh::to_root: CategoryAxis cannot be represented as a ROOT TH axis.");
    const auto& ax0 = h.template axis<0>();
    const auto& ax1 = h.template axis<1>();
    const auto& ax2 = h.template axis<2>();
    const auto e0 = detail::axis_edges(ax0);
    const auto e1 = detail::axis_edges(ax1);
    const auto e2 = detail::axis_edges(ax2);
    auto* th = new TH3D(name, title,
                        static_cast<int>(ax0.bins()), e0.data(),
                        static_cast<int>(ax1.bins()), e1.data(),
                        static_cast<int>(ax2.bins()), e2.data());
    th->Sumw2();

    const int N0 = static_cast<int>(ax0.bins());
    const int N1 = static_cast<int>(ax1.bins());
    const int N2 = static_cast<int>(ax2.bins());
    for (int i = 0; i <= N0 + 1; ++i) {
        const auto [okI, li] = detail::local_for_root_bin(ax0, i);
        if (!okI) continue;
        for (int j = 0; j <= N1 + 1; ++j) {
            const auto [okJ, lj] = detail::local_for_root_bin(ax1, j);
            if (!okJ) continue;
            for (int k = 0; k <= N2 + 1; ++k) {
                const auto [okK, lk] = detail::local_for_root_bin(ax2, k);
                if (!okK) continue;
                const auto& acc = h.at_nd(li, lj, lk);
                th->SetBinContent(i, j, k, detail::acc_value(acc));
                th->SetBinError(i, j, k,   std::sqrt(detail::acc_variance(acc)));
            }
        }
    }
    return th;
}

}
