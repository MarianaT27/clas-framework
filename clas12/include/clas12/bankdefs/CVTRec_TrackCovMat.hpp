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


/// HIPO bank wrapper for `CVTRec::TrackCovMat` (group 20500, item 28).
/// reconstructed CVT tracks covariance matrix in track representation
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CVTRec_TrackCovMat bank;
/// if (auto sc = bank.bind(event.get_bank("CVTRec::TrackCovMat")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class CVTRec_TrackCovMat {
 public:
    static constexpr std::string_view bank_name = "CVTRec::TrackCovMat";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 28;

    class Row;

    CVTRec_TrackCovMat() = default;

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
            hipo::column_handle h_cov_xx;
            hipo::column_handle h_cov_xy;
            hipo::column_handle h_cov_xz;
            hipo::column_handle h_cov_xpx;
            hipo::column_handle h_cov_xpy;
            hipo::column_handle h_cov_xpz;
            hipo::column_handle h_cov_yx;
            hipo::column_handle h_cov_yy;
            hipo::column_handle h_cov_yz;
            hipo::column_handle h_cov_ypx;
            hipo::column_handle h_cov_ypy;
            hipo::column_handle h_cov_ypz;
            hipo::column_handle h_cov_zx;
            hipo::column_handle h_cov_zy;
            hipo::column_handle h_cov_zz;
            hipo::column_handle h_cov_zpx;
            hipo::column_handle h_cov_zpy;
            hipo::column_handle h_cov_zpz;
            hipo::column_handle h_cov_pxx;
            hipo::column_handle h_cov_pxy;
            hipo::column_handle h_cov_pxz;
            hipo::column_handle h_cov_pxpx;
            hipo::column_handle h_cov_pxpy;
            hipo::column_handle h_cov_pxpz;
            hipo::column_handle h_cov_pyx;
            hipo::column_handle h_cov_pyy;
            hipo::column_handle h_cov_pyz;
            hipo::column_handle h_cov_pypx;
            hipo::column_handle h_cov_pypy;
            hipo::column_handle h_cov_pypz;
            hipo::column_handle h_cov_pzx;
            hipo::column_handle h_cov_pzy;
            hipo::column_handle h_cov_pzz;
            hipo::column_handle h_cov_pzpx;
            hipo::column_handle h_cov_pzpy;
            hipo::column_handle h_cov_pzpz;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("ID"); h) cache.h_ID = *h;
            else cache.h_ID = {};
            if (auto h = schema.get_handle("cov_xx"); h) cache.h_cov_xx = *h;
            else cache.h_cov_xx = {};
            if (auto h = schema.get_handle("cov_xy"); h) cache.h_cov_xy = *h;
            else cache.h_cov_xy = {};
            if (auto h = schema.get_handle("cov_xz"); h) cache.h_cov_xz = *h;
            else cache.h_cov_xz = {};
            if (auto h = schema.get_handle("cov_xpx"); h) cache.h_cov_xpx = *h;
            else cache.h_cov_xpx = {};
            if (auto h = schema.get_handle("cov_xpy"); h) cache.h_cov_xpy = *h;
            else cache.h_cov_xpy = {};
            if (auto h = schema.get_handle("cov_xpz"); h) cache.h_cov_xpz = *h;
            else cache.h_cov_xpz = {};
            if (auto h = schema.get_handle("cov_yx"); h) cache.h_cov_yx = *h;
            else cache.h_cov_yx = {};
            if (auto h = schema.get_handle("cov_yy"); h) cache.h_cov_yy = *h;
            else cache.h_cov_yy = {};
            if (auto h = schema.get_handle("cov_yz"); h) cache.h_cov_yz = *h;
            else cache.h_cov_yz = {};
            if (auto h = schema.get_handle("cov_ypx"); h) cache.h_cov_ypx = *h;
            else cache.h_cov_ypx = {};
            if (auto h = schema.get_handle("cov_ypy"); h) cache.h_cov_ypy = *h;
            else cache.h_cov_ypy = {};
            if (auto h = schema.get_handle("cov_ypz"); h) cache.h_cov_ypz = *h;
            else cache.h_cov_ypz = {};
            if (auto h = schema.get_handle("cov_zx"); h) cache.h_cov_zx = *h;
            else cache.h_cov_zx = {};
            if (auto h = schema.get_handle("cov_zy"); h) cache.h_cov_zy = *h;
            else cache.h_cov_zy = {};
            if (auto h = schema.get_handle("cov_zz"); h) cache.h_cov_zz = *h;
            else cache.h_cov_zz = {};
            if (auto h = schema.get_handle("cov_zpx"); h) cache.h_cov_zpx = *h;
            else cache.h_cov_zpx = {};
            if (auto h = schema.get_handle("cov_zpy"); h) cache.h_cov_zpy = *h;
            else cache.h_cov_zpy = {};
            if (auto h = schema.get_handle("cov_zpz"); h) cache.h_cov_zpz = *h;
            else cache.h_cov_zpz = {};
            if (auto h = schema.get_handle("cov_pxx"); h) cache.h_cov_pxx = *h;
            else cache.h_cov_pxx = {};
            if (auto h = schema.get_handle("cov_pxy"); h) cache.h_cov_pxy = *h;
            else cache.h_cov_pxy = {};
            if (auto h = schema.get_handle("cov_pxz"); h) cache.h_cov_pxz = *h;
            else cache.h_cov_pxz = {};
            if (auto h = schema.get_handle("cov_pxpx"); h) cache.h_cov_pxpx = *h;
            else cache.h_cov_pxpx = {};
            if (auto h = schema.get_handle("cov_pxpy"); h) cache.h_cov_pxpy = *h;
            else cache.h_cov_pxpy = {};
            if (auto h = schema.get_handle("cov_pxpz"); h) cache.h_cov_pxpz = *h;
            else cache.h_cov_pxpz = {};
            if (auto h = schema.get_handle("cov_pyx"); h) cache.h_cov_pyx = *h;
            else cache.h_cov_pyx = {};
            if (auto h = schema.get_handle("cov_pyy"); h) cache.h_cov_pyy = *h;
            else cache.h_cov_pyy = {};
            if (auto h = schema.get_handle("cov_pyz"); h) cache.h_cov_pyz = *h;
            else cache.h_cov_pyz = {};
            if (auto h = schema.get_handle("cov_pypx"); h) cache.h_cov_pypx = *h;
            else cache.h_cov_pypx = {};
            if (auto h = schema.get_handle("cov_pypy"); h) cache.h_cov_pypy = *h;
            else cache.h_cov_pypy = {};
            if (auto h = schema.get_handle("cov_pypz"); h) cache.h_cov_pypz = *h;
            else cache.h_cov_pypz = {};
            if (auto h = schema.get_handle("cov_pzx"); h) cache.h_cov_pzx = *h;
            else cache.h_cov_pzx = {};
            if (auto h = schema.get_handle("cov_pzy"); h) cache.h_cov_pzy = *h;
            else cache.h_cov_pzy = {};
            if (auto h = schema.get_handle("cov_pzz"); h) cache.h_cov_pzz = *h;
            else cache.h_cov_pzz = {};
            if (auto h = schema.get_handle("cov_pzpx"); h) cache.h_cov_pzpx = *h;
            else cache.h_cov_pzpx = {};
            if (auto h = schema.get_handle("cov_pzpy"); h) cache.h_cov_pzpy = *h;
            else cache.h_cov_pzpy = {};
            if (auto h = schema.get_handle("cov_pzpz"); h) cache.h_cov_pzpz = *h;
            else cache.h_cov_pzpz = {};
        }
        h_ID = cache.h_ID;
        h_cov_xx = cache.h_cov_xx;
        h_cov_xy = cache.h_cov_xy;
        h_cov_xz = cache.h_cov_xz;
        h_cov_xpx = cache.h_cov_xpx;
        h_cov_xpy = cache.h_cov_xpy;
        h_cov_xpz = cache.h_cov_xpz;
        h_cov_yx = cache.h_cov_yx;
        h_cov_yy = cache.h_cov_yy;
        h_cov_yz = cache.h_cov_yz;
        h_cov_ypx = cache.h_cov_ypx;
        h_cov_ypy = cache.h_cov_ypy;
        h_cov_ypz = cache.h_cov_ypz;
        h_cov_zx = cache.h_cov_zx;
        h_cov_zy = cache.h_cov_zy;
        h_cov_zz = cache.h_cov_zz;
        h_cov_zpx = cache.h_cov_zpx;
        h_cov_zpy = cache.h_cov_zpy;
        h_cov_zpz = cache.h_cov_zpz;
        h_cov_pxx = cache.h_cov_pxx;
        h_cov_pxy = cache.h_cov_pxy;
        h_cov_pxz = cache.h_cov_pxz;
        h_cov_pxpx = cache.h_cov_pxpx;
        h_cov_pxpy = cache.h_cov_pxpy;
        h_cov_pxpz = cache.h_cov_pxpz;
        h_cov_pyx = cache.h_cov_pyx;
        h_cov_pyy = cache.h_cov_pyy;
        h_cov_pyz = cache.h_cov_pyz;
        h_cov_pypx = cache.h_cov_pypx;
        h_cov_pypy = cache.h_cov_pypy;
        h_cov_pypz = cache.h_cov_pypz;
        h_cov_pzx = cache.h_cov_pzx;
        h_cov_pzy = cache.h_cov_pzy;
        h_cov_pzz = cache.h_cov_pzz;
        h_cov_pzpx = cache.h_cov_pzpx;
        h_cov_pzpy = cache.h_cov_pzpy;
        h_cov_pzpz = cache.h_cov_pzpz;
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
    /// cov_xx: helical track fit covariance matrix element : delta_x.delta_x
    [[nodiscard]] float cov_xx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_xx, row));
    }
    /// cov_xy: helical track fit covariance matrix element : delta_x.delta_y
    [[nodiscard]] float cov_xy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_xy, row));
    }
    /// cov_xz: helical track fit covariance matrix element : delta_x.delta_z
    [[nodiscard]] float cov_xz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_xz, row));
    }
    /// cov_xpx: helical track fit covariance matrix element : delta_x.delta_px
    [[nodiscard]] float cov_xpx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_xpx, row));
    }
    /// cov_xpy: helical track fit covariance matrix element : delta_x.delta_py
    [[nodiscard]] float cov_xpy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_xpy, row));
    }
    /// cov_xpz: helical track fit covariance matrix element : delta_x.delta_pz
    [[nodiscard]] float cov_xpz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_xpz, row));
    }
    /// cov_yx: helical track fit covariance matrix element : delta_y.delta_x
    [[nodiscard]] float cov_yx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_yx, row));
    }
    /// cov_yy: helical track fit covariance matrix element : delta_y.delta_y
    [[nodiscard]] float cov_yy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_yy, row));
    }
    /// cov_yz: helical track fit covariance matrix element : delta_y.delta_z
    [[nodiscard]] float cov_yz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_yz, row));
    }
    /// cov_ypx: helical track fit covariance matrix element : delta_y.delta_px
    [[nodiscard]] float cov_ypx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_ypx, row));
    }
    /// cov_ypy: helical track fit covariance matrix element : delta_y.delta_py
    [[nodiscard]] float cov_ypy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_ypy, row));
    }
    /// cov_ypz: helical track fit covariance matrix element : delta_y.delta_pz
    [[nodiscard]] float cov_ypz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_ypz, row));
    }
    /// cov_zx: helical track fit covariance matrix element : delta_z.delta_x
    [[nodiscard]] float cov_zx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_zx, row));
    }
    /// cov_zy: helical track fit covariance matrix element : delta_z.delta_y
    [[nodiscard]] float cov_zy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_zy, row));
    }
    /// cov_zz: helical track fit covariance matrix element : delta_z.delta_z
    [[nodiscard]] float cov_zz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_zz, row));
    }
    /// cov_zpx: helical track fit covariance matrix element : delta_z.delta_px
    [[nodiscard]] float cov_zpx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_zpx, row));
    }
    /// cov_zpy: helical track fit covariance matrix element : delta_z.delta_py
    [[nodiscard]] float cov_zpy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_zpy, row));
    }
    /// cov_zpz: helical track fit covariance matrix element : delta_z.delta_pz
    [[nodiscard]] float cov_zpz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_zpz, row));
    }
    /// cov_pxx: helical track fit covariance matrix element : delta_px.delta_x
    [[nodiscard]] float cov_pxx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pxx, row));
    }
    /// cov_pxy: helical track fit covariance matrix element : delta_px.delta_y
    [[nodiscard]] float cov_pxy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pxy, row));
    }
    /// cov_pxz: helical track fit covariance matrix element : delta_px.delta_z
    [[nodiscard]] float cov_pxz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pxz, row));
    }
    /// cov_pxpx: helical track fit covariance matrix element : delta_px.delta_px
    [[nodiscard]] float cov_pxpx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pxpx, row));
    }
    /// cov_pxpy: helical track fit covariance matrix element : delta_px.delta_py
    [[nodiscard]] float cov_pxpy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pxpy, row));
    }
    /// cov_pxpz: helical track fit covariance matrix element : delta_px.delta_pz
    [[nodiscard]] float cov_pxpz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pxpz, row));
    }
    /// cov_pyx: helical track fit covariance matrix element : delta_py.delta_x
    [[nodiscard]] float cov_pyx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pyx, row));
    }
    /// cov_pyy: helical track fit covariance matrix element : delta_py.delta_y
    [[nodiscard]] float cov_pyy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pyy, row));
    }
    /// cov_pyz: helical track fit covariance matrix element : delta_py.delta_z
    [[nodiscard]] float cov_pyz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pyz, row));
    }
    /// cov_pypx: helical track fit covariance matrix element : delta_py.delta_px
    [[nodiscard]] float cov_pypx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pypx, row));
    }
    /// cov_pypy: helical track fit covariance matrix element : delta_py.delta_py
    [[nodiscard]] float cov_pypy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pypy, row));
    }
    /// cov_pypz: helical track fit covariance matrix element : delta_py.delta_pz
    [[nodiscard]] float cov_pypz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pypz, row));
    }
    /// cov_pzx: helical track fit covariance matrix element : delta_pz.delta_x
    [[nodiscard]] float cov_pzx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pzx, row));
    }
    /// cov_pzy: helical track fit covariance matrix element : delta_pz.delta_y
    [[nodiscard]] float cov_pzy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pzy, row));
    }
    /// cov_pzz: helical track fit covariance matrix element : delta_pz.delta_z
    [[nodiscard]] float cov_pzz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pzz, row));
    }
    /// cov_pzpx: helical track fit covariance matrix element : delta_pz.delta_px
    [[nodiscard]] float cov_pzpx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pzpx, row));
    }
    /// cov_pzpy: helical track fit covariance matrix element : delta_pz.delta_py
    [[nodiscard]] float cov_pzpy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pzpy, row));
    }
    /// cov_pzpz: helical track fit covariance matrix element : delta_pz.delta_pz
    [[nodiscard]] float cov_pzpz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cov_pzpz, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ID_col() const noexcept {
        return m_view.column<int16_t>(h_ID);
    }
    /// cov_xx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_xx_col() const noexcept {
        return m_view.column<float>(h_cov_xx);
    }
    /// cov_xy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_xy_col() const noexcept {
        return m_view.column<float>(h_cov_xy);
    }
    /// cov_xz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_xz_col() const noexcept {
        return m_view.column<float>(h_cov_xz);
    }
    /// cov_xpx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_xpx_col() const noexcept {
        return m_view.column<float>(h_cov_xpx);
    }
    /// cov_xpy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_xpy_col() const noexcept {
        return m_view.column<float>(h_cov_xpy);
    }
    /// cov_xpz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_xpz_col() const noexcept {
        return m_view.column<float>(h_cov_xpz);
    }
    /// cov_yx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_yx_col() const noexcept {
        return m_view.column<float>(h_cov_yx);
    }
    /// cov_yy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_yy_col() const noexcept {
        return m_view.column<float>(h_cov_yy);
    }
    /// cov_yz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_yz_col() const noexcept {
        return m_view.column<float>(h_cov_yz);
    }
    /// cov_ypx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_ypx_col() const noexcept {
        return m_view.column<float>(h_cov_ypx);
    }
    /// cov_ypy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_ypy_col() const noexcept {
        return m_view.column<float>(h_cov_ypy);
    }
    /// cov_ypz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_ypz_col() const noexcept {
        return m_view.column<float>(h_cov_ypz);
    }
    /// cov_zx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_zx_col() const noexcept {
        return m_view.column<float>(h_cov_zx);
    }
    /// cov_zy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_zy_col() const noexcept {
        return m_view.column<float>(h_cov_zy);
    }
    /// cov_zz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_zz_col() const noexcept {
        return m_view.column<float>(h_cov_zz);
    }
    /// cov_zpx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_zpx_col() const noexcept {
        return m_view.column<float>(h_cov_zpx);
    }
    /// cov_zpy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_zpy_col() const noexcept {
        return m_view.column<float>(h_cov_zpy);
    }
    /// cov_zpz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_zpz_col() const noexcept {
        return m_view.column<float>(h_cov_zpz);
    }
    /// cov_pxx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pxx_col() const noexcept {
        return m_view.column<float>(h_cov_pxx);
    }
    /// cov_pxy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pxy_col() const noexcept {
        return m_view.column<float>(h_cov_pxy);
    }
    /// cov_pxz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pxz_col() const noexcept {
        return m_view.column<float>(h_cov_pxz);
    }
    /// cov_pxpx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pxpx_col() const noexcept {
        return m_view.column<float>(h_cov_pxpx);
    }
    /// cov_pxpy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pxpy_col() const noexcept {
        return m_view.column<float>(h_cov_pxpy);
    }
    /// cov_pxpz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pxpz_col() const noexcept {
        return m_view.column<float>(h_cov_pxpz);
    }
    /// cov_pyx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pyx_col() const noexcept {
        return m_view.column<float>(h_cov_pyx);
    }
    /// cov_pyy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pyy_col() const noexcept {
        return m_view.column<float>(h_cov_pyy);
    }
    /// cov_pyz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pyz_col() const noexcept {
        return m_view.column<float>(h_cov_pyz);
    }
    /// cov_pypx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pypx_col() const noexcept {
        return m_view.column<float>(h_cov_pypx);
    }
    /// cov_pypy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pypy_col() const noexcept {
        return m_view.column<float>(h_cov_pypy);
    }
    /// cov_pypz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pypz_col() const noexcept {
        return m_view.column<float>(h_cov_pypz);
    }
    /// cov_pzx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pzx_col() const noexcept {
        return m_view.column<float>(h_cov_pzx);
    }
    /// cov_pzy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pzy_col() const noexcept {
        return m_view.column<float>(h_cov_pzy);
    }
    /// cov_pzz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pzz_col() const noexcept {
        return m_view.column<float>(h_cov_pzz);
    }
    /// cov_pzpx column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pzpx_col() const noexcept {
        return m_view.column<float>(h_cov_pzpx);
    }
    /// cov_pzpy column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pzpy_col() const noexcept {
        return m_view.column<float>(h_cov_pzpy);
    }
    /// cov_pzpz column as a contiguous span.
    [[nodiscard]] std::span<const float> cov_pzpz_col() const noexcept {
        return m_view.column<float>(h_cov_pzpz);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_ID{};
    hipo::column_handle h_cov_xx{};
    hipo::column_handle h_cov_xy{};
    hipo::column_handle h_cov_xz{};
    hipo::column_handle h_cov_xpx{};
    hipo::column_handle h_cov_xpy{};
    hipo::column_handle h_cov_xpz{};
    hipo::column_handle h_cov_yx{};
    hipo::column_handle h_cov_yy{};
    hipo::column_handle h_cov_yz{};
    hipo::column_handle h_cov_ypx{};
    hipo::column_handle h_cov_ypy{};
    hipo::column_handle h_cov_ypz{};
    hipo::column_handle h_cov_zx{};
    hipo::column_handle h_cov_zy{};
    hipo::column_handle h_cov_zz{};
    hipo::column_handle h_cov_zpx{};
    hipo::column_handle h_cov_zpy{};
    hipo::column_handle h_cov_zpz{};
    hipo::column_handle h_cov_pxx{};
    hipo::column_handle h_cov_pxy{};
    hipo::column_handle h_cov_pxz{};
    hipo::column_handle h_cov_pxpx{};
    hipo::column_handle h_cov_pxpy{};
    hipo::column_handle h_cov_pxpz{};
    hipo::column_handle h_cov_pyx{};
    hipo::column_handle h_cov_pyy{};
    hipo::column_handle h_cov_pyz{};
    hipo::column_handle h_cov_pypx{};
    hipo::column_handle h_cov_pypy{};
    hipo::column_handle h_cov_pypz{};
    hipo::column_handle h_cov_pzx{};
    hipo::column_handle h_cov_pzy{};
    hipo::column_handle h_cov_pzz{};
    hipo::column_handle h_cov_pzpx{};
    hipo::column_handle h_cov_pzpy{};
    hipo::column_handle h_cov_pzpz{};

    friend class Row;
};

