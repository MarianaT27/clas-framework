// =============================================================================
// Auto-generated from dc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `TimeBasedTrkg::TBSegmentTrajectory` (group 20600, item 34).
/// reconstructed segment trajectory from hit-based tracking
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// TimeBasedTrkg_TBSegmentTrajectory bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::TBSegmentTrajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto segmentID = row.segmentID();
/// }
/// @endcode
class TimeBasedTrkg_TBSegmentTrajectory {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::TBSegmentTrajectory";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 34;

    class Row;

    TimeBasedTrkg_TBSegmentTrajectory() = default;

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
            int i_segmentID = -1;
            int i_sector = -1;
            int i_superlayer = -1;
            int i_layer = -1;
            int i_matchedHitID = -1;
            int i_trkDoca = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_segmentID = schema.exists("segmentID") ? schema.getEntryOrder("segmentID") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_superlayer = schema.exists("superlayer") ? schema.getEntryOrder("superlayer") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_matchedHitID = schema.exists("matchedHitID") ? schema.getEntryOrder("matchedHitID") : -1;
            cache.i_trkDoca = schema.exists("trkDoca") ? schema.getEntryOrder("trkDoca") : -1;
        }
        i_segmentID = cache.i_segmentID;
        i_sector = cache.i_sector;
        i_superlayer = cache.i_superlayer;
        i_layer = cache.i_layer;
        i_matchedHitID = cache.i_matchedHitID;
        i_trkDoca = cache.i_trkDoca;
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
    /// segmentID: id of the segment
    [[nodiscard]] int segmentID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_segmentID, row));
    }
    /// sector: sector of DC
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// superlayer: superlayer
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_superlayer, row));
    }
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// matchedHitID: matched hit id
    [[nodiscard]] int matchedHitID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_matchedHitID, row));
    }
    /// trkDoca: calculated track doca
    [[nodiscard]] float trkDoca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trkDoca, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_segmentID = -1;
    int i_sector = -1;
    int i_superlayer = -1;
    int i_layer = -1;
    int i_matchedHitID = -1;
    int i_trkDoca = -1;

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::TBSegmentTrajectory. Stores a pointer + row index only.
class TimeBasedTrkg_TBSegmentTrajectory::Row {
 public:
    Row(const TimeBasedTrkg_TBSegmentTrajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int segmentID() const noexcept { return m_bank->segmentID(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int matchedHitID() const noexcept { return m_bank->matchedHitID(m_row); }
    [[nodiscard]] float trkDoca() const noexcept { return m_bank->trkDoca(m_row); }
 private:
    const TimeBasedTrkg_TBSegmentTrajectory* m_bank;
    int m_row;
};

inline TimeBasedTrkg_TBSegmentTrajectory::Row TimeBasedTrkg_TBSegmentTrajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
