// =============================================================================
// Auto-generated from band.json - do not edit by hand.
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


/// HIPO bank wrapper for `BAND::laser` (group 22100, item 23).
/// laser hits from BAND
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// BAND_laser bank;
/// if (auto sc = bank.bind(event.get_bank("BAND::laser")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class BAND_laser {
 public:
    static constexpr std::string_view bank_name = "BAND::laser";
    static constexpr int bank_group = 22100;
    static constexpr int bank_item  = 23;

    class Row;

    BAND_laser() = default;

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
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_component;
            hipo::column_handle h_energy;
            hipo::column_handle h_time;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_ex;
            hipo::column_handle h_ey;
            hipo::column_handle h_ez;
            hipo::column_handle h_timeFadc;
            hipo::column_handle h_difftime;
            hipo::column_handle h_difftimeFadc;
            hipo::column_handle h_indexLpmt;
            hipo::column_handle h_indexRpmt;
            hipo::column_handle h_status;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
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
            if (auto h = schema.get_handle("ex"); h) cache.h_ex = *h;
            else cache.h_ex = {};
            if (auto h = schema.get_handle("ey"); h) cache.h_ey = *h;
            else cache.h_ey = {};
            if (auto h = schema.get_handle("ez"); h) cache.h_ez = *h;
            else cache.h_ez = {};
            if (auto h = schema.get_handle("timeFadc"); h) cache.h_timeFadc = *h;
            else cache.h_timeFadc = {};
            if (auto h = schema.get_handle("difftime"); h) cache.h_difftime = *h;
            else cache.h_difftime = {};
            if (auto h = schema.get_handle("difftimeFadc"); h) cache.h_difftimeFadc = *h;
            else cache.h_difftimeFadc = {};
            if (auto h = schema.get_handle("indexLpmt"); h) cache.h_indexLpmt = *h;
            else cache.h_indexLpmt = {};
            if (auto h = schema.get_handle("indexRpmt"); h) cache.h_indexRpmt = *h;
            else cache.h_indexRpmt = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
        }
        h_id = cache.h_id;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_energy = cache.h_energy;
        h_time = cache.h_time;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_ex = cache.h_ex;
        h_ey = cache.h_ey;
        h_ez = cache.h_ez;
        h_timeFadc = cache.h_timeFadc;
        h_difftime = cache.h_difftime;
        h_difftimeFadc = cache.h_difftimeFadc;
        h_indexLpmt = cache.h_indexLpmt;
        h_indexRpmt = cache.h_indexRpmt;
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
    /// id: id of the coincidence hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// sector: sector of BAND
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer or column id of BAND
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: paddle or row id of BAND
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// energy: Geometric mean of L and R ADC
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Meantime from TDC from L and R hit (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// x: X coor (cm) of coincidence hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Y coor (cm) of coincidence hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Z coor (cm) of coincidence hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// ex: X coor uncertainty (cm) of coincidence hit
    [[nodiscard]] float ex(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ex, row));
    }
    /// ey: Y coor uncertainty (cm) of coincidence hit
    [[nodiscard]] float ey(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ey, row));
    }
    /// ez: Z coor uncertainty (cm) of coincidence hit
    [[nodiscard]] float ez(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ez, row));
    }
    /// timeFadc: Meantime from FADC from L and R hit (ns)
    [[nodiscard]] float timeFadc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_timeFadc, row));
    }
    /// difftime: Calibrated time difference of L and R on a bar(ns) from TDC
    [[nodiscard]] float difftime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_difftime, row));
    }
    /// difftimeFadc: Calibrated time difference of L and R on a bar(ns) from FADC
    [[nodiscard]] float difftimeFadc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_difftimeFadc, row));
    }
    /// indexLpmt: index of rawhit from Left PMT, starts at zero
    [[nodiscard]] int indexLpmt(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexLpmt, row));
    }
    /// indexRpmt: index of rawhit from Right PMT, starts at zero
    [[nodiscard]] int indexRpmt(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexRpmt, row));
    }
    /// status: status of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
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
    /// ex column as a contiguous span.
    [[nodiscard]] std::span<const float> ex_col() const noexcept {
        return m_view.column<float>(h_ex);
    }
    /// ey column as a contiguous span.
    [[nodiscard]] std::span<const float> ey_col() const noexcept {
        return m_view.column<float>(h_ey);
    }
    /// ez column as a contiguous span.
    [[nodiscard]] std::span<const float> ez_col() const noexcept {
        return m_view.column<float>(h_ez);
    }
    /// timeFadc column as a contiguous span.
    [[nodiscard]] std::span<const float> timeFadc_col() const noexcept {
        return m_view.column<float>(h_timeFadc);
    }
    /// difftime column as a contiguous span.
    [[nodiscard]] std::span<const float> difftime_col() const noexcept {
        return m_view.column<float>(h_difftime);
    }
    /// difftimeFadc column as a contiguous span.
    [[nodiscard]] std::span<const float> difftimeFadc_col() const noexcept {
        return m_view.column<float>(h_difftimeFadc);
    }
    /// indexLpmt column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexLpmt_col() const noexcept {
        return m_view.column<int16_t>(h_indexLpmt);
    }
    /// indexRpmt column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexRpmt_col() const noexcept {
        return m_view.column<int16_t>(h_indexRpmt);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_ex{};
    hipo::column_handle h_ey{};
    hipo::column_handle h_ez{};
    hipo::column_handle h_timeFadc{};
    hipo::column_handle h_difftime{};
    hipo::column_handle h_difftimeFadc{};
    hipo::column_handle h_indexLpmt{};
    hipo::column_handle h_indexRpmt{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for BAND::laser. Stores a pointer + row index only.
class BAND_laser::Row {
 public:
    Row(const BAND_laser* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float ex() const noexcept { return m_bank->ex(m_row); }
    [[nodiscard]] float ey() const noexcept { return m_bank->ey(m_row); }
    [[nodiscard]] float ez() const noexcept { return m_bank->ez(m_row); }
    [[nodiscard]] float timeFadc() const noexcept { return m_bank->timeFadc(m_row); }
    [[nodiscard]] float difftime() const noexcept { return m_bank->difftime(m_row); }
    [[nodiscard]] float difftimeFadc() const noexcept { return m_bank->difftimeFadc(m_row); }
    [[nodiscard]] int indexLpmt() const noexcept { return m_bank->indexLpmt(m_row); }
    [[nodiscard]] int indexRpmt() const noexcept { return m_bank->indexRpmt(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const BAND_laser* m_bank;
    int m_row;
};

inline BAND_laser::Row BAND_laser::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
