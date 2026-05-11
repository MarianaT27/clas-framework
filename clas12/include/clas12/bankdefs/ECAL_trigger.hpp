// =============================================================================
// Auto-generated from ecal.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `ECAL::trigger` (group 20700, item 27).
/// Occupancy bank for trigger bits
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ECAL_trigger bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::trigger")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto bit = row.bit();
/// }
/// @endcode
class ECAL_trigger {
 public:
    static constexpr std::string_view bank_name = "ECAL::trigger";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 27;

    class Row;

    ECAL_trigger() = default;

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
            int i_bit = -1;
            int i_counts = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_bit = schema.exists("bit") ? schema.getEntryOrder("bit") : -1;
            cache.i_counts = schema.exists("counts") ? schema.getEntryOrder("counts") : -1;
        }
        i_bit = cache.i_bit;
        i_counts = cache.i_counts;
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
    /// bit: trigger bit
    [[nodiscard]] int bit(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_bit, row));
    }
    /// counts: bit count
    [[nodiscard]] int counts(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_counts, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_bit = -1;
    int i_counts = -1;

    friend class Row;
};

/// Lightweight row view for ECAL::trigger. Stores a pointer + row index only.
class ECAL_trigger::Row {
 public:
    Row(const ECAL_trigger* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int bit() const noexcept { return m_bank->bit(m_row); }
    [[nodiscard]] int counts() const noexcept { return m_bank->counts(m_row); }
 private:
    const ECAL_trigger* m_bank;
    int m_row;
};

inline ECAL_trigger::Row ECAL_trigger::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
