// =============================================================================
// Auto-generated from rtpc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RTPC::tracks` (group 21900, item 22).
/// reconstructed track info
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RTPC_tracks bank;
/// if (auto sc = bank.bind(event.get_bank("RTPC::tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trkID = row.trkID();
/// }
/// @endcode
class RTPC_tracks {
 public:
    static constexpr std::string_view bank_name = "RTPC::tracks";
    static constexpr int bank_group = 21900;
    static constexpr int bank_item  = 22;

    class Row;

    RTPC_tracks() = default;

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
            int i_trkID = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_vz = -1;
            int i_theta = -1;
            int i_phi = -1;
            int i_nhits = -1;
            int i_path = -1;
            int i_adcsum = -1;
            int i_dedx = -1;
            int i_r_helix = -1;
            int i_x_helix = -1;
            int i_y_helix = -1;
            int i_chi2_helix = -1;
            int i_min_row = -1;
            int i_min_col = -1;
            int i_max_row = -1;
            int i_max_col = -1;
            int i_min_time = -1;
            int i_max_time = -1;
            int i_min_radius = -1;
            int i_max_radius = -1;
            int i_min_phi = -1;
            int i_max_phi = -1;
            int i_tdiff = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
            cache.i_theta = schema.exists("theta") ? schema.getEntryOrder("theta") : -1;
            cache.i_phi = schema.exists("phi") ? schema.getEntryOrder("phi") : -1;
            cache.i_nhits = schema.exists("nhits") ? schema.getEntryOrder("nhits") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
            cache.i_adcsum = schema.exists("adcsum") ? schema.getEntryOrder("adcsum") : -1;
            cache.i_dedx = schema.exists("dedx") ? schema.getEntryOrder("dedx") : -1;
            cache.i_r_helix = schema.exists("r_helix") ? schema.getEntryOrder("r_helix") : -1;
            cache.i_x_helix = schema.exists("x_helix") ? schema.getEntryOrder("x_helix") : -1;
            cache.i_y_helix = schema.exists("y_helix") ? schema.getEntryOrder("y_helix") : -1;
            cache.i_chi2_helix = schema.exists("chi2_helix") ? schema.getEntryOrder("chi2_helix") : -1;
            cache.i_min_row = schema.exists("min_row") ? schema.getEntryOrder("min_row") : -1;
            cache.i_min_col = schema.exists("min_col") ? schema.getEntryOrder("min_col") : -1;
            cache.i_max_row = schema.exists("max_row") ? schema.getEntryOrder("max_row") : -1;
            cache.i_max_col = schema.exists("max_col") ? schema.getEntryOrder("max_col") : -1;
            cache.i_min_time = schema.exists("min_time") ? schema.getEntryOrder("min_time") : -1;
            cache.i_max_time = schema.exists("max_time") ? schema.getEntryOrder("max_time") : -1;
            cache.i_min_radius = schema.exists("min_radius") ? schema.getEntryOrder("min_radius") : -1;
            cache.i_max_radius = schema.exists("max_radius") ? schema.getEntryOrder("max_radius") : -1;
            cache.i_min_phi = schema.exists("min_phi") ? schema.getEntryOrder("min_phi") : -1;
            cache.i_max_phi = schema.exists("max_phi") ? schema.getEntryOrder("max_phi") : -1;
            cache.i_tdiff = schema.exists("tdiff") ? schema.getEntryOrder("tdiff") : -1;
        }
        i_trkID = cache.i_trkID;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_vz = cache.i_vz;
        i_theta = cache.i_theta;
        i_phi = cache.i_phi;
        i_nhits = cache.i_nhits;
        i_path = cache.i_path;
        i_adcsum = cache.i_adcsum;
        i_dedx = cache.i_dedx;
        i_r_helix = cache.i_r_helix;
        i_x_helix = cache.i_x_helix;
        i_y_helix = cache.i_y_helix;
        i_chi2_helix = cache.i_chi2_helix;
        i_min_row = cache.i_min_row;
        i_min_col = cache.i_min_col;
        i_max_row = cache.i_max_row;
        i_max_col = cache.i_max_col;
        i_min_time = cache.i_min_time;
        i_max_time = cache.i_max_time;
        i_min_radius = cache.i_min_radius;
        i_max_radius = cache.i_max_radius;
        i_min_phi = cache.i_min_phi;
        i_max_phi = cache.i_max_phi;
        i_tdiff = cache.i_tdiff;
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
    /// trkID: Track ID of the hit
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trkID, row));
    }
    /// px: X component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: Y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: Z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// vz: Z component of the reconstructed vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }
    /// theta: Theta from the Helix fit of the track (deg)
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_theta, row));
    }
    /// phi: Phi from the Helix fit of the track (deg)
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_phi, row));
    }
    /// nhits: Number of hits in the track
    [[nodiscard]] int nhits(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_nhits, row));
    }
    /// path: Length of the Track (mm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
    }
    /// adcsum: Total ADC sum of the Track
    [[nodiscard]] float adcsum(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_adcsum, row));
    }
    /// dedx: dEdx for the Track (MeV/mm)
    [[nodiscard]] float dedx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dedx, row));
    }
    /// r_helix: Radius of the helix fit (mm)
    [[nodiscard]] float r_helix(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_r_helix, row));
    }
    /// x_helix: X-coordinate of the center of helix fit (mm)
    [[nodiscard]] float x_helix(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x_helix, row));
    }
    /// y_helix: Y-coordinate of the center of helix fit (mm)
    [[nodiscard]] float y_helix(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y_helix, row));
    }
    /// chi2_helix: Chi squared of the helix fit
    [[nodiscard]] float chi2_helix(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2_helix, row));
    }
    /// min_row: Pad row of hit with smallest time
    [[nodiscard]] float min_row(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_min_row, row));
    }
    /// min_col: Pad col of hit with smallest time
    [[nodiscard]] float min_col(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_min_col, row));
    }
    /// max_row: Pad row of hit with largest time
    [[nodiscard]] float max_row(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_max_row, row));
    }
    /// max_col: Pad col of hit with largest time
    [[nodiscard]] float max_col(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_max_col, row));
    }
    /// min_time: Smallest time in track not shifted (ns)
    [[nodiscard]] float min_time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_min_time, row));
    }
    /// max_time: Largest time in track not shifted (ns)
    [[nodiscard]] float max_time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_max_time, row));
    }
    /// min_radius: Smallest reconstructed radius (mm)
    [[nodiscard]] float min_radius(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_min_radius, row));
    }
    /// max_radius: Largest reconstructed radius (mm)
    [[nodiscard]] float max_radius(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_max_radius, row));
    }
    /// min_phi: Smallest reconstructed phi (rad)
    [[nodiscard]] float min_phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_min_phi, row));
    }
    /// max_phi: Largest reconstructed phi (rad)
    [[nodiscard]] float max_phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_max_phi, row));
    }
    /// tdiff: Time shift (ns)
    [[nodiscard]] float tdiff(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tdiff, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_trkID = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_vz = -1;
    int i_theta = -1;
    int i_phi = -1;
    int i_nhits = -1;
    int i_path = -1;
    int i_adcsum = -1;
    int i_dedx = -1;
    int i_r_helix = -1;
    int i_x_helix = -1;
    int i_y_helix = -1;
    int i_chi2_helix = -1;
    int i_min_row = -1;
    int i_min_col = -1;
    int i_max_row = -1;
    int i_max_col = -1;
    int i_min_time = -1;
    int i_max_time = -1;
    int i_min_radius = -1;
    int i_max_radius = -1;
    int i_min_phi = -1;
    int i_max_phi = -1;
    int i_tdiff = -1;

    friend class Row;
};

