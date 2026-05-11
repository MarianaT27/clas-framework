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


/// HIPO bank wrapper for `RAW::vtp` (group 20000, item 14).
/// VTP bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RAW_vtp bank;
/// if (auto sc = bank.bind(event.get_bank("RAW::vtp")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class RAW_vtp {
 public:
    static constexpr std::string_view bank_name = "RAW::vtp";
    static constexpr int bank_group = 20000;
    static constexpr int bank_item  = 14;

    class Row;

    RAW_vtp() = default;

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
            int i_word = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_crate = schema.exists("crate") ? schema.getEntryOrder("crate") : -1;
            cache.i_word = schema.exists("word") ? schema.getEntryOrder("word") : -1;
        }
        i_crate = cache.i_crate;
        i_word = cache.i_word;
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
    /// crate: crate number
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_crate, row));
    }
    /// word: vtp value
    [[nodiscard]] int word(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_word, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_crate = -1;
    int i_word = -1;

    friend class Row;
};

/// Lightweight row view for RAW::vtp. Stores a pointer + row index only.
class RAW_vtp::Row {
 public:
    Row(const RAW_vtp* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int word() const noexcept { return m_bank->word(m_row); }
 private:
    const RAW_vtp* m_bank;
    int m_row;
};

inline RAW_vtp::Row RAW_vtp::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
