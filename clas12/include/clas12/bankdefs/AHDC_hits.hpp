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


/// HIPO bank wrapper for `AHDC::hits` (group 23000, item 23).
/// Raw Hits
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// AHDC_hits bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class AHDC_hits {
 public:
    static constexpr std::string_view bank_name = "AHDC::hits";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 23;
    static constexpr std::string_view bank_schema = "id/S,layer/B,superlayer/B,wire/I,doca/D,residual/D,time/D,adc/I,timeOverThreshold/D,trackid/I";

    class Row;

    AHDC_hits() = default;

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
            int i_layer = -1;
            int i_superlayer = -1;
            int i_wire = -1;
            int i_doca = -1;
            int i_residual = -1;
            int i_time = -1;
            int i_adc = -1;
            int i_timeOverThreshold = -1;
            int i_trackid = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_superlayer = schema.exists("superlayer") ? schema.getEntryOrder("superlayer") : -1;
            cache.i_wire = schema.exists("wire") ? schema.getEntryOrder("wire") : -1;
            cache.i_doca = schema.exists("doca") ? schema.getEntryOrder("doca") : -1;
            cache.i_residual = schema.exists("residual") ? schema.getEntryOrder("residual") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_adc = schema.exists("adc") ? schema.getEntryOrder("adc") : -1;
            cache.i_timeOverThreshold = schema.exists("timeOverThreshold") ? schema.getEntryOrder("timeOverThreshold") : -1;
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
        }
        i_id = cache.i_id;
        i_layer = cache.i_layer;
        i_superlayer = cache.i_superlayer;
        i_wire = cache.i_wire;
        i_doca = cache.i_doca;
        i_residual = cache.i_residual;
        i_time = cache.i_time;
        i_adc = cache.i_adc;
        i_timeOverThreshold = cache.i_timeOverThreshold;
        i_trackid = cache.i_trackid;
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
    /// id: hit id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// layer: layer number
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// superlayer: superlayer number
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_superlayer, row));
    }
    /// wire: wire number
    [[nodiscard]] int wire(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_wire, row));
    }
    /// doca: distance of closest approch (mm) is actually the distance from time2distance calibration
    [[nodiscard]] double doca(int row) const noexcept {
        return static_cast<double>(m_view->getDouble(i_doca, row));
    }
    /// residual: residual (mm) calculated by the Kalman Filter
    [[nodiscard]] double residual(int row) const noexcept {
        return static_cast<double>(m_view->getDouble(i_residual, row));
    }
    /// time: calibrated time (ns)
    [[nodiscard]] double time(int row) const noexcept {
        return static_cast<double>(m_view->getDouble(i_time, row));
    }
    /// adc: calibrated ADC
    [[nodiscard]] int adc(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_adc, row));
    }
    /// timeOverThreshold: calibrated time over threshold (ns)
    [[nodiscard]] double timeOverThreshold(int row) const noexcept {
        return static_cast<double>(m_view->getDouble(i_timeOverThreshold, row));
    }
    /// trackid: track id, is -1 if not associated to a Track
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trackid, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_layer = -1;
    int i_superlayer = -1;
    int i_wire = -1;
    int i_doca = -1;
    int i_residual = -1;
    int i_time = -1;
    int i_adc = -1;
    int i_timeOverThreshold = -1;
    int i_trackid = -1;

    friend class Row;
};

/// Lightweight row view for AHDC::hits. Stores a pointer + row index only.
class AHDC_hits::Row {
 public:
    Row(const AHDC_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int wire() const noexcept { return m_bank->wire(m_row); }
    [[nodiscard]] double doca() const noexcept { return m_bank->doca(m_row); }
    [[nodiscard]] double residual() const noexcept { return m_bank->residual(m_row); }
    [[nodiscard]] double time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int adc() const noexcept { return m_bank->adc(m_row); }
    [[nodiscard]] double timeOverThreshold() const noexcept { return m_bank->timeOverThreshold(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
 private:
    const AHDC_hits* m_bank;
    int m_row;
};

inline AHDC_hits::Row AHDC_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
