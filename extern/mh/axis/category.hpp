#pragma once

#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <vector>

#include "mh/axis/concepts.hpp"

namespace mh {

// Discrete categorical axis keyed by Key (e.g. std::string, int, enum).
// Lookup is a linear scan over the key vector — fine for small N (< ~32),
// which matches typical histogram category cardinality.
template <typename Key>
class CategoryAxis {
public:
    using value_type = Key;

    // Discrete axis: unknown keys are discarded (index() returns nullopt).
    // No dedicated flow bins (extent == bins).
    static constexpr bool has_flow = false;

    CategoryAxis() = default;
    CategoryAxis(std::initializer_list<Key> keys) : keys_(keys) {}

    template <std::input_iterator It>
    CategoryAxis(It first, It last) : keys_(first, last) {}

    [[nodiscard]] std::optional<std::ptrdiff_t>
    index(const Key& k) const noexcept {
        for (std::size_t i = 0; i < keys_.size(); ++i) {
            if (keys_[i] == k) return static_cast<std::ptrdiff_t>(i);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::size_t bins()   const noexcept { return keys_.size(); }
    [[nodiscard]] std::size_t extent() const noexcept { return keys_.size(); }

    // Categories have no meaningful lower/upper — report the index itself,
    // in double form, so the Axis concept is satisfied.
    [[nodiscard]] double lower(std::size_t i) const noexcept {
        return static_cast<double>(i);
    }
    [[nodiscard]] double upper(std::size_t i) const noexcept {
        return static_cast<double>(i + 1);
    }

    [[nodiscard]] const Key& key(std::size_t i) const noexcept { return keys_[i]; }

private:
    std::vector<Key> keys_;
};

static_assert(Axis<CategoryAxis<int>>);

}
