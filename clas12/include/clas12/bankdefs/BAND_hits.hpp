// =============================================================================
// Auto-generated from band.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `BAND::hits` (group 22100, item 21).
/// reconstructed coincidence hits from BAND
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// BAND_hits bank;
/// if (auto sc = bank.bind(event.get_bank("BAND::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class BAND_hits {
 public:
    static constexpr std::string_view bank_name = "BAND::hits";
    static constexpr int bank_group = 22100;
    static constexpr int bank_item  = 21;

    class Row;

    BAND_hits() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_ex = -1;
            int i_ey = -1;
            int i_ez = -1;
            int i_timeFadc = -1;
            int i_difftime = -1;
            int i_difftimeFadc = -1;
            int i_indexLpmt = -1;
            int i_indexRpmt = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_ex = schema.exists("ex") ? schema.getEntryOrder("ex") : -1;
            cache.i_ey = schema.exists("ey") ? schema.getEntryOrder("ey") : -1;
            cache.i_ez = schema.exists("ez") ? schema.getEntryOrder("ez") : -1;
            cache.i_timeFadc = schema.exists("timeFadc") ? schema.getEntryOrder("timeFadc") : -1;
            cache.i_difftime = schema.exists("difftime") ? schema.getEntryOrder("difftime") : -1;
            cache.i_difftimeFadc = schema.exists("difftimeFadc") ? schema.getEntryOrder("difftimeFadc") : -1;
            cache.i_indexLpmt = schema.exists("indexLpmt") ? schema.getEntryOrder("indexLpmt") : -1;
            cache.i_indexRpmt = schema.exists("indexRpmt") ? schema.getEntryOrder("indexRpmt") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_ex = cache.i_ex;
        i_ey = cache.i_ey;
        i_ez = cache.i_ez;
        i_timeFadc = cache.i_timeFadc;
        i_difftime = cache.i_difftime;
        i_difftimeFadc = cache.i_difftimeFadc;
        i_indexLpmt = cache.i_indexLpmt;
        i_indexRpmt = cache.i_indexRpmt;
        i_status = cache.i_status;
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
    /// id: id of the coincidence hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// sector: sector of BAND
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer or column id of BAND
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: paddle or row id of BAND
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// energy: Geometric mean of L and R ADC
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Meantime from TDC from L and R hit (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// x: X coor (cm) of coincidence hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Y coor (cm) of coincidence hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Z coor (cm) of coincidence hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// ex: X coor uncertainty (cm) of coincidence hit
    [[nodiscard]] float ex(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ex, row));
    }
    /// ey: Y coor uncertainty (cm) of coincidence hit
    [[nodiscard]] float ey(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ey, row));
    }
    /// ez: Z coor uncertainty (cm) of coincidence hit
    [[nodiscard]] float ez(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ez, row));
    }
    /// timeFadc: Meantime from FADC from L and R hit (ns)
    [[nodiscard]] float timeFadc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_timeFadc, row));
    }
    /// difftime: Calibrated time difference of L and R on a bar(ns) from TDC
    [[nodiscard]] float difftime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_difftime, row));
    }
    /// difftimeFadc: Calibrated time difference of L and R on a bar(ns) from FADC
    [[nodiscard]] float difftimeFadc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_difftimeFadc, row));
    }
    /// indexLpmt: index of rawhit from Left PMT, starts at zero
    [[nodiscard]] int indexLpmt(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexLpmt, row));
    }
    /// indexRpmt: index of rawhit from Right PMT, starts at zero
    [[nodiscard]] int indexRpmt(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexRpmt, row));
    }
    /// status: status of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_ex = -1;
    int i_ey = -1;
    int i_ez = -1;
    int i_timeFadc = -1;
    int i_difftime = -1;
    int i_difftimeFadc = -1;
    int i_indexLpmt = -1;
    int i_indexRpmt = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for BAND::hits. Stores a pointer + row index only.
class BAND_hits::Row {
 public:
    Row(const BAND_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float ex() const noexcept { return m_bank->ex(m_row); }
    [[nodiscard]] float ey() const noexcept { return m_bank->ey(m_row); }
    [[nodiscard]] float ez() const noexcept { return m_bank->ez(m_row); }
    [[nodiscard]] float timeFadc() const noexcept { return m_bank->timeFadc(m_row); }
    [[nodiscard]] float difftime() const noexcept { return m_bank->difftime(m_row); }
    [[nodiscard]] float difftimeFadc() const noexcept { return m_bank->difftimeFadc(m_row); }
    [[nodiscard]] int indexLpmt() const noexcept { return m_bank->indexLpmt(m_row); }
    [[nodiscard]] int indexRpmt() const noexcept { return m_bank->indexRpmt(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const BAND_hits* m_bank;
    int m_row;
};

inline BAND_hits::Row BAND_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
