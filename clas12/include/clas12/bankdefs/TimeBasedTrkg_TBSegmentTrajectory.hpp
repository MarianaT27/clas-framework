// =============================================================================
// Auto-generated from dc.json - do not edit by hand.
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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `TimeBasedTrkg::TBSegmentTrajectory` (group 20600, item 34).
/// reconstructed segment trajectory from hit-based tracking
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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
            hipo::column_handle h_segmentID;
            hipo::column_handle h_sector;
            hipo::column_handle h_superlayer;
            hipo::column_handle h_layer;
            hipo::column_handle h_matchedHitID;
            hipo::column_handle h_trkDoca;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("segmentID"); h) cache.h_segmentID = *h;
            else cache.h_segmentID = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("superlayer"); h) cache.h_superlayer = *h;
            else cache.h_superlayer = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("matchedHitID"); h) cache.h_matchedHitID = *h;
            else cache.h_matchedHitID = {};
            if (auto h = schema.get_handle("trkDoca"); h) cache.h_trkDoca = *h;
            else cache.h_trkDoca = {};
        }
        h_segmentID = cache.h_segmentID;
        h_sector = cache.h_sector;
        h_superlayer = cache.h_superlayer;
        h_layer = cache.h_layer;
        h_matchedHitID = cache.h_matchedHitID;
        h_trkDoca = cache.h_trkDoca;
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
    /// segmentID: id of the segment
    [[nodiscard]] int segmentID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_segmentID, row));
    }
    /// sector: sector of DC
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// superlayer: superlayer
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_superlayer, row));
    }
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// matchedHitID: matched hit id
    [[nodiscard]] int matchedHitID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_matchedHitID, row));
    }
    /// trkDoca: calculated track doca
    [[nodiscard]] float trkDoca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trkDoca, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// segmentID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> segmentID_col() const noexcept {
        return m_view.column<int16_t>(h_segmentID);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// superlayer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> superlayer_col() const noexcept {
        return m_view.column<int8_t>(h_superlayer);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// matchedHitID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> matchedHitID_col() const noexcept {
        return m_view.column<int16_t>(h_matchedHitID);
    }
    /// trkDoca column as a contiguous span.
    [[nodiscard]] std::span<const float> trkDoca_col() const noexcept {
        return m_view.column<float>(h_trkDoca);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_segmentID{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_superlayer{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_matchedHitID{};
    hipo::column_handle h_trkDoca{};

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

}  // namespace clas12::bankdefs
