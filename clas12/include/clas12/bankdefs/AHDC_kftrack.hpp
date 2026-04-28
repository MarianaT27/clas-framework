// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
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


/// HIPO bank wrapper for `AHDC::kftrack` (group 23000, item 26).
/// Reco Kalman Filter Tracks
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// AHDC_kftrack bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::kftrack")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trackid = row.trackid();
/// }
/// @endcode
class AHDC_kftrack {
 public:
    static constexpr std::string_view bank_name = "AHDC::kftrack";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 26;

    class Row;

    AHDC_kftrack() = default;

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
            hipo::column_handle h_trackid;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_px;
            hipo::column_handle h_py;
            hipo::column_handle h_pz;
            hipo::column_handle h_n_hits;
            hipo::column_handle h_sum_adc;
            hipo::column_handle h_path;
            hipo::column_handle h_dEdx;
            hipo::column_handle h_p_drift;
            hipo::column_handle h_chi2;
            hipo::column_handle h_sum_residuals;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("trackid"); h) cache.h_trackid = *h;
            else cache.h_trackid = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("px"); h) cache.h_px = *h;
            else cache.h_px = {};
            if (auto h = schema.get_handle("py"); h) cache.h_py = *h;
            else cache.h_py = {};
            if (auto h = schema.get_handle("pz"); h) cache.h_pz = *h;
            else cache.h_pz = {};
            if (auto h = schema.get_handle("n_hits"); h) cache.h_n_hits = *h;
            else cache.h_n_hits = {};
            if (auto h = schema.get_handle("sum_adc"); h) cache.h_sum_adc = *h;
            else cache.h_sum_adc = {};
            if (auto h = schema.get_handle("path"); h) cache.h_path = *h;
            else cache.h_path = {};
            if (auto h = schema.get_handle("dEdx"); h) cache.h_dEdx = *h;
            else cache.h_dEdx = {};
            if (auto h = schema.get_handle("p_drift"); h) cache.h_p_drift = *h;
            else cache.h_p_drift = {};
            if (auto h = schema.get_handle("chi2"); h) cache.h_chi2 = *h;
            else cache.h_chi2 = {};
            if (auto h = schema.get_handle("sum_residuals"); h) cache.h_sum_residuals = *h;
            else cache.h_sum_residuals = {};
        }
        h_trackid = cache.h_trackid;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_px = cache.h_px;
        h_py = cache.h_py;
        h_pz = cache.h_pz;
        h_n_hits = cache.h_n_hits;
        h_sum_adc = cache.h_sum_adc;
        h_path = cache.h_path;
        h_dEdx = cache.h_dEdx;
        h_p_drift = cache.h_p_drift;
        h_chi2 = cache.h_chi2;
        h_sum_residuals = cache.h_sum_residuals;
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
    /// trackid: track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trackid, row));
    }
    /// x: x position in mm
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: y position in mm
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: z position in mm
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// px: px in MeV
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: py in MeV
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: pz in MeV
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// n_hits: number of hits
    [[nodiscard]] int n_hits(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_n_hits, row));
    }
    /// sum_adc: Sum of adc (adc)
    [[nodiscard]] int sum_adc(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_sum_adc, row));
    }
    /// path: Length of the track (mm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_path, row));
    }
    /// dEdx: Deposited energy per path (adc/mm)
    [[nodiscard]] float dEdx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dEdx, row));
    }
    /// p_drift: Momentum in the drift region (MeV)
    [[nodiscard]] float p_drift(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p_drift, row));
    }
    /// chi2: Sum of squared residuals (mm^2)
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// sum_residuals: Sum of residuals (mm)
    [[nodiscard]] float sum_residuals(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_sum_residuals, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// trackid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trackid_col() const noexcept {
        return m_view.column<int32_t>(h_trackid);
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
    /// n_hits column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> n_hits_col() const noexcept {
        return m_view.column<int32_t>(h_n_hits);
    }
    /// sum_adc column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> sum_adc_col() const noexcept {
        return m_view.column<int32_t>(h_sum_adc);
    }
    /// path column as a contiguous span.
    [[nodiscard]] std::span<const float> path_col() const noexcept {
        return m_view.column<float>(h_path);
    }
    /// dEdx column as a contiguous span.
    [[nodiscard]] std::span<const float> dEdx_col() const noexcept {
        return m_view.column<float>(h_dEdx);
    }
    /// p_drift column as a contiguous span.
    [[nodiscard]] std::span<const float> p_drift_col() const noexcept {
        return m_view.column<float>(h_p_drift);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
    }
    /// sum_residuals column as a contiguous span.
    [[nodiscard]] std::span<const float> sum_residuals_col() const noexcept {
        return m_view.column<float>(h_sum_residuals);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_trackid{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_n_hits{};
    hipo::column_handle h_sum_adc{};
    hipo::column_handle h_path{};
    hipo::column_handle h_dEdx{};
    hipo::column_handle h_p_drift{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_sum_residuals{};

    friend class Row;
};

/// Lightweight row view for AHDC::kftrack. Stores a pointer + row index only.
class AHDC_kftrack::Row {
 public:
    Row(const AHDC_kftrack* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] int n_hits() const noexcept { return m_bank->n_hits(m_row); }
    [[nodiscard]] int sum_adc() const noexcept { return m_bank->sum_adc(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float dEdx() const noexcept { return m_bank->dEdx(m_row); }
    [[nodiscard]] float p_drift() const noexcept { return m_bank->p_drift(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] float sum_residuals() const noexcept { return m_bank->sum_residuals(m_row); }
 private:
    const AHDC_kftrack* m_bank;
    int m_row;
};

inline AHDC_kftrack::Row AHDC_kftrack::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
