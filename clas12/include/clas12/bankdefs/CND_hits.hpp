// =============================================================================
// Auto-generated from cnd.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `CND::hits` (group 20300, item 21).
/// reconstructed hit info from CND
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CND_hits bank;
/// if (auto sc = bank.bind(event.get_bank("CND::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CND_hits {
 public:
    static constexpr std::string_view bank_name = "CND::hits";
    static constexpr int bank_group = 20300;
    static constexpr int bank_item  = 21;

    class Row;

    CND_hits() = default;

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
            int i_trkID = -1;
            int i_clusterid = -1;
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
            int i_x_unc = -1;
            int i_y_unc = -1;
            int i_z_unc = -1;
            int i_tx = -1;
            int i_ty = -1;
            int i_tz = -1;
            int i_tlength = -1;
            int i_pathlength = -1;
            int i_indexLadc = -1;
            int i_indexRadc = -1;
            int i_indexLtdc = -1;
            int i_indexRtdc = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
            cache.i_clusterid = schema.exists("clusterid") ? schema.getEntryOrder("clusterid") : -1;
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
            cache.i_x_unc = schema.exists("x_unc") ? schema.getEntryOrder("x_unc") : -1;
            cache.i_y_unc = schema.exists("y_unc") ? schema.getEntryOrder("y_unc") : -1;
            cache.i_z_unc = schema.exists("z_unc") ? schema.getEntryOrder("z_unc") : -1;
            cache.i_tx = schema.exists("tx") ? schema.getEntryOrder("tx") : -1;
            cache.i_ty = schema.exists("ty") ? schema.getEntryOrder("ty") : -1;
            cache.i_tz = schema.exists("tz") ? schema.getEntryOrder("tz") : -1;
            cache.i_tlength = schema.exists("tlength") ? schema.getEntryOrder("tlength") : -1;
            cache.i_pathlength = schema.exists("pathlength") ? schema.getEntryOrder("pathlength") : -1;
            cache.i_indexLadc = schema.exists("indexLadc") ? schema.getEntryOrder("indexLadc") : -1;
            cache.i_indexRadc = schema.exists("indexRadc") ? schema.getEntryOrder("indexRadc") : -1;
            cache.i_indexLtdc = schema.exists("indexLtdc") ? schema.getEntryOrder("indexLtdc") : -1;
            cache.i_indexRtdc = schema.exists("indexRtdc") ? schema.getEntryOrder("indexRtdc") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_trkID = cache.i_trkID;
        i_clusterid = cache.i_clusterid;
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
        i_x_unc = cache.i_x_unc;
        i_y_unc = cache.i_y_unc;
        i_z_unc = cache.i_z_unc;
        i_tx = cache.i_tx;
        i_ty = cache.i_ty;
        i_tz = cache.i_tz;
        i_tlength = cache.i_tlength;
        i_pathlength = cache.i_pathlength;
        i_indexLadc = cache.i_indexLadc;
        i_indexRadc = cache.i_indexRadc;
        i_indexLtdc = cache.i_indexLtdc;
        i_indexRtdc = cache.i_indexRtdc;
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
    /// status: status of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// trkID: match CVT track index
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trkID, row));
    }
    /// clusterid: pointer to the cluster
    [[nodiscard]] int clusterid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterid, row));
    }
    /// sector: sector of CND
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: panel id of CND
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: paddle id of CND
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// energy: E dep (MeV) of hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Hit time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// energy_unc: E dep unc (MeV) of hit
    [[nodiscard]] float energy_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_unc, row));
    }
    /// time_unc: Hit time unc (ns)
    [[nodiscard]] float time_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_unc, row));
    }
    /// x: Global X coor (cm) of hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Global Y coor (cm) of hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Global Z coor (cm) of hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// x_unc: Global X coor unc (cm) of hit
    [[nodiscard]] float x_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x_unc, row));
    }
    /// y_unc: Global Y coor unc (cm) of hit
    [[nodiscard]] float y_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y_unc, row));
    }
    /// z_unc: Global Z coor unc (cm) of hit
    [[nodiscard]] float z_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z_unc, row));
    }
    /// tx: Global X coor (cm) of hit from CVT info -trkID index
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tx, row));
    }
    /// ty: Global Y coor (cm) of hit from CVT info - trkID index
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ty, row));
    }
    /// tz: Global Z coor (cm) of hit from CVT info - trkID index
    [[nodiscard]] float tz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tz, row));
    }
    /// tlength: pathlength of the track from the entrance point to the exit point through the hit bar
    [[nodiscard]] float tlength(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tlength, row));
    }
    /// pathlength: pathlength of the track from the vertex (doca point to the beamline to the midpoint between the entrance and exit of the hit bar
    [[nodiscard]] float pathlength(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pathlength, row));
    }
    /// indexLadc: index of the left adc
    [[nodiscard]] int indexLadc(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexLadc, row));
    }
    /// indexRadc: index of the right adc
    [[nodiscard]] int indexRadc(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexRadc, row));
    }
    /// indexLtdc: index of the left tdc
    [[nodiscard]] int indexLtdc(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexLtdc, row));
    }
    /// indexRtdc: index of the right tdc
    [[nodiscard]] int indexRtdc(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexRtdc, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_trkID = -1;
    int i_clusterid = -1;
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
    int i_x_unc = -1;
    int i_y_unc = -1;
    int i_z_unc = -1;
    int i_tx = -1;
    int i_ty = -1;
    int i_tz = -1;
    int i_tlength = -1;
    int i_pathlength = -1;
    int i_indexLadc = -1;
    int i_indexRadc = -1;
    int i_indexLtdc = -1;
    int i_indexRtdc = -1;

    friend class Row;
};

/// Lightweight row view for CND::hits. Stores a pointer + row index only.
class CND_hits::Row {
 public:
    Row(const CND_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
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
    [[nodiscard]] float x_unc() const noexcept { return m_bank->x_unc(m_row); }
    [[nodiscard]] float y_unc() const noexcept { return m_bank->y_unc(m_row); }
    [[nodiscard]] float z_unc() const noexcept { return m_bank->z_unc(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] float tlength() const noexcept { return m_bank->tlength(m_row); }
    [[nodiscard]] float pathlength() const noexcept { return m_bank->pathlength(m_row); }
    [[nodiscard]] int indexLadc() const noexcept { return m_bank->indexLadc(m_row); }
    [[nodiscard]] int indexRadc() const noexcept { return m_bank->indexRadc(m_row); }
    [[nodiscard]] int indexLtdc() const noexcept { return m_bank->indexLtdc(m_row); }
    [[nodiscard]] int indexRtdc() const noexcept { return m_bank->indexRtdc(m_row); }
 private:
    const CND_hits* m_bank;
    int m_row;
};

inline CND_hits::Row CND_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
