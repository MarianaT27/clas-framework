#pragma once

/**
 * @file AdaptiveBinner4D.hpp
 * @brief Adaptive (kd-tree / recursive-median) 4D binning for ~equal
 *        statistics per bin.
 *
 * A product grid (Q2 x xB x z x pT2) cannot be made equal-occupancy when the
 * variables are correlated: the quantile edges balance each 1D marginal, but
 * the joint grid still has empty corners and heavily over-filled cells.
 *
 * `AdaptiveBinner4D` instead partitions the 4D space into `target_bins`
 * axis-aligned cells, each holding ~the same number of points, by repeatedly
 * splitting the most-populated leaf at the median of its widest (range-
 * normalised) dimension. The result has no empty bins and a near-flat
 * occupancy.
 *
 * Workflow (two passes, like the quantile scan):
 *   1. build() from a representative sample of (Q2, xB, z, pT2) tuples, then
 *      save() the tree.
 *   2. load() the tree in the analysis and call bin_index() per event to map
 *      it to a leaf — that leaf index is the 4D histogram bin.
 *
 * The leaves are axis-aligned boxes, so leaf_boxes() yields each bin's 4D
 * extent for offline interpretation (the bins are irregular, so downstream
 * code needs this table instead of a simple (iQ2,ixB,iz,ipT2) decomposition).
 */

#include <algorithm>
#include <array>
#include <cstddef>
#include <fstream>
#include <limits>
#include <numeric>
#include <queue>
#include <string>
#include <vector>

namespace framework {

template <int NDIM>
class AdaptiveBinnerND {
   public:
    using Point = std::array<double, NDIM>;
    using Box   = std::array<std::array<double, 2>, NDIM>;  // [dim][0=lo,1=hi]

    struct Node {
        int    split_dim  = -1;   ///< -1 => leaf
        double split_val  = 0.0;  ///< x[split_dim] < split_val => left subtree
        int    left       = -1;
        int    right      = -1;
        int    leaf_index = -1;   ///< valid when split_dim == -1
    };

    AdaptiveBinnerND() = default;

    /// Build a tree with up to `target_bins` equal-occupancy leaves from
    /// `points` (consumed). Returns false if target_bins < 1 or there are
    /// fewer points than requested bins.
    bool build(std::vector<Point> points, int target_bins) {
        m_nodes.clear();
        m_n_leaves = 0;
        if (target_bins < 1 || points.size() < static_cast<std::size_t>(target_bins)) {
            return false;
        }

        const std::size_t n = points.size();
        for (int d = 0; d < NDIM; ++d) {
            m_global[d][0] = std::numeric_limits<double>::infinity();
            m_global[d][1] = -std::numeric_limits<double>::infinity();
        }
        for (const auto& p : points) {
            for (int d = 0; d < NDIM; ++d) {
                m_global[d][0] = std::min(m_global[d][0], p[d]);
                m_global[d][1] = std::max(m_global[d][1], p[d]);
            }
        }

        std::vector<int> idx(n);
        std::iota(idx.begin(), idx.end(), 0);

        // Root node (index 0) starts as a leaf covering the whole sample.
        m_nodes.push_back(Node{});

        struct WorkLeaf {
            int node_idx;
            int begin;
            int end;
            int size() const { return end - begin; }
        };
        auto by_size = [](const WorkLeaf& a, const WorkLeaf& b) {
            return a.size() < b.size();  // priority_queue = max-heap on size
        };
        std::priority_queue<WorkLeaf, std::vector<WorkLeaf>, decltype(by_size)> pq(by_size);
        pq.push({0, 0, static_cast<int>(n)});
        int n_leaves = 1;

        while (n_leaves < target_bins && !pq.empty()) {
            const WorkLeaf wl = pq.top();
            pq.pop();
            if (wl.size() < 2) continue;  // cannot split further; stays a leaf

            const int dim = widest_dim(idx, wl.begin, wl.end, points, m_global);
            const int mid = (wl.begin + wl.end) / 2;
            std::nth_element(idx.begin() + wl.begin, idx.begin() + mid,
                             idx.begin() + wl.end,
                             [&](int a, int b) { return points[a][dim] < points[b][dim]; });
            const double split_val = points[idx[mid]][dim];

            const int left_idx  = static_cast<int>(m_nodes.size());
            m_nodes.push_back(Node{});
            const int right_idx = static_cast<int>(m_nodes.size());
            m_nodes.push_back(Node{});

            Node& node = m_nodes[wl.node_idx];
            node.split_dim = dim;
            node.split_val = split_val;
            node.left      = left_idx;
            node.right     = right_idx;

            pq.push({left_idx, wl.begin, mid});
            pq.push({right_idx, mid, wl.end});
            ++n_leaves;  // one leaf became two
        }

        int leaf = 0;
        for (auto& node : m_nodes) {
            if (node.split_dim < 0) node.leaf_index = leaf++;
        }
        m_n_leaves = leaf;
        return m_n_leaves > 0;
    }

    /// Leaf index for a point (0 .. n_bins-1), or -1 if the tree is empty.
    /// Points outside the training range fall into the nearest edge leaf.
    [[nodiscard]] int bin_index(const Point& p) const noexcept {
        if (m_nodes.empty()) return -1;
        int n = 0;
        while (m_nodes[n].split_dim >= 0) {
            const Node& node = m_nodes[n];
            n = (p[node.split_dim] < node.split_val) ? node.left : node.right;
        }
        return m_nodes[n].leaf_index;
    }

