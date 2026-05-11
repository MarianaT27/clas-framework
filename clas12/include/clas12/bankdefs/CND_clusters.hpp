// =============================================================================
// Auto-generated from cnd.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `CND::clusters` (group 20300, item 22).
/// reconstructed cluster info from CND and CTOF
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CND_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("CND::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CND_clusters {
 public:
    static constexpr std::string_view bank_name = "CND::clusters";
    static constexpr int bank_group = 20300;
    static constexpr int bank_item  = 22;

    class Row;

    CND_clusters() = default;

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
            int i_id = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_size = -1;
            int i_energy = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_time = -1;
            int i_status = -1;
            int i_layermult = -1;
            int i_layer1 = -1;
            int i_layer2 = -1;
            int i_layer3 = -1;
            int i_pathLengthThruBar = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_layermult = schema.exists("layermult") ? schema.getEntryOrder("layermult") : -1;
            cache.i_layer1 = schema.exists("layer1") ? schema.getEntryOrder("layer1") : -1;
            cache.i_layer2 = schema.exists("layer2") ? schema.getEntryOrder("layer2") : -1;
            cache.i_layer3 = schema.exists("layer3") ? schema.getEntryOrder("layer3") : -1;
            cache.i_pathLengthThruBar = schema.exists("pathLengthThruBar") ? schema.getEntryOrder("pathLengthThruBar") : -1;
        }
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_size = cache.i_size;
        i_energy = cache.i_energy;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_time = cache.i_time;
        i_status = cache.i_status;
        i_layermult = cache.i_layermult;
        i_layer1 = cache.i_layer1;
        i_layer2 = cache.i_layer2;
        i_layer3 = cache.i_layer3;
        i_pathLengthThruBar = cache.i_pathLengthThruBar;
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
    /// id: id of the cluster
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// sector: sector of the cluster's seed hit
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer of the cluster's seed hit
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: component of the cluster's seed hit
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// size: number of hits of the cluster
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// energy: Sum of E dep (MeV) of the hits
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// x: center of X coor (cm) of the hits
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: center of Y coor (cm) of the hits
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: center of Z coor (cm) of the hits
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// time: center of time (ns) of the hits
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// status: cluster status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// layermult: layer multiplicity
    [[nodiscard]] int layermult(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layermult, row));
    }
    /// layer1: firt layer hit
    [[nodiscard]] int layer1(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer1, row));
    }
    /// layer2: second layer hit
    [[nodiscard]] int layer2(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer2, row));
    }
    /// layer3: third layer hit
    [[nodiscard]] int layer3(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer3, row));
    }
    /// pathLengthThruBar: pathlength of the track from the entrance point to the exit point through the cluster bars 
    [[nodiscard]] float pathLengthThruBar(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pathLengthThruBar, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_size = -1;
    int i_energy = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_time = -1;
    int i_status = -1;
    int i_layermult = -1;
    int i_layer1 = -1;
    int i_layer2 = -1;
    int i_layer3 = -1;
    int i_pathLengthThruBar = -1;

    friend class Row;
};

/// Lightweight row view for CND::clusters. Stores a pointer + row index only.
class CND_clusters::Row {
 public:
    Row(const CND_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int layermult() const noexcept { return m_bank->layermult(m_row); }
    [[nodiscard]] int layer1() const noexcept { return m_bank->layer1(m_row); }
    [[nodiscard]] int layer2() const noexcept { return m_bank->layer2(m_row); }
    [[nodiscard]] int layer3() const noexcept { return m_bank->layer3(m_row); }
    [[nodiscard]] float pathLengthThruBar() const noexcept { return m_bank->pathLengthThruBar(m_row); }
 private:
    const CND_clusters* m_bank;
    int m_row;
};

inline CND_clusters::Row CND_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
