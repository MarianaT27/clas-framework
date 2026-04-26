// =============================================================================
// Auto-generated from fmt.json - do not edit by hand.
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


/// HIPO bank wrapper for `FMT::Crosses` (group 20800, item 25).
/// reconstructed FMT crosses
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FMT_Crosses bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Crosses")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class FMT_Crosses {
 public:
    static constexpr std::string_view bank_name = "FMT::Crosses";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 25;

    class Row;

    FMT_Crosses() = default;

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
            hipo::column_handle h_cluster1Index;
            hipo::column_handle h_cluster2Index;
            hipo::column_handle h_trkID;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("index"); h) cache.h_index = *h;
            else cache.h_index = {};
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
            if (auto h = schema.get_handle("cluster1Index"); h) cache.h_cluster1Index = *h;
            else cache.h_cluster1Index = {};
            if (auto h = schema.get_handle("cluster2Index"); h) cache.h_cluster2Index = *h;
            else cache.h_cluster2Index = {};
            if (auto h = schema.get_handle("trkID"); h) cache.h_trkID = *h;
            else cache.h_trkID = {};
        }
        h_index = cache.h_index;
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
        h_cluster1Index = cache.h_cluster1Index;
        h_cluster2Index = cache.h_cluster2Index;
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
    /// index: index
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// region: region
    [[nodiscard]] int region(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_region, row));
    }
    /// x: cross x-coordinate
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: cross y-coordinate
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: cross z-coordinate
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// err_x: cross x-coordinate error
    [[nodiscard]] float err_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_x, row));
    }
    /// err_y: cross y-coordinate error
    [[nodiscard]] float err_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_y, row));
    }
    /// err_z: cross z-coordinate error
    [[nodiscard]] float err_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_err_z, row));
    }
    /// ux: cross x-direction (track unit tangent vector at the cross)
    [[nodiscard]] float ux(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ux, row));
    }
    /// uy: cross y-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_uy, row));
    }
    /// uz: cross z-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_uz, row));
    }
    /// cluster1Index: index of the  cluster in the Cross
    [[nodiscard]] int cluster1Index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_cluster1Index, row));
    }
    /// cluster2Index: index of the top layer  cluster in the Cross
    [[nodiscard]] int cluster2Index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_cluster2Index, row));
    }
    /// trkID: associated track index
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trkID, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index_col() const noexcept {
        return m_view.column<int16_t>(h_index);
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
    /// cluster1Index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> cluster1Index_col() const noexcept {
        return m_view.column<int16_t>(h_cluster1Index);
    }
    /// cluster2Index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> cluster2Index_col() const noexcept {
        return m_view.column<int16_t>(h_cluster2Index);
    }
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trkID_col() const noexcept {
        return m_view.column<int16_t>(h_trkID);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle index_handle() const noexcept { return h_index; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_index{};
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
    hipo::column_handle h_cluster1Index{};
    hipo::column_handle h_cluster2Index{};
    hipo::column_handle h_trkID{};

    friend class Row;
};

/// Lightweight row view for FMT::Crosses. Stores a pointer + row index only.
class FMT_Crosses::Row {
 public:
    Row(const FMT_Crosses* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
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
    [[nodiscard]] int cluster1Index() const noexcept { return m_bank->cluster1Index(m_row); }
    [[nodiscard]] int cluster2Index() const noexcept { return m_bank->cluster2Index(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
 private:
    const FMT_Crosses* m_bank;
    int m_row;
};

inline FMT_Crosses::Row FMT_Crosses::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
