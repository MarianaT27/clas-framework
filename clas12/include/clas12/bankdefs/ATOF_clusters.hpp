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


/// HIPO bank wrapper for `ATOF::clusters` (group 22500, item 22).
/// Clusters in ATOF
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ATOF_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("ATOF::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ATOF_clusters {
 public:
    static constexpr std::string_view bank_name = "ATOF::clusters";
    static constexpr int bank_group = 22500;
    static constexpr int bank_item  = 22;
    static constexpr std::string_view bank_schema = "id/S,n_bar/I,n_wedge/I,time/F,x/F,y/F,z/F,energy/F,pathlength/F,inpathlength/F,projID/S";

    class Row;

    ATOF_clusters() = default;

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
            int i_n_bar = -1;
            int i_n_wedge = -1;
            int i_time = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_energy = -1;
            int i_pathlength = -1;
            int i_inpathlength = -1;
            int i_projID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_n_bar = schema.exists("n_bar") ? schema.getEntryOrder("n_bar") : -1;
            cache.i_n_wedge = schema.exists("n_wedge") ? schema.getEntryOrder("n_wedge") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_pathlength = schema.exists("pathlength") ? schema.getEntryOrder("pathlength") : -1;
            cache.i_inpathlength = schema.exists("inpathlength") ? schema.getEntryOrder("inpathlength") : -1;
            cache.i_projID = schema.exists("projID") ? schema.getEntryOrder("projID") : -1;
        }
        i_id = cache.i_id;
        i_n_bar = cache.i_n_bar;
        i_n_wedge = cache.i_n_wedge;
        i_time = cache.i_time;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_energy = cache.i_energy;
        i_pathlength = cache.i_pathlength;
        i_inpathlength = cache.i_inpathlength;
        i_projID = cache.i_projID;
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
    /// n_bar: number of hits from the bars
    [[nodiscard]] int n_bar(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_n_bar, row));
    }
    /// n_wedge: number of hits from the wedges
    [[nodiscard]] int n_wedge(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_n_wedge, row));
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
    /// energy: energy in MeV
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// pathlength: path length to the cluster in mm
    [[nodiscard]] float pathlength(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pathlength, row));
    }
    /// inpathlength: path length inside the detector in mm
    [[nodiscard]] float inpathlength(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_inpathlength, row));
    }
    /// projID: id of the projected track matched. it is -1 if a straight track was assigned.
    [[nodiscard]] int projID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_projID, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_n_bar = -1;
    int i_n_wedge = -1;
    int i_time = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_energy = -1;
    int i_pathlength = -1;
    int i_inpathlength = -1;
    int i_projID = -1;

    friend class Row;
};

/// Lightweight row view for ATOF::clusters. Stores a pointer + row index only.
class ATOF_clusters::Row {
 public:
    Row(const ATOF_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int n_bar() const noexcept { return m_bank->n_bar(m_row); }
    [[nodiscard]] int n_wedge() const noexcept { return m_bank->n_wedge(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float pathlength() const noexcept { return m_bank->pathlength(m_row); }
    [[nodiscard]] float inpathlength() const noexcept { return m_bank->inpathlength(m_row); }
    [[nodiscard]] int projID() const noexcept { return m_bank->projID(m_row); }
 private:
    const ATOF_clusters* m_bank;
    int m_row;
};

inline ATOF_clusters::Row ATOF_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
