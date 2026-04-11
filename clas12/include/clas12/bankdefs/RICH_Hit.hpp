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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `RICH::Hit` (group 21800, item 40).
/// Reconstructed Hits in RICH
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RICH_Hit bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Hit")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Hit {
 public:
    static constexpr std::string_view bank_name = "RICH::Hit";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 40;

    class Row;

    RICH_Hit() = default;

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
        if (auto h = schema.get_handle("tile"); h) h_tile = *h;
        else h_tile = {};
        if (auto h = schema.get_handle("pmt"); h) h_pmt = *h;
        else h_pmt = {};
        if (auto h = schema.get_handle("anode"); h) h_anode = *h;
        else h_anode = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("rawtime"); h) h_rawtime = *h;
        else h_rawtime = {};
        if (auto h = schema.get_handle("cluster"); h) h_cluster = *h;
        else h_cluster = {};
        if (auto h = schema.get_handle("xtalk"); h) h_xtalk = *h;
        else h_xtalk = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
        if (auto h = schema.get_handle("duration"); h) h_duration = *h;
        else h_duration = {};
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
    /// id: Hit id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// sector: Hit sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_sector, row));
    }
    /// tile: Hit tile
    [[nodiscard]] int tile(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_tile, row));
    }
    /// pmt: Hit pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pmt, row));
    }
    /// anode: Hit anode
    [[nodiscard]] int anode(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_anode, row));
    }
    /// x: Hit x
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Hit y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Hit z
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// time: Hit time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// rawtime: Hit rawtime
    [[nodiscard]] float rawtime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_rawtime, row));
    }
    /// cluster: Hit cluster
    [[nodiscard]] int cluster(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_cluster, row));
    }
    /// xtalk: Hit xtalk
    [[nodiscard]] int xtalk(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_xtalk, row));
    }
    /// status: Hit status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// duration: Hit duration
    [[nodiscard]] int duration(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_duration, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
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
    /// anode column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> anode_col() const noexcept {
        return m_view.column<int16_t>(h_anode);
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
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// rawtime column as a contiguous span.
    [[nodiscard]] std::span<const float> rawtime_col() const noexcept {
        return m_view.column<float>(h_rawtime);
    }
    /// cluster column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> cluster_col() const noexcept {
        return m_view.column<int16_t>(h_cluster);
    }
    /// xtalk column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> xtalk_col() const noexcept {
        return m_view.column<int16_t>(h_xtalk);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// duration column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> duration_col() const noexcept {
        return m_view.column<int16_t>(h_duration);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_tile{};
    hipo::column_handle h_pmt{};
    hipo::column_handle h_anode{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_time{};
    hipo::column_handle h_rawtime{};
    hipo::column_handle h_cluster{};
    hipo::column_handle h_xtalk{};
    hipo::column_handle h_status{};
    hipo::column_handle h_duration{};

    friend class Row;
};

/// Lightweight row view for RICH::Hit. Stores a pointer + row index only.
class RICH_Hit::Row {
 public:
    Row(const RICH_Hit* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int tile() const noexcept { return m_bank->tile(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] int anode() const noexcept { return m_bank->anode(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float rawtime() const noexcept { return m_bank->rawtime(m_row); }
    [[nodiscard]] int cluster() const noexcept { return m_bank->cluster(m_row); }
    [[nodiscard]] int xtalk() const noexcept { return m_bank->xtalk(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int duration() const noexcept { return m_bank->duration(m_row); }
 private:
    const RICH_Hit* m_bank;
    int m_row;
};

inline RICH_Hit::Row RICH_Hit::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
