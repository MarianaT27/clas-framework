// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `AHDC::kftrack` (group 23000, item 26).
/// Reco Kalman Filter Tracks
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
            int i_trackid = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_n_hits = -1;
            int i_sum_adc = -1;
            int i_path = -1;
            int i_dEdx = -1;
            int i_p_drift = -1;
            int i_chi2 = -1;
            int i_sum_residuals = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_n_hits = schema.exists("n_hits") ? schema.getEntryOrder("n_hits") : -1;
            cache.i_sum_adc = schema.exists("sum_adc") ? schema.getEntryOrder("sum_adc") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
            cache.i_dEdx = schema.exists("dEdx") ? schema.getEntryOrder("dEdx") : -1;
            cache.i_p_drift = schema.exists("p_drift") ? schema.getEntryOrder("p_drift") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_sum_residuals = schema.exists("sum_residuals") ? schema.getEntryOrder("sum_residuals") : -1;
        }
        i_trackid = cache.i_trackid;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_n_hits = cache.i_n_hits;
        i_sum_adc = cache.i_sum_adc;
        i_path = cache.i_path;
        i_dEdx = cache.i_dEdx;
        i_p_drift = cache.i_p_drift;
        i_chi2 = cache.i_chi2;
        i_sum_residuals = cache.i_sum_residuals;
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
    /// trackid: track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trackid, row));
    }
    /// x: x position in mm
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y position in mm
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: z position in mm
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// px: px in MeV
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: py in MeV
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: pz in MeV
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// n_hits: number of hits
    [[nodiscard]] int n_hits(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_n_hits, row));
    }
    /// sum_adc: Sum of adc (adc)
    [[nodiscard]] int sum_adc(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_sum_adc, row));
    }
    /// path: Length of the track (mm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
    }
    /// dEdx: Deposited energy per path (adc/mm)
    [[nodiscard]] float dEdx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dEdx, row));
    }
    /// p_drift: Momentum in the drift region (MeV)
    [[nodiscard]] float p_drift(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p_drift, row));
    }
    /// chi2: Sum of squared residuals (mm^2)
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// sum_residuals: Sum of residuals (mm)
    [[nodiscard]] float sum_residuals(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_sum_residuals, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_trackid = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_n_hits = -1;
    int i_sum_adc = -1;
    int i_path = -1;
    int i_dEdx = -1;
    int i_p_drift = -1;
    int i_chi2 = -1;
    int i_sum_residuals = -1;

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
