// =============================================================================
// Auto-generated from bmt.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `BMTRec::Hits` (group 20100, item 21).
/// first pass reconstructed BMT hits on track
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// BMTRec_Hits bank;
/// if (auto sc = bank.bind(event.get_bank("BMTRec::Hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class BMTRec_Hits {
 public:
    static constexpr std::string_view bank_name = "BMTRec::Hits";
    static constexpr int bank_group = 20100;
    static constexpr int bank_item  = 21;

    class Row;

    BMTRec_Hits() = default;

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
            int i_ID = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_strip = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_fitResidual = -1;
            int i_trkingStat = -1;
            int i_clusterID = -1;
            int i_trkID = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_ID = schema.exists("ID") ? schema.getEntryOrder("ID") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_strip = schema.exists("strip") ? schema.getEntryOrder("strip") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_fitResidual = schema.exists("fitResidual") ? schema.getEntryOrder("fitResidual") : -1;
            cache.i_trkingStat = schema.exists("trkingStat") ? schema.getEntryOrder("trkingStat") : -1;
            cache.i_clusterID = schema.exists("clusterID") ? schema.getEntryOrder("clusterID") : -1;
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_ID = cache.i_ID;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_strip = cache.i_strip;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_fitResidual = cache.i_fitResidual;
        i_trkingStat = cache.i_trkingStat;
        i_clusterID = cache.i_clusterID;
        i_trkID = cache.i_trkID;
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
    /// ID: hit ID
    [[nodiscard]] int ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ID, row));
    }
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
    /// energy: energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// fitResidual: fitted hit residual
    [[nodiscard]] float fitResidual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fitResidual, row));
    }
    /// trkingStat: tracking status
    [[nodiscard]] int trkingStat(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_trkingStat, row));
    }
    /// clusterID: associated cluster ID
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterID, row));
    }
    /// trkID: associated track ID
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trkID, row));
    }
    /// status: status (0=good)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_ID = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_strip = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_fitResidual = -1;
    int i_trkingStat = -1;
    int i_clusterID = -1;
    int i_trkID = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for BMTRec::Hits. Stores a pointer + row index only.
class BMTRec_Hits::Row {
 public:
    Row(const BMTRec_Hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int strip() const noexcept { return m_bank->strip(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float fitResidual() const noexcept { return m_bank->fitResidual(m_row); }
    [[nodiscard]] int trkingStat() const noexcept { return m_bank->trkingStat(m_row); }
    [[nodiscard]] int clusterID() const noexcept { return m_bank->clusterID(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const BMTRec_Hits* m_bank;
    int m_row;
};

inline BMTRec_Hits::Row BMTRec_Hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
