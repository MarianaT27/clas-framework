// =============================================================================
// Auto-generated from recoil.json - do not edit by hand.
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


/// HIPO bank wrapper for `RECOIL::clusters` (group 22600, item 22).
/// reconstructed clusters from RECOIL
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RECOIL_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("RECOIL::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RECOIL_clusters {
 public:
    static constexpr std::string_view bank_name = "RECOIL::clusters";
    static constexpr int bank_group = 22600;
    static constexpr int bank_item  = 22;

    class Row;

    RECOIL_clusters() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
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
        if (auto h = schema.get_handle("xo"); h) h_xo = *h;
        else h_xo = {};
        if (auto h = schema.get_handle("yo"); h) h_yo = *h;
        else h_yo = {};
        if (auto h = schema.get_handle("zo"); h) h_zo = *h;
        else h_zo = {};
        if (auto h = schema.get_handle("xe"); h) h_xe = *h;
        else h_xe = {};
        if (auto h = schema.get_handle("ye"); h) h_ye = *h;
        else h_ye = {};
        if (auto h = schema.get_handle("ze"); h) h_ze = *h;
        else h_ze = {};
        if (auto h = schema.get_handle("size"); h) h_size = *h;
        else h_size = {};
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
    /// id: id of the cluster
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// sector: sector of RECOIL
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer of RECOIL
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// strip: seed strip
    [[nodiscard]] int strip(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_strip, row));
    }
    /// energy: energy of the cluster (eV)
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: time of the cluster (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// xo: strip origin X coordinate (cm)
    [[nodiscard]] float xo(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_xo, row));
    }
    /// yo: strip origin Y coordinate (cm)
    [[nodiscard]] float yo(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_yo, row));
    }
    /// zo: strip origin Z coordinate (cm)
    [[nodiscard]] float zo(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_zo, row));
    }
    /// xe: strip end    X coordinate (cm)
    [[nodiscard]] float xe(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_xe, row));
    }
    /// ye: strip end    Y coordinate (cm)
    [[nodiscard]] float ye(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ye, row));
    }
    /// ze: strip end    Z coordinate (cm)
    [[nodiscard]] float ze(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ze, row));
    }
    /// size: size of the cluster
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// status: status of the cluster
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
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
    /// xo column as a contiguous span.
    [[nodiscard]] std::span<const float> xo_col() const noexcept {
        return m_view.column<float>(h_xo);
    }
    /// yo column as a contiguous span.
    [[nodiscard]] std::span<const float> yo_col() const noexcept {
        return m_view.column<float>(h_yo);
    }
    /// zo column as a contiguous span.
    [[nodiscard]] std::span<const float> zo_col() const noexcept {
        return m_view.column<float>(h_zo);
    }
    /// xe column as a contiguous span.
    [[nodiscard]] std::span<const float> xe_col() const noexcept {
        return m_view.column<float>(h_xe);
    }
    /// ye column as a contiguous span.
    [[nodiscard]] std::span<const float> ye_col() const noexcept {
        return m_view.column<float>(h_ye);
    }
    /// ze column as a contiguous span.
    [[nodiscard]] std::span<const float> ze_col() const noexcept {
        return m_view.column<float>(h_ze);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_strip{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_xo{};
    hipo::column_handle h_yo{};
    hipo::column_handle h_zo{};
    hipo::column_handle h_xe{};
    hipo::column_handle h_ye{};
    hipo::column_handle h_ze{};
    hipo::column_handle h_size{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for RECOIL::clusters. Stores a pointer + row index only.
class RECOIL_clusters::Row {
 public:
    Row(const RECOIL_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int strip() const noexcept { return m_bank->strip(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float xo() const noexcept { return m_bank->xo(m_row); }
    [[nodiscard]] float yo() const noexcept { return m_bank->yo(m_row); }
    [[nodiscard]] float zo() const noexcept { return m_bank->zo(m_row); }
    [[nodiscard]] float xe() const noexcept { return m_bank->xe(m_row); }
    [[nodiscard]] float ye() const noexcept { return m_bank->ye(m_row); }
    [[nodiscard]] float ze() const noexcept { return m_bank->ze(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const RECOIL_clusters* m_bank;
    int m_row;
};

inline RECOIL_clusters::Row RECOIL_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
