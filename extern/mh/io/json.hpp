#pragma once

#include <cstddef>
#include <cstdio>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "mh/accum/count.hpp"
#include "mh/accum/profile.hpp"
#include "mh/accum/weighted.hpp"

namespace mh {
namespace detail {

inline void write_double(std::ostream& out, double v) {
    char buf[32];
    const int n = std::snprintf(buf, sizeof(buf), "%.17g", v);
    out.write(buf, n);
}

template <typename Acc>
void write_bin(std::ostream& out, const Acc& a) {
    if constexpr (std::is_same_v<Acc, CountAccumulator>) {
        out << a.n;
    } else if constexpr (std::is_same_v<Acc, WeightedAccumulator>) {
        out << "{\"sum_w\":";
        write_double(out, a.sum_w);
        out << ",\"sum_w2\":";
        write_double(out, a.sum_w2);
        out << "}";
    } else if constexpr (std::is_same_v<Acc, ProfileAccumulator>) {
        out << "{\"sum_wy\":";
        write_double(out, a.sum_wy);
        out << ",\"sum_wy2\":";
        write_double(out, a.sum_wy2);
        out << ",\"sum_w\":";
        write_double(out, a.sum_w);
        out << ",\"n\":" << a.n << "}";
    } else {
        out << "null";
    }
}

template <typename A>
void write_axis(std::ostream& out, const A& ax) {
    out << "{\"bins\":" << ax.bins() << ",\"edges\":[";
    const std::size_t n = ax.bins();
    for (std::size_t i = 0; i < n; ++i) {
        if (i) out << ',';
        write_double(out, ax.lower(i));
    }
    if (n > 0) {
        out << ',';
        write_double(out, ax.upper(n - 1));
    }
    out << "]}";
}

}

// Serialize a histogram to a JSON object: { "axes": [...], "values": [...] }.
// Bins are emitted in row-major order (last axis contiguous), matching
// the storage layout and what Python numpy.asarray would see.
template <typename Hist>
void to_json(std::ostream& out, const Hist& h) {
    out << "{\"axes\":[";
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ((Is == 0 ? (void)0 : (void)(out << ','),
          detail::write_axis(out, h.template axis<Is>())),
         ...);
    }(std::make_index_sequence<Hist::rank>{});
    out << "],\"values\":[";
    const std::size_t n = h.nbins();
    for (std::size_t i = 0; i < n; ++i) {
        if (i) out << ',';
        detail::write_bin(out, h.at_in_range(i));
    }
    out << "]}";
}

template <typename Hist>
[[nodiscard]] std::string to_json_string(const Hist& h) {
    std::ostringstream oss;
    to_json(oss, h);
    return oss.str();
}

}
