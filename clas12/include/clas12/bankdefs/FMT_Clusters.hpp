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


/// HIPO bank wrapper for `FMT::Clusters` (group 20800, item 22).
/// reconstructed FMT clusters
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FMT_Clusters bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class FMT_Clusters {
 public:
    static constexpr std::string_view bank_name = "FMT::Clusters";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 22;
    static constexpr std::string_view bank_schema = "index/S,sector/B,layer/B,size/S,centroid/F,centroidError/F,residual/F,doca/F,energy/F,time/F,seedStrip/S,seedIndex/S,trackIndex/S";

    class Row;

    FMT_Clusters() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_size = -1;
            int i_centroid = -1;
            int i_centroidError = -1;
            int i_residual = -1;
            int i_doca = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_seedStrip = -1;
            int i_seedIndex = -1;
            int i_trackIndex = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_centroid = schema.exists("centroid") ? schema.getEntryOrder("centroid") : -1;
            cache.i_centroidError = schema.exists("centroidError") ? schema.getEntryOrder("centroidError") : -1;
            cache.i_residual = schema.exists("residual") ? schema.getEntryOrder("residual") : -1;
            cache.i_doca = schema.exists("doca") ? schema.getEntryOrder("doca") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_seedStrip = schema.exists("seedStrip") ? schema.getEntryOrder("seedStrip") : -1;
            cache.i_seedIndex = schema.exists("seedIndex") ? schema.getEntryOrder("seedIndex") : -1;
            cache.i_trackIndex = schema.exists("trackIndex") ? schema.getEntryOrder("trackIndex") : -1;
        }
        i_index = cache.i_index;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_size = cache.i_size;
        i_centroid = cache.i_centroid;
        i_centroidError = cache.i_centroidError;
        i_residual = cache.i_residual;
        i_doca = cache.i_doca;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_seedStrip = cache.i_seedStrip;
        i_seedIndex = cache.i_seedIndex;
        i_trackIndex = cache.i_trackIndex;
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
    /// index: index
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
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
    /// centroid: centroid position in local coordinates
    [[nodiscard]] float centroid(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_centroid, row));
    }
    /// centroidError: centroid position error
    [[nodiscard]] float centroidError(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_centroidError, row));
    }
    /// residual: centroid residual
    [[nodiscard]] float residual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_residual, row));
    }
    /// doca: dc track doca
    [[nodiscard]] float doca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_doca, row));
    }
    /// energy: cluster total energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: cluster time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// seedStrip: seed strip number
    [[nodiscard]] int seedStrip(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_seedStrip, row));
    }
    /// seedIndex: seed index in hits bank
    [[nodiscard]] int seedIndex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_seedIndex, row));
    }
    /// trackIndex: associated track index
    [[nodiscard]] int trackIndex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trackIndex, row));
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
    int i_sector = -1;
    int i_layer = -1;
    int i_size = -1;
    int i_centroid = -1;
    int i_centroidError = -1;
    int i_residual = -1;
    int i_doca = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_seedStrip = -1;
    int i_seedIndex = -1;
    int i_trackIndex = -1;

    friend class Row;
};

/// Lightweight row view for FMT::Clusters. Stores a pointer + row index only.
class FMT_Clusters::Row {
 public:
    Row(const FMT_Clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] float centroid() const noexcept { return m_bank->centroid(m_row); }
    [[nodiscard]] float centroidError() const noexcept { return m_bank->centroidError(m_row); }
    [[nodiscard]] float residual() const noexcept { return m_bank->residual(m_row); }
    [[nodiscard]] float doca() const noexcept { return m_bank->doca(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int seedStrip() const noexcept { return m_bank->seedStrip(m_row); }
    [[nodiscard]] int seedIndex() const noexcept { return m_bank->seedIndex(m_row); }
    [[nodiscard]] int trackIndex() const noexcept { return m_bank->trackIndex(m_row); }
 private:
    const FMT_Clusters* m_bank;
    int m_row;
};

inline FMT_Clusters::Row FMT_Clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