    [[nodiscard]] int  n_bins() const noexcept { return m_n_leaves; }
    [[nodiscard]] bool valid()  const noexcept { return m_n_leaves > 0; }

    /// Axis-aligned bounding box of every leaf, indexed by leaf index. Outer
    /// edges are clamped to the global sample range. Useful for offline
    /// interpretation of each 4D bin.
    [[nodiscard]] std::vector<Box> leaf_boxes() const {
        std::vector<Box> boxes(static_cast<std::size_t>(m_n_leaves));
        if (m_nodes.empty()) return boxes;
        descend_box(0, m_global, boxes);
        return boxes;
    }

    // ---- Serialisation (plain text; no external deps) -------------------

    bool save(const std::string& path) const {
        std::ofstream out(path);
        if (!out) return false;
        out << "# AdaptiveBinnerND tree v1, ndim=" << NDIM << "\n";
        out << "ndim " << NDIM << "\n";
        out << "n_leaves " << m_n_leaves << "\n";
        out << "global_lo";
        for (int d = 0; d < NDIM; ++d) out << ' ' << m_global[d][0];
        out << "\nglobal_hi";
        for (int d = 0; d < NDIM; ++d) out << ' ' << m_global[d][1];
        // Columns per node row: idx split_dim split_val left right leaf_index.
        // (No inline comment line here — load() reads the rows immediately
        // after the count.)
        out << "\nnodes " << m_nodes.size() << "\n";
        for (std::size_t i = 0; i < m_nodes.size(); ++i) {
            const Node& nd = m_nodes[i];
            out << i << ' ' << nd.split_dim << ' ' << nd.split_val << ' '
                << nd.left << ' ' << nd.right << ' ' << nd.leaf_index << "\n";
        }
        return static_cast<bool>(out);
    }

    bool load(const std::string& path) {
        std::ifstream in(path);
        if (!in) return false;
        m_nodes.clear();
        m_n_leaves = 0;
        std::string tok;
        std::size_t node_count = 0;
        while (in >> tok) {
            if (tok == "#") { std::string rest; std::getline(in, rest); continue; }
            if (tok == "ndim")     { int v; in >> v; if (v != NDIM) return false; continue; }
            if (tok == "n_leaves") { in >> m_n_leaves; continue; }
            if (tok == "global_lo") { for (int d = 0; d < NDIM; ++d) in >> m_global[d][0]; continue; }
            if (tok == "global_hi") { for (int d = 0; d < NDIM; ++d) in >> m_global[d][1]; continue; }
            if (tok == "nodes") {
                in >> node_count;
                m_nodes.assign(node_count, Node{});
                for (std::size_t k = 0; k < node_count; ++k) {
                    std::size_t i;
                    Node nd;
                    if (!(in >> i >> nd.split_dim >> nd.split_val >> nd.left >> nd.right >> nd.leaf_index))
                        return false;
                    if (i < m_nodes.size()) m_nodes[i] = nd;
                }
                continue;
            }
        }
        return !m_nodes.empty() && m_n_leaves > 0;
    }

   private:
    /// Dimension with the largest range, normalised by that dimension's
    /// global span, over the points in idx[begin, end). The normalisation
    /// lets the disparate scales (Q2 ~ GeV^2, xB ~ 0..1) compete fairly.
    static int widest_dim(const std::vector<int>& idx, int begin, int end,
                          const std::vector<Point>& points, const Box& global) {
        std::array<double, NDIM> lo, hi;
        for (int d = 0; d < NDIM; ++d) {
            lo[d] = std::numeric_limits<double>::infinity();
            hi[d] = -std::numeric_limits<double>::infinity();
        }
        for (int i = begin; i < end; ++i) {
            const Point& p = points[idx[i]];
            for (int d = 0; d < NDIM; ++d) {
                lo[d] = std::min(lo[d], p[d]);
                hi[d] = std::max(hi[d], p[d]);
            }
        }
        int best_dim = 0;
        double best = -1.0;
        for (int d = 0; d < NDIM; ++d) {
            const double gspan = global[d][1] - global[d][0];
            const double spread = (gspan > 0.0) ? (hi[d] - lo[d]) / gspan : 0.0;
            if (spread > best) { best = spread; best_dim = d; }
        }
        return best_dim;
    }

    void descend_box(int node, Box box, std::vector<Box>& out) const {
        const Node& nd = m_nodes[node];
        if (nd.split_dim < 0) {
            if (nd.leaf_index >= 0 && nd.leaf_index < static_cast<int>(out.size()))
                out[static_cast<std::size_t>(nd.leaf_index)] = box;
            return;
        }
        Box lbox = box, rbox = box;
        lbox[nd.split_dim][1] = nd.split_val;  // left:  hi = split
        rbox[nd.split_dim][0] = nd.split_val;  // right: lo = split
        descend_box(nd.left, lbox, out);
        descend_box(nd.right, rbox, out);
    }

    std::vector<Node> m_nodes;       ///< node 0 = root
    int               m_n_leaves = 0;
    Box               m_global{};    ///< global bounding box of the build sample
};

/// 4D (Q2, xB, z, pT2) — pi0 multiplicity-ratio and azimuthal-modulation trees.
using AdaptiveBinner4D = AdaptiveBinnerND<4>;
/// 3D (Q2, xB, z) — pi0 pT-broadening tree (integrates over pT).
using AdaptiveBinner3D = AdaptiveBinnerND<3>;

}  // namespace framework
