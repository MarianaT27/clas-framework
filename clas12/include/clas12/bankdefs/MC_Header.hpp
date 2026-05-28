// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `MC::Header` (group 40, item 0).
/// Head bank for the generated event
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// MC_Header bank;
/// if (auto sc = bank.bind(event.get_bank("MC::Header")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto run = row.run();
/// }
/// @endcode
class MC_Header {
 public:
    static constexpr std::string_view bank_name = "MC::Header";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 0;

    class Row;

    MC_Header() = default;

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
            int i_run = -1;
            int i_event = -1;
            int i_type = -1;
            int i_helicity = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_run = schema.exists("run") ? schema.getEntryOrder("run") : -1;
            cache.i_event = schema.exists("event") ? schema.getEntryOrder("event") : -1;
            cache.i_type = schema.exists("type") ? schema.getEntryOrder("type") : -1;
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
        }
        i_run = cache.i_run;
        i_event = cache.i_event;
        i_type = cache.i_type;
        i_helicity = cache.i_helicity;
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
    /// run: Run number
    [[nodiscard]] int run(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_run, row));
    }
    /// event: Event number
    [[nodiscard]] int event(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_event, row));
    }
    /// type: Event type
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_type, row));
    }
    /// helicity: Beam helicity
    [[nodiscard]] float helicity(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_helicity, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_run = -1;
    int i_event = -1;
    int i_type = -1;
    int i_helicity = -1;

    friend class Row;
};

/// Lightweight row view for MC::Header. Stores a pointer + row index only.
class MC_Header::Row {
 public:
    Row(const MC_Header* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int run() const noexcept { return m_bank->run(m_row); }
    [[nodiscard]] int event() const noexcept { return m_bank->event(m_row); }
    [[nodiscard]] int type() const noexcept { return m_bank->type(m_row); }
    [[nodiscard]] float helicity() const noexcept { return m_bank->helicity(m_row); }
 private:
    const MC_Header* m_bank;
    int m_row;
};

inline MC_Header::Row MC_Header::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
