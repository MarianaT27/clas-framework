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


/// HIPO bank wrapper for `RECFT::Event` (group 300, item 41).
/// Event Header Bank, shadowing REC::Event with FT-based start time
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RECFT_Event bank;
/// if (auto sc = bank.bind(event.get_bank("RECFT::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto category = row.category();
/// }
/// @endcode
class RECFT_Event {
 public:
    static constexpr std::string_view bank_name = "RECFT::Event";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 41;

    class Row;

    RECFT_Event() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("category"); h) h_category = *h;
        else h_category = {};
        if (auto h = schema.get_handle("startTime"); h) h_startTime = *h;
        else h_startTime = {};
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
    /// category: Undefined
    [[nodiscard]] int64_t category(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_category, row));
    }
    /// startTime: Event Start Time (ns)
    [[nodiscard]] float startTime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_startTime, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// category column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> category_col() const noexcept {
        return m_view.column<int64_t>(h_category);
    }
    /// startTime column as a contiguous span.
    [[nodiscard]] std::span<const float> startTime_col() const noexcept {
        return m_view.column<float>(h_startTime);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_category{};
    hipo::column_handle h_startTime{};

    friend class Row;
};

/// Lightweight row view for RECFT::Event. Stores a pointer + row index only.
class RECFT_Event::Row {
 public:
    Row(const RECFT_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int64_t category() const noexcept { return m_bank->category(m_row); }
    [[nodiscard]] float startTime() const noexcept { return m_bank->startTime(m_row); }
 private:
    const RECFT_Event* m_bank;
    int m_row;
};

inline RECFT_Event::Row RECFT_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
