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


/// HIPO bank wrapper for `FMT::Hits` (group 20800, item 21).
/// reconstructed FMT hits
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FMT_Hits bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class FMT_Hits {
 public:
    static constexpr std::string_view bank_name = "FMT::Hits";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 21;

    class Row;

    FMT_Hits() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("strip"); h) h_strip = *h;
        else h_strip = {};
        if (auto h = schema.get_handle("energy"); h) h_energy = *h;
        else h_energy = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("localY"); h) h_localY = *h;
        else h_localY = {};
        if (auto h = schema.get_handle("residual"); h) h_residual = *h;
        else h_residual = {};
        if (auto h = schema.get_handle("adcIndex"); h) h_adcIndex = *h;
        else h_adcIndex = {};
        if (auto h = schema.get_handle("clusterIndex"); h) h_clusterIndex = *h;
        else h_clusterIndex = {};
        if (auto h = schema.get_handle("trackIndex"); h) h_trackIndex = *h;
        else h_trackIndex = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
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
    /// sector: hit sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: hit layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// strip: hit strip
    [[nodiscard]] int strip(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_strip, row));
    }
    /// energy: hit energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: hit time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// localY: hit local y coordinate
    [[nodiscard]] float localY(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_localY, row));
    }
    /// residual: fitted hit residual
    [[nodiscard]] float residual(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_residual, row));
    }
    /// adcIndex: associated adc index
    [[nodiscard]] int adcIndex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_adcIndex, row));
    }
    /// clusterIndex: associated cluster index
    [[nodiscard]] int clusterIndex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusterIndex, row));
    }
    /// trackIndex: associated track index
    [[nodiscard]] int trackIndex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trackIndex, row));
    }
    /// status: hits status (0==good)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// strip column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> strip_col() const noexcept {
        return m_view.column<int16_t>(h_strip);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// localY column as a contiguous span.
    [[nodiscard]] std::span<const float> localY_col() const noexcept {
        return m_view.column<float>(h_localY);
    }
    /// residual column as a contiguous span.
    [[nodiscard]] std::span<const float> residual_col() const noexcept {
        return m_view.column<float>(h_residual);
    }
    /// adcIndex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> adcIndex_col() const noexcept {
        return m_view.column<int16_t>(h_adcIndex);
    }
    /// clusterIndex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusterIndex_col() const noexcept {
        return m_view.column<int16_t>(h_clusterIndex);
    }
    /// trackIndex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trackIndex_col() const noexcept {
        return m_view.column<int16_t>(h_trackIndex);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> status_col() const noexcept {
        return m_view.column<int8_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_strip{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_localY{};
    hipo::column_handle h_residual{};
    hipo::column_handle h_adcIndex{};
    hipo::column_handle h_clusterIndex{};
    hipo::column_handle h_trackIndex{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for FMT::Hits. Stores a pointer + row index only.
class FMT_Hits::Row {
 public:
    Row(const FMT_Hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int strip() const noexcept { return m_bank->strip(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float localY() const noexcept { return m_bank->localY(m_row); }
    [[nodiscard]] float residual() const noexcept { return m_bank->residual(m_row); }
    [[nodiscard]] int adcIndex() const noexcept { return m_bank->adcIndex(m_row); }
    [[nodiscard]] int clusterIndex() const noexcept { return m_bank->clusterIndex(m_row); }
    [[nodiscard]] int trackIndex() const noexcept { return m_bank->trackIndex(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const FMT_Hits* m_bank;
    int m_row;
};

inline FMT_Hits::Row FMT_Hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
