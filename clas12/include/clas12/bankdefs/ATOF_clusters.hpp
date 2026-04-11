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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `ATOF::clusters` (group 22500, item 22).
/// Clusters in ATOF
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ATOF_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("ATOF::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ATOF_clusters {
 public:
    static constexpr std::string_view bank_name = "ATOF::clusters";
    static constexpr int bank_group = 22500;
    static constexpr int bank_item  = 22;

    class Row;

    ATOF_clusters() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("n_bar"); h) h_n_bar = *h;
        else h_n_bar = {};
        if (auto h = schema.get_handle("n_wedge"); h) h_n_wedge = *h;
        else h_n_wedge = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("energy"); h) h_energy = *h;
        else h_energy = {};
        if (auto h = schema.get_handle("pathlength"); h) h_pathlength = *h;
        else h_pathlength = {};
        if (auto h = schema.get_handle("inpathlength"); h) h_inpathlength = *h;
        else h_inpathlength = {};
        if (auto h = schema.get_handle("projID"); h) h_projID = *h;
        else h_projID = {};
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
    /// id: hit id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// n_bar: number of hits from the bars
    [[nodiscard]] int n_bar(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_n_bar, row));
    }
    /// n_wedge: number of hits from the wedges
    [[nodiscard]] int n_wedge(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_n_wedge, row));
    }
    /// time: time in ns
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// x: x position in mm
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: y position in mm
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: z position in mm
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// energy: energy in MeV
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// pathlength: path length to the cluster in mm
    [[nodiscard]] float pathlength(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathlength, row));
    }
    /// inpathlength: path length inside the detector in mm
    [[nodiscard]] float inpathlength(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_inpathlength, row));
    }
    /// projID: id of the projected track matched. it is -1 if a straight track was assigned.
    [[nodiscard]] int projID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_projID, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// n_bar column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> n_bar_col() const noexcept {
        return m_view.column<int32_t>(h_n_bar);
    }
    /// n_wedge column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> n_wedge_col() const noexcept {
        return m_view.column<int32_t>(h_n_wedge);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
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
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// pathlength column as a contiguous span.
    [[nodiscard]] std::span<const float> pathlength_col() const noexcept {
        return m_view.column<float>(h_pathlength);
    }
    /// inpathlength column as a contiguous span.
    [[nodiscard]] std::span<const float> inpathlength_col() const noexcept {
        return m_view.column<float>(h_inpathlength);
    }
    /// projID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> projID_col() const noexcept {
        return m_view.column<int16_t>(h_projID);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_n_bar{};
    hipo::column_handle h_n_wedge{};
    hipo::column_handle h_time{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_pathlength{};
    hipo::column_handle h_inpathlength{};
    hipo::column_handle h_projID{};

    friend class Row;
};

/// Lightweight row view for ATOF::clusters. Stores a pointer + row index only.
class ATOF_clusters::Row {
 public:
    Row(const ATOF_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int n_bar() const noexcept { return m_bank->n_bar(m_row); }
    [[nodiscard]] int n_wedge() const noexcept { return m_bank->n_wedge(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float pathlength() const noexcept { return m_bank->pathlength(m_row); }
    [[nodiscard]] float inpathlength() const noexcept { return m_bank->inpathlength(m_row); }
    [[nodiscard]] int projID() const noexcept { return m_bank->projID(m_row); }
 private:
    const ATOF_clusters* m_bank;
    int m_row;
};

inline ATOF_clusters::Row ATOF_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
