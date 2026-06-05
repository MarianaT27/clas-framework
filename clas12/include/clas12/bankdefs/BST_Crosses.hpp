// =============================================================================
// Auto-generated from bst.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `BST::Crosses` (group 20200, item 35).
/// reconstructed BST crosses
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// BST_Crosses bank;
/// if (auto sc = bank.bind(event.get_bank("BST::Crosses")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class BST_Crosses {
 public:
    static constexpr std::string_view bank_name = "BST::Crosses";
    static constexpr int bank_group = 20200;
    static constexpr int bank_item  = 35;
    static constexpr std::string_view bank_schema = "ID/S,sector/B,region/B,x/F,y/F,z/F,err_x/F,err_y/F,err_z/F,x0/F,y0/F,z0/F,err_x0/F,err_y0/F,err_z0/F,ux/F,uy/F,uz/F,Cluster1_ID/S,Cluster2_ID/S,trkID/S";

    class Row;

    BST_Crosses() = default;

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
            int i_sector = -1;
            int i_region = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_err_x = -1;
            int i_err_y = -1;
            int i_err_z = -1;
            int i_x0 = -1;
            int i_y0 = -1;
            int i_z0 = -1;
            int i_err_x0 = -1;
            int i_err_y0 = -1;
            int i_err_z0 = -1;
            int i_ux = -1;
            int i_uy = -1;
            int i_uz = -1;
            int i_Cluster1_ID = -1;
            int i_Cluster2_ID = -1;
            int i_trkID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_ID = schema.exists("ID") ? schema.getEntryOrder("ID") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_region = schema.exists("region") ? schema.getEntryOrder("region") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_err_x = schema.exists("err_x") ? schema.getEntryOrder("err_x") : -1;
            cache.i_err_y = schema.exists("err_y") ? schema.getEntryOrder("err_y") : -1;
            cache.i_err_z = schema.exists("err_z") ? schema.getEntryOrder("err_z") : -1;
            cache.i_x0 = schema.exists("x0") ? schema.getEntryOrder("x0") : -1;
            cache.i_y0 = schema.exists("y0") ? schema.getEntryOrder("y0") : -1;
            cache.i_z0 = schema.exists("z0") ? schema.getEntryOrder("z0") : -1;
            cache.i_err_x0 = schema.exists("err_x0") ? schema.getEntryOrder("err_x0") : -1;
            cache.i_err_y0 = schema.exists("err_y0") ? schema.getEntryOrder("err_y0") : -1;
            cache.i_err_z0 = schema.exists("err_z0") ? schema.getEntryOrder("err_z0") : -1;
            cache.i_ux = schema.exists("ux") ? schema.getEntryOrder("ux") : -1;
            cache.i_uy = schema.exists("uy") ? schema.getEntryOrder("uy") : -1;
            cache.i_uz = schema.exists("uz") ? schema.getEntryOrder("uz") : -1;
            cache.i_Cluster1_ID = schema.exists("Cluster1_ID") ? schema.getEntryOrder("Cluster1_ID") : -1;
            cache.i_Cluster2_ID = schema.exists("Cluster2_ID") ? schema.getEntryOrder("Cluster2_ID") : -1;
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
        }
        i_ID = cache.i_ID;
        i_sector = cache.i_sector;
        i_region = cache.i_region;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_err_x = cache.i_err_x;
        i_err_y = cache.i_err_y;
        i_err_z = cache.i_err_z;
        i_x0 = cache.i_x0;
        i_y0 = cache.i_y0;
        i_z0 = cache.i_z0;
        i_err_x0 = cache.i_err_x0;
        i_err_y0 = cache.i_err_y0;
        i_err_z0 = cache.i_err_z0;
        i_ux = cache.i_ux;
        i_uy = cache.i_uy;
        i_uz = cache.i_uz;
        i_Cluster1_ID = cache.i_Cluster1_ID;
        i_Cluster2_ID = cache.i_Cluster2_ID;
        i_trkID = cache.i_trkID;
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
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// region: region
    [[nodiscard]] int region(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_region, row));
    }
    /// x: BST cross x-coordinate
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: BST cross y-coordinate
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: BST cross z-coordinate
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// err_x: BST cross x-coordinate error
    [[nodiscard]] float err_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_x, row));
    }
    /// err_y: BST cross y-coordinate error
    [[nodiscard]] float err_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_y, row));
    }
    /// err_z: BST cross z-coordinate error
    [[nodiscard]] float err_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_z, row));
    }
    /// x0: BST cross x0-coordinate
    [[nodiscard]] float x0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x0, row));
    }
    /// y0: BST cross y0-coordinate
    [[nodiscard]] float y0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y0, row));
    }
    /// z0: BST cross z0-coordinate
    [[nodiscard]] float z0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z0, row));
    }
    /// err_x0: BST cross x0-coordinate error
    [[nodiscard]] float err_x0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_x0, row));
    }
    /// err_y0: BST cross y0-coordinate error
    [[nodiscard]] float err_y0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_y0, row));
    }
    /// err_z0: BST cross z0-coordinate error
    [[nodiscard]] float err_z0(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_z0, row));
    }
    /// ux: BST cross x-direction (track unit tangent vector at the cross)
    [[nodiscard]] float ux(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ux, row));
    }
    /// uy: BST cross y-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_uy, row));
    }
    /// uz: BST cross z-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_uz, row));
    }
    /// Cluster1_ID: ID of the  cluster in the Cross
    [[nodiscard]] int Cluster1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster1_ID, row));
    }
    /// Cluster2_ID: ID of the top layer  cluster in the Cross
    [[nodiscard]] int Cluster2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Cluster2_ID, row));
    }
    /// trkID: associated track ID
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trkID, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_ID = -1;
    int i_sector = -1;
    int i_region = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_err_x = -1;
    int i_err_y = -1;
    int i_err_z = -1;
    int i_x0 = -1;
    int i_y0 = -1;
    int i_z0 = -1;
    int i_err_x0 = -1;
    int i_err_y0 = -1;
    int i_err_z0 = -1;
    int i_ux = -1;
    int i_uy = -1;
    int i_uz = -1;
    int i_Cluster1_ID = -1;
    int i_Cluster2_ID = -1;
    int i_trkID = -1;

    friend class Row;
};

