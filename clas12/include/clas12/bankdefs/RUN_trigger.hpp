// =============================================================================
// Auto-generated from header.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RUN::trigger` (group 10000, item 13).
/// RUN trigger information
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RUN_trigger bank;
/// if (auto sc = bank.bind(event.get_bank("RUN::trigger")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RUN_trigger {
 public:
    static constexpr std::string_view bank_name = "RUN::trigger";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 13;
    static constexpr std::string_view bank_schema = "id/I,trigger/I";

    class Row;

    RUN_trigger() = default;

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
            int i_id = -1;
            int i_trigger = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_trigger = schema.exists("trigger") ? schema.getEntryOrder("trigger") : -1;
        }
        i_id = cache.i_id;
        i_trigger = cache.i_trigger;
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
    /// id: id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_id, row));
    }
    /// trigger: trigger word
    [[nodiscard]] int trigger(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trigger, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_trigger = -1;

    friend class Row;
};

/// Lightweight row view for RUN::trigger. Stores a pointer + row index only.
class RUN_trigger::Row {
 public:
    Row(const RUN_trigger* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int trigger() const noexcept { return m_bank->trigger(m_row); }
 private:
    const RUN_trigger* m_bank;
    int m_row;
};

inline RUN_trigger::Row RUN_trigger::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
