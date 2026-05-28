// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `ATOF::hits` (group 22500, item 21).
/// Reconstructed ATOF hits
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ATOF_hits bank;
/// if (auto sc = bank.bind(event.get_bank("ATOF::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ATOF_hits {
 public:
    static constexpr std::string_view bank_name = "ATOF::hits";
    static constexpr int bank_group = 22500;
    static constexpr int bank_item  = 21;

    class Row;

    ATOF_hits() = default;

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
            int i_id = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_time = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_energy = -1;
            int i_clusterid = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_clusterid = schema.exists("clusterid") ? schema.getEntryOrder("clusterid") : -1;
        }
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_time = cache.i_time;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_energy = cache.i_energy;
        i_clusterid = cache.i_clusterid;
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
    /// id: hit id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// sector: atof sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_sector, row));
    }
    /// layer: atof layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_layer, row));
    }
    /// component: atof component
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_component, row));
    }
    /// time: time in ns
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// x: x position in mm
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y position in mm
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: z position in mm
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// energy: deposited energy in MeV
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// clusterid: id of cluster to which the hit was associated
    [[nodiscard]] int clusterid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterid, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_time = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_energy = -1;
    int i_clusterid = -1;

    friend class Row;
};

/// Lightweight row view for ATOF::hits. Stores a pointer + row index only.
class ATOF_hits::Row {
 public:
    Row(const ATOF_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
 private:
    const ATOF_hits* m_bank;
    int m_row;
};

inline ATOF_hits::Row ATOF_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
