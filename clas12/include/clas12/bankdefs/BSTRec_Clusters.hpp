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


/// HIPO bank wrapper for `BSTRec::Clusters` (group 20200, item 22).
/// first pass reconstructed BST clusters
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// BSTRec_Clusters bank;
/// if (auto sc = bank.bind(event.get_bank("BSTRec::Clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class BSTRec_Clusters {
 public:
    static constexpr std::string_view bank_name = "BSTRec::Clusters";
    static constexpr int bank_group = 20200;
    static constexpr int bank_item  = 22;
    static constexpr std::string_view bank_schema = "ID/S,sector/B,layer/B,size/S,ETot/F,time/F,seedE/F,seedStrip/S,centroid/F,centroidError/F,centroidResidual/F,seedResidual/F,Hit1_ID/S,Hit2_ID/S,Hit3_ID/S,Hit4_ID/S,Hit5_ID/S,trkID/S,x1/F,y1/F,z1/F,x2/F,y2/F,z2/F,cx/F,cy/F,cz/F,lx/F,ly/F,lz/F,sx/F,sy/F,sz/F,nx/F,ny/F,nz/F,e/F";

    class Row;

    BSTRec_Clusters() = default;

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
            int i_layer = -1;
            int i_size = -1;
            int i_ETot = -1;
            int i_time = -1;
            int i_seedE = -1;
            int i_seedStrip = -1;
            int i_centroid = -1;
            int i_centroidError = -1;
            int i_centroidResidual = -1;
            int i_seedResidual = -1;
            int i_Hit1_ID = -1;
            int i_Hit2_ID = -1;
            int i_Hit3_ID = -1;
            int i_Hit4_ID = -1;
            int i_Hit5_ID = -1;
            int i_trkID = -1;
            int i_x1 = -1;
            int i_y1 = -1;
            int i_z1 = -1;
            int i_x2 = -1;
            int i_y2 = -1;
            int i_z2 = -1;
            int i_cx = -1;
            int i_cy = -1;
            int i_cz = -1;
            int i_lx = -1;
            int i_ly = -1;
            int i_lz = -1;
            int i_sx = -1;
            int i_sy = -1;
            int i_sz = -1;
            int i_nx = -1;
            int i_ny = -1;
            int i_nz = -1;
            int i_e = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_ID = schema.exists("ID") ? schema.getEntryOrder("ID") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_ETot = schema.exists("ETot") ? schema.getEntryOrder("ETot") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_seedE = schema.exists("seedE") ? schema.getEntryOrder("seedE") : -1;
            cache.i_seedStrip = schema.exists("seedStrip") ? schema.getEntryOrder("seedStrip") : -1;
            cache.i_centroid = schema.exists("centroid") ? schema.getEntryOrder("centroid") : -1;
            cache.i_centroidError = schema.exists("centroidError") ? schema.getEntryOrder("centroidError") : -1;
            cache.i_centroidResidual = schema.exists("centroidResidual") ? schema.getEntryOrder("centroidResidual") : -1;
            cache.i_seedResidual = schema.exists("seedResidual") ? schema.getEntryOrder("seedResidual") : -1;
            cache.i_Hit1_ID = schema.exists("Hit1_ID") ? schema.getEntryOrder("Hit1_ID") : -1;
            cache.i_Hit2_ID = schema.exists("Hit2_ID") ? schema.getEntryOrder("Hit2_ID") : -1;
            cache.i_Hit3_ID = schema.exists("Hit3_ID") ? schema.getEntryOrder("Hit3_ID") : -1;
            cache.i_Hit4_ID = schema.exists("Hit4_ID") ? schema.getEntryOrder("Hit4_ID") : -1;
            cache.i_Hit5_ID = schema.exists("Hit5_ID") ? schema.getEntryOrder("Hit5_ID") : -1;
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
            cache.i_x1 = schema.exists("x1") ? schema.getEntryOrder("x1") : -1;
            cache.i_y1 = schema.exists("y1") ? schema.getEntryOrder("y1") : -1;
            cache.i_z1 = schema.exists("z1") ? schema.getEntryOrder("z1") : -1;
            cache.i_x2 = schema.exists("x2") ? schema.getEntryOrder("x2") : -1;
            cache.i_y2 = schema.exists("y2") ? schema.getEntryOrder("y2") : -1;
            cache.i_z2 = schema.exists("z2") ? schema.getEntryOrder("z2") : -1;
            cache.i_cx = schema.exists("cx") ? schema.getEntryOrder("cx") : -1;
            cache.i_cy = schema.exists("cy") ? schema.getEntryOrder("cy") : -1;
            cache.i_cz = schema.exists("cz") ? schema.getEntryOrder("cz") : -1;
            cache.i_lx = schema.exists("lx") ? schema.getEntryOrder("lx") : -1;
            cache.i_ly = schema.exists("ly") ? schema.getEntryOrder("ly") : -1;
            cache.i_lz = schema.exists("lz") ? schema.getEntryOrder("lz") : -1;
            cache.i_sx = schema.exists("sx") ? schema.getEntryOrder("sx") : -1;
            cache.i_sy = schema.exists("sy") ? schema.getEntryOrder("sy") : -1;
            cache.i_sz = schema.exists("sz") ? schema.getEntryOrder("sz") : -1;
            cache.i_nx = schema.exists("nx") ? schema.getEntryOrder("nx") : -1;
            cache.i_ny = schema.exists("ny") ? schema.getEntryOrder("ny") : -1;
            cache.i_nz = schema.exists("nz") ? schema.getEntryOrder("nz") : -1;
            cache.i_e = schema.exists("e") ? schema.getEntryOrder("e") : -1;
        }
        i_ID = cache.i_ID;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_size = cache.i_size;
        i_ETot = cache.i_ETot;
        i_time = cache.i_time;
        i_seedE = cache.i_seedE;
        i_seedStrip = cache.i_seedStrip;
        i_centroid = cache.i_centroid;
        i_centroidError = cache.i_centroidError;
        i_centroidResidual = cache.i_centroidResidual;
        i_seedResidual = cache.i_seedResidual;
        i_Hit1_ID = cache.i_Hit1_ID;
        i_Hit2_ID = cache.i_Hit2_ID;
        i_Hit3_ID = cache.i_Hit3_ID;
        i_Hit4_ID = cache.i_Hit4_ID;
        i_Hit5_ID = cache.i_Hit5_ID;
        i_trkID = cache.i_trkID;
        i_x1 = cache.i_x1;
        i_y1 = cache.i_y1;
        i_z1 = cache.i_z1;
        i_x2 = cache.i_x2;
        i_y2 = cache.i_y2;
        i_z2 = cache.i_z2;
        i_cx = cache.i_cx;
        i_cy = cache.i_cy;
        i_cz = cache.i_cz;
        i_lx = cache.i_lx;
        i_ly = cache.i_ly;
        i_lz = cache.i_lz;
        i_sx = cache.i_sx;
        i_sy = cache.i_sy;
        i_sz = cache.i_sz;
        i_nx = cache.i_nx;
        i_ny = cache.i_ny;
        i_nz = cache.i_nz;
        i_e = cache.i_e;
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
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// size: cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// ETot: cluster total energy
    [[nodiscard]] float ETot(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ETot, row));
    }
    /// time: cluster time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// seedE: energy of the seed 
    [[nodiscard]] float seedE(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_seedE, row));
    }
    /// seedStrip: seed strip
    [[nodiscard]] int seedStrip(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_seedStrip, row));
    }
    /// centroid: centroid strip number
    [[nodiscard]] float centroid(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_centroid, row));
    }
    /// centroidError: centroid spatial error
    [[nodiscard]] float centroidError(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_centroidError, row));
    }
    /// centroidResidual: centroid residual
    [[nodiscard]] float centroidResidual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_centroidResidual, row));
    }
    /// seedResidual: seed residual
    [[nodiscard]] float seedResidual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_seedResidual, row));
    }
    /// Hit1_ID: Index of hit 1 in cluster
    [[nodiscard]] int Hit1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit1_ID, row));
    }
    /// Hit2_ID: Index of hit 2 in cluster
    [[nodiscard]] int Hit2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit2_ID, row));
    }
    /// Hit3_ID: Index of hit 3 in cluster
    [[nodiscard]] int Hit3_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit3_ID, row));
    }
    /// Hit4_ID: Index of hit 4 in cluster
    [[nodiscard]] int Hit4_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit4_ID, row));
    }
    /// Hit5_ID: Index of hit 5 in cluster
    [[nodiscard]] int Hit5_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Hit5_ID, row));
    }
    /// trkID: associated track ID
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trkID, row));
    }
    /// x1: cluster first end point x
    [[nodiscard]] float x1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x1, row));
    }
    /// y1: cluster first end point y
    [[nodiscard]] float y1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y1, row));
    }
    /// z1: cluster first end point z
    [[nodiscard]] float z1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z1, row));
    }
    /// x2: cluster second end point x
    [[nodiscard]] float x2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x2, row));
    }
    /// y2: cluster second end point y
    [[nodiscard]] float y2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y2, row));
    }
    /// z2: cluster second end point z
    [[nodiscard]] float z2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z2, row));
    }
    /// cx: bmt-C cluster arc center x
    [[nodiscard]] float cx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cx, row));
    }
    /// cy: bmt-C cluster arc center y
    [[nodiscard]] float cy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cy, row));
    }
    /// cz: bmt-C cluster arc center z
    [[nodiscard]] float cz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cz, row));
    }
    /// lx: vector x comp  along cluster pseudo-strip direction
    [[nodiscard]] float lx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lx, row));
    }
    /// ly: vector y comp  along cluster pseudo-strip direction
    [[nodiscard]] float ly(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ly, row));
    }
    /// lz: vector z comp  along cluster pseudo-strip direction
    [[nodiscard]] float lz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lz, row));
    }
    /// sx: vector x comp perpendicular to cluster pseudo-strip direction in the module plane
    [[nodiscard]] float sx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_sx, row));
    }
    /// sy: vector y comp perpendicular to cluster pseudo-strip direction in the module plane
    [[nodiscard]] float sy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_sy, row));
    }
    /// sz: vector z comp perpendicular to cluster pseudo-strip direction in the module plane
    [[nodiscard]] float sz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_sz, row));
    }
    /// nx: vector x comp normal to the cluster module plane
    [[nodiscard]] float nx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_nx, row));
    }
    /// ny: vector y comp normal to the cluster module plane
    [[nodiscard]] float ny(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ny, row));
    }
    /// nz: vector z comp normal to the cluster module plane
    [[nodiscard]] float nz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_nz, row));
    }
    /// e: cluster spacial uncertainty
    [[nodiscard]] float e(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_e, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_ID = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_size = -1;
    int i_ETot = -1;
    int i_time = -1;
    int i_seedE = -1;
    int i_seedStrip = -1;
    int i_centroid = -1;
    int i_centroidError = -1;
    int i_centroidResidual = -1;
    int i_seedResidual = -1;
    int i_Hit1_ID = -1;
    int i_Hit2_ID = -1;
    int i_Hit3_ID = -1;
    int i_Hit4_ID = -1;
    int i_Hit5_ID = -1;
    int i_trkID = -1;
    int i_x1 = -1;
    int i_y1 = -1;
    int i_z1 = -1;
    int i_x2 = -1;
    int i_y2 = -1;
    int i_z2 = -1;
    int i_cx = -1;
    int i_cy = -1;
    int i_cz = -1;
    int i_lx = -1;
    int i_ly = -1;
    int i_lz = -1;
    int i_sx = -1;
    int i_sy = -1;
    int i_sz = -1;
    int i_nx = -1;
    int i_ny = -1;
    int i_nz = -1;
    int i_e = -1;

    friend class Row;
};

