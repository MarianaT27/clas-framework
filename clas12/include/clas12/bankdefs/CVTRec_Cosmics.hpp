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


/// HIPO bank wrapper for `CVTRec::Cosmics` (group 20500, item 30).
/// reconstructed SVT cosmic tracks
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CVTRec_Cosmics bank;
/// if (auto sc = bank.bind(event.get_bank("CVTRec::Cosmics")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class CVTRec_Cosmics {
 public:
    static constexpr std::string_view bank_name = "CVTRec::Cosmics";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 30;

    class Row;

    CVTRec_Cosmics() = default;

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
            int i_trkline_yx_slope = -1;
            int i_trkline_yx_interc = -1;
            int i_trkline_yz_slope = -1;
            int i_trkline_yz_interc = -1;
            int i_theta = -1;
            int i_phi = -1;
            int i_chi2 = -1;
            int i_ndf = -1;
            int i_cov_x02 = -1;
            int i_cov_x0z0 = -1;
            int i_cov_x0tx = -1;
            int i_cov_x0tz = -1;
            int i_cov_z02 = -1;
            int i_cov_z0tx = -1;
            int i_cov_z0tz = -1;
            int i_cov_tx2 = -1;
            int i_cov_txtz = -1;
            int i_cov_tz2 = -1;
            int i_Cross1_ID = -1;
            int i_Cross2_ID = -1;
            int i_Cross3_ID = -1;
            int i_Cross4_ID = -1;
            int i_Cross5_ID = -1;
            int i_Cross6_ID = -1;
            int i_Cross7_ID = -1;
            int i_Cross8_ID = -1;
            int i_Cross9_ID = -1;
            int i_Cross10_ID = -1;
            int i_Cross11_ID = -1;
            int i_Cross12_ID = -1;
            int i_Cross13_ID = -1;
            int i_Cross14_ID = -1;
            int i_Cross15_ID = -1;
            int i_Cross16_ID = -1;
            int i_Cross17_ID = -1;
            int i_Cross18_ID = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_ID = schema.exists("ID") ? schema.getEntryOrder("ID") : -1;
            cache.i_trkline_yx_slope = schema.exists("trkline_yx_slope") ? schema.getEntryOrder("trkline_yx_slope") : -1;
            cache.i_trkline_yx_interc = schema.exists("trkline_yx_interc") ? schema.getEntryOrder("trkline_yx_interc") : -1;
            cache.i_trkline_yz_slope = schema.exists("trkline_yz_slope") ? schema.getEntryOrder("trkline_yz_slope") : -1;
            cache.i_trkline_yz_interc = schema.exists("trkline_yz_interc") ? schema.getEntryOrder("trkline_yz_interc") : -1;
            cache.i_theta = schema.exists("theta") ? schema.getEntryOrder("theta") : -1;
            cache.i_phi = schema.exists("phi") ? schema.getEntryOrder("phi") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_ndf = schema.exists("ndf") ? schema.getEntryOrder("ndf") : -1;
            cache.i_cov_x02 = schema.exists("cov_x02") ? schema.getEntryOrder("cov_x02") : -1;
            cache.i_cov_x0z0 = schema.exists("cov_x0z0") ? schema.getEntryOrder("cov_x0z0") : -1;
            cache.i_cov_x0tx = schema.exists("cov_x0tx") ? schema.getEntryOrder("cov_x0tx") : -1;
            cache.i_cov_x0tz = schema.exists("cov_x0tz") ? schema.getEntryOrder("cov_x0tz") : -1;
            cache.i_cov_z02 = schema.exists("cov_z02") ? schema.getEntryOrder("cov_z02") : -1;
            cache.i_cov_z0tx = schema.exists("cov_z0tx") ? schema.getEntryOrder("cov_z0tx") : -1;
            cache.i_cov_z0tz = schema.exists("cov_z0tz") ? schema.getEntryOrder("cov_z0tz") : -1;
            cache.i_cov_tx2 = schema.exists("cov_tx2") ? schema.getEntryOrder("cov_tx2") : -1;
            cache.i_cov_txtz = schema.exists("cov_txtz") ? schema.getEntryOrder("cov_txtz") : -1;
            cache.i_cov_tz2 = schema.exists("cov_tz2") ? schema.getEntryOrder("cov_tz2") : -1;
            cache.i_Cross1_ID = schema.exists("Cross1_ID") ? schema.getEntryOrder("Cross1_ID") : -1;
            cache.i_Cross2_ID = schema.exists("Cross2_ID") ? schema.getEntryOrder("Cross2_ID") : -1;
            cache.i_Cross3_ID = schema.exists("Cross3_ID") ? schema.getEntryOrder("Cross3_ID") : -1;
            cache.i_Cross4_ID = schema.exists("Cross4_ID") ? schema.getEntryOrder("Cross4_ID") : -1;
            cache.i_Cross5_ID = schema.exists("Cross5_ID") ? schema.getEntryOrder("Cross5_ID") : -1;
            cache.i_Cross6_ID = schema.exists("Cross6_ID") ? schema.getEntryOrder("Cross6_ID") : -1;
            cache.i_Cross7_ID = schema.exists("Cross7_ID") ? schema.getEntryOrder("Cross7_ID") : -1;
            cache.i_Cross8_ID = schema.exists("Cross8_ID") ? schema.getEntryOrder("Cross8_ID") : -1;
            cache.i_Cross9_ID = schema.exists("Cross9_ID") ? schema.getEntryOrder("Cross9_ID") : -1;
            cache.i_Cross10_ID = schema.exists("Cross10_ID") ? schema.getEntryOrder("Cross10_ID") : -1;
            cache.i_Cross11_ID = schema.exists("Cross11_ID") ? schema.getEntryOrder("Cross11_ID") : -1;
            cache.i_Cross12_ID = schema.exists("Cross12_ID") ? schema.getEntryOrder("Cross12_ID") : -1;
            cache.i_Cross13_ID = schema.exists("Cross13_ID") ? schema.getEntryOrder("Cross13_ID") : -1;
            cache.i_Cross14_ID = schema.exists("Cross14_ID") ? schema.getEntryOrder("Cross14_ID") : -1;
            cache.i_Cross15_ID = schema.exists("Cross15_ID") ? schema.getEntryOrder("Cross15_ID") : -1;
            cache.i_Cross16_ID = schema.exists("Cross16_ID") ? schema.getEntryOrder("Cross16_ID") : -1;
            cache.i_Cross17_ID = schema.exists("Cross17_ID") ? schema.getEntryOrder("Cross17_ID") : -1;
            cache.i_Cross18_ID = schema.exists("Cross18_ID") ? schema.getEntryOrder("Cross18_ID") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_ID = cache.i_ID;
        i_trkline_yx_slope = cache.i_trkline_yx_slope;
        i_trkline_yx_interc = cache.i_trkline_yx_interc;
        i_trkline_yz_slope = cache.i_trkline_yz_slope;
        i_trkline_yz_interc = cache.i_trkline_yz_interc;
        i_theta = cache.i_theta;
        i_phi = cache.i_phi;
        i_chi2 = cache.i_chi2;
        i_ndf = cache.i_ndf;
        i_cov_x02 = cache.i_cov_x02;
        i_cov_x0z0 = cache.i_cov_x0z0;
        i_cov_x0tx = cache.i_cov_x0tx;
        i_cov_x0tz = cache.i_cov_x0tz;
        i_cov_z02 = cache.i_cov_z02;
        i_cov_z0tx = cache.i_cov_z0tx;
        i_cov_z0tz = cache.i_cov_z0tz;
        i_cov_tx2 = cache.i_cov_tx2;
        i_cov_txtz = cache.i_cov_txtz;
        i_cov_tz2 = cache.i_cov_tz2;
        i_Cross1_ID = cache.i_Cross1_ID;
        i_Cross2_ID = cache.i_Cross2_ID;
        i_Cross3_ID = cache.i_Cross3_ID;
        i_Cross4_ID = cache.i_Cross4_ID;
        i_Cross5_ID = cache.i_Cross5_ID;
        i_Cross6_ID = cache.i_Cross6_ID;
        i_Cross7_ID = cache.i_Cross7_ID;
        i_Cross8_ID = cache.i_Cross8_ID;
        i_Cross9_ID = cache.i_Cross9_ID;
        i_Cross10_ID = cache.i_Cross10_ID;
        i_Cross11_ID = cache.i_Cross11_ID;
        i_Cross12_ID = cache.i_Cross12_ID;
        i_Cross13_ID = cache.i_Cross13_ID;
        i_Cross14_ID = cache.i_Cross14_ID;
        i_Cross15_ID = cache.i_Cross15_ID;
        i_Cross16_ID = cache.i_Cross16_ID;
        i_Cross17_ID = cache.i_Cross17_ID;
        i_Cross18_ID = cache.i_Cross18_ID;
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
    /// trkline_yx_slope: trkline_yx_slope
    [[nodiscard]] float trkline_yx_slope(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trkline_yx_slope, row));
    }
    /// trkline_yx_interc: trkline_yx_interc
    [[nodiscard]] float trkline_yx_interc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trkline_yx_interc, row));
    }
    /// trkline_yz_slope: trkline_yz_slope
    [[nodiscard]] float trkline_yz_slope(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trkline_yz_slope, row));
    }
    /// trkline_yz_interc: trkline_yz_interc
    [[nodiscard]] float trkline_yz_interc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trkline_yz_interc, row));
    }
    /// theta: theta (deg)
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_theta, row));
    }
    /// phi: phi (deg)
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_phi, row));
    }
    /// chi2: Fit chi^2 
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// ndf: number of degrees of freedom used in the fit
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ndf, row));
    }
    /// cov_x02: covariance matrix element : delta_x0^2
    [[nodiscard]] float cov_x02(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_x02, row));
    }
    /// cov_x0z0: covariance matrix element : delta_x0z0
    [[nodiscard]] float cov_x0z0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_x0z0, row));
    }
    /// cov_x0tx: covariance matrix element : delta_x0tx
    [[nodiscard]] float cov_x0tx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_x0tx, row));
    }
    /// cov_x0tz: covariance matrix element : delta_x0tz
    [[nodiscard]] float cov_x0tz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_x0tz, row));
    }
    /// cov_z02: covariance matrix element : delta_z0^2
    [[nodiscard]] float cov_z02(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_z02, row));
    }
    /// cov_z0tx: covariance matrix element : delta_z0tz
    [[nodiscard]] float cov_z0tx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_z0tx, row));
    }
    /// cov_z0tz: covariance matrix element : delta_z0tz
    [[nodiscard]] float cov_z0tz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_z0tz, row));
    }
    /// cov_tx2: covariance matrix element : delta_tx^2
    [[nodiscard]] float cov_tx2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_tx2, row));
    }
    /// cov_txtz: covariance matrix element : delta_txtz
    [[nodiscard]] float cov_txtz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_txtz, row));
    }
    /// cov_tz2: covariance matrix element : delta_tz^2
    [[nodiscard]] float cov_tz2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_tz2, row));
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
    /// Cross10_ID: ID of cross in the track
    [[nodiscard]] int Cross10_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross10_ID, row));
    }
    /// Cross11_ID: ID of cross in the track
    [[nodiscard]] int Cross11_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross11_ID, row));
    }
    /// Cross12_ID: ID of cross in the track
    [[nodiscard]] int Cross12_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross12_ID, row));
    }
    /// Cross13_ID: ID of cross in the track
    [[nodiscard]] int Cross13_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross13_ID, row));
    }
    /// Cross14_ID: ID of cross in the track
    [[nodiscard]] int Cross14_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross14_ID, row));
    }
    /// Cross15_ID: ID of cross in the track
    [[nodiscard]] int Cross15_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross15_ID, row));
    }
    /// Cross16_ID: ID of cross in the track
    [[nodiscard]] int Cross16_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross16_ID, row));
    }
    /// Cross17_ID: ID of cross in the track
    [[nodiscard]] int Cross17_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross17_ID, row));
    }
    /// Cross18_ID: ID of cross in the track
    [[nodiscard]] int Cross18_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross18_ID, row));
    }
    /// status: ID of cross in the track
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_ID = -1;
    int i_trkline_yx_slope = -1;
    int i_trkline_yx_interc = -1;
    int i_trkline_yz_slope = -1;
    int i_trkline_yz_interc = -1;
    int i_theta = -1;
    int i_phi = -1;
    int i_chi2 = -1;
    int i_ndf = -1;
    int i_cov_x02 = -1;
    int i_cov_x0z0 = -1;
    int i_cov_x0tx = -1;
    int i_cov_x0tz = -1;
    int i_cov_z02 = -1;
    int i_cov_z0tx = -1;
    int i_cov_z0tz = -1;
    int i_cov_tx2 = -1;
    int i_cov_txtz = -1;
    int i_cov_tz2 = -1;
    int i_Cross1_ID = -1;
    int i_Cross2_ID = -1;
    int i_Cross3_ID = -1;
    int i_Cross4_ID = -1;
    int i_Cross5_ID = -1;
    int i_Cross6_ID = -1;
    int i_Cross7_ID = -1;
    int i_Cross8_ID = -1;
    int i_Cross9_ID = -1;
    int i_Cross10_ID = -1;
    int i_Cross11_ID = -1;
    int i_Cross12_ID = -1;
    int i_Cross13_ID = -1;
    int i_Cross14_ID = -1;
    int i_Cross15_ID = -1;
    int i_Cross16_ID = -1;
    int i_Cross17_ID = -1;
    int i_Cross18_ID = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for CVTRec::Cosmics. Stores a pointer + row index only.