/// Lightweight row view for RTPC::tracks. Stores a pointer + row index only.
class RTPC_tracks::Row {
 public:
    Row(const RTPC_tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float theta() const noexcept { return m_bank->theta(m_row); }
    [[nodiscard]] float phi() const noexcept { return m_bank->phi(m_row); }
    [[nodiscard]] int nhits() const noexcept { return m_bank->nhits(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float adcsum() const noexcept { return m_bank->adcsum(m_row); }
    [[nodiscard]] float dedx() const noexcept { return m_bank->dedx(m_row); }
    [[nodiscard]] float r_helix() const noexcept { return m_bank->r_helix(m_row); }
    [[nodiscard]] float x_helix() const noexcept { return m_bank->x_helix(m_row); }
    [[nodiscard]] float y_helix() const noexcept { return m_bank->y_helix(m_row); }
    [[nodiscard]] float chi2_helix() const noexcept { return m_bank->chi2_helix(m_row); }
    [[nodiscard]] float min_row() const noexcept { return m_bank->min_row(m_row); }
    [[nodiscard]] float min_col() const noexcept { return m_bank->min_col(m_row); }
    [[nodiscard]] float max_row() const noexcept { return m_bank->max_row(m_row); }
    [[nodiscard]] float max_col() const noexcept { return m_bank->max_col(m_row); }
    [[nodiscard]] float min_time() const noexcept { return m_bank->min_time(m_row); }
    [[nodiscard]] float max_time() const noexcept { return m_bank->max_time(m_row); }
    [[nodiscard]] float min_radius() const noexcept { return m_bank->min_radius(m_row); }
    [[nodiscard]] float max_radius() const noexcept { return m_bank->max_radius(m_row); }
    [[nodiscard]] float min_phi() const noexcept { return m_bank->min_phi(m_row); }
    [[nodiscard]] float max_phi() const noexcept { return m_bank->max_phi(m_row); }
    [[nodiscard]] float tdiff() const noexcept { return m_bank->tdiff(m_row); }
 private:
    const RTPC_tracks* m_bank;
    int m_row;
};

inline RTPC_tracks::Row RTPC_tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
