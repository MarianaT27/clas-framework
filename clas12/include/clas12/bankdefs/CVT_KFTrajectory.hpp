// =============================================================================
// Auto-generated from cvt.json - do not edit by hand.
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


/// HIPO bank wrapper for `CVT::KFTrajectory` (group 20500, item 42).
/// Trajectory bank output from KF
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CVT_KFTrajectory bank;
/// if (auto sc = bank.bind(event.get_bank("CVT::KFTrajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CVT_KFTrajectory {
 public:
    static constexpr std::string_view bank_name = "CVT::KFTrajectory";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 42;

    class Row;

    CVT_KFTrajectory() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("detector"); h) h_detector = *h;
        else h_detector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("index"); h) h_index = *h;
        else h_index = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("px"); h) h_px = *h;
        else h_px = {};
        if (auto h = schema.get_handle("py"); h) h_py = *h;
        else h_py = {};
        if (auto h = schema.get_handle("pz"); h) h_pz = *h;
        else h_pz = {};
        if (auto h = schema.get_handle("path"); h) h_path = *h;
        else h_path = {};
        if (auto h = schema.get_handle("dx"); h) h_dx = *h;
        else h_dx = {};
        if (auto h = schema.get_handle("dE"); h) h_dE = *h;
        else h_dE = {};
        if (auto h = schema.get_handle("trackRes"); h) h_trackRes = *h;
        else h_trackRes = {};
        if (auto h = schema.get_handle("transportedRes"); h) h_transportedRes = *h;
        else h_transportedRes = {};
        if (auto h = schema.get_handle("filteredRes"); h) h_filteredRes = *h;
        else h_filteredRes = {};
        if (auto h = schema.get_handle("smoothedRes"); h) h_smoothedRes = *h;
        else h_smoothedRes = {};
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
    /// id: id of the track
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// detector: detector type id
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_detector, row));
    }
    /// layer: id of the detector layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// index: id of the measureents surface
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_index, row));
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
    /// px: track x momentum at detector surface (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: track y momentum at detector surface (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: track z momentum at detector surface (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// path: track path length from previous point
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }
    /// dx: track path length through the surface material
    [[nodiscard]] float dx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dx, row));
    }
    /// dE: track energy loss
    [[nodiscard]] float dE(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dE, row));
    }
    /// trackRes: track residual (-999 for passive layer)
    [[nodiscard]] float trackRes(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trackRes, row));
    }
    /// transportedRes: filtered residual (-999 for passive layer)
    [[nodiscard]] float transportedRes(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_transportedRes, row));
    }
    /// filteredRes: filtered residual (-999 for passive layer)
    [[nodiscard]] float filteredRes(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_filteredRes, row));
    }
    /// smoothedRes: smoothed residual (-999 for passive layer)
    [[nodiscard]] float smoothedRes(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_smoothedRes, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// detector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> detector_col() const noexcept {
        return m_view.column<int8_t>(h_detector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> index_col() const noexcept {
        return m_view.column<int8_t>(h_index);
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
    /// path column as a contiguous span.
    [[nodiscard]] std::span<const float> path_col() const noexcept {
        return m_view.column<float>(h_path);
    }
    /// dx column as a contiguous span.
    [[nodiscard]] std::span<const float> dx_col() const noexcept {
        return m_view.column<float>(h_dx);
    }
    /// dE column as a contiguous span.
    [[nodiscard]] std::span<const float> dE_col() const noexcept {
        return m_view.column<float>(h_dE);
    }
    /// trackRes column as a contiguous span.
    [[nodiscard]] std::span<const float> trackRes_col() const noexcept {
        return m_view.column<float>(h_trackRes);
    }
    /// transportedRes column as a contiguous span.
    [[nodiscard]] std::span<const float> transportedRes_col() const noexcept {
        return m_view.column<float>(h_transportedRes);
    }
    /// filteredRes column as a contiguous span.
    [[nodiscard]] std::span<const float> filteredRes_col() const noexcept {
        return m_view.column<float>(h_filteredRes);
    }
    /// smoothedRes column as a contiguous span.
    [[nodiscard]] std::span<const float> smoothedRes_col() const noexcept {
        return m_view.column<float>(h_smoothedRes);
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
    hipo::column_handle h_id{};
    hipo::column_handle h_detector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_index{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_path{};
    hipo::column_handle h_dx{};
    hipo::column_handle h_dE{};
    hipo::column_handle h_trackRes{};
    hipo::column_handle h_transportedRes{};
    hipo::column_handle h_filteredRes{};
    hipo::column_handle h_smoothedRes{};

    friend class Row;
};

/// Lightweight row view for CVT::KFTrajectory. Stores a pointer + row index only.
class CVT_KFTrajectory::Row {
 public:
    Row(const CVT_KFTrajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float dE() const noexcept { return m_bank->dE(m_row); }
    [[nodiscard]] float trackRes() const noexcept { return m_bank->trackRes(m_row); }
    [[nodiscard]] float transportedRes() const noexcept { return m_bank->transportedRes(m_row); }
    [[nodiscard]] float filteredRes() const noexcept { return m_bank->filteredRes(m_row); }
    [[nodiscard]] float smoothedRes() const noexcept { return m_bank->smoothedRes(m_row); }
 private:
    const CVT_KFTrajectory* m_bank;
    int m_row;
};

inline CVT_KFTrajectory::Row CVT_KFTrajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
