// =============================================================================
// Auto-generated from tof.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `CTOF::clusters` (group 20400, item 22).
/// reconstructed clusters from CTOF
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CTOF_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("CTOF::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CTOF_clusters {
 public:
    static constexpr std::string_view bank_name = "CTOF::clusters";
    static constexpr int bank_group = 20400;
    static constexpr int bank_item  = 22;
    static constexpr std::string_view bank_schema = "id/S,status/S,trackid/S,size/S,sector/B,layer/B,component/S,energy/F,time/F,energy_unc/F,time_unc/F,x/F,y/F,z/F,pathLengthThruBar/F";

    class Row;

    CTOF_clusters() = default;

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
            int i_status = -1;
            int i_trackid = -1;
            int i_size = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_energy_unc = -1;
            int i_time_unc = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_pathLengthThruBar = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_energy_unc = schema.exists("energy_unc") ? schema.getEntryOrder("energy_unc") : -1;
            cache.i_time_unc = schema.exists("time_unc") ? schema.getEntryOrder("time_unc") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_pathLengthThruBar = schema.exists("pathLengthThruBar") ? schema.getEntryOrder("pathLengthThruBar") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_trackid = cache.i_trackid;
        i_size = cache.i_size;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_energy_unc = cache.i_energy_unc;
        i_time_unc = cache.i_time_unc;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_pathLengthThruBar = cache.i_pathLengthThruBar;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// status: status of the cluster seed
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// trackid: matched DC track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trackid, row));
    }
    /// size: cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// sector: sector of CTOF
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: panel id of CTOF
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: paddle id of CTOF cluster seed
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// energy: E dep (MeV) of the cluster
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Cluster seed time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// energy_unc: E dep unc (MeV) of the cluster
    [[nodiscard]] float energy_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_unc, row));
    }
    /// time_unc: Cluster seed time unc (ns)
    [[nodiscard]] float time_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_unc, row));
    }
    /// x: Global X coor (cm) of cluster seed
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Global Y coor (cm) of cluster seed
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Global Z coor (cm) of cluster seed
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// pathLengthThruBar: pathlength of the track from the entrance point to the exit point through the cluster bars 
    [[nodiscard]] float pathLengthThruBar(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pathLengthThruBar, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_trackid = -1;
    int i_size = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_energy_unc = -1;
    int i_time_unc = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_pathLengthThruBar = -1;

    friend class Row;
};

/// Lightweight row view for CTOF::clusters. Stores a pointer + row index only.
class CTOF_clusters::Row {
 public:
    Row(const CTOF_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float energy_unc() const noexcept { return m_bank->energy_unc(m_row); }
    [[nodiscard]] float time_unc() const noexcept { return m_bank->time_unc(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float pathLengthThruBar() const noexcept { return m_bank->pathLengthThruBar(m_row); }
 private:
    const CTOF_clusters* m_bank;
    int m_row;
};

inline CTOF_clusters::Row CTOF_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
