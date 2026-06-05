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


/// HIPO bank wrapper for `FTOF::rawhits` (group 21200, item 20).
/// reconstructed hit info from L/R FTOF PMTs
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTOF_rawhits bank;
/// if (auto sc = bank.bind(event.get_bank("FTOF::rawhits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class FTOF_rawhits {
 public:
    static constexpr std::string_view bank_name = "FTOF::rawhits";
    static constexpr int bank_group = 21200;
    static constexpr int bank_item  = 20;
    static constexpr std::string_view bank_schema = "id/S,status/S,sector/B,layer/B,component/S,energy_left/F,energy_right/F,time_left/F,time_right/F,energy_left_unc/F,energy_right_unc/F,time_left_unc/F,time_right_unc/F";

    class Row;

    FTOF_rawhits() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_energy_left = -1;
            int i_energy_right = -1;
            int i_time_left = -1;
            int i_time_right = -1;
            int i_energy_left_unc = -1;
            int i_energy_right_unc = -1;
            int i_time_left_unc = -1;
            int i_time_right_unc = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_energy_left = schema.exists("energy_left") ? schema.getEntryOrder("energy_left") : -1;
            cache.i_energy_right = schema.exists("energy_right") ? schema.getEntryOrder("energy_right") : -1;
            cache.i_time_left = schema.exists("time_left") ? schema.getEntryOrder("time_left") : -1;
            cache.i_time_right = schema.exists("time_right") ? schema.getEntryOrder("time_right") : -1;
            cache.i_energy_left_unc = schema.exists("energy_left_unc") ? schema.getEntryOrder("energy_left_unc") : -1;
            cache.i_energy_right_unc = schema.exists("energy_right_unc") ? schema.getEntryOrder("energy_right_unc") : -1;
            cache.i_time_left_unc = schema.exists("time_left_unc") ? schema.getEntryOrder("time_left_unc") : -1;
            cache.i_time_right_unc = schema.exists("time_right_unc") ? schema.getEntryOrder("time_right_unc") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_energy_left = cache.i_energy_left;
        i_energy_right = cache.i_energy_right;
        i_time_left = cache.i_time_left;
        i_time_right = cache.i_time_right;
        i_energy_left_unc = cache.i_energy_left_unc;
        i_energy_right_unc = cache.i_energy_right_unc;
        i_time_left_unc = cache.i_time_left_unc;
        i_time_right_unc = cache.i_time_right_unc;
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
    /// sector: sector of FTOF
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: panel id of FTOF (1-1A, 2-1B, 3-2
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: paddle id of FTOF
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// energy_left: E dep (MeV) from L PMT
    [[nodiscard]] float energy_left(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_left, row));
    }
    /// energy_right: E dep (MeV) from R PMT
    [[nodiscard]] float energy_right(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_right, row));
    }
    /// time_left: Hit time (ns) from L PMT
    [[nodiscard]] float time_left(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_left, row));
    }
    /// time_right: Hit time (ns) from R PMT
    [[nodiscard]] float time_right(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_right, row));
    }
    /// energy_left_unc: E dep unc (MeV) from L PMT
    [[nodiscard]] float energy_left_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_left_unc, row));
    }
    /// energy_right_unc: E dep unc (MeV) from R PMT
    [[nodiscard]] float energy_right_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy_right_unc, row));
    }
    /// time_left_unc: Hit time unc (ns) from L PMT
    [[nodiscard]] float time_left_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_left_unc, row));
    }
    /// time_right_unc: Hit time unc (ns) from R PMT
    [[nodiscard]] float time_right_unc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time_right_unc, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_energy_left = -1;
    int i_energy_right = -1;
    int i_time_left = -1;
    int i_time_right = -1;
    int i_energy_left_unc = -1;
    int i_energy_right_unc = -1;
    int i_time_left_unc = -1;
    int i_time_right_unc = -1;

    friend class Row;
};

/// Lightweight row view for FTOF::rawhits. Stores a pointer + row index only.
class FTOF_rawhits::Row {
 public:
    Row(const FTOF_rawhits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy_left() const noexcept { return m_bank->energy_left(m_row); }
    [[nodiscard]] float energy_right() const noexcept { return m_bank->energy_right(m_row); }
    [[nodiscard]] float time_left() const noexcept { return m_bank->time_left(m_row); }
    [[nodiscard]] float time_right() const noexcept { return m_bank->time_right(m_row); }
    [[nodiscard]] float energy_left_unc() const noexcept { return m_bank->energy_left_unc(m_row); }
    [[nodiscard]] float energy_right_unc() const noexcept { return m_bank->energy_right_unc(m_row); }
    [[nodiscard]] float time_left_unc() const noexcept { return m_bank->time_left_unc(m_row); }
    [[nodiscard]] float time_right_unc() const noexcept { return m_bank->time_right_unc(m_row); }
 private:
    const FTOF_rawhits* m_bank;
    int m_row;
};

inline FTOF_rawhits::Row FTOF_rawhits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
