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


/// HIPO bank wrapper for `BAND::adc` (group 22100, item 11).
/// ADC bank for the Backward Angle Neutron Detector
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// BAND_adc bank;
/// if (auto sc = bank.bind(event.get_bank("BAND::adc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class BAND_adc {
 public:
    static constexpr std::string_view bank_name = "BAND::adc";
    static constexpr int bank_group = 22100;
    static constexpr int bank_item  = 11;

    class Row;

    BAND_adc() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_order = -1;
            int i_ADC = -1;
            int i_amplitude = -1;
            int i_time = -1;
            int i_ped = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_order = schema.exists("order") ? schema.getEntryOrder("order") : -1;
            cache.i_ADC = schema.exists("ADC") ? schema.getEntryOrder("ADC") : -1;
            cache.i_amplitude = schema.exists("amplitude") ? schema.getEntryOrder("amplitude") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_ped = schema.exists("ped") ? schema.getEntryOrder("ped") : -1;
        }
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_order = cache.i_order;
        i_ADC = cache.i_ADC;
        i_amplitude = cache.i_amplitude;
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
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer (1-6)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: strips
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// order: order: 0 - ADCL , 1 - ADCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_order, row));
    }
    /// ADC: ADC integral from pulse fit
    [[nodiscard]] int ADC(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_ADC, row));
    }
    /// amplitude: ADC maximum
    [[nodiscard]] int amplitude(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_amplitude, row));
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
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_order = -1;
    int i_ADC = -1;
    int i_amplitude = -1;
    int i_time = -1;
    int i_ped = -1;

    friend class Row;
};

/// Lightweight row view for BAND::adc. Stores a pointer + row index only.
class BAND_adc::Row {
 public:
    Row(const BAND_adc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int order() const noexcept { return m_bank->order(m_row); }
    [[nodiscard]] int ADC() const noexcept { return m_bank->ADC(m_row); }
    [[nodiscard]] int amplitude() const noexcept { return m_bank->amplitude(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int ped() const noexcept { return m_bank->ped(m_row); }
 private:
    const BAND_adc* m_bank;
    int m_row;
};

inline BAND_adc::Row BAND_adc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
