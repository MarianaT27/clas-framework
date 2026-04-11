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


/// HIPO bank wrapper for `REC::CovMat` (group 300, item 38).
/// reconstructed track covariance matrix
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// REC_CovMat bank;
/// if (auto sc = bank.bind(event.get_bank("REC::CovMat")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class REC_CovMat {
 public:
    static constexpr std::string_view bank_name = "REC::CovMat";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 38;

    class Row;

    REC_CovMat() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("index"); h) h_index = *h;
        else h_index = {};
        if (auto h = schema.get_handle("pindex"); h) h_pindex = *h;
        else h_pindex = {};
        if (auto h = schema.get_handle("C11"); h) h_C11 = *h;
        else h_C11 = {};
        if (auto h = schema.get_handle("C12"); h) h_C12 = *h;
        else h_C12 = {};
        if (auto h = schema.get_handle("C13"); h) h_C13 = *h;
        else h_C13 = {};
        if (auto h = schema.get_handle("C14"); h) h_C14 = *h;
        else h_C14 = {};
        if (auto h = schema.get_handle("C15"); h) h_C15 = *h;
        else h_C15 = {};
        if (auto h = schema.get_handle("C16"); h) h_C16 = *h;
        else h_C16 = {};
        if (auto h = schema.get_handle("C22"); h) h_C22 = *h;
        else h_C22 = {};
        if (auto h = schema.get_handle("C23"); h) h_C23 = *h;
        else h_C23 = {};
        if (auto h = schema.get_handle("C24"); h) h_C24 = *h;
        else h_C24 = {};
        if (auto h = schema.get_handle("C25"); h) h_C25 = *h;
        else h_C25 = {};
        if (auto h = schema.get_handle("C26"); h) h_C26 = *h;
        else h_C26 = {};
        if (auto h = schema.get_handle("C33"); h) h_C33 = *h;
        else h_C33 = {};
        if (auto h = schema.get_handle("C34"); h) h_C34 = *h;
        else h_C34 = {};
        if (auto h = schema.get_handle("C35"); h) h_C35 = *h;
        else h_C35 = {};
        if (auto h = schema.get_handle("C36"); h) h_C36 = *h;
        else h_C36 = {};
        if (auto h = schema.get_handle("C44"); h) h_C44 = *h;
        else h_C44 = {};
        if (auto h = schema.get_handle("C45"); h) h_C45 = *h;
        else h_C45 = {};
        if (auto h = schema.get_handle("C46"); h) h_C46 = *h;
        else h_C46 = {};
        if (auto h = schema.get_handle("C55"); h) h_C55 = *h;
        else h_C55 = {};
        if (auto h = schema.get_handle("C56"); h) h_C56 = *h;
        else h_C56 = {};
        if (auto h = schema.get_handle("C66"); h) h_C66 = *h;
        else h_C66 = {};
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
    /// index: index of the track in Tracks bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// pindex: row number in the particle bank hit is associated with
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pindex, row));
    }
    /// C11: var(x) (cm^2)
    [[nodiscard]] float C11(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C11, row));
    }
    /// C12: cov(x,y)
    [[nodiscard]] float C12(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C12, row));
    }
    /// C13: cov(x,z)
    [[nodiscard]] float C13(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C13, row));
    }
    /// C14: cov(x,theta)
    [[nodiscard]] float C14(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C14, row));
    }
    /// C15: cov(x,phi)
    [[nodiscard]] float C15(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C15, row));
    }
    /// C16: cov(x,p)
    [[nodiscard]] float C16(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C16, row));
    }
    /// C22: var(y) (cm^2)
    [[nodiscard]] float C22(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C22, row));
    }
    /// C23: cov(y,z)
    [[nodiscard]] float C23(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C23, row));
    }
    /// C24: cov(y,theta)
    [[nodiscard]] float C24(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C24, row));
    }
    /// C25: cov(y,phi)
    [[nodiscard]] float C25(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C25, row));
    }
    /// C26: cov(y,p)
    [[nodiscard]] float C26(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C26, row));
    }
    /// C33: var(z) (cm^2)
    [[nodiscard]] float C33(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C33, row));
    }
    /// C34: cov(z,theta)
    [[nodiscard]] float C34(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C34, row));
    }
    /// C35: cov(z,phi)
    [[nodiscard]] float C35(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C35, row));
    }
    /// C36: cov(z,p)
    [[nodiscard]] float C36(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C36, row));
    }
    /// C44: var(theta) (rad^2)
    [[nodiscard]] float C44(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C44, row));
    }
    /// C45: cov(theta,phi)
    [[nodiscard]] float C45(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C45, row));
    }
    /// C46: cov(theta,p)
    [[nodiscard]] float C46(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C46, row));
    }
    /// C55: var(phi) (rad^2)
    [[nodiscard]] float C55(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C55, row));
    }
    /// C56: cov(phi,p)
    [[nodiscard]] float C56(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C56, row));
    }
    /// C66: var(p) (GeV^2)
    [[nodiscard]] float C66(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C66, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index_col() const noexcept {
        return m_view.column<int16_t>(h_index);
    }
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pindex_col() const noexcept {
        return m_view.column<int16_t>(h_pindex);
    }
    /// C11 column as a contiguous span.
    [[nodiscard]] std::span<const float> C11_col() const noexcept {
        return m_view.column<float>(h_C11);
    }
    /// C12 column as a contiguous span.
    [[nodiscard]] std::span<const float> C12_col() const noexcept {
        return m_view.column<float>(h_C12);
    }
    /// C13 column as a contiguous span.
    [[nodiscard]] std::span<const float> C13_col() const noexcept {
        return m_view.column<float>(h_C13);
    }
    /// C14 column as a contiguous span.
    [[nodiscard]] std::span<const float> C14_col() const noexcept {
        return m_view.column<float>(h_C14);
    }
    /// C15 column as a contiguous span.
    [[nodiscard]] std::span<const float> C15_col() const noexcept {
        return m_view.column<float>(h_C15);
    }
    /// C16 column as a contiguous span.
    [[nodiscard]] std::span<const float> C16_col() const noexcept {
        return m_view.column<float>(h_C16);
    }
    /// C22 column as a contiguous span.
    [[nodiscard]] std::span<const float> C22_col() const noexcept {
        return m_view.column<float>(h_C22);
    }
    /// C23 column as a contiguous span.
    [[nodiscard]] std::span<const float> C23_col() const noexcept {
        return m_view.column<float>(h_C23);
    }
    /// C24 column as a contiguous span.
    [[nodiscard]] std::span<const float> C24_col() const noexcept {
        return m_view.column<float>(h_C24);
    }
    /// C25 column as a contiguous span.
    [[nodiscard]] std::span<const float> C25_col() const noexcept {
        return m_view.column<float>(h_C25);
    }
    /// C26 column as a contiguous span.
    [[nodiscard]] std::span<const float> C26_col() const noexcept {
        return m_view.column<float>(h_C26);
    }
    /// C33 column as a contiguous span.
    [[nodiscard]] std::span<const float> C33_col() const noexcept {
        return m_view.column<float>(h_C33);
    }
    /// C34 column as a contiguous span.
    [[nodiscard]] std::span<const float> C34_col() const noexcept {
        return m_view.column<float>(h_C34);
    }
    /// C35 column as a contiguous span.
    [[nodiscard]] std::span<const float> C35_col() const noexcept {
        return m_view.column<float>(h_C35);
    }
    /// C36 column as a contiguous span.
    [[nodiscard]] std::span<const float> C36_col() const noexcept {
        return m_view.column<float>(h_C36);
    }
    /// C44 column as a contiguous span.
    [[nodiscard]] std::span<const float> C44_col() const noexcept {
        return m_view.column<float>(h_C44);
    }
    /// C45 column as a contiguous span.
    [[nodiscard]] std::span<const float> C45_col() const noexcept {
        return m_view.column<float>(h_C45);
    }
    /// C46 column as a contiguous span.
    [[nodiscard]] std::span<const float> C46_col() const noexcept {
        return m_view.column<float>(h_C46);
    }
    /// C55 column as a contiguous span.
    [[nodiscard]] std::span<const float> C55_col() const noexcept {
        return m_view.column<float>(h_C55);
    }
    /// C56 column as a contiguous span.
    [[nodiscard]] std::span<const float> C56_col() const noexcept {
        return m_view.column<float>(h_C56);
    }
    /// C66 column as a contiguous span.
    [[nodiscard]] std::span<const float> C66_col() const noexcept {
        return m_view.column<float>(h_C66);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle index_handle() const noexcept { return h_index; }
    [[nodiscard]] hipo::column_handle pindex_handle() const noexcept { return h_pindex; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_index{};
    hipo::column_handle h_pindex{};
    hipo::column_handle h_C11{};
    hipo::column_handle h_C12{};
    hipo::column_handle h_C13{};
    hipo::column_handle h_C14{};
    hipo::column_handle h_C15{};
    hipo::column_handle h_C16{};
    hipo::column_handle h_C22{};
    hipo::column_handle h_C23{};
    hipo::column_handle h_C24{};
    hipo::column_handle h_C25{};
    hipo::column_handle h_C26{};
    hipo::column_handle h_C33{};
    hipo::column_handle h_C34{};
    hipo::column_handle h_C35{};
    hipo::column_handle h_C36{};
    hipo::column_handle h_C44{};
    hipo::column_handle h_C45{};
    hipo::column_handle h_C46{};
    hipo::column_handle h_C55{};
    hipo::column_handle h_C56{};
    hipo::column_handle h_C66{};

    friend class Row;
};

/// Lightweight row view for REC::CovMat. Stores a pointer + row index only.
class REC_CovMat::Row {
 public:
    Row(const REC_CovMat* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] float C11() const noexcept { return m_bank->C11(m_row); }
    [[nodiscard]] float C12() const noexcept { return m_bank->C12(m_row); }
    [[nodiscard]] float C13() const noexcept { return m_bank->C13(m_row); }
    [[nodiscard]] float C14() const noexcept { return m_bank->C14(m_row); }
    [[nodiscard]] float C15() const noexcept { return m_bank->C15(m_row); }
    [[nodiscard]] float C16() const noexcept { return m_bank->C16(m_row); }
    [[nodiscard]] float C22() const noexcept { return m_bank->C22(m_row); }
    [[nodiscard]] float C23() const noexcept { return m_bank->C23(m_row); }
    [[nodiscard]] float C24() const noexcept { return m_bank->C24(m_row); }
    [[nodiscard]] float C25() const noexcept { return m_bank->C25(m_row); }
    [[nodiscard]] float C26() const noexcept { return m_bank->C26(m_row); }
    [[nodiscard]] float C33() const noexcept { return m_bank->C33(m_row); }
    [[nodiscard]] float C34() const noexcept { return m_bank->C34(m_row); }
    [[nodiscard]] float C35() const noexcept { return m_bank->C35(m_row); }
    [[nodiscard]] float C36() const noexcept { return m_bank->C36(m_row); }
    [[nodiscard]] float C44() const noexcept { return m_bank->C44(m_row); }
    [[nodiscard]] float C45() const noexcept { return m_bank->C45(m_row); }
    [[nodiscard]] float C46() const noexcept { return m_bank->C46(m_row); }
    [[nodiscard]] float C55() const noexcept { return m_bank->C55(m_row); }
    [[nodiscard]] float C56() const noexcept { return m_bank->C56(m_row); }
    [[nodiscard]] float C66() const noexcept { return m_bank->C66(m_row); }
 private:
    const REC_CovMat* m_bank;
    int m_row;
};

inline REC_CovMat::Row REC_CovMat::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
