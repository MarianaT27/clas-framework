// =============================================================================
// Auto-generated from tof.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `CTOF::rawhits` (group 20400, item 20).
/// reconstructed hit info from U/D CTOF PMTs
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CTOF_rawhits bank;
/// if (auto sc = bank.bind(event.get_bank("CTOF::rawhits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CTOF_rawhits {
 public:
    static constexpr std::string_view bank_name = "CTOF::rawhits";
    static constexpr int bank_group = 20400;
    static constexpr int bank_item  = 20;

    class Row;

    CTOF_rawhits() = default;

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
            int i_id = -1;
            int i_status = -1;
            int i_component = -1;
            int i_energy_up = -1;
            int i_energy_down = -1;
            int i_time_up = -1;
            int i_time_down = -1;
            int i_energy_up_unc = -1;
            int i_energy_down_unc = -1;
            int i_time_up_unc = -1;
            int i_time_down_unc = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_energy_up = schema.exists("energy_up") ? schema.getEntryOrder("energy_up") : -1;
            cache.i_energy_down = schema.exists("energy_down") ? schema.getEntryOrder("energy_down") : -1;
            cache.i_time_up = schema.exists("time_up") ? schema.getEntryOrder("time_up") : -1;
            cache.i_time_down = schema.exists("time_down") ? schema.getEntryOrder("time_down") : -1;
            cache.i_energy_up_unc = schema.exists("energy_up_unc") ? schema.getEntryOrder("energy_up_unc") : -1;
            cache.i_energy_down_unc = schema.exists("energy_down_unc") ? schema.getEntryOrder("energy_down_unc") : -1;
            cache.i_time_up_unc = schema.exists("time_up_unc") ? schema.getEntryOrder("time_up_unc") : -1;
            cache.i_time_down_unc = schema.exists("time_down_unc") ? schema.getEntryOrder("time_down_unc") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_component = cache.i_component;
        i_energy_up = cache.i_energy_up;
        i_energy_down = cache.i_energy_down;
        i_time_up = cache.i_time_up;
        i_time_down = cache.i_time_down;
        i_energy_up_unc = cache.i_energy_up_unc;
        i_energy_down_unc = cache.i_energy_down_unc;
        i_time_up_unc = cache.i_time_up_unc;
        i_time_down_unc = cache.i_time_down_unc;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// status: hit status, a 4-digit word defined based on presence (0) or absence (1) of TDCR-TDCL-ADCR-ADCL
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// component: paddle id of CTOF
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// energy_up: E dep (MeV) from U PMT
    [[nodiscard]] float energy_up(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_up, row));
    }
    /// energy_down: E dep (MeV) from D PMT
    [[nodiscard]] float energy_down(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_down, row));
    }
    /// time_up: Hit time (ns) from U PMT
    [[nodiscard]] float time_up(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_up, row));
    }
    /// time_down: Hit time (ns) from D PMT
    [[nodiscard]] float time_down(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_down, row));
    }
    /// energy_up_unc: E dep unc (MeV) from U PMT
    [[nodiscard]] float energy_up_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_up_unc, row));
    }
    /// energy_down_unc: E dep unc (MeV) from D PMT
    [[nodiscard]] float energy_down_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_down_unc, row));
    }
    /// time_up_unc: Hit time unc (ns) from U PMT
    [[nodiscard]] float time_up_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_up_unc, row));
    }
    /// time_down_unc: Hit time unc (ns) from D PMT
    [[nodiscard]] float time_down_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_down_unc, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_component = -1;
    int i_energy_up = -1;
    int i_energy_down = -1;
    int i_time_up = -1;
    int i_time_down = -1;
    int i_energy_up_unc = -1;
    int i_energy_down_unc = -1;
    int i_time_up_unc = -1;
    int i_time_down_unc = -1;

    friend class Row;
};

/// Lightweight row view for CTOF::rawhits. Stores a pointer + row index only.
class CTOF_rawhits::Row {
 public:
    Row(const CTOF_rawhits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy_up() const noexcept { return m_bank->energy_up(m_row); }
    [[nodiscard]] float energy_down() const noexcept { return m_bank->energy_down(m_row); }
    [[nodiscard]] float time_up() const noexcept { return m_bank->time_up(m_row); }
    [[nodiscard]] float time_down() const noexcept { return m_bank->time_down(m_row); }
    [[nodiscard]] float energy_up_unc() const noexcept { return m_bank->energy_up_unc(m_row); }
    [[nodiscard]] float energy_down_unc() const noexcept { return m_bank->energy_down_unc(m_row); }
    [[nodiscard]] float time_up_unc() const noexcept { return m_bank->time_up_unc(m_row); }
    [[nodiscard]] float time_down_unc() const noexcept { return m_bank->time_down_unc(m_row); }
 private:
    const CTOF_rawhits* m_bank;
    int m_row;
};

inline CTOF_rawhits::Row CTOF_rawhits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
