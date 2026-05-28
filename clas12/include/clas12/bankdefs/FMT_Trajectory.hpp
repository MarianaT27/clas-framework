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


/// HIPO bank wrapper for `FMT::Trajectory` (group 20800, item 37).
/// FMT tracks trajectory bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FMT_Trajectory bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Trajectory")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class FMT_Trajectory {
 public:
    static constexpr std::string_view bank_name = "FMT::Trajectory";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 37;

    class Row;

    FMT_Trajectory() = default;

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
            int i_detector = -1;
            int i_layer = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_tx = -1;
            int i_ty = -1;
            int i_tz = -1;
            int i_lx = -1;
            int i_ly = -1;
            int i_lz = -1;
            int i_dx = -1;
            int i_dy = -1;
            int i_dz = -1;
            int i_path = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_tx = schema.exists("tx") ? schema.getEntryOrder("tx") : -1;
            cache.i_ty = schema.exists("ty") ? schema.getEntryOrder("ty") : -1;
            cache.i_tz = schema.exists("tz") ? schema.getEntryOrder("tz") : -1;
            cache.i_lx = schema.exists("lx") ? schema.getEntryOrder("lx") : -1;
            cache.i_ly = schema.exists("ly") ? schema.getEntryOrder("ly") : -1;
            cache.i_lz = schema.exists("lz") ? schema.getEntryOrder("lz") : -1;
            cache.i_dx = schema.exists("dx") ? schema.getEntryOrder("dx") : -1;
            cache.i_dy = schema.exists("dy") ? schema.getEntryOrder("dy") : -1;
            cache.i_dz = schema.exists("dz") ? schema.getEntryOrder("dz") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
        }
        i_index = cache.i_index;
        i_detector = cache.i_detector;
        i_layer = cache.i_layer;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_tx = cache.i_tx;
        i_ty = cache.i_ty;
        i_tz = cache.i_tz;
        i_lx = cache.i_lx;
        i_ly = cache.i_ly;
        i_lz = cache.i_lz;
        i_dx = cache.i_dx;
        i_dy = cache.i_dy;
        i_dz = cache.i_dz;
        i_path = cache.i_path;
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
    /// detector: id of the detector
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
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
    /// lx: track x position in local coordinates (cm)
    [[nodiscard]] float lx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lx, row));
    }
    /// ly: track y position in local coordinates (cm)
    [[nodiscard]] float ly(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ly, row));
    }
    /// lz: track z position in local coordinates (cm)
    [[nodiscard]] float lz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lz, row));
    }
    /// dx: DC track x position in local coordinates (cm)
    [[nodiscard]] float dx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dx, row));
    }
    /// dy: DC track y position in local coordinates (cm)
    [[nodiscard]] float dy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dy, row));
    }
    /// dz: DC track z position in local coordinates (cm)
    [[nodiscard]] float dz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dz, row));
    }
    /// path: pathlength of the track from the track vertex to the detector surface (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
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
    int i_detector = -1;
    int i_layer = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_tx = -1;
    int i_ty = -1;
    int i_tz = -1;
    int i_lx = -1;
    int i_ly = -1;
    int i_lz = -1;
    int i_dx = -1;
    int i_dy = -1;
    int i_dz = -1;
    int i_path = -1;

    friend class Row;
};

/// Lightweight row view for FMT::Trajectory. Stores a pointer + row index only.
class FMT_Trajectory::Row {
 public:
    Row(const FMT_Trajectory* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] float lx() const noexcept { return m_bank->lx(m_row); }
    [[nodiscard]] float ly() const noexcept { return m_bank->ly(m_row); }
    [[nodiscard]] float lz() const noexcept { return m_bank->lz(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float dy() const noexcept { return m_bank->dy(m_row); }
    [[nodiscard]] float dz() const noexcept { return m_bank->dz(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
 private:
    const FMT_Trajectory* m_bank;
    int m_row;
};

inline FMT_Trajectory::Row FMT_Trajectory::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
