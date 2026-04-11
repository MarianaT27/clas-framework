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


/// HIPO bank wrapper for `HEL::decoder` (group 22000, item 14).
/// Helicity decoder board data
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HEL_decoder bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::decoder")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto helicity = row.helicity();
/// }
/// @endcode
class HEL_decoder {
 public:
    static constexpr std::string_view bank_name = "HEL::decoder";
    static constexpr int bank_group = 22000;
    static constexpr int bank_item  = 14;

    class Row;

    HEL_decoder() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("helicity"); h) h_helicity = *h;
        else h_helicity = {};
        if (auto h = schema.get_handle("pair"); h) h_pair = *h;
        else h_pair = {};
        if (auto h = schema.get_handle("pattern"); h) h_pattern = *h;
        else h_pattern = {};
        if (auto h = schema.get_handle("tSettle"); h) h_tSettle = *h;
        else h_tSettle = {};
        if (auto h = schema.get_handle("helicityPattern"); h) h_helicityPattern = *h;
        else h_helicityPattern = {};
        if (auto h = schema.get_handle("polarity"); h) h_polarity = *h;
        else h_polarity = {};
        if (auto h = schema.get_handle("phase"); h) h_phase = *h;
        else h_phase = {};
        if (auto h = schema.get_handle("timestamp"); h) h_timestamp = *h;
        else h_timestamp = {};
        if (auto h = schema.get_handle("helicitySeed"); h) h_helicitySeed = *h;
        else h_helicitySeed = {};
        if (auto h = schema.get_handle("nTStableRE"); h) h_nTStableRE = *h;
        else h_nTStableRE = {};
        if (auto h = schema.get_handle("nTStableFE"); h) h_nTStableFE = *h;
        else h_nTStableFE = {};
        if (auto h = schema.get_handle("nPattern"); h) h_nPattern = *h;
        else h_nPattern = {};
        if (auto h = schema.get_handle("nPair"); h) h_nPair = *h;
        else h_nPair = {};
        if (auto h = schema.get_handle("tStableStart"); h) h_tStableStart = *h;
        else h_tStableStart = {};
        if (auto h = schema.get_handle("tStableEnd"); h) h_tStableEnd = *h;
        else h_tStableEnd = {};
        if (auto h = schema.get_handle("tStableTime"); h) h_tStableTime = *h;
        else h_tStableTime = {};
        if (auto h = schema.get_handle("tSettleTime"); h) h_tSettleTime = *h;
        else h_tSettleTime = {};
        if (auto h = schema.get_handle("patternArray"); h) h_patternArray = *h;
        else h_patternArray = {};
        if (auto h = schema.get_handle("pairArray"); h) h_pairArray = *h;
        else h_pairArray = {};
        if (auto h = schema.get_handle("helicityArray"); h) h_helicityArray = *h;
        else h_helicityArray = {};
        if (auto h = schema.get_handle("helicityPArray"); h) h_helicityPArray = *h;
        else h_helicityPArray = {};
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
    /// helicity: HELICITY
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicity, row));
    }
    /// pair: PAIR_SYNC
    [[nodiscard]] int pair(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pair, row));
    }
    /// pattern: PATTERN_SYNC
    [[nodiscard]] int pattern(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pattern, row));
    }
    /// tSettle: T_SETTLE
    [[nodiscard]] int tSettle(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_tSettle, row));
    }
    /// helicityPattern: HELICITY at pattern start
    [[nodiscard]] int helicityPattern(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicityPattern, row));
    }
    /// polarity: event polarity (XOR of 3,4)
    [[nodiscard]] int polarity(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_polarity, row));
    }
    /// phase: pattern phase count
    [[nodiscard]] int phase(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_phase, row));
    }
    /// timestamp: timestamp (4ns)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_timestamp, row));
    }
    /// helicitySeed: helicity seed
    [[nodiscard]] int helicitySeed(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_helicitySeed, row));
    }
    /// nTStableRE: count of T_STABLE rising edge
    [[nodiscard]] int nTStableRE(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_nTStableRE, row));
    }
    /// nTStableFE: count of T_STABLE falling edge
    [[nodiscard]] int nTStableFE(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_nTStableFE, row));
    }
    /// nPattern: count of PATTERN_SYNC
    [[nodiscard]] int nPattern(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_nPattern, row));
    }
    /// nPair: count of PAIR_SYNC
    [[nodiscard]] int nPair(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_nPair, row));
    }
    /// tStableStart: time of trigger since start of T_STABLE (8 ns
    [[nodiscard]] int tStableStart(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tStableStart, row));
    }
    /// tStableEnd: time of trigger since end of T_STABLE (8 ns
    [[nodiscard]] int tStableEnd(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tStableEnd, row));
    }
    /// tStableTime: time duration of last complete T_STABLE period (8 ns
    [[nodiscard]] int tStableTime(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tStableTime, row));
    }
    /// tSettleTime: time duration of last complete T_SETTLE period (8 ns
    [[nodiscard]] int tSettleTime(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tSettleTime, row));
    }
    /// patternArray: last 32 windows of PATTERN_SYNC
    [[nodiscard]] int patternArray(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_patternArray, row));
    }
    /// pairArray: last 32 windows of PAIR_SYNC
    [[nodiscard]] int pairArray(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pairArray, row));
    }
    /// helicityArray: last 32 windows of HELICITY_STATE
    [[nodiscard]] int helicityArray(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_helicityArray, row));
    }
    /// helicityPArray: last 32 windows of HELICITY_STATE at start of pattern
    [[nodiscard]] int helicityPArray(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_helicityPArray, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// helicity column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicity_col() const noexcept {
        return m_view.column<int8_t>(h_helicity);
    }
    /// pair column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> pair_col() const noexcept {
        return m_view.column<int8_t>(h_pair);
    }
    /// pattern column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> pattern_col() const noexcept {
        return m_view.column<int8_t>(h_pattern);
    }
    /// tSettle column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> tSettle_col() const noexcept {
        return m_view.column<int8_t>(h_tSettle);
    }
    /// helicityPattern column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicityPattern_col() const noexcept {
        return m_view.column<int8_t>(h_helicityPattern);
    }
    /// polarity column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> polarity_col() const noexcept {
        return m_view.column<int8_t>(h_polarity);
    }
    /// phase column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> phase_col() const noexcept {
        return m_view.column<int8_t>(h_phase);
    }
    /// timestamp column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> timestamp_col() const noexcept {
        return m_view.column<int64_t>(h_timestamp);
    }
    /// helicitySeed column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> helicitySeed_col() const noexcept {
        return m_view.column<int32_t>(h_helicitySeed);
    }
    /// nTStableRE column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> nTStableRE_col() const noexcept {
        return m_view.column<int32_t>(h_nTStableRE);
    }
    /// nTStableFE column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> nTStableFE_col() const noexcept {
        return m_view.column<int32_t>(h_nTStableFE);
    }
    /// nPattern column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> nPattern_col() const noexcept {
        return m_view.column<int32_t>(h_nPattern);
    }
    /// nPair column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> nPair_col() const noexcept {
        return m_view.column<int32_t>(h_nPair);
    }
    /// tStableStart column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tStableStart_col() const noexcept {
        return m_view.column<int32_t>(h_tStableStart);
    }
    /// tStableEnd column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tStableEnd_col() const noexcept {
        return m_view.column<int32_t>(h_tStableEnd);
    }
    /// tStableTime column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tStableTime_col() const noexcept {
        return m_view.column<int32_t>(h_tStableTime);
    }
    /// tSettleTime column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tSettleTime_col() const noexcept {
        return m_view.column<int32_t>(h_tSettleTime);
    }
    /// patternArray column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> patternArray_col() const noexcept {
        return m_view.column<int32_t>(h_patternArray);
    }
    /// pairArray column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pairArray_col() const noexcept {
        return m_view.column<int32_t>(h_pairArray);
    }
    /// helicityArray column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> helicityArray_col() const noexcept {
        return m_view.column<int32_t>(h_helicityArray);
    }
    /// helicityPArray column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> helicityPArray_col() const noexcept {
        return m_view.column<int32_t>(h_helicityPArray);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_helicity{};
    hipo::column_handle h_pair{};
    hipo::column_handle h_pattern{};
    hipo::column_handle h_tSettle{};
    hipo::column_handle h_helicityPattern{};
    hipo::column_handle h_polarity{};
    hipo::column_handle h_phase{};
    hipo::column_handle h_timestamp{};
    hipo::column_handle h_helicitySeed{};
    hipo::column_handle h_nTStableRE{};
    hipo::column_handle h_nTStableFE{};
    hipo::column_handle h_nPattern{};
    hipo::column_handle h_nPair{};
    hipo::column_handle h_tStableStart{};
    hipo::column_handle h_tStableEnd{};
    hipo::column_handle h_tStableTime{};
    hipo::column_handle h_tSettleTime{};
    hipo::column_handle h_patternArray{};
    hipo::column_handle h_pairArray{};
    hipo::column_handle h_helicityArray{};
    hipo::column_handle h_helicityPArray{};

    friend class Row;
};

