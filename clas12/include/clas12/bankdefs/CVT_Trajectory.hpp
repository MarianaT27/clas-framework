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


/// HIPO bank wrapper for `CVT::Trajectory` (group 20500, item 41).
/// Trajectory bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CVT_Trajectory bank;
/// if (auto sc = bank.bind(event.get_bank("CVT::Trajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CVT_Trajectory {
 public:
    static constexpr std::string_view bank_name = "CVT::Trajectory";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 41;

    class Row;

    CVT_Trajectory() = default;

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
            hipo::column_handle h_detector;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_cx;
            hipo::column_handle h_cy;
            hipo::column_handle h_cz;
            hipo::column_handle h_p;
            hipo::column_handle h_phi;
            hipo::column_handle h_theta;
            hipo::column_handle h_langle;
            hipo::column_handle h_centroid;
            hipo::column_handle h_path;
            hipo::column_handle h_dx;
            hipo::column_handle h_edge;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("detector"); h) cache.h_detector = *h;
            else cache.h_detector = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("cx"); h) cache.h_cx = *h;
            else cache.h_cx = {};
            if (auto h = schema.get_handle("cy"); h) cache.h_cy = *h;
            else cache.h_cy = {};
            if (auto h = schema.get_handle("cz"); h) cache.h_cz = *h;
            else cache.h_cz = {};
            if (auto h = schema.get_handle("p"); h) cache.h_p = *h;
            else cache.h_p = {};
            if (auto h = schema.get_handle("phi"); h) cache.h_phi = *h;
            else cache.h_phi = {};
            if (auto h = schema.get_handle("theta"); h) cache.h_theta = *h;
            else cache.h_theta = {};
            if (auto h = schema.get_handle("langle"); h) cache.h_langle = *h;
            else cache.h_langle = {};
            if (auto h = schema.get_handle("centroid"); h) cache.h_centroid = *h;
            else cache.h_centroid = {};
            if (auto h = schema.get_handle("path"); h) cache.h_path = *h;
            else cache.h_path = {};
            if (auto h = schema.get_handle("dx"); h) cache.h_dx = *h;
            else cache.h_dx = {};
            if (auto h = schema.get_handle("edge"); h) cache.h_edge = *h;
            else cache.h_edge = {};
        }
        h_id = cache.h_id;
        h_detector = cache.h_detector;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_cx = cache.h_cx;
        h_cy = cache.h_cy;
        h_cz = cache.h_cz;
        h_p = cache.h_p;
        h_phi = cache.h_phi;
        h_theta = cache.h_theta;
        h_langle = cache.h_langle;
        h_centroid = cache.h_centroid;
        h_path = cache.h_path;
        h_dx = cache.h_dx;
        h_edge = cache.h_edge;
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
    /// detector: id of the detector
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_detector, row));
    }
    /// sector: id of the sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
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
    /// cx: track x direction at detector surface (cm)
    [[nodiscard]] float cx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cx, row));
    }
    /// cy: track y direction at detector surface (cm)
    [[nodiscard]] float cy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cy, row));
    }
    /// cz: track z direction at detector surface (cm)
    [[nodiscard]] float cz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cz, row));
    }
    /// p: track momentum (GeV)
    [[nodiscard]] float p(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p, row));
    }
    /// phi: local phi at detector surface (rad)
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_phi, row));
    }
    /// theta: local theta at detector surface (rad)
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_theta, row));
    }
    /// langle: local angle with module surface (rad)
    [[nodiscard]] float langle(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_langle, row));
    }
    /// centroid: estimated strip centroid
    [[nodiscard]] float centroid(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroid, row));
    }
    /// path: pathlength of the track from the track vertex to the detector surface (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }
    /// dx: pathlength of the track within the detector (cm)
    [[nodiscard]] float dx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dx, row));
    }
    /// edge: distance to the closest detector edge (cm), -1 if edge is not defined, 0 if outside, >0 if inside
    [[nodiscard]] float edge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_edge, row));
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
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
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
    /// p column as a contiguous span.
    [[nodiscard]] std::span<const float> p_col() const noexcept {
        return m_view.column<float>(h_p);
    }
    /// phi column as a contiguous span.
    [[nodiscard]] std::span<const float> phi_col() const noexcept {
        return m_view.column<float>(h_phi);
    }
    /// theta column as a contiguous span.
    [[nodiscard]] std::span<const float> theta_col() const noexcept {
        return m_view.column<float>(h_theta);
    }
    /// langle column as a contiguous span.
    [[nodiscard]] std::span<const float> langle_col() const noexcept {
        return m_view.column<float>(h_langle);
    }
    /// centroid column as a contiguous span.
    [[nodiscard]] std::span<const float> centroid_col() const noexcept {
        return m_view.column<float>(h_centroid);
    }
    /// path column as a contiguous span.
    [[nodiscard]] std::span<const float> path_col() const noexcept {
        return m_view.column<float>(h_path);
    }
    /// dx column as a contiguous span.
    [[nodiscard]] std::span<const float> dx_col() const noexcept {
        return m_view.column<float>(h_dx);
    }
    /// edge column as a contiguous span.
    [[nodiscard]] std::span<const float> edge_col() const noexcept {
        return m_view.column<float>(h_edge);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_detector{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_cx{};
    hipo::column_handle h_cy{};
    hipo::column_handle h_cz{};
    hipo::column_handle h_p{};
    hipo::column_handle h_phi{};
    hipo::column_handle h_theta{};
    hipo::column_handle h_langle{};
    hipo::column_handle h_centroid{};
    hipo::column_handle h_path{};
    hipo::column_handle h_dx{};
    hipo::column_handle h_edge{};

    friend class Row;
};

/// Lightweight row view for CVT::Trajectory. Stores a pointer + row index only.
class CVT_Trajectory::Row {
 public:
    Row(const CVT_Trajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float cx() const noexcept { return m_bank->cx(m_row); }
    [[nodiscard]] float cy() const noexcept { return m_bank->cy(m_row); }
    [[nodiscard]] float cz() const noexcept { return m_bank->cz(m_row); }
    [[nodiscard]] float p() const noexcept { return m_bank->p(m_row); }
    [[nodiscard]] float phi() const noexcept { return m_bank->phi(m_row); }
    [[nodiscard]] float theta() const noexcept { return m_bank->theta(m_row); }
    [[nodiscard]] float langle() const noexcept { return m_bank->langle(m_row); }
    [[nodiscard]] float centroid() const noexcept { return m_bank->centroid(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float edge() const noexcept { return m_bank->edge(m_row); }
 private:
    const CVT_Trajectory* m_bank;
    int m_row;
};

inline CVT_Trajectory::Row CVT_Trajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
