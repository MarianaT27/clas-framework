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


/// HIPO bank wrapper for `DC::jitter` (group 20600, item 13).
/// DCRB timestamps bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// DC_jitter bank;
/// if (auto sc = bank.bind(event.get_bank("DC::jitter")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class DC_jitter {
 public:
    static constexpr std::string_view bank_name = "DC::jitter";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 13;
    static constexpr std::string_view bank_schema = "crate/B,slot/B,timestamp/L";

    class Row;

    DC_jitter() = default;

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
            int i_timestamp = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_crate = schema.exists("crate") ? schema.getEntryOrder("crate") : -1;
            cache.i_slot = schema.exists("slot") ? schema.getEntryOrder("slot") : -1;
            cache.i_timestamp = schema.exists("timestamp") ? schema.getEntryOrder("timestamp") : -1;
        }
        i_crate = cache.i_crate;
        i_slot = cache.i_slot;
        i_timestamp = cache.i_timestamp;
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
    /// crate: crate
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_crate, row));
    }
    /// slot: slot
    [[nodiscard]] int slot(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_slot, row));
    }
    /// timestamp: timestamp (in units of 8 ns)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_timestamp, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_crate = -1;
    int i_slot = -1;
    int i_timestamp = -1;

    friend class Row;
};

/// Lightweight row view for DC::jitter. Stores a pointer + row index only.
class DC_jitter::Row {
 public:
    Row(const DC_jitter* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int slot() const noexcept { return m_bank->slot(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
 private:
    const DC_jitter* m_bank;
    int m_row;
};

inline DC_jitter::Row DC_jitter::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
