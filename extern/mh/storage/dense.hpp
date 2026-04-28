#pragma once

#include <cstddef>
#include <vector>

#include "mh/accum/concepts.hpp"
#include "mh/storage/concepts.hpp"

namespace mh {

template <Accumulator Acc>
class DenseStorage {
public:
    using value_type = Acc;

    DenseStorage() = default;
    explicit DenseStorage(std::size_t n) : data_(n) {}

    void resize(std::size_t n) { data_.assign(n, Acc{}); }
    void reset() { std::fill(data_.begin(), data_.end(), Acc{}); }

    [[nodiscard]] Acc&       operator[](std::size_t i)       noexcept { return data_[i]; }
    [[nodiscard]] const Acc& operator[](std::size_t i) const noexcept { return data_[i]; }

    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }

    [[nodiscard]] Acc*       data()       noexcept { return data_.data(); }
    [[nodiscard]] const Acc* data() const noexcept { return data_.data(); }

    [[nodiscard]] auto begin()       noexcept { return data_.begin(); }
    [[nodiscard]] auto end()         noexcept { return data_.end(); }
    [[nodiscard]] auto begin() const noexcept { return data_.begin(); }
    [[nodiscard]] auto end()   const noexcept { return data_.end(); }

    DenseStorage& operator+=(const DenseStorage& rhs) noexcept {
        const auto n = size();
        for (std::size_t i = 0; i < n; ++i) data_[i] += rhs.data_[i];
        return *this;
    }

private:
    std::vector<Acc> data_;
};

}
