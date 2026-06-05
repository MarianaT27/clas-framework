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


/// HIPO bank wrapper for `RAW::scaler` (group 20000, item 13).
/// Scaler bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RAW_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("RAW::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class RAW_scaler {
 public:
    static constexpr std::string_view bank_name = "RAW::scaler";
    static constexpr int bank_group = 20000;
    static constexpr int bank_item  = 13;
    static constexpr std::string_view bank_schema = "crate/B,slot/B,channel/S,helicity/B,quartet/B,value/L";

    class Row;

    RAW_scaler() = default;

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
            int i_helicity = -1;
            int i_quartet = -1;
            int i_value = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_crate = schema.exists("crate") ? schema.getEntryOrder("crate") : -1;
            cache.i_slot = schema.exists("slot") ? schema.getEntryOrder("slot") : -1;
            cache.i_channel = schema.exists("channel") ? schema.getEntryOrder("channel") : -1;
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
            cache.i_quartet = schema.exists("quartet") ? schema.getEntryOrder("quartet") : -1;
            cache.i_value = schema.exists("value") ? schema.getEntryOrder("value") : -1;
        }
        i_crate = cache.i_crate;
        i_slot = cache.i_slot;
        i_channel = cache.i_channel;
        i_helicity = cache.i_helicity;
        i_quartet = cache.i_quartet;
        i_value = cache.i_value;
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
    /// crate: scaler crate
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_crate, row));
    }
    /// slot: scaler slot
    [[nodiscard]] int slot(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_slot, row));
    }
    /// channel: scaler channel (0/32 = FCUP, 1/33 = SLM, 2/24 = Clock)
    [[nodiscard]] int channel(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_channel, row));
    }
    /// helicity: helicity
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicity, row));
    }
    /// quartet: quartet
    [[nodiscard]] int quartet(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_quartet, row));
    }
    /// value: scaler value
    [[nodiscard]] int64_t value(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_value, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_crate = -1;
    int i_slot = -1;
    int i_channel = -1;
    int i_helicity = -1;
    int i_quartet = -1;
    int i_value = -1;

    friend class Row;
};

/// Lightweight row view for RAW::scaler. Stores a pointer + row index only.
class RAW_scaler::Row {
 public:
    Row(const RAW_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int slot() const noexcept { return m_bank->slot(m_row); }
    [[nodiscard]] int channel() const noexcept { return m_bank->channel(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int quartet() const noexcept { return m_bank->quartet(m_row); }
    [[nodiscard]] int64_t value() const noexcept { return m_bank->value(m_row); }
 private:
    const RAW_scaler* m_bank;
    int m_row;
};

inline RAW_scaler::Row RAW_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