class CVTRec_Cosmics::Row {
 public:
    Row(const CVTRec_Cosmics* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] float trkline_yx_slope() const noexcept { return m_bank->trkline_yx_slope(m_row); }
    [[nodiscard]] float trkline_yx_interc() const noexcept { return m_bank->trkline_yx_interc(m_row); }
    [[nodiscard]] float trkline_yz_slope() const noexcept { return m_bank->trkline_yz_slope(m_row); }
    [[nodiscard]] float trkline_yz_interc() const noexcept { return m_bank->trkline_yz_interc(m_row); }
    [[nodiscard]] float theta() const noexcept { return m_bank->theta(m_row); }
    [[nodiscard]] float phi() const noexcept { return m_bank->phi(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int ndf() const noexcept { return m_bank->ndf(m_row); }
    [[nodiscard]] float cov_x02() const noexcept { return m_bank->cov_x02(m_row); }
    [[nodiscard]] float cov_x0z0() const noexcept { return m_bank->cov_x0z0(m_row); }
    [[nodiscard]] float cov_x0tx() const noexcept { return m_bank->cov_x0tx(m_row); }
    [[nodiscard]] float cov_x0tz() const noexcept { return m_bank->cov_x0tz(m_row); }
    [[nodiscard]] float cov_z02() const noexcept { return m_bank->cov_z02(m_row); }
    [[nodiscard]] float cov_z0tx() const noexcept { return m_bank->cov_z0tx(m_row); }
    [[nodiscard]] float cov_z0tz() const noexcept { return m_bank->cov_z0tz(m_row); }
    [[nodiscard]] float cov_tx2() const noexcept { return m_bank->cov_tx2(m_row); }
    [[nodiscard]] float cov_txtz() const noexcept { return m_bank->cov_txtz(m_row); }
    [[nodiscard]] float cov_tz2() const noexcept { return m_bank->cov_tz2(m_row); }
    [[nodiscard]] int Cross1_ID() const noexcept { return m_bank->Cross1_ID(m_row); }
    [[nodiscard]] int Cross2_ID() const noexcept { return m_bank->Cross2_ID(m_row); }
    [[nodiscard]] int Cross3_ID() const noexcept { return m_bank->Cross3_ID(m_row); }
    [[nodiscard]] int Cross4_ID() const noexcept { return m_bank->Cross4_ID(m_row); }
    [[nodiscard]] int Cross5_ID() const noexcept { return m_bank->Cross5_ID(m_row); }
    [[nodiscard]] int Cross6_ID() const noexcept { return m_bank->Cross6_ID(m_row); }
    [[nodiscard]] int Cross7_ID() const noexcept { return m_bank->Cross7_ID(m_row); }
    [[nodiscard]] int Cross8_ID() const noexcept { return m_bank->Cross8_ID(m_row); }
    [[nodiscard]] int Cross9_ID() const noexcept { return m_bank->Cross9_ID(m_row); }
    [[nodiscard]] int Cross10_ID() const noexcept { return m_bank->Cross10_ID(m_row); }
    [[nodiscard]] int Cross11_ID() const noexcept { return m_bank->Cross11_ID(m_row); }
    [[nodiscard]] int Cross12_ID() const noexcept { return m_bank->Cross12_ID(m_row); }
    [[nodiscard]] int Cross13_ID() const noexcept { return m_bank->Cross13_ID(m_row); }
    [[nodiscard]] int Cross14_ID() const noexcept { return m_bank->Cross14_ID(m_row); }
    [[nodiscard]] int Cross15_ID() const noexcept { return m_bank->Cross15_ID(m_row); }
    [[nodiscard]] int Cross16_ID() const noexcept { return m_bank->Cross16_ID(m_row); }
    [[nodiscard]] int Cross17_ID() const noexcept { return m_bank->Cross17_ID(m_row); }
    [[nodiscard]] int Cross18_ID() const noexcept { return m_bank->Cross18_ID(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const CVTRec_Cosmics* m_bank;
    int m_row;
};

inline CVTRec_Cosmics::Row CVTRec_Cosmics::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
