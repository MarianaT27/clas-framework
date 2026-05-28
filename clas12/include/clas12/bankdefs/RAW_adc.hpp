// =============================================================================
// Auto-generated from data.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RAW::adc` (group 20000, item 11).
/// ADC bank for untranslated signals
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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

    /// Bind to a live hipo::bank. Per-thread cache keyed by the schema's
    /// (group, item): the first bind() in a thread resolves every column
    /// name to its item index via schema.getEntryOrder(); subsequent
    /// bind()s hit the cache. Keying by (group,item) rather than the schema
    /// POINTER is critical: the framework constructs a fresh hipo::bank
    /// (copying its schema to a new address) every event, so a pointer key
    /// would miss every event and re-resolve all columns by string. (group,
    /// item) is stable across those copies, so resolution is paid once per
    /// thread per bank type. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            int gi_group = -1;
            int gi_item  = -2;
            int i_crate = -1;
            int i_slot = -1;
            int i_channel = -1;
            int i_order = -1;
            int i_ADC = -1;
            int i_time = -1;
            int i_ped = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_crate = schema.exists("crate") ? schema.getEntryOrder("crate") : -1;
            cache.i_slot = schema.exists("slot") ? schema.getEntryOrder("slot") : -1;
            cache.i_channel = schema.exists("channel") ? schema.getEntryOrder("channel") : -1;
            cache.i_order = schema.exists("order") ? schema.getEntryOrder("order") : -1;
            cache.i_ADC = schema.exists("ADC") ? schema.getEntryOrder("ADC") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_ped = schema.exists("ped") ? schema.getEntryOrder("ped") : -1;
        }
        i_crate = cache.i_crate;
        i_slot = cache.i_slot;
        i_channel = cache.i_channel;
        i_order = cache.i_order;
        i_ADC = cache.i_ADC;
        i_time = cache.i_time;
        i_ped = cache.i_ped;
        return true;
    }

    /// Current row count of the bound bank (0 if unbound).
    [[nodiscard]] int rows() const noexcept { return m_view ? m_view->getRows() : 0; }

    /// True if bind() has been called.
    [[nodiscard]] bool valid() const noexcept { return m_view != nullptr; }

    /// Access the underlying hipo::bank (for advanced use).
    [[nodiscard]] const hipo::bank& view() const noexcept { return *m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (item-index based, no string lookup)
    // -------------------------------------------------------------------
    /// crate: crate id
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_crate, row));
    }
    /// slot: slot id
    [[nodiscard]] int slot(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_slot, row));
    }
    /// channel: channel id
    [[nodiscard]] int channel(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_channel, row));
    }
    /// order: order: 0 - ADCL , 1 - ADCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_order, row));
    }
    /// ADC: ADC integral from pulse fit
    [[nodiscard]] int ADC(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_ADC, row));
    }
    /// time: time from pulse fit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// ped: pedestal from pulse analysis
    [[nodiscard]] int ped(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ped, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_crate = -1;
    int i_slot = -1;
    int i_channel = -1;
    int i_order = -1;
    int i_ADC = -1;
    int i_time = -1;
    int i_ped = -1;

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

}  // namespace bankdefs
