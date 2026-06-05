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


/// HIPO bank wrapper for `TimeBasedTrkg::Trajectory` (group 20600, item 53).
/// Trajectory bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// TimeBasedTrkg_Trajectory bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::Trajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_Trajectory {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::Trajectory";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 53;
    static constexpr std::string_view bank_schema = "id/S,detector/B,sector/B,layer/B,x/F,y/F,z/F,tx/F,ty/F,tz/F,B/F,path/F,dx/F,edge/F";

    class Row;

    TimeBasedTrkg_Trajectory() = default;

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
            int i_detector = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_tx = -1;
            int i_ty = -1;
            int i_tz = -1;
            int i_B = -1;
            int i_path = -1;
            int i_dx = -1;
            int i_edge = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_tx = schema.exists("tx") ? schema.getEntryOrder("tx") : -1;
            cache.i_ty = schema.exists("ty") ? schema.getEntryOrder("ty") : -1;
            cache.i_tz = schema.exists("tz") ? schema.getEntryOrder("tz") : -1;
            cache.i_B = schema.exists("B") ? schema.getEntryOrder("B") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
            cache.i_dx = schema.exists("dx") ? schema.getEntryOrder("dx") : -1;
            cache.i_edge = schema.exists("edge") ? schema.getEntryOrder("edge") : -1;
        }
        i_id = cache.i_id;
        i_detector = cache.i_detector;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_tx = cache.i_tx;
        i_ty = cache.i_ty;
        i_tz = cache.i_tz;
        i_B = cache.i_B;
        i_path = cache.i_path;
        i_dx = cache.i_dx;
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
    /// id: id of the track
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// detector: id of the detector
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// sector: id of the sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: id of the layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// x: track x position at detector surface (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: track y position at detector surface (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: track z position at detector surface (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// tx: track unit direction vector x component at detector surface
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tx, row));
    }
    /// ty: track unit direction vector y component at detector surface
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ty, row));
    }
    /// tz: track unit direction vector z component at detector surface
    [[nodiscard]] float tz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tz, row));
    }
    /// B: B-field magnitude at detector surface (cm)
    [[nodiscard]] float B(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_B, row));
    }
    /// path: pathlength of the track from the track vertex to the detector surface (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
    }
    /// dx: pathlength of the track through the detector element (cm)
    [[nodiscard]] float dx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dx, row));
    }
    /// edge: distance to the closest detector edge (cm), -1 if edge is not defined, 0 if outside, >0 if inside
    [[nodiscard]] float edge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_edge, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_detector = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_tx = -1;
    int i_ty = -1;
    int i_tz = -1;
    int i_B = -1;
    int i_path = -1;
    int i_dx = -1;
    int i_edge = -1;

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::Trajectory. Stores a pointer + row index only.
class TimeBasedTrkg_Trajectory::Row {
 public:
    Row(const TimeBasedTrkg_Trajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] float B() const noexcept { return m_bank->B(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float edge() const noexcept { return m_bank->edge(m_row); }
 private:
    const TimeBasedTrkg_Trajectory* m_bank;
    int m_row;
};

inline TimeBasedTrkg_Trajectory::Row TimeBasedTrkg_Trajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