/// Lightweight row view for HEL::decoder. Stores a pointer + row index only.
class HEL_decoder::Row {
 public:
    Row(const HEL_decoder* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int pair() const noexcept { return m_bank->pair(m_row); }
    [[nodiscard]] int pattern() const noexcept { return m_bank->pattern(m_row); }
    [[nodiscard]] int tSettle() const noexcept { return m_bank->tSettle(m_row); }
    [[nodiscard]] int helicityPattern() const noexcept { return m_bank->helicityPattern(m_row); }
    [[nodiscard]] int polarity() const noexcept { return m_bank->polarity(m_row); }
    [[nodiscard]] int phase() const noexcept { return m_bank->phase(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
    [[nodiscard]] int helicitySeed() const noexcept { return m_bank->helicitySeed(m_row); }
    [[nodiscard]] int nTStableRE() const noexcept { return m_bank->nTStableRE(m_row); }
    [[nodiscard]] int nTStableFE() const noexcept { return m_bank->nTStableFE(m_row); }
    [[nodiscard]] int nPattern() const noexcept { return m_bank->nPattern(m_row); }
    [[nodiscard]] int nPair() const noexcept { return m_bank->nPair(m_row); }
    [[nodiscard]] int tStableStart() const noexcept { return m_bank->tStableStart(m_row); }
    [[nodiscard]] int tStableEnd() const noexcept { return m_bank->tStableEnd(m_row); }
    [[nodiscard]] int tStableTime() const noexcept { return m_bank->tStableTime(m_row); }
    [[nodiscard]] int tSettleTime() const noexcept { return m_bank->tSettleTime(m_row); }
    [[nodiscard]] int patternArray() const noexcept { return m_bank->patternArray(m_row); }
    [[nodiscard]] int pairArray() const noexcept { return m_bank->pairArray(m_row); }
    [[nodiscard]] int helicityArray() const noexcept { return m_bank->helicityArray(m_row); }
    [[nodiscard]] int helicityPArray() const noexcept { return m_bank->helicityPArray(m_row); }
 private:
    const HEL_decoder* m_bank;
    int m_row;
};

inline HEL_decoder::Row HEL_decoder::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
