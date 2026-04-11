// =============================================================================
// Auto-generated from ecal.json - do not edit by hand.
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


/// HIPO bank wrapper for `ECAL::scaler` (group 20700, item 26).
/// Occupancy bank for the Electromagnetic Calorimeter
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ECAL_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class ECAL_scaler {
 public:
    static constexpr std::string_view bank_name = "ECAL::scaler";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 26;

    class Row;

    ECAL_scaler() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("component"); h) h_component = *h;
        else h_component = {};
        if (auto h = schema.get_handle("acount"); h) h_acount = *h;
        else h_acount = {};
        if (auto h = schema.get_handle("tcount"); h) h_tcount = *h;
        else h_tcount = {};
        if (auto h = schema.get_handle("avalue"); h) h_avalue = *h;
        else h_avalue = {};
        if (auto h = schema.get_handle("tvalue"); h) h_tvalue = *h;
        else h_tvalue = {};
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
    /// sector: sector (1-6)
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer (1..9)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: strip
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// acount: ADC counts
    [[nodiscard]] int acount(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_acount, row));
    }
    /// tcount: TDC counts
    [[nodiscard]] int tcount(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tcount, row));
    }
    /// avalue: ADC value
    [[nodiscard]] int avalue(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_avalue, row));
    }
    /// tvalue: TDC value
    [[nodiscard]] int tvalue(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tvalue, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
    }
    /// acount column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> acount_col() const noexcept {
        return m_view.column<int32_t>(h_acount);
    }
    /// tcount column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tcount_col() const noexcept {
        return m_view.column<int32_t>(h_tcount);
    }
    /// avalue column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> avalue_col() const noexcept {
        return m_view.column<int32_t>(h_avalue);
    }
    /// tvalue column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tvalue_col() const noexcept {
        return m_view.column<int32_t>(h_tvalue);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_acount{};
    hipo::column_handle h_tcount{};
    hipo::column_handle h_avalue{};
    hipo::column_handle h_tvalue{};

    friend class Row;
};

/// Lightweight row view for ECAL::scaler. Stores a pointer + row index only.
class ECAL_scaler::Row {
 public:
    Row(const ECAL_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int acount() const noexcept { return m_bank->acount(m_row); }
    [[nodiscard]] int tcount() const noexcept { return m_bank->tcount(m_row); }
    [[nodiscard]] int avalue() const noexcept { return m_bank->avalue(m_row); }
    [[nodiscard]] int tvalue() const noexcept { return m_bank->tvalue(m_row); }
 private:
    const ECAL_scaler* m_bank;
    int m_row;
};

inline ECAL_scaler::Row ECAL_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
