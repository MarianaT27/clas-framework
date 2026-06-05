// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RECHBAI::ForwardTagger` (group 400, item 14).
/// Forward Tagger information for Particles bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECHBAI_ForwardTagger bank;
/// if (auto sc = bank.bind(event.get_bank("RECHBAI::ForwardTagger")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class RECHBAI_ForwardTagger {
 public:
    static constexpr std::string_view bank_name = "RECHBAI::ForwardTagger";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 14;
    static constexpr std::string_view bank_schema = "index/S,pindex/S,detector/B,layer/B,energy/F,time/F,path/F,chi2/F,x/F,y/F,z/F,dx/F,dy/F,radius/F,size/S,status/S";

    class Row;

    RECHBAI_ForwardTagger() = default;

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
            int i_pindex = -1;
            int i_detector = -1;
            int i_layer = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_path = -1;
            int i_chi2 = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_dx = -1;
            int i_dy = -1;
            int i_radius = -1;
            int i_size = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_path = schema.exists("path") ? schema.getEntryOrder("path") : -1;
            cache.i_chi2 = schema.exists("chi2") ? schema.getEntryOrder("chi2") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_dx = schema.exists("dx") ? schema.getEntryOrder("dx") : -1;
            cache.i_dy = schema.exists("dy") ? schema.getEntryOrder("dy") : -1;
            cache.i_radius = schema.exists("radius") ? schema.getEntryOrder("radius") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_index = cache.i_index;
        i_pindex = cache.i_pindex;
        i_detector = cache.i_detector;
        i_layer = cache.i_layer;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_path = cache.i_path;
        i_chi2 = cache.i_chi2;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_dx = cache.i_dx;
        i_dy = cache.i_dy;
        i_radius = cache.i_radius;
        i_size = cache.i_size;
        i_status = cache.i_status;
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
    /// index: index of the cluster in the specific detector bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }
    /// pindex: row number in the particle bank hit is associated with
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pindex, row));
    }
    /// detector: Detector ID, as defined in org.jlab.detector.base.DetectorType
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// energy: Energy associated with the cluster (GeV)
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Time associated with the cluster (cm)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// path: Path from vertex to the cluster position (cm)
    [[nodiscard]] float path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_path, row));
    }
    /// chi2: Chi2 (or quality) of cluster-particle matching
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2, row));
    }
    /// x: X coordinate of the cluster
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Y coordinate of the cluster
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Z coordinate of the cluster
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// dx: Cluster width in x
    [[nodiscard]] float dx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dx, row));
    }
    /// dy: Cluster width in y
    [[nodiscard]] float dy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dy, row));
    }
    /// radius: Cluster radius
    [[nodiscard]] float radius(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_radius, row));
    }
    /// size: size of the cluster
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// status: hit status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int index_handle() const noexcept { return i_index; }
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_index = -1;
    int i_pindex = -1;
    int i_detector = -1;
    int i_layer = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_path = -1;
    int i_chi2 = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_dx = -1;
    int i_dy = -1;
    int i_radius = -1;
    int i_size = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for RECHBAI::ForwardTagger. Stores a pointer + row index only.
class RECHBAI_ForwardTagger::Row {
 public:
    Row(const RECHBAI_ForwardTagger* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float path() const noexcept { return m_bank->path(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float dx() const noexcept { return m_bank->dx(m_row); }
    [[nodiscard]] float dy() const noexcept { return m_bank->dy(m_row); }
    [[nodiscard]] float radius() const noexcept { return m_bank->radius(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const RECHBAI_ForwardTagger* m_bank;
    int m_row;
};

inline RECHBAI_ForwardTagger::Row RECHBAI_ForwardTagger::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
