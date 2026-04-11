// =============================================================================
// Auto-generated from tof.json - do not edit by hand.
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


/// HIPO bank wrapper for `FTOF::hits` (group 21200, item 31).
/// reconstructed hit info from FTOF
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FTOF_hits bank;
/// if (auto sc = bank.bind(event.get_bank("FTOF::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class FTOF_hits {
 public:
    static constexpr std::string_view bank_name = "FTOF::hits";
    static constexpr int bank_group = 21200;
    static constexpr int bank_item  = 31;

    class Row;

    FTOF_hits() = default;

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
        if (auto h = schema.get_handle("trackid"); h) h_trackid = *h;
        else h_trackid = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("component"); h) h_component = *h;
        else h_component = {};
        if (auto h = schema.get_handle("energy"); h) h_energy = *h;
        else h_energy = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("energy_unc"); h) h_energy_unc = *h;
        else h_energy_unc = {};
        if (auto h = schema.get_handle("time_unc"); h) h_time_unc = *h;
        else h_time_unc = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("x_unc"); h) h_x_unc = *h;
        else h_x_unc = {};
        if (auto h = schema.get_handle("y_unc"); h) h_y_unc = *h;
        else h_y_unc = {};
        if (auto h = schema.get_handle("z_unc"); h) h_z_unc = *h;
        else h_z_unc = {};
        if (auto h = schema.get_handle("tx"); h) h_tx = *h;
        else h_tx = {};
        if (auto h = schema.get_handle("ty"); h) h_ty = *h;
        else h_ty = {};
        if (auto h = schema.get_handle("tz"); h) h_tz = *h;
        else h_tz = {};
        if (auto h = schema.get_handle("adc_idx1"); h) h_adc_idx1 = *h;
        else h_adc_idx1 = {};
        if (auto h = schema.get_handle("adc_idx2"); h) h_adc_idx2 = *h;
        else h_adc_idx2 = {};
        if (auto h = schema.get_handle("tdc_idx1"); h) h_tdc_idx1 = *h;
        else h_tdc_idx1 = {};
        if (auto h = schema.get_handle("tdc_idx2"); h) h_tdc_idx2 = *h;
        else h_tdc_idx2 = {};
        if (auto h = schema.get_handle("clusterid"); h) h_clusterid = *h;
        else h_clusterid = {};
        if (auto h = schema.get_handle("pathLength"); h) h_pathLength = *h;
        else h_pathLength = {};
        if (auto h = schema.get_handle("pathLengthThruBar"); h) h_pathLengthThruBar = *h;
        else h_pathLengthThruBar = {};
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
    /// status: hit status defined based on presence (0) or absence (1) of TDCR-TDCL-ADCR-ADCL
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// trackid: matched DC track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trackid, row));
    }
    /// sector: sector of FTOF
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: panel id of FTOF (1-1A, 2-1B, 3-2
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: paddle id of FTOF
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// energy: E dep (MeV) of hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Hit time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// energy_unc: E dep unc (MeV) of hit
    [[nodiscard]] float energy_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_unc, row));
    }
    /// time_unc: Hit time unc (ns)
    [[nodiscard]] float time_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_unc, row));
    }
    /// x: Global X coor (cm) of hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Global Y coor (cm) of hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Global Z coor (cm) of hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// x_unc: Global X coor unc (cm) of hit
    [[nodiscard]] float x_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x_unc, row));
    }
    /// y_unc: Global Y coor unc (cm) of hit
    [[nodiscard]] float y_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y_unc, row));
    }
    /// z_unc: Global Z coor unc (cm) of hit
    [[nodiscard]] float z_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z_unc, row));
    }
    /// tx: Global X coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tx, row));
    }
    /// ty: Global Y coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ty, row));
    }
    /// tz: Global Z coor (cm) of hit from DC info - trackid index
    [[nodiscard]] float tz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tz, row));
    }
    /// adc_idx1: index of hit with ADCL in adc bank, starts at zero
    [[nodiscard]] int adc_idx1(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_adc_idx1, row));
    }
    /// adc_idx2: index of hit with ADCR in adc bank, starts at zero
    [[nodiscard]] int adc_idx2(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_adc_idx2, row));
    }
    /// tdc_idx1: index of hit with TDCL in tdc bank, starts at zero
    [[nodiscard]] int tdc_idx1(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_tdc_idx1, row));
    }
    /// tdc_idx2: index of hit with TDCR in tdc bank, starts at zero
    [[nodiscard]] int tdc_idx2(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_tdc_idx2, row));
    }
    /// clusterid: pointer to the cluster
    [[nodiscard]] int clusterid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusterid, row));
    }
    /// pathLength: pathlength of the track from the vertex (doca point to the beamline to the midpoint between the entrance and exit of the hit bar
    [[nodiscard]] float pathLength(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathLength, row));
    }
    /// pathLengthThruBar: pathlength of the track from the entrance point to the exit point through the hit bar 
    [[nodiscard]] float pathLengthThruBar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathLengthThruBar, row));
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
    /// trackid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trackid_col() const noexcept {
        return m_view.column<int16_t>(h_trackid);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// energy_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_unc_col() const noexcept {
        return m_view.column<float>(h_energy_unc);
    }
    /// time_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> time_unc_col() const noexcept {
        return m_view.column<float>(h_time_unc);
    }
    /// x column as a contiguous span.
    [[nodiscard]] std::span<const float> x_col() const noexcept {
        return m_view.column<float>(h_x);
    }
    /// y column as a contiguous span.
    [[nodiscard]] std::span<const float> y_col() const noexcept {
        return m_view.column<float>(h_y);
    }
    /// z column as a contiguous span.
    [[nodiscard]] std::span<const float> z_col() const noexcept {
        return m_view.column<float>(h_z);
    }
    /// x_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> x_unc_col() const noexcept {
        return m_view.column<float>(h_x_unc);
    }
    /// y_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> y_unc_col() const noexcept {
        return m_view.column<float>(h_y_unc);
    }
    /// z_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> z_unc_col() const noexcept {
        return m_view.column<float>(h_z_unc);
    }
    /// tx column as a contiguous span.
    [[nodiscard]] std::span<const float> tx_col() const noexcept {
        return m_view.column<float>(h_tx);
    }
    /// ty column as a contiguous span.
    [[nodiscard]] std::span<const float> ty_col() const noexcept {
        return m_view.column<float>(h_ty);
    }
    /// tz column as a contiguous span.
    [[nodiscard]] std::span<const float> tz_col() const noexcept {
        return m_view.column<float>(h_tz);
    }
    /// adc_idx1 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> adc_idx1_col() const noexcept {
        return m_view.column<int16_t>(h_adc_idx1);
    }
    /// adc_idx2 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> adc_idx2_col() const noexcept {
        return m_view.column<int16_t>(h_adc_idx2);
    }
    /// tdc_idx1 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> tdc_idx1_col() const noexcept {
        return m_view.column<int16_t>(h_tdc_idx1);
    }
    /// tdc_idx2 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> tdc_idx2_col() const noexcept {
        return m_view.column<int16_t>(h_tdc_idx2);
    }
    /// clusterid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusterid_col() const noexcept {
        return m_view.column<int16_t>(h_clusterid);
    }
    /// pathLength column as a contiguous span.
    [[nodiscard]] std::span<const float> pathLength_col() const noexcept {
        return m_view.column<float>(h_pathLength);
    }
    /// pathLengthThruBar column as a contiguous span.
    [[nodiscard]] std::span<const float> pathLengthThruBar_col() const noexcept {
        return m_view.column<float>(h_pathLengthThruBar);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_trackid{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_energy_unc{};
    hipo::column_handle h_time_unc{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_x_unc{};
    hipo::column_handle h_y_unc{};
    hipo::column_handle h_z_unc{};
    hipo::column_handle h_tx{};
    hipo::column_handle h_ty{};
    hipo::column_handle h_tz{};
    hipo::column_handle h_adc_idx1{};
    hipo::column_handle h_adc_idx2{};
    hipo::column_handle h_tdc_idx1{};
    hipo::column_handle h_tdc_idx2{};
    hipo::column_handle h_clusterid{};
    hipo::column_handle h_pathLength{};
    hipo::column_handle h_pathLengthThruBar{};

    friend class Row;
};

/// Lightweight row view for FTOF::hits. Stores a pointer + row index only.
class FTOF_hits::Row {
 public:
    Row(const FTOF_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float energy_unc() const noexcept { return m_bank->energy_unc(m_row); }
    [[nodiscard]] float time_unc() const noexcept { return m_bank->time_unc(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float x_unc() const noexcept { return m_bank->x_unc(m_row); }
    [[nodiscard]] float y_unc() const noexcept { return m_bank->y_unc(m_row); }
    [[nodiscard]] float z_unc() const noexcept { return m_bank->z_unc(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] int adc_idx1() const noexcept { return m_bank->adc_idx1(m_row); }
    [[nodiscard]] int adc_idx2() const noexcept { return m_bank->adc_idx2(m_row); }
    [[nodiscard]] int tdc_idx1() const noexcept { return m_bank->tdc_idx1(m_row); }
    [[nodiscard]] int tdc_idx2() const noexcept { return m_bank->tdc_idx2(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
    [[nodiscard]] float pathLength() const noexcept { return m_bank->pathLength(m_row); }
    [[nodiscard]] float pathLengthThruBar() const noexcept { return m_bank->pathLengthThruBar(m_row); }
 private:
    const FTOF_hits* m_bank;
    int m_row;
};

inline FTOF_hits::Row FTOF_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
