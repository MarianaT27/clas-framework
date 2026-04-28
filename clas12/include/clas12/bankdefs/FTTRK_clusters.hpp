// =============================================================================
// Auto-generated from ft.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include <bank_view.h>
#include <schema.h>

namespace bankdefs {


/// HIPO bank wrapper for `FTTRK::clusters` (group 21300, item 22).
/// Reconstructed clusters in FT tracker
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    /// Bind to a live bank_view. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column handle by name; all
    /// subsequent bind()s for the same schema hit the cache and copy 25-30
    /// trivially-copyable column_handle words.
    /// Returns false if the view is not valid.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        struct HandleCache {
            const hipo::schema* schema_ptr = nullptr;
            hipo::column_handle h_size;
            hipo::column_handle h_id;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_energy;
            hipo::column_handle h_maxEnergy;
            hipo::column_handle h_seed;
            hipo::column_handle h_centroid;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("size"); h) cache.h_size = *h;
            else cache.h_size = {};
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("maxEnergy"); h) cache.h_maxEnergy = *h;
            else cache.h_maxEnergy = {};
            if (auto h = schema.get_handle("seed"); h) cache.h_seed = *h;
            else cache.h_seed = {};
            if (auto h = schema.get_handle("centroid"); h) cache.h_centroid = *h;
            else cache.h_centroid = {};
        }
        h_size = cache.h_size;
        h_id = cache.h_id;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_energy = cache.h_energy;
        h_maxEnergy = cache.h_maxEnergy;
        h_seed = cache.h_seed;
        h_centroid = cache.h_centroid;
        return true;
    }

    /// Current row count of the bound bank view.
    [[nodiscard]] int rows() const noexcept { return m_view.rows(); }

    /// True if bind() has been called with a valid view.
    [[nodiscard]] bool valid() const noexcept { return m_view.valid(); }

    /// Access the underlying bank_view (for advanced use).
    [[nodiscard]] const hipo::bank_view& view() const noexcept { return m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (handle-based, zero string lookup)
    // -------------------------------------------------------------------
    /// size: Cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// id: Cluster ID
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// sector: Cluster sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: Cluster layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// energy: Cluster total energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// maxEnergy: Seed energy
    [[nodiscard]] float maxEnergy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_maxEnergy, row));
    }
    /// seed: Seed strip
    [[nodiscard]] int seed(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_seed, row));
    }
    /// centroid: Centroid strip number
    [[nodiscard]] float centroid(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroid, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// maxEnergy column as a contiguous span.
    [[nodiscard]] std::span<const float> maxEnergy_col() const noexcept {
        return m_view.column<float>(h_maxEnergy);
    }
    /// seed column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> seed_col() const noexcept {
        return m_view.column<int16_t>(h_seed);
    }
    /// centroid column as a contiguous span.
    [[nodiscard]] std::span<const float> centroid_col() const noexcept {
        return m_view.column<float>(h_centroid);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_size{};
    hipo::column_handle h_id{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_maxEnergy{};
    hipo::column_handle h_seed{};
    hipo::column_handle h_centroid{};

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
