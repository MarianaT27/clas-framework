// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RECAI::Calorimeter` (group 400, item 32).
/// Calorimeter Responses for Particles bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECAI_Calorimeter bank;
/// if (auto sc = bank.bind(event.get_bank("RECAI::Calorimeter")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class RECAI_Calorimeter {
 public:
    static constexpr std::string_view bank_name = "RECAI::Calorimeter";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 32;
    static constexpr std::string_view bank_schema = "index/S,pindex/S,detector/B,sector/B,layer/B,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,hx/F,hy/F,hz/F,lu/F,lv/F,lw/F,du/F,dv/F,dw/F,m2u/F,m2v/F,m2w/F,m3u/F,m3v/F,m3w/F,status/S";

    class Row;

    RECAI_Calorimeter() = default;

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
            int i_index = -1;
            int i_pindex = -1;
            int i_detector = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_path = -1;
            int i_chi2 = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_hx = -1;
            int i_hy = -1;
            int i_hz = -1;
            int i_lu = -1;
            int i_lv = -1;
            int i_lw = -1;
            int i_du = -1;
            int i_dv = -1;
            int i_dw = -1;
            int i_m2u = -1;
            int i_m2v = -1;
            int i_m2w = -1;
            int i_m3u = -1;
            int i_m3v = -1;
            int i_m3w = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_hx = schema.exists("hx") ? schema.getEntryOrder("hx") : -1;
            cache.i_hy = schema.exists("hy") ? schema.getEntryOrder("hy") : -1;
            cache.i_hz = schema.exists("hz") ? schema.getEntryOrder("hz") : -1;
            cache.i_lu = schema.exists("lu") ? schema.getEntryOrder("lu") : -1;
            cache.i_lv = schema.exists("lv") ? schema.getEntryOrder("lv") : -1;
            cache.i_lw = schema.exists("lw") ? schema.getEntryOrder("lw") : -1;
            cache.i_du = schema.exists("du") ? schema.getEntryOrder("du") : -1;
            cache.i_dv = schema.exists("dv") ? schema.getEntryOrder("dv") : -1;
            cache.i_dw = schema.exists("dw") ? schema.getEntryOrder("dw") : -1;
            cache.i_m2u = schema.exists("m2u") ? schema.getEntryOrder("m2u") : -1;
            cache.i_m2v = schema.exists("m2v") ? schema.getEntryOrder("m2v") : -1;
            cache.i_m2w = schema.exists("m2w") ? schema.getEntryOrder("m2w") : -1;
            cache.i_m3u = schema.exists("m3u") ? schema.getEntryOrder("m3u") : -1;
            cache.i_m3v = schema.exists("m3v") ? schema.getEntryOrder("m3v") : -1;
            cache.i_m3w = schema.exists("m3w") ? schema.getEntryOrder("m3w") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_index = cache.i_index;
        i_pindex = cache.i_pindex;
        i_detector = cache.i_detector;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_path = cache.i_path;
        i_chi2 = cache.i_chi2;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_hx = cache.i_hx;
        i_hy = cache.i_hy;
        i_hz = cache.i_hz;
        i_lu = cache.i_lu;
        i_lv = cache.i_lv;
        i_lw = cache.i_lw;
        i_du = cache.i_du;
        i_dv = cache.i_dv;
        i_dw = cache.i_dw;
        i_m2u = cache.i_m2u;
        i_m2v = cache.i_m2v;
        i_m2w = cache.i_m2w;
        i_m3u = cache.i_m3u;
        i_m3v = cache.i_m3v;
        i_m3w = cache.i_m3w;
        i_status = cache.i_status;
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
    /// index: index of the hit in the specific detector bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }
    /// pindex: row number in the particle bank hit is associated with
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pindex, row));
    }
    /// detector: Detector ID, as defined in org.jlab.detector.base.DetectorType
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// sector: Sector of the Detector hit
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: Layer ID, as defined in org.jlab.detector.base.DetectorLayer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// energy: Energy associated with the hit (GeV)
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Time associated with the hit (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// path: Path from vertex to the hit position (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
    }
    /// chi2: Chi2 (or quality) of hit-track matching
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// x: X coordinate of the hit (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Y coordinate of the hit (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Z coordinate of the hit (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// hx: X coordinate of the matched hit (cm)
    [[nodiscard]] float hx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_hx, row));
    }
    /// hy: Y coordinate of the mathced hit (cm)
    [[nodiscard]] float hy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_hy, row));
    }
    /// hz: Z coordinate of the matched hit (cm)
    [[nodiscard]] float hz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_hz, row));
    }
    /// lu: distance on U-side
    [[nodiscard]] float lu(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lu, row));
    }
    /// lv: distance on V-side
    [[nodiscard]] float lv(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lv, row));
    }
    /// lw: distance on W-side
    [[nodiscard]] float lw(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lw, row));
    }
    /// du: shower width on U-side
    [[nodiscard]] float du(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_du, row));
    }
    /// dv: shower width on V-side
    [[nodiscard]] float dv(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dv, row));
    }
    /// dw: shower width on w-side
    [[nodiscard]] float dw(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dw, row));
    }
    /// m2u: 2nd moment of the shower on U-side
    [[nodiscard]] float m2u(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m2u, row));
    }
    /// m2v: 2nd moment of the shower on V-side
    [[nodiscard]] float m2v(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m2v, row));
    }
    /// m2w: 2nd moment of the shower on W-side
    [[nodiscard]] float m2w(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m2w, row));
    }
    /// m3u: 3rd moment of the shower on U-side
    [[nodiscard]] float m3u(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m3u, row));
    }
    /// m3v: 3rd moment of the shower on V-side
    [[nodiscard]] float m3v(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m3v, row));
    }
    /// m3w: 3rd moment of the shower on W-side
    [[nodiscard]] float m3w(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_m3w, row));
    }
    /// status: hit status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int index_handle() const noexcept { return i_index; }
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_index = -1;
    int i_pindex = -1;
    int i_detector = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_path = -1;
    int i_chi2 = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_hx = -1;
    int i_hy = -1;
    int i_hz = -1;
    int i_lu = -1;
    int i_lv = -1;
    int i_lw = -1;
    int i_du = -1;
    int i_dv = -1;
    int i_dw = -1;
    int i_m2u = -1;
    int i_m2v = -1;
    int i_m2w = -1;
    int i_m3u = -1;
    int i_m3v = -1;
    int i_m3w = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for RECAI::Calorimeter. Stores a pointer + row index only.
