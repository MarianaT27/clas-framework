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


/// HIPO bank wrapper for `ATOF::tdc` (group 22500, item 12).
/// TDC bank for the ALERT TOF
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ATOF_tdc bank;
/// if (auto sc = bank.bind(event.get_bank("ATOF::tdc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class ATOF_tdc {
 public:
    static constexpr std::string_view bank_name = "ATOF::tdc";
    static constexpr int bank_group = 22500;
    static constexpr int bank_item  = 12;

    class Row;

    ATOF_tdc() = default;

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
            hipo::column_handle h_TDC;
            hipo::column_handle h_ToT;
            hipo::column_handle h_timestamp;
            hipo::column_handle h_trigger;
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
            if (auto h = schema.get_handle("TDC"); h) cache.h_TDC = *h;
            else cache.h_TDC = {};
            if (auto h = schema.get_handle("ToT"); h) cache.h_ToT = *h;
            else cache.h_ToT = {};
            if (auto h = schema.get_handle("timestamp"); h) cache.h_timestamp = *h;
            else cache.h_timestamp = {};
            if (auto h = schema.get_handle("trigger"); h) cache.h_trigger = *h;
            else cache.h_trigger = {};
        }
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_order = cache.h_order;
        h_TDC = cache.h_TDC;
        h_ToT = cache.h_ToT;
        h_timestamp = cache.h_timestamp;
        h_trigger = cache.h_trigger;
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
    /// component: signal (1-2)
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// order: order: 2 - TDCL , 3 - TDCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_order, row));
    }
    /// TDC: TDC value
    [[nodiscard]] int TDC(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_TDC, row));
    }
    /// ToT: Time Over Threshold
    [[nodiscard]] int ToT(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_ToT, row));
    }
    /// timestamp: timestamp
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_timestamp, row));
    }
    /// trigger: trigger number
    [[nodiscard]] int trigger(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trigger, row));
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
    /// TDC column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> TDC_col() const noexcept {
        return m_view.column<int32_t>(h_TDC);
    }
    /// ToT column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> ToT_col() const noexcept {
        return m_view.column<int32_t>(h_ToT);
    }
    /// timestamp column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> timestamp_col() const noexcept {
        return m_view.column<int64_t>(h_timestamp);
    }
    /// trigger column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trigger_col() const noexcept {
        return m_view.column<int32_t>(h_trigger);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_order{};
    hipo::column_handle h_TDC{};
    hipo::column_handle h_ToT{};
    hipo::column_handle h_timestamp{};
    hipo::column_handle h_trigger{};

    friend class Row;
};

/// Lightweight row view for ATOF::tdc. Stores a pointer + row index only.
class ATOF_tdc::Row {
 public:
    Row(const ATOF_tdc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int order() const noexcept { return m_bank->order(m_row); }
    [[nodiscard]] int TDC() const noexcept { return m_bank->TDC(m_row); }
    [[nodiscard]] int ToT() const noexcept { return m_bank->ToT(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
    [[nodiscard]] int trigger() const noexcept { return m_bank->trigger(m_row); }
 private:
    const ATOF_tdc* m_bank;
    int m_row;
};

inline ATOF_tdc::Row ATOF_tdc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
