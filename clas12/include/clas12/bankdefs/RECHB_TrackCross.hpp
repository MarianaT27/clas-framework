// =============================================================================
// Auto-generated from event.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RECHB::TrackCross` (group 300, item 17).
/// Track Cross information for Particles bank (deprecated by RECHB::Traj)
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECHB_TrackCross bank;
/// if (auto sc = bank.bind(event.get_bank("RECHB::TrackCross")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class RECHB_TrackCross {
 public:
    static constexpr std::string_view bank_name = "RECHB::TrackCross";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 17;
    static constexpr std::string_view bank_schema = "index/S,pindex/S,detector/B,sector/B,layer/B,c_x/F,c_y/F,c_z/F,c_ux/F,c_uy/F,c_uz/F,status/S";

    class Row;

    RECHB_TrackCross() = default;

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
            int i_index = -1;
            int i_pindex = -1;
            int i_detector = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_c_x = -1;
            int i_c_y = -1;
            int i_c_z = -1;
            int i_c_ux = -1;
            int i_c_uy = -1;
            int i_c_uz = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_c_x = schema.exists("c_x") ? schema.getEntryOrder("c_x") : -1;
            cache.i_c_y = schema.exists("c_y") ? schema.getEntryOrder("c_y") : -1;
            cache.i_c_z = schema.exists("c_z") ? schema.getEntryOrder("c_z") : -1;
            cache.i_c_ux = schema.exists("c_ux") ? schema.getEntryOrder("c_ux") : -1;
            cache.i_c_uy = schema.exists("c_uy") ? schema.getEntryOrder("c_uy") : -1;
            cache.i_c_uz = schema.exists("c_uz") ? schema.getEntryOrder("c_uz") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_index = cache.i_index;
        i_pindex = cache.i_pindex;
        i_detector = cache.i_detector;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_c_x = cache.i_c_x;
        i_c_y = cache.i_c_y;
        i_c_z = cache.i_c_z;
        i_c_ux = cache.i_c_ux;
        i_c_uy = cache.i_c_uy;
        i_c_uz = cache.i_c_uz;
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
    /// index: index of the cross in the specific detector bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }
    /// pindex: row number in the track bank hit is associated with
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pindex, row));
    }
    /// detector: Detector ID, as defined in org.jlab.detector.base.DetectorType
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// sector: sector of the track
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer of the track
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// c_x: Cross x-position in the lab (in cm)
    [[nodiscard]] float c_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c_x, row));
    }
    /// c_y: Cross y-position in the lab (in cm)
    [[nodiscard]] float c_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c_y, row));
    }
    /// c_z: Cross z-position in the lab (in cm)
    [[nodiscard]] float c_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c_z, row));
    }
    /// c_ux: Cross unit x-direction vector in the lab
    [[nodiscard]] float c_ux(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c_ux, row));
    }
    /// c_uy: Cross unit y-direction vector in the lab
    [[nodiscard]] float c_uy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c_uy, row));
    }
    /// c_uz: Cross unit z-direction vector in the lab
    [[nodiscard]] float c_uz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c_uz, row));
    }
    /// status: hit status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int index_handle() const noexcept { return i_index; }
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_index = -1;
    int i_pindex = -1;
    int i_detector = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_c_x = -1;
    int i_c_y = -1;
    int i_c_z = -1;
    int i_c_ux = -1;
    int i_c_uy = -1;
    int i_c_uz = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for RECHB::TrackCross. Stores a pointer + row index only.
class RECHB_TrackCross::Row {
 public:
    Row(const RECHB_TrackCross* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float c_x() const noexcept { return m_bank->c_x(m_row); }
    [[nodiscard]] float c_y() const noexcept { return m_bank->c_y(m_row); }
    [[nodiscard]] float c_z() const noexcept { return m_bank->c_z(m_row); }
    [[nodiscard]] float c_ux() const noexcept { return m_bank->c_ux(m_row); }
    [[nodiscard]] float c_uy() const noexcept { return m_bank->c_uy(m_row); }
    [[nodiscard]] float c_uz() const noexcept { return m_bank->c_uz(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const RECHB_TrackCross* m_bank;
    int m_row;
};

inline RECHB_TrackCross::Row RECHB_TrackCross::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
