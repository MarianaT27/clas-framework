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


/// HIPO bank wrapper for `RAW::tdc` (group 20000, item 12).
/// TDC bank for untranslated signals
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RAW_tdc bank;
/// if (auto sc = bank.bind(event.get_bank("RAW::tdc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class RAW_tdc {
 public:
    static constexpr std::string_view bank_name = "RAW::tdc";
    static constexpr int bank_group = 20000;
    static constexpr int bank_item  = 12;

    class Row;

    RAW_tdc() = default;

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
            hipo::column_handle h_crate;
            hipo::column_handle h_slot;
            hipo::column_handle h_channel;
            hipo::column_handle h_order;
            hipo::column_handle h_TDC;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("crate"); h) cache.h_crate = *h;
            else cache.h_crate = {};
            if (auto h = schema.get_handle("slot"); h) cache.h_slot = *h;
            else cache.h_slot = {};
            if (auto h = schema.get_handle("channel"); h) cache.h_channel = *h;
            else cache.h_channel = {};
            if (auto h = schema.get_handle("order"); h) cache.h_order = *h;
            else cache.h_order = {};
            if (auto h = schema.get_handle("TDC"); h) cache.h_TDC = *h;
            else cache.h_TDC = {};
        }
        h_crate = cache.h_crate;
        h_slot = cache.h_slot;
        h_channel = cache.h_channel;
        h_order = cache.h_order;
        h_TDC = cache.h_TDC;
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
    /// order: order: 2 - TDCL , 3 - TDCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_order, row));
    }
    /// TDC: TDC value
    [[nodiscard]] int TDC(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_TDC, row));
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
    /// TDC column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> TDC_col() const noexcept {
        return m_view.column<int32_t>(h_TDC);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_crate{};
    hipo::column_handle h_slot{};
    hipo::column_handle h_channel{};
    hipo::column_handle h_order{};
    hipo::column_handle h_TDC{};

    friend class Row;
};

/// Lightweight row view for RAW::tdc. Stores a pointer + row index only.
class RAW_tdc::Row {
 public:
    Row(const RAW_tdc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int slot() const noexcept { return m_bank->slot(m_row); }
    [[nodiscard]] int channel() const noexcept { return m_bank->channel(m_row); }
    [[nodiscard]] int order() const noexcept { return m_bank->order(m_row); }
    [[nodiscard]] int TDC() const noexcept { return m_bank->TDC(m_row); }
 private:
    const RAW_tdc* m_bank;
    int m_row;
};

inline RAW_tdc::Row RAW_tdc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
