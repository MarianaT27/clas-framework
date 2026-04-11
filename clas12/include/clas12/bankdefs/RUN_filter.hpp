// =============================================================================
// Auto-generated from header.json - do not edit by hand.
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


/// HIPO bank wrapper for `RUN::filter` (group 10000, item 15).
/// Random trigger filter information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RUN_filter bank;
/// if (auto sc = bank.bind(event.get_bank("RUN::filter")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto json = row.json();
/// }
/// @endcode
class RUN_filter {
 public:
    static constexpr std::string_view bank_name = "RUN::filter";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 15;

    class Row;

    RUN_filter() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("json"); h) h_json = *h;
        else h_json = {};
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
    /// json: json character
    [[nodiscard]] int json(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_json, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// json column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> json_col() const noexcept {
        return m_view.column<int8_t>(h_json);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_json{};

    friend class Row;
};

/// Lightweight row view for RUN::filter. Stores a pointer + row index only.
class RUN_filter::Row {
 public:
    Row(const RUN_filter* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int json() const noexcept { return m_bank->json(m_row); }
 private:
    const RUN_filter* m_bank;
    int m_row;
};

inline RUN_filter::Row RUN_filter::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
