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


/// HIPO bank wrapper for `DC::doca` (group 20600, item 14).
/// Drift Chamber DOCA bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// DC_doca bank;
/// if (auto sc = bank.bind(event.get_bank("DC::doca")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto LR = row.LR();
/// }
/// @endcode
class DC_doca {
 public:
    static constexpr std::string_view bank_name = "DC::doca";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 14;

    class Row;

    DC_doca() = default;

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
            int i_LR = -1;
            int i_doca = -1;
            int i_sdoca = -1;
            int i_time = -1;
            int i_stime = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_LR = schema.exists("LR") ? schema.getEntryOrder("LR") : -1;
            cache.i_doca = schema.exists("doca") ? schema.getEntryOrder("doca") : -1;
            cache.i_sdoca = schema.exists("sdoca") ? schema.getEntryOrder("sdoca") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_stime = schema.exists("stime") ? schema.getEntryOrder("stime") : -1;
        }
        i_LR = cache.i_LR;
        i_doca = cache.i_doca;
        i_sdoca = cache.i_sdoca;
        i_time = cache.i_time;
        i_stime = cache.i_stime;
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
    /// LR: left right ambiguity
    [[nodiscard]] int LR(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_LR, row));
    }
    /// doca: doca for the hit
    [[nodiscard]] float doca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_doca, row));
    }
    /// sdoca: smeared doca
    [[nodiscard]] float sdoca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_sdoca, row));
    }
    /// time: time for the hit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// stime: smeared time
    [[nodiscard]] float stime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_stime, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_LR = -1;
    int i_doca = -1;
    int i_sdoca = -1;
    int i_time = -1;
    int i_stime = -1;

    friend class Row;
};

/// Lightweight row view for DC::doca. Stores a pointer + row index only.
class DC_doca::Row {
 public:
    Row(const DC_doca* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int LR() const noexcept { return m_bank->LR(m_row); }
    [[nodiscard]] float doca() const noexcept { return m_bank->doca(m_row); }
    [[nodiscard]] float sdoca() const noexcept { return m_bank->sdoca(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float stime() const noexcept { return m_bank->stime(m_row); }
 private:
    const DC_doca* m_bank;
    int m_row;
};

inline DC_doca::Row DC_doca::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
