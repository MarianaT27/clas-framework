#pragma once

/**
 * @file Accumulator.hpp
 * @brief Concept for mergeable per-worker partial results.
 *
 * In a coffea-style analysis pattern, each worker thread accumulates
 * a private copy of the result (histogram, cutflow, skim buffer,
 * custom counter ...), and a final merge step sums everything into a
 * single output. This header defines the `Accumulator<T>` concept that
 * any such type must satisfy: `merge(const T&)` folds another partial
 * result into this one, and `reset()` clears it back to the identity.
 *
 * Types modelling the concept:
 *   - `ThreadLocalHistogram1D` / `ThreadLocalHistogram2D`
 *   - `Cutflow`
 *   - User-defined per-worker buffers (e.g. vectors of skimmed events)
 */

#include <concepts>

namespace clas12::framework {

/**
 * @brief A type models `Accumulator<T>` iff partial results of type
 *        `T` can be folded together and reset to the identity.
 *
 * @tparam T The mergeable partial-result type.
 *
 * Required member functions:
 *   - `void merge(const T& other)` - fold `other` into `*this`.
 *   - `void reset()` - restore the identity (empty result).
 */
template <typename T>
concept Accumulator = requires(T& a, const T& b) {
    { a.merge(b) } -> std::same_as<void>;
    { a.reset()  } -> std::same_as<void>;
};

}  // namespace clas12::framework
