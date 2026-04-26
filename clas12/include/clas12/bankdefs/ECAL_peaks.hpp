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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `ECAL::peaks` (group 20700, item 22).
/// reconstructed peaks from ECAL
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ECAL_peaks bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::peaks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ECAL_peaks {
 public:
    static constexpr std::string_view bank_name = "ECAL::peaks";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 22;

    class Row;

    ECAL_peaks() = default;

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
            hipo::column_handle h_energy;
            hipo::column_handle h_time;
            hipo::column_handle h_xo;
            hipo::column_handle h_yo;
            hipo::column_handle h_zo;
            hipo::column_handle h_xe;
            hipo::column_handle h_ye;
            hipo::column_handle h_ze;
            hipo::column_handle h_width;
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
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("xo"); h) cache.h_xo = *h;
            else cache.h_xo = {};
            if (auto h = schema.get_handle("yo"); h) cache.h_yo = *h;
            else cache.h_yo = {};
            if (auto h = schema.get_handle("zo"); h) cache.h_zo = *h;
            else cache.h_zo = {};
            if (auto h = schema.get_handle("xe"); h) cache.h_xe = *h;
            else cache.h_xe = {};
            if (auto h = schema.get_handle("ye"); h) cache.h_ye = *h;
            else cache.h_ye = {};
            if (auto h = schema.get_handle("ze"); h) cache.h_ze = *h;
            else cache.h_ze = {};
            if (auto h = schema.get_handle("width"); h) cache.h_width = *h;
            else cache.h_width = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_energy = cache.h_energy;
        h_time = cache.h_time;
        h_xo = cache.h_xo;
        h_yo = cache.h_yo;
        h_zo = cache.h_zo;
        h_xe = cache.h_xe;
        h_ye = cache.h_ye;
        h_ze = cache.h_ze;
        h_width = cache.h_width;
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
    /// energy: Energy of the hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Time of the hit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// xo: strip origin X coordinate
    [[nodiscard]] float xo(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_xo, row));
    }
    /// yo: strip origin Y coordinate
    [[nodiscard]] float yo(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_yo, row));
    }
    /// zo: strip origin Z coordinate
    [[nodiscard]] float zo(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_zo, row));
    }
    /// xe: strip end    X coordinate
    [[nodiscard]] float xe(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_xe, row));
    }
    /// ye: strip end    Y coordinate
    [[nodiscard]] float ye(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ye, row));
    }
    /// ze: strip end    Z coordinate
    [[nodiscard]] float ze(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ze, row));
    }
    /// width: width of the peak
    [[nodiscard]] float width(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_width, row));
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
    /// width column as a contiguous span.
    [[nodiscard]] std::span<const float> width_col() const noexcept {
        return m_view.column<float>(h_width);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_xo{};
    hipo::column_handle h_yo{};
    hipo::column_handle h_zo{};
    hipo::column_handle h_xe{};
    hipo::column_handle h_ye{};
    hipo::column_handle h_ze{};
    hipo::column_handle h_width{};

    friend class Row;
};

/// Lightweight row view for ECAL::peaks. Stores a pointer + row index only.
class ECAL_peaks::Row {
 public:
    Row(const ECAL_peaks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float xo() const noexcept { return m_bank->xo(m_row); }
    [[nodiscard]] float yo() const noexcept { return m_bank->yo(m_row); }
    [[nodiscard]] float zo() const noexcept { return m_bank->zo(m_row); }
    [[nodiscard]] float xe() const noexcept { return m_bank->xe(m_row); }
    [[nodiscard]] float ye() const noexcept { return m_bank->ye(m_row); }
    [[nodiscard]] float ze() const noexcept { return m_bank->ze(m_row); }
    [[nodiscard]] float width() const noexcept { return m_bank->width(m_row); }
 private:
    const ECAL_peaks* m_bank;
    int m_row;
};

inline ECAL_peaks::Row ECAL_peaks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
