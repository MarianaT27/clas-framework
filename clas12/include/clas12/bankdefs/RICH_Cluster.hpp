// =============================================================================
// Auto-generated from rich.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RICH::Cluster` (group 21800, item 42).
/// Reconstructed Clusters in RICH
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RICH_Cluster bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Cluster")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Cluster {
 public:
    static constexpr std::string_view bank_name = "RICH::Cluster";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 42;

    class Row;

    RICH_Cluster() = default;

    /// Bind to a live hipo::bank. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column name to its item
    /// index via schema.getEntryOrder(); subsequent bind()s for the same
    /// schema hit the cache. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            const hipo::schema* schema_ptr = nullptr;
            int i_id = -1;
            int i_size = -1;
            int i_sector = -1;
            int i_tile = -1;
            int i_pmt = -1;
            int i_charge = -1;
            int i_time = -1;
            int i_rawtime = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_wtime = -1;
            int i_wx = -1;
            int i_wy = -1;
            int i_wz = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_tile = schema.exists("tile") ? schema.getEntryOrder("tile") : -1;
            cache.i_pmt = schema.exists("pmt") ? schema.getEntryOrder("pmt") : -1;
            cache.i_charge = schema.exists("charge") ? schema.getEntryOrder("charge") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_rawtime = schema.exists("rawtime") ? schema.getEntryOrder("rawtime") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_wtime = schema.exists("wtime") ? schema.getEntryOrder("wtime") : -1;
            cache.i_wx = schema.exists("wx") ? schema.getEntryOrder("wx") : -1;
            cache.i_wy = schema.exists("wy") ? schema.getEntryOrder("wy") : -1;
            cache.i_wz = schema.exists("wz") ? schema.getEntryOrder("wz") : -1;
        }
        i_id = cache.i_id;
        i_size = cache.i_size;
        i_sector = cache.i_sector;
        i_tile = cache.i_tile;
        i_pmt = cache.i_pmt;
        i_charge = cache.i_charge;
        i_time = cache.i_time;
        i_rawtime = cache.i_rawtime;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_wtime = cache.i_wtime;
        i_wx = cache.i_wx;
        i_wy = cache.i_wy;
        i_wz = cache.i_wz;
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
    /// id: Cluster id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// size: Cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// sector: Cluster sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_sector, row));
    }
    /// tile: Cluster tile
    [[nodiscard]] int tile(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_tile, row));
    }
    /// pmt: Cluster pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pmt, row));
    }
    /// charge: Cluster charge
    [[nodiscard]] float charge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_charge, row));
    }
    /// time: Cluster time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// rawtime: Cluster rawtime
    [[nodiscard]] float rawtime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_rawtime, row));
    }
    /// x: Cluster x
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Cluster y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Cluster z
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// wtime: Cluster wtime
    [[nodiscard]] float wtime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_wtime, row));
    }
    /// wx: Cluster wx
    [[nodiscard]] float wx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_wx, row));
    }
    /// wy: Cluster wy
    [[nodiscard]] float wy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_wy, row));
    }
    /// wz: Cluster wz
    [[nodiscard]] float wz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_wz, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_size = -1;
    int i_sector = -1;
    int i_tile = -1;
    int i_pmt = -1;
    int i_charge = -1;
    int i_time = -1;
    int i_rawtime = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_wtime = -1;
    int i_wx = -1;
    int i_wy = -1;
    int i_wz = -1;

    friend class Row;
};

/// Lightweight row view for RICH::Cluster. Stores a pointer + row index only.
class RICH_Cluster::Row {
 public:
    Row(const RICH_Cluster* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int tile() const noexcept { return m_bank->tile(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] float charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float rawtime() const noexcept { return m_bank->rawtime(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float wtime() const noexcept { return m_bank->wtime(m_row); }
    [[nodiscard]] float wx() const noexcept { return m_bank->wx(m_row); }
    [[nodiscard]] float wy() const noexcept { return m_bank->wy(m_row); }
    [[nodiscard]] float wz() const noexcept { return m_bank->wz(m_row); }
 private:
    const RICH_Cluster* m_bank;
    int m_row;
};

inline RICH_Cluster::Row RICH_Cluster::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
