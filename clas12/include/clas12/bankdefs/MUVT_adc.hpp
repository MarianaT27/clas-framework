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


/// HIPO bank wrapper for `MUVT::adc` (group 22900, item 11).
/// ADC bank for the muCLAS12 Vertex Tracker
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MUVT_adc bank;
/// if (auto sc = bank.bind(event.get_bank("MUVT::adc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class MUVT_adc {
 public:
    static constexpr std::string_view bank_name = "MUVT::adc";
    static constexpr int bank_group = 22900;
    static constexpr int bank_item  = 11;

    class Row;

    MUVT_adc() = default;

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
        }
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_order = cache.h_order;
        h_ADC = cache.h_ADC;
        h_time = cache.h_time;
        h_ped = cache.h_ped;
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
    /// sector: sector (=1)
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer (=1)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: crystal id
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
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
    [[nodiscard]] std::span<const int16_t> ped_col() const noexcept {
        return m_view.column<int16_t>(h_ped);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_order{};
    hipo::column_handle h_ADC{};
    hipo::column_handle h_time{};
    hipo::column_handle h_ped{};

    friend class Row;
};

/// Lightweight row view for MUVT::adc. Stores a pointer + row index only.
class MUVT_adc::Row {
 public:
    Row(const MUVT_adc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int order() const noexcept { return m_bank->order(m_row); }
    [[nodiscard]] int ADC() const noexcept { return m_bank->ADC(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int ped() const noexcept { return m_bank->ped(m_row); }
 private:
    const MUVT_adc* m_bank;
    int m_row;
};

inline MUVT_adc::Row MUVT_adc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
