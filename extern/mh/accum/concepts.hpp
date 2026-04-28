#pragma once

#include <concepts>
#include <type_traits>

namespace mh {

template <typename Acc>
concept Accumulator = requires(Acc& a, const Acc& ca, double w) {
    Acc{};
    a.fill();
    a.fill(w);
    { ca + ca } -> std::same_as<Acc>;
    a += ca;
} && std::is_trivially_copyable_v<Acc>
  && std::is_default_constructible_v<Acc>;

}
