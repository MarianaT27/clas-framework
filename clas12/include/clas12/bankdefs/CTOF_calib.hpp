// =============================================================================
// Auto-generated from tof.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include <bank_view.h>
#include <schema.h>

namespace clas12::bankdefs {


/// HIPO bank wrapper for `CTOF::calib` (group 20400, item 25).
/// reconstructed hit info from CTOF calibration
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    class Row;

    CTOF_calib() = default;

    /// Bind to a live bank_view. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column handle by name; all
    /// subsequent bind()s for the same schema hit the cache and copy 25-30
    /// trivially-copyable column_handle words.
    /// Returns false if the view is not valid.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        struct HandleCache {
            const hipo::schema* schema_ptr = nullptr;
            hipo::column_handle h_id;
            hipo::column_handle h_status;
            hipo::column_handle h_trackid;
            hipo::column_handle h_component;
            hipo::column_handle h_energy;
            hipo::column_handle h_time;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_tx;
            hipo::column_handle h_ty;
            hipo::column_handle h_tz;
            hipo::column_handle h_adc1;
            hipo::column_handle h_adc2;
            hipo::column_handle h_tdc1;
            hipo::column_handle h_tdc2;
            hipo::column_handle h_pindex;
            hipo::column_handle h_pid;
            hipo::column_handle h_charge;
            hipo::column_handle h_px;
            hipo::column_handle h_py;
            hipo::column_handle h_pz;
            hipo::column_handle h_vx;
            hipo::column_handle h_vy;
            hipo::column_handle h_vz;
            hipo::column_handle h_vt;
            hipo::column_handle h_chi2;
            hipo::column_handle h_NDF;
            hipo::column_handle h_pathLength;
            hipo::column_handle h_pathLengthThruBar;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("trackid"); h) cache.h_trackid = *h;
            else cache.h_trackid = {};
            if (auto h = schema.get_handle("component"); h) cache.h_component = *h;
            else cache.h_component = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("tx"); h) cache.h_tx = *h;
            else cache.h_tx = {};
            if (auto h = schema.get_handle("ty"); h) cache.h_ty = *h;
            else cache.h_ty = {};
            if (auto h = schema.get_handle("tz"); h) cache.h_tz = *h;
            else cache.h_tz = {};
            if (auto h = schema.get_handle("adc1"); h) cache.h_adc1 = *h;
            else cache.h_adc1 = {};
            if (auto h = schema.get_handle("adc2"); h) cache.h_adc2 = *h;
            else cache.h_adc2 = {};
            if (auto h = schema.get_handle("tdc1"); h) cache.h_tdc1 = *h;
            else cache.h_tdc1 = {};
            if (auto h = schema.get_handle("tdc2"); h) cache.h_tdc2 = *h;
            else cache.h_tdc2 = {};
            if (auto h = schema.get_handle("pindex"); h) cache.h_pindex = *h;
            else cache.h_pindex = {};
            if (auto h = schema.get_handle("pid"); h) cache.h_pid = *h;
            else cache.h_pid = {};
            if (auto h = schema.get_handle("charge"); h) cache.h_charge = *h;
            else cache.h_charge = {};
            if (auto h = schema.get_handle("px"); h) cache.h_px = *h;
            else cache.h_px = {};
            if (auto h = schema.get_handle("py"); h) cache.h_py = *h;
            else cache.h_py = {};
            if (auto h = schema.get_handle("pz"); h) cache.h_pz = *h;
            else cache.h_pz = {};
            if (auto h = schema.get_handle("vx"); h) cache.h_vx = *h;
            else cache.h_vx = {};
            if (auto h = schema.get_handle("vy"); h) cache.h_vy = *h;
            else cache.h_vy = {};
            if (auto h = schema.get_handle("vz"); h) cache.h_vz = *h;
            else cache.h_vz = {};
            if (auto h = schema.get_handle("vt"); h) cache.h_vt = *h;
            else cache.h_vt = {};
            if (auto h = schema.get_handle("chi2"); h) cache.h_chi2 = *h;
            else cache.h_chi2 = {};
            if (auto h = schema.get_handle("NDF"); h) cache.h_NDF = *h;
            else cache.h_NDF = {};
            if (auto h = schema.get_handle("pathLength"); h) cache.h_pathLength = *h;
            else cache.h_pathLength = {};
            if (auto h = schema.get_handle("pathLengthThruBar"); h) cache.h_pathLengthThruBar = *h;
            else cache.h_pathLengthThruBar = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_trackid = cache.h_trackid;
        h_component = cache.h_component;
        h_energy = cache.h_energy;
        h_time = cache.h_time;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_tx = cache.h_tx;
        h_ty = cache.h_ty;
        h_tz = cache.h_tz;
        h_adc1 = cache.h_adc1;
        h_adc2 = cache.h_adc2;
        h_tdc1 = cache.h_tdc1;
        h_tdc2 = cache.h_tdc2;
        h_pindex = cache.h_pindex;
        h_pid = cache.h_pid;
        h_charge = cache.h_charge;
        h_px = cache.h_px;
        h_py = cache.h_py;
        h_pz = cache.h_pz;
        h_vx = cache.h_vx;
        h_vy = cache.h_vy;
        h_vz = cache.h_vz;
        h_vt = cache.h_vt;
        h_chi2 = cache.h_chi2;
        h_NDF = cache.h_NDF;
        h_pathLength = cache.h_pathLength;
        h_pathLengthThruBar = cache.h_pathLengthThruBar;
        return true;
    }

    /// Current row count of the bound bank view.
    [[nodiscard]] int rows() const noexcept { return m_view.rows(); }

    /// True if bind() has been called with a valid view.
    [[nodiscard]] bool valid() const noexcept { return m_view.valid(); }

    /// Access the underlying bank_view (for advanced use).
    [[nodiscard]] const hipo::bank_view& view() const noexcept { return m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (handle-based, zero string lookup)
    // -------------------------------------------------------------------
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: hit status defined based on presence (0) or absence (1) of TDCR-TDCL-ADCR-ADCL
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// trackid: matched DC track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trackid, row));
    }
    /// component: paddle id of CTOF
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// energy: E dep (MeV) of hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Hit time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// x: Global X coor (cm) of hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Global Y coor (cm) of hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Global Z coor (cm) of hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// tx: Global X coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tx, row));
    }
    /// ty: Global Y coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ty, row));
    }
    /// tz: Global Z coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float tz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tz, row));
    }
    /// adc1: ADCL
    [[nodiscard]] int adc1(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_adc1, row));
    }
    /// adc2: ADCR
    [[nodiscard]] int adc2(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_adc2, row));
    }
    /// tdc1: TDCL
    [[nodiscard]] int tdc1(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tdc1, row));
    }
    /// tdc2: TDCR
    [[nodiscard]] int tdc2(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tdc2, row));
    }
    /// pindex: particle positionin REC::Particle
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pindex, row));
    }
    /// pid: particle id
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pid, row));
    }
    /// charge: particle charge
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_charge, row));
    }
    /// px: particle momentum x component
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: particle momentum y component
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: particle momentum z component
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// vx: particle vertex x coordinate
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vx, row));
    }
    /// vy: particle vertex y coordinate
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vy, row));
    }
    /// vz: particle vertex z coordinate
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }
    /// vt: vertex-corrected event start time
    [[nodiscard]] float vt(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vt, row));
    }
    /// chi2: track chi2
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// NDF: NDF
    [[nodiscard]] int NDF(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_NDF, row));
    }
    /// pathLength: pathlength of the track from the vertex (doca point to the beamline to the midpoint between the entrance and exit of the hit bar
    [[nodiscard]] float pathLength(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathLength, row));
    }
    /// pathLengthThruBar: pathlength of the track from the entrance point to the exit point through the hit bar 
    [[nodiscard]] float pathLengthThruBar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathLengthThruBar, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// trackid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trackid_col() const noexcept {
        return m_view.column<int16_t>(h_trackid);
    }
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// x column as a contiguous span.
    [[nodiscard]] std::span<const float> x_col() const noexcept {
        return m_view.column<float>(h_x);
    }
    /// y column as a contiguous span.
    [[nodiscard]] std::span<const float> y_col() const noexcept {
        return m_view.column<float>(h_y);
    }
    /// z column as a contiguous span.
    [[nodiscard]] std::span<const float> z_col() const noexcept {
        return m_view.column<float>(h_z);
    }
    /// tx column as a contiguous span.
    [[nodiscard]] std::span<const float> tx_col() const noexcept {
        return m_view.column<float>(h_tx);
    }
    /// ty column as a contiguous span.
    [[nodiscard]] std::span<const float> ty_col() const noexcept {
        return m_view.column<float>(h_ty);
    }
    /// tz column as a contiguous span.
    [[nodiscard]] std::span<const float> tz_col() const noexcept {
        return m_view.column<float>(h_tz);
    }
    /// adc1 column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> adc1_col() const noexcept {
        return m_view.column<int32_t>(h_adc1);
    }
    /// adc2 column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> adc2_col() const noexcept {
        return m_view.column<int32_t>(h_adc2);
    }
    /// tdc1 column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tdc1_col() const noexcept {
        return m_view.column<int32_t>(h_tdc1);
    }
    /// tdc2 column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tdc2_col() const noexcept {
        return m_view.column<int32_t>(h_tdc2);
    }
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pindex_col() const noexcept {
        return m_view.column<int16_t>(h_pindex);
    }
    /// pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pid_col() const noexcept {
        return m_view.column<int32_t>(h_pid);
    }
    /// charge column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> charge_col() const noexcept {
        return m_view.column<int8_t>(h_charge);
    }
    /// px column as a contiguous span.
    [[nodiscard]] std::span<const float> px_col() const noexcept {
        return m_view.column<float>(h_px);
    }
    /// py column as a contiguous span.
    [[nodiscard]] std::span<const float> py_col() const noexcept {
        return m_view.column<float>(h_py);
    }
    /// pz column as a contiguous span.
    [[nodiscard]] std::span<const float> pz_col() const noexcept {
        return m_view.column<float>(h_pz);
    }
    /// vx column as a contiguous span.
    [[nodiscard]] std::span<const float> vx_col() const noexcept {
        return m_view.column<float>(h_vx);
    }
    /// vy column as a contiguous span.
    [[nodiscard]] std::span<const float> vy_col() const noexcept {
        return m_view.column<float>(h_vy);
    }
    /// vz column as a contiguous span.
    [[nodiscard]] std::span<const float> vz_col() const noexcept {
        return m_view.column<float>(h_vz);
    }
    /// vt column as a contiguous span.
    [[nodiscard]] std::span<const float> vt_col() const noexcept {
        return m_view.column<float>(h_vt);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
    }
    /// NDF column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> NDF_col() const noexcept {
        return m_view.column<int16_t>(h_NDF);
    }
    /// pathLength column as a contiguous span.
    [[nodiscard]] std::span<const float> pathLength_col() const noexcept {
        return m_view.column<float>(h_pathLength);
    }
    /// pathLengthThruBar column as a contiguous span.
    [[nodiscard]] std::span<const float> pathLengthThruBar_col() const noexcept {
        return m_view.column<float>(h_pathLengthThruBar);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle pindex_handle() const noexcept { return h_pindex; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_trackid{};
    hipo::column_handle h_component{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_tx{};
    hipo::column_handle h_ty{};
    hipo::column_handle h_tz{};
    hipo::column_handle h_adc1{};
    hipo::column_handle h_adc2{};
    hipo::column_handle h_tdc1{};
    hipo::column_handle h_tdc2{};
    hipo::column_handle h_pindex{};
    hipo::column_handle h_pid{};
    hipo::column_handle h_charge{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_vx{};
    hipo::column_handle h_vy{};
    hipo::column_handle h_vz{};
    hipo::column_handle h_vt{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_NDF{};
    hipo::column_handle h_pathLength{};
    hipo::column_handle h_pathLengthThruBar{};

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

}  // namespace clas12::bankdefs
