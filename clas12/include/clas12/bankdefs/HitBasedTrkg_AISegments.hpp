// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `HitBasedTrkg::AISegments` (group 20600, item 63).
/// reconstructed segments using DC wire positions
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// HitBasedTrkg_AISegments bank;
/// if (auto sc = bank.bind(event.get_bank("HitBasedTrkg::AISegments")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HitBasedTrkg_AISegments {
 public:
    static constexpr std::string_view bank_name = "HitBasedTrkg::AISegments";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 63;

    class Row;

    HitBasedTrkg_AISegments() = default;

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
            int i_status = -1;
            int i_sector = -1;
            int i_superlayer = -1;
            int i_Cluster_ID = -1;
            int i_Hit1_ID = -1;
            int i_Hit2_ID = -1;
            int i_Hit3_ID = -1;
            int i_Hit4_ID = -1;
            int i_Hit5_ID = -1;
            int i_Hit6_ID = -1;
            int i_Hit7_ID = -1;
            int i_Hit8_ID = -1;
            int i_Hit9_ID = -1;
            int i_Hit10_ID = -1;
            int i_Hit11_ID = -1;
            int i_Hit12_ID = -1;
            int i_avgWire = -1;
            int i_fitChisqProb = -1;
            int i_fitSlope = -1;
            int i_fitSlopeErr = -1;
            int i_fitInterc = -1;
            int i_fitIntercErr = -1;
            int i_SegEndPoint1X = -1;
            int i_SegEndPoint1Z = -1;
            int i_SegEndPoint2X = -1;
            int i_SegEndPoint2Z = -1;
            int i_size = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_superlayer = schema.exists("superlayer") ? schema.getEntryOrder("superlayer") : -1;
            cache.i_Cluster_ID = schema.exists("Cluster_ID") ? schema.getEntryOrder("Cluster_ID") : -1;
            cache.i_Hit1_ID = schema.exists("Hit1_ID") ? schema.getEntryOrder("Hit1_ID") : -1;
            cache.i_Hit2_ID = schema.exists("Hit2_ID") ? schema.getEntryOrder("Hit2_ID") : -1;
            cache.i_Hit3_ID = schema.exists("Hit3_ID") ? schema.getEntryOrder("Hit3_ID") : -1;
            cache.i_Hit4_ID = schema.exists("Hit4_ID") ? schema.getEntryOrder("Hit4_ID") : -1;
            cache.i_Hit5_ID = schema.exists("Hit5_ID") ? schema.getEntryOrder("Hit5_ID") : -1;
            cache.i_Hit6_ID = schema.exists("Hit6_ID") ? schema.getEntryOrder("Hit6_ID") : -1;
            cache.i_Hit7_ID = schema.exists("Hit7_ID") ? schema.getEntryOrder("Hit7_ID") : -1;
            cache.i_Hit8_ID = schema.exists("Hit8_ID") ? schema.getEntryOrder("Hit8_ID") : -1;
            cache.i_Hit9_ID = schema.exists("Hit9_ID") ? schema.getEntryOrder("Hit9_ID") : -1;
            cache.i_Hit10_ID = schema.exists("Hit10_ID") ? schema.getEntryOrder("Hit10_ID") : -1;
            cache.i_Hit11_ID = schema.exists("Hit11_ID") ? schema.getEntryOrder("Hit11_ID") : -1;
            cache.i_Hit12_ID = schema.exists("Hit12_ID") ? schema.getEntryOrder("Hit12_ID") : -1;
            cache.i_avgWire = schema.exists("avgWire") ? schema.getEntryOrder("avgWire") : -1;
            cache.i_fitChisqProb = schema.exists("fitChisqProb") ? schema.getEntryOrder("fitChisqProb") : -1;
            cache.i_fitSlope = schema.exists("fitSlope") ? schema.getEntryOrder("fitSlope") : -1;
            cache.i_fitSlopeErr = schema.exists("fitSlopeErr") ? schema.getEntryOrder("fitSlopeErr") : -1;
            cache.i_fitInterc = schema.exists("fitInterc") ? schema.getEntryOrder("fitInterc") : -1;
            cache.i_fitIntercErr = schema.exists("fitIntercErr") ? schema.getEntryOrder("fitIntercErr") : -1;
            cache.i_SegEndPoint1X = schema.exists("SegEndPoint1X") ? schema.getEntryOrder("SegEndPoint1X") : -1;
            cache.i_SegEndPoint1Z = schema.exists("SegEndPoint1Z") ? schema.getEntryOrder("SegEndPoint1Z") : -1;
            cache.i_SegEndPoint2X = schema.exists("SegEndPoint2X") ? schema.getEntryOrder("SegEndPoint2X") : -1;
            cache.i_SegEndPoint2Z = schema.exists("SegEndPoint2Z") ? schema.getEntryOrder("SegEndPoint2Z") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_superlayer = cache.i_superlayer;
        i_Cluster_ID = cache.i_Cluster_ID;
        i_Hit1_ID = cache.i_Hit1_ID;
        i_Hit2_ID = cache.i_Hit2_ID;
        i_Hit3_ID = cache.i_Hit3_ID;
        i_Hit4_ID = cache.i_Hit4_ID;
        i_Hit5_ID = cache.i_Hit5_ID;
        i_Hit6_ID = cache.i_Hit6_ID;
        i_Hit7_ID = cache.i_Hit7_ID;
        i_Hit8_ID = cache.i_Hit8_ID;
        i_Hit9_ID = cache.i_Hit9_ID;
        i_Hit10_ID = cache.i_Hit10_ID;
        i_Hit11_ID = cache.i_Hit11_ID;
        i_Hit12_ID = cache.i_Hit12_ID;
        i_avgWire = cache.i_avgWire;
        i_fitChisqProb = cache.i_fitChisqProb;
        i_fitSlope = cache.i_fitSlope;
        i_fitSlopeErr = cache.i_fitSlopeErr;
        i_fitInterc = cache.i_fitInterc;
        i_fitIntercErr = cache.i_fitIntercErr;
        i_SegEndPoint1X = cache.i_SegEndPoint1X;
        i_SegEndPoint1Z = cache.i_SegEndPoint1Z;
        i_SegEndPoint2X = cache.i_SegEndPoint2X;
        i_SegEndPoint2Z = cache.i_SegEndPoint2Z;
        i_size = cache.i_size;
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
    /// id: id of the segment
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// status: status of the segment
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// sector: sector of the segment
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// superlayer: superlayer of superlayer
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_superlayer, row));
    }
    /// Cluster_ID: associated cluster id
    [[nodiscard]] int Cluster_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster_ID, row));
    }
    /// Hit1_ID: id of hit1 in cluster
    [[nodiscard]] int Hit1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit1_ID, row));
    }
    /// Hit2_ID: id of hit2 in cluster
    [[nodiscard]] int Hit2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit2_ID, row));
    }
    /// Hit3_ID: id of hit3 in cluster
    [[nodiscard]] int Hit3_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit3_ID, row));
    }
    /// Hit4_ID: id of hit4 in cluster
    [[nodiscard]] int Hit4_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit4_ID, row));
    }
    /// Hit5_ID: id of hit5 in cluster
    [[nodiscard]] int Hit5_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit5_ID, row));
    }
    /// Hit6_ID: id of hit6 in cluster
    [[nodiscard]] int Hit6_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit6_ID, row));
    }
    /// Hit7_ID: id of hit7 in cluster
    [[nodiscard]] int Hit7_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit7_ID, row));
    }
    /// Hit8_ID: id of hit8 in cluster
    [[nodiscard]] int Hit8_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit8_ID, row));
    }
    /// Hit9_ID: id of hit9 in cluster
    [[nodiscard]] int Hit9_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit9_ID, row));
    }
    /// Hit10_ID: id of hit10 in cluster
    [[nodiscard]] int Hit10_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit10_ID, row));
    }
    /// Hit11_ID: id of hit11 in cluster
    [[nodiscard]] int Hit11_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit11_ID, row));
    }
    /// Hit12_ID: id of hit12 in cluster
    [[nodiscard]] int Hit12_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit12_ID, row));
    }
    /// avgWire: average wire number
    [[nodiscard]] float avgWire(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgWire, row));
    }
    /// fitChisqProb: fit chisq prob.
    [[nodiscard]] float fitChisqProb(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fitChisqProb, row));
    }
    /// fitSlope: line fit slope
    [[nodiscard]] float fitSlope(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fitSlope, row));
    }
    /// fitSlopeErr: error on slope
    [[nodiscard]] float fitSlopeErr(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fitSlopeErr, row));
    }
    /// fitInterc: line fit intercept
    [[nodiscard]] float fitInterc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fitInterc, row));
    }
    /// fitIntercErr: error on the intercept
    [[nodiscard]] float fitIntercErr(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fitIntercErr, row));
    }
    /// SegEndPoint1X: Segment 1st endpoint x coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint1X(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_SegEndPoint1X, row));
    }
    /// SegEndPoint1Z: Segment 1st endpoint z coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint1Z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_SegEndPoint1Z, row));
    }
    /// SegEndPoint2X: Segment 2nd endpoint x coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint2X(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_SegEndPoint2X, row));
    }
    /// SegEndPoint2Z: Segment 2nd endpoint z coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint2Z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_SegEndPoint2Z, row));
    }
    /// size: size of segment
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_size, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_superlayer = -1;
    int i_Cluster_ID = -1;
    int i_Hit1_ID = -1;
    int i_Hit2_ID = -1;
    int i_Hit3_ID = -1;
    int i_Hit4_ID = -1;
    int i_Hit5_ID = -1;
    int i_Hit6_ID = -1;
    int i_Hit7_ID = -1;
    int i_Hit8_ID = -1;
    int i_Hit9_ID = -1;
    int i_Hit10_ID = -1;
    int i_Hit11_ID = -1;
    int i_Hit12_ID = -1;
    int i_avgWire = -1;
    int i_fitChisqProb = -1;
    int i_fitSlope = -1;
    int i_fitSlopeErr = -1;
    int i_fitInterc = -1;
    int i_fitIntercErr = -1;
    int i_SegEndPoint1X = -1;
    int i_SegEndPoint1Z = -1;
    int i_SegEndPoint2X = -1;
    int i_SegEndPoint2Z = -1;
    int i_size = -1;

    friend class Row;
};

