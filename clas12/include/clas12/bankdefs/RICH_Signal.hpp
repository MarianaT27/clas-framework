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


/// HIPO bank wrapper for `RICH::Signal` (group 21800, item 41).
/// Reconstructed Signals in RICH
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RICH_Signal bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Signal")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Signal {
 public:
    static constexpr std::string_view bank_name = "RICH::Signal";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 41;

    class Row;

    RICH_Signal() = default;

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
            hipo::column_handle h_hindex;
            hipo::column_handle h_sector;
            hipo::column_handle h_pmt;
            hipo::column_handle h_anode;
            hipo::column_handle h_size;
            hipo::column_handle h_status;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_time;
            hipo::column_handle h_rawtime;
            hipo::column_handle h_charge;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("hindex"); h) cache.h_hindex = *h;
            else cache.h_hindex = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("pmt"); h) cache.h_pmt = *h;
            else cache.h_pmt = {};
            if (auto h = schema.get_handle("anode"); h) cache.h_anode = *h;
            else cache.h_anode = {};
            if (auto h = schema.get_handle("size"); h) cache.h_size = *h;
            else cache.h_size = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("rawtime"); h) cache.h_rawtime = *h;
            else cache.h_rawtime = {};
            if (auto h = schema.get_handle("charge"); h) cache.h_charge = *h;
            else cache.h_charge = {};
        }
        h_id = cache.h_id;
        h_hindex = cache.h_hindex;
        h_sector = cache.h_sector;
        h_pmt = cache.h_pmt;
        h_anode = cache.h_anode;
        h_size = cache.h_size;
        h_status = cache.h_status;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_time = cache.h_time;
        h_rawtime = cache.h_rawtime;
        h_charge = cache.h_charge;
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
    /// id: Signal id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// hindex: Signal hindex
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_hindex, row));
    }
    /// sector: Signal sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_sector, row));
    }
    /// pmt: Signal pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pmt, row));
    }
    /// anode: Signal anode
    [[nodiscard]] int anode(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_anode, row));
    }
    /// size: Signal size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// status: Signal status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// x: Signal x
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Signal y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Signal z
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// time: Signal time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// rawtime: Signal rawtime
    [[nodiscard]] float rawtime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_rawtime, row));
    }
    /// charge: Signal charge
    [[nodiscard]] float charge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_charge, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// hindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> hindex_col() const noexcept {
        return m_view.column<int16_t>(h_hindex);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> sector_col() const noexcept {
        return m_view.column<int16_t>(h_sector);
    }
    /// pmt column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pmt_col() const noexcept {
        return m_view.column<int16_t>(h_pmt);
    }
    /// anode column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> anode_col() const noexcept {
        return m_view.column<int16_t>(h_anode);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
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
    /// charge column as a contiguous span.
    [[nodiscard]] std::span<const float> charge_col() const noexcept {
        return m_view.column<float>(h_charge);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle hindex_handle() const noexcept { return h_hindex; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_hindex{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_pmt{};
    hipo::column_handle h_anode{};
    hipo::column_handle h_size{};
    hipo::column_handle h_status{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_time{};
    hipo::column_handle h_rawtime{};
    hipo::column_handle h_charge{};

    friend class Row;
};

/// Lightweight row view for RICH::Signal. Stores a pointer + row index only.
class RICH_Signal::Row {
 public:
    Row(const RICH_Signal* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] int anode() const noexcept { return m_bank->anode(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float rawtime() const noexcept { return m_bank->rawtime(m_row); }
    [[nodiscard]] float charge() const noexcept { return m_bank->charge(m_row); }
 private:
    const RICH_Signal* m_bank;
    int m_row;
};

inline RICH_Signal::Row RICH_Signal::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
