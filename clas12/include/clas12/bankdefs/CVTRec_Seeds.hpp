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

namespace bankdefs {


/// HIPO bank wrapper for `CVTRec::Seeds` (group 20500, item 25).
/// first pass reconstructed CVT track seeds
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CVTRec_Seeds bank;
/// if (auto sc = bank.bind(event.get_bank("CVTRec::Seeds")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class CVTRec_Seeds {
 public:
    static constexpr std::string_view bank_name = "CVTRec::Seeds";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 25;

    class Row;

    CVTRec_Seeds() = default;

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
            hipo::column_handle h_ID;
            hipo::column_handle h_fittingMethod;
            hipo::column_handle h_q;
            hipo::column_handle h_p;
            hipo::column_handle h_pt;
            hipo::column_handle h_phi0;
            hipo::column_handle h_tandip;
            hipo::column_handle h_z0;
            hipo::column_handle h_d0;
            hipo::column_handle h_cov_d02;
            hipo::column_handle h_cov_d0phi0;
            hipo::column_handle h_cov_d0rho;
            hipo::column_handle h_cov_phi02;
            hipo::column_handle h_cov_phi0rho;
            hipo::column_handle h_cov_rho2;
            hipo::column_handle h_cov_z02;
            hipo::column_handle h_cov_z0tandip;
            hipo::column_handle h_cov_tandip2;
            hipo::column_handle h_circlefit_chi2_per_ndf;
            hipo::column_handle h_linefit_chi2_per_ndf;
            hipo::column_handle h_chi2;
            hipo::column_handle h_ndf;
            hipo::column_handle h_Cross1_ID;
            hipo::column_handle h_Cross2_ID;
            hipo::column_handle h_Cross3_ID;
            hipo::column_handle h_Cross4_ID;
            hipo::column_handle h_Cross5_ID;
            hipo::column_handle h_Cross6_ID;
            hipo::column_handle h_Cross7_ID;
            hipo::column_handle h_Cross8_ID;
            hipo::column_handle h_Cross9_ID;
            hipo::column_handle h_xb;
            hipo::column_handle h_yb;
            hipo::column_handle h_fracmctru;
            hipo::column_handle h_fracmcmatch;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("ID"); h) cache.h_ID = *h;
            else cache.h_ID = {};
            if (auto h = schema.get_handle("fittingMethod"); h) cache.h_fittingMethod = *h;
            else cache.h_fittingMethod = {};
            if (auto h = schema.get_handle("q"); h) cache.h_q = *h;
            else cache.h_q = {};
            if (auto h = schema.get_handle("p"); h) cache.h_p = *h;
            else cache.h_p = {};
            if (auto h = schema.get_handle("pt"); h) cache.h_pt = *h;
            else cache.h_pt = {};
            if (auto h = schema.get_handle("phi0"); h) cache.h_phi0 = *h;
            else cache.h_phi0 = {};
            if (auto h = schema.get_handle("tandip"); h) cache.h_tandip = *h;
            else cache.h_tandip = {};
            if (auto h = schema.get_handle("z0"); h) cache.h_z0 = *h;
            else cache.h_z0 = {};
            if (auto h = schema.get_handle("d0"); h) cache.h_d0 = *h;
            else cache.h_d0 = {};
            if (auto h = schema.get_handle("cov_d02"); h) cache.h_cov_d02 = *h;
            else cache.h_cov_d02 = {};
            if (auto h = schema.get_handle("cov_d0phi0"); h) cache.h_cov_d0phi0 = *h;
            else cache.h_cov_d0phi0 = {};
            if (auto h = schema.get_handle("cov_d0rho"); h) cache.h_cov_d0rho = *h;
            else cache.h_cov_d0rho = {};
            if (auto h = schema.get_handle("cov_phi02"); h) cache.h_cov_phi02 = *h;
            else cache.h_cov_phi02 = {};
            if (auto h = schema.get_handle("cov_phi0rho"); h) cache.h_cov_phi0rho = *h;
            else cache.h_cov_phi0rho = {};
            if (auto h = schema.get_handle("cov_rho2"); h) cache.h_cov_rho2 = *h;
            else cache.h_cov_rho2 = {};
            if (auto h = schema.get_handle("cov_z02"); h) cache.h_cov_z02 = *h;
            else cache.h_cov_z02 = {};
            if (auto h = schema.get_handle("cov_z0tandip"); h) cache.h_cov_z0tandip = *h;
            else cache.h_cov_z0tandip = {};
            if (auto h = schema.get_handle("cov_tandip2"); h) cache.h_cov_tandip2 = *h;
            else cache.h_cov_tandip2 = {};
            if (auto h = schema.get_handle("circlefit_chi2_per_ndf"); h) cache.h_circlefit_chi2_per_ndf = *h;
            else cache.h_circlefit_chi2_per_ndf = {};
            if (auto h = schema.get_handle("linefit_chi2_per_ndf"); h) cache.h_linefit_chi2_per_ndf = *h;
            else cache.h_linefit_chi2_per_ndf = {};
            if (auto h = schema.get_handle("chi2"); h) cache.h_chi2 = *h;
            else cache.h_chi2 = {};
            if (auto h = schema.get_handle("ndf"); h) cache.h_ndf = *h;
            else cache.h_ndf = {};
            if (auto h = schema.get_handle("Cross1_ID"); h) cache.h_Cross1_ID = *h;
            else cache.h_Cross1_ID = {};
            if (auto h = schema.get_handle("Cross2_ID"); h) cache.h_Cross2_ID = *h;
            else cache.h_Cross2_ID = {};
            if (auto h = schema.get_handle("Cross3_ID"); h) cache.h_Cross3_ID = *h;
            else cache.h_Cross3_ID = {};
            if (auto h = schema.get_handle("Cross4_ID"); h) cache.h_Cross4_ID = *h;
            else cache.h_Cross4_ID = {};
            if (auto h = schema.get_handle("Cross5_ID"); h) cache.h_Cross5_ID = *h;
            else cache.h_Cross5_ID = {};
            if (auto h = schema.get_handle("Cross6_ID"); h) cache.h_Cross6_ID = *h;
            else cache.h_Cross6_ID = {};
            if (auto h = schema.get_handle("Cross7_ID"); h) cache.h_Cross7_ID = *h;
            else cache.h_Cross7_ID = {};
            if (auto h = schema.get_handle("Cross8_ID"); h) cache.h_Cross8_ID = *h;
            else cache.h_Cross8_ID = {};
            if (auto h = schema.get_handle("Cross9_ID"); h) cache.h_Cross9_ID = *h;
            else cache.h_Cross9_ID = {};
            if (auto h = schema.get_handle("xb"); h) cache.h_xb = *h;
            else cache.h_xb = {};
            if (auto h = schema.get_handle("yb"); h) cache.h_yb = *h;
            else cache.h_yb = {};
            if (auto h = schema.get_handle("fracmctru"); h) cache.h_fracmctru = *h;
            else cache.h_fracmctru = {};
            if (auto h = schema.get_handle("fracmcmatch"); h) cache.h_fracmcmatch = *h;
            else cache.h_fracmcmatch = {};
        }
        h_ID = cache.h_ID;
        h_fittingMethod = cache.h_fittingMethod;
        h_q = cache.h_q;
        h_p = cache.h_p;
        h_pt = cache.h_pt;
        h_phi0 = cache.h_phi0;
        h_tandip = cache.h_tandip;
        h_z0 = cache.h_z0;
        h_d0 = cache.h_d0;
        h_cov_d02 = cache.h_cov_d02;
        h_cov_d0phi0 = cache.h_cov_d0phi0;
        h_cov_d0rho = cache.h_cov_d0rho;
        h_cov_phi02 = cache.h_cov_phi02;
        h_cov_phi0rho = cache.h_cov_phi0rho;
        h_cov_rho2 = cache.h_cov_rho2;
        h_cov_z02 = cache.h_cov_z02;
        h_cov_z0tandip = cache.h_cov_z0tandip;
        h_cov_tandip2 = cache.h_cov_tandip2;
        h_circlefit_chi2_per_ndf = cache.h_circlefit_chi2_per_ndf;
        h_linefit_chi2_per_ndf = cache.h_linefit_chi2_per_ndf;
        h_chi2 = cache.h_chi2;
        h_ndf = cache.h_ndf;
        h_Cross1_ID = cache.h_Cross1_ID;
        h_Cross2_ID = cache.h_Cross2_ID;
        h_Cross3_ID = cache.h_Cross3_ID;
        h_Cross4_ID = cache.h_Cross4_ID;
        h_Cross5_ID = cache.h_Cross5_ID;
        h_Cross6_ID = cache.h_Cross6_ID;
        h_Cross7_ID = cache.h_Cross7_ID;
        h_Cross8_ID = cache.h_Cross8_ID;
        h_Cross9_ID = cache.h_Cross9_ID;
        h_xb = cache.h_xb;
        h_yb = cache.h_yb;
        h_fracmctru = cache.h_fracmctru;
        h_fracmcmatch = cache.h_fracmcmatch;
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
    /// fittingMethod: fitting method (1 for global fit, 2 for Kalman Filter)
    [[nodiscard]] int fittingMethod(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_fittingMethod, row));
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
    /// circlefit_chi2_per_ndf: Circle Fit chi^2/ndf 
    [[nodiscard]] float circlefit_chi2_per_ndf(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_circlefit_chi2_per_ndf, row));
    }
    /// linefit_chi2_per_ndf:  Line Fit chi^2/ndf 
    [[nodiscard]] float linefit_chi2_per_ndf(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_linefit_chi2_per_ndf, row));
    }
    /// chi2: Fit chi^2 
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// ndf: Fit ndf 
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ndf, row));
    }
    /// Cross1_ID: ID of cross in the track
    [[nodiscard]] int Cross1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross1_ID, row));
    }
    /// Cross2_ID: ID of cross in the track
    [[nodiscard]] int Cross2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross2_ID, row));
    }
    /// Cross3_ID: ID of cross in the track
    [[nodiscard]] int Cross3_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross3_ID, row));
    }
    /// Cross4_ID: ID of cross in the track
    [[nodiscard]] int Cross4_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross4_ID, row));
    }
    /// Cross5_ID: ID of cross in the track
    [[nodiscard]] int Cross5_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross5_ID, row));
    }
    /// Cross6_ID: ID of cross in the track
    [[nodiscard]] int Cross6_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross6_ID, row));
    }
    /// Cross7_ID: ID of cross in the track
    [[nodiscard]] int Cross7_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross7_ID, row));
    }
    /// Cross8_ID: ID of cross in the track
    [[nodiscard]] int Cross8_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross8_ID, row));
    }
    /// Cross9_ID: ID of cross in the track
    [[nodiscard]] int Cross9_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross9_ID, row));
    }
    /// xb: x offset  
    [[nodiscard]] float xb(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_xb, row));
    }
    /// yb: y offset  
    [[nodiscard]] float yb(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_yb, row));
    }
    /// fracmctru: percentage of hits on track truth matched  
    [[nodiscard]] float fracmctru(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fracmctru, row));
    }
    /// fracmcmatch: percentage of hits on track truth matched over total nb true hits 
    [[nodiscard]] float fracmcmatch(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fracmcmatch, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ID_col() const noexcept {
        return m_view.column<int16_t>(h_ID);
    }
    /// fittingMethod column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> fittingMethod_col() const noexcept {
        return m_view.column<int8_t>(h_fittingMethod);
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
    /// circlefit_chi2_per_ndf column as a contiguous span.
    [[nodiscard]] std::span<const float> circlefit_chi2_per_ndf_col() const noexcept {
        return m_view.column<float>(h_circlefit_chi2_per_ndf);
    }
    /// linefit_chi2_per_ndf column as a contiguous span.
    [[nodiscard]] std::span<const float> linefit_chi2_per_ndf_col() const noexcept {
        return m_view.column<float>(h_linefit_chi2_per_ndf);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
    }
    /// ndf column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ndf_col() const noexcept {
        return m_view.column<int16_t>(h_ndf);
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
    /// Cross4_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross4_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross4_ID);
    }
    /// Cross5_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross5_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross5_ID);
    }
    /// Cross6_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross6_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross6_ID);
    }
    /// Cross7_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross7_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross7_ID);
    }
    /// Cross8_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross8_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross8_ID);
    }
    /// Cross9_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross9_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross9_ID);
    }
    /// xb column as a contiguous span.
    [[nodiscard]] std::span<const float> xb_col() const noexcept {
        return m_view.column<float>(h_xb);
    }
    /// yb column as a contiguous span.
    [[nodiscard]] std::span<const float> yb_col() const noexcept {
        return m_view.column<float>(h_yb);
    }
    /// fracmctru column as a contiguous span.
    [[nodiscard]] std::span<const float> fracmctru_col() const noexcept {
        return m_view.column<float>(h_fracmctru);
    }
    /// fracmcmatch column as a contiguous span.
    [[nodiscard]] std::span<const float> fracmcmatch_col() const noexcept {
        return m_view.column<float>(h_fracmcmatch);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_ID{};
    hipo::column_handle h_fittingMethod{};
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
    hipo::column_handle h_circlefit_chi2_per_ndf{};
    hipo::column_handle h_linefit_chi2_per_ndf{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_ndf{};
    hipo::column_handle h_Cross1_ID{};
    hipo::column_handle h_Cross2_ID{};
    hipo::column_handle h_Cross3_ID{};
    hipo::column_handle h_Cross4_ID{};
    hipo::column_handle h_Cross5_ID{};
    hipo::column_handle h_Cross6_ID{};
    hipo::column_handle h_Cross7_ID{};
    hipo::column_handle h_Cross8_ID{};
    hipo::column_handle h_Cross9_ID{};
    hipo::column_handle h_xb{};
    hipo::column_handle h_yb{};
    hipo::column_handle h_fracmctru{};
    hipo::column_handle h_fracmcmatch{};

    friend class Row;
};

/// Lightweight row view for CVTRec::Seeds. Stores a pointer + row index only.
class CVTRec_Seeds::Row {
 public:
    Row(const CVTRec_Seeds* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int fittingMethod() const noexcept { return m_bank->fittingMethod(m_row); }
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
    [[nodiscard]] float circlefit_chi2_per_ndf() const noexcept { return m_bank->circlefit_chi2_per_ndf(m_row); }
    [[nodiscard]] float linefit_chi2_per_ndf() const noexcept { return m_bank->linefit_chi2_per_ndf(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int ndf() const noexcept { return m_bank->ndf(m_row); }
    [[nodiscard]] int Cross1_ID() const noexcept { return m_bank->Cross1_ID(m_row); }
    [[nodiscard]] int Cross2_ID() const noexcept { return m_bank->Cross2_ID(m_row); }
    [[nodiscard]] int Cross3_ID() const noexcept { return m_bank->Cross3_ID(m_row); }
    [[nodiscard]] int Cross4_ID() const noexcept { return m_bank->Cross4_ID(m_row); }
    [[nodiscard]] int Cross5_ID() const noexcept { return m_bank->Cross5_ID(m_row); }
    [[nodiscard]] int Cross6_ID() const noexcept { return m_bank->Cross6_ID(m_row); }
    [[nodiscard]] int Cross7_ID() const noexcept { return m_bank->Cross7_ID(m_row); }
    [[nodiscard]] int Cross8_ID() const noexcept { return m_bank->Cross8_ID(m_row); }
    [[nodiscard]] int Cross9_ID() const noexcept { return m_bank->Cross9_ID(m_row); }
    [[nodiscard]] float xb() const noexcept { return m_bank->xb(m_row); }
    [[nodiscard]] float yb() const noexcept { return m_bank->yb(m_row); }
    [[nodiscard]] float fracmctru() const noexcept { return m_bank->fracmctru(m_row); }
    [[nodiscard]] float fracmcmatch() const noexcept { return m_bank->fracmcmatch(m_row); }
 private:
    const CVTRec_Seeds* m_bank;
    int m_row;
};

inline CVTRec_Seeds::Row CVTRec_Seeds::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
