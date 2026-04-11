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

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("superlayer"); h) h_superlayer = *h;
        else h_superlayer = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("wire"); h) h_wire = *h;
        else h_wire = {};
        if (auto h = schema.get_handle("TDC"); h) h_TDC = *h;
        else h_TDC = {};
        if (auto h = schema.get_handle("jitter"); h) h_jitter = *h;
        else h_jitter = {};
        if (auto h = schema.get_handle("doca"); h) h_doca = *h;
        else h_doca = {};
        if (auto h = schema.get_handle("docaError"); h) h_docaError = *h;
        else h_docaError = {};
        if (auto h = schema.get_handle("trkDoca"); h) h_trkDoca = *h;
        else h_trkDoca = {};
        if (auto h = schema.get_handle("timeResidual"); h) h_timeResidual = *h;
        else h_timeResidual = {};
        if (auto h = schema.get_handle("fitResidual"); h) h_fitResidual = *h;
        else h_fitResidual = {};
        if (auto h = schema.get_handle("DAFWeight"); h) h_DAFWeight = *h;
        else h_DAFWeight = {};
        if (auto h = schema.get_handle("LR"); h) h_LR = *h;
        else h_LR = {};
        if (auto h = schema.get_handle("X"); h) h_X = *h;
        else h_X = {};
        if (auto h = schema.get_handle("Z"); h) h_Z = *h;
        else h_Z = {};
        if (auto h = schema.get_handle("B"); h) h_B = *h;
        else h_B = {};
        if (auto h = schema.get_handle("Alpha"); h) h_Alpha = *h;
        else h_Alpha = {};
        if (auto h = schema.get_handle("TProp"); h) h_TProp = *h;
        else h_TProp = {};
        if (auto h = schema.get_handle("TFlight"); h) h_TFlight = *h;
        else h_TFlight = {};
        if (auto h = schema.get_handle("T0"); h) h_T0 = *h;
        else h_T0 = {};
        if (auto h = schema.get_handle("TStart"); h) h_TStart = *h;
        else h_TStart = {};
        if (auto h = schema.get_handle("dDoca"); h) h_dDoca = *h;
        else h_dDoca = {};
        if (auto h = schema.get_handle("clusterID"); h) h_clusterID = *h;
        else h_clusterID = {};
        if (auto h = schema.get_handle("trkID"); h) h_trkID = *h;
        else h_trkID = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("beta"); h) h_beta = *h;
        else h_beta = {};
        if (auto h = schema.get_handle("tBeta"); h) h_tBeta = *h;
        else h_tBeta = {};
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

}  // namespace clas12::bankdefs
