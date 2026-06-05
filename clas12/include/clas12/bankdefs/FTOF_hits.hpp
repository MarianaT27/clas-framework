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


/// HIPO bank wrapper for `FTOF::hits` (group 21200, item 31).
/// reconstructed hit info from FTOF
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTOF_hits bank;
/// if (auto sc = bank.bind(event.get_bank("FTOF::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class FTOF_hits {
 public:
    static constexpr std::string_view bank_name = "FTOF::hits";
    static constexpr int bank_group = 21200;
    static constexpr int bank_item  = 31;
    static constexpr std::string_view bank_schema = "id/S,status/S,trackid/S,sector/B,layer/B,component/S,energy/F,time/F,energy_unc/F,time_unc/F,x/F,y/F,z/F,x_unc/F,y_unc/F,z_unc/F,tx/F,ty/F,tz/F,adc_idx1/S,adc_idx2/S,tdc_idx1/S,tdc_idx2/S,clusterid/S,pathLength/F,pathLengthThruBar/F";

    class Row;

    FTOF_hits() = default;

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
            int i_adc_idx1 = -1;
            int i_adc_idx2 = -1;
            int i_tdc_idx1 = -1;
            int i_tdc_idx2 = -1;
            int i_clusterid = -1;
            int i_pathLength = -1;
            int i_pathLengthThruBar = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
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
            cache.i_adc_idx1 = schema.exists("adc_idx1") ? schema.getEntryOrder("adc_idx1") : -1;
            cache.i_adc_idx2 = schema.exists("adc_idx2") ? schema.getEntryOrder("adc_idx2") : -1;
            cache.i_tdc_idx1 = schema.exists("tdc_idx1") ? schema.getEntryOrder("tdc_idx1") : -1;
            cache.i_tdc_idx2 = schema.exists("tdc_idx2") ? schema.getEntryOrder("tdc_idx2") : -1;
            cache.i_clusterid = schema.exists("clusterid") ? schema.getEntryOrder("clusterid") : -1;
            cache.i_pathLength = schema.exists("pathLength") ? schema.getEntryOrder("pathLength") : -1;
            cache.i_pathLengthThruBar = schema.exists("pathLengthThruBar") ? schema.getEntryOrder("pathLengthThruBar") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_trackid = cache.i_trackid;
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
        i_adc_idx1 = cache.i_adc_idx1;
        i_adc_idx2 = cache.i_adc_idx2;
        i_tdc_idx1 = cache.i_tdc_idx1;
        i_tdc_idx2 = cache.i_tdc_idx2;
        i_clusterid = cache.i_clusterid;
        i_pathLength = cache.i_pathLength;
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
    /// status: hit status defined based on presence (0) or absence (1) of TDCR-TDCL-ADCR-ADCL
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// trackid: matched DC track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trackid, row));
    }
    /// sector: sector of FTOF
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: panel id of FTOF (1-1A, 2-1B, 3-2
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: paddle id of FTOF
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
    /// tx: Global X coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tx, row));
    }
    /// ty: Global Y coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ty, row));
    }
    /// tz: Global Z coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float tz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tz, row));
    }
    /// adc_idx1: index of hit with ADCL in adc bank, starts at zero
    [[nodiscard]] int adc_idx1(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_adc_idx1, row));
    }
    /// adc_idx2: index of hit with ADCR in adc bank, starts at zero
    [[nodiscard]] int adc_idx2(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_adc_idx2, row));
    }
    /// tdc_idx1: index of hit with TDCL in tdc bank, starts at zero
    [[nodiscard]] int tdc_idx1(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_tdc_idx1, row));
    }
    /// tdc_idx2: index of hit with TDCR in tdc bank, starts at zero
    [[nodiscard]] int tdc_idx2(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_tdc_idx2, row));
    }
    /// clusterid: pointer to the cluster
    [[nodiscard]] int clusterid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterid, row));
    }
    /// pathLength: pathlength of the track from the vertex (doca point to the beamline to the midpoint between the entrance and exit of the hit bar
    [[nodiscard]] float pathLength(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pathLength, row));
    }
    /// pathLengthThruBar: pathlength of the track from the entrance point to the exit point through the hit bar 
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
    int i_adc_idx1 = -1;
    int i_adc_idx2 = -1;
    int i_tdc_idx1 = -1;
    int i_tdc_idx2 = -1;
    int i_clusterid = -1;
    int i_pathLength = -1;
    int i_pathLengthThruBar = -1;

    friend class Row;
};

/// Lightweight row view for FTOF::hits. Stores a pointer + row index only.
class FTOF_hits::Row {
 public:
    Row(const FTOF_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
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
    [[nodiscard]] int adc_idx1() const noexcept { return m_bank->adc_idx1(m_row); }
    [[nodiscard]] int adc_idx2() const noexcept { return m_bank->adc_idx2(m_row); }
    [[nodiscard]] int tdc_idx1() const noexcept { return m_bank->tdc_idx1(m_row); }
    [[nodiscard]] int tdc_idx2() const noexcept { return m_bank->tdc_idx2(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
    [[nodiscard]] float pathLength() const noexcept { return m_bank->pathLength(m_row); }
    [[nodiscard]] float pathLengthThruBar() const noexcept { return m_bank->pathLengthThruBar(m_row); }
 private:
    const FTOF_hits* m_bank;
    int m_row;
};

inline FTOF_hits::Row FTOF_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
