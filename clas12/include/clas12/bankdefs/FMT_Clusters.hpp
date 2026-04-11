// =============================================================================
// Auto-generated from fmt.json - do not edit by hand.
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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `FMT::Clusters` (group 20800, item 22).
/// reconstructed FMT clusters
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    class Row;

    FMT_Clusters() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("index"); h) h_index = *h;
        else h_index = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("size"); h) h_size = *h;
        else h_size = {};
        if (auto h = schema.get_handle("centroid"); h) h_centroid = *h;
        else h_centroid = {};
        if (auto h = schema.get_handle("centroidError"); h) h_centroidError = *h;
        else h_centroidError = {};
        if (auto h = schema.get_handle("residual"); h) h_residual = *h;
        else h_residual = {};
        if (auto h = schema.get_handle("doca"); h) h_doca = *h;
        else h_doca = {};
        if (auto h = schema.get_handle("energy"); h) h_energy = *h;
        else h_energy = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("seedStrip"); h) h_seedStrip = *h;
        else h_seedStrip = {};
        if (auto h = schema.get_handle("seedIndex"); h) h_seedIndex = *h;
        else h_seedIndex = {};
        if (auto h = schema.get_handle("trackIndex"); h) h_trackIndex = *h;
        else h_trackIndex = {};
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
    /// index: index
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// size: cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// centroid: centroid position in local coordinates
    [[nodiscard]] float centroid(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroid, row));
    }
    /// centroidError: centroid position error
    [[nodiscard]] float centroidError(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroidError, row));
    }
    /// residual: centroid residual
    [[nodiscard]] float residual(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_residual, row));
    }
    /// doca: dc track doca
    [[nodiscard]] float doca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_doca, row));
    }
    /// energy: cluster total energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: cluster time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// seedStrip: seed strip number
    [[nodiscard]] int seedStrip(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_seedStrip, row));
    }
    /// seedIndex: seed index in hits bank
    [[nodiscard]] int seedIndex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_seedIndex, row));
    }
    /// trackIndex: associated track index
    [[nodiscard]] int trackIndex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trackIndex, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index_col() const noexcept {
        return m_view.column<int16_t>(h_index);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// centroid column as a contiguous span.
    [[nodiscard]] std::span<const float> centroid_col() const noexcept {
        return m_view.column<float>(h_centroid);
    }
    /// centroidError column as a contiguous span.
    [[nodiscard]] std::span<const float> centroidError_col() const noexcept {
        return m_view.column<float>(h_centroidError);
    }
    /// residual column as a contiguous span.
    [[nodiscard]] std::span<const float> residual_col() const noexcept {
        return m_view.column<float>(h_residual);
    }
    /// doca column as a contiguous span.
    [[nodiscard]] std::span<const float> doca_col() const noexcept {
        return m_view.column<float>(h_doca);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// seedStrip column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> seedStrip_col() const noexcept {
        return m_view.column<int16_t>(h_seedStrip);
    }
    /// seedIndex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> seedIndex_col() const noexcept {
        return m_view.column<int16_t>(h_seedIndex);
    }
    /// trackIndex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trackIndex_col() const noexcept {
        return m_view.column<int16_t>(h_trackIndex);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle index_handle() const noexcept { return h_index; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_index{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_size{};
    hipo::column_handle h_centroid{};
    hipo::column_handle h_centroidError{};
    hipo::column_handle h_residual{};
    hipo::column_handle h_doca{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_seedStrip{};
    hipo::column_handle h_seedIndex{};
    hipo::column_handle h_trackIndex{};

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

}  // namespace clas12::bankdefs
