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


/// HIPO bank wrapper for `COAT::config` (group 10000, item 17).
/// COATJAVA configuration, e.g. version numbers, CLARA YAML settings
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// COAT_config bank;
/// if (auto sc = bank.bind(event.get_bank("COAT::config")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto json = row.json();
/// }
/// @endcode
class COAT_config {
 public:
    static constexpr std::string_view bank_name = "COAT::config";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 17;

    class Row;

    COAT_config() = default;

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
            int i_json = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_json = schema.exists("json") ? schema.getEntryOrder("json") : -1;
        }
        i_json = cache.i_json;
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
    /// json: json character
    [[nodiscard]] int json(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_json, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_json = -1;

    friend class Row;
};

/// Lightweight row view for COAT::config. Stores a pointer + row index only.
class COAT_config::Row {
 public:
    Row(const COAT_config* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int json() const noexcept { return m_bank->json(m_row); }
 private:
    const COAT_config* m_bank;
    int m_row;
};

inline COAT_config::Row COAT_config::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
