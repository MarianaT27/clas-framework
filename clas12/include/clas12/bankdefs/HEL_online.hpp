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


/// HIPO bank wrapper for `HEL::online` (group 22000, item 13).
/// Helicity with online delay-correction in Level3 trigger
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// HEL_online bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::online")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto helicity = row.helicity();
/// }
/// @endcode
class HEL_online {
 public:
    static constexpr std::string_view bank_name = "HEL::online";
    static constexpr int bank_group = 22000;
    static constexpr int bank_item  = 13;

    class Row;

    HEL_online() = default;

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
            int i_helicity = -1;
            int i_helicityRaw = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
            cache.i_helicityRaw = schema.exists("helicityRaw") ? schema.getEntryOrder("helicityRaw") : -1;
        }
        i_helicity = cache.i_helicity;
        i_helicityRaw = cache.i_helicityRaw;
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
    /// helicity: online-delay-corrected helicity, with HWP-correction (0=UDF)
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicity, row));
    }
    /// helicityRaw: online-delay-corrected helicity (0=UDF)
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicityRaw, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_helicity = -1;
    int i_helicityRaw = -1;

    friend class Row;
};

/// Lightweight row view for HEL::online. Stores a pointer + row index only.
class HEL_online::Row {
 public:
    Row(const HEL_online* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int helicityRaw() const noexcept { return m_bank->helicityRaw(m_row); }
 private:
    const HEL_online* m_bank;
    int m_row;
};

inline HEL_online::Row HEL_online::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
