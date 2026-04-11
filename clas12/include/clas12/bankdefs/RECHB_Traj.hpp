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


/// HIPO bank wrapper for `RECHB::Traj` (group 300, item 18).
/// Trajectory information for Particles bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RECHB_Traj bank;
/// if (auto sc = bank.bind(event.get_bank("RECHB::Traj")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto pindex = row.pindex();
/// }
/// @endcode
class RECHB_Traj {
 public:
    static constexpr std::string_view bank_name = "RECHB::Traj";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 18;

    class Row;

    RECHB_Traj() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("pindex"); h) h_pindex = *h;
        else h_pindex = {};
        if (auto h = schema.get_handle("index"); h) h_index = *h;
        else h_index = {};
        if (auto h = schema.get_handle("detector"); h) h_detector = *h;
        else h_detector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("cx"); h) h_cx = *h;
        else h_cx = {};
        if (auto h = schema.get_handle("cy"); h) h_cy = *h;
        else h_cy = {};
        if (auto h = schema.get_handle("cz"); h) h_cz = *h;
        else h_cz = {};
        if (auto h = schema.get_handle("path"); h) h_path = *h;
        else h_path = {};
        if (auto h = schema.get_handle("edge"); h) h_edge = *h;
        else h_edge = {};
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
    /// pindex: index of the particle in REC::Particle
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pindex, row));
    }
    /// index: index of the track in its tracking bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// detector: DetectorID as defined in org.jlab.detector.DetectorType
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_detector, row));
    }
    /// layer: Layer ID as defined in org.jlab.detector.DetectorLayer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// x: x-position of the track at the detector surface (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: y-position of the track at the detector surface (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: z-position of the track at the detector surface (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// cx: direction cosline of the track at the detector surface
    [[nodiscard]] float cx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cx, row));
    }
    /// cy: direction cosline of the track at the detector surface
    [[nodiscard]] float cy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cy, row));
    }
    /// cz: direction cosline of the track at the detector surface
    [[nodiscard]] float cz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cz, row));
    }
    /// path: path length from primary vertex to the detector surface (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }
    /// edge: signed distance between track position and closest detector edge (cm)
    [[nodiscard]] float edge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_edge, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pindex_col() const noexcept {
        return m_view.column<int16_t>(h_pindex);
    }
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
    /// cx column as a contiguous span.
    [[nodiscard]] std::span<const float> cx_col() const noexcept {
        return m_view.column<float>(h_cx);
    }
    /// cy column as a contiguous span.
    [[nodiscard]] std::span<const float> cy_col() const noexcept {
        return m_view.column<float>(h_cy);
    }
    /// cz column as a contiguous span.
    [[nodiscard]] std::span<const float> cz_col() const noexcept {
        return m_view.column<float>(h_cz);
    }
    /// path column as a contiguous span.
    [[nodiscard]] std::span<const float> path_col() const noexcept {
        return m_view.column<float>(h_path);
    }
    /// edge column as a contiguous span.
    [[nodiscard]] std::span<const float> edge_col() const noexcept {
        return m_view.column<float>(h_edge);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle pindex_handle() const noexcept { return h_pindex; }
    [[nodiscard]] hipo::column_handle index_handle() const noexcept { return h_index; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_pindex{};
    hipo::column_handle h_index{};
    hipo::column_handle h_detector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_cx{};
    hipo::column_handle h_cy{};
    hipo::column_handle h_cz{};
    hipo::column_handle h_path{};
    hipo::column_handle h_edge{};

    friend class Row;
};

/// Lightweight row view for RECHB::Traj. Stores a pointer + row index only.
class RECHB_Traj::Row {
 public:
    Row(const RECHB_Traj* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float cx() const noexcept { return m_bank->cx(m_row); }
    [[nodiscard]] float cy() const noexcept { return m_bank->cy(m_row); }
    [[nodiscard]] float cz() const noexcept { return m_bank->cz(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float edge() const noexcept { return m_bank->edge(m_row); }
 private:
    const RECHB_Traj* m_bank;
    int m_row;
};

inline RECHB_Traj::Row RECHB_Traj::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
