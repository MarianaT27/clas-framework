// =============================================================================
// Auto-generated from event.json - do not edit by hand.
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


/// HIPO bank wrapper for `REC::Calorimeter` (group 300, item 32).
/// Calorimeter Responses for Particles bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// REC_Calorimeter bank;
/// if (auto sc = bank.bind(event.get_bank("REC::Calorimeter")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class REC_Calorimeter {
 public:
    static constexpr std::string_view bank_name = "REC::Calorimeter";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 32;

    class Row;

    REC_Calorimeter() = default;

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
            hipo::column_handle h_index;
            hipo::column_handle h_pindex;
            hipo::column_handle h_detector;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_energy;
            hipo::column_handle h_time;
            hipo::column_handle h_path;
            hipo::column_handle h_chi2;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_hx;
            hipo::column_handle h_hy;
            hipo::column_handle h_hz;
            hipo::column_handle h_lu;
            hipo::column_handle h_lv;
            hipo::column_handle h_lw;
            hipo::column_handle h_du;
            hipo::column_handle h_dv;
            hipo::column_handle h_dw;
            hipo::column_handle h_m2u;
            hipo::column_handle h_m2v;
            hipo::column_handle h_m2w;
            hipo::column_handle h_m3u;
            hipo::column_handle h_m3v;
            hipo::column_handle h_m3w;
            hipo::column_handle h_status;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("index"); h) cache.h_index = *h;
            else cache.h_index = {};
            if (auto h = schema.get_handle("pindex"); h) cache.h_pindex = *h;
            else cache.h_pindex = {};
            if (auto h = schema.get_handle("detector"); h) cache.h_detector = *h;
            else cache.h_detector = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("path"); h) cache.h_path = *h;
            else cache.h_path = {};
            if (auto h = schema.get_handle("chi2"); h) cache.h_chi2 = *h;
            else cache.h_chi2 = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("hx"); h) cache.h_hx = *h;
            else cache.h_hx = {};
            if (auto h = schema.get_handle("hy"); h) cache.h_hy = *h;
            else cache.h_hy = {};
            if (auto h = schema.get_handle("hz"); h) cache.h_hz = *h;
            else cache.h_hz = {};
            if (auto h = schema.get_handle("lu"); h) cache.h_lu = *h;
            else cache.h_lu = {};
            if (auto h = schema.get_handle("lv"); h) cache.h_lv = *h;
            else cache.h_lv = {};
            if (auto h = schema.get_handle("lw"); h) cache.h_lw = *h;
            else cache.h_lw = {};
            if (auto h = schema.get_handle("du"); h) cache.h_du = *h;
            else cache.h_du = {};
            if (auto h = schema.get_handle("dv"); h) cache.h_dv = *h;
            else cache.h_dv = {};
            if (auto h = schema.get_handle("dw"); h) cache.h_dw = *h;
            else cache.h_dw = {};
            if (auto h = schema.get_handle("m2u"); h) cache.h_m2u = *h;
            else cache.h_m2u = {};
            if (auto h = schema.get_handle("m2v"); h) cache.h_m2v = *h;
            else cache.h_m2v = {};
            if (auto h = schema.get_handle("m2w"); h) cache.h_m2w = *h;
            else cache.h_m2w = {};
            if (auto h = schema.get_handle("m3u"); h) cache.h_m3u = *h;
            else cache.h_m3u = {};
            if (auto h = schema.get_handle("m3v"); h) cache.h_m3v = *h;
            else cache.h_m3v = {};
            if (auto h = schema.get_handle("m3w"); h) cache.h_m3w = *h;
            else cache.h_m3w = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
        }
        h_index = cache.h_index;
        h_pindex = cache.h_pindex;
        h_detector = cache.h_detector;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_energy = cache.h_energy;
        h_time = cache.h_time;
        h_path = cache.h_path;
        h_chi2 = cache.h_chi2;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_hx = cache.h_hx;
        h_hy = cache.h_hy;
        h_hz = cache.h_hz;
        h_lu = cache.h_lu;
        h_lv = cache.h_lv;
        h_lw = cache.h_lw;
        h_du = cache.h_du;
        h_dv = cache.h_dv;
        h_dw = cache.h_dw;
        h_m2u = cache.h_m2u;
        h_m2v = cache.h_m2v;
        h_m2w = cache.h_m2w;
        h_m3u = cache.h_m3u;
        h_m3v = cache.h_m3v;
        h_m3w = cache.h_m3w;
        h_status = cache.h_status;
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
    /// index: index of the hit in the specific detector bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// pindex: row number in the particle bank hit is associated with
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pindex, row));
    }
    /// detector: Detector ID, as defined in org.jlab.detector.base.DetectorType
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_detector, row));
    }
    /// sector: Sector of the Detector hit
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: Layer ID, as defined in org.jlab.detector.base.DetectorLayer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// energy: Energy associated with the hit (GeV)
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Time associated with the hit (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// path: Path from vertex to the hit position (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }
    /// chi2: Chi2 (or quality) of hit-track matching
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// x: X coordinate of the hit (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Y coordinate of the hit (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Z coordinate of the hit (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// hx: X coordinate of the matched hit (cm)
    [[nodiscard]] float hx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_hx, row));
    }
    /// hy: Y coordinate of the mathced hit (cm)
    [[nodiscard]] float hy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_hy, row));
    }
    /// hz: Z coordinate of the matched hit (cm)
    [[nodiscard]] float hz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_hz, row));
    }
    /// lu: distance on U-side
    [[nodiscard]] float lu(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lu, row));
    }
    /// lv: distance on V-side
    [[nodiscard]] float lv(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lv, row));
    }
    /// lw: distance on W-side
    [[nodiscard]] float lw(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lw, row));
    }
    /// du: shower width on U-side
    [[nodiscard]] float du(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_du, row));
    }
    /// dv: shower width on V-side
    [[nodiscard]] float dv(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dv, row));
    }
    /// dw: shower width on w-side
    [[nodiscard]] float dw(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dw, row));
    }
    /// m2u: 2nd moment of the shower on U-side
    [[nodiscard]] float m2u(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m2u, row));
    }
    /// m2v: 2nd moment of the shower on V-side
    [[nodiscard]] float m2v(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m2v, row));
    }
    /// m2w: 2nd moment of the shower on W-side
    [[nodiscard]] float m2w(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m2w, row));
    }
    /// m3u: 3rd moment of the shower on U-side
    [[nodiscard]] float m3u(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m3u, row));
    }
    /// m3v: 3rd moment of the shower on V-side
    [[nodiscard]] float m3v(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m3v, row));
    }
    /// m3w: 3rd moment of the shower on W-side
    [[nodiscard]] float m3w(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m3w, row));
    }
    /// status: hit status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index_col() const noexcept {
        return m_view.column<int16_t>(h_index);
    }
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pindex_col() const noexcept {
        return m_view.column<int16_t>(h_pindex);
    }
    /// detector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> detector_col() const noexcept {
        return m_view.column<int8_t>(h_detector);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// path column as a contiguous span.
    [[nodiscard]] std::span<const float> path_col() const noexcept {
        return m_view.column<float>(h_path);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
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
    /// hx column as a contiguous span.
    [[nodiscard]] std::span<const float> hx_col() const noexcept {
        return m_view.column<float>(h_hx);
    }
    /// hy column as a contiguous span.
    [[nodiscard]] std::span<const float> hy_col() const noexcept {
        return m_view.column<float>(h_hy);
    }
    /// hz column as a contiguous span.
    [[nodiscard]] std::span<const float> hz_col() const noexcept {
        return m_view.column<float>(h_hz);
    }
    /// lu column as a contiguous span.
    [[nodiscard]] std::span<const float> lu_col() const noexcept {
        return m_view.column<float>(h_lu);
    }
    /// lv column as a contiguous span.
    [[nodiscard]] std::span<const float> lv_col() const noexcept {
        return m_view.column<float>(h_lv);
    }
    /// lw column as a contiguous span.
    [[nodiscard]] std::span<const float> lw_col() const noexcept {
        return m_view.column<float>(h_lw);
    }
    /// du column as a contiguous span.
    [[nodiscard]] std::span<const float> du_col() const noexcept {
        return m_view.column<float>(h_du);
    }
    /// dv column as a contiguous span.
    [[nodiscard]] std::span<const float> dv_col() const noexcept {
        return m_view.column<float>(h_dv);
    }
    /// dw column as a contiguous span.
    [[nodiscard]] std::span<const float> dw_col() const noexcept {
        return m_view.column<float>(h_dw);
    }
    /// m2u column as a contiguous span.
    [[nodiscard]] std::span<const float> m2u_col() const noexcept {
        return m_view.column<float>(h_m2u);
    }
    /// m2v column as a contiguous span.
    [[nodiscard]] std::span<const float> m2v_col() const noexcept {
        return m_view.column<float>(h_m2v);
    }
    /// m2w column as a contiguous span.
    [[nodiscard]] std::span<const float> m2w_col() const noexcept {
        return m_view.column<float>(h_m2w);
    }
    /// m3u column as a contiguous span.
    [[nodiscard]] std::span<const float> m3u_col() const noexcept {
        return m_view.column<float>(h_m3u);
    }
    /// m3v column as a contiguous span.
    [[nodiscard]] std::span<const float> m3v_col() const noexcept {
        return m_view.column<float>(h_m3v);
    }
    /// m3w column as a contiguous span.
    [[nodiscard]] std::span<const float> m3w_col() const noexcept {
        return m_view.column<float>(h_m3w);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle index_handle() const noexcept { return h_index; }
    [[nodiscard]] hipo::column_handle pindex_handle() const noexcept { return h_pindex; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_index{};
    hipo::column_handle h_pindex{};
    hipo::column_handle h_detector{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_path{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_hx{};
    hipo::column_handle h_hy{};
    hipo::column_handle h_hz{};
    hipo::column_handle h_lu{};
    hipo::column_handle h_lv{};
    hipo::column_handle h_lw{};
    hipo::column_handle h_du{};
    hipo::column_handle h_dv{};
    hipo::column_handle h_dw{};
    hipo::column_handle h_m2u{};
    hipo::column_handle h_m2v{};
    hipo::column_handle h_m2w{};
    hipo::column_handle h_m3u{};
    hipo::column_handle h_m3v{};
    hipo::column_handle h_m3w{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for REC::Calorimeter. Stores a pointer + row index only.
class REC_Calorimeter::Row {
 public:
    Row(const REC_Calorimeter* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
    const REC_Calorimeter* m_bank;
    int m_row;
};

inline REC_Calorimeter::Row REC_Calorimeter::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