/// Lightweight row view for HitBasedTrkg::AISegments. Stores a pointer + row index only.
class HitBasedTrkg_AISegments::Row {
 public:
    Row(const HitBasedTrkg_AISegments* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int Cluster_ID() const noexcept { return m_bank->Cluster_ID(m_row); }
    [[nodiscard]] int Hit1_ID() const noexcept { return m_bank->Hit1_ID(m_row); }
    [[nodiscard]] int Hit2_ID() const noexcept { return m_bank->Hit2_ID(m_row); }
    [[nodiscard]] int Hit3_ID() const noexcept { return m_bank->Hit3_ID(m_row); }
    [[nodiscard]] int Hit4_ID() const noexcept { return m_bank->Hit4_ID(m_row); }
    [[nodiscard]] int Hit5_ID() const noexcept { return m_bank->Hit5_ID(m_row); }
    [[nodiscard]] int Hit6_ID() const noexcept { return m_bank->Hit6_ID(m_row); }
    [[nodiscard]] int Hit7_ID() const noexcept { return m_bank->Hit7_ID(m_row); }
    [[nodiscard]] int Hit8_ID() const noexcept { return m_bank->Hit8_ID(m_row); }
    [[nodiscard]] int Hit9_ID() const noexcept { return m_bank->Hit9_ID(m_row); }
    [[nodiscard]] int Hit10_ID() const noexcept { return m_bank->Hit10_ID(m_row); }
    [[nodiscard]] int Hit11_ID() const noexcept { return m_bank->Hit11_ID(m_row); }
    [[nodiscard]] int Hit12_ID() const noexcept { return m_bank->Hit12_ID(m_row); }
    [[nodiscard]] float avgWire() const noexcept { return m_bank->avgWire(m_row); }
    [[nodiscard]] float fitChisqProb() const noexcept { return m_bank->fitChisqProb(m_row); }
    [[nodiscard]] float fitSlope() const noexcept { return m_bank->fitSlope(m_row); }
    [[nodiscard]] float fitSlopeErr() const noexcept { return m_bank->fitSlopeErr(m_row); }
    [[nodiscard]] float fitInterc() const noexcept { return m_bank->fitInterc(m_row); }
    [[nodiscard]] float fitIntercErr() const noexcept { return m_bank->fitIntercErr(m_row); }
    [[nodiscard]] float SegEndPoint1X() const noexcept { return m_bank->SegEndPoint1X(m_row); }
    [[nodiscard]] float SegEndPoint1Z() const noexcept { return m_bank->SegEndPoint1Z(m_row); }
    [[nodiscard]] float SegEndPoint2X() const noexcept { return m_bank->SegEndPoint2X(m_row); }
    [[nodiscard]] float SegEndPoint2Z() const noexcept { return m_bank->SegEndPoint2Z(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
 private:
    const HitBasedTrkg_AISegments* m_bank;
    int m_row;
};

inline HitBasedTrkg_AISegments::Row HitBasedTrkg_AISegments::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
