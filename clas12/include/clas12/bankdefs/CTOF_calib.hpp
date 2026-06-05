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


/// HIPO bank wrapper for `CTOF::calib` (group 20400, item 25).
/// reconstructed hit info from CTOF calibration
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CTOF_calib bank;
/// if (auto sc = bank.bind(event.get_bank("CTOF::calib")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CTOF_calib {
 public:
    static constexpr std::string_view bank_name = "CTOF::calib";
    static constexpr int bank_group = 20400;
    static constexpr int bank_item  = 25;
    static constexpr std::string_view bank_schema = "id/S,status/S,trackid/S,component/S,energy/F,time/F,x/F,y/F,z/F,tx/F,ty/F,tz/F,adc1/I,adc2/I,tdc1/I,tdc2/I,pindex/S,pid/I,charge/B,px/F,py/F,pz/F,vx/F,vy/F,vz/F,vt/F,chi2/F,NDF/S,pathLength/F,pathLengthThruBar/F";

    class Row;

    CTOF_calib() = default;

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
            int i_component = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_tx = -1;
            int i_ty = -1;
            int i_tz = -1;
            int i_adc1 = -1;
            int i_adc2 = -1;
            int i_tdc1 = -1;
            int i_tdc2 = -1;
            int i_pindex = -1;
            int i_pid = -1;
            int i_charge = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_vx = -1;
            int i_vy = -1;
            int i_vz = -1;
            int i_vt = -1;
            int i_chi2 = -1;
            int i_NDF = -1;
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
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_tx = schema.exists("tx") ? schema.getEntryOrder("tx") : -1;
            cache.i_ty = schema.exists("ty") ? schema.getEntryOrder("ty") : -1;
            cache.i_tz = schema.exists("tz") ? schema.getEntryOrder("tz") : -1;
            cache.i_adc1 = schema.exists("adc1") ? schema.getEntryOrder("adc1") : -1;
            cache.i_adc2 = schema.exists("adc2") ? schema.getEntryOrder("adc2") : -1;
            cache.i_tdc1 = schema.exists("tdc1") ? schema.getEntryOrder("tdc1") : -1;
            cache.i_tdc2 = schema.exists("tdc2") ? schema.getEntryOrder("tdc2") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_pid = schema.exists("pid") ? schema.getEntryOrder("pid") : -1;
            cache.i_charge = schema.exists("charge") ? schema.getEntryOrder("charge") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_vx = schema.exists("vx") ? schema.getEntryOrder("vx") : -1;
            cache.i_vy = schema.exists("vy") ? schema.getEntryOrder("vy") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
            cache.i_vt = schema.exists("vt") ? schema.getEntryOrder("vt") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_NDF = schema.exists("NDF") ? schema.getEntryOrder("NDF") : -1;
            cache.i_pathLength = schema.exists("pathLength") ? schema.getEntryOrder("pathLength") : -1;
            cache.i_pathLengthThruBar = schema.exists("pathLengthThruBar") ? schema.getEntryOrder("pathLengthThruBar") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_trackid = cache.i_trackid;
        i_component = cache.i_component;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_tx = cache.i_tx;
        i_ty = cache.i_ty;
        i_tz = cache.i_tz;
        i_adc1 = cache.i_adc1;
        i_adc2 = cache.i_adc2;
        i_tdc1 = cache.i_tdc1;
        i_tdc2 = cache.i_tdc2;
        i_pindex = cache.i_pindex;
        i_pid = cache.i_pid;
        i_charge = cache.i_charge;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_vx = cache.i_vx;
        i_vy = cache.i_vy;
        i_vz = cache.i_vz;
        i_vt = cache.i_vt;
        i_chi2 = cache.i_chi2;
        i_NDF = cache.i_NDF;
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
    /// component: paddle id of CTOF
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
    /// adc1: ADCL
    [[nodiscard]] int adc1(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_adc1, row));
    }
    /// adc2: ADCR
    [[nodiscard]] int adc2(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_adc2, row));
    }
    /// tdc1: TDCL
    [[nodiscard]] int tdc1(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tdc1, row));
    }
    /// tdc2: TDCR
    [[nodiscard]] int tdc2(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tdc2, row));
    }
    /// pindex: particle positionin REC::Particle
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pindex, row));
    }
    /// pid: particle id
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pid, row));
    }
    /// charge: particle charge
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_charge, row));
    }
    /// px: particle momentum x component
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: particle momentum y component
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: particle momentum z component
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// vx: particle vertex x coordinate
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vx, row));
    }
    /// vy: particle vertex y coordinate
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vy, row));
    }
    /// vz: particle vertex z coordinate
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }
    /// vt: vertex-corrected event start time
    [[nodiscard]] float vt(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vt, row));
    }
    /// chi2: track chi2
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// NDF: NDF
    [[nodiscard]] int NDF(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_NDF, row));
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

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_trackid = -1;
    int i_component = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_tx = -1;
    int i_ty = -1;
    int i_tz = -1;
    int i_adc1 = -1;
    int i_adc2 = -1;
    int i_tdc1 = -1;
    int i_tdc2 = -1;
    int i_pindex = -1;
    int i_pid = -1;
    int i_charge = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_vx = -1;
    int i_vy = -1;
    int i_vz = -1;
    int i_vt = -1;
    int i_chi2 = -1;
    int i_NDF = -1;
    int i_pathLength = -1;
    int i_pathLengthThruBar = -1;

    friend class Row;
};

/// Lightweight row view for CTOF::calib. Stores a pointer + row index only.
class CTOF_calib::Row {
 public:
    Row(const CTOF_calib* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] int adc1() const noexcept { return m_bank->adc1(m_row); }
    [[nodiscard]] int adc2() const noexcept { return m_bank->adc2(m_row); }
    [[nodiscard]] int tdc1() const noexcept { return m_bank->tdc1(m_row); }
    [[nodiscard]] int tdc2() const noexcept { return m_bank->tdc2(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] int charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float vx() const noexcept { return m_bank->vx(m_row); }
    [[nodiscard]] float vy() const noexcept { return m_bank->vy(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float vt() const noexcept { return m_bank->vt(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int NDF() const noexcept { return m_bank->NDF(m_row); }
    [[nodiscard]] float pathLength() const noexcept { return m_bank->pathLength(m_row); }
    [[nodiscard]] float pathLengthThruBar() const noexcept { return m_bank->pathLengthThruBar(m_row); }
 private:
    const CTOF_calib* m_bank;
    int m_row;
};

inline CTOF_calib::Row CTOF_calib::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
