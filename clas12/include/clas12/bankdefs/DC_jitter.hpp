// =============================================================================
// Auto-generated from data.json - do not edit by hand.
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


/// HIPO bank wrapper for `DC::jitter` (group 20600, item 13).
/// DCRB timestamps bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// DC_jitter bank;
/// if (auto sc = bank.bind(event.get_bank("DC::jitter")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class DC_jitter {
 public:
    static constexpr std::string_view bank_name = "DC::jitter";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 13;

    class Row;

    DC_jitter() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("crate"); h) h_crate = *h;
        else h_crate = {};
        if (auto h = schema.get_handle("slot"); h) h_slot = *h;
        else h_slot = {};
        if (auto h = schema.get_handle("timestamp"); h) h_timestamp = *h;
        else h_timestamp = {};
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
    /// crate: crate
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_crate, row));
    }
    /// slot: slot
    [[nodiscard]] int slot(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_slot, row));
    }
    /// timestamp: timestamp (in units of 8 ns)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_timestamp, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// crate column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> crate_col() const noexcept {
        return m_view.column<int8_t>(h_crate);
    }
    /// slot column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> slot_col() const noexcept {
        return m_view.column<int8_t>(h_slot);
    }
    /// timestamp column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> timestamp_col() const noexcept {
        return m_view.column<int64_t>(h_timestamp);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_crate{};
    hipo::column_handle h_slot{};
    hipo::column_handle h_timestamp{};

    friend class Row;
};

/// Lightweight row view for DC::jitter. Stores a pointer + row index only.
class DC_jitter::Row {
 public:
    Row(const DC_jitter* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int slot() const noexcept { return m_bank->slot(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
 private:
    const DC_jitter* m_bank;
    int m_row;
};

inline DC_jitter::Row DC_jitter::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
