// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RECAIFT::Event` (group 400, item 41).
/// Event Header Bank, shadowing RECAI::Event with FT-based start time
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECAIFT_Event bank;
/// if (auto sc = bank.bind(event.get_bank("RECAIFT::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto category = row.category();
/// }
/// @endcode
class RECAIFT_Event {
 public:
    static constexpr std::string_view bank_name = "RECAIFT::Event";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 41;
    static constexpr std::string_view bank_schema = "category/L,startTime/F";

    class Row;

    RECAIFT_Event() = default;

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
            int i_category = -1;
            int i_startTime = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_category = schema.exists("category") ? schema.getEntryOrder("category") : -1;
            cache.i_startTime = schema.exists("startTime") ? schema.getEntryOrder("startTime") : -1;
        }
        i_category = cache.i_category;
        i_startTime = cache.i_startTime;
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
    /// category: Undefined
    [[nodiscard]] int64_t category(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_category, row));
    }
    /// startTime: Event Start Time (ns)
    [[nodiscard]] float startTime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_startTime, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_category = -1;
    int i_startTime = -1;

    friend class Row;
};

/// Lightweight row view for RECAIFT::Event. Stores a pointer + row index only.
class RECAIFT_Event::Row {
 public:
    Row(const RECAIFT_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int64_t category() const noexcept { return m_bank->category(m_row); }
    [[nodiscard]] float startTime() const noexcept { return m_bank->startTime(m_row); }
 private:
    const RECAIFT_Event* m_bank;
    int m_row;
};

inline RECAIFT_Event::Row RECAIFT_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
