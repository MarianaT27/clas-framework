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


/// HIPO bank wrapper for `FTTRK::clusters` (group 21300, item 22).
/// Reconstructed clusters in FT tracker
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTTRK_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("FTTRK::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto size = row.size();
/// }
/// @endcode
class FTTRK_clusters {
 public:
    static constexpr std::string_view bank_name = "FTTRK::clusters";
    static constexpr int bank_group = 21300;
    static constexpr int bank_item  = 22;

    class Row;

    FTTRK_clusters() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_energy = -1;
            int i_maxEnergy = -1;
            int i_seed = -1;
            int i_centroid = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_maxEnergy = schema.exists("maxEnergy") ? schema.getEntryOrder("maxEnergy") : -1;
            cache.i_seed = schema.exists("seed") ? schema.getEntryOrder("seed") : -1;
            cache.i_centroid = schema.exists("centroid") ? schema.getEntryOrder("centroid") : -1;
        }
        i_size = cache.i_size;
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_energy = cache.i_energy;
        i_maxEnergy = cache.i_maxEnergy;
        i_seed = cache.i_seed;
        i_centroid = cache.i_centroid;
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
    /// sector: Cluster sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: Cluster layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// energy: Cluster total energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// maxEnergy: Seed energy
    [[nodiscard]] float maxEnergy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_maxEnergy, row));
    }
    /// seed: Seed strip
    [[nodiscard]] int seed(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_seed, row));
    }
    /// centroid: Centroid strip number
    [[nodiscard]] float centroid(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_centroid, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_size = -1;
    int i_id = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_energy = -1;
    int i_maxEnergy = -1;
    int i_seed = -1;
    int i_centroid = -1;

    friend class Row;
};

/// Lightweight row view for FTTRK::clusters. Stores a pointer + row index only.
class FTTRK_clusters::Row {
 public:
    Row(const FTTRK_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float maxEnergy() const noexcept { return m_bank->maxEnergy(m_row); }
    [[nodiscard]] int seed() const noexcept { return m_bank->seed(m_row); }
    [[nodiscard]] float centroid() const noexcept { return m_bank->centroid(m_row); }
 private:
    const FTTRK_clusters* m_bank;
    int m_row;
};

inline FTTRK_clusters::Row FTTRK_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
