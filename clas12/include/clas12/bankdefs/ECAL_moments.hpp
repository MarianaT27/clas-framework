// =============================================================================
// Auto-generated from ecal.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `ECAL::moments` (group 20700, item 25).
/// ECCAL bank for clusters containing distances an moments
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ECAL_moments bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::moments")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto distU = row.distU();
/// }
/// @endcode
class ECAL_moments {
 public:
    static constexpr std::string_view bank_name = "ECAL::moments";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 25;
    static constexpr std::string_view bank_schema = "distU/F,distV/F,distW/F,m1u/F,m1v/F,m1w/F,m2u/F,m2v/F,m2w/F,m3u/F,m3v/F,m3w/F";

    class Row;

    ECAL_moments() = default;

    /// Bind to a live hipo::bank. Per-thread cache keyed by the schema's
    /// (group, item): the first bind() in a thread resolves every column
    /// name to its item index via schema.getEntryOrder(); subsequent
    /// bind()s hit the cache. Keying by (group,item) rather than the schema
    /// POINTER is critical: the framework constructs a fresh hipo::bank
    /// (copying its schema to a new address) every event, so a pointer key
    /// would miss every event and re-resolve all columns by string. (group,
    /// item) is stable across those copies, so resolution is paid once per
    /// thread per bank type. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            int gi_group = -1;
            int gi_item  = -2;
            int i_distU = -1;
            int i_distV = -1;
            int i_distW = -1;
            int i_m1u = -1;
            int i_m1v = -1;
            int i_m1w = -1;
            int i_m2u = -1;
            int i_m2v = -1;
            int i_m2w = -1;
            int i_m3u = -1;
            int i_m3v = -1;
            int i_m3w = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_distU = schema.exists("distU") ? schema.getEntryOrder("distU") : -1;
            cache.i_distV = schema.exists("distV") ? schema.getEntryOrder("distV") : -1;
            cache.i_distW = schema.exists("distW") ? schema.getEntryOrder("distW") : -1;
            cache.i_m1u = schema.exists("m1u") ? schema.getEntryOrder("m1u") : -1;
            cache.i_m1v = schema.exists("m1v") ? schema.getEntryOrder("m1v") : -1;
            cache.i_m1w = schema.exists("m1w") ? schema.getEntryOrder("m1w") : -1;
            cache.i_m2u = schema.exists("m2u") ? schema.getEntryOrder("m2u") : -1;
            cache.i_m2v = schema.exists("m2v") ? schema.getEntryOrder("m2v") : -1;
            cache.i_m2w = schema.exists("m2w") ? schema.getEntryOrder("m2w") : -1;
            cache.i_m3u = schema.exists("m3u") ? schema.getEntryOrder("m3u") : -1;
            cache.i_m3v = schema.exists("m3v") ? schema.getEntryOrder("m3v") : -1;
            cache.i_m3w = schema.exists("m3w") ? schema.getEntryOrder("m3w") : -1;
        }
        i_distU = cache.i_distU;
        i_distV = cache.i_distV;
        i_distW = cache.i_distW;
        i_m1u = cache.i_m1u;
        i_m1v = cache.i_m1v;
        i_m1w = cache.i_m1w;
        i_m2u = cache.i_m2u;
        i_m2v = cache.i_m2v;
        i_m2w = cache.i_m2w;
        i_m3u = cache.i_m3u;
        i_m3v = cache.i_m3v;
        i_m3w = cache.i_m3w;
        return true;
    }

    /// Current row count of the bound bank (0 if unbound).
    [[nodiscard]] int rows() const noexcept { return m_view ? m_view->getRows() : 0; }

    /// True if bind() has been called.
    [[nodiscard]] bool valid() const noexcept { return m_view != nullptr; }

    /// Access the underlying hipo::bank (for advanced use).
    [[nodiscard]] const hipo::bank& view() const noexcept { return *m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (item-index based, no string lookup)
    // -------------------------------------------------------------------
    /// distU: distance fomr u edge
    [[nodiscard]] float distU(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_distU, row));
    }
    /// distV: distance from v edge
    [[nodiscard]] float distV(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_distV, row));
    }
    /// distW: distance from w edge
    [[nodiscard]] float distW(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_distW, row));
    }
    /// m1u: second moment
    [[nodiscard]] float m1u(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m1u, row));
    }
    /// m1v: second moment
    [[nodiscard]] float m1v(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m1v, row));
    }
    /// m1w: second moment
    [[nodiscard]] float m1w(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m1w, row));
    }
    /// m2u: second moment
    [[nodiscard]] float m2u(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m2u, row));
    }
    /// m2v: second moment
    [[nodiscard]] float m2v(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m2v, row));
    }
    /// m2w: second moment
    [[nodiscard]] float m2w(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m2w, row));
    }
    /// m3u: third moment
    [[nodiscard]] float m3u(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m3u, row));
    }
    /// m3v: third moment
    [[nodiscard]] float m3v(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m3v, row));
    }
    /// m3w: third moment
    [[nodiscard]] float m3w(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m3w, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_distU = -1;
    int i_distV = -1;
    int i_distW = -1;
    int i_m1u = -1;
    int i_m1v = -1;
    int i_m1w = -1;
    int i_m2u = -1;
    int i_m2v = -1;
    int i_m2w = -1;
    int i_m3u = -1;
    int i_m3v = -1;
    int i_m3w = -1;

    friend class Row;
};

/// Lightweight row view for ECAL::moments. Stores a pointer + row index only.
class ECAL_moments::Row {
 public:
    Row(const ECAL_moments* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float distU() const noexcept { return m_bank->distU(m_row); }
    [[nodiscard]] float distV() const noexcept { return m_bank->distV(m_row); }
    [[nodiscard]] float distW() const noexcept { return m_bank->distW(m_row); }
    [[nodiscard]] float m1u() const noexcept { return m_bank->m1u(m_row); }
    [[nodiscard]] float m1v() const noexcept { return m_bank->m1v(m_row); }
    [[nodiscard]] float m1w() const noexcept { return m_bank->m1w(m_row); }
    [[nodiscard]] float m2u() const noexcept { return m_bank->m2u(m_row); }
    [[nodiscard]] float m2v() const noexcept { return m_bank->m2v(m_row); }
    [[nodiscard]] float m2w() const noexcept { return m_bank->m2w(m_row); }
    [[nodiscard]] float m3u() const noexcept { return m_bank->m3u(m_row); }
    [[nodiscard]] float m3v() const noexcept { return m_bank->m3v(m_row); }
    [[nodiscard]] float m3w() const noexcept { return m_bank->m3w(m_row); }
 private:
    const ECAL_moments* m_bank;
    int m_row;
};

inline ECAL_moments::Row ECAL_moments::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
