// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
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

namespace bankdefs {


/// HIPO bank wrapper for `ALERT::projections` (group 23000, item 31).
/// Track Projections to ATOF
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ALERT_projections bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::projections")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ALERT_projections {
 public:
    static constexpr std::string_view bank_name = "ALERT::projections";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 31;

    class Row;

    ALERT_projections() = default;

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
            hipo::column_handle h_trackid;
            hipo::column_handle h_x_at_bar;
            hipo::column_handle h_y_at_bar;
            hipo::column_handle h_z_at_bar;
            hipo::column_handle h_l_at_bar;
            hipo::column_handle h_l_in_bar;
            hipo::column_handle h_x_at_wedge;
            hipo::column_handle h_y_at_wedge;
            hipo::column_handle h_z_at_wedge;
            hipo::column_handle h_l_at_wedge;
            hipo::column_handle h_l_in_wedge;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("trackid"); h) cache.h_trackid = *h;
            else cache.h_trackid = {};
            if (auto h = schema.get_handle("x_at_bar"); h) cache.h_x_at_bar = *h;
            else cache.h_x_at_bar = {};
            if (auto h = schema.get_handle("y_at_bar"); h) cache.h_y_at_bar = *h;
            else cache.h_y_at_bar = {};
            if (auto h = schema.get_handle("z_at_bar"); h) cache.h_z_at_bar = *h;
            else cache.h_z_at_bar = {};
            if (auto h = schema.get_handle("l_at_bar"); h) cache.h_l_at_bar = *h;
            else cache.h_l_at_bar = {};
            if (auto h = schema.get_handle("l_in_bar"); h) cache.h_l_in_bar = *h;
            else cache.h_l_in_bar = {};
            if (auto h = schema.get_handle("x_at_wedge"); h) cache.h_x_at_wedge = *h;
            else cache.h_x_at_wedge = {};
            if (auto h = schema.get_handle("y_at_wedge"); h) cache.h_y_at_wedge = *h;
            else cache.h_y_at_wedge = {};
            if (auto h = schema.get_handle("z_at_wedge"); h) cache.h_z_at_wedge = *h;
            else cache.h_z_at_wedge = {};
            if (auto h = schema.get_handle("l_at_wedge"); h) cache.h_l_at_wedge = *h;
            else cache.h_l_at_wedge = {};
            if (auto h = schema.get_handle("l_in_wedge"); h) cache.h_l_in_wedge = *h;
            else cache.h_l_in_wedge = {};
        }
        h_id = cache.h_id;
        h_trackid = cache.h_trackid;
        h_x_at_bar = cache.h_x_at_bar;
        h_y_at_bar = cache.h_y_at_bar;
        h_z_at_bar = cache.h_z_at_bar;
        h_l_at_bar = cache.h_l_at_bar;
        h_l_in_bar = cache.h_l_in_bar;
        h_x_at_wedge = cache.h_x_at_wedge;
        h_y_at_wedge = cache.h_y_at_wedge;
        h_z_at_wedge = cache.h_z_at_wedge;
        h_l_at_wedge = cache.h_l_at_wedge;
        h_l_in_wedge = cache.h_l_in_wedge;
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
    /// id: projection id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// trackid: id of the track that was projected
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trackid, row));
    }
    /// x_at_bar: x position at atof bar (middle surface) in mm
    [[nodiscard]] float x_at_bar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x_at_bar, row));
    }
    /// y_at_bar: y position at atof bar (middle surface) in mm
    [[nodiscard]] float y_at_bar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y_at_bar, row));
    }
    /// z_at_bar: z position at atof bar (middle surface) in mm
    [[nodiscard]] float z_at_bar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z_at_bar, row));
    }
    /// l_at_bar: path length at atof bar (inner surface) in mm
    [[nodiscard]] float l_at_bar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_l_at_bar, row));
    }
    /// l_in_bar: path length inside atof bar in mm
    [[nodiscard]] float l_in_bar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_l_in_bar, row));
    }
    /// x_at_wedge: x position at atof wedge (middle surface) in mm
    [[nodiscard]] float x_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x_at_wedge, row));
    }
    /// y_at_wedge: y position at atof wedge (middle surface) in mm
    [[nodiscard]] float y_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y_at_wedge, row));
    }
    /// z_at_wedge: z position at atof wedge (middle surface) in mm
    [[nodiscard]] float z_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z_at_wedge, row));
    }
    /// l_at_wedge: path length at atof wedge (inner surface) in mm
    [[nodiscard]] float l_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_l_at_wedge, row));
    }
    /// l_in_wedge: path length inside atof wedge in mm
    [[nodiscard]] float l_in_wedge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_l_in_wedge, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// trackid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trackid_col() const noexcept {
        return m_view.column<int16_t>(h_trackid);
    }
    /// x_at_bar column as a contiguous span.
    [[nodiscard]] std::span<const float> x_at_bar_col() const noexcept {
        return m_view.column<float>(h_x_at_bar);
    }
    /// y_at_bar column as a contiguous span.
    [[nodiscard]] std::span<const float> y_at_bar_col() const noexcept {
        return m_view.column<float>(h_y_at_bar);
    }
    /// z_at_bar column as a contiguous span.
    [[nodiscard]] std::span<const float> z_at_bar_col() const noexcept {
        return m_view.column<float>(h_z_at_bar);
    }
    /// l_at_bar column as a contiguous span.
    [[nodiscard]] std::span<const float> l_at_bar_col() const noexcept {
        return m_view.column<float>(h_l_at_bar);
    }
    /// l_in_bar column as a contiguous span.
    [[nodiscard]] std::span<const float> l_in_bar_col() const noexcept {
        return m_view.column<float>(h_l_in_bar);
    }
    /// x_at_wedge column as a contiguous span.
    [[nodiscard]] std::span<const float> x_at_wedge_col() const noexcept {
        return m_view.column<float>(h_x_at_wedge);
    }
    /// y_at_wedge column as a contiguous span.
    [[nodiscard]] std::span<const float> y_at_wedge_col() const noexcept {
        return m_view.column<float>(h_y_at_wedge);
    }
    /// z_at_wedge column as a contiguous span.
    [[nodiscard]] std::span<const float> z_at_wedge_col() const noexcept {
        return m_view.column<float>(h_z_at_wedge);
    }
    /// l_at_wedge column as a contiguous span.
    [[nodiscard]] std::span<const float> l_at_wedge_col() const noexcept {
        return m_view.column<float>(h_l_at_wedge);
    }
    /// l_in_wedge column as a contiguous span.
    [[nodiscard]] std::span<const float> l_in_wedge_col() const noexcept {
        return m_view.column<float>(h_l_in_wedge);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_trackid{};
    hipo::column_handle h_x_at_bar{};
    hipo::column_handle h_y_at_bar{};
    hipo::column_handle h_z_at_bar{};
    hipo::column_handle h_l_at_bar{};
    hipo::column_handle h_l_in_bar{};
    hipo::column_handle h_x_at_wedge{};
    hipo::column_handle h_y_at_wedge{};
    hipo::column_handle h_z_at_wedge{};
    hipo::column_handle h_l_at_wedge{};
    hipo::column_handle h_l_in_wedge{};

    friend class Row;
};

