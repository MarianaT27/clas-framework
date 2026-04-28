#pragma once

/**
 * @file HistogramAliases.hpp
 * @brief Convenience aliases for the standard mh::Histogram shape used in CLAS12.
 *
 * Algorithms own histograms as value members. The default storage is
 * AtomicStorage so concurrent fills from worker threads are lock-free, and
 * the default accumulator is CountAccumulator (unweighted). All axes are
 * RegularAxis<> by default — algorithms that need variable binning,
 * categories, or weighted accumulators reach for `mh::Histogram` directly.
 *
 * Usage:
 * @code
 * framework::H1<> h_p{mh::regular(0.0, 11.0, 100)};
 * framework::H2<> h_p_vs_theta{mh::regular(0.0, 45.0, 90), mh::regular(0.0, 11.0, 100)};
 * h_p.fill(track.p());
 * h_p_vs_theta.fill(track.theta(), track.p());
 * @endcode
 */

#include <mh/mh.hpp>

namespace framework {

template <typename Acc = mh::CountAccumulator>
using H1 = mh::Histogram<Acc, mh::AtomicStorage<Acc>, mh::RegularAxis<>>;

template <typename Acc = mh::CountAccumulator>
using H2 = mh::Histogram<Acc, mh::AtomicStorage<Acc>,
                         mh::RegularAxis<>, mh::RegularAxis<>>;

template <typename Acc = mh::CountAccumulator>
using H3 = mh::Histogram<Acc, mh::AtomicStorage<Acc>,
                         mh::RegularAxis<>, mh::RegularAxis<>, mh::RegularAxis<>>;

}  // namespace framework
