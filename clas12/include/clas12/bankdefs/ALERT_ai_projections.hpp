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


/// HIPO bank wrapper for `ALERT::ai:projections` (group 23000, item 32).
/// Track Projections to ATOF given by AI
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ALERT_ai_projections bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::ai:projections")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trackid = row.trackid();
/// }
/// @endcode
class ALERT_ai_projections {
 public:
    static constexpr std::string_view bank_name = "ALERT::ai:projections";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 32;

    class Row;

    ALERT_ai_projections() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("trackid"); h) h_trackid = *h;
        else h_trackid = {};
        if (auto h = schema.get_handle("matched_atof_hit_id"); h) h_matched_atof_hit_id = *h;
        else h_matched_atof_hit_id = {};
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
    /// trackid: track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trackid, row));
    }
    /// matched_atof_hit_id: id of the matched ATOF hit, -1 if no hit was matched
    [[nodiscard]] int matched_atof_hit_id(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_matched_atof_hit_id, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// trackid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trackid_col() const noexcept {
        return m_view.column<int32_t>(h_trackid);
    }
    /// matched_atof_hit_id column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> matched_atof_hit_id_col() const noexcept {
        return m_view.column<int32_t>(h_matched_atof_hit_id);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_trackid{};
    hipo::column_handle h_matched_atof_hit_id{};

    friend class Row;
};

/// Lightweight row view for ALERT::ai:projections. Stores a pointer + row index only.
class ALERT_ai_projections::Row {
 public:
    Row(const ALERT_ai_projections* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int matched_atof_hit_id() const noexcept { return m_bank->matched_atof_hit_id(m_row); }
 private:
    const ALERT_ai_projections* m_bank;
    int m_row;
};

inline ALERT_ai_projections::Row ALERT_ai_projections::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
