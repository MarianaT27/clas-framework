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


/// HIPO bank wrapper for `REC::TrackCross` (group 300, item 37).
/// Track Cross information for Particles bank (deprecated by REC::Traj)
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// REC_TrackCross bank;
/// if (auto sc = bank.bind(event.get_bank("REC::TrackCross")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class REC_TrackCross {
 public:
    static constexpr std::string_view bank_name = "REC::TrackCross";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 37;

    class Row;

    REC_TrackCross() = default;

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
            hipo::column_handle h_c_x;
            hipo::column_handle h_c_y;
            hipo::column_handle h_c_z;
            hipo::column_handle h_c_ux;
            hipo::column_handle h_c_uy;
            hipo::column_handle h_c_uz;
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
            if (auto h = schema.get_handle("c_x"); h) cache.h_c_x = *h;
            else cache.h_c_x = {};
            if (auto h = schema.get_handle("c_y"); h) cache.h_c_y = *h;
            else cache.h_c_y = {};
            if (auto h = schema.get_handle("c_z"); h) cache.h_c_z = *h;
            else cache.h_c_z = {};
            if (auto h = schema.get_handle("c_ux"); h) cache.h_c_ux = *h;
            else cache.h_c_ux = {};
            if (auto h = schema.get_handle("c_uy"); h) cache.h_c_uy = *h;
            else cache.h_c_uy = {};
            if (auto h = schema.get_handle("c_uz"); h) cache.h_c_uz = *h;
            else cache.h_c_uz = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
        }
        h_index = cache.h_index;
        h_pindex = cache.h_pindex;
        h_detector = cache.h_detector;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_c_x = cache.h_c_x;
        h_c_y = cache.h_c_y;
        h_c_z = cache.h_c_z;
        h_c_ux = cache.h_c_ux;
        h_c_uy = cache.h_c_uy;
        h_c_uz = cache.h_c_uz;
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
    /// index: index of the cross in the specific detector bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// pindex: row number in the track bank hit is associated with
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pindex, row));
    }
    /// detector: Detector ID, as defined in org.jlab.detector.base.DetectorType
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_detector, row));
    }
    /// sector: sector of the track
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer of the track
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// c_x: Cross x-position in the lab (in cm)
    [[nodiscard]] float c_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c_x, row));
    }
    /// c_y: Cross y-position in the lab (in cm)
    [[nodiscard]] float c_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c_y, row));
    }
    /// c_z: Cross z-position in the lab (in cm)
    [[nodiscard]] float c_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c_z, row));
    }
    /// c_ux: Cross unit x-direction vector in the lab
    [[nodiscard]] float c_ux(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c_ux, row));
    }
    /// c_uy: Cross unit y-direction vector in the lab
    [[nodiscard]] float c_uy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c_uy, row));
    }
    /// c_uz: Cross unit z-direction vector in the lab
    [[nodiscard]] float c_uz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c_uz, row));
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
    /// c_x column as a contiguous span.
    [[nodiscard]] std::span<const float> c_x_col() const noexcept {
        return m_view.column<float>(h_c_x);
    }
    /// c_y column as a contiguous span.
    [[nodiscard]] std::span<const float> c_y_col() const noexcept {
        return m_view.column<float>(h_c_y);
    }
    /// c_z column as a contiguous span.
    [[nodiscard]] std::span<const float> c_z_col() const noexcept {
        return m_view.column<float>(h_c_z);
    }
    /// c_ux column as a contiguous span.
    [[nodiscard]] std::span<const float> c_ux_col() const noexcept {
        return m_view.column<float>(h_c_ux);
    }
    /// c_uy column as a contiguous span.
    [[nodiscard]] std::span<const float> c_uy_col() const noexcept {
        return m_view.column<float>(h_c_uy);
    }
    /// c_uz column as a contiguous span.
    [[nodiscard]] std::span<const float> c_uz_col() const noexcept {
        return m_view.column<float>(h_c_uz);
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
    hipo::column_handle h_c_x{};
    hipo::column_handle h_c_y{};
    hipo::column_handle h_c_z{};
    hipo::column_handle h_c_ux{};
    hipo::column_handle h_c_uy{};
    hipo::column_handle h_c_uz{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for REC::TrackCross. Stores a pointer + row index only.
class REC_TrackCross::Row {
 public:
    Row(const REC_TrackCross* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float c_x() const noexcept { return m_bank->c_x(m_row); }
    [[nodiscard]] float c_y() const noexcept { return m_bank->c_y(m_row); }
    [[nodiscard]] float c_z() const noexcept { return m_bank->c_z(m_row); }
    [[nodiscard]] float c_ux() const noexcept { return m_bank->c_ux(m_row); }
    [[nodiscard]] float c_uy() const noexcept { return m_bank->c_uy(m_row); }
    [[nodiscard]] float c_uz() const noexcept { return m_bank->c_uz(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const REC_TrackCross* m_bank;
    int m_row;
};

inline REC_TrackCross::Row REC_TrackCross::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
