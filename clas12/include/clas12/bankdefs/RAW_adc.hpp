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


/// HIPO bank wrapper for `RAW::adc` (group 20000, item 11).
/// ADC bank for untranslated signals
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RAW_adc bank;
/// if (auto sc = bank.bind(event.get_bank("RAW::adc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class RAW_adc {
 public:
    static constexpr std::string_view bank_name = "RAW::adc";
    static constexpr int bank_group = 20000;
    static constexpr int bank_item  = 11;

    class Row;

    RAW_adc() = default;

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
        if (auto h = schema.get_handle("channel"); h) h_channel = *h;
        else h_channel = {};
        if (auto h = schema.get_handle("order"); h) h_order = *h;
        else h_order = {};
        if (auto h = schema.get_handle("ADC"); h) h_ADC = *h;
        else h_ADC = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("ped"); h) h_ped = *h;
        else h_ped = {};
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
    /// crate: crate id
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_crate, row));
    }
    /// slot: slot id
    [[nodiscard]] int slot(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_slot, row));
    }
    /// channel: channel id
    [[nodiscard]] int channel(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_channel, row));
    }
    /// order: order: 0 - ADCL , 1 - ADCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_order, row));
    }
    /// ADC: ADC integral from pulse fit
    [[nodiscard]] int ADC(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_ADC, row));
    }
    /// time: time from pulse fit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// ped: pedestal from pulse analysis
    [[nodiscard]] int ped(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ped, row));
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
    /// channel column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> channel_col() const noexcept {
        return m_view.column<int16_t>(h_channel);
    }
    /// order column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> order_col() const noexcept {
        return m_view.column<int8_t>(h_order);
    }
    /// ADC column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> ADC_col() const noexcept {
        return m_view.column<int32_t>(h_ADC);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// ped column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ped_col() const noexcept {
        return m_view.column<int16_t>(h_ped);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_crate{};
    hipo::column_handle h_slot{};
    hipo::column_handle h_channel{};
    hipo::column_handle h_order{};
    hipo::column_handle h_ADC{};
    hipo::column_handle h_time{};
    hipo::column_handle h_ped{};

    friend class Row;
};

/// Lightweight row view for RAW::adc. Stores a pointer + row index only.
class RAW_adc::Row {
 public:
    Row(const RAW_adc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int slot() const noexcept { return m_bank->slot(m_row); }
    [[nodiscard]] int channel() const noexcept { return m_bank->channel(m_row); }
    [[nodiscard]] int order() const noexcept { return m_bank->order(m_row); }
    [[nodiscard]] int ADC() const noexcept { return m_bank->ADC(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int ped() const noexcept { return m_bank->ped(m_row); }
 private:
    const RAW_adc* m_bank;
    int m_row;
};

inline RAW_adc::Row RAW_adc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