/// Lightweight row view for CVTRec::TrackCovMat. Stores a pointer + row index only.
class CVTRec_TrackCovMat::Row {
 public:
    Row(const CVTRec_TrackCovMat* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] float cov_xx() const noexcept { return m_bank->cov_xx(m_row); }
    [[nodiscard]] float cov_xy() const noexcept { return m_bank->cov_xy(m_row); }
    [[nodiscard]] float cov_xz() const noexcept { return m_bank->cov_xz(m_row); }
    [[nodiscard]] float cov_xpx() const noexcept { return m_bank->cov_xpx(m_row); }
    [[nodiscard]] float cov_xpy() const noexcept { return m_bank->cov_xpy(m_row); }
    [[nodiscard]] float cov_xpz() const noexcept { return m_bank->cov_xpz(m_row); }
    [[nodiscard]] float cov_yx() const noexcept { return m_bank->cov_yx(m_row); }
    [[nodiscard]] float cov_yy() const noexcept { return m_bank->cov_yy(m_row); }
    [[nodiscard]] float cov_yz() const noexcept { return m_bank->cov_yz(m_row); }
    [[nodiscard]] float cov_ypx() const noexcept { return m_bank->cov_ypx(m_row); }
    [[nodiscard]] float cov_ypy() const noexcept { return m_bank->cov_ypy(m_row); }
    [[nodiscard]] float cov_ypz() const noexcept { return m_bank->cov_ypz(m_row); }
    [[nodiscard]] float cov_zx() const noexcept { return m_bank->cov_zx(m_row); }
    [[nodiscard]] float cov_zy() const noexcept { return m_bank->cov_zy(m_row); }
    [[nodiscard]] float cov_zz() const noexcept { return m_bank->cov_zz(m_row); }
    [[nodiscard]] float cov_zpx() const noexcept { return m_bank->cov_zpx(m_row); }
    [[nodiscard]] float cov_zpy() const noexcept { return m_bank->cov_zpy(m_row); }
    [[nodiscard]] float cov_zpz() const noexcept { return m_bank->cov_zpz(m_row); }
    [[nodiscard]] float cov_pxx() const noexcept { return m_bank->cov_pxx(m_row); }
    [[nodiscard]] float cov_pxy() const noexcept { return m_bank->cov_pxy(m_row); }
    [[nodiscard]] float cov_pxz() const noexcept { return m_bank->cov_pxz(m_row); }
    [[nodiscard]] float cov_pxpx() const noexcept { return m_bank->cov_pxpx(m_row); }
    [[nodiscard]] float cov_pxpy() const noexcept { return m_bank->cov_pxpy(m_row); }
    [[nodiscard]] float cov_pxpz() const noexcept { return m_bank->cov_pxpz(m_row); }
    [[nodiscard]] float cov_pyx() const noexcept { return m_bank->cov_pyx(m_row); }
    [[nodiscard]] float cov_pyy() const noexcept { return m_bank->cov_pyy(m_row); }
    [[nodiscard]] float cov_pyz() const noexcept { return m_bank->cov_pyz(m_row); }
    [[nodiscard]] float cov_pypx() const noexcept { return m_bank->cov_pypx(m_row); }
    [[nodiscard]] float cov_pypy() const noexcept { return m_bank->cov_pypy(m_row); }
    [[nodiscard]] float cov_pypz() const noexcept { return m_bank->cov_pypz(m_row); }
    [[nodiscard]] float cov_pzx() const noexcept { return m_bank->cov_pzx(m_row); }
    [[nodiscard]] float cov_pzy() const noexcept { return m_bank->cov_pzy(m_row); }
    [[nodiscard]] float cov_pzz() const noexcept { return m_bank->cov_pzz(m_row); }
    [[nodiscard]] float cov_pzpx() const noexcept { return m_bank->cov_pzpx(m_row); }
    [[nodiscard]] float cov_pzpy() const noexcept { return m_bank->cov_pzpy(m_row); }
    [[nodiscard]] float cov_pzpz() const noexcept { return m_bank->cov_pzpz(m_row); }
 private:
    const CVTRec_TrackCovMat* m_bank;
    int m_row;
};

inline CVTRec_TrackCovMat::Row CVTRec_TrackCovMat::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
