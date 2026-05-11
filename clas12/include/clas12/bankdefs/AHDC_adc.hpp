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


/// HIPO bank wrapper for `AHDC::adc` (group 22400, item 11).
/// ADC bank for the ALERT Wire Chamber
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// AHDC_adc bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::adc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class AHDC_adc {
 public:
    static constexpr std::string_view bank_name = "AHDC::adc";
    static constexpr int bank_group = 22400;
    static constexpr int bank_item  = 11;

    class Row;

    AHDC_adc() = default;

    /// Bind to a live hipo::bank. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column name to its item
    /// index via schema.getEntryOrder(); subsequent bind()s for the same
    /// schema hit the cache. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            const hipo::schema* schema_ptr = nullptr;
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_order = -1;
            int i_ADC = -1;
            int i_time = -1;
            int i_ped = -1;
            int i_windex = -1;
            int i_integral = -1;
            int i_leadingEdgeTime = -1;
            int i_timeOverThreshold = -1;
            int i_constantFractionTime = -1;
            int i_wfType = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_order = schema.exists("order") ? schema.getEntryOrder("order") : -1;
            cache.i_ADC = schema.exists("ADC") ? schema.getEntryOrder("ADC") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_ped = schema.exists("ped") ? schema.getEntryOrder("ped") : -1;
            cache.i_windex = schema.exists("windex") ? schema.getEntryOrder("windex") : -1;
            cache.i_integral = schema.exists("integral") ? schema.getEntryOrder("integral") : -1;
            cache.i_leadingEdgeTime = schema.exists("leadingEdgeTime") ? schema.getEntryOrder("leadingEdgeTime") : -1;
            cache.i_timeOverThreshold = schema.exists("timeOverThreshold") ? schema.getEntryOrder("timeOverThreshold") : -1;
            cache.i_constantFractionTime = schema.exists("constantFractionTime") ? schema.getEntryOrder("constantFractionTime") : -1;
            cache.i_wfType = schema.exists("wfType") ? schema.getEntryOrder("wfType") : -1;
        }
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_order = cache.i_order;
        i_ADC = cache.i_ADC;
        i_time = cache.i_time;
        i_ped = cache.i_ped;
        i_windex = cache.i_windex;
        i_integral = cache.i_integral;
        i_leadingEdgeTime = cache.i_leadingEdgeTime;
        i_timeOverThreshold = cache.i_timeOverThreshold;
        i_constantFractionTime = cache.i_constantFractionTime;
        i_wfType = cache.i_wfType;
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
    /// layer: 2 digits: first digit is superlayer, second digit is layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: wire number
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// order: Stereo angle 0 - in, 1 - out
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_order, row));
    }
    /// ADC: average of 3 points around the max of ADC
    [[nodiscard]] int ADC(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_ADC, row));
    }
    /// time: time corresponding to the max of ADC (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// ped: average ADC of the first 4 samples
    [[nodiscard]] float ped(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ped, row));
    }
    /// windex: row index in waveform bank
    [[nodiscard]] int windex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_windex, row));
    }
    /// integral: sum of ADC
    [[nodiscard]] int integral(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_integral, row));
    }
    /// leadingEdgeTime: time at half amplitude (ns)
    [[nodiscard]] float leadingEdgeTime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_leadingEdgeTime, row));
    }
    /// timeOverThreshold: time over threshold at half amplitude (ns)
    [[nodiscard]] float timeOverThreshold(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_timeOverThreshold, row));
    }
    /// constantFractionTime: time from constant fraction discriminator (ns)
    [[nodiscard]] float constantFractionTime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_constantFractionTime, row));
    }
    /// wfType: waveform type classification, good <= 2, bad=others
    [[nodiscard]] int wfType(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_wfType, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int windex_handle() const noexcept { return i_windex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_order = -1;
    int i_ADC = -1;
    int i_time = -1;
    int i_ped = -1;
    int i_windex = -1;
    int i_integral = -1;
    int i_leadingEdgeTime = -1;
    int i_timeOverThreshold = -1;
    int i_constantFractionTime = -1;
    int i_wfType = -1;

    friend class Row;
};

/// Lightweight row view for AHDC::adc. Stores a pointer + row index only.
class AHDC_adc::Row {
 public:
    Row(const AHDC_adc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int order() const noexcept { return m_bank->order(m_row); }
    [[nodiscard]] int ADC() const noexcept { return m_bank->ADC(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float ped() const noexcept { return m_bank->ped(m_row); }
    [[nodiscard]] int windex() const noexcept { return m_bank->windex(m_row); }
    [[nodiscard]] int integral() const noexcept { return m_bank->integral(m_row); }
    [[nodiscard]] float leadingEdgeTime() const noexcept { return m_bank->leadingEdgeTime(m_row); }
    [[nodiscard]] float timeOverThreshold() const noexcept { return m_bank->timeOverThreshold(m_row); }
    [[nodiscard]] float constantFractionTime() const noexcept { return m_bank->constantFractionTime(m_row); }
    [[nodiscard]] int wfType() const noexcept { return m_bank->wfType(m_row); }
 private:
    const AHDC_adc* m_bank;
    int m_row;
};

inline AHDC_adc::Row AHDC_adc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