/// Lightweight row view for BSTRec::Clusters. Stores a pointer + row index only.
class BSTRec_Clusters::Row {
 public:
    Row(const BSTRec_Clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] float ETot() const noexcept { return m_bank->ETot(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float seedE() const noexcept { return m_bank->seedE(m_row); }
    [[nodiscard]] int seedStrip() const noexcept { return m_bank->seedStrip(m_row); }
    [[nodiscard]] float centroid() const noexcept { return m_bank->centroid(m_row); }
    [[nodiscard]] float centroidError() const noexcept { return m_bank->centroidError(m_row); }
    [[nodiscard]] float centroidResidual() const noexcept { return m_bank->centroidResidual(m_row); }
    [[nodiscard]] float seedResidual() const noexcept { return m_bank->seedResidual(m_row); }
    [[nodiscard]] int Hit1_ID() const noexcept { return m_bank->Hit1_ID(m_row); }
    [[nodiscard]] int Hit2_ID() const noexcept { return m_bank->Hit2_ID(m_row); }
    [[nodiscard]] int Hit3_ID() const noexcept { return m_bank->Hit3_ID(m_row); }
    [[nodiscard]] int Hit4_ID() const noexcept { return m_bank->Hit4_ID(m_row); }
    [[nodiscard]] int Hit5_ID() const noexcept { return m_bank->Hit5_ID(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] float x1() const noexcept { return m_bank->x1(m_row); }
    [[nodiscard]] float y1() const noexcept { return m_bank->y1(m_row); }
    [[nodiscard]] float z1() const noexcept { return m_bank->z1(m_row); }
    [[nodiscard]] float x2() const noexcept { return m_bank->x2(m_row); }
    [[nodiscard]] float y2() const noexcept { return m_bank->y2(m_row); }
    [[nodiscard]] float z2() const noexcept { return m_bank->z2(m_row); }
    [[nodiscard]] float cx() const noexcept { return m_bank->cx(m_row); }
    [[nodiscard]] float cy() const noexcept { return m_bank->cy(m_row); }
    [[nodiscard]] float cz() const noexcept { return m_bank->cz(m_row); }
    [[nodiscard]] float lx() const noexcept { return m_bank->lx(m_row); }
    [[nodiscard]] float ly() const noexcept { return m_bank->ly(m_row); }
    [[nodiscard]] float lz() const noexcept { return m_bank->lz(m_row); }
    [[nodiscard]] float sx() const noexcept { return m_bank->sx(m_row); }
    [[nodiscard]] float sy() const noexcept { return m_bank->sy(m_row); }
    [[nodiscard]] float sz() const noexcept { return m_bank->sz(m_row); }
    [[nodiscard]] float nx() const noexcept { return m_bank->nx(m_row); }
    [[nodiscard]] float ny() const noexcept { return m_bank->ny(m_row); }
    [[nodiscard]] float nz() const noexcept { return m_bank->nz(m_row); }
    [[nodiscard]] float e() const noexcept { return m_bank->e(m_row); }
 private:
    const BSTRec_Clusters* m_bank;
    int m_row;
};

inline BSTRec_Clusters::Row BSTRec_Clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
