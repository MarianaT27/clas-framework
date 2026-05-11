// =============================================================================
// Auto-generated from decays.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `DECAYS::Event` (group 900, item 10).
/// Event Header Bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// DECAYS_Event bank;
/// if (auto sc = bank.bind(event.get_bank("DECAYS::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto tarmass = row.tarmass();
/// }
/// @endcode
class DECAYS_Event {
 public:
    static constexpr std::string_view bank_name = "DECAYS::Event";
    static constexpr int bank_group = 900;
    static constexpr int bank_item  = 10;

    class Row;

    DECAYS_Event() = default;

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
            int i_tarmass = -1;
            int i_beamenergy = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_tarmass = schema.exists("tarmass") ? schema.getEntryOrder("tarmass") : -1;
            cache.i_beamenergy = schema.exists("beamenergy") ? schema.getEntryOrder("beamenergy") : -1;
        }
        i_tarmass = cache.i_tarmass;
        i_beamenergy = cache.i_beamenergy;
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
    /// tarmass: target mass
    [[nodiscard]] float tarmass(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tarmass, row));
    }
    /// beamenergy: beam energy
    [[nodiscard]] float beamenergy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_beamenergy, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_tarmass = -1;
    int i_beamenergy = -1;

    friend class Row;
};

/// Lightweight row view for DECAYS::Event. Stores a pointer + row index only.
class DECAYS_Event::Row {
 public:
    Row(const DECAYS_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float tarmass() const noexcept { return m_bank->tarmass(m_row); }
    [[nodiscard]] float beamenergy() const noexcept { return m_bank->beamenergy(m_row); }
 private:
    const DECAYS_Event* m_bank;
    int m_row;
};

inline DECAYS_Event::Row DECAYS_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
