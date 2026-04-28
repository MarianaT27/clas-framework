// =============================================================================
// Auto-generated from tof.json - do not edit by hand.
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


/// HIPO bank wrapper for `FTOF::rawhits` (group 21200, item 20).
/// reconstructed hit info from L/R FTOF PMTs
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FTOF_rawhits bank;
/// if (auto sc = bank.bind(event.get_bank("FTOF::rawhits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class FTOF_rawhits {
 public:
    static constexpr std::string_view bank_name = "FTOF::rawhits";
    static constexpr int bank_group = 21200;
    static constexpr int bank_item  = 20;

    class Row;

    FTOF_rawhits() = default;

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
            hipo::column_handle h_component;
            hipo::column_handle h_energy_left;
            hipo::column_handle h_energy_right;
            hipo::column_handle h_time_left;
            hipo::column_handle h_time_right;
            hipo::column_handle h_energy_left_unc;
            hipo::column_handle h_energy_right_unc;
            hipo::column_handle h_time_left_unc;
            hipo::column_handle h_time_right_unc;
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
            if (auto h = schema.get_handle("component"); h) cache.h_component = *h;
            else cache.h_component = {};
            if (auto h = schema.get_handle("energy_left"); h) cache.h_energy_left = *h;
            else cache.h_energy_left = {};
            if (auto h = schema.get_handle("energy_right"); h) cache.h_energy_right = *h;
            else cache.h_energy_right = {};
            if (auto h = schema.get_handle("time_left"); h) cache.h_time_left = *h;
            else cache.h_time_left = {};
            if (auto h = schema.get_handle("time_right"); h) cache.h_time_right = *h;
            else cache.h_time_right = {};
            if (auto h = schema.get_handle("energy_left_unc"); h) cache.h_energy_left_unc = *h;
            else cache.h_energy_left_unc = {};
            if (auto h = schema.get_handle("energy_right_unc"); h) cache.h_energy_right_unc = *h;
            else cache.h_energy_right_unc = {};
            if (auto h = schema.get_handle("time_left_unc"); h) cache.h_time_left_unc = *h;
            else cache.h_time_left_unc = {};
            if (auto h = schema.get_handle("time_right_unc"); h) cache.h_time_right_unc = *h;
            else cache.h_time_right_unc = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_energy_left = cache.h_energy_left;
        h_energy_right = cache.h_energy_right;
        h_time_left = cache.h_time_left;
        h_time_right = cache.h_time_right;
        h_energy_left_unc = cache.h_energy_left_unc;
        h_energy_right_unc = cache.h_energy_right_unc;
        h_time_left_unc = cache.h_time_left_unc;
        h_time_right_unc = cache.h_time_right_unc;
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
    /// status: hit status, a 4-digit word defined based on presence (0) or absence (1) of TDCR-TDCL-ADCR-ADCL
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: sector of FTOF
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: panel id of FTOF (1-1A, 2-1B, 3-2
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: paddle id of FTOF
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// energy_left: E dep (MeV) from L PMT
    [[nodiscard]] float energy_left(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_left, row));
    }
    /// energy_right: E dep (MeV) from R PMT
    [[nodiscard]] float energy_right(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_right, row));
    }
    /// time_left: Hit time (ns) from L PMT
    [[nodiscard]] float time_left(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_left, row));
    }
    /// time_right: Hit time (ns) from R PMT
    [[nodiscard]] float time_right(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_right, row));
    }
    /// energy_left_unc: E dep unc (MeV) from L PMT
    [[nodiscard]] float energy_left_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_left_unc, row));
    }
    /// energy_right_unc: E dep unc (MeV) from R PMT
    [[nodiscard]] float energy_right_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_right_unc, row));
    }
    /// time_left_unc: Hit time unc (ns) from L PMT
    [[nodiscard]] float time_left_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_left_unc, row));
    }
    /// time_right_unc: Hit time unc (ns) from R PMT
    [[nodiscard]] float time_right_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_right_unc, row));
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
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
    }
    /// energy_left column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_left_col() const noexcept {
        return m_view.column<float>(h_energy_left);
    }
    /// energy_right column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_right_col() const noexcept {
        return m_view.column<float>(h_energy_right);
    }
    /// time_left column as a contiguous span.
    [[nodiscard]] std::span<const float> time_left_col() const noexcept {
        return m_view.column<float>(h_time_left);
    }
    /// time_right column as a contiguous span.
    [[nodiscard]] std::span<const float> time_right_col() const noexcept {
        return m_view.column<float>(h_time_right);
    }
    /// energy_left_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_left_unc_col() const noexcept {
        return m_view.column<float>(h_energy_left_unc);
    }
    /// energy_right_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_right_unc_col() const noexcept {
        return m_view.column<float>(h_energy_right_unc);
    }
    /// time_left_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> time_left_unc_col() const noexcept {
        return m_view.column<float>(h_time_left_unc);
    }
    /// time_right_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> time_right_unc_col() const noexcept {
        return m_view.column<float>(h_time_right_unc);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_energy_left{};
    hipo::column_handle h_energy_right{};
    hipo::column_handle h_time_left{};
    hipo::column_handle h_time_right{};
    hipo::column_handle h_energy_left_unc{};
    hipo::column_handle h_energy_right_unc{};
    hipo::column_handle h_time_left_unc{};
    hipo::column_handle h_time_right_unc{};

    friend class Row;
};

/// Lightweight row view for FTOF::rawhits. Stores a pointer + row index only.
class FTOF_rawhits::Row {
 public:
    Row(const FTOF_rawhits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy_left() const noexcept { return m_bank->energy_left(m_row); }
    [[nodiscard]] float energy_right() const noexcept { return m_bank->energy_right(m_row); }
    [[nodiscard]] float time_left() const noexcept { return m_bank->time_left(m_row); }
    [[nodiscard]] float time_right() const noexcept { return m_bank->time_right(m_row); }
    [[nodiscard]] float energy_left_unc() const noexcept { return m_bank->energy_left_unc(m_row); }
    [[nodiscard]] float energy_right_unc() const noexcept { return m_bank->energy_right_unc(m_row); }
    [[nodiscard]] float time_left_unc() const noexcept { return m_bank->time_left_unc(m_row); }
    [[nodiscard]] float time_right_unc() const noexcept { return m_bank->time_right_unc(m_row); }
 private:
    const FTOF_rawhits* m_bank;
    int m_row;
};

inline FTOF_rawhits::Row FTOF_rawhits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
