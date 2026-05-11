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


/// HIPO bank wrapper for `FTHODO::clusters` (group 21100, item 22).
/// Reconstructed clusters in FT hodoscope
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTHODO_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("FTHODO::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto size = row.size();
/// }
/// @endcode
class FTHODO_clusters {
 public:
    static constexpr std::string_view bank_name = "FTHODO::clusters";
    static constexpr int bank_group = 21100;
    static constexpr int bank_item  = 22;

    class Row;

    FTHODO_clusters() = default;

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
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
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

    friend class Row;
};

/// Lightweight row view for FTHODO::clusters. Stores a pointer + row index only.
class FTHODO_clusters::Row {
 public:
    Row(const FTHODO_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
 private:
    const FTHODO_clusters* m_bank;
    int m_row;
};

inline FTHODO_clusters::Row FTHODO_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
