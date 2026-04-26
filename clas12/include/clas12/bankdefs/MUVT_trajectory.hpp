// =============================================================================
// Auto-generated from mu.json - do not edit by hand.
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


/// HIPO bank wrapper for `MUVT::trajectory` (group 22900, item 37).
/// MUVT tracks trajectory bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MUVT_trajectory bank;
/// if (auto sc = bank.bind(event.get_bank("MUVT::trajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class MUVT_trajectory {
 public:
    static constexpr std::string_view bank_name = "MUVT::trajectory";
    static constexpr int bank_group = 22900;
    static constexpr int bank_item  = 37;

    class Row;

    MUVT_trajectory() = default;

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
            hipo::column_handle h_detector;
            hipo::column_handle h_layer;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_tx;
            hipo::column_handle h_ty;
            hipo::column_handle h_tz;
            hipo::column_handle h_lx;
            hipo::column_handle h_ly;
            hipo::column_handle h_lz;
            hipo::column_handle h_dx;
            hipo::column_handle h_dy;
            hipo::column_handle h_dz;
            hipo::column_handle h_path;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("index"); h) cache.h_index = *h;
            else cache.h_index = {};
            if (auto h = schema.get_handle("detector"); h) cache.h_detector = *h;
            else cache.h_detector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
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
            if (auto h = schema.get_handle("lx"); h) cache.h_lx = *h;
            else cache.h_lx = {};
            if (auto h = schema.get_handle("ly"); h) cache.h_ly = *h;
            else cache.h_ly = {};
            if (auto h = schema.get_handle("lz"); h) cache.h_lz = *h;
            else cache.h_lz = {};
            if (auto h = schema.get_handle("dx"); h) cache.h_dx = *h;
            else cache.h_dx = {};
            if (auto h = schema.get_handle("dy"); h) cache.h_dy = *h;
            else cache.h_dy = {};
            if (auto h = schema.get_handle("dz"); h) cache.h_dz = *h;
            else cache.h_dz = {};
            if (auto h = schema.get_handle("path"); h) cache.h_path = *h;
            else cache.h_path = {};
        }
        h_index = cache.h_index;
        h_detector = cache.h_detector;
        h_layer = cache.h_layer;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_tx = cache.h_tx;
        h_ty = cache.h_ty;
        h_tz = cache.h_tz;
        h_lx = cache.h_lx;
        h_ly = cache.h_ly;
        h_lz = cache.h_lz;
        h_dx = cache.h_dx;
        h_dy = cache.h_dy;
        h_dz = cache.h_dz;
        h_path = cache.h_path;
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
    /// index: index of the track in the DC bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// detector: id of the detector
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_detector, row));
    }
    /// layer: id of the layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// x: track x position at detector surface (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: track y position at detector surface (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: track z position at detector surface (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// tx: track unit direction vector x component at detector surface
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tx, row));
    }
    /// ty: track unit direction vector y component at detector surface
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ty, row));
    }
    /// tz: track unit direction vector z component at detector surface
    [[nodiscard]] float tz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tz, row));
    }
    /// lx: track x position in local coordinates (cm)
    [[nodiscard]] float lx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lx, row));
    }
    /// ly: track y position in local coordinates (cm)
    [[nodiscard]] float ly(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ly, row));
    }
    /// lz: track z position in local coordinates (cm)
    [[nodiscard]] float lz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lz, row));
    }
    /// dx: DC track x position in local coordinates (cm)
    [[nodiscard]] float dx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dx, row));
    }
    /// dy: DC track y position in local coordinates (cm)
    [[nodiscard]] float dy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dy, row));
    }
    /// dz: DC track z position in local coordinates (cm)
    [[nodiscard]] float dz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dz, row));
    }
    /// path: pathlength of the track from the track vertex to the detector surface (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index_col() const noexcept {
        return m_view.column<int16_t>(h_index);
    }
    /// detector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> detector_col() const noexcept {
        return m_view.column<int8_t>(h_detector);
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
    /// lx column as a contiguous span.
    [[nodiscard]] std::span<const float> lx_col() const noexcept {
        return m_view.column<float>(h_lx);
    }
    /// ly column as a contiguous span.
    [[nodiscard]] std::span<const float> ly_col() const noexcept {
        return m_view.column<float>(h_ly);
    }
    /// lz column as a contiguous span.
    [[nodiscard]] std::span<const float> lz_col() const noexcept {
        return m_view.column<float>(h_lz);
    }
    /// dx column as a contiguous span.
    [[nodiscard]] std::span<const float> dx_col() const noexcept {
        return m_view.column<float>(h_dx);
    }
    /// dy column as a contiguous span.
    [[nodiscard]] std::span<const float> dy_col() const noexcept {
        return m_view.column<float>(h_dy);
    }
    /// dz column as a contiguous span.
    [[nodiscard]] std::span<const float> dz_col() const noexcept {
        return m_view.column<float>(h_dz);
    }
    /// path column as a contiguous span.
    [[nodiscard]] std::span<const float> path_col() const noexcept {
        return m_view.column<float>(h_path);
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
    hipo::column_handle h_detector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_tx{};
    hipo::column_handle h_ty{};
    hipo::column_handle h_tz{};
    hipo::column_handle h_lx{};
    hipo::column_handle h_ly{};
    hipo::column_handle h_lz{};
    hipo::column_handle h_dx{};
    hipo::column_handle h_dy{};
    hipo::column_handle h_dz{};
    hipo::column_handle h_path{};

    friend class Row;
};

/// Lightweight row view for MUVT::trajectory. Stores a pointer + row index only.
class MUVT_trajectory::Row {
 public:
    Row(const MUVT_trajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] float lx() const noexcept { return m_bank->lx(m_row); }
    [[nodiscard]] float ly() const noexcept { return m_bank->ly(m_row); }
    [[nodiscard]] float lz() const noexcept { return m_bank->lz(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float dy() const noexcept { return m_bank->dy(m_row); }
    [[nodiscard]] float dz() const noexcept { return m_bank->dz(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
 private:
    const MUVT_trajectory* m_bank;
    int m_row;
};

inline MUVT_trajectory::Row MUVT_trajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
