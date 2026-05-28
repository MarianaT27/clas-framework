// =============================================================================
// Auto-generated from ft.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `FTCAL::hits` (group 21000, item 21).
/// Reconstructed Hits in FT calorimeter
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTCAL_hits bank;
/// if (auto sc = bank.bind(event.get_bank("FTCAL::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto idx = row.idx();
/// }
/// @endcode
class FTCAL_hits {
 public:
    static constexpr std::string_view bank_name = "FTCAL::hits";
    static constexpr int bank_group = 21000;
    static constexpr int bank_item  = 21;

    class Row;

    FTCAL_hits() = default;

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
            int i_idx = -1;
            int i_idy = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_hitID = -1;
            int i_clusterID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_idx = schema.exists("idx") ? schema.getEntryOrder("idx") : -1;
            cache.i_idy = schema.exists("idy") ? schema.getEntryOrder("idy") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_hitID = schema.exists("hitID") ? schema.getEntryOrder("hitID") : -1;
            cache.i_clusterID = schema.exists("clusterID") ? schema.getEntryOrder("clusterID") : -1;
        }
        i_idx = cache.i_idx;
        i_idy = cache.i_idy;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_hitID = cache.i_hitID;
        i_clusterID = cache.i_clusterID;
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
    /// idx: x id of grid
    [[nodiscard]] int idx(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_idx, row));
    }
    /// idy: y id of grid
    [[nodiscard]] int idy(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_idy, row));
    }
    /// x: Hit X position (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Hit Y position (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Hit Z position (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// energy: Hit Energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Hit Time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// hitID: Hit Pointer to ADC bank
    [[nodiscard]] int hitID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_hitID, row));
    }
    /// clusterID: Hit Pointer to Cluster Bank
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterID, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_idx = -1;
    int i_idy = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_hitID = -1;
    int i_clusterID = -1;

    friend class Row;
};

/// Lightweight row view for FTCAL::hits. Stores a pointer + row index only.
class FTCAL_hits::Row {
 public:
    Row(const FTCAL_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int idx() const noexcept { return m_bank->idx(m_row); }
    [[nodiscard]] int idy() const noexcept { return m_bank->idy(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int hitID() const noexcept { return m_bank->hitID(m_row); }
    [[nodiscard]] int clusterID() const noexcept { return m_bank->clusterID(m_row); }
 private:
    const FTCAL_hits* m_bank;
    int m_row;
};

inline FTCAL_hits::Row FTCAL_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
