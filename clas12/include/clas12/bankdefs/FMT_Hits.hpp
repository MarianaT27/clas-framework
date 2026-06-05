// =============================================================================
// Auto-generated from fmt.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `FMT::Hits` (group 20800, item 21).
/// reconstructed FMT hits
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FMT_Hits bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class FMT_Hits {
 public:
    static constexpr std::string_view bank_name = "FMT::Hits";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 21;
    static constexpr std::string_view bank_schema = "sector/B,layer/B,strip/S,energy/F,time/F,localY/F,residual/F,adcIndex/S,clusterIndex/S,trackIndex/S,status/B";

    class Row;

    FMT_Hits() = default;

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
            int i_strip = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_localY = -1;
            int i_residual = -1;
            int i_adcIndex = -1;
            int i_clusterIndex = -1;
            int i_trackIndex = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_strip = schema.exists("strip") ? schema.getEntryOrder("strip") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_localY = schema.exists("localY") ? schema.getEntryOrder("localY") : -1;
            cache.i_residual = schema.exists("residual") ? schema.getEntryOrder("residual") : -1;
            cache.i_adcIndex = schema.exists("adcIndex") ? schema.getEntryOrder("adcIndex") : -1;
            cache.i_clusterIndex = schema.exists("clusterIndex") ? schema.getEntryOrder("clusterIndex") : -1;
            cache.i_trackIndex = schema.exists("trackIndex") ? schema.getEntryOrder("trackIndex") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_strip = cache.i_strip;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_localY = cache.i_localY;
        i_residual = cache.i_residual;
        i_adcIndex = cache.i_adcIndex;
        i_clusterIndex = cache.i_clusterIndex;
        i_trackIndex = cache.i_trackIndex;
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
    /// sector: hit sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: hit layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// strip: hit strip
    [[nodiscard]] int strip(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_strip, row));
    }
    /// energy: hit energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: hit time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// localY: hit local y coordinate
    [[nodiscard]] float localY(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_localY, row));
    }
    /// residual: fitted hit residual
    [[nodiscard]] float residual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_residual, row));
    }
    /// adcIndex: associated adc index
    [[nodiscard]] int adcIndex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_adcIndex, row));
    }
    /// clusterIndex: associated cluster index
    [[nodiscard]] int clusterIndex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterIndex, row));
    }
    /// trackIndex: associated track index
    [[nodiscard]] int trackIndex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trackIndex, row));
    }
    /// status: hits status (0==good)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_sector = -1;
    int i_layer = -1;
    int i_strip = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_localY = -1;
    int i_residual = -1;
    int i_adcIndex = -1;
    int i_clusterIndex = -1;
    int i_trackIndex = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for FMT::Hits. Stores a pointer + row index only.
class FMT_Hits::Row {
 public:
    Row(const FMT_Hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int strip() const noexcept { return m_bank->strip(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float localY() const noexcept { return m_bank->localY(m_row); }
    [[nodiscard]] float residual() const noexcept { return m_bank->residual(m_row); }
    [[nodiscard]] int adcIndex() const noexcept { return m_bank->adcIndex(m_row); }
    [[nodiscard]] int clusterIndex() const noexcept { return m_bank->clusterIndex(m_row); }
    [[nodiscard]] int trackIndex() const noexcept { return m_bank->trackIndex(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const FMT_Hits* m_bank;
    int m_row;
};

inline FMT_Hits::Row FMT_Hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
