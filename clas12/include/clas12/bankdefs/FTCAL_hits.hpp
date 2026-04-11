// =============================================================================
// Auto-generated from ft.json - do not edit by hand.
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


/// HIPO bank wrapper for `FTCAL::hits` (group 21000, item 21).
/// Reconstructed Hits in FT calorimeter
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FTCAL_hits bank;
/// if (auto sc = bank.bind(event.get_bank("FTCAL::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto idx = row.idx();
/// }
/// @endcode
class FTCAL_hits {
 public:
    static constexpr std::string_view bank_name = "FTCAL::hits";
    static constexpr int bank_group = 21000;
    static constexpr int bank_item  = 21;

    class Row;

    FTCAL_hits() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("idx"); h) h_idx = *h;
        else h_idx = {};
        if (auto h = schema.get_handle("idy"); h) h_idy = *h;
        else h_idy = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("energy"); h) h_energy = *h;
        else h_energy = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("hitID"); h) h_hitID = *h;
        else h_hitID = {};
        if (auto h = schema.get_handle("clusterID"); h) h_clusterID = *h;
        else h_clusterID = {};
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
    /// idx: x id of grid
    [[nodiscard]] int idx(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_idx, row));
    }
    /// idy: y id of grid
    [[nodiscard]] int idy(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_idy, row));
    }
    /// x: Hit X position (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Hit Y position (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Hit Z position (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// energy: Hit Energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Hit Time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// hitID: Hit Pointer to ADC bank
    [[nodiscard]] int hitID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_hitID, row));
    }
    /// clusterID: Hit Pointer to Cluster Bank
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusterID, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// idx column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> idx_col() const noexcept {
        return m_view.column<int8_t>(h_idx);
    }
    /// idy column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> idy_col() const noexcept {
        return m_view.column<int8_t>(h_idy);
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
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// hitID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> hitID_col() const noexcept {
        return m_view.column<int16_t>(h_hitID);
    }
    /// clusterID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusterID_col() const noexcept {
        return m_view.column<int16_t>(h_clusterID);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_idx{};
    hipo::column_handle h_idy{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_hitID{};
    hipo::column_handle h_clusterID{};

    friend class Row;
};

/// Lightweight row view for FTCAL::hits. Stores a pointer + row index only.
class FTCAL_hits::Row {
 public:
    Row(const FTCAL_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int idx() const noexcept { return m_bank->idx(m_row); }
    [[nodiscard]] int idy() const noexcept { return m_bank->idy(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int hitID() const noexcept { return m_bank->hitID(m_row); }
    [[nodiscard]] int clusterID() const noexcept { return m_bank->clusterID(m_row); }
 private:
    const FTCAL_hits* m_bank;
    int m_row;
};

inline FTCAL_hits::Row FTCAL_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
