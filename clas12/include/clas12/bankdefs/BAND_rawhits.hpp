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


/// HIPO bank wrapper for `BAND::rawhits` (group 22100, item 22).
/// single PMT hits from BAND
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// BAND_rawhits bank;
/// if (auto sc = bank.bind(event.get_bank("BAND::rawhits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class BAND_rawhits {
 public:
    static constexpr std::string_view bank_name = "BAND::rawhits";
    static constexpr int bank_group = 22100;
    static constexpr int bank_item  = 22;

    class Row;

    BAND_rawhits() = default;

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
            int i_id = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_side = -1;
            int i_adc = -1;
            int i_ampl = -1;
            int i_time = -1;
            int i_timeFadc = -1;
            int i_timeCorr = -1;
            int i_indexTdc = -1;
            int i_indexAdc = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_side = schema.exists("side") ? schema.getEntryOrder("side") : -1;
            cache.i_adc = schema.exists("adc") ? schema.getEntryOrder("adc") : -1;
            cache.i_ampl = schema.exists("ampl") ? schema.getEntryOrder("ampl") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_timeFadc = schema.exists("timeFadc") ? schema.getEntryOrder("timeFadc") : -1;
            cache.i_timeCorr = schema.exists("timeCorr") ? schema.getEntryOrder("timeCorr") : -1;
            cache.i_indexTdc = schema.exists("indexTdc") ? schema.getEntryOrder("indexTdc") : -1;
            cache.i_indexAdc = schema.exists("indexAdc") ? schema.getEntryOrder("indexAdc") : -1;
        }
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_side = cache.i_side;
        i_adc = cache.i_adc;
        i_ampl = cache.i_ampl;
        i_time = cache.i_time;
        i_timeFadc = cache.i_timeFadc;
        i_timeCorr = cache.i_timeCorr;
        i_indexTdc = cache.i_indexTdc;
        i_indexAdc = cache.i_indexAdc;
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
    /// side: side of bar (L = 0 and R = 1)
    [[nodiscard]] int side(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_side, row));
    }
    /// adc: Raw ADC from FADC
    [[nodiscard]] float adc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_adc, row));
    }
    /// ampl: ADC ampl from FADC
    [[nodiscard]] float ampl(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ampl, row));
    }
    /// time: Raw Time (ns) from TDC
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// timeFadc: Raw Time (ns) from FADC
    [[nodiscard]] float timeFadc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_timeFadc, row));
    }
    /// timeCorr: Raw Time (ns) from TDC corrected for trigger phase
    [[nodiscard]] float timeCorr(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_timeCorr, row));
    }
    /// indexTdc: index to raw information in BAND::TDC bank
    [[nodiscard]] int indexTdc(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexTdc, row));
    }
    /// indexAdc: index to raw information in BAND::ADC bank
    [[nodiscard]] int indexAdc(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexAdc, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_side = -1;
    int i_adc = -1;
    int i_ampl = -1;
    int i_time = -1;
    int i_timeFadc = -1;
    int i_timeCorr = -1;
    int i_indexTdc = -1;
    int i_indexAdc = -1;

    friend class Row;
};

/// Lightweight row view for BAND::rawhits. Stores a pointer + row index only.
class BAND_rawhits::Row {
 public:
    Row(const BAND_rawhits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int side() const noexcept { return m_bank->side(m_row); }
    [[nodiscard]] float adc() const noexcept { return m_bank->adc(m_row); }
    [[nodiscard]] float ampl() const noexcept { return m_bank->ampl(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float timeFadc() const noexcept { return m_bank->timeFadc(m_row); }
    [[nodiscard]] float timeCorr() const noexcept { return m_bank->timeCorr(m_row); }
    [[nodiscard]] int indexTdc() const noexcept { return m_bank->indexTdc(m_row); }
    [[nodiscard]] int indexAdc() const noexcept { return m_bank->indexAdc(m_row); }
 private:
    const BAND_rawhits* m_bank;
    int m_row;
};

inline BAND_rawhits::Row BAND_rawhits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
