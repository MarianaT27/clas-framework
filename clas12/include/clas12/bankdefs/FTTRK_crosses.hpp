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


/// HIPO bank wrapper for `FTTRK::crosses` (group 21300, item 23).
/// Reconstructed crosses in FT tracker
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTTRK_crosses bank;
/// if (auto sc = bank.bind(event.get_bank("FTTRK::crosses")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto size = row.size();
/// }
/// @endcode
class FTTRK_crosses {
 public:
    static constexpr std::string_view bank_name = "FTTRK::crosses";
    static constexpr int bank_group = 21300;
    static constexpr int bank_item  = 23;

    class Row;

    FTTRK_crosses() = default;

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
            int i_size = -1;
            int i_id = -1;
            int i_sector = -1;
            int i_detector = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_Cluster1ID = -1;
            int i_Cluster2ID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_Cluster1ID = schema.exists("Cluster1ID") ? schema.getEntryOrder("Cluster1ID") : -1;
            cache.i_Cluster2ID = schema.exists("Cluster2ID") ? schema.getEntryOrder("Cluster2ID") : -1;
        }
        i_size = cache.i_size;
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_detector = cache.i_detector;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_Cluster1ID = cache.i_Cluster1ID;
        i_Cluster2ID = cache.i_Cluster2ID;
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
    /// size: Size of cross bank
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// id: Cross ID
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// sector: Cross sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// detector: Cross detector
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// x: Cross X position (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Cross Y position (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Cross Z position (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// energy: Cross Energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Cross Time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// Cluster1ID: ID of the upstream layer cluster in the Cross
    [[nodiscard]] int Cluster1ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster1ID, row));
    }
    /// Cluster2ID: ID of the downstream layer cluster in the Cross
    [[nodiscard]] int Cluster2ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster2ID, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_size = -1;
    int i_id = -1;
    int i_sector = -1;
    int i_detector = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_Cluster1ID = -1;
    int i_Cluster2ID = -1;

    friend class Row;
};

/// Lightweight row view for FTTRK::crosses. Stores a pointer + row index only.
class FTTRK_crosses::Row {
 public:
    Row(const FTTRK_crosses* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int Cluster1ID() const noexcept { return m_bank->Cluster1ID(m_row); }
    [[nodiscard]] int Cluster2ID() const noexcept { return m_bank->Cluster2ID(m_row); }
 private:
    const FTTRK_crosses* m_bank;
    int m_row;
};

inline FTTRK_crosses::Row FTTRK_crosses::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
