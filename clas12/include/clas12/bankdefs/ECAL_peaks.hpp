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


/// HIPO bank wrapper for `ECAL::peaks` (group 20700, item 22).
/// reconstructed peaks from ECAL
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ECAL_peaks bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::peaks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ECAL_peaks {
 public:
    static constexpr std::string_view bank_name = "ECAL::peaks";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 22;
    static constexpr std::string_view bank_schema = "id/S,status/S,sector/B,layer/B,energy/F,time/F,xo/F,yo/F,zo/F,xe/F,ye/F,ze/F,width/F";

    class Row;

    ECAL_peaks() = default;

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
            int i_energy = -1;
            int i_time = -1;
            int i_xo = -1;
            int i_yo = -1;
            int i_zo = -1;
            int i_xe = -1;
            int i_ye = -1;
            int i_ze = -1;
            int i_width = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_xo = schema.exists("xo") ? schema.getEntryOrder("xo") : -1;
            cache.i_yo = schema.exists("yo") ? schema.getEntryOrder("yo") : -1;
            cache.i_zo = schema.exists("zo") ? schema.getEntryOrder("zo") : -1;
            cache.i_xe = schema.exists("xe") ? schema.getEntryOrder("xe") : -1;
            cache.i_ye = schema.exists("ye") ? schema.getEntryOrder("ye") : -1;
            cache.i_ze = schema.exists("ze") ? schema.getEntryOrder("ze") : -1;
            cache.i_width = schema.exists("width") ? schema.getEntryOrder("width") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_xo = cache.i_xo;
        i_yo = cache.i_yo;
        i_zo = cache.i_zo;
        i_xe = cache.i_xe;
        i_ye = cache.i_ye;
        i_ze = cache.i_ze;
        i_width = cache.i_width;
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
    /// status: status of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// sector: sector of ECAL
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: Layer of ECAL (1-3:PCAL, 4-6:ECIN, 7-9:ECOUT
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// energy: Energy of the hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Time of the hit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// xo: strip origin X coordinate
    [[nodiscard]] float xo(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_xo, row));
    }
    /// yo: strip origin Y coordinate
    [[nodiscard]] float yo(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_yo, row));
    }
    /// zo: strip origin Z coordinate
    [[nodiscard]] float zo(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_zo, row));
    }
    /// xe: strip end    X coordinate
    [[nodiscard]] float xe(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_xe, row));
    }
    /// ye: strip end    Y coordinate
    [[nodiscard]] float ye(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ye, row));
    }
    /// ze: strip end    Z coordinate
    [[nodiscard]] float ze(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ze, row));
    }
    /// width: width of the peak
    [[nodiscard]] float width(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_width, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_xo = -1;
    int i_yo = -1;
    int i_zo = -1;
    int i_xe = -1;
    int i_ye = -1;
    int i_ze = -1;
    int i_width = -1;

    friend class Row;
};

/// Lightweight row view for ECAL::peaks. Stores a pointer + row index only.
class ECAL_peaks::Row {
 public:
    Row(const ECAL_peaks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float xo() const noexcept { return m_bank->xo(m_row); }
    [[nodiscard]] float yo() const noexcept { return m_bank->yo(m_row); }
    [[nodiscard]] float zo() const noexcept { return m_bank->zo(m_row); }
    [[nodiscard]] float xe() const noexcept { return m_bank->xe(m_row); }
    [[nodiscard]] float ye() const noexcept { return m_bank->ye(m_row); }
    [[nodiscard]] float ze() const noexcept { return m_bank->ze(m_row); }
    [[nodiscard]] float width() const noexcept { return m_bank->width(m_row); }
 private:
    const ECAL_peaks* m_bank;
    int m_row;
};

inline ECAL_peaks::Row ECAL_peaks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
