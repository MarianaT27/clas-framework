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

namespace bankdefs {


/// HIPO bank wrapper for `TimeBasedTrkg::TBHits` (group 20600, item 31).
/// reconstructed hits using DC timing information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// TimeBasedTrkg_TBHits bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::TBHits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_TBHits {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::TBHits";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 31;

    class Row;

    TimeBasedTrkg_TBHits() = default;

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
            hipo::column_handle h_layer;
            hipo::column_handle h_wire;
            hipo::column_handle h_TDC;
            hipo::column_handle h_jitter;
            hipo::column_handle h_doca;
            hipo::column_handle h_docaError;
            hipo::column_handle h_trkDoca;
            hipo::column_handle h_timeResidual;
            hipo::column_handle h_fitResidual;
            hipo::column_handle h_DAFWeight;
            hipo::column_handle h_LR;
            hipo::column_handle h_X;
            hipo::column_handle h_Z;
            hipo::column_handle h_B;
            hipo::column_handle h_Alpha;
            hipo::column_handle h_TProp;
            hipo::column_handle h_TFlight;
            hipo::column_handle h_T0;
            hipo::column_handle h_TStart;
            hipo::column_handle h_dDoca;
            hipo::column_handle h_clusterID;
            hipo::column_handle h_trkID;
            hipo::column_handle h_time;
            hipo::column_handle h_beta;
            hipo::column_handle h_tBeta;
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
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("wire"); h) cache.h_wire = *h;
            else cache.h_wire = {};
            if (auto h = schema.get_handle("TDC"); h) cache.h_TDC = *h;
            else cache.h_TDC = {};
            if (auto h = schema.get_handle("jitter"); h) cache.h_jitter = *h;
            else cache.h_jitter = {};
            if (auto h = schema.get_handle("doca"); h) cache.h_doca = *h;
            else cache.h_doca = {};
            if (auto h = schema.get_handle("docaError"); h) cache.h_docaError = *h;
            else cache.h_docaError = {};
            if (auto h = schema.get_handle("trkDoca"); h) cache.h_trkDoca = *h;
            else cache.h_trkDoca = {};
            if (auto h = schema.get_handle("timeResidual"); h) cache.h_timeResidual = *h;
            else cache.h_timeResidual = {};
            if (auto h = schema.get_handle("fitResidual"); h) cache.h_fitResidual = *h;
            else cache.h_fitResidual = {};
            if (auto h = schema.get_handle("DAFWeight"); h) cache.h_DAFWeight = *h;
            else cache.h_DAFWeight = {};
            if (auto h = schema.get_handle("LR"); h) cache.h_LR = *h;
            else cache.h_LR = {};
            if (auto h = schema.get_handle("X"); h) cache.h_X = *h;
            else cache.h_X = {};
            if (auto h = schema.get_handle("Z"); h) cache.h_Z = *h;
            else cache.h_Z = {};
            if (auto h = schema.get_handle("B"); h) cache.h_B = *h;
            else cache.h_B = {};
            if (auto h = schema.get_handle("Alpha"); h) cache.h_Alpha = *h;
            else cache.h_Alpha = {};
            if (auto h = schema.get_handle("TProp"); h) cache.h_TProp = *h;
            else cache.h_TProp = {};
            if (auto h = schema.get_handle("TFlight"); h) cache.h_TFlight = *h;
            else cache.h_TFlight = {};
            if (auto h = schema.get_handle("T0"); h) cache.h_T0 = *h;
            else cache.h_T0 = {};
            if (auto h = schema.get_handle("TStart"); h) cache.h_TStart = *h;
            else cache.h_TStart = {};
            if (auto h = schema.get_handle("dDoca"); h) cache.h_dDoca = *h;
            else cache.h_dDoca = {};
            if (auto h = schema.get_handle("clusterID"); h) cache.h_clusterID = *h;
            else cache.h_clusterID = {};
            if (auto h = schema.get_handle("trkID"); h) cache.h_trkID = *h;
            else cache.h_trkID = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("beta"); h) cache.h_beta = *h;
            else cache.h_beta = {};
            if (auto h = schema.get_handle("tBeta"); h) cache.h_tBeta = *h;
            else cache.h_tBeta = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_superlayer = cache.h_superlayer;
        h_layer = cache.h_layer;
        h_wire = cache.h_wire;
        h_TDC = cache.h_TDC;
        h_jitter = cache.h_jitter;
        h_doca = cache.h_doca;
        h_docaError = cache.h_docaError;
        h_trkDoca = cache.h_trkDoca;
        h_timeResidual = cache.h_timeResidual;
        h_fitResidual = cache.h_fitResidual;
        h_DAFWeight = cache.h_DAFWeight;
        h_LR = cache.h_LR;
        h_X = cache.h_X;
        h_Z = cache.h_Z;
        h_B = cache.h_B;
        h_Alpha = cache.h_Alpha;
        h_TProp = cache.h_TProp;
        h_TFlight = cache.h_TFlight;
        h_T0 = cache.h_T0;
        h_TStart = cache.h_TStart;
        h_dDoca = cache.h_dDoca;
        h_clusterID = cache.h_clusterID;
        h_trkID = cache.h_trkID;
        h_time = cache.h_time;
        h_beta = cache.h_beta;
        h_tBeta = cache.h_tBeta;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: id of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: DC sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// superlayer: DC superlayer (1...6)
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_superlayer, row));
    }
    /// layer: DC layer in superlayer (1...6)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// wire: wire id of DC
    [[nodiscard]] int wire(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_wire, row));
    }
    /// TDC: raw time of the hit
    [[nodiscard]] int TDC(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_TDC, row));
    }
    /// jitter: time jitter (ns)
    [[nodiscard]] int jitter(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_jitter, row));
    }
    /// doca: doca of the hit calculated from TDC (in cm)
    [[nodiscard]] float doca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_doca, row));
    }
    /// docaError: uncertainty on doca of the hit calculated from TDC (in cm)
    [[nodiscard]] float docaError(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_docaError, row));
    }
    /// trkDoca: track doca of the hit (in cm)
    [[nodiscard]] float trkDoca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trkDoca, row));
    }
    /// timeResidual: time residual of the hit (in cm)
    [[nodiscard]] float timeResidual(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_timeResidual, row));
    }
    /// fitResidual: fit residual of the hit (in cm, from KF)
    [[nodiscard]] float fitResidual(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitResidual, row));
    }
    /// DAFWeight: Weight by DAF; the 1st bit of status indicates that a hit is belong to single or double
    [[nodiscard]] float DAFWeight(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_DAFWeight, row));
    }
    /// LR: Left/Right ambiguity of the hit
    [[nodiscard]] int LR(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_LR, row));
    }
    /// X: wire x-coordinate  in tilted-sector
    [[nodiscard]] float X(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_X, row));
    }
    /// Z: wire z-coordinate  in tilted-sector
    [[nodiscard]] float Z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Z, row));
    }
    /// B: B-field intensity at hit position in tilted-sector system
    [[nodiscard]] float B(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_B, row));
    }
    /// Alpha: local angle (degr.) in tilted-sector system
    [[nodiscard]] float Alpha(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Alpha, row));
    }
    /// TProp: t propagation along the wire (ns)
    [[nodiscard]] float TProp(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_TProp, row));
    }
    /// TFlight: time of flight correction (ns)
    [[nodiscard]] float TFlight(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_TFlight, row));
    }
    /// T0: T0 (ns)
    [[nodiscard]] float T0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_T0, row));
    }
    /// TStart: event start time used (ns)
    [[nodiscard]] float TStart(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_TStart, row));
    }
    /// dDoca: delta Doca correction (cm)
    [[nodiscard]] float dDoca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dDoca, row));
    }
    /// clusterID: ID of associated cluster
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusterID, row));
    }
    /// trkID: ID of associated track
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_trkID, row));
    }
    /// time: time used in tracking (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// beta: beta used in tracking
    [[nodiscard]] float beta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_beta, row));
    }
    /// tBeta: beta-dependent time correction used in tracking
    [[nodiscard]] float tBeta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tBeta, row));
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
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// wire column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> wire_col() const noexcept {
        return m_view.column<int16_t>(h_wire);
    }
    /// TDC column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> TDC_col() const noexcept {
        return m_view.column<int32_t>(h_TDC);
    }
    /// jitter column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> jitter_col() const noexcept {
        return m_view.column<int8_t>(h_jitter);
    }
    /// doca column as a contiguous span.
    [[nodiscard]] std::span<const float> doca_col() const noexcept {
        return m_view.column<float>(h_doca);
    }
    /// docaError column as a contiguous span.
    [[nodiscard]] std::span<const float> docaError_col() const noexcept {
        return m_view.column<float>(h_docaError);
    }
    /// trkDoca column as a contiguous span.
    [[nodiscard]] std::span<const float> trkDoca_col() const noexcept {
        return m_view.column<float>(h_trkDoca);
    }
    /// timeResidual column as a contiguous span.
    [[nodiscard]] std::span<const float> timeResidual_col() const noexcept {
        return m_view.column<float>(h_timeResidual);
    }
    /// fitResidual column as a contiguous span.
    [[nodiscard]] std::span<const float> fitResidual_col() const noexcept {
        return m_view.column<float>(h_fitResidual);
    }
    /// DAFWeight column as a contiguous span.
    [[nodiscard]] std::span<const float> DAFWeight_col() const noexcept {
        return m_view.column<float>(h_DAFWeight);
    }
    /// LR column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> LR_col() const noexcept {
        return m_view.column<int8_t>(h_LR);
    }
    /// X column as a contiguous span.
    [[nodiscard]] std::span<const float> X_col() const noexcept {
        return m_view.column<float>(h_X);
    }
    /// Z column as a contiguous span.
    [[nodiscard]] std::span<const float> Z_col() const noexcept {
        return m_view.column<float>(h_Z);
    }
    /// B column as a contiguous span.
    [[nodiscard]] std::span<const float> B_col() const noexcept {
        return m_view.column<float>(h_B);
    }
    /// Alpha column as a contiguous span.
    [[nodiscard]] std::span<const float> Alpha_col() const noexcept {
        return m_view.column<float>(h_Alpha);
    }
    /// TProp column as a contiguous span.
    [[nodiscard]] std::span<const float> TProp_col() const noexcept {
        return m_view.column<float>(h_TProp);
    }
    /// TFlight column as a contiguous span.
    [[nodiscard]] std::span<const float> TFlight_col() const noexcept {
        return m_view.column<float>(h_TFlight);
    }
    /// T0 column as a contiguous span.
    [[nodiscard]] std::span<const float> T0_col() const noexcept {
        return m_view.column<float>(h_T0);
    }
    /// TStart column as a contiguous span.
    [[nodiscard]] std::span<const float> TStart_col() const noexcept {
        return m_view.column<float>(h_TStart);
    }
    /// dDoca column as a contiguous span.
    [[nodiscard]] std::span<const float> dDoca_col() const noexcept {
        return m_view.column<float>(h_dDoca);
    }
    /// clusterID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusterID_col() const noexcept {
        return m_view.column<int16_t>(h_clusterID);
    }
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> trkID_col() const noexcept {
        return m_view.column<int8_t>(h_trkID);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// beta column as a contiguous span.
    [[nodiscard]] std::span<const float> beta_col() const noexcept {
        return m_view.column<float>(h_beta);
    }
    /// tBeta column as a contiguous span.
    [[nodiscard]] std::span<const float> tBeta_col() const noexcept {
        return m_view.column<float>(h_tBeta);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_superlayer{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_wire{};
    hipo::column_handle h_TDC{};
    hipo::column_handle h_jitter{};
    hipo::column_handle h_doca{};
    hipo::column_handle h_docaError{};
    hipo::column_handle h_trkDoca{};
    hipo::column_handle h_timeResidual{};
    hipo::column_handle h_fitResidual{};
    hipo::column_handle h_DAFWeight{};
    hipo::column_handle h_LR{};
    hipo::column_handle h_X{};
    hipo::column_handle h_Z{};
    hipo::column_handle h_B{};
    hipo::column_handle h_Alpha{};
    hipo::column_handle h_TProp{};
    hipo::column_handle h_TFlight{};
    hipo::column_handle h_T0{};
    hipo::column_handle h_TStart{};
    hipo::column_handle h_dDoca{};
    hipo::column_handle h_clusterID{};
    hipo::column_handle h_trkID{};
    hipo::column_handle h_time{};
    hipo::column_handle h_beta{};
    hipo::column_handle h_tBeta{};

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::TBHits. Stores a pointer + row index only.
class TimeBasedTrkg_TBHits::Row {
 public:
    Row(const TimeBasedTrkg_TBHits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int wire() const noexcept { return m_bank->wire(m_row); }
    [[nodiscard]] int TDC() const noexcept { return m_bank->TDC(m_row); }
    [[nodiscard]] int jitter() const noexcept { return m_bank->jitter(m_row); }
    [[nodiscard]] float doca() const noexcept { return m_bank->doca(m_row); }
    [[nodiscard]] float docaError() const noexcept { return m_bank->docaError(m_row); }
    [[nodiscard]] float trkDoca() const noexcept { return m_bank->trkDoca(m_row); }
    [[nodiscard]] float timeResidual() const noexcept { return m_bank->timeResidual(m_row); }
    [[nodiscard]] float fitResidual() const noexcept { return m_bank->fitResidual(m_row); }
    [[nodiscard]] float DAFWeight() const noexcept { return m_bank->DAFWeight(m_row); }
    [[nodiscard]] int LR() const noexcept { return m_bank->LR(m_row); }
    [[nodiscard]] float X() const noexcept { return m_bank->X(m_row); }
    [[nodiscard]] float Z() const noexcept { return m_bank->Z(m_row); }
    [[nodiscard]] float B() const noexcept { return m_bank->B(m_row); }
    [[nodiscard]] float Alpha() const noexcept { return m_bank->Alpha(m_row); }
    [[nodiscard]] float TProp() const noexcept { return m_bank->TProp(m_row); }
    [[nodiscard]] float TFlight() const noexcept { return m_bank->TFlight(m_row); }
    [[nodiscard]] float T0() const noexcept { return m_bank->T0(m_row); }
    [[nodiscard]] float TStart() const noexcept { return m_bank->TStart(m_row); }
    [[nodiscard]] float dDoca() const noexcept { return m_bank->dDoca(m_row); }
    [[nodiscard]] int clusterID() const noexcept { return m_bank->clusterID(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float beta() const noexcept { return m_bank->beta(m_row); }
    [[nodiscard]] float tBeta() const noexcept { return m_bank->tBeta(m_row); }
 private:
    const TimeBasedTrkg_TBHits* m_bank;
    int m_row;
};

inline TimeBasedTrkg_TBHits::Row TimeBasedTrkg_TBHits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
