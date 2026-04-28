#pragma once

#include <concepts>
#include <cstddef>

namespace mh {

// A Storage owns the per-bin data. Its value_type names the Accumulator.
// Histograms access bins through operator[](size_t) and mutate shape
// through resize/reset. Concurrency-friendly storages additionally
// expose atomic_fill() — see HasAtomicFill below.
template <typename S>
concept Storage = requires(S& s, const S& cs, std::size_t i) {
    typename S::value_type;
    { s[i]      } -> std::same_as<typename S::value_type&>;
    { cs[i]     } -> std::same_as<const typename S::value_type&>;
    { cs.size() } -> std::convertible_to<std::size_t>;
    s.resize(i);
    s.reset();
};

// A Storage is atomic-capable if it exposes atomic_fill(i) and
// atomic_fill(i, w). The Histogram template uses this trait to route
// fill() through the lock-free path when available.
template <typename S>
concept HasAtomicFill = requires(S& s, std::size_t i, double w) {
    s.atomic_fill(i);
    s.atomic_fill(i, w);
};

}
