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


/// HIPO bank wrapper for `DC::calib` (group 20600, item 55).
/// reconstructed hits using DC timing information
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// DC_calib bank;
/// if (auto sc = bank.bind(event.get_bank("DC::calib")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class DC_calib {
 public:
    static constexpr std::string_view bank_name = "DC::calib";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 55;

    class Row;

    DC_calib() = default;

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
            int i_layer = -1;
            int i_wire = -1;
            int i_TDC = -1;
            int i_jitter = -1;
            int i_doca = -1;
            int i_docaError = -1;
            int i_trkDoca = -1;
            int i_timeResidual = -1;
            int i_fitResidual = -1;
            int i_DAFWeight = -1;
            int i_LR = -1;
            int i_X = -1;
            int i_Z = -1;
            int i_B = -1;
            int i_Alpha = -1;
            int i_TProp = -1;
            int i_TFlight = -1;
            int i_T0 = -1;
            int i_TStart = -1;
            int i_dDoca = -1;
            int i_clusterID = -1;
            int i_trkID = -1;
            int i_time = -1;
            int i_beta = -1;
            int i_tBeta = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_superlayer = schema.exists("superlayer") ? schema.getEntryOrder("superlayer") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_wire = schema.exists("wire") ? schema.getEntryOrder("wire") : -1;
            cache.i_TDC = schema.exists("TDC") ? schema.getEntryOrder("TDC") : -1;
            cache.i_jitter = schema.exists("jitter") ? schema.getEntryOrder("jitter") : -1;
            cache.i_doca = schema.exists("doca") ? schema.getEntryOrder("doca") : -1;
            cache.i_docaError = schema.exists("docaError") ? schema.getEntryOrder("docaError") : -1;
            cache.i_trkDoca = schema.exists("trkDoca") ? schema.getEntryOrder("trkDoca") : -1;
            cache.i_timeResidual = schema.exists("timeResidual") ? schema.getEntryOrder("timeResidual") : -1;
            cache.i_fitResidual = schema.exists("fitResidual") ? schema.getEntryOrder("fitResidual") : -1;
            cache.i_DAFWeight = schema.exists("DAFWeight") ? schema.getEntryOrder("DAFWeight") : -1;
            cache.i_LR = schema.exists("LR") ? schema.getEntryOrder("LR") : -1;
            cache.i_X = schema.exists("X") ? schema.getEntryOrder("X") : -1;
            cache.i_Z = schema.exists("Z") ? schema.getEntryOrder("Z") : -1;
            cache.i_B = schema.exists("B") ? schema.getEntryOrder("B") : -1;
            cache.i_Alpha = schema.exists("Alpha") ? schema.getEntryOrder("Alpha") : -1;
            cache.i_TProp = schema.exists("TProp") ? schema.getEntryOrder("TProp") : -1;
            cache.i_TFlight = schema.exists("TFlight") ? schema.getEntryOrder("TFlight") : -1;
            cache.i_T0 = schema.exists("T0") ? schema.getEntryOrder("T0") : -1;
            cache.i_TStart = schema.exists("TStart") ? schema.getEntryOrder("TStart") : -1;
            cache.i_dDoca = schema.exists("dDoca") ? schema.getEntryOrder("dDoca") : -1;
            cache.i_clusterID = schema.exists("clusterID") ? schema.getEntryOrder("clusterID") : -1;
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_beta = schema.exists("beta") ? schema.getEntryOrder("beta") : -1;
            cache.i_tBeta = schema.exists("tBeta") ? schema.getEntryOrder("tBeta") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_superlayer = cache.i_superlayer;
        i_layer = cache.i_layer;
        i_wire = cache.i_wire;
        i_TDC = cache.i_TDC;
        i_jitter = cache.i_jitter;
        i_doca = cache.i_doca;
        i_docaError = cache.i_docaError;
        i_trkDoca = cache.i_trkDoca;
        i_timeResidual = cache.i_timeResidual;
        i_fitResidual = cache.i_fitResidual;
        i_DAFWeight = cache.i_DAFWeight;
        i_LR = cache.i_LR;
        i_X = cache.i_X;
        i_Z = cache.i_Z;
        i_B = cache.i_B;
        i_Alpha = cache.i_Alpha;
        i_TProp = cache.i_TProp;
        i_TFlight = cache.i_TFlight;
        i_T0 = cache.i_T0;
        i_TStart = cache.i_TStart;
        i_dDoca = cache.i_dDoca;
        i_clusterID = cache.i_clusterID;
        i_trkID = cache.i_trkID;
        i_time = cache.i_time;
        i_beta = cache.i_beta;
        i_tBeta = cache.i_tBeta;
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
    /// status: id of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// sector: DC sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// superlayer: DC superlayer (1...6)
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_superlayer, row));
    }
    /// layer: DC layer in superlayer (1...6)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// wire: wire id of DC
    [[nodiscard]] int wire(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_wire, row));
    }
    /// TDC: raw time of the hit
    [[nodiscard]] int TDC(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_TDC, row));
    }
    /// jitter: time jitter (ns)
    [[nodiscard]] int jitter(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_jitter, row));
    }
    /// doca: doca of the hit calculated from TDC (in cm)
    [[nodiscard]] float doca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_doca, row));
    }
    /// docaError: uncertainty on doca of the hit calculated from TDC (in cm)
    [[nodiscard]] float docaError(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_docaError, row));
    }
    /// trkDoca: track doca of the hit (in cm)
    [[nodiscard]] float trkDoca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trkDoca, row));
    }
    /// timeResidual: time residual of the hit (in cm)
    [[nodiscard]] float timeResidual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_timeResidual, row));
    }
    /// fitResidual: fit residual of the hit (in cm, from KF)
    [[nodiscard]] float fitResidual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fitResidual, row));
    }
    /// DAFWeight: Weight by DAF; the 1st bit of status indicates that a hit is belong to single or double
    [[nodiscard]] float DAFWeight(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_DAFWeight, row));
    }
    /// LR: Left/Right ambiguity of the hit
    [[nodiscard]] int LR(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_LR, row));
    }
    /// X: wire x-coordinate  in tilted-sector
    [[nodiscard]] float X(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_X, row));
    }
    /// Z: wire z-coordinate  in tilted-sector
    [[nodiscard]] float Z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_Z, row));
    }
    /// B: B-field intensity at hit position in tilted-sector system
    [[nodiscard]] float B(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_B, row));
    }
    /// Alpha: local angle (degr.) in tilted-sector system
    [[nodiscard]] float Alpha(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_Alpha, row));
    }
    /// TProp: t propagation along the wire (ns)
    [[nodiscard]] float TProp(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_TProp, row));
    }
    /// TFlight: time of flight correction (ns)
    [[nodiscard]] float TFlight(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_TFlight, row));
    }
    /// T0: T0 (ns)
    [[nodiscard]] float T0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_T0, row));
    }
    /// TStart: event start time used (ns)
    [[nodiscard]] float TStart(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_TStart, row));
    }
    /// dDoca: delta Doca correction (cm)
    [[nodiscard]] float dDoca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dDoca, row));
    }
    /// clusterID: ID of associated cluster
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterID, row));
    }
    /// trkID: ID of associated track
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_trkID, row));
    }
    /// time: time used in tracking (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// beta: beta used in tracking
    [[nodiscard]] float beta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_beta, row));
    }
    /// tBeta: beta-dependent time correction used in tracking
    [[nodiscard]] float tBeta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tBeta, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_superlayer = -1;
    int i_layer = -1;
    int i_wire = -1;
    int i_TDC = -1;
    int i_jitter = -1;
    int i_doca = -1;
    int i_docaError = -1;
    int i_trkDoca = -1;
    int i_timeResidual = -1;
    int i_fitResidual = -1;
    int i_DAFWeight = -1;
    int i_LR = -1;
    int i_X = -1;
    int i_Z = -1;
    int i_B = -1;
    int i_Alpha = -1;
    int i_TProp = -1;
    int i_TFlight = -1;
    int i_T0 = -1;
    int i_TStart = -1;
    int i_dDoca = -1;
    int i_clusterID = -1;
    int i_trkID = -1;
    int i_time = -1;
    int i_beta = -1;
    int i_tBeta = -1;

    friend class Row;
};

/// Lightweight row view for DC::calib. Stores a pointer + row index only.
class DC_calib::Row {
 public:
    Row(const DC_calib* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
    const DC_calib* m_bank;
    int m_row;
};

inline DC_calib::Row DC_calib::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