class RECAI_Calorimeter::Row {
 public:
    Row(const RECAI_Calorimeter* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float hx() const noexcept { return m_bank->hx(m_row); }
    [[nodiscard]] float hy() const noexcept { return m_bank->hy(m_row); }
    [[nodiscard]] float hz() const noexcept { return m_bank->hz(m_row); }
    [[nodiscard]] float lu() const noexcept { return m_bank->lu(m_row); }
    [[nodiscard]] float lv() const noexcept { return m_bank->lv(m_row); }
    [[nodiscard]] float lw() const noexcept { return m_bank->lw(m_row); }
    [[nodiscard]] float du() const noexcept { return m_bank->du(m_row); }
    [[nodiscard]] float dv() const noexcept { return m_bank->dv(m_row); }
    [[nodiscard]] float dw() const noexcept { return m_bank->dw(m_row); }
    [[nodiscard]] float m2u() const noexcept { return m_bank->m2u(m_row); }
    [[nodiscard]] float m2v() const noexcept { return m_bank->m2v(m_row); }
    [[nodiscard]] float m2w() const noexcept { return m_bank->m2w(m_row); }
    [[nodiscard]] float m3u() const noexcept { return m_bank->m3u(m_row); }
    [[nodiscard]] float m3v() const noexcept { return m_bank->m3v(m_row); }
    [[nodiscard]] float m3w() const noexcept { return m_bank->m3w(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const RECAI_Calorimeter* m_bank;
    int m_row;
};

inline RECAI_Calorimeter::Row RECAI_Calorimeter::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
