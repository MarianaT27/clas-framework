// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
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


/// HIPO bank wrapper for `RECAI::ScintExtras` (group 400, item 43).
/// Scintillator Cluster Parameters
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RECAI_ScintExtras bank;
/// if (auto sc = bank.bind(event.get_bank("RECAI::ScintExtras")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto dedx = row.dedx();
/// }
/// @endcode
class RECAI_ScintExtras {
 public:
    static constexpr std::string_view bank_name = "RECAI::ScintExtras";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 43;

    class Row;

    RECAI_ScintExtras() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("dedx"); h) h_dedx = *h;
        else h_dedx = {};
        if (auto h = schema.get_handle("size"); h) h_size = *h;
        else h_size = {};
        if (auto h = schema.get_handle("layermult"); h) h_layermult = *h;
        else h_layermult = {};
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
    /// dedx: Energy deposition per distance travelled (MeV/cm)
    [[nodiscard]] float dedx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dedx, row));
    }
    /// size: cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// layermult: cluster layer multiplicity
    [[nodiscard]] int layermult(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layermult, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// dedx column as a contiguous span.
    [[nodiscard]] std::span<const float> dedx_col() const noexcept {
        return m_view.column<float>(h_dedx);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// layermult column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layermult_col() const noexcept {
        return m_view.column<int8_t>(h_layermult);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_dedx{};
    hipo::column_handle h_size{};
    hipo::column_handle h_layermult{};

    friend class Row;
};

/// Lightweight row view for RECAI::ScintExtras. Stores a pointer + row index only.
class RECAI_ScintExtras::Row {
 public:
    Row(const RECAI_ScintExtras* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float dedx() const noexcept { return m_bank->dedx(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int layermult() const noexcept { return m_bank->layermult(m_row); }
 private:
    const RECAI_ScintExtras* m_bank;
    int m_row;
};

inline RECAI_ScintExtras::Row RECAI_ScintExtras::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
