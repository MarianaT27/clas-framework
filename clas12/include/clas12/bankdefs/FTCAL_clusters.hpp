// =============================================================================
// Auto-generated from ft.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `FTCAL::clusters` (group 21000, item 22).
/// Reconstructed Clusters in FT calorimeter
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTCAL_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("FTCAL::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto size = row.size();
/// }
/// @endcode
class FTCAL_clusters {
 public:
    static constexpr std::string_view bank_name = "FTCAL::clusters";
    static constexpr int bank_group = 21000;
    static constexpr int bank_item  = 22;

    class Row;

    FTCAL_clusters() = default;

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
            int i_size = -1;
            int i_id = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_widthX = -1;
            int i_widthY = -1;
            int i_radius = -1;
            int i_time = -1;
            int i_energy = -1;
            int i_maxEnergy = -1;
            int i_recEnergy = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_widthX = schema.exists("widthX") ? schema.getEntryOrder("widthX") : -1;
            cache.i_widthY = schema.exists("widthY") ? schema.getEntryOrder("widthY") : -1;
            cache.i_radius = schema.exists("radius") ? schema.getEntryOrder("radius") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_maxEnergy = schema.exists("maxEnergy") ? schema.getEntryOrder("maxEnergy") : -1;
            cache.i_recEnergy = schema.exists("recEnergy") ? schema.getEntryOrder("recEnergy") : -1;
        }
        i_size = cache.i_size;
        i_id = cache.i_id;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_widthX = cache.i_widthX;
        i_widthY = cache.i_widthY;
        i_radius = cache.i_radius;
        i_time = cache.i_time;
        i_energy = cache.i_energy;
        i_maxEnergy = cache.i_maxEnergy;
        i_recEnergy = cache.i_recEnergy;
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
    /// size: Cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// id: Cluster ID
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// x: Cluster centroid X moment (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Cluster centroid Y moment (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Cluster centroid Z moment (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// widthX: Cluster width in x (cm)
    [[nodiscard]] float widthX(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_widthX, row));
    }
    /// widthY: Cluster width in y (cm)
    [[nodiscard]] float widthY(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_widthY, row));
    }
    /// radius: Cluster radius (cm)
    [[nodiscard]] float radius(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_radius, row));
    }
    /// time: Cluster timing information
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// energy: Cluster total energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// maxEnergy: Cluster maximum energy
    [[nodiscard]] float maxEnergy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_maxEnergy, row));
    }
    /// recEnergy: Cluster reconstructed energy
    [[nodiscard]] float recEnergy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recEnergy, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_size = -1;
    int i_id = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_widthX = -1;
    int i_widthY = -1;
    int i_radius = -1;
    int i_time = -1;
    int i_energy = -1;
    int i_maxEnergy = -1;
    int i_recEnergy = -1;

    friend class Row;
};

/// Lightweight row view for FTCAL::clusters. Stores a pointer + row index only.
class FTCAL_clusters::Row {
 public:
    Row(const FTCAL_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float widthX() const noexcept { return m_bank->widthX(m_row); }
    [[nodiscard]] float widthY() const noexcept { return m_bank->widthY(m_row); }
    [[nodiscard]] float radius() const noexcept { return m_bank->radius(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float maxEnergy() const noexcept { return m_bank->maxEnergy(m_row); }
    [[nodiscard]] float recEnergy() const noexcept { return m_bank->recEnergy(m_row); }
 private:
    const FTCAL_clusters* m_bank;
    int m_row;
};

inline FTCAL_clusters::Row FTCAL_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
