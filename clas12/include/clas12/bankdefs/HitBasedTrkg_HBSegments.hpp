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


/// HIPO bank wrapper for `HitBasedTrkg::HBSegments` (group 20600, item 23).
/// reconstructed segments using DC wire positions
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HitBasedTrkg_HBSegments bank;
/// if (auto sc = bank.bind(event.get_bank("HitBasedTrkg::HBSegments")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HitBasedTrkg_HBSegments {
 public:
    static constexpr std::string_view bank_name = "HitBasedTrkg::HBSegments";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 23;

    class Row;

    HitBasedTrkg_HBSegments() = default;

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
            hipo::column_handle h_status;
            hipo::column_handle h_sector;
            hipo::column_handle h_superlayer;
            hipo::column_handle h_Cluster_ID;
            hipo::column_handle h_Hit1_ID;
            hipo::column_handle h_Hit2_ID;
            hipo::column_handle h_Hit3_ID;
            hipo::column_handle h_Hit4_ID;
            hipo::column_handle h_Hit5_ID;
            hipo::column_handle h_Hit6_ID;
            hipo::column_handle h_Hit7_ID;
            hipo::column_handle h_Hit8_ID;
            hipo::column_handle h_Hit9_ID;
            hipo::column_handle h_Hit10_ID;
            hipo::column_handle h_Hit11_ID;
            hipo::column_handle h_Hit12_ID;
            hipo::column_handle h_avgWire;
            hipo::column_handle h_fitChisqProb;
            hipo::column_handle h_fitSlope;
            hipo::column_handle h_fitSlopeErr;
            hipo::column_handle h_fitInterc;
            hipo::column_handle h_fitIntercErr;
            hipo::column_handle h_SegEndPoint1X;
            hipo::column_handle h_SegEndPoint1Z;
            hipo::column_handle h_SegEndPoint2X;
            hipo::column_handle h_SegEndPoint2Z;
            hipo::column_handle h_size;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("superlayer"); h) cache.h_superlayer = *h;
            else cache.h_superlayer = {};
            if (auto h = schema.get_handle("Cluster_ID"); h) cache.h_Cluster_ID = *h;
            else cache.h_Cluster_ID = {};
            if (auto h = schema.get_handle("Hit1_ID"); h) cache.h_Hit1_ID = *h;
            else cache.h_Hit1_ID = {};
            if (auto h = schema.get_handle("Hit2_ID"); h) cache.h_Hit2_ID = *h;
            else cache.h_Hit2_ID = {};
            if (auto h = schema.get_handle("Hit3_ID"); h) cache.h_Hit3_ID = *h;
            else cache.h_Hit3_ID = {};
            if (auto h = schema.get_handle("Hit4_ID"); h) cache.h_Hit4_ID = *h;
            else cache.h_Hit4_ID = {};
            if (auto h = schema.get_handle("Hit5_ID"); h) cache.h_Hit5_ID = *h;
            else cache.h_Hit5_ID = {};
            if (auto h = schema.get_handle("Hit6_ID"); h) cache.h_Hit6_ID = *h;
            else cache.h_Hit6_ID = {};
            if (auto h = schema.get_handle("Hit7_ID"); h) cache.h_Hit7_ID = *h;
            else cache.h_Hit7_ID = {};
            if (auto h = schema.get_handle("Hit8_ID"); h) cache.h_Hit8_ID = *h;
            else cache.h_Hit8_ID = {};
            if (auto h = schema.get_handle("Hit9_ID"); h) cache.h_Hit9_ID = *h;
            else cache.h_Hit9_ID = {};
            if (auto h = schema.get_handle("Hit10_ID"); h) cache.h_Hit10_ID = *h;
            else cache.h_Hit10_ID = {};
            if (auto h = schema.get_handle("Hit11_ID"); h) cache.h_Hit11_ID = *h;
            else cache.h_Hit11_ID = {};
            if (auto h = schema.get_handle("Hit12_ID"); h) cache.h_Hit12_ID = *h;
            else cache.h_Hit12_ID = {};
            if (auto h = schema.get_handle("avgWire"); h) cache.h_avgWire = *h;
            else cache.h_avgWire = {};
            if (auto h = schema.get_handle("fitChisqProb"); h) cache.h_fitChisqProb = *h;
            else cache.h_fitChisqProb = {};
            if (auto h = schema.get_handle("fitSlope"); h) cache.h_fitSlope = *h;
            else cache.h_fitSlope = {};
            if (auto h = schema.get_handle("fitSlopeErr"); h) cache.h_fitSlopeErr = *h;
            else cache.h_fitSlopeErr = {};
            if (auto h = schema.get_handle("fitInterc"); h) cache.h_fitInterc = *h;
            else cache.h_fitInterc = {};
            if (auto h = schema.get_handle("fitIntercErr"); h) cache.h_fitIntercErr = *h;
            else cache.h_fitIntercErr = {};
            if (auto h = schema.get_handle("SegEndPoint1X"); h) cache.h_SegEndPoint1X = *h;
            else cache.h_SegEndPoint1X = {};
            if (auto h = schema.get_handle("SegEndPoint1Z"); h) cache.h_SegEndPoint1Z = *h;
            else cache.h_SegEndPoint1Z = {};
            if (auto h = schema.get_handle("SegEndPoint2X"); h) cache.h_SegEndPoint2X = *h;
            else cache.h_SegEndPoint2X = {};
            if (auto h = schema.get_handle("SegEndPoint2Z"); h) cache.h_SegEndPoint2Z = *h;
            else cache.h_SegEndPoint2Z = {};
            if (auto h = schema.get_handle("size"); h) cache.h_size = *h;
            else cache.h_size = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_superlayer = cache.h_superlayer;
        h_Cluster_ID = cache.h_Cluster_ID;
        h_Hit1_ID = cache.h_Hit1_ID;
        h_Hit2_ID = cache.h_Hit2_ID;
        h_Hit3_ID = cache.h_Hit3_ID;
        h_Hit4_ID = cache.h_Hit4_ID;
        h_Hit5_ID = cache.h_Hit5_ID;
        h_Hit6_ID = cache.h_Hit6_ID;
        h_Hit7_ID = cache.h_Hit7_ID;
        h_Hit8_ID = cache.h_Hit8_ID;
        h_Hit9_ID = cache.h_Hit9_ID;
        h_Hit10_ID = cache.h_Hit10_ID;
        h_Hit11_ID = cache.h_Hit11_ID;
        h_Hit12_ID = cache.h_Hit12_ID;
        h_avgWire = cache.h_avgWire;
        h_fitChisqProb = cache.h_fitChisqProb;
        h_fitSlope = cache.h_fitSlope;
        h_fitSlopeErr = cache.h_fitSlopeErr;
        h_fitInterc = cache.h_fitInterc;
        h_fitIntercErr = cache.h_fitIntercErr;
        h_SegEndPoint1X = cache.h_SegEndPoint1X;
        h_SegEndPoint1Z = cache.h_SegEndPoint1Z;
        h_SegEndPoint2X = cache.h_SegEndPoint2X;
        h_SegEndPoint2Z = cache.h_SegEndPoint2Z;
        h_size = cache.h_size;
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
    /// id: id of the segment
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: status of the segment
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: sector of the segment
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// superlayer: superlayer of superlayer
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_superlayer, row));
    }
    /// Cluster_ID: associated cluster id
    [[nodiscard]] int Cluster_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster_ID, row));
    }
    /// Hit1_ID: id of hit1 in cluster
    [[nodiscard]] int Hit1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit1_ID, row));
    }
    /// Hit2_ID: id of hit2 in cluster
    [[nodiscard]] int Hit2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit2_ID, row));
    }
    /// Hit3_ID: id of hit3 in cluster
    [[nodiscard]] int Hit3_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit3_ID, row));
    }
    /// Hit4_ID: id of hit4 in cluster
    [[nodiscard]] int Hit4_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit4_ID, row));
    }
    /// Hit5_ID: id of hit5 in cluster
    [[nodiscard]] int Hit5_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit5_ID, row));
    }
    /// Hit6_ID: id of hit6 in cluster
    [[nodiscard]] int Hit6_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit6_ID, row));
    }
    /// Hit7_ID: id of hit7 in cluster
    [[nodiscard]] int Hit7_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit7_ID, row));
    }
    /// Hit8_ID: id of hit8 in cluster
    [[nodiscard]] int Hit8_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit8_ID, row));
    }
    /// Hit9_ID: id of hit9 in cluster
    [[nodiscard]] int Hit9_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit9_ID, row));
    }
    /// Hit10_ID: id of hit10 in cluster
    [[nodiscard]] int Hit10_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit10_ID, row));
    }
    /// Hit11_ID: id of hit11 in cluster
    [[nodiscard]] int Hit11_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit11_ID, row));
    }
    /// Hit12_ID: id of hit12 in cluster
    [[nodiscard]] int Hit12_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit12_ID, row));
    }
    /// avgWire: average wire number
    [[nodiscard]] float avgWire(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgWire, row));
    }
    /// fitChisqProb: fit chisq prob.
    [[nodiscard]] float fitChisqProb(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitChisqProb, row));
    }
    /// fitSlope: line fit slope
    [[nodiscard]] float fitSlope(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitSlope, row));
    }
    /// fitSlopeErr: error on slope
    [[nodiscard]] float fitSlopeErr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitSlopeErr, row));
    }
    /// fitInterc: line fit intercept
    [[nodiscard]] float fitInterc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitInterc, row));
    }
    /// fitIntercErr: error on the intercept
    [[nodiscard]] float fitIntercErr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitIntercErr, row));
    }
    /// SegEndPoint1X: Segment 1st endpoint x coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint1X(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_SegEndPoint1X, row));
    }
    /// SegEndPoint1Z: Segment 1st endpoint z coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint1Z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_SegEndPoint1Z, row));
    }
    /// SegEndPoint2X: Segment 2nd endpoint x coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint2X(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_SegEndPoint2X, row));
    }
    /// SegEndPoint2Z: Segment 2nd endpoint z coordinate in the sector coordinate system (for ced display)
    [[nodiscard]] float SegEndPoint2Z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_SegEndPoint2Z, row));
    }
    /// size: size of segment
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_size, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// superlayer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> superlayer_col() const noexcept {
        return m_view.column<int8_t>(h_superlayer);
    }
    /// Cluster_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster_ID);
    }
    /// Hit1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit1_ID);
    }
    /// Hit2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit2_ID);
    }
    /// Hit3_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit3_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit3_ID);
    }
    /// Hit4_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit4_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit4_ID);
    }
    /// Hit5_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit5_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit5_ID);
    }
    /// Hit6_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit6_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit6_ID);
    }
    /// Hit7_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit7_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit7_ID);
    }
    /// Hit8_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit8_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit8_ID);
    }
    /// Hit9_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit9_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit9_ID);
    }
    /// Hit10_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit10_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit10_ID);
    }
    /// Hit11_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit11_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit11_ID);
    }
    /// Hit12_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit12_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit12_ID);
    }
    /// avgWire column as a contiguous span.
    [[nodiscard]] std::span<const float> avgWire_col() const noexcept {
        return m_view.column<float>(h_avgWire);
    }
    /// fitChisqProb column as a contiguous span.
    [[nodiscard]] std::span<const float> fitChisqProb_col() const noexcept {
        return m_view.column<float>(h_fitChisqProb);
    }
    /// fitSlope column as a contiguous span.
    [[nodiscard]] std::span<const float> fitSlope_col() const noexcept {
        return m_view.column<float>(h_fitSlope);
    }
    /// fitSlopeErr column as a contiguous span.
    [[nodiscard]] std::span<const float> fitSlopeErr_col() const noexcept {
        return m_view.column<float>(h_fitSlopeErr);
    }
    /// fitInterc column as a contiguous span.
    [[nodiscard]] std::span<const float> fitInterc_col() const noexcept {
        return m_view.column<float>(h_fitInterc);
    }
    /// fitIntercErr column as a contiguous span.
    [[nodiscard]] std::span<const float> fitIntercErr_col() const noexcept {
        return m_view.column<float>(h_fitIntercErr);
    }
    /// SegEndPoint1X column as a contiguous span.
    [[nodiscard]] std::span<const float> SegEndPoint1X_col() const noexcept {
        return m_view.column<float>(h_SegEndPoint1X);
    }
    /// SegEndPoint1Z column as a contiguous span.
    [[nodiscard]] std::span<const float> SegEndPoint1Z_col() const noexcept {
        return m_view.column<float>(h_SegEndPoint1Z);
    }
    /// SegEndPoint2X column as a contiguous span.
    [[nodiscard]] std::span<const float> SegEndPoint2X_col() const noexcept {
        return m_view.column<float>(h_SegEndPoint2X);
    }
    /// SegEndPoint2Z column as a contiguous span.
    [[nodiscard]] std::span<const float> SegEndPoint2Z_col() const noexcept {
        return m_view.column<float>(h_SegEndPoint2Z);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> size_col() const noexcept {
        return m_view.column<int8_t>(h_size);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_superlayer{};
    hipo::column_handle h_Cluster_ID{};
    hipo::column_handle h_Hit1_ID{};
    hipo::column_handle h_Hit2_ID{};
    hipo::column_handle h_Hit3_ID{};
    hipo::column_handle h_Hit4_ID{};
    hipo::column_handle h_Hit5_ID{};
    hipo::column_handle h_Hit6_ID{};
    hipo::column_handle h_Hit7_ID{};
    hipo::column_handle h_Hit8_ID{};
    hipo::column_handle h_Hit9_ID{};
    hipo::column_handle h_Hit10_ID{};
    hipo::column_handle h_Hit11_ID{};
    hipo::column_handle h_Hit12_ID{};
    hipo::column_handle h_avgWire{};
    hipo::column_handle h_fitChisqProb{};
    hipo::column_handle h_fitSlope{};
    hipo::column_handle h_fitSlopeErr{};
    hipo::column_handle h_fitInterc{};
    hipo::column_handle h_fitIntercErr{};
    hipo::column_handle h_SegEndPoint1X{};
    hipo::column_handle h_SegEndPoint1Z{};
    hipo::column_handle h_SegEndPoint2X{};
    hipo::column_handle h_SegEndPoint2Z{};
    hipo::column_handle h_size{};

    friend class Row;
};

/// Lightweight row view for HitBasedTrkg::HBSegments. Stores a pointer + row index only.
class HitBasedTrkg_HBSegments::Row {
 public:
    Row(const HitBasedTrkg_HBSegments* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
    const HitBasedTrkg_HBSegments* m_bank;
    int m_row;
};

inline HitBasedTrkg_HBSegments::Row HitBasedTrkg_HBSegments::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
