// =============================================================================
// Auto-generated from cvt.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `CVT::KFTrajectory` (group 20500, item 42).
/// Trajectory bank output from KF
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CVT_KFTrajectory bank;
/// if (auto sc = bank.bind(event.get_bank("CVT::KFTrajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CVT_KFTrajectory {
 public:
    static constexpr std::string_view bank_name = "CVT::KFTrajectory";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 42;

    class Row;

    CVT_KFTrajectory() = default;

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
            int i_layer = -1;
            int i_index = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_path = -1;
            int i_dx = -1;
            int i_dE = -1;
            int i_trackRes = -1;
            int i_transportedRes = -1;
            int i_filteredRes = -1;
            int i_smoothedRes = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
            cache.i_dx = schema.exists("dx") ? schema.getEntryOrder("dx") : -1;
            cache.i_dE = schema.exists("dE") ? schema.getEntryOrder("dE") : -1;
            cache.i_trackRes = schema.exists("trackRes") ? schema.getEntryOrder("trackRes") : -1;
            cache.i_transportedRes = schema.exists("transportedRes") ? schema.getEntryOrder("transportedRes") : -1;
            cache.i_filteredRes = schema.exists("filteredRes") ? schema.getEntryOrder("filteredRes") : -1;
            cache.i_smoothedRes = schema.exists("smoothedRes") ? schema.getEntryOrder("smoothedRes") : -1;
        }
        i_id = cache.i_id;
        i_detector = cache.i_detector;
        i_layer = cache.i_layer;
        i_index = cache.i_index;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_path = cache.i_path;
        i_dx = cache.i_dx;
        i_dE = cache.i_dE;
        i_trackRes = cache.i_trackRes;
        i_transportedRes = cache.i_transportedRes;
        i_filteredRes = cache.i_filteredRes;
        i_smoothedRes = cache.i_smoothedRes;
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
    /// detector: detector type id
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// layer: id of the detector layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// index: id of the measureents surface
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_index, row));
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
    /// px: track x momentum at detector surface (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: track y momentum at detector surface (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: track z momentum at detector surface (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// path: track path length from previous point
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
    }
    /// dx: track path length through the surface material
    [[nodiscard]] float dx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dx, row));
    }
    /// dE: track energy loss
    [[nodiscard]] float dE(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dE, row));
    }
    /// trackRes: track residual (-999 for passive layer)
    [[nodiscard]] float trackRes(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trackRes, row));
    }
    /// transportedRes: filtered residual (-999 for passive layer)
    [[nodiscard]] float transportedRes(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_transportedRes, row));
    }
    /// filteredRes: filtered residual (-999 for passive layer)
    [[nodiscard]] float filteredRes(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_filteredRes, row));
    }
    /// smoothedRes: smoothed residual (-999 for passive layer)
    [[nodiscard]] float smoothedRes(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_smoothedRes, row));
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
    int i_id = -1;
    int i_detector = -1;
    int i_layer = -1;
    int i_index = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_path = -1;
    int i_dx = -1;
    int i_dE = -1;
    int i_trackRes = -1;
    int i_transportedRes = -1;
    int i_filteredRes = -1;
    int i_smoothedRes = -1;

    friend class Row;
};

/// Lightweight row view for CVT::KFTrajectory. Stores a pointer + row index only.
class CVT_KFTrajectory::Row {
 public:
    Row(const CVT_KFTrajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float dE() const noexcept { return m_bank->dE(m_row); }
    [[nodiscard]] float trackRes() const noexcept { return m_bank->trackRes(m_row); }
    [[nodiscard]] float transportedRes() const noexcept { return m_bank->transportedRes(m_row); }
    [[nodiscard]] float filteredRes() const noexcept { return m_bank->filteredRes(m_row); }
    [[nodiscard]] float smoothedRes() const noexcept { return m_bank->smoothedRes(m_row); }
 private:
    const CVT_KFTrajectory* m_bank;
    int m_row;
};

inline CVT_KFTrajectory::Row CVT_KFTrajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
