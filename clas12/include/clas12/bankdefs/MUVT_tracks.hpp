// =============================================================================
// Auto-generated from mu.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `MUVT::tracks` (group 22900, item 36).
/// reconstructed tracks using MUVT information
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// MUVT_tracks bank;
/// if (auto sc = bank.bind(event.get_bank("MUVT::tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class MUVT_tracks {
 public:
    static constexpr std::string_view bank_name = "MUVT::tracks";
    static constexpr int bank_group = 22900;
    static constexpr int bank_item  = 36;

    class Row;

    MUVT_tracks() = default;

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
            int i_index = -1;
            int i_status = -1;
            int i_sector = -1;
            int i_vx = -1;
            int i_vy = -1;
            int i_vz = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_charge = -1;
            int i_chi2 = -1;
            int i_NDF = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_vx = schema.exists("vx") ? schema.getEntryOrder("vx") : -1;
            cache.i_vy = schema.exists("vy") ? schema.getEntryOrder("vy") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_charge = schema.exists("charge") ? schema.getEntryOrder("charge") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_NDF = schema.exists("NDF") ? schema.getEntryOrder("NDF") : -1;
        }
        i_index = cache.i_index;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_vx = cache.i_vx;
        i_vy = cache.i_vy;
        i_vz = cache.i_vz;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_charge = cache.i_charge;
        i_chi2 = cache.i_chi2;
        i_NDF = cache.i_NDF;
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
    /// index: index of the track in the DC bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }
    /// status: status of the track (0: refitted using FMT, 1: original DC track)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_status, row));
    }
    /// sector: sector of the track in DC
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// vx: Vertex x-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vx, row));
    }
    /// vy: Vertex y-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vy, row));
    }
    /// vz: Vertex z-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }
    /// px: 3-momentum x-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: 3-momentum y-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: 3-momentum z-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// charge: charge of the track
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_charge, row));
    }
    /// chi2: chi^2 of the fit
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// NDF: number of degrees of freedom of the fit
    [[nodiscard]] int NDF(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_NDF, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int index_handle() const noexcept { return i_index; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_index = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_vx = -1;
    int i_vy = -1;
    int i_vz = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_charge = -1;
    int i_chi2 = -1;
    int i_NDF = -1;

    friend class Row;
};

/// Lightweight row view for MUVT::tracks. Stores a pointer + row index only.
class MUVT_tracks::Row {
 public:
    Row(const MUVT_tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] float vx() const noexcept { return m_bank->vx(m_row); }
    [[nodiscard]] float vy() const noexcept { return m_bank->vy(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] int charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int NDF() const noexcept { return m_bank->NDF(m_row); }
 private:
    const MUVT_tracks* m_bank;
    int m_row;
};

inline MUVT_tracks::Row MUVT_tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
