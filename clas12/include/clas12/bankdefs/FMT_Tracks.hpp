// =============================================================================
// Auto-generated from fmt.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `FMT::Tracks` (group 20800, item 36).
/// reconstructed tracks using FMT information
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FMT_Tracks bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class FMT_Tracks {
 public:
    static constexpr std::string_view bank_name = "FMT::Tracks";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 36;

    class Row;

    FMT_Tracks() = default;

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
            int i_index = -1;
            int i_status = -1;
            int i_sector = -1;
            int i_Vtx0_x = -1;
            int i_Vtx0_y = -1;
            int i_Vtx0_z = -1;
            int i_p0_x = -1;
            int i_p0_y = -1;
            int i_p0_z = -1;
            int i_q = -1;
            int i_chi2 = -1;
            int i_NDF = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_Vtx0_x = schema.exists("Vtx0_x") ? schema.getEntryOrder("Vtx0_x") : -1;
            cache.i_Vtx0_y = schema.exists("Vtx0_y") ? schema.getEntryOrder("Vtx0_y") : -1;
            cache.i_Vtx0_z = schema.exists("Vtx0_z") ? schema.getEntryOrder("Vtx0_z") : -1;
            cache.i_p0_x = schema.exists("p0_x") ? schema.getEntryOrder("p0_x") : -1;
            cache.i_p0_y = schema.exists("p0_y") ? schema.getEntryOrder("p0_y") : -1;
            cache.i_p0_z = schema.exists("p0_z") ? schema.getEntryOrder("p0_z") : -1;
            cache.i_q = schema.exists("q") ? schema.getEntryOrder("q") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_NDF = schema.exists("NDF") ? schema.getEntryOrder("NDF") : -1;
        }
        i_index = cache.i_index;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_Vtx0_x = cache.i_Vtx0_x;
        i_Vtx0_y = cache.i_Vtx0_y;
        i_Vtx0_z = cache.i_Vtx0_z;
        i_p0_x = cache.i_p0_x;
        i_p0_y = cache.i_p0_y;
        i_p0_z = cache.i_p0_z;
        i_q = cache.i_q;
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
    /// Vtx0_x: Vertex x-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_Vtx0_x, row));
    }
    /// Vtx0_y: Vertex y-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_Vtx0_y, row));
    }
    /// Vtx0_z: Vertex z-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_Vtx0_z, row));
    }
    /// p0_x: 3-momentum x-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float p0_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p0_x, row));
    }
    /// p0_y: 3-momentum y-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float p0_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p0_y, row));
    }
    /// p0_z: 3-momentum z-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float p0_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p0_z, row));
    }
    /// q: charge of the track
    [[nodiscard]] int q(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_q, row));
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
    int i_Vtx0_x = -1;
    int i_Vtx0_y = -1;
    int i_Vtx0_z = -1;
    int i_p0_x = -1;
    int i_p0_y = -1;
    int i_p0_z = -1;
    int i_q = -1;
    int i_chi2 = -1;
    int i_NDF = -1;

    friend class Row;
};

/// Lightweight row view for FMT::Tracks. Stores a pointer + row index only.
class FMT_Tracks::Row {
 public:
    Row(const FMT_Tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] float Vtx0_x() const noexcept { return m_bank->Vtx0_x(m_row); }
    [[nodiscard]] float Vtx0_y() const noexcept { return m_bank->Vtx0_y(m_row); }
    [[nodiscard]] float Vtx0_z() const noexcept { return m_bank->Vtx0_z(m_row); }
    [[nodiscard]] float p0_x() const noexcept { return m_bank->p0_x(m_row); }
    [[nodiscard]] float p0_y() const noexcept { return m_bank->p0_y(m_row); }
    [[nodiscard]] float p0_z() const noexcept { return m_bank->p0_z(m_row); }
    [[nodiscard]] int q() const noexcept { return m_bank->q(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int NDF() const noexcept { return m_bank->NDF(m_row); }
 private:
    const FMT_Tracks* m_bank;
    int m_row;
};

inline FMT_Tracks::Row FMT_Tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
