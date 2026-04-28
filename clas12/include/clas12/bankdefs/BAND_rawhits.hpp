// =============================================================================
// Auto-generated from band.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include <bank_view.h>
#include <schema.h>

namespace bankdefs {


/// HIPO bank wrapper for `BAND::rawhits` (group 22100, item 22).
/// single PMT hits from BAND
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    /// Bind to a live bank_view. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column handle by name; all
    /// subsequent bind()s for the same schema hit the cache and copy 25-30
    /// trivially-copyable column_handle words.
    /// Returns false if the view is not valid.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        struct HandleCache {
            const hipo::schema* schema_ptr = nullptr;
            hipo::column_handle h_id;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_component;
            hipo::column_handle h_side;
            hipo::column_handle h_adc;
            hipo::column_handle h_ampl;
            hipo::column_handle h_time;
            hipo::column_handle h_timeFadc;
            hipo::column_handle h_timeCorr;
            hipo::column_handle h_indexTdc;
            hipo::column_handle h_indexAdc;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("component"); h) cache.h_component = *h;
            else cache.h_component = {};
            if (auto h = schema.get_handle("side"); h) cache.h_side = *h;
            else cache.h_side = {};
            if (auto h = schema.get_handle("adc"); h) cache.h_adc = *h;
            else cache.h_adc = {};
            if (auto h = schema.get_handle("ampl"); h) cache.h_ampl = *h;
            else cache.h_ampl = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("timeFadc"); h) cache.h_timeFadc = *h;
            else cache.h_timeFadc = {};
            if (auto h = schema.get_handle("timeCorr"); h) cache.h_timeCorr = *h;
            else cache.h_timeCorr = {};
            if (auto h = schema.get_handle("indexTdc"); h) cache.h_indexTdc = *h;
            else cache.h_indexTdc = {};
            if (auto h = schema.get_handle("indexAdc"); h) cache.h_indexAdc = *h;
            else cache.h_indexAdc = {};
        }
        h_id = cache.h_id;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_side = cache.h_side;
        h_adc = cache.h_adc;
        h_ampl = cache.h_ampl;
        h_time = cache.h_time;
        h_timeFadc = cache.h_timeFadc;
        h_timeCorr = cache.h_timeCorr;
        h_indexTdc = cache.h_indexTdc;
        h_indexAdc = cache.h_indexAdc;
        return true;
    }

    /// Current row count of the bound bank view.
    [[nodiscard]] int rows() const noexcept { return m_view.rows(); }

    /// True if bind() has been called with a valid view.
    [[nodiscard]] bool valid() const noexcept { return m_view.valid(); }

    /// Access the underlying bank_view (for advanced use).
    [[nodiscard]] const hipo::bank_view& view() const noexcept { return m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (handle-based, zero string lookup)
    // -------------------------------------------------------------------
    /// id: id of the coincidence hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// sector: sector of BAND
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer or column id of BAND
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: paddle or row id of BAND
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// side: side of bar (L = 0 and R = 1)
    [[nodiscard]] int side(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_side, row));
    }
    /// adc: Raw ADC from FADC
    [[nodiscard]] float adc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_adc, row));
    }
    /// ampl: ADC ampl from FADC
    [[nodiscard]] float ampl(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ampl, row));
    }
    /// time: Raw Time (ns) from TDC
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// timeFadc: Raw Time (ns) from FADC
    [[nodiscard]] float timeFadc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_timeFadc, row));
    }
    /// timeCorr: Raw Time (ns) from TDC corrected for trigger phase
    [[nodiscard]] float timeCorr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_timeCorr, row));
    }
    /// indexTdc: index to raw information in BAND::TDC bank
    [[nodiscard]] int indexTdc(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexTdc, row));
    }
    /// indexAdc: index to raw information in BAND::ADC bank
    [[nodiscard]] int indexAdc(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexAdc, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
    }
    /// side column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> side_col() const noexcept {
        return m_view.column<int16_t>(h_side);
    }
    /// adc column as a contiguous span.
    [[nodiscard]] std::span<const float> adc_col() const noexcept {
        return m_view.column<float>(h_adc);
    }
    /// ampl column as a contiguous span.
    [[nodiscard]] std::span<const float> ampl_col() const noexcept {
        return m_view.column<float>(h_ampl);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// timeFadc column as a contiguous span.
    [[nodiscard]] std::span<const float> timeFadc_col() const noexcept {
        return m_view.column<float>(h_timeFadc);
    }
    /// timeCorr column as a contiguous span.
    [[nodiscard]] std::span<const float> timeCorr_col() const noexcept {
        return m_view.column<float>(h_timeCorr);
    }
    /// indexTdc column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexTdc_col() const noexcept {
        return m_view.column<int16_t>(h_indexTdc);
    }
    /// indexAdc column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexAdc_col() const noexcept {
        return m_view.column<int16_t>(h_indexAdc);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_side{};
    hipo::column_handle h_adc{};
    hipo::column_handle h_ampl{};
    hipo::column_handle h_time{};
    hipo::column_handle h_timeFadc{};
    hipo::column_handle h_timeCorr{};
    hipo::column_handle h_indexTdc{};
    hipo::column_handle h_indexAdc{};

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
