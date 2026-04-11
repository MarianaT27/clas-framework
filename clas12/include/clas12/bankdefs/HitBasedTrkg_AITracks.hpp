// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
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


/// HIPO bank wrapper for `HitBasedTrkg::AITracks` (group 20600, item 66).
/// reconstructed tracks using DC wire positions
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HitBasedTrkg_AITracks bank;
/// if (auto sc = bank.bind(event.get_bank("HitBasedTrkg::AITracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HitBasedTrkg_AITracks {
 public:
    static constexpr std::string_view bank_name = "HitBasedTrkg::AITracks";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 66;

    class Row;

    HitBasedTrkg_AITracks() = default;

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
        if (auto h = schema.get_handle("c1_x"); h) h_c1_x = *h;
        else h_c1_x = {};
        if (auto h = schema.get_handle("c1_y"); h) h_c1_y = *h;
        else h_c1_y = {};
        if (auto h = schema.get_handle("c1_z"); h) h_c1_z = *h;
        else h_c1_z = {};
        if (auto h = schema.get_handle("c1_ux"); h) h_c1_ux = *h;
        else h_c1_ux = {};
        if (auto h = schema.get_handle("c1_uy"); h) h_c1_uy = *h;
        else h_c1_uy = {};
        if (auto h = schema.get_handle("c1_uz"); h) h_c1_uz = *h;
        else h_c1_uz = {};
        if (auto h = schema.get_handle("c3_x"); h) h_c3_x = *h;
        else h_c3_x = {};
        if (auto h = schema.get_handle("c3_y"); h) h_c3_y = *h;
        else h_c3_y = {};
        if (auto h = schema.get_handle("c3_z"); h) h_c3_z = *h;
        else h_c3_z = {};
        if (auto h = schema.get_handle("c3_ux"); h) h_c3_ux = *h;
        else h_c3_ux = {};
        if (auto h = schema.get_handle("c3_uy"); h) h_c3_uy = *h;
        else h_c3_uy = {};
        if (auto h = schema.get_handle("c3_uz"); h) h_c3_uz = *h;
        else h_c3_uz = {};
        if (auto h = schema.get_handle("t1_x"); h) h_t1_x = *h;
        else h_t1_x = {};
        if (auto h = schema.get_handle("t1_y"); h) h_t1_y = *h;
        else h_t1_y = {};
        if (auto h = schema.get_handle("t1_z"); h) h_t1_z = *h;
        else h_t1_z = {};
        if (auto h = schema.get_handle("t1_px"); h) h_t1_px = *h;
        else h_t1_px = {};
        if (auto h = schema.get_handle("t1_py"); h) h_t1_py = *h;
        else h_t1_py = {};
        if (auto h = schema.get_handle("t1_pz"); h) h_t1_pz = *h;
        else h_t1_pz = {};
        if (auto h = schema.get_handle("Vtx0_x"); h) h_Vtx0_x = *h;
        else h_Vtx0_x = {};
        if (auto h = schema.get_handle("Vtx0_y"); h) h_Vtx0_y = *h;
        else h_Vtx0_y = {};
        if (auto h = schema.get_handle("Vtx0_z"); h) h_Vtx0_z = *h;
        else h_Vtx0_z = {};
        if (auto h = schema.get_handle("p0_x"); h) h_p0_x = *h;
        else h_p0_x = {};
        if (auto h = schema.get_handle("p0_y"); h) h_p0_y = *h;
        else h_p0_y = {};
        if (auto h = schema.get_handle("p0_z"); h) h_p0_z = *h;
        else h_p0_z = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("tx"); h) h_tx = *h;
        else h_tx = {};
        if (auto h = schema.get_handle("ty"); h) h_ty = *h;
        else h_ty = {};
        if (auto h = schema.get_handle("Cross1_ID"); h) h_Cross1_ID = *h;
        else h_Cross1_ID = {};
        if (auto h = schema.get_handle("Cross2_ID"); h) h_Cross2_ID = *h;
        else h_Cross2_ID = {};
        if (auto h = schema.get_handle("Cross3_ID"); h) h_Cross3_ID = *h;
        else h_Cross3_ID = {};
        if (auto h = schema.get_handle("Cluster1_ID"); h) h_Cluster1_ID = *h;
        else h_Cluster1_ID = {};
        if (auto h = schema.get_handle("Cluster2_ID"); h) h_Cluster2_ID = *h;
        else h_Cluster2_ID = {};
        if (auto h = schema.get_handle("Cluster3_ID"); h) h_Cluster3_ID = *h;
        else h_Cluster3_ID = {};
        if (auto h = schema.get_handle("Cluster4_ID"); h) h_Cluster4_ID = *h;
        else h_Cluster4_ID = {};
        if (auto h = schema.get_handle("Cluster5_ID"); h) h_Cluster5_ID = *h;
        else h_Cluster5_ID = {};
        if (auto h = schema.get_handle("Cluster6_ID"); h) h_Cluster6_ID = *h;
        else h_Cluster6_ID = {};
        if (auto h = schema.get_handle("q"); h) h_q = *h;
        else h_q = {};
        if (auto h = schema.get_handle("pathlength"); h) h_pathlength = *h;
        else h_pathlength = {};
        if (auto h = schema.get_handle("chi2"); h) h_chi2 = *h;
        else h_chi2 = {};
        if (auto h = schema.get_handle("ndf"); h) h_ndf = *h;
        else h_ndf = {};
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
    /// id: id of the track
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: status of the track
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: sector of the track
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// c1_x: Upstream Region 1 cross x-position in the lab (in cm)
    [[nodiscard]] float c1_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c1_x, row));
    }
    /// c1_y: Upstream Region 1 cross y-position in the lab (in cm)
    [[nodiscard]] float c1_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c1_y, row));
    }
    /// c1_z: Upstream Region 1 cross z-position in the lab (in cm)
    [[nodiscard]] float c1_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c1_z, row));
    }
    /// c1_ux: Upstream Region 1 cross unit x-direction vector in the lab
    [[nodiscard]] float c1_ux(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c1_ux, row));
    }
    /// c1_uy: Upstream Region 1 cross unit y-direction vector in the lab
    [[nodiscard]] float c1_uy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c1_uy, row));
    }
    /// c1_uz: Upstream Region 1 cross unit z-direction vector in the lab
    [[nodiscard]] float c1_uz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c1_uz, row));
    }
    /// c3_x: Downstream Region 3 cross x-position in the lab (in cm)
    [[nodiscard]] float c3_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c3_x, row));
    }
    /// c3_y: Downstream Region 3 cross y-position in the lab (in cm)
    [[nodiscard]] float c3_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c3_y, row));
    }
    /// c3_z: Downstream Region 3 cross z-position in the lab (in cm)
    [[nodiscard]] float c3_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c3_z, row));
    }
    /// c3_ux: Downstream Region 3 cross unit x-direction vector in the lab
    [[nodiscard]] float c3_ux(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c3_ux, row));
    }
    /// c3_uy: Downstream Region 3 cross unit y-direction vector in the lab
    [[nodiscard]] float c3_uy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c3_uy, row));
    }
    /// c3_uz: Downstream Region 3 cross unit z-direction vector in the lab
    [[nodiscard]] float c3_uz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_c3_uz, row));
    }
    /// t1_x: Upstream Region 1 track x-position in the lab (in cm)
    [[nodiscard]] float t1_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_t1_x, row));
    }
    /// t1_y: Upstream Region 1 track y-position in the lab (in cm)
    [[nodiscard]] float t1_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_t1_y, row));
    }
    /// t1_z: Upstream Region 1 track z-position in the lab (in cm)
    [[nodiscard]] float t1_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_t1_z, row));
    }
    /// t1_px: Upstream Region 1 track unit x-momentum vector in the lab
    [[nodiscard]] float t1_px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_t1_px, row));
    }
    /// t1_py: Upstream Region 1 track unit y-momentum vector in the lab
    [[nodiscard]] float t1_py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_t1_py, row));
    }
    /// t1_pz: Upstream Region 1 track unit z-momentum vector in the lab
    [[nodiscard]] float t1_pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_t1_pz, row));
    }
    /// Vtx0_x: Vertex x-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Vtx0_x, row));
    }
    /// Vtx0_y: Vertex y-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Vtx0_y, row));
    }
    /// Vtx0_z: Vertex z-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Vtx0_z, row));
    }
    /// p0_x: 3-momentum x-coordinate of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float p0_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p0_x, row));
    }
    /// p0_y: 3-momentum y-coordinate of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float p0_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p0_y, row));
    }
    /// p0_z: 3-momentum z-coordinate of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float p0_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p0_z, row));
    }
    /// x: last site stateVec in tilted sector x (in cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: last site stateVec in tilted sector y (in cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: last site stateVec in tilted sector z (in cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// tx: last site stateVec in tilted sector tx (px/pz)
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tx, row));
    }
    /// ty: last site stateVec in tilted sector ty (py/pz)
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ty, row));
    }
    /// Cross1_ID: id of first cross on track
    [[nodiscard]] int Cross1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross1_ID, row));
    }
    /// Cross2_ID: id of second cross on track
    [[nodiscard]] int Cross2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross2_ID, row));
    }
    /// Cross3_ID: id of third cross on track
    [[nodiscard]] int Cross3_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross3_ID, row));
    }
    /// Cluster1_ID: cluster ID in superlayer 1
    [[nodiscard]] int Cluster1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster1_ID, row));
    }
    /// Cluster2_ID: cluster ID in superlayer 2
    [[nodiscard]] int Cluster2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster2_ID, row));
    }
    /// Cluster3_ID: cluster ID in superlayer 3
    [[nodiscard]] int Cluster3_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster3_ID, row));
    }
    /// Cluster4_ID: cluster ID in superlayer 4
    [[nodiscard]] int Cluster4_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster4_ID, row));
    }
    /// Cluster5_ID: cluster ID in superlayer 5
    [[nodiscard]] int Cluster5_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster5_ID, row));
    }
    /// Cluster6_ID: cluster ID in superlayer 6
    [[nodiscard]] int Cluster6_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cluster6_ID, row));
    }
    /// q: charge of the track
    [[nodiscard]] int q(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_q, row));
    }
    /// pathlength: pathlength of the track
    [[nodiscard]] float pathlength(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathlength, row));
    }
    /// chi2: fit chi2 of the track
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// ndf: fit ndf of the track
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ndf, row));
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
    /// c1_x column as a contiguous span.
    [[nodiscard]] std::span<const float> c1_x_col() const noexcept {
        return m_view.column<float>(h_c1_x);
    }
    /// c1_y column as a contiguous span.
    [[nodiscard]] std::span<const float> c1_y_col() const noexcept {
        return m_view.column<float>(h_c1_y);
    }
    /// c1_z column as a contiguous span.
    [[nodiscard]] std::span<const float> c1_z_col() const noexcept {
        return m_view.column<float>(h_c1_z);
    }
    /// c1_ux column as a contiguous span.
    [[nodiscard]] std::span<const float> c1_ux_col() const noexcept {
        return m_view.column<float>(h_c1_ux);
    }
    /// c1_uy column as a contiguous span.
    [[nodiscard]] std::span<const float> c1_uy_col() const noexcept {
        return m_view.column<float>(h_c1_uy);
    }
    /// c1_uz column as a contiguous span.
    [[nodiscard]] std::span<const float> c1_uz_col() const noexcept {
        return m_view.column<float>(h_c1_uz);
    }
    /// c3_x column as a contiguous span.
    [[nodiscard]] std::span<const float> c3_x_col() const noexcept {
        return m_view.column<float>(h_c3_x);
    }
    /// c3_y column as a contiguous span.
    [[nodiscard]] std::span<const float> c3_y_col() const noexcept {
        return m_view.column<float>(h_c3_y);
    }
    /// c3_z column as a contiguous span.
    [[nodiscard]] std::span<const float> c3_z_col() const noexcept {
        return m_view.column<float>(h_c3_z);
    }
    /// c3_ux column as a contiguous span.
    [[nodiscard]] std::span<const float> c3_ux_col() const noexcept {
        return m_view.column<float>(h_c3_ux);
    }
    /// c3_uy column as a contiguous span.
    [[nodiscard]] std::span<const float> c3_uy_col() const noexcept {
        return m_view.column<float>(h_c3_uy);
    }
    /// c3_uz column as a contiguous span.
    [[nodiscard]] std::span<const float> c3_uz_col() const noexcept {
        return m_view.column<float>(h_c3_uz);
    }
    /// t1_x column as a contiguous span.
    [[nodiscard]] std::span<const float> t1_x_col() const noexcept {
        return m_view.column<float>(h_t1_x);
    }
    /// t1_y column as a contiguous span.
    [[nodiscard]] std::span<const float> t1_y_col() const noexcept {
        return m_view.column<float>(h_t1_y);
    }
    /// t1_z column as a contiguous span.
    [[nodiscard]] std::span<const float> t1_z_col() const noexcept {
        return m_view.column<float>(h_t1_z);
    }
    /// t1_px column as a contiguous span.
    [[nodiscard]] std::span<const float> t1_px_col() const noexcept {
        return m_view.column<float>(h_t1_px);
    }
    /// t1_py column as a contiguous span.
    [[nodiscard]] std::span<const float> t1_py_col() const noexcept {
        return m_view.column<float>(h_t1_py);
    }
    /// t1_pz column as a contiguous span.
    [[nodiscard]] std::span<const float> t1_pz_col() const noexcept {
        return m_view.column<float>(h_t1_pz);
    }
    /// Vtx0_x column as a contiguous span.
    [[nodiscard]] std::span<const float> Vtx0_x_col() const noexcept {
        return m_view.column<float>(h_Vtx0_x);
    }
    /// Vtx0_y column as a contiguous span.
    [[nodiscard]] std::span<const float> Vtx0_y_col() const noexcept {
        return m_view.column<float>(h_Vtx0_y);
    }
    /// Vtx0_z column as a contiguous span.
    [[nodiscard]] std::span<const float> Vtx0_z_col() const noexcept {
        return m_view.column<float>(h_Vtx0_z);
    }
    /// p0_x column as a contiguous span.
    [[nodiscard]] std::span<const float> p0_x_col() const noexcept {
        return m_view.column<float>(h_p0_x);
    }
    /// p0_y column as a contiguous span.
    [[nodiscard]] std::span<const float> p0_y_col() const noexcept {
        return m_view.column<float>(h_p0_y);
    }
    /// p0_z column as a contiguous span.
    [[nodiscard]] std::span<const float> p0_z_col() const noexcept {
        return m_view.column<float>(h_p0_z);
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
    /// tx column as a contiguous span.
    [[nodiscard]] std::span<const float> tx_col() const noexcept {
        return m_view.column<float>(h_tx);
    }
    /// ty column as a contiguous span.
    [[nodiscard]] std::span<const float> ty_col() const noexcept {
        return m_view.column<float>(h_ty);
    }
    /// Cross1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross1_ID);
    }
    /// Cross2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross2_ID);
    }
    /// Cross3_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross3_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross3_ID);
    }
    /// Cluster1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster1_ID);
    }
    /// Cluster2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster2_ID);
    }
    /// Cluster3_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster3_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster3_ID);
    }
    /// Cluster4_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster4_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster4_ID);
    }
    /// Cluster5_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster5_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster5_ID);
    }
    /// Cluster6_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cluster6_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cluster6_ID);
    }
    /// q column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> q_col() const noexcept {
        return m_view.column<int8_t>(h_q);
    }
    /// pathlength column as a contiguous span.
    [[nodiscard]] std::span<const float> pathlength_col() const noexcept {
        return m_view.column<float>(h_pathlength);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
    }
    /// ndf column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ndf_col() const noexcept {
        return m_view.column<int16_t>(h_ndf);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_c1_x{};
    hipo::column_handle h_c1_y{};
    hipo::column_handle h_c1_z{};
    hipo::column_handle h_c1_ux{};
    hipo::column_handle h_c1_uy{};
    hipo::column_handle h_c1_uz{};
    hipo::column_handle h_c3_x{};
    hipo::column_handle h_c3_y{};
    hipo::column_handle h_c3_z{};
    hipo::column_handle h_c3_ux{};
    hipo::column_handle h_c3_uy{};
    hipo::column_handle h_c3_uz{};
    hipo::column_handle h_t1_x{};
    hipo::column_handle h_t1_y{};
    hipo::column_handle h_t1_z{};
    hipo::column_handle h_t1_px{};
    hipo::column_handle h_t1_py{};
    hipo::column_handle h_t1_pz{};
    hipo::column_handle h_Vtx0_x{};
    hipo::column_handle h_Vtx0_y{};
    hipo::column_handle h_Vtx0_z{};
    hipo::column_handle h_p0_x{};
    hipo::column_handle h_p0_y{};
    hipo::column_handle h_p0_z{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_tx{};
    hipo::column_handle h_ty{};
    hipo::column_handle h_Cross1_ID{};
    hipo::column_handle h_Cross2_ID{};
    hipo::column_handle h_Cross3_ID{};
    hipo::column_handle h_Cluster1_ID{};
    hipo::column_handle h_Cluster2_ID{};
    hipo::column_handle h_Cluster3_ID{};
    hipo::column_handle h_Cluster4_ID{};
    hipo::column_handle h_Cluster5_ID{};
    hipo::column_handle h_Cluster6_ID{};
    hipo::column_handle h_q{};
    hipo::column_handle h_pathlength{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_ndf{};

    friend class Row;
};

/// Lightweight row view for HitBasedTrkg::AITracks. Stores a pointer + row index only.
class HitBasedTrkg_AITracks::Row {
 public:
    Row(const HitBasedTrkg_AITracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] float c1_x() const noexcept { return m_bank->c1_x(m_row); }
    [[nodiscard]] float c1_y() const noexcept { return m_bank->c1_y(m_row); }
    [[nodiscard]] float c1_z() const noexcept { return m_bank->c1_z(m_row); }
    [[nodiscard]] float c1_ux() const noexcept { return m_bank->c1_ux(m_row); }
    [[nodiscard]] float c1_uy() const noexcept { return m_bank->c1_uy(m_row); }
    [[nodiscard]] float c1_uz() const noexcept { return m_bank->c1_uz(m_row); }
    [[nodiscard]] float c3_x() const noexcept { return m_bank->c3_x(m_row); }
    [[nodiscard]] float c3_y() const noexcept { return m_bank->c3_y(m_row); }
    [[nodiscard]] float c3_z() const noexcept { return m_bank->c3_z(m_row); }
    [[nodiscard]] float c3_ux() const noexcept { return m_bank->c3_ux(m_row); }
    [[nodiscard]] float c3_uy() const noexcept { return m_bank->c3_uy(m_row); }
    [[nodiscard]] float c3_uz() const noexcept { return m_bank->c3_uz(m_row); }
    [[nodiscard]] float t1_x() const noexcept { return m_bank->t1_x(m_row); }
    [[nodiscard]] float t1_y() const noexcept { return m_bank->t1_y(m_row); }
    [[nodiscard]] float t1_z() const noexcept { return m_bank->t1_z(m_row); }
    [[nodiscard]] float t1_px() const noexcept { return m_bank->t1_px(m_row); }
    [[nodiscard]] float t1_py() const noexcept { return m_bank->t1_py(m_row); }
    [[nodiscard]] float t1_pz() const noexcept { return m_bank->t1_pz(m_row); }
    [[nodiscard]] float Vtx0_x() const noexcept { return m_bank->Vtx0_x(m_row); }
    [[nodiscard]] float Vtx0_y() const noexcept { return m_bank->Vtx0_y(m_row); }
    [[nodiscard]] float Vtx0_z() const noexcept { return m_bank->Vtx0_z(m_row); }
    [[nodiscard]] float p0_x() const noexcept { return m_bank->p0_x(m_row); }
    [[nodiscard]] float p0_y() const noexcept { return m_bank->p0_y(m_row); }
    [[nodiscard]] float p0_z() const noexcept { return m_bank->p0_z(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] int Cross1_ID() const noexcept { return m_bank->Cross1_ID(m_row); }
    [[nodiscard]] int Cross2_ID() const noexcept { return m_bank->Cross2_ID(m_row); }
    [[nodiscard]] int Cross3_ID() const noexcept { return m_bank->Cross3_ID(m_row); }
    [[nodiscard]] int Cluster1_ID() const noexcept { return m_bank->Cluster1_ID(m_row); }
    [[nodiscard]] int Cluster2_ID() const noexcept { return m_bank->Cluster2_ID(m_row); }
    [[nodiscard]] int Cluster3_ID() const noexcept { return m_bank->Cluster3_ID(m_row); }
    [[nodiscard]] int Cluster4_ID() const noexcept { return m_bank->Cluster4_ID(m_row); }
    [[nodiscard]] int Cluster5_ID() const noexcept { return m_bank->Cluster5_ID(m_row); }
    [[nodiscard]] int Cluster6_ID() const noexcept { return m_bank->Cluster6_ID(m_row); }
    [[nodiscard]] int q() const noexcept { return m_bank->q(m_row); }
    [[nodiscard]] float pathlength() const noexcept { return m_bank->pathlength(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int ndf() const noexcept { return m_bank->ndf(m_row); }
 private:
    const HitBasedTrkg_AITracks* m_bank;
    int m_row;
};

inline HitBasedTrkg_AITracks::Row HitBasedTrkg_AITracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
