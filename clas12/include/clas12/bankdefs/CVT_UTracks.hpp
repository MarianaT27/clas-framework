// =============================================================================
// Auto-generated from cvt.json - do not edit by hand.
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


/// HIPO bank wrapper for `CVT::UTracks` (group 20500, item 37).
/// reconstructed CVT tracks
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CVT_UTracks bank;
/// if (auto sc = bank.bind(event.get_bank("CVT::UTracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class CVT_UTracks {
 public:
    static constexpr std::string_view bank_name = "CVT::UTracks";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 37;

    class Row;

    CVT_UTracks() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("ID"); h) h_ID = *h;
        else h_ID = {};
        if (auto h = schema.get_handle("q"); h) h_q = *h;
        else h_q = {};
        if (auto h = schema.get_handle("p"); h) h_p = *h;
        else h_p = {};
        if (auto h = schema.get_handle("pt"); h) h_pt = *h;
        else h_pt = {};
        if (auto h = schema.get_handle("phi0"); h) h_phi0 = *h;
        else h_phi0 = {};
        if (auto h = schema.get_handle("tandip"); h) h_tandip = *h;
        else h_tandip = {};
        if (auto h = schema.get_handle("z0"); h) h_z0 = *h;
        else h_z0 = {};
        if (auto h = schema.get_handle("d0"); h) h_d0 = *h;
        else h_d0 = {};
        if (auto h = schema.get_handle("cov_d02"); h) h_cov_d02 = *h;
        else h_cov_d02 = {};
        if (auto h = schema.get_handle("cov_d0phi0"); h) h_cov_d0phi0 = *h;
        else h_cov_d0phi0 = {};
        if (auto h = schema.get_handle("cov_d0rho"); h) h_cov_d0rho = *h;
        else h_cov_d0rho = {};
        if (auto h = schema.get_handle("cov_phi02"); h) h_cov_phi02 = *h;
        else h_cov_phi02 = {};
        if (auto h = schema.get_handle("cov_phi0rho"); h) h_cov_phi0rho = *h;
        else h_cov_phi0rho = {};
        if (auto h = schema.get_handle("cov_rho2"); h) h_cov_rho2 = *h;
        else h_cov_rho2 = {};
        if (auto h = schema.get_handle("cov_z02"); h) h_cov_z02 = *h;
        else h_cov_z02 = {};
        if (auto h = schema.get_handle("cov_z0tandip"); h) h_cov_z0tandip = *h;
        else h_cov_z0tandip = {};
        if (auto h = schema.get_handle("cov_tandip2"); h) h_cov_tandip2 = *h;
        else h_cov_tandip2 = {};
        if (auto h = schema.get_handle("chi2"); h) h_chi2 = *h;
        else h_chi2 = {};
        if (auto h = schema.get_handle("ndf"); h) h_ndf = *h;
        else h_ndf = {};
        if (auto h = schema.get_handle("pid"); h) h_pid = *h;
        else h_pid = {};
        if (auto h = schema.get_handle("seedID"); h) h_seedID = *h;
        else h_seedID = {};
        if (auto h = schema.get_handle("xb"); h) h_xb = *h;
        else h_xb = {};
        if (auto h = schema.get_handle("yb"); h) h_yb = *h;
        else h_yb = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
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
    /// ID: ID
    [[nodiscard]] int ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ID, row));
    }
    /// q: charge
    [[nodiscard]] int q(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_q, row));
    }
    /// p: total momentum
    [[nodiscard]] float p(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p, row));
    }
    /// pt: transverse momentum
    [[nodiscard]] float pt(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pt, row));
    }
    /// phi0: helical track fit parameter: phi at DOCA
    [[nodiscard]] float phi0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_phi0, row));
    }
    /// tandip: helical track fit parameter: dip angle
    [[nodiscard]] float tandip(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tandip, row));
    }
    /// z0: helical track fit parameter: value of z at the DOCA
    [[nodiscard]] float z0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z0, row));
    }
    /// d0: helical track fit parameter: Distance of Closest Approach
    [[nodiscard]] float d0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_d0, row));
    }
    /// cov_d02: helical track fit covariance matrix element : delta_d0^2
    [[nodiscard]] float cov_d02(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_d02, row));
    }
    /// cov_d0phi0: helical track fit covariance matrix element : delta_d0.delta_phi0
    [[nodiscard]] float cov_d0phi0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_d0phi0, row));
    }
    /// cov_d0rho: helical track fit covariance matrix element : delta_d0.delta_rho
    [[nodiscard]] float cov_d0rho(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_d0rho, row));
    }
    /// cov_phi02: helical track fit covariance matrix element : delta_phi0^2
    [[nodiscard]] float cov_phi02(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_phi02, row));
    }
    /// cov_phi0rho: helical track fit covariance matrix element : delta_phi0.delta_rho
    [[nodiscard]] float cov_phi0rho(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_phi0rho, row));
    }
    /// cov_rho2: helical track fit covariance matrix element : delta_rho.delta_rho
    [[nodiscard]] float cov_rho2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_rho2, row));
    }
    /// cov_z02: helical track fit covariance matrix element : delta_z0^2
    [[nodiscard]] float cov_z02(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_z02, row));
    }
    /// cov_z0tandip: helical track fit covariance matrix element : delta_z0.delta_tandip
    [[nodiscard]] float cov_z0tandip(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_z0tandip, row));
    }
    /// cov_tandip2: helical track fit covariance matrix element : delta_tandip^2
    [[nodiscard]] float cov_tandip2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_tandip2, row));
    }
    /// chi2: Fit chi^2 
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// ndf: Fit ndf 
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ndf, row));
    }
    /// pid: PID assumption
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pid, row));
    }
    /// seedID: ID of track seed
    [[nodiscard]] int seedID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_seedID, row));
    }
    /// xb: x offset  
    [[nodiscard]] float xb(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_xb, row));
    }
    /// yb: y offset  
    [[nodiscard]] float yb(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_yb, row));
    }
    /// status: track status : nabc (n = KF iterations a = nb SVT crosses, b = nb BMT Z clusters, c = nb BMT C clusters, used in fit)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ID_col() const noexcept {
        return m_view.column<int16_t>(h_ID);
    }
    /// q column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> q_col() const noexcept {
        return m_view.column<int8_t>(h_q);
    }
    /// p column as a contiguous span.
    [[nodiscard]] std::span<const float> p_col() const noexcept {
        return m_view.column<float>(h_p);
    }
    /// pt column as a contiguous span.
    [[nodiscard]] std::span<const float> pt_col() const noexcept {
        return m_view.column<float>(h_pt);
    }
    /// phi0 column as a contiguous span.
    [[nodiscard]] std::span<const float> phi0_col() const noexcept {
        return m_view.column<float>(h_phi0);
    }
    /// tandip column as a contiguous span.
    [[nodiscard]] std::span<const float> tandip_col() const noexcept {
        return m_view.column<float>(h_tandip);
    }
    /// z0 column as a contiguous span.
    [[nodiscard]] std::span<const float> z0_col() const noexcept {
        return m_view.column<float>(h_z0);
    }
    /// d0 column as a contiguous span.
    [[nodiscard]] std::span<const float> d0_col() const noexcept {
        return m_view.column<float>(h_d0);
    }
    /// cov_d02 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_d02_col() const noexcept {
        return m_view.column<float>(h_cov_d02);
    }
    /// cov_d0phi0 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_d0phi0_col() const noexcept {
        return m_view.column<float>(h_cov_d0phi0);
    }
    /// cov_d0rho column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_d0rho_col() const noexcept {
        return m_view.column<float>(h_cov_d0rho);
    }
    /// cov_phi02 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_phi02_col() const noexcept {
        return m_view.column<float>(h_cov_phi02);
    }
    /// cov_phi0rho column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_phi0rho_col() const noexcept {
        return m_view.column<float>(h_cov_phi0rho);
    }
    /// cov_rho2 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_rho2_col() const noexcept {
        return m_view.column<float>(h_cov_rho2);
    }
    /// cov_z02 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_z02_col() const noexcept {
        return m_view.column<float>(h_cov_z02);
    }
    /// cov_z0tandip column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_z0tandip_col() const noexcept {
        return m_view.column<float>(h_cov_z0tandip);
    }
    /// cov_tandip2 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_tandip2_col() const noexcept {
        return m_view.column<float>(h_cov_tandip2);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
    }
    /// ndf column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ndf_col() const noexcept {
        return m_view.column<int16_t>(h_ndf);
    }
    /// pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pid_col() const noexcept {
        return m_view.column<int32_t>(h_pid);
    }
    /// seedID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> seedID_col() const noexcept {
        return m_view.column<int16_t>(h_seedID);
    }
    /// xb column as a contiguous span.
    [[nodiscard]] std::span<const float> xb_col() const noexcept {
        return m_view.column<float>(h_xb);
    }
    /// yb column as a contiguous span.
    [[nodiscard]] std::span<const float> yb_col() const noexcept {
        return m_view.column<float>(h_yb);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_ID{};
    hipo::column_handle h_q{};
    hipo::column_handle h_p{};
    hipo::column_handle h_pt{};
    hipo::column_handle h_phi0{};
    hipo::column_handle h_tandip{};
    hipo::column_handle h_z0{};
    hipo::column_handle h_d0{};
    hipo::column_handle h_cov_d02{};
    hipo::column_handle h_cov_d0phi0{};
    hipo::column_handle h_cov_d0rho{};
    hipo::column_handle h_cov_phi02{};
    hipo::column_handle h_cov_phi0rho{};
    hipo::column_handle h_cov_rho2{};
    hipo::column_handle h_cov_z02{};
    hipo::column_handle h_cov_z0tandip{};
    hipo::column_handle h_cov_tandip2{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_ndf{};
    hipo::column_handle h_pid{};
    hipo::column_handle h_seedID{};
    hipo::column_handle h_xb{};
    hipo::column_handle h_yb{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for CVT::UTracks. Stores a pointer + row index only.
class CVT_UTracks::Row {
 public:
    Row(const CVT_UTracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int q() const noexcept { return m_bank->q(m_row); }
    [[nodiscard]] float p() const noexcept { return m_bank->p(m_row); }
    [[nodiscard]] float pt() const noexcept { return m_bank->pt(m_row); }
    [[nodiscard]] float phi0() const noexcept { return m_bank->phi0(m_row); }
    [[nodiscard]] float tandip() const noexcept { return m_bank->tandip(m_row); }
    [[nodiscard]] float z0() const noexcept { return m_bank->z0(m_row); }
    [[nodiscard]] float d0() const noexcept { return m_bank->d0(m_row); }
    [[nodiscard]] float cov_d02() const noexcept { return m_bank->cov_d02(m_row); }
    [[nodiscard]] float cov_d0phi0() const noexcept { return m_bank->cov_d0phi0(m_row); }
    [[nodiscard]] float cov_d0rho() const noexcept { return m_bank->cov_d0rho(m_row); }
    [[nodiscard]] float cov_phi02() const noexcept { return m_bank->cov_phi02(m_row); }
    [[nodiscard]] float cov_phi0rho() const noexcept { return m_bank->cov_phi0rho(m_row); }
    [[nodiscard]] float cov_rho2() const noexcept { return m_bank->cov_rho2(m_row); }
    [[nodiscard]] float cov_z02() const noexcept { return m_bank->cov_z02(m_row); }
    [[nodiscard]] float cov_z0tandip() const noexcept { return m_bank->cov_z0tandip(m_row); }
    [[nodiscard]] float cov_tandip2() const noexcept { return m_bank->cov_tandip2(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int ndf() const noexcept { return m_bank->ndf(m_row); }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] int seedID() const noexcept { return m_bank->seedID(m_row); }
    [[nodiscard]] float xb() const noexcept { return m_bank->xb(m_row); }
    [[nodiscard]] float yb() const noexcept { return m_bank->yb(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const CVT_UTracks* m_bank;
    int m_row;
};

inline CVT_UTracks::Row CVT_UTracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
