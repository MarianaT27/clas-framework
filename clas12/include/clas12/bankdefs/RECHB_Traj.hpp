// =============================================================================
// Auto-generated from event.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RECHB::Traj` (group 300, item 18).
/// Trajectory information for Particles bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECHB_Traj bank;
/// if (auto sc = bank.bind(event.get_bank("RECHB::Traj")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto pindex = row.pindex();
/// }
/// @endcode
class RECHB_Traj {
 public:
    static constexpr std::string_view bank_name = "RECHB::Traj";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 18;
    static constexpr std::string_view bank_schema = "pindex/S,index/S,detector/B,layer/B,x/F,y/F,z/F,cx/F,cy/F,cz/F,path/F,edge/F";

    class Row;

    RECHB_Traj() = default;

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
            int i_pindex = -1;
            int i_index = -1;
            int i_detector = -1;
            int i_layer = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_cx = -1;
            int i_cy = -1;
            int i_cz = -1;
            int i_path = -1;
            int i_edge = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_cx = schema.exists("cx") ? schema.getEntryOrder("cx") : -1;
            cache.i_cy = schema.exists("cy") ? schema.getEntryOrder("cy") : -1;
            cache.i_cz = schema.exists("cz") ? schema.getEntryOrder("cz") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
            cache.i_edge = schema.exists("edge") ? schema.getEntryOrder("edge") : -1;
        }
        i_pindex = cache.i_pindex;
        i_index = cache.i_index;
        i_detector = cache.i_detector;
        i_layer = cache.i_layer;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_cx = cache.i_cx;
        i_cy = cache.i_cy;
        i_cz = cache.i_cz;
        i_path = cache.i_path;
        i_edge = cache.i_edge;
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
    /// pindex: index of the particle in REC::Particle
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pindex, row));
    }
    /// index: index of the track in its tracking bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }
    /// detector: DetectorID as defined in org.jlab.detector.DetectorType
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// layer: Layer ID as defined in org.jlab.detector.DetectorLayer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// x: x-position of the track at the detector surface (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y-position of the track at the detector surface (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: z-position of the track at the detector surface (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// cx: direction cosline of the track at the detector surface
    [[nodiscard]] float cx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cx, row));
    }
    /// cy: direction cosline of the track at the detector surface
    [[nodiscard]] float cy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cy, row));
    }
    /// cz: direction cosline of the track at the detector surface
    [[nodiscard]] float cz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cz, row));
    }
    /// path: path length from primary vertex to the detector surface (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
    }
    /// edge: signed distance between track position and closest detector edge (cm)
    [[nodiscard]] float edge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_edge, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }
    [[nodiscard]] int index_handle() const noexcept { return i_index; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_pindex = -1;
    int i_index = -1;
    int i_detector = -1;
    int i_layer = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_cx = -1;
    int i_cy = -1;
    int i_cz = -1;
    int i_path = -1;
    int i_edge = -1;

    friend class Row;
};

/// Lightweight row view for RECHB::Traj. Stores a pointer + row index only.
class RECHB_Traj::Row {
 public:
    Row(const RECHB_Traj* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float cx() const noexcept { return m_bank->cx(m_row); }
    [[nodiscard]] float cy() const noexcept { return m_bank->cy(m_row); }
    [[nodiscard]] float cz() const noexcept { return m_bank->cz(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float edge() const noexcept { return m_bank->edge(m_row); }
 private:
    const RECHB_Traj* m_bank;
    int m_row;
};

inline RECHB_Traj::Row RECHB_Traj::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
