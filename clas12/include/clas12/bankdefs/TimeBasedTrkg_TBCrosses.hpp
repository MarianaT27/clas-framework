// =============================================================================
// Auto-generated from dc.json - do not edit by hand.
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


/// HIPO bank wrapper for `TimeBasedTrkg::TBCrosses` (group 20600, item 35).
/// reconstructed segments using DC timing information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// TimeBasedTrkg_TBCrosses bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::TBCrosses")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_TBCrosses {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::TBCrosses";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 35;

    class Row;

    TimeBasedTrkg_TBCrosses() = default;

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
            hipo::column_handle h_sector;
            hipo::column_handle h_region;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_err_x;
            hipo::column_handle h_err_y;
            hipo::column_handle h_err_z;
            hipo::column_handle h_ux;
            hipo::column_handle h_uy;
            hipo::column_handle h_uz;
            hipo::column_handle h_err_ux;
            hipo::column_handle h_err_uy;
            hipo::column_handle h_err_uz;
            hipo::column_handle h_Segment1_ID;
            hipo::column_handle h_Segment2_ID;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("region"); h) cache.h_region = *h;
            else cache.h_region = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("err_x"); h) cache.h_err_x = *h;
            else cache.h_err_x = {};
            if (auto h = schema.get_handle("err_y"); h) cache.h_err_y = *h;
            else cache.h_err_y = {};
            if (auto h = schema.get_handle("err_z"); h) cache.h_err_z = *h;
            else cache.h_err_z = {};
            if (auto h = schema.get_handle("ux"); h) cache.h_ux = *h;
            else cache.h_ux = {};
            if (auto h = schema.get_handle("uy"); h) cache.h_uy = *h;
            else cache.h_uy = {};
            if (auto h = schema.get_handle("uz"); h) cache.h_uz = *h;
            else cache.h_uz = {};
            if (auto h = schema.get_handle("err_ux"); h) cache.h_err_ux = *h;
            else cache.h_err_ux = {};
            if (auto h = schema.get_handle("err_uy"); h) cache.h_err_uy = *h;
            else cache.h_err_uy = {};
            if (auto h = schema.get_handle("err_uz"); h) cache.h_err_uz = *h;
            else cache.h_err_uz = {};
            if (auto h = schema.get_handle("Segment1_ID"); h) cache.h_Segment1_ID = *h;
            else cache.h_Segment1_ID = {};
            if (auto h = schema.get_handle("Segment2_ID"); h) cache.h_Segment2_ID = *h;
            else cache.h_Segment2_ID = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_region = cache.h_region;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_err_x = cache.h_err_x;
        h_err_y = cache.h_err_y;
        h_err_z = cache.h_err_z;
        h_ux = cache.h_ux;
        h_uy = cache.h_uy;
        h_uz = cache.h_uz;
        h_err_ux = cache.h_err_ux;
        h_err_uy = cache.h_err_uy;
        h_err_uz = cache.h_err_uz;
        h_Segment1_ID = cache.h_Segment1_ID;
        h_Segment2_ID = cache.h_Segment2_ID;
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
    /// id: id of the cross
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: status of the cross
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: sector of the cross
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// region: region of the cross
    [[nodiscard]] int region(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_region, row));
    }
    /// x: DC track cross x-coordinate (in the DC tilted sector coordinate system)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: DC track cross y-coordinate (in the DC tilted sector coordinate system)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: DC track cross z-coordinate (in the DC tilted sector coordinate system)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// err_x: DC track cross x-coordinate uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_x, row));
    }
    /// err_y: DC track cross y-coordinate uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_y, row));
    }
    /// err_z: DC track cross z-coordinate uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_z, row));
    }
    /// ux: DC track cross x-direction (in the DC tilted sector coordinate system)
    [[nodiscard]] float ux(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ux, row));
    }
    /// uy: DC track cross y-direction (in the DC tilted sector coordinate system)
    [[nodiscard]] float uy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_uy, row));
    }
    /// uz: DC track cross z-direction (in the DC tilted sector coordinate system)
    [[nodiscard]] float uz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_uz, row));
    }
    /// err_ux: DC track cross x-direction uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_ux(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_ux, row));
    }
    /// err_uy: DC track cross y-direction uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_uy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_uy, row));
    }
    /// err_uz: DC track cross z-direction uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_uz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_uz, row));
    }
    /// Segment1_ID: id of first superlayer used in segment
    [[nodiscard]] int Segment1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Segment1_ID, row));
    }
    /// Segment2_ID: id of second superlayer used in segment
    [[nodiscard]] int Segment2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Segment2_ID, row));
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
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// region column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> region_col() const noexcept {
        return m_view.column<int8_t>(h_region);
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
    /// err_x column as a contiguous span.
    [[nodiscard]] std::span<const float> err_x_col() const noexcept {
        return m_view.column<float>(h_err_x);
    }
    /// err_y column as a contiguous span.
    [[nodiscard]] std::span<const float> err_y_col() const noexcept {
        return m_view.column<float>(h_err_y);
    }
    /// err_z column as a contiguous span.
    [[nodiscard]] std::span<const float> err_z_col() const noexcept {
        return m_view.column<float>(h_err_z);
    }
    /// ux column as a contiguous span.
    [[nodiscard]] std::span<const float> ux_col() const noexcept {
        return m_view.column<float>(h_ux);
    }
    /// uy column as a contiguous span.
    [[nodiscard]] std::span<const float> uy_col() const noexcept {
        return m_view.column<float>(h_uy);
    }
    /// uz column as a contiguous span.
    [[nodiscard]] std::span<const float> uz_col() const noexcept {
        return m_view.column<float>(h_uz);
    }
    /// err_ux column as a contiguous span.
    [[nodiscard]] std::span<const float> err_ux_col() const noexcept {
        return m_view.column<float>(h_err_ux);
    }
    /// err_uy column as a contiguous span.
    [[nodiscard]] std::span<const float> err_uy_col() const noexcept {
        return m_view.column<float>(h_err_uy);
    }
    /// err_uz column as a contiguous span.
    [[nodiscard]] std::span<const float> err_uz_col() const noexcept {
        return m_view.column<float>(h_err_uz);
    }
    /// Segment1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Segment1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Segment1_ID);
    }
    /// Segment2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Segment2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Segment2_ID);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_region{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_err_x{};
    hipo::column_handle h_err_y{};
    hipo::column_handle h_err_z{};
    hipo::column_handle h_ux{};
    hipo::column_handle h_uy{};
    hipo::column_handle h_uz{};
    hipo::column_handle h_err_ux{};
    hipo::column_handle h_err_uy{};
    hipo::column_handle h_err_uz{};
    hipo::column_handle h_Segment1_ID{};
    hipo::column_handle h_Segment2_ID{};

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::TBCrosses. Stores a pointer + row index only.
class TimeBasedTrkg_TBCrosses::Row {
 public:
    Row(const TimeBasedTrkg_TBCrosses* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int region() const noexcept { return m_bank->region(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float err_x() const noexcept { return m_bank->err_x(m_row); }
    [[nodiscard]] float err_y() const noexcept { return m_bank->err_y(m_row); }
    [[nodiscard]] float err_z() const noexcept { return m_bank->err_z(m_row); }
    [[nodiscard]] float ux() const noexcept { return m_bank->ux(m_row); }
    [[nodiscard]] float uy() const noexcept { return m_bank->uy(m_row); }
    [[nodiscard]] float uz() const noexcept { return m_bank->uz(m_row); }
    [[nodiscard]] float err_ux() const noexcept { return m_bank->err_ux(m_row); }
    [[nodiscard]] float err_uy() const noexcept { return m_bank->err_uy(m_row); }
    [[nodiscard]] float err_uz() const noexcept { return m_bank->err_uz(m_row); }
    [[nodiscard]] int Segment1_ID() const noexcept { return m_bank->Segment1_ID(m_row); }
    [[nodiscard]] int Segment2_ID() const noexcept { return m_bank->Segment2_ID(m_row); }
 private:
    const TimeBasedTrkg_TBCrosses* m_bank;
    int m_row;
};

inline TimeBasedTrkg_TBCrosses::Row TimeBasedTrkg_TBCrosses::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
