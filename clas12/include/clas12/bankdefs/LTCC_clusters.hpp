// =============================================================================
// Auto-generated from ltcc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `LTCC::clusters` (group 21600, item 22).
/// reconstructed clusters from the LTCC
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// LTCC_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("LTCC::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class LTCC_clusters {
 public:
    static constexpr std::string_view bank_name = "LTCC::clusters";
    static constexpr int bank_group = 21600;
    static constexpr int bank_item  = 22;

    class Row;

    LTCC_clusters() = default;

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
            int i_segment = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_nphe = -1;
            int i_time = -1;
            int i_nHits = -1;
            int i_minTheta = -1;
            int i_maxTheta = -1;
            int i_minPhi = -1;
            int i_maxPhi = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_segment = schema.exists("segment") ? schema.getEntryOrder("segment") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_nphe = schema.exists("nphe") ? schema.getEntryOrder("nphe") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_nHits = schema.exists("nHits") ? schema.getEntryOrder("nHits") : -1;
            cache.i_minTheta = schema.exists("minTheta") ? schema.getEntryOrder("minTheta") : -1;
            cache.i_maxTheta = schema.exists("maxTheta") ? schema.getEntryOrder("maxTheta") : -1;
            cache.i_minPhi = schema.exists("minPhi") ? schema.getEntryOrder("minPhi") : -1;
            cache.i_maxPhi = schema.exists("maxPhi") ? schema.getEntryOrder("maxPhi") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_segment = cache.i_segment;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_nphe = cache.i_nphe;
        i_time = cache.i_time;
        i_nHits = cache.i_nHits;
        i_minTheta = cache.i_minTheta;
        i_maxTheta = cache.i_maxTheta;
        i_minPhi = cache.i_minPhi;
        i_maxPhi = cache.i_maxPhi;
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
    /// id: id of the cluster
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// status: good (1) or bad (0)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_status, row));
    }
    /// sector: sector of LTCC
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// segment: weighted central cluster segment
    [[nodiscard]] int segment(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_segment, row));
    }
    /// x: crude cluster center (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: crude cluster center (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: crude cluster center (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// nphe: number of photo-electrons
    [[nodiscard]] float nphe(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_nphe, row));
    }
    /// time: cluster time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// nHits: number of hits in this cluster
    [[nodiscard]] int nHits(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_nHits, row));
    }
    /// minTheta: minimum theta angle of the cluster (deg)
    [[nodiscard]] float minTheta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_minTheta, row));
    }
    /// maxTheta: maximum theta angle of the cluster (deg)
    [[nodiscard]] float maxTheta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_maxTheta, row));
    }
    /// minPhi: minimum phi angle of the cluster (deg)
    [[nodiscard]] float minPhi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_minPhi, row));
    }
    /// maxPhi: maximum phi angle of the cluster (deg)
    [[nodiscard]] float maxPhi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_maxPhi, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_segment = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_nphe = -1;
    int i_time = -1;
    int i_nHits = -1;
    int i_minTheta = -1;
    int i_maxTheta = -1;
    int i_minPhi = -1;
    int i_maxPhi = -1;

    friend class Row;
};

/// Lightweight row view for LTCC::clusters. Stores a pointer + row index only.
class LTCC_clusters::Row {
 public:
    Row(const LTCC_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int segment() const noexcept { return m_bank->segment(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float nphe() const noexcept { return m_bank->nphe(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int nHits() const noexcept { return m_bank->nHits(m_row); }
    [[nodiscard]] float minTheta() const noexcept { return m_bank->minTheta(m_row); }
    [[nodiscard]] float maxTheta() const noexcept { return m_bank->maxTheta(m_row); }
    [[nodiscard]] float minPhi() const noexcept { return m_bank->minPhi(m_row); }
    [[nodiscard]] float maxPhi() const noexcept { return m_bank->maxPhi(m_row); }
 private:
    const LTCC_clusters* m_bank;
    int m_row;
};

inline LTCC_clusters::Row LTCC_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
