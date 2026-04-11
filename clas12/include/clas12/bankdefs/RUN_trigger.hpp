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


/// HIPO bank wrapper for `RUN::trigger` (group 10000, item 13).
/// RUN trigger information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RUN_trigger bank;
/// if (auto sc = bank.bind(event.get_bank("RUN::trigger")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RUN_trigger {
 public:
    static constexpr std::string_view bank_name = "RUN::trigger";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 13;

    class Row;

    RUN_trigger() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("trigger"); h) h_trigger = *h;
        else h_trigger = {};
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
    /// id: id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_id, row));
    }
    /// trigger: trigger word
    [[nodiscard]] int trigger(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trigger, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> id_col() const noexcept {
        return m_view.column<int32_t>(h_id);
    }
    /// trigger column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trigger_col() const noexcept {
        return m_view.column<int32_t>(h_trigger);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_trigger{};

    friend class Row;
};

/// Lightweight row view for RUN::trigger. Stores a pointer + row index only.
class RUN_trigger::Row {
 public:
    Row(const RUN_trigger* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int trigger() const noexcept { return m_bank->trigger(m_row); }
 private:
    const RUN_trigger* m_bank;
    int m_row;
};

inline RUN_trigger::Row RUN_trigger::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
