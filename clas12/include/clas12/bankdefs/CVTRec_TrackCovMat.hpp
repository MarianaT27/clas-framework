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


/// HIPO bank wrapper for `CVTRec::TrackCovMat` (group 20500, item 28).
/// reconstructed CVT tracks covariance matrix in track representation
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
            int i_cov_xx = -1;
            int i_cov_xy = -1;
            int i_cov_xz = -1;
            int i_cov_xpx = -1;
            int i_cov_xpy = -1;
            int i_cov_xpz = -1;
            int i_cov_yx = -1;
            int i_cov_yy = -1;
            int i_cov_yz = -1;
            int i_cov_ypx = -1;
            int i_cov_ypy = -1;
            int i_cov_ypz = -1;
            int i_cov_zx = -1;
            int i_cov_zy = -1;
            int i_cov_zz = -1;
            int i_cov_zpx = -1;
            int i_cov_zpy = -1;
            int i_cov_zpz = -1;
            int i_cov_pxx = -1;
            int i_cov_pxy = -1;
            int i_cov_pxz = -1;
            int i_cov_pxpx = -1;
            int i_cov_pxpy = -1;
            int i_cov_pxpz = -1;
            int i_cov_pyx = -1;
            int i_cov_pyy = -1;
            int i_cov_pyz = -1;
            int i_cov_pypx = -1;
            int i_cov_pypy = -1;
            int i_cov_pypz = -1;
            int i_cov_pzx = -1;
            int i_cov_pzy = -1;
            int i_cov_pzz = -1;
            int i_cov_pzpx = -1;
            int i_cov_pzpy = -1;
            int i_cov_pzpz = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_ID = schema.exists("ID") ? schema.getEntryOrder("ID") : -1;
            cache.i_cov_xx = schema.exists("cov_xx") ? schema.getEntryOrder("cov_xx") : -1;
            cache.i_cov_xy = schema.exists("cov_xy") ? schema.getEntryOrder("cov_xy") : -1;
            cache.i_cov_xz = schema.exists("cov_xz") ? schema.getEntryOrder("cov_xz") : -1;
            cache.i_cov_xpx = schema.exists("cov_xpx") ? schema.getEntryOrder("cov_xpx") : -1;
            cache.i_cov_xpy = schema.exists("cov_xpy") ? schema.getEntryOrder("cov_xpy") : -1;
            cache.i_cov_xpz = schema.exists("cov_xpz") ? schema.getEntryOrder("cov_xpz") : -1;
            cache.i_cov_yx = schema.exists("cov_yx") ? schema.getEntryOrder("cov_yx") : -1;
            cache.i_cov_yy = schema.exists("cov_yy") ? schema.getEntryOrder("cov_yy") : -1;
            cache.i_cov_yz = schema.exists("cov_yz") ? schema.getEntryOrder("cov_yz") : -1;
            cache.i_cov_ypx = schema.exists("cov_ypx") ? schema.getEntryOrder("cov_ypx") : -1;
            cache.i_cov_ypy = schema.exists("cov_ypy") ? schema.getEntryOrder("cov_ypy") : -1;
            cache.i_cov_ypz = schema.exists("cov_ypz") ? schema.getEntryOrder("cov_ypz") : -1;
            cache.i_cov_zx = schema.exists("cov_zx") ? schema.getEntryOrder("cov_zx") : -1;
            cache.i_cov_zy = schema.exists("cov_zy") ? schema.getEntryOrder("cov_zy") : -1;
            cache.i_cov_zz = schema.exists("cov_zz") ? schema.getEntryOrder("cov_zz") : -1;
            cache.i_cov_zpx = schema.exists("cov_zpx") ? schema.getEntryOrder("cov_zpx") : -1;
            cache.i_cov_zpy = schema.exists("cov_zpy") ? schema.getEntryOrder("cov_zpy") : -1;
            cache.i_cov_zpz = schema.exists("cov_zpz") ? schema.getEntryOrder("cov_zpz") : -1;
            cache.i_cov_pxx = schema.exists("cov_pxx") ? schema.getEntryOrder("cov_pxx") : -1;
            cache.i_cov_pxy = schema.exists("cov_pxy") ? schema.getEntryOrder("cov_pxy") : -1;
            cache.i_cov_pxz = schema.exists("cov_pxz") ? schema.getEntryOrder("cov_pxz") : -1;
            cache.i_cov_pxpx = schema.exists("cov_pxpx") ? schema.getEntryOrder("cov_pxpx") : -1;
            cache.i_cov_pxpy = schema.exists("cov_pxpy") ? schema.getEntryOrder("cov_pxpy") : -1;
            cache.i_cov_pxpz = schema.exists("cov_pxpz") ? schema.getEntryOrder("cov_pxpz") : -1;
            cache.i_cov_pyx = schema.exists("cov_pyx") ? schema.getEntryOrder("cov_pyx") : -1;
            cache.i_cov_pyy = schema.exists("cov_pyy") ? schema.getEntryOrder("cov_pyy") : -1;
            cache.i_cov_pyz = schema.exists("cov_pyz") ? schema.getEntryOrder("cov_pyz") : -1;
            cache.i_cov_pypx = schema.exists("cov_pypx") ? schema.getEntryOrder("cov_pypx") : -1;
            cache.i_cov_pypy = schema.exists("cov_pypy") ? schema.getEntryOrder("cov_pypy") : -1;
            cache.i_cov_pypz = schema.exists("cov_pypz") ? schema.getEntryOrder("cov_pypz") : -1;
            cache.i_cov_pzx = schema.exists("cov_pzx") ? schema.getEntryOrder("cov_pzx") : -1;
            cache.i_cov_pzy = schema.exists("cov_pzy") ? schema.getEntryOrder("cov_pzy") : -1;
            cache.i_cov_pzz = schema.exists("cov_pzz") ? schema.getEntryOrder("cov_pzz") : -1;
            cache.i_cov_pzpx = schema.exists("cov_pzpx") ? schema.getEntryOrder("cov_pzpx") : -1;
            cache.i_cov_pzpy = schema.exists("cov_pzpy") ? schema.getEntryOrder("cov_pzpy") : -1;
            cache.i_cov_pzpz = schema.exists("cov_pzpz") ? schema.getEntryOrder("cov_pzpz") : -1;
        }
        i_ID = cache.i_ID;
        i_cov_xx = cache.i_cov_xx;
        i_cov_xy = cache.i_cov_xy;
        i_cov_xz = cache.i_cov_xz;
        i_cov_xpx = cache.i_cov_xpx;
        i_cov_xpy = cache.i_cov_xpy;
        i_cov_xpz = cache.i_cov_xpz;
        i_cov_yx = cache.i_cov_yx;
        i_cov_yy = cache.i_cov_yy;
        i_cov_yz = cache.i_cov_yz;
        i_cov_ypx = cache.i_cov_ypx;
        i_cov_ypy = cache.i_cov_ypy;
        i_cov_ypz = cache.i_cov_ypz;
        i_cov_zx = cache.i_cov_zx;
        i_cov_zy = cache.i_cov_zy;
        i_cov_zz = cache.i_cov_zz;
        i_cov_zpx = cache.i_cov_zpx;
        i_cov_zpy = cache.i_cov_zpy;
        i_cov_zpz = cache.i_cov_zpz;
        i_cov_pxx = cache.i_cov_pxx;
        i_cov_pxy = cache.i_cov_pxy;
        i_cov_pxz = cache.i_cov_pxz;
        i_cov_pxpx = cache.i_cov_pxpx;
        i_cov_pxpy = cache.i_cov_pxpy;
        i_cov_pxpz = cache.i_cov_pxpz;
        i_cov_pyx = cache.i_cov_pyx;
        i_cov_pyy = cache.i_cov_pyy;
        i_cov_pyz = cache.i_cov_pyz;
        i_cov_pypx = cache.i_cov_pypx;
        i_cov_pypy = cache.i_cov_pypy;
        i_cov_pypz = cache.i_cov_pypz;
        i_cov_pzx = cache.i_cov_pzx;
        i_cov_pzy = cache.i_cov_pzy;
        i_cov_pzz = cache.i_cov_pzz;
        i_cov_pzpx = cache.i_cov_pzpx;
        i_cov_pzpy = cache.i_cov_pzpy;
        i_cov_pzpz = cache.i_cov_pzpz;
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
    /// cov_xx: helical track fit covariance matrix element : delta_x.delta_x
    [[nodiscard]] float cov_xx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_xx, row));
    }
    /// cov_xy: helical track fit covariance matrix element : delta_x.delta_y
    [[nodiscard]] float cov_xy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_xy, row));
    }
    /// cov_xz: helical track fit covariance matrix element : delta_x.delta_z
    [[nodiscard]] float cov_xz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_xz, row));
    }
    /// cov_xpx: helical track fit covariance matrix element : delta_x.delta_px
    [[nodiscard]] float cov_xpx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_xpx, row));
    }
    /// cov_xpy: helical track fit covariance matrix element : delta_x.delta_py
    [[nodiscard]] float cov_xpy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_xpy, row));
    }
    /// cov_xpz: helical track fit covariance matrix element : delta_x.delta_pz
    [[nodiscard]] float cov_xpz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_xpz, row));
    }
    /// cov_yx: helical track fit covariance matrix element : delta_y.delta_x
    [[nodiscard]] float cov_yx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_yx, row));
    }
    /// cov_yy: helical track fit covariance matrix element : delta_y.delta_y
    [[nodiscard]] float cov_yy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_yy, row));
    }
    /// cov_yz: helical track fit covariance matrix element : delta_y.delta_z
    [[nodiscard]] float cov_yz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_yz, row));
    }
    /// cov_ypx: helical track fit covariance matrix element : delta_y.delta_px
    [[nodiscard]] float cov_ypx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_ypx, row));
    }
    /// cov_ypy: helical track fit covariance matrix element : delta_y.delta_py
    [[nodiscard]] float cov_ypy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_ypy, row));
    }
    /// cov_ypz: helical track fit covariance matrix element : delta_y.delta_pz
    [[nodiscard]] float cov_ypz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_ypz, row));
    }
    /// cov_zx: helical track fit covariance matrix element : delta_z.delta_x
    [[nodiscard]] float cov_zx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_zx, row));
    }
    /// cov_zy: helical track fit covariance matrix element : delta_z.delta_y
    [[nodiscard]] float cov_zy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_zy, row));
    }
    /// cov_zz: helical track fit covariance matrix element : delta_z.delta_z
    [[nodiscard]] float cov_zz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_zz, row));
    }
    /// cov_zpx: helical track fit covariance matrix element : delta_z.delta_px
    [[nodiscard]] float cov_zpx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_zpx, row));
    }
    /// cov_zpy: helical track fit covariance matrix element : delta_z.delta_py
    [[nodiscard]] float cov_zpy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_zpy, row));
    }
    /// cov_zpz: helical track fit covariance matrix element : delta_z.delta_pz
    [[nodiscard]] float cov_zpz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_zpz, row));
    }
    /// cov_pxx: helical track fit covariance matrix element : delta_px.delta_x
    [[nodiscard]] float cov_pxx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pxx, row));
    }
    /// cov_pxy: helical track fit covariance matrix element : delta_px.delta_y
    [[nodiscard]] float cov_pxy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pxy, row));
    }
    /// cov_pxz: helical track fit covariance matrix element : delta_px.delta_z
    [[nodiscard]] float cov_pxz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pxz, row));
    }
    /// cov_pxpx: helical track fit covariance matrix element : delta_px.delta_px
    [[nodiscard]] float cov_pxpx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pxpx, row));
    }
    /// cov_pxpy: helical track fit covariance matrix element : delta_px.delta_py
    [[nodiscard]] float cov_pxpy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pxpy, row));
    }
    /// cov_pxpz: helical track fit covariance matrix element : delta_px.delta_pz
    [[nodiscard]] float cov_pxpz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pxpz, row));
    }
    /// cov_pyx: helical track fit covariance matrix element : delta_py.delta_x
    [[nodiscard]] float cov_pyx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pyx, row));
    }
    /// cov_pyy: helical track fit covariance matrix element : delta_py.delta_y
    [[nodiscard]] float cov_pyy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pyy, row));
    }
    /// cov_pyz: helical track fit covariance matrix element : delta_py.delta_z
    [[nodiscard]] float cov_pyz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pyz, row));
    }
    /// cov_pypx: helical track fit covariance matrix element : delta_py.delta_px
    [[nodiscard]] float cov_pypx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pypx, row));
    }
    /// cov_pypy: helical track fit covariance matrix element : delta_py.delta_py
    [[nodiscard]] float cov_pypy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pypy, row));
    }
    /// cov_pypz: helical track fit covariance matrix element : delta_py.delta_pz
    [[nodiscard]] float cov_pypz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pypz, row));
    }
    /// cov_pzx: helical track fit covariance matrix element : delta_pz.delta_x
    [[nodiscard]] float cov_pzx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pzx, row));
    }
    /// cov_pzy: helical track fit covariance matrix element : delta_pz.delta_y
    [[nodiscard]] float cov_pzy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pzy, row));
    }
    /// cov_pzz: helical track fit covariance matrix element : delta_pz.delta_z
    [[nodiscard]] float cov_pzz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pzz, row));
    }
    /// cov_pzpx: helical track fit covariance matrix element : delta_pz.delta_px
    [[nodiscard]] float cov_pzpx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pzpx, row));
    }
    /// cov_pzpy: helical track fit covariance matrix element : delta_pz.delta_py
    [[nodiscard]] float cov_pzpy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pzpy, row));
    }
    /// cov_pzpz: helical track fit covariance matrix element : delta_pz.delta_pz
    [[nodiscard]] float cov_pzpz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cov_pzpz, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_ID = -1;
    int i_cov_xx = -1;
    int i_cov_xy = -1;
    int i_cov_xz = -1;
    int i_cov_xpx = -1;
    int i_cov_xpy = -1;
    int i_cov_xpz = -1;
    int i_cov_yx = -1;
    int i_cov_yy = -1;
    int i_cov_yz = -1;
    int i_cov_ypx = -1;
    int i_cov_ypy = -1;
    int i_cov_ypz = -1;
    int i_cov_zx = -1;
    int i_cov_zy = -1;
    int i_cov_zz = -1;
    int i_cov_zpx = -1;
    int i_cov_zpy = -1;
    int i_cov_zpz = -1;
    int i_cov_pxx = -1;
    int i_cov_pxy = -1;
    int i_cov_pxz = -1;
    int i_cov_pxpx = -1;
    int i_cov_pxpy = -1;
    int i_cov_pxpz = -1;
    int i_cov_pyx = -1;
    int i_cov_pyy = -1;
    int i_cov_pyz = -1;
    int i_cov_pypx = -1;
    int i_cov_pypy = -1;
    int i_cov_pypz = -1;
    int i_cov_pzx = -1;
    int i_cov_pzy = -1;
    int i_cov_pzz = -1;
    int i_cov_pzpx = -1;
    int i_cov_pzpy = -1;
    int i_cov_pzpz = -1;

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
