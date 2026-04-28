// =============================================================================
// Auto-generated from ecal.json - do not edit by hand.
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


/// HIPO bank wrapper for `ECAL::clusters` (group 20700, item 23).
/// reconstructed clusters in ECAL
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ECAL_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ECAL_clusters {
 public:
    static constexpr std::string_view bank_name = "ECAL::clusters";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 23;

    class Row;

    ECAL_clusters() = default;

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
            hipo::column_handle h_id;
            hipo::column_handle h_status;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_energy;
            hipo::column_handle h_time;
            hipo::column_handle h_widthU;
            hipo::column_handle h_widthV;
            hipo::column_handle h_widthW;
            hipo::column_handle h_idU;
            hipo::column_handle h_idV;
            hipo::column_handle h_idW;
            hipo::column_handle h_coordU;
            hipo::column_handle h_coordV;
            hipo::column_handle h_coordW;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("widthU"); h) cache.h_widthU = *h;
            else cache.h_widthU = {};
            if (auto h = schema.get_handle("widthV"); h) cache.h_widthV = *h;
            else cache.h_widthV = {};
            if (auto h = schema.get_handle("widthW"); h) cache.h_widthW = *h;
            else cache.h_widthW = {};
            if (auto h = schema.get_handle("idU"); h) cache.h_idU = *h;
            else cache.h_idU = {};
            if (auto h = schema.get_handle("idV"); h) cache.h_idV = *h;
            else cache.h_idV = {};
            if (auto h = schema.get_handle("idW"); h) cache.h_idW = *h;
            else cache.h_idW = {};
            if (auto h = schema.get_handle("coordU"); h) cache.h_coordU = *h;
            else cache.h_coordU = {};
            if (auto h = schema.get_handle("coordV"); h) cache.h_coordV = *h;
            else cache.h_coordV = {};
            if (auto h = schema.get_handle("coordW"); h) cache.h_coordW = *h;
            else cache.h_coordW = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_energy = cache.h_energy;
        h_time = cache.h_time;
        h_widthU = cache.h_widthU;
        h_widthV = cache.h_widthV;
        h_widthW = cache.h_widthW;
        h_idU = cache.h_idU;
        h_idV = cache.h_idV;
        h_idW = cache.h_idW;
        h_coordU = cache.h_coordU;
        h_coordV = cache.h_coordV;
        h_coordW = cache.h_coordW;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: status of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: sector of ECAL
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: Layer of ECAL (1-3:PCAL, 4-6:ECIN, 7-9:ECOUT
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// x: X coordinate of the hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Y coordinate of the hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Z coordinate of the hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// energy: Energy of the hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Energy of the hit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// widthU: width of U peak
    [[nodiscard]] float widthU(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_widthU, row));
    }
    /// widthV: width of V peak
    [[nodiscard]] float widthV(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_widthV, row));
    }
    /// widthW: width of W peak
    [[nodiscard]] float widthW(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_widthW, row));
    }
    /// idU: id of U peak
    [[nodiscard]] int idU(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_idU, row));
    }
    /// idV: id of V peak
    [[nodiscard]] int idV(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_idV, row));
    }
    /// idW: id of W peak
    [[nodiscard]] int idW(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_idW, row));
    }
    /// coordU: U coordinate 
    [[nodiscard]] int coordU(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_coordU, row));
    }
    /// coordV: V coordinate
    [[nodiscard]] int coordV(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_coordV, row));
    }
    /// coordW: W coordinate
    [[nodiscard]] int coordW(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_coordW, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// x column as a contiguous span.
    [[nodiscard]] std::span<const float> x_col() const noexcept {
        return m_view.column<float>(h_x);
    }
    /// y column as a contiguous span.
    [[nodiscard]] std::span<const float> y_col() const noexcept {
        return m_view.column<float>(h_y);
    }
    /// z column as a contiguous span.
    [[nodiscard]] std::span<const float> z_col() const noexcept {
        return m_view.column<float>(h_z);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// widthU column as a contiguous span.
    [[nodiscard]] std::span<const float> widthU_col() const noexcept {
        return m_view.column<float>(h_widthU);
    }
    /// widthV column as a contiguous span.
    [[nodiscard]] std::span<const float> widthV_col() const noexcept {
        return m_view.column<float>(h_widthV);
    }
    /// widthW column as a contiguous span.
    [[nodiscard]] std::span<const float> widthW_col() const noexcept {
        return m_view.column<float>(h_widthW);
    }
    /// idU column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> idU_col() const noexcept {
        return m_view.column<int8_t>(h_idU);
    }
    /// idV column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> idV_col() const noexcept {
        return m_view.column<int8_t>(h_idV);
    }
    /// idW column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> idW_col() const noexcept {
        return m_view.column<int8_t>(h_idW);
    }
    /// coordU column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> coordU_col() const noexcept {
        return m_view.column<int32_t>(h_coordU);
    }
    /// coordV column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> coordV_col() const noexcept {
        return m_view.column<int32_t>(h_coordV);
    }
    /// coordW column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> coordW_col() const noexcept {
        return m_view.column<int32_t>(h_coordW);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_widthU{};
    hipo::column_handle h_widthV{};
    hipo::column_handle h_widthW{};
    hipo::column_handle h_idU{};
    hipo::column_handle h_idV{};
    hipo::column_handle h_idW{};
    hipo::column_handle h_coordU{};
    hipo::column_handle h_coordV{};
    hipo::column_handle h_coordW{};

    friend class Row;
};

/// Lightweight row view for ECAL::clusters. Stores a pointer + row index only.
class ECAL_clusters::Row {
 public:
    Row(const ECAL_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float widthU() const noexcept { return m_bank->widthU(m_row); }
    [[nodiscard]] float widthV() const noexcept { return m_bank->widthV(m_row); }
    [[nodiscard]] float widthW() const noexcept { return m_bank->widthW(m_row); }
    [[nodiscard]] int idU() const noexcept { return m_bank->idU(m_row); }
    [[nodiscard]] int idV() const noexcept { return m_bank->idV(m_row); }
    [[nodiscard]] int idW() const noexcept { return m_bank->idW(m_row); }
    [[nodiscard]] int coordU() const noexcept { return m_bank->coordU(m_row); }
    [[nodiscard]] int coordV() const noexcept { return m_bank->coordV(m_row); }
    [[nodiscard]] int coordW() const noexcept { return m_bank->coordW(m_row); }
 private:
    const ECAL_clusters* m_bank;
    int m_row;
};

inline ECAL_clusters::Row ECAL_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
