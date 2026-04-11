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


/// HIPO bank wrapper for `HEL::flip` (group 22000, item 12).
/// Helicity flips from ADC bank.  State values are -1/0/+1, where 0 is invalid.
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HEL_flip bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::flip")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto run = row.run();
/// }
/// @endcode
class HEL_flip {
 public:
    static constexpr std::string_view bank_name = "HEL::flip";
    static constexpr int bank_group = 22000;
    static constexpr int bank_item  = 12;

    class Row;

    HEL_flip() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("run"); h) h_run = *h;
        else h_run = {};
        if (auto h = schema.get_handle("event"); h) h_event = *h;
        else h_event = {};
        if (auto h = schema.get_handle("timestamp"); h) h_timestamp = *h;
        else h_timestamp = {};
        if (auto h = schema.get_handle("helicity"); h) h_helicity = *h;
        else h_helicity = {};
        if (auto h = schema.get_handle("helicityRaw"); h) h_helicityRaw = *h;
        else h_helicityRaw = {};
        if (auto h = schema.get_handle("pair"); h) h_pair = *h;
        else h_pair = {};
        if (auto h = schema.get_handle("pattern"); h) h_pattern = *h;
        else h_pattern = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
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
    /// run: run number
    [[nodiscard]] int run(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_run, row));
    }
    /// event: event number
    [[nodiscard]] int event(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_event, row));
    }
    /// timestamp: timestamp (4ns)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_timestamp, row));
    }
    /// helicity: helicity state with half-wave plate correction
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicity, row));
    }
    /// helicityRaw: helicity state without half-wave plate correction
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicityRaw, row));
    }
    /// pair: pair sync state
    [[nodiscard]] int pair(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pair, row));
    }
    /// pattern: pattern sync state
    [[nodiscard]] int pattern(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pattern, row));
    }
    /// status: status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// run column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> run_col() const noexcept {
        return m_view.column<int32_t>(h_run);
    }
    /// event column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> event_col() const noexcept {
        return m_view.column<int32_t>(h_event);
    }
    /// timestamp column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> timestamp_col() const noexcept {
        return m_view.column<int64_t>(h_timestamp);
    }
    /// helicity column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicity_col() const noexcept {
        return m_view.column<int8_t>(h_helicity);
    }
    /// helicityRaw column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicityRaw_col() const noexcept {
        return m_view.column<int8_t>(h_helicityRaw);
    }
    /// pair column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> pair_col() const noexcept {
        return m_view.column<int8_t>(h_pair);
    }
    /// pattern column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> pattern_col() const noexcept {
        return m_view.column<int8_t>(h_pattern);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> status_col() const noexcept {
        return m_view.column<int8_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_run{};
    hipo::column_handle h_event{};
    hipo::column_handle h_timestamp{};
    hipo::column_handle h_helicity{};
    hipo::column_handle h_helicityRaw{};
    hipo::column_handle h_pair{};
    hipo::column_handle h_pattern{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for HEL::flip. Stores a pointer + row index only.
class HEL_flip::Row {
 public:
    Row(const HEL_flip* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int run() const noexcept { return m_bank->run(m_row); }
    [[nodiscard]] int event() const noexcept { return m_bank->event(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int helicityRaw() const noexcept { return m_bank->helicityRaw(m_row); }
    [[nodiscard]] int pair() const noexcept { return m_bank->pair(m_row); }
    [[nodiscard]] int pattern() const noexcept { return m_bank->pattern(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const HEL_flip* m_bank;
    int m_row;
};

inline HEL_flip::Row HEL_flip::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
