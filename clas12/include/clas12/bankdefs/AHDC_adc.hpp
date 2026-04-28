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

namespace bankdefs {


/// HIPO bank wrapper for `AHDC::adc` (group 22400, item 11).
/// ADC bank for the ALERT Wire Chamber
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    /// Bind to a live bank_view. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column handle by name; all
    /// subsequent bind()s for the same schema hit the cache and copy 25-30
    /// trivially-copyable column_handle words.
    /// Returns false if the view is not valid.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        struct HandleCache {
            const hipo::schema* schema_ptr = nullptr;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_component;
            hipo::column_handle h_order;
            hipo::column_handle h_ADC;
            hipo::column_handle h_time;
            hipo::column_handle h_ped;
            hipo::column_handle h_windex;
            hipo::column_handle h_integral;
            hipo::column_handle h_leadingEdgeTime;
            hipo::column_handle h_timeOverThreshold;
            hipo::column_handle h_constantFractionTime;
            hipo::column_handle h_wfType;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("component"); h) cache.h_component = *h;
            else cache.h_component = {};
            if (auto h = schema.get_handle("order"); h) cache.h_order = *h;
            else cache.h_order = {};
            if (auto h = schema.get_handle("ADC"); h) cache.h_ADC = *h;
            else cache.h_ADC = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("ped"); h) cache.h_ped = *h;
            else cache.h_ped = {};
            if (auto h = schema.get_handle("windex"); h) cache.h_windex = *h;
            else cache.h_windex = {};
            if (auto h = schema.get_handle("integral"); h) cache.h_integral = *h;
            else cache.h_integral = {};
            if (auto h = schema.get_handle("leadingEdgeTime"); h) cache.h_leadingEdgeTime = *h;
            else cache.h_leadingEdgeTime = {};
            if (auto h = schema.get_handle("timeOverThreshold"); h) cache.h_timeOverThreshold = *h;
            else cache.h_timeOverThreshold = {};
            if (auto h = schema.get_handle("constantFractionTime"); h) cache.h_constantFractionTime = *h;
            else cache.h_constantFractionTime = {};
            if (auto h = schema.get_handle("wfType"); h) cache.h_wfType = *h;
            else cache.h_wfType = {};
        }
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_order = cache.h_order;
        h_ADC = cache.h_ADC;
        h_time = cache.h_time;
        h_ped = cache.h_ped;
        h_windex = cache.h_windex;
        h_integral = cache.h_integral;
        h_leadingEdgeTime = cache.h_leadingEdgeTime;
        h_timeOverThreshold = cache.h_timeOverThreshold;
        h_constantFractionTime = cache.h_constantFractionTime;
        h_wfType = cache.h_wfType;
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
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: 2 digits: first digit is superlayer, second digit is layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: wire number
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// order: Stereo angle 0 - in, 1 - out
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_order, row));
    }
    /// ADC: average of 3 points around the max of ADC
    [[nodiscard]] int ADC(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_ADC, row));
    }
    /// time: time corresponding to the max of ADC (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// ped: average ADC of the first 4 samples
    [[nodiscard]] float ped(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ped, row));
    }
    /// windex: row index in waveform bank
    [[nodiscard]] int windex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_windex, row));
    }
    /// integral: sum of ADC
    [[nodiscard]] int integral(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_integral, row));
    }
    /// leadingEdgeTime: time at half amplitude (ns)
    [[nodiscard]] float leadingEdgeTime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_leadingEdgeTime, row));
    }
    /// timeOverThreshold: time over threshold at half amplitude (ns)
    [[nodiscard]] float timeOverThreshold(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_timeOverThreshold, row));
    }
    /// constantFractionTime: time from constant fraction discriminator (ns)
    [[nodiscard]] float constantFractionTime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_constantFractionTime, row));
    }
    /// wfType: waveform type classification, good <= 2, bad=others
    [[nodiscard]] int wfType(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_wfType, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
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
    [[nodiscard]] std::span<const float> ped_col() const noexcept {
        return m_view.column<float>(h_ped);
    }
    /// windex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> windex_col() const noexcept {
        return m_view.column<int16_t>(h_windex);
    }
    /// integral column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> integral_col() const noexcept {
        return m_view.column<int32_t>(h_integral);
    }
    /// leadingEdgeTime column as a contiguous span.
    [[nodiscard]] std::span<const float> leadingEdgeTime_col() const noexcept {
        return m_view.column<float>(h_leadingEdgeTime);
    }
    /// timeOverThreshold column as a contiguous span.
    [[nodiscard]] std::span<const float> timeOverThreshold_col() const noexcept {
        return m_view.column<float>(h_timeOverThreshold);
    }
    /// constantFractionTime column as a contiguous span.
    [[nodiscard]] std::span<const float> constantFractionTime_col() const noexcept {
        return m_view.column<float>(h_constantFractionTime);
    }
    /// wfType column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> wfType_col() const noexcept {
        return m_view.column<int16_t>(h_wfType);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle windex_handle() const noexcept { return h_windex; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_order{};
    hipo::column_handle h_ADC{};
    hipo::column_handle h_time{};
    hipo::column_handle h_ped{};
    hipo::column_handle h_windex{};
    hipo::column_handle h_integral{};
    hipo::column_handle h_leadingEdgeTime{};
    hipo::column_handle h_timeOverThreshold{};
    hipo::column_handle h_constantFractionTime{};
    hipo::column_handle h_wfType{};

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
