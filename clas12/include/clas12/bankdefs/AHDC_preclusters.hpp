// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `AHDC::preclusters` (group 23000, item 24).
/// Pre Clusters info
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// AHDC_preclusters bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::preclusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto x = row.x();
/// }
/// @endcode
class AHDC_preclusters {
 public:
    static constexpr std::string_view bank_name = "AHDC::preclusters";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 24;

    class Row;

    AHDC_preclusters() = default;

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
            int i_x = -1;
            int i_y = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
        }
        i_x = cache.i_x;
        i_y = cache.i_y;
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
    /// x: x info (mm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y info (mm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_x = -1;
    int i_y = -1;

    friend class Row;
};

/// Lightweight row view for AHDC::preclusters. Stores a pointer + row index only.
class AHDC_preclusters::Row {
 public:
    Row(const AHDC_preclusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
 private:
    const AHDC_preclusters* m_bank;
    int m_row;
};

inline AHDC_preclusters::Row AHDC_preclusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