/// Lightweight row view for BST::Crosses. Stores a pointer + row index only.
class BST_Crosses::Row {
 public:
    Row(const BST_Crosses* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int region() const noexcept { return m_bank->region(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float err_x() const noexcept { return m_bank->err_x(m_row); }
    [[nodiscard]] float err_y() const noexcept { return m_bank->err_y(m_row); }
    [[nodiscard]] float err_z() const noexcept { return m_bank->err_z(m_row); }
    [[nodiscard]] float x0() const noexcept { return m_bank->x0(m_row); }
    [[nodiscard]] float y0() const noexcept { return m_bank->y0(m_row); }
    [[nodiscard]] float z0() const noexcept { return m_bank->z0(m_row); }
    [[nodiscard]] float err_x0() const noexcept { return m_bank->err_x0(m_row); }
    [[nodiscard]] float err_y0() const noexcept { return m_bank->err_y0(m_row); }
    [[nodiscard]] float err_z0() const noexcept { return m_bank->err_z0(m_row); }
    [[nodiscard]] float ux() const noexcept { return m_bank->ux(m_row); }
    [[nodiscard]] float uy() const noexcept { return m_bank->uy(m_row); }
    [[nodiscard]] float uz() const noexcept { return m_bank->uz(m_row); }
    [[nodiscard]] int Cluster1_ID() const noexcept { return m_bank->Cluster1_ID(m_row); }
    [[nodiscard]] int Cluster2_ID() const noexcept { return m_bank->Cluster2_ID(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
 private:
    const BST_Crosses* m_bank;
    int m_row;
};

inline BST_Crosses::Row BST_Crosses::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
