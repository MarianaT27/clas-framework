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


/// HIPO bank wrapper for `CVT::UTracks` (group 20500, item 37).
/// reconstructed CVT tracks
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
    static constexpr std::string_view bank_schema = "ID/S,q/B,p/F,pt/F,phi0/F,tandip/F,z0/F,d0/F,cov_d02/F,cov_d0phi0/F,cov_d0rho/F,cov_phi02/F,cov_phi0rho/F,cov_rho2/F,cov_z02/F,cov_z0tandip/F,cov_tandip2/F,chi2/F,ndf/S,pid/I,seedID/S,xb/F,yb/F,status/S";

    class Row;

    CVT_UTracks() = default;

    /// Bind to a live hipo::bank. Per-thread cache keyed by the schema's
    /// (group, item): the first bind() in a thread resolves every column
    /// name to its item index via schema.getEntryOrder(); subsequent
    /// bind()s hit the cache. Keying by (group,item) rather than the schema
    /// POINTER is critical: the framework constructs a fresh hipo::bank
    /// (copying its schema to a new address) every event, so a pointer key
    /// would miss every event and re-resolve all columns by string. (group,
    /// item) is stable across those copies, so resolution is paid once per
    /// thread per bank type. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            int gi_group = -1;
            int gi_item  = -2;
            int i_ID = -1;
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
            int i_chi2 = -1;
            int i_ndf = -1;
            int i_pid = -1;
            int i_seedID = -1;
            int i_xb = -1;
            int i_yb = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_ID = schema.exists("ID") ? schema.getEntryOrder("ID") : -1;
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
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_ndf = schema.exists("ndf") ? schema.getEntryOrder("ndf") : -1;
            cache.i_pid = schema.exists("pid") ? schema.getEntryOrder("pid") : -1;
            cache.i_seedID = schema.exists("seedID") ? schema.getEntryOrder("seedID") : -1;
            cache.i_xb = schema.exists("xb") ? schema.getEntryOrder("xb") : -1;
            cache.i_yb = schema.exists("yb") ? schema.getEntryOrder("yb") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_ID = cache.i_ID;
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
        i_chi2 = cache.i_chi2;
        i_ndf = cache.i_ndf;
        i_pid = cache.i_pid;
        i_seedID = cache.i_seedID;
        i_xb = cache.i_xb;
        i_yb = cache.i_yb;
        i_status = cache.i_status;
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
    /// chi2: Fit chi^2 
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// ndf: Fit ndf 
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ndf, row));
    }
    /// pid: PID assumption
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pid, row));
    }
    /// seedID: ID of track seed
    [[nodiscard]] int seedID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_seedID, row));
    }
    /// xb: x offset  
    [[nodiscard]] float xb(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_xb, row));
    }
    /// yb: y offset  
    [[nodiscard]] float yb(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_yb, row));
    }
    /// status: track status : nabc (n = KF iterations a = nb SVT crosses, b = nb BMT Z clusters, c = nb BMT C clusters, used in fit)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_ID = -1;
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
    int i_chi2 = -1;
    int i_ndf = -1;
    int i_pid = -1;
    int i_seedID = -1;
    int i_xb = -1;
    int i_yb = -1;
    int i_status = -1;

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

}  // namespace bankdefs
