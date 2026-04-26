// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
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


/// HIPO bank wrapper for `TimeBasedTrkg::AITrajectory` (group 20600, item 93).
/// Trajectory bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// TimeBasedTrkg_AITrajectory bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::AITrajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_AITrajectory {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::AITrajectory";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 93;

    class Row;

    TimeBasedTrkg_AITrajectory() = default;

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
            hipo::column_handle h_tx;
            hipo::column_handle h_ty;
            hipo::column_handle h_tz;
            hipo::column_handle h_B;
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
            if (auto h = schema.get_handle("tx"); h) cache.h_tx = *h;
            else cache.h_tx = {};
            if (auto h = schema.get_handle("ty"); h) cache.h_ty = *h;
            else cache.h_ty = {};
            if (auto h = schema.get_handle("tz"); h) cache.h_tz = *h;
            else cache.h_tz = {};
            if (auto h = schema.get_handle("B"); h) cache.h_B = *h;
            else cache.h_B = {};
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
        h_tx = cache.h_tx;
        h_ty = cache.h_ty;
        h_tz = cache.h_tz;
        h_B = cache.h_B;
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
    /// B: B-field magnitude at detector surface (cm)
    [[nodiscard]] float B(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_B, row));
    }
    /// path: pathlength of the track from the track vertex to the detector surface (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }
    /// dx: pathlength of the track through the detector element (cm)
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
    /// B column as a contiguous span.
    [[nodiscard]] std::span<const float> B_col() const noexcept {
        return m_view.column<float>(h_B);
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
    hipo::column_handle h_tx{};
    hipo::column_handle h_ty{};
    hipo::column_handle h_tz{};
    hipo::column_handle h_B{};
    hipo::column_handle h_path{};
    hipo::column_handle h_dx{};
    hipo::column_handle h_edge{};

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::AITrajectory. Stores a pointer + row index only.
class TimeBasedTrkg_AITrajectory::Row {
 public:
    Row(const TimeBasedTrkg_AITrajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] float B() const noexcept { return m_bank->B(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float edge() const noexcept { return m_bank->edge(m_row); }
 private:
    const TimeBasedTrkg_AITrajectory* m_bank;
    int m_row;
};

inline TimeBasedTrkg_AITrajectory::Row TimeBasedTrkg_AITrajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
