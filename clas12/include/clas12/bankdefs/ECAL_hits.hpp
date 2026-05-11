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


/// HIPO bank wrapper for `ECAL::hits` (group 20700, item 21).
/// ECAL hits
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ECAL_hits bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ECAL_hits {
 public:
    static constexpr std::string_view bank_name = "ECAL::hits";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 21;

    class Row;

    ECAL_hits() = default;

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
            int i_clusterId = -1;
            int i_status = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_strip = -1;
            int i_peakid = -1;
            int i_energy = -1;
            int i_time = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_clusterId = schema.exists("clusterId") ? schema.getEntryOrder("clusterId") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_strip = schema.exists("strip") ? schema.getEntryOrder("strip") : -1;
            cache.i_peakid = schema.exists("peakid") ? schema.getEntryOrder("peakid") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
        }
        i_id = cache.i_id;
        i_clusterId = cache.i_clusterId;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_strip = cache.i_strip;
        i_peakid = cache.i_peakid;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
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
    /// clusterId: id of the cluster the hit belongs to
    [[nodiscard]] int clusterId(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterId, row));
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
    /// strip: Strip number
    [[nodiscard]] int strip(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_strip, row));
    }
    /// peakid: Peak id
    [[nodiscard]] int peakid(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_peakid, row));
    }
    /// energy: Energy of the hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Time of the hit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_clusterId = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_strip = -1;
    int i_peakid = -1;
    int i_energy = -1;
    int i_time = -1;

    friend class Row;
};

/// Lightweight row view for ECAL::hits. Stores a pointer + row index only.
class ECAL_hits::Row {
 public:
    Row(const ECAL_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int clusterId() const noexcept { return m_bank->clusterId(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int strip() const noexcept { return m_bank->strip(m_row); }
    [[nodiscard]] int peakid() const noexcept { return m_bank->peakid(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
 private:
    const ECAL_hits* m_bank;
    int m_row;
};

inline ECAL_hits::Row ECAL_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
