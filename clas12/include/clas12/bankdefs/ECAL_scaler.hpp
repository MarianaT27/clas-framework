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


/// HIPO bank wrapper for `ECAL::scaler` (group 20700, item 26).
/// Occupancy bank for the Electromagnetic Calorimeter
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ECAL_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class ECAL_scaler {
 public:
    static constexpr std::string_view bank_name = "ECAL::scaler";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 26;

    class Row;

    ECAL_scaler() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_acount = -1;
            int i_tcount = -1;
            int i_avalue = -1;
            int i_tvalue = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_acount = schema.exists("acount") ? schema.getEntryOrder("acount") : -1;
            cache.i_tcount = schema.exists("tcount") ? schema.getEntryOrder("tcount") : -1;
            cache.i_avalue = schema.exists("avalue") ? schema.getEntryOrder("avalue") : -1;
            cache.i_tvalue = schema.exists("tvalue") ? schema.getEntryOrder("tvalue") : -1;
        }
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_acount = cache.i_acount;
        i_tcount = cache.i_tcount;
        i_avalue = cache.i_avalue;
        i_tvalue = cache.i_tvalue;
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
    /// sector: sector (1-6)
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer (1..9)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: strip
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// acount: ADC counts
    [[nodiscard]] int acount(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_acount, row));
    }
    /// tcount: TDC counts
    [[nodiscard]] int tcount(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tcount, row));
    }
    /// avalue: ADC value
    [[nodiscard]] int avalue(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_avalue, row));
    }
    /// tvalue: TDC value
    [[nodiscard]] int tvalue(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tvalue, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_acount = -1;
    int i_tcount = -1;
    int i_avalue = -1;
    int i_tvalue = -1;

    friend class Row;
};

/// Lightweight row view for ECAL::scaler. Stores a pointer + row index only.
class ECAL_scaler::Row {
 public:
    Row(const ECAL_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int acount() const noexcept { return m_bank->acount(m_row); }
    [[nodiscard]] int tcount() const noexcept { return m_bank->tcount(m_row); }
    [[nodiscard]] int avalue() const noexcept { return m_bank->avalue(m_row); }
    [[nodiscard]] int tvalue() const noexcept { return m_bank->tvalue(m_row); }
 private:
    const ECAL_scaler* m_bank;
    int m_row;
};

inline ECAL_scaler::Row ECAL_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
