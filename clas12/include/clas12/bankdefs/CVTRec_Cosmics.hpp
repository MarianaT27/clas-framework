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


/// HIPO bank wrapper for `CVTRec::Cosmics` (group 20500, item 30).
/// reconstructed SVT cosmic tracks
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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
            hipo::column_handle h_trkline_yx_slope;
            hipo::column_handle h_trkline_yx_interc;
            hipo::column_handle h_trkline_yz_slope;
            hipo::column_handle h_trkline_yz_interc;
            hipo::column_handle h_theta;
            hipo::column_handle h_phi;
            hipo::column_handle h_chi2;
            hipo::column_handle h_ndf;
            hipo::column_handle h_cov_x02;
            hipo::column_handle h_cov_x0z0;
            hipo::column_handle h_cov_x0tx;
            hipo::column_handle h_cov_x0tz;
            hipo::column_handle h_cov_z02;
            hipo::column_handle h_cov_z0tx;
            hipo::column_handle h_cov_z0tz;
            hipo::column_handle h_cov_tx2;
            hipo::column_handle h_cov_txtz;
            hipo::column_handle h_cov_tz2;
            hipo::column_handle h_Cross1_ID;
            hipo::column_handle h_Cross2_ID;
            hipo::column_handle h_Cross3_ID;
            hipo::column_handle h_Cross4_ID;
            hipo::column_handle h_Cross5_ID;
            hipo::column_handle h_Cross6_ID;
            hipo::column_handle h_Cross7_ID;
            hipo::column_handle h_Cross8_ID;
            hipo::column_handle h_Cross9_ID;
            hipo::column_handle h_Cross10_ID;
            hipo::column_handle h_Cross11_ID;
            hipo::column_handle h_Cross12_ID;
            hipo::column_handle h_Cross13_ID;
            hipo::column_handle h_Cross14_ID;
            hipo::column_handle h_Cross15_ID;
            hipo::column_handle h_Cross16_ID;
            hipo::column_handle h_Cross17_ID;
            hipo::column_handle h_Cross18_ID;
            hipo::column_handle h_status;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("ID"); h) cache.h_ID = *h;
            else cache.h_ID = {};
            if (auto h = schema.get_handle("trkline_yx_slope"); h) cache.h_trkline_yx_slope = *h;
            else cache.h_trkline_yx_slope = {};
            if (auto h = schema.get_handle("trkline_yx_interc"); h) cache.h_trkline_yx_interc = *h;
            else cache.h_trkline_yx_interc = {};
            if (auto h = schema.get_handle("trkline_yz_slope"); h) cache.h_trkline_yz_slope = *h;
            else cache.h_trkline_yz_slope = {};
            if (auto h = schema.get_handle("trkline_yz_interc"); h) cache.h_trkline_yz_interc = *h;
            else cache.h_trkline_yz_interc = {};
            if (auto h = schema.get_handle("theta"); h) cache.h_theta = *h;
            else cache.h_theta = {};
            if (auto h = schema.get_handle("phi"); h) cache.h_phi = *h;
            else cache.h_phi = {};
            if (auto h = schema.get_handle("chi2"); h) cache.h_chi2 = *h;
            else cache.h_chi2 = {};
            if (auto h = schema.get_handle("ndf"); h) cache.h_ndf = *h;
            else cache.h_ndf = {};
            if (auto h = schema.get_handle("cov_x02"); h) cache.h_cov_x02 = *h;
            else cache.h_cov_x02 = {};
            if (auto h = schema.get_handle("cov_x0z0"); h) cache.h_cov_x0z0 = *h;
            else cache.h_cov_x0z0 = {};
            if (auto h = schema.get_handle("cov_x0tx"); h) cache.h_cov_x0tx = *h;
            else cache.h_cov_x0tx = {};
            if (auto h = schema.get_handle("cov_x0tz"); h) cache.h_cov_x0tz = *h;
            else cache.h_cov_x0tz = {};
            if (auto h = schema.get_handle("cov_z02"); h) cache.h_cov_z02 = *h;
            else cache.h_cov_z02 = {};
            if (auto h = schema.get_handle("cov_z0tx"); h) cache.h_cov_z0tx = *h;
            else cache.h_cov_z0tx = {};
            if (auto h = schema.get_handle("cov_z0tz"); h) cache.h_cov_z0tz = *h;
            else cache.h_cov_z0tz = {};
            if (auto h = schema.get_handle("cov_tx2"); h) cache.h_cov_tx2 = *h;
            else cache.h_cov_tx2 = {};
            if (auto h = schema.get_handle("cov_txtz"); h) cache.h_cov_txtz = *h;
            else cache.h_cov_txtz = {};
            if (auto h = schema.get_handle("cov_tz2"); h) cache.h_cov_tz2 = *h;
            else cache.h_cov_tz2 = {};
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
            if (auto h = schema.get_handle("Cross10_ID"); h) cache.h_Cross10_ID = *h;
            else cache.h_Cross10_ID = {};
            if (auto h = schema.get_handle("Cross11_ID"); h) cache.h_Cross11_ID = *h;
            else cache.h_Cross11_ID = {};
            if (auto h = schema.get_handle("Cross12_ID"); h) cache.h_Cross12_ID = *h;
            else cache.h_Cross12_ID = {};
            if (auto h = schema.get_handle("Cross13_ID"); h) cache.h_Cross13_ID = *h;
            else cache.h_Cross13_ID = {};
            if (auto h = schema.get_handle("Cross14_ID"); h) cache.h_Cross14_ID = *h;
            else cache.h_Cross14_ID = {};
            if (auto h = schema.get_handle("Cross15_ID"); h) cache.h_Cross15_ID = *h;
            else cache.h_Cross15_ID = {};
            if (auto h = schema.get_handle("Cross16_ID"); h) cache.h_Cross16_ID = *h;
            else cache.h_Cross16_ID = {};
            if (auto h = schema.get_handle("Cross17_ID"); h) cache.h_Cross17_ID = *h;
            else cache.h_Cross17_ID = {};
            if (auto h = schema.get_handle("Cross18_ID"); h) cache.h_Cross18_ID = *h;
            else cache.h_Cross18_ID = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
        }
        h_ID = cache.h_ID;
        h_trkline_yx_slope = cache.h_trkline_yx_slope;
        h_trkline_yx_interc = cache.h_trkline_yx_interc;
        h_trkline_yz_slope = cache.h_trkline_yz_slope;
        h_trkline_yz_interc = cache.h_trkline_yz_interc;
        h_theta = cache.h_theta;
        h_phi = cache.h_phi;
        h_chi2 = cache.h_chi2;
        h_ndf = cache.h_ndf;
        h_cov_x02 = cache.h_cov_x02;
        h_cov_x0z0 = cache.h_cov_x0z0;
        h_cov_x0tx = cache.h_cov_x0tx;
        h_cov_x0tz = cache.h_cov_x0tz;
        h_cov_z02 = cache.h_cov_z02;
        h_cov_z0tx = cache.h_cov_z0tx;
        h_cov_z0tz = cache.h_cov_z0tz;
        h_cov_tx2 = cache.h_cov_tx2;
        h_cov_txtz = cache.h_cov_txtz;
        h_cov_tz2 = cache.h_cov_tz2;
        h_Cross1_ID = cache.h_Cross1_ID;
        h_Cross2_ID = cache.h_Cross2_ID;
        h_Cross3_ID = cache.h_Cross3_ID;
        h_Cross4_ID = cache.h_Cross4_ID;
        h_Cross5_ID = cache.h_Cross5_ID;
        h_Cross6_ID = cache.h_Cross6_ID;
        h_Cross7_ID = cache.h_Cross7_ID;
        h_Cross8_ID = cache.h_Cross8_ID;
        h_Cross9_ID = cache.h_Cross9_ID;
        h_Cross10_ID = cache.h_Cross10_ID;
        h_Cross11_ID = cache.h_Cross11_ID;
        h_Cross12_ID = cache.h_Cross12_ID;
        h_Cross13_ID = cache.h_Cross13_ID;
        h_Cross14_ID = cache.h_Cross14_ID;
        h_Cross15_ID = cache.h_Cross15_ID;
        h_Cross16_ID = cache.h_Cross16_ID;
        h_Cross17_ID = cache.h_Cross17_ID;
        h_Cross18_ID = cache.h_Cross18_ID;
        h_status = cache.h_status;
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
    /// trkline_yx_slope: trkline_yx_slope
    [[nodiscard]] float trkline_yx_slope(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trkline_yx_slope, row));
    }
    /// trkline_yx_interc: trkline_yx_interc
    [[nodiscard]] float trkline_yx_interc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trkline_yx_interc, row));
    }
    /// trkline_yz_slope: trkline_yz_slope
    [[nodiscard]] float trkline_yz_slope(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trkline_yz_slope, row));
    }
    /// trkline_yz_interc: trkline_yz_interc
    [[nodiscard]] float trkline_yz_interc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trkline_yz_interc, row));
    }
    /// theta: theta (deg)
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_theta, row));
    }
    /// phi: phi (deg)
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_phi, row));
    }
    /// chi2: Fit chi^2 
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// ndf: number of degrees of freedom used in the fit
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ndf, row));
    }
    /// cov_x02: covariance matrix element : delta_x0^2
    [[nodiscard]] float cov_x02(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_x02, row));
    }
    /// cov_x0z0: covariance matrix element : delta_x0z0
    [[nodiscard]] float cov_x0z0(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_x0z0, row));
    }
    /// cov_x0tx: covariance matrix element : delta_x0tx
    [[nodiscard]] float cov_x0tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_x0tx, row));
    }
    /// cov_x0tz: covariance matrix element : delta_x0tz
    [[nodiscard]] float cov_x0tz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_x0tz, row));
    }
    /// cov_z02: covariance matrix element : delta_z0^2
    [[nodiscard]] float cov_z02(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_z02, row));
    }
    /// cov_z0tx: covariance matrix element : delta_z0tz
    [[nodiscard]] float cov_z0tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_z0tx, row));
    }
    /// cov_z0tz: covariance matrix element : delta_z0tz
    [[nodiscard]] float cov_z0tz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_z0tz, row));
    }
    /// cov_tx2: covariance matrix element : delta_tx^2
    [[nodiscard]] float cov_tx2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_tx2, row));
    }
    /// cov_txtz: covariance matrix element : delta_txtz
    [[nodiscard]] float cov_txtz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_txtz, row));
    }
    /// cov_tz2: covariance matrix element : delta_tz^2
    [[nodiscard]] float cov_tz2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_tz2, row));
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
    /// Cross10_ID: ID of cross in the track
    [[nodiscard]] int Cross10_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross10_ID, row));
    }
    /// Cross11_ID: ID of cross in the track
    [[nodiscard]] int Cross11_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross11_ID, row));
    }
    /// Cross12_ID: ID of cross in the track
    [[nodiscard]] int Cross12_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross12_ID, row));
    }
    /// Cross13_ID: ID of cross in the track
    [[nodiscard]] int Cross13_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross13_ID, row));
    }
    /// Cross14_ID: ID of cross in the track
    [[nodiscard]] int Cross14_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross14_ID, row));
    }
    /// Cross15_ID: ID of cross in the track
    [[nodiscard]] int Cross15_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross15_ID, row));
    }
    /// Cross16_ID: ID of cross in the track
    [[nodiscard]] int Cross16_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross16_ID, row));
    }
    /// Cross17_ID: ID of cross in the track
    [[nodiscard]] int Cross17_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross17_ID, row));
    }
    /// Cross18_ID: ID of cross in the track
    [[nodiscard]] int Cross18_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Cross18_ID, row));
    }
    /// status: ID of cross in the track
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
    /// trkline_yx_slope column as a contiguous span.
    [[nodiscard]] std::span<const float> trkline_yx_slope_col() const noexcept {
        return m_view.column<float>(h_trkline_yx_slope);
    }
    /// trkline_yx_interc column as a contiguous span.
    [[nodiscard]] std::span<const float> trkline_yx_interc_col() const noexcept {
        return m_view.column<float>(h_trkline_yx_interc);
    }
    /// trkline_yz_slope column as a contiguous span.
    [[nodiscard]] std::span<const float> trkline_yz_slope_col() const noexcept {
        return m_view.column<float>(h_trkline_yz_slope);
    }
    /// trkline_yz_interc column as a contiguous span.
    [[nodiscard]] std::span<const float> trkline_yz_interc_col() const noexcept {
        return m_view.column<float>(h_trkline_yz_interc);
    }
    /// theta column as a contiguous span.
    [[nodiscard]] std::span<const float> theta_col() const noexcept {
        return m_view.column<float>(h_theta);
    }
    /// phi column as a contiguous span.
    [[nodiscard]] std::span<const float> phi_col() const noexcept {
        return m_view.column<float>(h_phi);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
    }
    /// ndf column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ndf_col() const noexcept {
        return m_view.column<int16_t>(h_ndf);
    }
    /// cov_x02 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_x02_col() const noexcept {
        return m_view.column<float>(h_cov_x02);
    }
    /// cov_x0z0 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_x0z0_col() const noexcept {
        return m_view.column<float>(h_cov_x0z0);
    }
    /// cov_x0tx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_x0tx_col() const noexcept {
        return m_view.column<float>(h_cov_x0tx);
    }
    /// cov_x0tz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_x0tz_col() const noexcept {
        return m_view.column<float>(h_cov_x0tz);
    }
    /// cov_z02 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_z02_col() const noexcept {
        return m_view.column<float>(h_cov_z02);
    }
    /// cov_z0tx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_z0tx_col() const noexcept {
        return m_view.column<float>(h_cov_z0tx);
    }
    /// cov_z0tz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_z0tz_col() const noexcept {
        return m_view.column<float>(h_cov_z0tz);
    }
    /// cov_tx2 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_tx2_col() const noexcept {
        return m_view.column<float>(h_cov_tx2);
    }
    /// cov_txtz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_txtz_col() const noexcept {
        return m_view.column<float>(h_cov_txtz);
    }
    /// cov_tz2 column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_tz2_col() const noexcept {
        return m_view.column<float>(h_cov_tz2);
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
    /// Cross10_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross10_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross10_ID);
    }
    /// Cross11_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross11_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross11_ID);
    }
    /// Cross12_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross12_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross12_ID);
    }
    /// Cross13_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross13_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross13_ID);
    }
    /// Cross14_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross14_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross14_ID);
    }
    /// Cross15_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross15_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross15_ID);
    }
    /// Cross16_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross16_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross16_ID);
    }
    /// Cross17_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross17_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross17_ID);
    }
    /// Cross18_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Cross18_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Cross18_ID);
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
    hipo::column_handle h_trkline_yx_slope{};
    hipo::column_handle h_trkline_yx_interc{};
    hipo::column_handle h_trkline_yz_slope{};
    hipo::column_handle h_trkline_yz_interc{};
    hipo::column_handle h_theta{};
    hipo::column_handle h_phi{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_ndf{};
    hipo::column_handle h_cov_x02{};
    hipo::column_handle h_cov_x0z0{};
    hipo::column_handle h_cov_x0tx{};
    hipo::column_handle h_cov_x0tz{};
    hipo::column_handle h_cov_z02{};
    hipo::column_handle h_cov_z0tx{};
    hipo::column_handle h_cov_z0tz{};
    hipo::column_handle h_cov_tx2{};
    hipo::column_handle h_cov_txtz{};
    hipo::column_handle h_cov_tz2{};
    hipo::column_handle h_Cross1_ID{};
    hipo::column_handle h_Cross2_ID{};
    hipo::column_handle h_Cross3_ID{};
    hipo::column_handle h_Cross4_ID{};
    hipo::column_handle h_Cross5_ID{};
    hipo::column_handle h_Cross6_ID{};
    hipo::column_handle h_Cross7_ID{};
    hipo::column_handle h_Cross8_ID{};
    hipo::column_handle h_Cross9_ID{};
    hipo::column_handle h_Cross10_ID{};
    hipo::column_handle h_Cross11_ID{};
    hipo::column_handle h_Cross12_ID{};
    hipo::column_handle h_Cross13_ID{};
    hipo::column_handle h_Cross14_ID{};
    hipo::column_handle h_Cross15_ID{};
    hipo::column_handle h_Cross16_ID{};
    hipo::column_handle h_Cross17_ID{};
    hipo::column_handle h_Cross18_ID{};
    hipo::column_handle h_status{};

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
