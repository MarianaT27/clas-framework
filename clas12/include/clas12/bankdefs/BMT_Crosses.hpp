// =============================================================================
// Auto-generated from bmt.json - do not edit by hand.
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


/// HIPO bank wrapper for `BMT::Crosses` (group 20100, item 35).
/// reconstructed BMT crosses
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// BMT_Crosses bank;
/// if (auto sc = bank.bind(event.get_bank("BMT::Crosses")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class BMT_Crosses {
 public:
    static constexpr std::string_view bank_name = "BMT::Crosses";
    static constexpr int bank_group = 20100;
    static constexpr int bank_item  = 35;

    class Row;

    BMT_Crosses() = default;

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
            hipo::column_handle h_ID;
            hipo::column_handle h_sector;
            hipo::column_handle h_region;
            hipo::column_handle h_layer;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_err_x;
            hipo::column_handle h_err_y;
            hipo::column_handle h_err_z;
            hipo::column_handle h_x0;
            hipo::column_handle h_y0;
            hipo::column_handle h_z0;
            hipo::column_handle h_err_x0;
            hipo::column_handle h_err_y0;
            hipo::column_handle h_err_z0;
            hipo::column_handle h_ux;
            hipo::column_handle h_uy;
            hipo::column_handle h_uz;
            hipo::column_handle h_Cluster1_ID;
            hipo::column_handle h_Cluster2_ID;
            hipo::column_handle h_trkID;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("ID"); h) cache.h_ID = *h;
            else cache.h_ID = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("region"); h) cache.h_region = *h;
            else cache.h_region = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
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
            if (auto h = schema.get_handle("x0"); h) cache.h_x0 = *h;
            else cache.h_x0 = {};
            if (auto h = schema.get_handle("y0"); h) cache.h_y0 = *h;
            else cache.h_y0 = {};
            if (auto h = schema.get_handle("z0"); h) cache.h_z0 = *h;
            else cache.h_z0 = {};
            if (auto h = schema.get_handle("err_x0"); h) cache.h_err_x0 = *h;
            else cache.h_err_x0 = {};
            if (auto h = schema.get_handle("err_y0"); h) cache.h_err_y0 = *h;
            else cache.h_err_y0 = {};
            if (auto h = schema.get_handle("err_z0"); h) cache.h_err_z0 = *h;
            else cache.h_err_z0 = {};
            if (auto h = schema.get_handle("ux"); h) cache.h_ux = *h;
            else cache.h_ux = {};
            if (auto h = schema.get_handle("uy"); h) cache.h_uy = *h;
            else cache.h_uy = {};
            if (auto h = schema.get_handle("uz"); h) cache.h_uz = *h;
            else cache.h_uz = {};
            if (auto h = schema.get_handle("Cluster1_ID"); h) cache.h_Cluster1_ID = *h;
            else cache.h_Cluster1_ID = {};
            if (auto h = schema.get_handle("Cluster2_ID"); h) cache.h_Cluster2_ID = *h;
            else cache.h_Cluster2_ID = {};
            if (auto h = schema.get_handle("trkID"); h) cache.h_trkID = *h;
            else cache.h_trkID = {};
        }
        h_ID = cache.h_ID;
        h_sector = cache.h_sector;
        h_region = cache.h_region;
        h_layer = cache.h_layer;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_err_x = cache.h_err_x;
        h_err_y = cache.h_err_y;
        h_err_z = cache.h_err_z;
        h_x0 = cache.h_x0;
        h_y0 = cache.h_y0;
        h_z0 = cache.h_z0;
        h_err_x0 = cache.h_err_x0;
        h_err_y0 = cache.h_err_y0;
        h_err_z0 = cache.h_err_z0;
        h_ux = cache.h_ux;
        h_uy = cache.h_uy;
        h_uz = cache.h_uz;
        h_Cluster1_ID = cache.h_Cluster1_ID;
        h_Cluster2_ID = cache.h_Cluster2_ID;
        h_trkID = cache.h_trkID;
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
    /// ID: ID
    [[nodiscard]] int ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ID, row));
    }
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// region: region
    [[nodiscard]] int region(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_region, row));
    }
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// x: BMT cross x-coordinate
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: BMT cross y-coordinate
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: BMT cross z-coordinate
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// err_x: BMT cross x-coordinate error
    [[nodiscard]] float err_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_x, row));
    }
    /// err_y: BMT cross y-coordinate error
    [[nodiscard]] float err_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_y, row));
    }
    /// err_z: BMT cross z-coordinate error
    [[nodiscard]] float err_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_z, row));
    }
    /// x0: BMT cross x0-coordinate
    [[nodiscard]] float x0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x0, row));
    }
    /// y0: BMT cross y0-coordinate
    [[nodiscard]] float y0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y0, row));
    }
    /// z0: BMT cross z0-coordinate
    [[nodiscard]] float z0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z0, row));
    }
    /// err_x0: BMT cross x0-coordinate error
    [[nodiscard]] float err_x0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_x0, row));
    }
    /// err_y0: BMT cross y0-coordinate error
    [[nodiscard]] float err_y0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_y0, row));
    }
    /// err_z0: BMT cross z0-coordinate error
    [[nodiscard]] float err_z0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_z0, row));
    }
    /// ux: BMT cross x-direction (track unit tangent vector at the cross)
    [[nodiscard]] float ux(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ux, row));
    }
    /// uy: BMT cross y-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_uy, row));
    }
    /// uz: BMT cross z-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_uz, row));
    }
    /// Cluster1_ID: ID of the  cluster in the Cross
    [[nodiscard]] int Cluster1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster1_ID, row));
    }
    /// Cluster2_ID: ID of the top layer  cluster in the Cross
    [[nodiscard]] int Cluster2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster2_ID, row));
    }
    /// trkID: associated track ID
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trkID, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ID_col() const noexcept {
        return m_view.column<int16_t>(h_ID);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// region column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> region_col() const noexcept {
        return m_view.column<int8_t>(h_region);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
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
    /// x0 column as a contiguous span.
    [[nodiscard]] std::span<const float> x0_col() const noexcept {
        return m_view.column<float>(h_x0);
    }
    /// y0 column as a contiguous span.
    [[nodiscard]] std::span<const float> y0_col() const noexcept {
        return m_view.column<float>(h_y0);
    }
    /// z0 column as a contiguous span.
    [[nodiscard]] std::span<const float> z0_col() const noexcept {
        return m_view.column<float>(h_z0);
    }
    /// err_x0 column as a contiguous span.
    [[nodiscard]] std::span<const float> err_x0_col() const noexcept {
        return m_view.column<float>(h_err_x0);
    }
    /// err_y0 column as a contiguous span.
    [[nodiscard]] std::span<const float> err_y0_col() const noexcept {
        return m_view.column<float>(h_err_y0);
    }
    /// err_z0 column as a contiguous span.
    [[nodiscard]] std::span<const float> err_z0_col() const noexcept {
        return m_view.column<float>(h_err_z0);
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
    /// Cluster1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster1_ID);
    }
    /// Cluster2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster2_ID);
    }
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trkID_col() const noexcept {
        return m_view.column<int16_t>(h_trkID);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_ID{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_region{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_err_x{};
    hipo::column_handle h_err_y{};
    hipo::column_handle h_err_z{};
    hipo::column_handle h_x0{};
    hipo::column_handle h_y0{};
    hipo::column_handle h_z0{};
    hipo::column_handle h_err_x0{};
    hipo::column_handle h_err_y0{};
    hipo::column_handle h_err_z0{};
    hipo::column_handle h_ux{};
    hipo::column_handle h_uy{};
    hipo::column_handle h_uz{};
    hipo::column_handle h_Cluster1_ID{};
    hipo::column_handle h_Cluster2_ID{};
    hipo::column_handle h_trkID{};

    friend class Row;
};

/// Lightweight row view for BMT::Crosses. Stores a pointer + row index only.
class BMT_Crosses::Row {
 public:
    Row(const BMT_Crosses* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int region() const noexcept { return m_bank->region(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float err_x() const noexcept { return m_bank->err_x(m_row); }
    [[nodiscard]] float err_y() const noexcept { return m_bank->err_y(m_row); }
    [[nodiscard]] float err_z() const noexcept { return m_bank->err_z(m_row); }
    [[nodiscard]] float x0() const noexcept { return m_bank->x0(m_row); }
    [[nodiscard]] float y0() const noexcept { return m_bank->y0(m_row); }
    [[nodiscard]] float z0() const noexcept { return m_bank->z0(m_row); }
    [[nodiscard]] float err_x0() const noexcept { return m_bank->err_x0(m_row); }
    [[nodiscard]] float err_y0() const noexcept { return m_bank->err_y0(m_row); }
    [[nodiscard]] float err_z0() const noexcept { return m_bank->err_z0(m_row); }
    [[nodiscard]] float ux() const noexcept { return m_bank->ux(m_row); }
    [[nodiscard]] float uy() const noexcept { return m_bank->uy(m_row); }
    [[nodiscard]] float uz() const noexcept { return m_bank->uz(m_row); }
    [[nodiscard]] int Cluster1_ID() const noexcept { return m_bank->Cluster1_ID(m_row); }
    [[nodiscard]] int Cluster2_ID() const noexcept { return m_bank->Cluster2_ID(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
 private:
    const BMT_Crosses* m_bank;
    int m_row;
};

inline BMT_Crosses::Row BMT_Crosses::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
