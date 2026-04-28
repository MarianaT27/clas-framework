// =============================================================================
// Auto-generated from rich.json - do not edit by hand.
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


/// HIPO bank wrapper for `RICH::Cluster` (group 21800, item 42).
/// Reconstructed Clusters in RICH
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RICH_Cluster bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Cluster")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Cluster {
 public:
    static constexpr std::string_view bank_name = "RICH::Cluster";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 42;

    class Row;

    RICH_Cluster() = default;

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
            hipo::column_handle h_size;
            hipo::column_handle h_sector;
            hipo::column_handle h_tile;
            hipo::column_handle h_pmt;
            hipo::column_handle h_charge;
            hipo::column_handle h_time;
            hipo::column_handle h_rawtime;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_wtime;
            hipo::column_handle h_wx;
            hipo::column_handle h_wy;
            hipo::column_handle h_wz;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("size"); h) cache.h_size = *h;
            else cache.h_size = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("tile"); h) cache.h_tile = *h;
            else cache.h_tile = {};
            if (auto h = schema.get_handle("pmt"); h) cache.h_pmt = *h;
            else cache.h_pmt = {};
            if (auto h = schema.get_handle("charge"); h) cache.h_charge = *h;
            else cache.h_charge = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("rawtime"); h) cache.h_rawtime = *h;
            else cache.h_rawtime = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("wtime"); h) cache.h_wtime = *h;
            else cache.h_wtime = {};
            if (auto h = schema.get_handle("wx"); h) cache.h_wx = *h;
            else cache.h_wx = {};
            if (auto h = schema.get_handle("wy"); h) cache.h_wy = *h;
            else cache.h_wy = {};
            if (auto h = schema.get_handle("wz"); h) cache.h_wz = *h;
            else cache.h_wz = {};
        }
        h_id = cache.h_id;
        h_size = cache.h_size;
        h_sector = cache.h_sector;
        h_tile = cache.h_tile;
        h_pmt = cache.h_pmt;
        h_charge = cache.h_charge;
        h_time = cache.h_time;
        h_rawtime = cache.h_rawtime;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_wtime = cache.h_wtime;
        h_wx = cache.h_wx;
        h_wy = cache.h_wy;
        h_wz = cache.h_wz;
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
    /// id: Cluster id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// size: Cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// sector: Cluster sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_sector, row));
    }
    /// tile: Cluster tile
    [[nodiscard]] int tile(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_tile, row));
    }
    /// pmt: Cluster pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pmt, row));
    }
    /// charge: Cluster charge
    [[nodiscard]] float charge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_charge, row));
    }
    /// time: Cluster time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// rawtime: Cluster rawtime
    [[nodiscard]] float rawtime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_rawtime, row));
    }
    /// x: Cluster x
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Cluster y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Cluster z
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// wtime: Cluster wtime
    [[nodiscard]] float wtime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_wtime, row));
    }
    /// wx: Cluster wx
    [[nodiscard]] float wx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_wx, row));
    }
    /// wy: Cluster wy
    [[nodiscard]] float wy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_wy, row));
    }
    /// wz: Cluster wz
    [[nodiscard]] float wz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_wz, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> sector_col() const noexcept {
        return m_view.column<int16_t>(h_sector);
    }
    /// tile column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> tile_col() const noexcept {
        return m_view.column<int16_t>(h_tile);
    }
    /// pmt column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pmt_col() const noexcept {
        return m_view.column<int16_t>(h_pmt);
    }
    /// charge column as a contiguous span.
    [[nodiscard]] std::span<const float> charge_col() const noexcept {
        return m_view.column<float>(h_charge);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// rawtime column as a contiguous span.
    [[nodiscard]] std::span<const float> rawtime_col() const noexcept {
        return m_view.column<float>(h_rawtime);
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
    /// wtime column as a contiguous span.
    [[nodiscard]] std::span<const float> wtime_col() const noexcept {
        return m_view.column<float>(h_wtime);
    }
    /// wx column as a contiguous span.
    [[nodiscard]] std::span<const float> wx_col() const noexcept {
        return m_view.column<float>(h_wx);
    }
    /// wy column as a contiguous span.
    [[nodiscard]] std::span<const float> wy_col() const noexcept {
        return m_view.column<float>(h_wy);
    }
    /// wz column as a contiguous span.
    [[nodiscard]] std::span<const float> wz_col() const noexcept {
        return m_view.column<float>(h_wz);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_size{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_tile{};
    hipo::column_handle h_pmt{};
    hipo::column_handle h_charge{};
    hipo::column_handle h_time{};
    hipo::column_handle h_rawtime{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_wtime{};
    hipo::column_handle h_wx{};
    hipo::column_handle h_wy{};
    hipo::column_handle h_wz{};

    friend class Row;
};

/// Lightweight row view for RICH::Cluster. Stores a pointer + row index only.
class RICH_Cluster::Row {
 public:
    Row(const RICH_Cluster* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int tile() const noexcept { return m_bank->tile(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] float charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float rawtime() const noexcept { return m_bank->rawtime(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float wtime() const noexcept { return m_bank->wtime(m_row); }
    [[nodiscard]] float wx() const noexcept { return m_bank->wx(m_row); }
    [[nodiscard]] float wy() const noexcept { return m_bank->wy(m_row); }
    [[nodiscard]] float wz() const noexcept { return m_bank->wz(m_row); }
 private:
    const RICH_Cluster* m_bank;
    int m_row;
};

inline RICH_Cluster::Row RICH_Cluster::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
