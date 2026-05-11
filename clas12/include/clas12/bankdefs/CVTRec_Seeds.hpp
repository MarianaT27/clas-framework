// =============================================================================
// Auto-generated from cvt.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `CVTRec::Seeds` (group 20500, item 25).
/// first pass reconstructed CVT track seeds
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
            int i_ID = -1;
            int i_fittingMethod = -1;
            int i_q = -1;
            int i_p = -1;
            int i_pt = -1;
            int i_phi0 = -1;
            int i_tandip = -1;
            int i_z0 = -1;
            int i_d0 = -1;
            int i_cov_d02 = -1;
            int i_cov_d0phi0 = -1;
            int i_cov_d0rho = -1;
            int i_cov_phi02 = -1;
            int i_cov_phi0rho = -1;
            int i_cov_rho2 = -1;
            int i_cov_z02 = -1;
            int i_cov_z0tandip = -1;
            int i_cov_tandip2 = -1;
            int i_circlefit_chi2_per_ndf = -1;
            int i_linefit_chi2_per_ndf = -1;
            int i_chi2 = -1;
            int i_ndf = -1;
            int i_Cross1_ID = -1;
            int i_Cross2_ID = -1;
            int i_Cross3_ID = -1;
            int i_Cross4_ID = -1;
            int i_Cross5_ID = -1;
            int i_Cross6_ID = -1;
            int i_Cross7_ID = -1;
            int i_Cross8_ID = -1;
            int i_Cross9_ID = -1;
            int i_xb = -1;
            int i_yb = -1;
            int i_fracmctru = -1;
            int i_fracmcmatch = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_ID = schema.exists("ID") ? schema.getEntryOrder("ID") : -1;
            cache.i_fittingMethod = schema.exists("fittingMethod") ? schema.getEntryOrder("fittingMethod") : -1;
            cache.i_q = schema.exists("q") ? schema.getEntryOrder("q") : -1;
            cache.i_p = schema.exists("p") ? schema.getEntryOrder("p") : -1;
            cache.i_pt = schema.exists("pt") ? schema.getEntryOrder("pt") : -1;
            cache.i_phi0 = schema.exists("phi0") ? schema.getEntryOrder("phi0") : -1;
            cache.i_tandip = schema.exists("tandip") ? schema.getEntryOrder("tandip") : -1;
            cache.i_z0 = schema.exists("z0") ? schema.getEntryOrder("z0") : -1;
            cache.i_d0 = schema.exists("d0") ? schema.getEntryOrder("d0") : -1;
            cache.i_cov_d02 = schema.exists("cov_d02") ? schema.getEntryOrder("cov_d02") : -1;
            cache.i_cov_d0phi0 = schema.exists("cov_d0phi0") ? schema.getEntryOrder("cov_d0phi0") : -1;
            cache.i_cov_d0rho = schema.exists("cov_d0rho") ? schema.getEntryOrder("cov_d0rho") : -1;
            cache.i_cov_phi02 = schema.exists("cov_phi02") ? schema.getEntryOrder("cov_phi02") : -1;
            cache.i_cov_phi0rho = schema.exists("cov_phi0rho") ? schema.getEntryOrder("cov_phi0rho") : -1;
            cache.i_cov_rho2 = schema.exists("cov_rho2") ? schema.getEntryOrder("cov_rho2") : -1;
            cache.i_cov_z02 = schema.exists("cov_z02") ? schema.getEntryOrder("cov_z02") : -1;
            cache.i_cov_z0tandip = schema.exists("cov_z0tandip") ? schema.getEntryOrder("cov_z0tandip") : -1;
            cache.i_cov_tandip2 = schema.exists("cov_tandip2") ? schema.getEntryOrder("cov_tandip2") : -1;
            cache.i_circlefit_chi2_per_ndf = schema.exists("circlefit_chi2_per_ndf") ? schema.getEntryOrder("circlefit_chi2_per_ndf") : -1;
            cache.i_linefit_chi2_per_ndf = schema.exists("linefit_chi2_per_ndf") ? schema.getEntryOrder("linefit_chi2_per_ndf") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_ndf = schema.exists("ndf") ? schema.getEntryOrder("ndf") : -1;
            cache.i_Cross1_ID = schema.exists("Cross1_ID") ? schema.getEntryOrder("Cross1_ID") : -1;
            cache.i_Cross2_ID = schema.exists("Cross2_ID") ? schema.getEntryOrder("Cross2_ID") : -1;
            cache.i_Cross3_ID = schema.exists("Cross3_ID") ? schema.getEntryOrder("Cross3_ID") : -1;
            cache.i_Cross4_ID = schema.exists("Cross4_ID") ? schema.getEntryOrder("Cross4_ID") : -1;
            cache.i_Cross5_ID = schema.exists("Cross5_ID") ? schema.getEntryOrder("Cross5_ID") : -1;
            cache.i_Cross6_ID = schema.exists("Cross6_ID") ? schema.getEntryOrder("Cross6_ID") : -1;
            cache.i_Cross7_ID = schema.exists("Cross7_ID") ? schema.getEntryOrder("Cross7_ID") : -1;
            cache.i_Cross8_ID = schema.exists("Cross8_ID") ? schema.getEntryOrder("Cross8_ID") : -1;
            cache.i_Cross9_ID = schema.exists("Cross9_ID") ? schema.getEntryOrder("Cross9_ID") : -1;
            cache.i_xb = schema.exists("xb") ? schema.getEntryOrder("xb") : -1;
            cache.i_yb = schema.exists("yb") ? schema.getEntryOrder("yb") : -1;
            cache.i_fracmctru = schema.exists("fracmctru") ? schema.getEntryOrder("fracmctru") : -1;
            cache.i_fracmcmatch = schema.exists("fracmcmatch") ? schema.getEntryOrder("fracmcmatch") : -1;
        }
        i_ID = cache.i_ID;
        i_fittingMethod = cache.i_fittingMethod;
        i_q = cache.i_q;
        i_p = cache.i_p;
        i_pt = cache.i_pt;
        i_phi0 = cache.i_phi0;
        i_tandip = cache.i_tandip;
        i_z0 = cache.i_z0;
        i_d0 = cache.i_d0;
        i_cov_d02 = cache.i_cov_d02;
        i_cov_d0phi0 = cache.i_cov_d0phi0;
        i_cov_d0rho = cache.i_cov_d0rho;
        i_cov_phi02 = cache.i_cov_phi02;
        i_cov_phi0rho = cache.i_cov_phi0rho;
        i_cov_rho2 = cache.i_cov_rho2;
        i_cov_z02 = cache.i_cov_z02;
        i_cov_z0tandip = cache.i_cov_z0tandip;
        i_cov_tandip2 = cache.i_cov_tandip2;
        i_circlefit_chi2_per_ndf = cache.i_circlefit_chi2_per_ndf;
        i_linefit_chi2_per_ndf = cache.i_linefit_chi2_per_ndf;
        i_chi2 = cache.i_chi2;
        i_ndf = cache.i_ndf;
        i_Cross1_ID = cache.i_Cross1_ID;
        i_Cross2_ID = cache.i_Cross2_ID;
        i_Cross3_ID = cache.i_Cross3_ID;
        i_Cross4_ID = cache.i_Cross4_ID;
        i_Cross5_ID = cache.i_Cross5_ID;
        i_Cross6_ID = cache.i_Cross6_ID;
        i_Cross7_ID = cache.i_Cross7_ID;
        i_Cross8_ID = cache.i_Cross8_ID;
        i_Cross9_ID = cache.i_Cross9_ID;
        i_xb = cache.i_xb;
        i_yb = cache.i_yb;
        i_fracmctru = cache.i_fracmctru;
        i_fracmcmatch = cache.i_fracmcmatch;
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
    /// ID: ID
    [[nodiscard]] int ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ID, row));
    }
    /// fittingMethod: fitting method (1 for global fit, 2 for Kalman Filter)
    [[nodiscard]] int fittingMethod(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_fittingMethod, row));
    }
    /// q: charge
    [[nodiscard]] int q(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_q, row));
    }
    /// p: total momentum
    [[nodiscard]] float p(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p, row));
    }
    /// pt: transverse momentum
    [[nodiscard]] float pt(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pt, row));
    }
    /// phi0: helical track fit parameter: phi at DOCA
    [[nodiscard]] float phi0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_phi0, row));
    }
    /// tandip: helical track fit parameter: dip angle
    [[nodiscard]] float tandip(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tandip, row));
    }
    /// z0: helical track fit parameter: value of z at the DOCA
    [[nodiscard]] float z0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z0, row));
    }
    /// d0: helical track fit parameter: Distance of Closest Approach
    [[nodiscard]] float d0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_d0, row));
    }
    /// cov_d02: helical track fit covariance matrix element : delta_d0^2
    [[nodiscard]] float cov_d02(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_d02, row));
    }
    /// cov_d0phi0: helical track fit covariance matrix element : delta_d0.delta_phi0
    [[nodiscard]] float cov_d0phi0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_d0phi0, row));
    }
    /// cov_d0rho: helical track fit covariance matrix element : delta_d0.delta_rho
    [[nodiscard]] float cov_d0rho(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_d0rho, row));
    }
    /// cov_phi02: helical track fit covariance matrix element : delta_phi0^2
    [[nodiscard]] float cov_phi02(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_phi02, row));
    }
    /// cov_phi0rho: helical track fit covariance matrix element : delta_phi0.delta_rho
    [[nodiscard]] float cov_phi0rho(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_phi0rho, row));
    }
    /// cov_rho2: helical track fit covariance matrix element : delta_rho.delta_rho
    [[nodiscard]] float cov_rho2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_rho2, row));
    }
    /// cov_z02: helical track fit covariance matrix element : delta_z0^2
    [[nodiscard]] float cov_z02(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_z02, row));
    }
    /// cov_z0tandip: helical track fit covariance matrix element : delta_z0.delta_tandip
    [[nodiscard]] float cov_z0tandip(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_z0tandip, row));
    }
    /// cov_tandip2: helical track fit covariance matrix element : delta_tandip^2
    [[nodiscard]] float cov_tandip2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_tandip2, row));
    }
    /// circlefit_chi2_per_ndf: Circle Fit chi^2/ndf 
    [[nodiscard]] float circlefit_chi2_per_ndf(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_circlefit_chi2_per_ndf, row));
    }
    /// linefit_chi2_per_ndf:  Line Fit chi^2/ndf 
    [[nodiscard]] float linefit_chi2_per_ndf(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_linefit_chi2_per_ndf, row));
    }
    /// chi2: Fit chi^2 
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// ndf: Fit ndf 
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ndf, row));
    }
    /// Cross1_ID: ID of cross in the track
    [[nodiscard]] int Cross1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross1_ID, row));
    }
    /// Cross2_ID: ID of cross in the track
    [[nodiscard]] int Cross2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross2_ID, row));
    }
    /// Cross3_ID: ID of cross in the track
    [[nodiscard]] int Cross3_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross3_ID, row));
    }
    /// Cross4_ID: ID of cross in the track
    [[nodiscard]] int Cross4_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross4_ID, row));
    }
    /// Cross5_ID: ID of cross in the track
    [[nodiscard]] int Cross5_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross5_ID, row));
    }
    /// Cross6_ID: ID of cross in the track
    [[nodiscard]] int Cross6_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross6_ID, row));
    }
    /// Cross7_ID: ID of cross in the track
    [[nodiscard]] int Cross7_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross7_ID, row));
    }
    /// Cross8_ID: ID of cross in the track
    [[nodiscard]] int Cross8_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross8_ID, row));
    }
    /// Cross9_ID: ID of cross in the track
    [[nodiscard]] int Cross9_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross9_ID, row));
    }
    /// xb: x offset  
    [[nodiscard]] float xb(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_xb, row));
    }
    /// yb: y offset  
    [[nodiscard]] float yb(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_yb, row));
    }
    /// fracmctru: percentage of hits on track truth matched  
    [[nodiscard]] float fracmctru(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fracmctru, row));
    }
    /// fracmcmatch: percentage of hits on track truth matched over total nb true hits 
    [[nodiscard]] float fracmcmatch(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fracmcmatch, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_ID = -1;
    int i_fittingMethod = -1;
    int i_q = -1;
    int i_p = -1;
    int i_pt = -1;
    int i_phi0 = -1;
    int i_tandip = -1;
    int i_z0 = -1;
    int i_d0 = -1;
    int i_cov_d02 = -1;
    int i_cov_d0phi0 = -1;
    int i_cov_d0rho = -1;
    int i_cov_phi02 = -1;
    int i_cov_phi0rho = -1;
    int i_cov_rho2 = -1;
    int i_cov_z02 = -1;
    int i_cov_z0tandip = -1;
    int i_cov_tandip2 = -1;
    int i_circlefit_chi2_per_ndf = -1;
    int i_linefit_chi2_per_ndf = -1;
    int i_chi2 = -1;
    int i_ndf = -1;
    int i_Cross1_ID = -1;
    int i_Cross2_ID = -1;
    int i_Cross3_ID = -1;
    int i_Cross4_ID = -1;
    int i_Cross5_ID = -1;
    int i_Cross6_ID = -1;
    int i_Cross7_ID = -1;
    int i_Cross8_ID = -1;
    int i_Cross9_ID = -1;
    int i_xb = -1;
    int i_yb = -1;
    int i_fracmctru = -1;
    int i_fracmcmatch = -1;

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
