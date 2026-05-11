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


/// HIPO bank wrapper for `RAW::tdc` (group 20000, item 12).
/// TDC bank for untranslated signals
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
            int i_crate = -1;
            int i_slot = -1;
            int i_channel = -1;
            int i_order = -1;
            int i_TDC = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_crate = schema.exists("crate") ? schema.getEntryOrder("crate") : -1;
            cache.i_slot = schema.exists("slot") ? schema.getEntryOrder("slot") : -1;
            cache.i_channel = schema.exists("channel") ? schema.getEntryOrder("channel") : -1;
            cache.i_order = schema.exists("order") ? schema.getEntryOrder("order") : -1;
            cache.i_TDC = schema.exists("TDC") ? schema.getEntryOrder("TDC") : -1;
        }
        i_crate = cache.i_crate;
        i_slot = cache.i_slot;
        i_channel = cache.i_channel;
        i_order = cache.i_order;
        i_TDC = cache.i_TDC;
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
    /// order: order: 2 - TDCL , 3 - TDCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_order, row));
    }
    /// TDC: TDC value
    [[nodiscard]] int TDC(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_TDC, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_crate = -1;
    int i_slot = -1;
    int i_channel = -1;
    int i_order = -1;
    int i_TDC = -1;

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

}  // namespace bankdefs
