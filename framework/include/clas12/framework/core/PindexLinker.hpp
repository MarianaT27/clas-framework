#pragma once

/**
 * @file PindexLinker.hpp
 * @brief Fast lookup of bank rows by pindex (or any int link column).
 *
 * Detector-response banks in CLAS12 point back at REC::Particle rows through
 * an integer column -- usually "pindex", sometimes "hbindex" or "mcindex".
 * Consumers typically want "give me every row in this bank that points at
 * particle N".
 *
 * The old pattern built a std::map<int, std::vector<int>> per call, reading
 * the pindex column one row at a time via a string lookup:
 *
 *     auto index_map = build_index_map(calo_bank, "pindex");  // slow
 *     for (int row : index_map[pindex]) { ... }
 *
 * PindexLinker does two things differently:
 *
 * 1. **Reads the entire link column as a typed span once** using the
 *    column_handle already exposed by generated bank wrappers. There is no
 *    per-row string lookup and no per-row get<T>() dispatch.
 *
 * 2. **Stores the reverse map in a sorted flat array**, indexed by a prefix-
 *    sum table. Lookup by particle index is O(log N) bounds + walk, with no
 *    hash map allocation past build().
 *
 * Usage:
 * @code
 *   bankdefs::REC_Calorimeter calo;
 *   calo.bind(event.get_bank("REC::Calorimeter"));
 *
 *   framework::PindexLinker link;
 *   link.build(calo.view(), calo.pindex_handle());
 *
 *   for (int row : link.rows_for(pindex)) {
 *       auto hit = calo.row(row);
 *       if (hit.layer() == PCAL) ...
 *   }
 * @endcode
 *
 * For generated wrappers that expose a `pindex_handle()`, the free-standing
 * helper `link_by_pindex(bank)` below is the one-liner most callers want.
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <vector>

#include <bank_view.h>
#include <schema.h>

namespace framework {

/**
 * @brief Fast reverse index from "link column value" -> "rows that point at it".
 *
 * After build(), rows_for(key) returns a view over the contiguous run of row
 * indices whose link column equals `key`. The row indices are sorted so
 * callers can iterate in bank order.
 */
class PindexLinker {
   public:
    /// Build the reverse index from a bank_view and a pre-resolved handle.
    /// The handle must refer to a column whose native type is int8/16/32.
    auto build(const hipo::bank_view& view, hipo::column_handle link_handle) -> void {
        m_rows.clear();
        m_offsets.clear();
        m_min_key = 0;

        const int n_rows = view.rows();
        if (n_rows <= 0 || !link_handle.valid() || !view.valid()) {
            return;
        }

        // visit_column dispatches on the column's actual type so we never
        // call get<T>() with the wrong size. The visitor gets a typed span,
        // reads every link value once, then materialises the reverse map.
        view.visit_column(link_handle, [this, n_rows](auto col) {
            using value_t = std::remove_cv_t<typename decltype(col)::value_type>;

            // Pass 1: find key range.
            int key_min = 0;
            int key_max = -1;
            for (int r = 0; r < n_rows; ++r) {
                const int key = static_cast<int>(col[r]);
                if (r == 0 || key < key_min) key_min = key;
                if (r == 0 || key > key_max) key_max = key;
            }
            if (key_max < key_min) return;  // no rows

            m_min_key = key_min;
            const int n_buckets = key_max - key_min + 1;

            // Pass 2: count per bucket.
            m_offsets.assign(n_buckets + 1, 0);
            for (int r = 0; r < n_rows; ++r) {
                const int key = static_cast<int>(col[r]);
                m_offsets[key - key_min + 1]++;
            }
            // Prefix sum -> bucket offsets.
            for (int i = 1; i <= n_buckets; ++i) {
                m_offsets[i] += m_offsets[i - 1];
            }

            // Pass 3: scatter row indices into flat m_rows using a cursor
            // starting at each bucket's offset.
            m_rows.resize(n_rows);
            std::vector<int> cursor(m_offsets.begin(), m_offsets.begin() + n_buckets);
            for (int r = 0; r < n_rows; ++r) {
                const int key = static_cast<int>(col[r]);
                const int b = key - key_min;
                m_rows[cursor[b]++] = r;
            }
            (void)sizeof(value_t);  // suppress unused-type warning for trivial ints
        });
    }

