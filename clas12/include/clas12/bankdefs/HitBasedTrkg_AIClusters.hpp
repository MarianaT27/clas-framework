// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
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


/// HIPO bank wrapper for `HitBasedTrkg::AIClusters` (group 20600, item 62).
/// reconstructed clusters using DC wire positions
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HitBasedTrkg_AIClusters bank;
/// if (auto sc = bank.bind(event.get_bank("HitBasedTrkg::AIClusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HitBasedTrkg_AIClusters {
 public:
    static constexpr std::string_view bank_name = "HitBasedTrkg::AIClusters";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 62;

    class Row;

    HitBasedTrkg_AIClusters() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("superlayer"); h) h_superlayer = *h;
        else h_superlayer = {};
        if (auto h = schema.get_handle("Hit1_ID"); h) h_Hit1_ID = *h;
        else h_Hit1_ID = {};
        if (auto h = schema.get_handle("Hit2_ID"); h) h_Hit2_ID = *h;
        else h_Hit2_ID = {};
        if (auto h = schema.get_handle("Hit3_ID"); h) h_Hit3_ID = *h;
        else h_Hit3_ID = {};
        if (auto h = schema.get_handle("Hit4_ID"); h) h_Hit4_ID = *h;
        else h_Hit4_ID = {};
        if (auto h = schema.get_handle("Hit5_ID"); h) h_Hit5_ID = *h;
        else h_Hit5_ID = {};
        if (auto h = schema.get_handle("Hit6_ID"); h) h_Hit6_ID = *h;
        else h_Hit6_ID = {};
        if (auto h = schema.get_handle("Hit7_ID"); h) h_Hit7_ID = *h;
        else h_Hit7_ID = {};
        if (auto h = schema.get_handle("Hit8_ID"); h) h_Hit8_ID = *h;
        else h_Hit8_ID = {};
        if (auto h = schema.get_handle("Hit9_ID"); h) h_Hit9_ID = *h;
        else h_Hit9_ID = {};
        if (auto h = schema.get_handle("Hit10_ID"); h) h_Hit10_ID = *h;
        else h_Hit10_ID = {};
        if (auto h = schema.get_handle("Hit11_ID"); h) h_Hit11_ID = *h;
        else h_Hit11_ID = {};
        if (auto h = schema.get_handle("Hit12_ID"); h) h_Hit12_ID = *h;
        else h_Hit12_ID = {};
        if (auto h = schema.get_handle("avgWire"); h) h_avgWire = *h;
        else h_avgWire = {};
        if (auto h = schema.get_handle("fitChisqProb"); h) h_fitChisqProb = *h;
        else h_fitChisqProb = {};
        if (auto h = schema.get_handle("fitSlope"); h) h_fitSlope = *h;
        else h_fitSlope = {};
        if (auto h = schema.get_handle("fitSlopeErr"); h) h_fitSlopeErr = *h;
        else h_fitSlopeErr = {};
        if (auto h = schema.get_handle("fitInterc"); h) h_fitInterc = *h;
        else h_fitInterc = {};
        if (auto h = schema.get_handle("fitIntercErr"); h) h_fitIntercErr = *h;
        else h_fitIntercErr = {};
        if (auto h = schema.get_handle("size"); h) h_size = *h;
        else h_size = {};
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
    /// id: id of the cluster
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: status of the cluster
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: sector of the cluster
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// superlayer: superlayer of the cluster
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_superlayer, row));
    }
    /// Hit1_ID: id of hit1 in cluster
    [[nodiscard]] int Hit1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit1_ID, row));
    }
    /// Hit2_ID: id of hit2 in cluster
    [[nodiscard]] int Hit2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit2_ID, row));
    }
    /// Hit3_ID: id of hit3 in cluster
    [[nodiscard]] int Hit3_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit3_ID, row));
    }
    /// Hit4_ID: id of hit4 in cluster
    [[nodiscard]] int Hit4_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit4_ID, row));
    }
    /// Hit5_ID: id of hit5 in cluster
    [[nodiscard]] int Hit5_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit5_ID, row));
    }
    /// Hit6_ID: id of hit6 in cluster
    [[nodiscard]] int Hit6_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit6_ID, row));
    }
    /// Hit7_ID: id of hit7 in cluster
    [[nodiscard]] int Hit7_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit7_ID, row));
    }
    /// Hit8_ID: id of hit8 in cluster
    [[nodiscard]] int Hit8_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit8_ID, row));
    }
    /// Hit9_ID: id of hit9 in cluster
    [[nodiscard]] int Hit9_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit9_ID, row));
    }
    /// Hit10_ID: id of hit10 in cluster
    [[nodiscard]] int Hit10_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit10_ID, row));
    }
    /// Hit11_ID: id of hit11 in cluster
    [[nodiscard]] int Hit11_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit11_ID, row));
    }
    /// Hit12_ID: id of hit12 in cluster
    [[nodiscard]] int Hit12_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit12_ID, row));
    }
    /// avgWire: average wire number
    [[nodiscard]] float avgWire(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgWire, row));
    }
    /// fitChisqProb: fit chisq prob.
    [[nodiscard]] float fitChisqProb(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitChisqProb, row));
    }
    /// fitSlope: line fit slope
    [[nodiscard]] float fitSlope(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitSlope, row));
    }
    /// fitSlopeErr: error on slope
    [[nodiscard]] float fitSlopeErr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitSlopeErr, row));
    }
    /// fitInterc: line fit intercept
    [[nodiscard]] float fitInterc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitInterc, row));
    }
    /// fitIntercErr: error on the intercept
    [[nodiscard]] float fitIntercErr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitIntercErr, row));
    }
    /// size: cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_size, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// superlayer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> superlayer_col() const noexcept {
        return m_view.column<int8_t>(h_superlayer);
    }
    /// Hit1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit1_ID);
    }
    /// Hit2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit2_ID);
    }
    /// Hit3_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit3_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit3_ID);
    }
    /// Hit4_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit4_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit4_ID);
    }
    /// Hit5_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit5_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit5_ID);
    }
    /// Hit6_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit6_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit6_ID);
    }
    /// Hit7_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit7_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit7_ID);
    }
    /// Hit8_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit8_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit8_ID);
    }
    /// Hit9_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit9_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit9_ID);
    }
    /// Hit10_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit10_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit10_ID);
    }
    /// Hit11_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit11_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit11_ID);
    }
    /// Hit12_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit12_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit12_ID);
    }
    /// avgWire column as a contiguous span.
    [[nodiscard]] std::span<const float> avgWire_col() const noexcept {
        return m_view.column<float>(h_avgWire);
    }
    /// fitChisqProb column as a contiguous span.
    [[nodiscard]] std::span<const float> fitChisqProb_col() const noexcept {
        return m_view.column<float>(h_fitChisqProb);
    }
    /// fitSlope column as a contiguous span.
    [[nodiscard]] std::span<const float> fitSlope_col() const noexcept {
        return m_view.column<float>(h_fitSlope);
    }
    /// fitSlopeErr column as a contiguous span.
    [[nodiscard]] std::span<const float> fitSlopeErr_col() const noexcept {
        return m_view.column<float>(h_fitSlopeErr);
    }
    /// fitInterc column as a contiguous span.
    [[nodiscard]] std::span<const float> fitInterc_col() const noexcept {
        return m_view.column<float>(h_fitInterc);
    }
    /// fitIntercErr column as a contiguous span.
    [[nodiscard]] std::span<const float> fitIntercErr_col() const noexcept {
        return m_view.column<float>(h_fitIntercErr);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> size_col() const noexcept {
        return m_view.column<int8_t>(h_size);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_superlayer{};
    hipo::column_handle h_Hit1_ID{};
    hipo::column_handle h_Hit2_ID{};
    hipo::column_handle h_Hit3_ID{};
    hipo::column_handle h_Hit4_ID{};
    hipo::column_handle h_Hit5_ID{};
    hipo::column_handle h_Hit6_ID{};
    hipo::column_handle h_Hit7_ID{};
    hipo::column_handle h_Hit8_ID{};
    hipo::column_handle h_Hit9_ID{};
    hipo::column_handle h_Hit10_ID{};
    hipo::column_handle h_Hit11_ID{};
    hipo::column_handle h_Hit12_ID{};
    hipo::column_handle h_avgWire{};
    hipo::column_handle h_fitChisqProb{};
    hipo::column_handle h_fitSlope{};
    hipo::column_handle h_fitSlopeErr{};
    hipo::column_handle h_fitInterc{};
    hipo::column_handle h_fitIntercErr{};
    hipo::column_handle h_size{};

    friend class Row;
};

/// Lightweight row view for HitBasedTrkg::AIClusters. Stores a pointer + row index only.
class HitBasedTrkg_AIClusters::Row {
 public:
    Row(const HitBasedTrkg_AIClusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int Hit1_ID() const noexcept { return m_bank->Hit1_ID(m_row); }
    [[nodiscard]] int Hit2_ID() const noexcept { return m_bank->Hit2_ID(m_row); }
    [[nodiscard]] int Hit3_ID() const noexcept { return m_bank->Hit3_ID(m_row); }
    [[nodiscard]] int Hit4_ID() const noexcept { return m_bank->Hit4_ID(m_row); }
    [[nodiscard]] int Hit5_ID() const noexcept { return m_bank->Hit5_ID(m_row); }
    [[nodiscard]] int Hit6_ID() const noexcept { return m_bank->Hit6_ID(m_row); }
    [[nodiscard]] int Hit7_ID() const noexcept { return m_bank->Hit7_ID(m_row); }
    [[nodiscard]] int Hit8_ID() const noexcept { return m_bank->Hit8_ID(m_row); }
    [[nodiscard]] int Hit9_ID() const noexcept { return m_bank->Hit9_ID(m_row); }
    [[nodiscard]] int Hit10_ID() const noexcept { return m_bank->Hit10_ID(m_row); }
    [[nodiscard]] int Hit11_ID() const noexcept { return m_bank->Hit11_ID(m_row); }
    [[nodiscard]] int Hit12_ID() const noexcept { return m_bank->Hit12_ID(m_row); }
    [[nodiscard]] float avgWire() const noexcept { return m_bank->avgWire(m_row); }
    [[nodiscard]] float fitChisqProb() const noexcept { return m_bank->fitChisqProb(m_row); }
    [[nodiscard]] float fitSlope() const noexcept { return m_bank->fitSlope(m_row); }
    [[nodiscard]] float fitSlopeErr() const noexcept { return m_bank->fitSlopeErr(m_row); }
    [[nodiscard]] float fitInterc() const noexcept { return m_bank->fitInterc(m_row); }
    [[nodiscard]] float fitIntercErr() const noexcept { return m_bank->fitIntercErr(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
 private:
    const HitBasedTrkg_AIClusters* m_bank;
    int m_row;
};

inline HitBasedTrkg_AIClusters::Row HitBasedTrkg_AIClusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
