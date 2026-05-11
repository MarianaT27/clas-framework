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


/// HIPO bank wrapper for `FMT::Crosses` (group 20800, item 25).
/// reconstructed FMT crosses
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FMT_Crosses bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Crosses")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class FMT_Crosses {
 public:
    static constexpr std::string_view bank_name = "FMT::Crosses";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 25;

    class Row;

    FMT_Crosses() = default;

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
            int i_sector = -1;
            int i_region = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_err_x = -1;
            int i_err_y = -1;
            int i_err_z = -1;
            int i_ux = -1;
            int i_uy = -1;
            int i_uz = -1;
            int i_cluster1Index = -1;
            int i_cluster2Index = -1;
            int i_trkID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_region = schema.exists("region") ? schema.getEntryOrder("region") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_err_x = schema.exists("err_x") ? schema.getEntryOrder("err_x") : -1;
            cache.i_err_y = schema.exists("err_y") ? schema.getEntryOrder("err_y") : -1;
            cache.i_err_z = schema.exists("err_z") ? schema.getEntryOrder("err_z") : -1;
            cache.i_ux = schema.exists("ux") ? schema.getEntryOrder("ux") : -1;
            cache.i_uy = schema.exists("uy") ? schema.getEntryOrder("uy") : -1;
            cache.i_uz = schema.exists("uz") ? schema.getEntryOrder("uz") : -1;
            cache.i_cluster1Index = schema.exists("cluster1Index") ? schema.getEntryOrder("cluster1Index") : -1;
            cache.i_cluster2Index = schema.exists("cluster2Index") ? schema.getEntryOrder("cluster2Index") : -1;
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
        }
        i_index = cache.i_index;
        i_sector = cache.i_sector;
        i_region = cache.i_region;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_err_x = cache.i_err_x;
        i_err_y = cache.i_err_y;
        i_err_z = cache.i_err_z;
        i_ux = cache.i_ux;
        i_uy = cache.i_uy;
        i_uz = cache.i_uz;
        i_cluster1Index = cache.i_cluster1Index;
        i_cluster2Index = cache.i_cluster2Index;
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
    /// index: index
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// region: region
    [[nodiscard]] int region(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_region, row));
    }
    /// x: cross x-coordinate
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: cross y-coordinate
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: cross z-coordinate
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// err_x: cross x-coordinate error
    [[nodiscard]] float err_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_x, row));
    }
    /// err_y: cross y-coordinate error
    [[nodiscard]] float err_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_y, row));
    }
    /// err_z: cross z-coordinate error
    [[nodiscard]] float err_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_z, row));
    }
    /// ux: cross x-direction (track unit tangent vector at the cross)
    [[nodiscard]] float ux(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ux, row));
    }
    /// uy: cross y-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_uy, row));
    }
    /// uz: cross z-direction (track unit tangent vector at the cross)
    [[nodiscard]] float uz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_uz, row));
    }
    /// cluster1Index: index of the  cluster in the Cross
    [[nodiscard]] int cluster1Index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_cluster1Index, row));
    }
    /// cluster2Index: index of the top layer  cluster in the Cross
    [[nodiscard]] int cluster2Index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_cluster2Index, row));
    }
    /// trkID: associated track index
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trkID, row));
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
    int i_sector = -1;
    int i_region = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_err_x = -1;
    int i_err_y = -1;
    int i_err_z = -1;
    int i_ux = -1;
    int i_uy = -1;
    int i_uz = -1;
    int i_cluster1Index = -1;
    int i_cluster2Index = -1;
    int i_trkID = -1;

    friend class Row;
};

/// Lightweight row view for FMT::Crosses. Stores a pointer + row index only.
class FMT_Crosses::Row {
 public:
    Row(const FMT_Crosses* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int region() const noexcept { return m_bank->region(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float err_x() const noexcept { return m_bank->err_x(m_row); }
    [[nodiscard]] float err_y() const noexcept { return m_bank->err_y(m_row); }
    [[nodiscard]] float err_z() const noexcept { return m_bank->err_z(m_row); }
    [[nodiscard]] float ux() const noexcept { return m_bank->ux(m_row); }
    [[nodiscard]] float uy() const noexcept { return m_bank->uy(m_row); }
    [[nodiscard]] float uz() const noexcept { return m_bank->uz(m_row); }
    [[nodiscard]] int cluster1Index() const noexcept { return m_bank->cluster1Index(m_row); }
    [[nodiscard]] int cluster2Index() const noexcept { return m_bank->cluster2Index(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
 private:
    const FMT_Crosses* m_bank;
    int m_row;
};

inline FMT_Crosses::Row FMT_Crosses::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
