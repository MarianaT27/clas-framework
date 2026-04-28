#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace mh {

// Index-based pool of histogram clones for parallel fills.
//
// Usage:
//     mh::thread_pool pool(proto, std::thread::hardware_concurrency());
//     // In each worker thread (tid = 0..N-1):
//     pool[tid].fill(x);
//     // After all threads joined:
//     auto result = pool.merge();
//
// merge() performs a pairwise (tree-shaped) reduction rather than a plain
// left fold — equal total work, but lower floating-point error and a
// structure that can be parallelized per level without API changes.
template <typename Hist>
class thread_pool {
public:
    thread_pool(const Hist& proto, std::size_t n)
        : clones_(n, proto) {}

    [[nodiscard]] Hist&       operator[](std::size_t i)       noexcept { return clones_[i]; }
    [[nodiscard]] const Hist& operator[](std::size_t i) const noexcept { return clones_[i]; }

    [[nodiscard]] std::size_t size() const noexcept { return clones_.size(); }

    [[nodiscard]] Hist merge() const {
        std::vector<Hist> cur = clones_;
        while (cur.size() > 1) {
            std::vector<Hist> next;
            next.reserve((cur.size() + 1) / 2);
            const std::size_t pairs = cur.size() / 2;
            for (std::size_t i = 0; i < pairs; ++i) {
                cur[2 * i] += cur[2 * i + 1];
                next.push_back(std::move(cur[2 * i]));
            }
            if (cur.size() & 1u) next.push_back(std::move(cur.back()));
            cur = std::move(next);
        }
        return std::move(cur.front());
    }

private:
    std::vector<Hist> clones_;
};

}
