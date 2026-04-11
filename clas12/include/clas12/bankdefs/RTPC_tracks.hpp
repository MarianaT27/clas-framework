// =============================================================================
// Auto-generated from rtpc.json - do not edit by hand.
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


/// HIPO bank wrapper for `RTPC::tracks` (group 21900, item 22).
/// reconstructed track info
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RTPC_tracks bank;
/// if (auto sc = bank.bind(event.get_bank("RTPC::tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trkID = row.trkID();
/// }
/// @endcode
class RTPC_tracks {
 public:
    static constexpr std::string_view bank_name = "RTPC::tracks";
    static constexpr int bank_group = 21900;
    static constexpr int bank_item  = 22;

    class Row;

    RTPC_tracks() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("trkID"); h) h_trkID = *h;
        else h_trkID = {};
        if (auto h = schema.get_handle("px"); h) h_px = *h;
        else h_px = {};
        if (auto h = schema.get_handle("py"); h) h_py = *h;
        else h_py = {};
        if (auto h = schema.get_handle("pz"); h) h_pz = *h;
        else h_pz = {};
        if (auto h = schema.get_handle("vz"); h) h_vz = *h;
        else h_vz = {};
        if (auto h = schema.get_handle("theta"); h) h_theta = *h;
        else h_theta = {};
        if (auto h = schema.get_handle("phi"); h) h_phi = *h;
        else h_phi = {};
        if (auto h = schema.get_handle("nhits"); h) h_nhits = *h;
        else h_nhits = {};
        if (auto h = schema.get_handle("path"); h) h_path = *h;
        else h_path = {};
        if (auto h = schema.get_handle("adcsum"); h) h_adcsum = *h;
        else h_adcsum = {};
        if (auto h = schema.get_handle("dedx"); h) h_dedx = *h;
        else h_dedx = {};
        if (auto h = schema.get_handle("r_helix"); h) h_r_helix = *h;
        else h_r_helix = {};
        if (auto h = schema.get_handle("x_helix"); h) h_x_helix = *h;
        else h_x_helix = {};
        if (auto h = schema.get_handle("y_helix"); h) h_y_helix = *h;
        else h_y_helix = {};
        if (auto h = schema.get_handle("chi2_helix"); h) h_chi2_helix = *h;
        else h_chi2_helix = {};
        if (auto h = schema.get_handle("min_row"); h) h_min_row = *h;
        else h_min_row = {};
        if (auto h = schema.get_handle("min_col"); h) h_min_col = *h;
        else h_min_col = {};
        if (auto h = schema.get_handle("max_row"); h) h_max_row = *h;
        else h_max_row = {};
        if (auto h = schema.get_handle("max_col"); h) h_max_col = *h;
        else h_max_col = {};
        if (auto h = schema.get_handle("min_time"); h) h_min_time = *h;
        else h_min_time = {};
        if (auto h = schema.get_handle("max_time"); h) h_max_time = *h;
        else h_max_time = {};
        if (auto h = schema.get_handle("min_radius"); h) h_min_radius = *h;
        else h_min_radius = {};
        if (auto h = schema.get_handle("max_radius"); h) h_max_radius = *h;
        else h_max_radius = {};
        if (auto h = schema.get_handle("min_phi"); h) h_min_phi = *h;
        else h_min_phi = {};
        if (auto h = schema.get_handle("max_phi"); h) h_max_phi = *h;
        else h_max_phi = {};
        if (auto h = schema.get_handle("tdiff"); h) h_tdiff = *h;
        else h_tdiff = {};
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
    /// trkID: Track ID of the hit
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trkID, row));
    }
    /// px: X component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: Y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: Z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// vz: Z component of the reconstructed vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }
    /// theta: Theta from the Helix fit of the track (deg)
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_theta, row));
    }
    /// phi: Phi from the Helix fit of the track (deg)
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_phi, row));
    }
    /// nhits: Number of hits in the track
    [[nodiscard]] int nhits(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_nhits, row));
    }
    /// path: Length of the Track (mm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }
    /// adcsum: Total ADC sum of the Track
    [[nodiscard]] float adcsum(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_adcsum, row));
    }
    /// dedx: dEdx for the Track (MeV/mm)
    [[nodiscard]] float dedx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dedx, row));
    }
    /// r_helix: Radius of the helix fit (mm)
    [[nodiscard]] float r_helix(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_r_helix, row));
    }
    /// x_helix: X-coordinate of the center of helix fit (mm)
    [[nodiscard]] float x_helix(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x_helix, row));
    }
    /// y_helix: Y-coordinate of the center of helix fit (mm)
    [[nodiscard]] float y_helix(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y_helix, row));
    }
    /// chi2_helix: Chi squared of the helix fit
    [[nodiscard]] float chi2_helix(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2_helix, row));
    }
    /// min_row: Pad row of hit with smallest time
    [[nodiscard]] float min_row(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_min_row, row));
    }
    /// min_col: Pad col of hit with smallest time
    [[nodiscard]] float min_col(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_min_col, row));
    }
    /// max_row: Pad row of hit with largest time
    [[nodiscard]] float max_row(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_max_row, row));
    }
    /// max_col: Pad col of hit with largest time
    [[nodiscard]] float max_col(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_max_col, row));
    }
    /// min_time: Smallest time in track not shifted (ns)
    [[nodiscard]] float min_time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_min_time, row));
    }
    /// max_time: Largest time in track not shifted (ns)
    [[nodiscard]] float max_time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_max_time, row));
    }
    /// min_radius: Smallest reconstructed radius (mm)
    [[nodiscard]] float min_radius(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_min_radius, row));
    }
    /// max_radius: Largest reconstructed radius (mm)
    [[nodiscard]] float max_radius(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_max_radius, row));
    }
    /// min_phi: Smallest reconstructed phi (rad)
    [[nodiscard]] float min_phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_min_phi, row));
    }
    /// max_phi: Largest reconstructed phi (rad)
    [[nodiscard]] float max_phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_max_phi, row));
    }
    /// tdiff: Time shift (ns)
    [[nodiscard]] float tdiff(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tdiff, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trkID_col() const noexcept {
        return m_view.column<int32_t>(h_trkID);
    }
    /// px column as a contiguous span.
    [[nodiscard]] std::span<const float> px_col() const noexcept {
        return m_view.column<float>(h_px);
    }
    /// py column as a contiguous span.
    [[nodiscard]] std::span<const float> py_col() const noexcept {
        return m_view.column<float>(h_py);
    }
    /// pz column as a contiguous span.
    [[nodiscard]] std::span<const float> pz_col() const noexcept {
        return m_view.column<float>(h_pz);
    }
    /// vz column as a contiguous span.
    [[nodiscard]] std::span<const float> vz_col() const noexcept {
        return m_view.column<float>(h_vz);
    }
    /// theta column as a contiguous span.
    [[nodiscard]] std::span<const float> theta_col() const noexcept {
        return m_view.column<float>(h_theta);
    }
    /// phi column as a contiguous span.
    [[nodiscard]] std::span<const float> phi_col() const noexcept {
        return m_view.column<float>(h_phi);
    }
    /// nhits column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> nhits_col() const noexcept {
        return m_view.column<int32_t>(h_nhits);
    }
    /// path column as a contiguous span.
    [[nodiscard]] std::span<const float> path_col() const noexcept {
        return m_view.column<float>(h_path);
    }
    /// adcsum column as a contiguous span.
    [[nodiscard]] std::span<const float> adcsum_col() const noexcept {
        return m_view.column<float>(h_adcsum);
    }
    /// dedx column as a contiguous span.
    [[nodiscard]] std::span<const float> dedx_col() const noexcept {
        return m_view.column<float>(h_dedx);
    }
    /// r_helix column as a contiguous span.
    [[nodiscard]] std::span<const float> r_helix_col() const noexcept {
        return m_view.column<float>(h_r_helix);
    }
    /// x_helix column as a contiguous span.
    [[nodiscard]] std::span<const float> x_helix_col() const noexcept {
        return m_view.column<float>(h_x_helix);
    }
    /// y_helix column as a contiguous span.
    [[nodiscard]] std::span<const float> y_helix_col() const noexcept {
        return m_view.column<float>(h_y_helix);
    }
    /// chi2_helix column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_helix_col() const noexcept {
        return m_view.column<float>(h_chi2_helix);
    }
    /// min_row column as a contiguous span.
    [[nodiscard]] std::span<const float> min_row_col() const noexcept {
        return m_view.column<float>(h_min_row);
    }
    /// min_col column as a contiguous span.
    [[nodiscard]] std::span<const float> min_col_col() const noexcept {
        return m_view.column<float>(h_min_col);
    }
    /// max_row column as a contiguous span.
    [[nodiscard]] std::span<const float> max_row_col() const noexcept {
        return m_view.column<float>(h_max_row);
    }
    /// max_col column as a contiguous span.
    [[nodiscard]] std::span<const float> max_col_col() const noexcept {
        return m_view.column<float>(h_max_col);
    }
    /// min_time column as a contiguous span.
    [[nodiscard]] std::span<const float> min_time_col() const noexcept {
        return m_view.column<float>(h_min_time);
    }
    /// max_time column as a contiguous span.
    [[nodiscard]] std::span<const float> max_time_col() const noexcept {
        return m_view.column<float>(h_max_time);
    }
    /// min_radius column as a contiguous span.
    [[nodiscard]] std::span<const float> min_radius_col() const noexcept {
        return m_view.column<float>(h_min_radius);
    }
    /// max_radius column as a contiguous span.
    [[nodiscard]] std::span<const float> max_radius_col() const noexcept {
        return m_view.column<float>(h_max_radius);
    }
    /// min_phi column as a contiguous span.
    [[nodiscard]] std::span<const float> min_phi_col() const noexcept {
        return m_view.column<float>(h_min_phi);
    }
    /// max_phi column as a contiguous span.
    [[nodiscard]] std::span<const float> max_phi_col() const noexcept {
        return m_view.column<float>(h_max_phi);
    }
    /// tdiff column as a contiguous span.
    [[nodiscard]] std::span<const float> tdiff_col() const noexcept {
        return m_view.column<float>(h_tdiff);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_trkID{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_vz{};
    hipo::column_handle h_theta{};
    hipo::column_handle h_phi{};
    hipo::column_handle h_nhits{};
    hipo::column_handle h_path{};
    hipo::column_handle h_adcsum{};
    hipo::column_handle h_dedx{};
    hipo::column_handle h_r_helix{};
    hipo::column_handle h_x_helix{};
    hipo::column_handle h_y_helix{};
    hipo::column_handle h_chi2_helix{};
    hipo::column_handle h_min_row{};
    hipo::column_handle h_min_col{};
    hipo::column_handle h_max_row{};
    hipo::column_handle h_max_col{};
    hipo::column_handle h_min_time{};
    hipo::column_handle h_max_time{};
    hipo::column_handle h_min_radius{};
    hipo::column_handle h_max_radius{};
    hipo::column_handle h_min_phi{};
    hipo::column_handle h_max_phi{};
    hipo::column_handle h_tdiff{};

    friend class Row;
};

/// Lightweight row view for RTPC::tracks. Stores a pointer + row index only.
class RTPC_tracks::Row {
 public:
    Row(const RTPC_tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float theta() const noexcept { return m_bank->theta(m_row); }
    [[nodiscard]] float phi() const noexcept { return m_bank->phi(m_row); }
    [[nodiscard]] int nhits() const noexcept { return m_bank->nhits(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float adcsum() const noexcept { return m_bank->adcsum(m_row); }
    [[nodiscard]] float dedx() const noexcept { return m_bank->dedx(m_row); }
    [[nodiscard]] float r_helix() const noexcept { return m_bank->r_helix(m_row); }
    [[nodiscard]] float x_helix() const noexcept { return m_bank->x_helix(m_row); }
    [[nodiscard]] float y_helix() const noexcept { return m_bank->y_helix(m_row); }
    [[nodiscard]] float chi2_helix() const noexcept { return m_bank->chi2_helix(m_row); }
    [[nodiscard]] float min_row() const noexcept { return m_bank->min_row(m_row); }
    [[nodiscard]] float min_col() const noexcept { return m_bank->min_col(m_row); }
    [[nodiscard]] float max_row() const noexcept { return m_bank->max_row(m_row); }
    [[nodiscard]] float max_col() const noexcept { return m_bank->max_col(m_row); }
    [[nodiscard]] float min_time() const noexcept { return m_bank->min_time(m_row); }
    [[nodiscard]] float max_time() const noexcept { return m_bank->max_time(m_row); }
    [[nodiscard]] float min_radius() const noexcept { return m_bank->min_radius(m_row); }
    [[nodiscard]] float max_radius() const noexcept { return m_bank->max_radius(m_row); }
    [[nodiscard]] float min_phi() const noexcept { return m_bank->min_phi(m_row); }
    [[nodiscard]] float max_phi() const noexcept { return m_bank->max_phi(m_row); }
    [[nodiscard]] float tdiff() const noexcept { return m_bank->tdiff(m_row); }
 private:
    const RTPC_tracks* m_bank;
    int m_row;
};

inline RTPC_tracks::Row RTPC_tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