/// Lightweight row view for ALERT::projections. Stores a pointer + row index only.
class ALERT_projections::Row {
 public:
    Row(const ALERT_projections* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] float x_at_bar() const noexcept { return m_bank->x_at_bar(m_row); }
    [[nodiscard]] float y_at_bar() const noexcept { return m_bank->y_at_bar(m_row); }
    [[nodiscard]] float z_at_bar() const noexcept { return m_bank->z_at_bar(m_row); }
    [[nodiscard]] float l_at_bar() const noexcept { return m_bank->l_at_bar(m_row); }
    [[nodiscard]] float l_in_bar() const noexcept { return m_bank->l_in_bar(m_row); }
    [[nodiscard]] float x_at_wedge() const noexcept { return m_bank->x_at_wedge(m_row); }
    [[nodiscard]] float y_at_wedge() const noexcept { return m_bank->y_at_wedge(m_row); }
    [[nodiscard]] float z_at_wedge() const noexcept { return m_bank->z_at_wedge(m_row); }
    [[nodiscard]] float l_at_wedge() const noexcept { return m_bank->l_at_wedge(m_row); }
    [[nodiscard]] float l_in_wedge() const noexcept { return m_bank->l_in_wedge(m_row); }
 private:
    const ALERT_projections* m_bank;
    int m_row;
};

inline ALERT_projections::Row ALERT_projections::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
