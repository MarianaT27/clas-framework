// =============================================================================
// Auto-generated from dc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `TimeBasedTrkg::TBTracks` (group 20600, item 36).
/// reconstructed tracks using DC timing information
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// TimeBasedTrkg_TBTracks bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::TBTracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_TBTracks {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::TBTracks";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 36;

    class Row;

    TimeBasedTrkg_TBTracks() = default;

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
            int i_id = -1;
            int i_status = -1;
            int i_sector = -1;
            int i_c1_x = -1;
            int i_c1_y = -1;
            int i_c1_z = -1;
            int i_c1_ux = -1;
            int i_c1_uy = -1;
            int i_c1_uz = -1;
            int i_c3_x = -1;
            int i_c3_y = -1;
            int i_c3_z = -1;
            int i_c3_ux = -1;
            int i_c3_uy = -1;
            int i_c3_uz = -1;
            int i_t1_x = -1;
            int i_t1_y = -1;
            int i_t1_z = -1;
            int i_t1_px = -1;
            int i_t1_py = -1;
            int i_t1_pz = -1;
            int i_Vtx0_x = -1;
            int i_Vtx0_y = -1;
            int i_Vtx0_z = -1;
            int i_p0_x = -1;
            int i_p0_y = -1;
            int i_p0_z = -1;
            int i_Cross1_ID = -1;
            int i_Cross2_ID = -1;
            int i_Cross3_ID = -1;
            int i_Cluster1_ID = -1;
            int i_Cluster2_ID = -1;
            int i_Cluster3_ID = -1;
            int i_Cluster4_ID = -1;
            int i_Cluster5_ID = -1;
            int i_Cluster6_ID = -1;
            int i_q = -1;
            int i_pathlength = -1;
            int i_chi2 = -1;
            int i_ndf = -1;
            int i_ndf0 = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_c1_x = schema.exists("c1_x") ? schema.getEntryOrder("c1_x") : -1;
            cache.i_c1_y = schema.exists("c1_y") ? schema.getEntryOrder("c1_y") : -1;
            cache.i_c1_z = schema.exists("c1_z") ? schema.getEntryOrder("c1_z") : -1;
            cache.i_c1_ux = schema.exists("c1_ux") ? schema.getEntryOrder("c1_ux") : -1;
            cache.i_c1_uy = schema.exists("c1_uy") ? schema.getEntryOrder("c1_uy") : -1;
            cache.i_c1_uz = schema.exists("c1_uz") ? schema.getEntryOrder("c1_uz") : -1;
            cache.i_c3_x = schema.exists("c3_x") ? schema.getEntryOrder("c3_x") : -1;
            cache.i_c3_y = schema.exists("c3_y") ? schema.getEntryOrder("c3_y") : -1;
            cache.i_c3_z = schema.exists("c3_z") ? schema.getEntryOrder("c3_z") : -1;
            cache.i_c3_ux = schema.exists("c3_ux") ? schema.getEntryOrder("c3_ux") : -1;
            cache.i_c3_uy = schema.exists("c3_uy") ? schema.getEntryOrder("c3_uy") : -1;
            cache.i_c3_uz = schema.exists("c3_uz") ? schema.getEntryOrder("c3_uz") : -1;
            cache.i_t1_x = schema.exists("t1_x") ? schema.getEntryOrder("t1_x") : -1;
            cache.i_t1_y = schema.exists("t1_y") ? schema.getEntryOrder("t1_y") : -1;
            cache.i_t1_z = schema.exists("t1_z") ? schema.getEntryOrder("t1_z") : -1;
            cache.i_t1_px = schema.exists("t1_px") ? schema.getEntryOrder("t1_px") : -1;
            cache.i_t1_py = schema.exists("t1_py") ? schema.getEntryOrder("t1_py") : -1;
            cache.i_t1_pz = schema.exists("t1_pz") ? schema.getEntryOrder("t1_pz") : -1;
            cache.i_Vtx0_x = schema.exists("Vtx0_x") ? schema.getEntryOrder("Vtx0_x") : -1;
            cache.i_Vtx0_y = schema.exists("Vtx0_y") ? schema.getEntryOrder("Vtx0_y") : -1;
            cache.i_Vtx0_z = schema.exists("Vtx0_z") ? schema.getEntryOrder("Vtx0_z") : -1;
            cache.i_p0_x = schema.exists("p0_x") ? schema.getEntryOrder("p0_x") : -1;
            cache.i_p0_y = schema.exists("p0_y") ? schema.getEntryOrder("p0_y") : -1;
            cache.i_p0_z = schema.exists("p0_z") ? schema.getEntryOrder("p0_z") : -1;
            cache.i_Cross1_ID = schema.exists("Cross1_ID") ? schema.getEntryOrder("Cross1_ID") : -1;
            cache.i_Cross2_ID = schema.exists("Cross2_ID") ? schema.getEntryOrder("Cross2_ID") : -1;
            cache.i_Cross3_ID = schema.exists("Cross3_ID") ? schema.getEntryOrder("Cross3_ID") : -1;
            cache.i_Cluster1_ID = schema.exists("Cluster1_ID") ? schema.getEntryOrder("Cluster1_ID") : -1;
            cache.i_Cluster2_ID = schema.exists("Cluster2_ID") ? schema.getEntryOrder("Cluster2_ID") : -1;
            cache.i_Cluster3_ID = schema.exists("Cluster3_ID") ? schema.getEntryOrder("Cluster3_ID") : -1;
            cache.i_Cluster4_ID = schema.exists("Cluster4_ID") ? schema.getEntryOrder("Cluster4_ID") : -1;
            cache.i_Cluster5_ID = schema.exists("Cluster5_ID") ? schema.getEntryOrder("Cluster5_ID") : -1;
            cache.i_Cluster6_ID = schema.exists("Cluster6_ID") ? schema.getEntryOrder("Cluster6_ID") : -1;
            cache.i_q = schema.exists("q") ? schema.getEntryOrder("q") : -1;
            cache.i_pathlength = schema.exists("pathlength") ? schema.getEntryOrder("pathlength") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_ndf = schema.exists("ndf") ? schema.getEntryOrder("ndf") : -1;
            cache.i_ndf0 = schema.exists("ndf0") ? schema.getEntryOrder("ndf0") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_c1_x = cache.i_c1_x;
        i_c1_y = cache.i_c1_y;
        i_c1_z = cache.i_c1_z;
        i_c1_ux = cache.i_c1_ux;
        i_c1_uy = cache.i_c1_uy;
        i_c1_uz = cache.i_c1_uz;
        i_c3_x = cache.i_c3_x;
        i_c3_y = cache.i_c3_y;
        i_c3_z = cache.i_c3_z;
        i_c3_ux = cache.i_c3_ux;
        i_c3_uy = cache.i_c3_uy;
        i_c3_uz = cache.i_c3_uz;
        i_t1_x = cache.i_t1_x;
        i_t1_y = cache.i_t1_y;
        i_t1_z = cache.i_t1_z;
        i_t1_px = cache.i_t1_px;
        i_t1_py = cache.i_t1_py;
        i_t1_pz = cache.i_t1_pz;
        i_Vtx0_x = cache.i_Vtx0_x;
        i_Vtx0_y = cache.i_Vtx0_y;
        i_Vtx0_z = cache.i_Vtx0_z;
        i_p0_x = cache.i_p0_x;
        i_p0_y = cache.i_p0_y;
        i_p0_z = cache.i_p0_z;
        i_Cross1_ID = cache.i_Cross1_ID;
        i_Cross2_ID = cache.i_Cross2_ID;
        i_Cross3_ID = cache.i_Cross3_ID;
        i_Cluster1_ID = cache.i_Cluster1_ID;
        i_Cluster2_ID = cache.i_Cluster2_ID;
        i_Cluster3_ID = cache.i_Cluster3_ID;
        i_Cluster4_ID = cache.i_Cluster4_ID;
        i_Cluster5_ID = cache.i_Cluster5_ID;
        i_Cluster6_ID = cache.i_Cluster6_ID;
        i_q = cache.i_q;
        i_pathlength = cache.i_pathlength;
        i_chi2 = cache.i_chi2;
        i_ndf = cache.i_ndf;
        i_ndf0 = cache.i_ndf0;
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
    /// id: id of the track
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// status: status of the track
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// sector: sector of the track
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// c1_x: Upstream Region 1 cross x-position in the lab (in cm)
    [[nodiscard]] float c1_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c1_x, row));
    }
    /// c1_y: Upstream Region 1 cross y-position in the lab (in cm)
    [[nodiscard]] float c1_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c1_y, row));
    }
    /// c1_z: Upstream Region 1 cross z-position in the lab (in cm)
    [[nodiscard]] float c1_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c1_z, row));
    }
    /// c1_ux: Upstream Region 1 cross unit x-direction vector in the lab
    [[nodiscard]] float c1_ux(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c1_ux, row));
    }
    /// c1_uy: Upstream Region 1 cross unit y-direction vector in the lab
    [[nodiscard]] float c1_uy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c1_uy, row));
    }
    /// c1_uz: Upstream Region 1 cross unit z-direction vector in the lab
    [[nodiscard]] float c1_uz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c1_uz, row));
    }
    /// c3_x: Downstream Region 3 cross x-position in the lab (in cm)
    [[nodiscard]] float c3_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c3_x, row));
    }
    /// c3_y: Downstream Region 3 cross y-position in the lab (in cm)
    [[nodiscard]] float c3_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c3_y, row));
    }
    /// c3_z: Downstream Region 3 cross z-position in the lab (in cm)
    [[nodiscard]] float c3_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c3_z, row));
    }
    /// c3_ux: Downstream Region 3 cross unit x-direction vector in the lab
    [[nodiscard]] float c3_ux(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c3_ux, row));
    }
    /// c3_uy: Downstream Region 3 cross unit y-direction vector in the lab
    [[nodiscard]] float c3_uy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c3_uy, row));
    }
    /// c3_uz: Downstream Region 3 cross unit z-direction vector in the lab
    [[nodiscard]] float c3_uz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_c3_uz, row));
    }
    /// t1_x: Upstream Region 1 track x-position in the lab (in cm)
    [[nodiscard]] float t1_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_t1_x, row));
    }
    /// t1_y: Upstream Region 1 track y-position in the lab (in cm)
    [[nodiscard]] float t1_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_t1_y, row));
    }
    /// t1_z: Upstream Region 1 track z-position in the lab (in cm)
    [[nodiscard]] float t1_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_t1_z, row));
    }
    /// t1_px: Upstream Region 1 track unit x-momentum vector in the lab
    [[nodiscard]] float t1_px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_t1_px, row));
    }
    /// t1_py: Upstream Region 1 track unit y-momentum vector in the lab
    [[nodiscard]] float t1_py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_t1_py, row));
    }
    /// t1_pz: Upstream Region 1 track unit z-momentum vector in the lab
    [[nodiscard]] float t1_pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_t1_pz, row));
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
    /// p0_x: 3-momentum x-coordinate of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float p0_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p0_x, row));
    }
    /// p0_y: 3-momentum y-coordinate of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float p0_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p0_y, row));
    }
    /// p0_z: 3-momentum z-coordinate of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float p0_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p0_z, row));
    }
    /// Cross1_ID: id of first cross on track
    [[nodiscard]] int Cross1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross1_ID, row));
    }
    /// Cross2_ID: id of second cross on track
    [[nodiscard]] int Cross2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross2_ID, row));
    }
    /// Cross3_ID: id of third cross on track
    [[nodiscard]] int Cross3_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cross3_ID, row));
    }
    /// Cluster1_ID: cluster ID in superlayer 1
    [[nodiscard]] int Cluster1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster1_ID, row));
    }
    /// Cluster2_ID: cluster ID in superlayer 2
    [[nodiscard]] int Cluster2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster2_ID, row));
    }
    /// Cluster3_ID: cluster ID in superlayer 3
    [[nodiscard]] int Cluster3_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster3_ID, row));
    }
    /// Cluster4_ID: cluster ID in superlayer 4
    [[nodiscard]] int Cluster4_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster4_ID, row));
    }
    /// Cluster5_ID: cluster ID in superlayer 5
    [[nodiscard]] int Cluster5_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster5_ID, row));
    }
    /// Cluster6_ID: cluster ID in superlayer 6
    [[nodiscard]] int Cluster6_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster6_ID, row));
    }
    /// q: charge of the track
    [[nodiscard]] int q(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_q, row));
    }
    /// pathlength: pathlength of the track
    [[nodiscard]] float pathlength(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pathlength, row));
    }
    /// chi2: fit chi2 of the track
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// ndf: fit ndf weightd by DAF for the track
    [[nodiscard]] int ndf(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ndf, row));
    }
    /// ndf0: traditional ndf for the track
    [[nodiscard]] int ndf0(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ndf0, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_c1_x = -1;
    int i_c1_y = -1;
    int i_c1_z = -1;
    int i_c1_ux = -1;
    int i_c1_uy = -1;
    int i_c1_uz = -1;
    int i_c3_x = -1;
    int i_c3_y = -1;
    int i_c3_z = -1;
    int i_c3_ux = -1;
    int i_c3_uy = -1;
    int i_c3_uz = -1;
    int i_t1_x = -1;
    int i_t1_y = -1;
    int i_t1_z = -1;
    int i_t1_px = -1;
    int i_t1_py = -1;
    int i_t1_pz = -1;
    int i_Vtx0_x = -1;
    int i_Vtx0_y = -1;
    int i_Vtx0_z = -1;
    int i_p0_x = -1;
    int i_p0_y = -1;
    int i_p0_z = -1;
    int i_Cross1_ID = -1;
    int i_Cross2_ID = -1;
    int i_Cross3_ID = -1;
    int i_Cluster1_ID = -1;
    int i_Cluster2_ID = -1;
    int i_Cluster3_ID = -1;
    int i_Cluster4_ID = -1;
    int i_Cluster5_ID = -1;
    int i_Cluster6_ID = -1;
    int i_q = -1;
    int i_pathlength = -1;
    int i_chi2 = -1;
    int i_ndf = -1;
    int i_ndf0 = -1;

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::TBTracks. Stores a pointer + row index only.
class TimeBasedTrkg_TBTracks::Row {
 public:
    Row(const TimeBasedTrkg_TBTracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] float c1_x() const noexcept { return m_bank->c1_x(m_row); }
    [[nodiscard]] float c1_y() const noexcept { return m_bank->c1_y(m_row); }
    [[nodiscard]] float c1_z() const noexcept { return m_bank->c1_z(m_row); }
    [[nodiscard]] float c1_ux() const noexcept { return m_bank->c1_ux(m_row); }
    [[nodiscard]] float c1_uy() const noexcept { return m_bank->c1_uy(m_row); }
    [[nodiscard]] float c1_uz() const noexcept { return m_bank->c1_uz(m_row); }
    [[nodiscard]] float c3_x() const noexcept { return m_bank->c3_x(m_row); }
    [[nodiscard]] float c3_y() const noexcept { return m_bank->c3_y(m_row); }
    [[nodiscard]] float c3_z() const noexcept { return m_bank->c3_z(m_row); }
    [[nodiscard]] float c3_ux() const noexcept { return m_bank->c3_ux(m_row); }
    [[nodiscard]] float c3_uy() const noexcept { return m_bank->c3_uy(m_row); }
    [[nodiscard]] float c3_uz() const noexcept { return m_bank->c3_uz(m_row); }
    [[nodiscard]] float t1_x() const noexcept { return m_bank->t1_x(m_row); }
    [[nodiscard]] float t1_y() const noexcept { return m_bank->t1_y(m_row); }
    [[nodiscard]] float t1_z() const noexcept { return m_bank->t1_z(m_row); }
    [[nodiscard]] float t1_px() const noexcept { return m_bank->t1_px(m_row); }
    [[nodiscard]] float t1_py() const noexcept { return m_bank->t1_py(m_row); }
    [[nodiscard]] float t1_pz() const noexcept { return m_bank->t1_pz(m_row); }
    [[nodiscard]] float Vtx0_x() const noexcept { return m_bank->Vtx0_x(m_row); }
    [[nodiscard]] float Vtx0_y() const noexcept { return m_bank->Vtx0_y(m_row); }
    [[nodiscard]] float Vtx0_z() const noexcept { return m_bank->Vtx0_z(m_row); }
    [[nodiscard]] float p0_x() const noexcept { return m_bank->p0_x(m_row); }
    [[nodiscard]] float p0_y() const noexcept { return m_bank->p0_y(m_row); }
    [[nodiscard]] float p0_z() const noexcept { return m_bank->p0_z(m_row); }
    [[nodiscard]] int Cross1_ID() const noexcept { return m_bank->Cross1_ID(m_row); }
    [[nodiscard]] int Cross2_ID() const noexcept { return m_bank->Cross2_ID(m_row); }
    [[nodiscard]] int Cross3_ID() const noexcept { return m_bank->Cross3_ID(m_row); }
    [[nodiscard]] int Cluster1_ID() const noexcept { return m_bank->Cluster1_ID(m_row); }
    [[nodiscard]] int Cluster2_ID() const noexcept { return m_bank->Cluster2_ID(m_row); }
    [[nodiscard]] int Cluster3_ID() const noexcept { return m_bank->Cluster3_ID(m_row); }
    [[nodiscard]] int Cluster4_ID() const noexcept { return m_bank->Cluster4_ID(m_row); }
    [[nodiscard]] int Cluster5_ID() const noexcept { return m_bank->Cluster5_ID(m_row); }
    [[nodiscard]] int Cluster6_ID() const noexcept { return m_bank->Cluster6_ID(m_row); }
    [[nodiscard]] int q() const noexcept { return m_bank->q(m_row); }
    [[nodiscard]] float pathlength() const noexcept { return m_bank->pathlength(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int ndf() const noexcept { return m_bank->ndf(m_row); }
    [[nodiscard]] int ndf0() const noexcept { return m_bank->ndf0(m_row); }
 private:
    const TimeBasedTrkg_TBTracks* m_bank;
    int m_row;
};

inline TimeBasedTrkg_TBTracks::Row TimeBasedTrkg_TBTracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