    /// Returns the contiguous run of row indices whose link column equals `key`.
    /// Empty span if no rows match (including when `key` is outside the key range).
    [[nodiscard]] auto rows_for(int key) const noexcept -> std::span<const int> {
        if (m_offsets.empty()) return {};
        const int bucket = key - m_min_key;
        const int n_buckets = static_cast<int>(m_offsets.size()) - 1;
        if (bucket < 0 || bucket >= n_buckets) return {};
        const int begin = m_offsets[bucket];
        const int end = m_offsets[bucket + 1];
        return std::span<const int>(m_rows.data() + begin, end - begin);
    }

    /// True if build() has been called with a non-empty bank.
    [[nodiscard]] auto empty() const noexcept -> bool { return m_rows.empty(); }

    /// Total row count of the bank indexed.
    [[nodiscard]] auto size() const noexcept -> std::size_t { return m_rows.size(); }

   private:
    std::vector<int> m_rows;     ///< Row indices grouped by bucket.
    std::vector<int> m_offsets;  ///< Prefix sum: bucket b is m_rows[m_offsets[b]..m_offsets[b+1]).
    int m_min_key = 0;           ///< The key corresponding to bucket 0.
};

/**
 * @brief Build a PindexLinker for any generated bank wrapper that exposes
 *        a `pindex_handle()` member.
 *
 * Works with bankdefs::REC_Calorimeter, REC_Scintillator, etc.
 */
template <typename BankT>
inline auto link_by_pindex(const BankT& bank) -> PindexLinker {
    PindexLinker linker;
    linker.build(bank.view(), bank.pindex_handle());
    return linker;
}

/**
 * @brief Lazy range over `BankT::Row` proxies for a set of row indices.
 *
 * Holds a pointer into Event's per-event bank cache (`BankT*`) plus a span
 * of row indices (typically obtained from `PindexLinker::rows_for`). Each
 * dereference constructs a `BankT::Row` by value — the Row is a lightweight
 * proxy (pointer + int), so the range carries no allocations and can be
 * copied freely.
 *
 * Any generated wrapper that exposes a nested `Row` type and a
 * `row(int)` accessor works; that covers every bank produced by
 * `scripts/gen_bankdefs.py`.
 */
template <typename BankT>
class PindexRowView {
   public:
    using Row = typename BankT::Row;

    PindexRowView() noexcept = default;
    PindexRowView(const BankT* bank, std::span<const int> rows) noexcept
        : m_bank{bank}, m_rows{rows} {}

    class iterator {
       public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = Row;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = Row;

        iterator() noexcept = default;
        iterator(const BankT* bank, const int* ptr) noexcept
            : m_bank{bank}, m_ptr{ptr} {}

        auto operator*() const noexcept -> Row { return m_bank->row(*m_ptr); }
        auto operator++() noexcept -> iterator& { ++m_ptr; return *this; }
        auto operator++(int) noexcept -> iterator { auto tmp = *this; ++m_ptr; return tmp; }
        auto operator==(const iterator& o) const noexcept -> bool { return m_ptr == o.m_ptr; }
        auto operator!=(const iterator& o) const noexcept -> bool { return m_ptr != o.m_ptr; }

       private:
        const BankT* m_bank = nullptr;
        const int*   m_ptr  = nullptr;
    };

    [[nodiscard]] auto begin() const noexcept -> iterator { return {m_bank, m_rows.data()}; }
    [[nodiscard]] auto end()   const noexcept -> iterator { return {m_bank, m_rows.data() + m_rows.size()}; }
    [[nodiscard]] auto size()  const noexcept -> std::size_t { return m_rows.size(); }
    [[nodiscard]] auto empty() const noexcept -> bool { return m_rows.empty(); }
    [[nodiscard]] auto operator[](std::size_t i) const noexcept -> Row {
        return m_bank->row(m_rows[i]);
    }

    /// Raw row indices, for callers that need the underlying span (e.g. to
    /// cross-reference a sibling bank via `BankT::row(row_index())`).
    [[nodiscard]] auto row_indices() const noexcept -> std::span<const int> { return m_rows; }

    /// Pointer to the underlying bound bank wrapper.
    [[nodiscard]] auto bank() const noexcept -> const BankT* { return m_bank; }

   private:
    const BankT*          m_bank = nullptr;
    std::span<const int>  m_rows{};
};

}  // namespace framework
