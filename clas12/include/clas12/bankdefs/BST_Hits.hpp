// =============================================================================
// Auto-generated from bst.json - do not edit by hand.
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


/// HIPO bank wrapper for `BST::Hits` (group 20200, item 31).
/// reconstructed BST hits on track
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// BST_Hits bank;
/// if (auto sc = bank.bind(event.get_bank("BST::Hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class BST_Hits {
 public:
    static constexpr std::string_view bank_name = "BST::Hits";
    static constexpr int bank_group = 20200;
    static constexpr int bank_item  = 31;

    class Row;

    BST_Hits() = default;

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
            hipo::column_handle h_ID;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_strip;
            hipo::column_handle h_energy;
            hipo::column_handle h_time;
            hipo::column_handle h_fitResidual;
            hipo::column_handle h_trkingStat;
            hipo::column_handle h_clusterID;
            hipo::column_handle h_trkID;
            hipo::column_handle h_status;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("ID"); h) cache.h_ID = *h;
            else cache.h_ID = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("strip"); h) cache.h_strip = *h;
            else cache.h_strip = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("fitResidual"); h) cache.h_fitResidual = *h;
            else cache.h_fitResidual = {};
            if (auto h = schema.get_handle("trkingStat"); h) cache.h_trkingStat = *h;
            else cache.h_trkingStat = {};
            if (auto h = schema.get_handle("clusterID"); h) cache.h_clusterID = *h;
            else cache.h_clusterID = {};
            if (auto h = schema.get_handle("trkID"); h) cache.h_trkID = *h;
            else cache.h_trkID = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
        }
        h_ID = cache.h_ID;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_strip = cache.h_strip;
        h_energy = cache.h_energy;
        h_time = cache.h_time;
        h_fitResidual = cache.h_fitResidual;
        h_trkingStat = cache.h_trkingStat;
        h_clusterID = cache.h_clusterID;
        h_trkID = cache.h_trkID;
        h_status = cache.h_status;
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
    /// ID: hit ID
    [[nodiscard]] int ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ID, row));
    }
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
    /// energy: energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// fitResidual: fitted hit residual
    [[nodiscard]] float fitResidual(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fitResidual, row));
    }
    /// trkingStat: tracking status
    [[nodiscard]] int trkingStat(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_trkingStat, row));
    }
    /// clusterID: associated cluster ID
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusterID, row));
    }
    /// trkID: associated track ID
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trkID, row));
    }
    /// status: status (0=good)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ID_col() const noexcept {
        return m_view.column<int16_t>(h_ID);
    }
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
    /// fitResidual column as a contiguous span.
    [[nodiscard]] std::span<const float> fitResidual_col() const noexcept {
        return m_view.column<float>(h_fitResidual);
    }
    /// trkingStat column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> trkingStat_col() const noexcept {
        return m_view.column<int8_t>(h_trkingStat);
    }
    /// clusterID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusterID_col() const noexcept {
        return m_view.column<int16_t>(h_clusterID);
    }
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trkID_col() const noexcept {
        return m_view.column<int16_t>(h_trkID);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> status_col() const noexcept {
        return m_view.column<int8_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_ID{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_strip{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_fitResidual{};
    hipo::column_handle h_trkingStat{};
    hipo::column_handle h_clusterID{};
    hipo::column_handle h_trkID{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for BST::Hits. Stores a pointer + row index only.
class BST_Hits::Row {
 public:
    Row(const BST_Hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int strip() const noexcept { return m_bank->strip(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float fitResidual() const noexcept { return m_bank->fitResidual(m_row); }
    [[nodiscard]] int trkingStat() const noexcept { return m_bank->trkingStat(m_row); }
    [[nodiscard]] int clusterID() const noexcept { return m_bank->clusterID(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const BST_Hits* m_bank;
    int m_row;
};

inline BST_Hits::Row BST_Hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
