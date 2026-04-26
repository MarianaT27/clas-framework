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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `CTOF::rawhits` (group 20400, item 20).
/// reconstructed hit info from U/D CTOF PMTs
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CTOF_rawhits bank;
/// if (auto sc = bank.bind(event.get_bank("CTOF::rawhits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CTOF_rawhits {
 public:
    static constexpr std::string_view bank_name = "CTOF::rawhits";
    static constexpr int bank_group = 20400;
    static constexpr int bank_item  = 20;

    class Row;

    CTOF_rawhits() = default;

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
            hipo::column_handle h_component;
            hipo::column_handle h_energy_up;
            hipo::column_handle h_energy_down;
            hipo::column_handle h_time_up;
            hipo::column_handle h_time_down;
            hipo::column_handle h_energy_up_unc;
            hipo::column_handle h_energy_down_unc;
            hipo::column_handle h_time_up_unc;
            hipo::column_handle h_time_down_unc;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("component"); h) cache.h_component = *h;
            else cache.h_component = {};
            if (auto h = schema.get_handle("energy_up"); h) cache.h_energy_up = *h;
            else cache.h_energy_up = {};
            if (auto h = schema.get_handle("energy_down"); h) cache.h_energy_down = *h;
            else cache.h_energy_down = {};
            if (auto h = schema.get_handle("time_up"); h) cache.h_time_up = *h;
            else cache.h_time_up = {};
            if (auto h = schema.get_handle("time_down"); h) cache.h_time_down = *h;
            else cache.h_time_down = {};
            if (auto h = schema.get_handle("energy_up_unc"); h) cache.h_energy_up_unc = *h;
            else cache.h_energy_up_unc = {};
            if (auto h = schema.get_handle("energy_down_unc"); h) cache.h_energy_down_unc = *h;
            else cache.h_energy_down_unc = {};
            if (auto h = schema.get_handle("time_up_unc"); h) cache.h_time_up_unc = *h;
            else cache.h_time_up_unc = {};
            if (auto h = schema.get_handle("time_down_unc"); h) cache.h_time_down_unc = *h;
            else cache.h_time_down_unc = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_component = cache.h_component;
        h_energy_up = cache.h_energy_up;
        h_energy_down = cache.h_energy_down;
        h_time_up = cache.h_time_up;
        h_time_down = cache.h_time_down;
        h_energy_up_unc = cache.h_energy_up_unc;
        h_energy_down_unc = cache.h_energy_down_unc;
        h_time_up_unc = cache.h_time_up_unc;
        h_time_down_unc = cache.h_time_down_unc;
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
    /// component: paddle id of CTOF
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// energy_up: E dep (MeV) from U PMT
    [[nodiscard]] float energy_up(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_up, row));
    }
    /// energy_down: E dep (MeV) from D PMT
    [[nodiscard]] float energy_down(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_down, row));
    }
    /// time_up: Hit time (ns) from U PMT
    [[nodiscard]] float time_up(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_up, row));
    }
    /// time_down: Hit time (ns) from D PMT
    [[nodiscard]] float time_down(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_down, row));
    }
    /// energy_up_unc: E dep unc (MeV) from U PMT
    [[nodiscard]] float energy_up_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_up_unc, row));
    }
    /// energy_down_unc: E dep unc (MeV) from D PMT
    [[nodiscard]] float energy_down_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_down_unc, row));
    }
    /// time_up_unc: Hit time unc (ns) from U PMT
    [[nodiscard]] float time_up_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_up_unc, row));
    }
    /// time_down_unc: Hit time unc (ns) from D PMT
    [[nodiscard]] float time_down_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_down_unc, row));
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
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
    }
    /// energy_up column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_up_col() const noexcept {
        return m_view.column<float>(h_energy_up);
    }
    /// energy_down column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_down_col() const noexcept {
        return m_view.column<float>(h_energy_down);
    }
    /// time_up column as a contiguous span.
    [[nodiscard]] std::span<const float> time_up_col() const noexcept {
        return m_view.column<float>(h_time_up);
    }
    /// time_down column as a contiguous span.
    [[nodiscard]] std::span<const float> time_down_col() const noexcept {
        return m_view.column<float>(h_time_down);
    }
    /// energy_up_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_up_unc_col() const noexcept {
        return m_view.column<float>(h_energy_up_unc);
    }
    /// energy_down_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_down_unc_col() const noexcept {
        return m_view.column<float>(h_energy_down_unc);
    }
    /// time_up_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> time_up_unc_col() const noexcept {
        return m_view.column<float>(h_time_up_unc);
    }
    /// time_down_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> time_down_unc_col() const noexcept {
        return m_view.column<float>(h_time_down_unc);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_component{};
    hipo::column_handle h_energy_up{};
    hipo::column_handle h_energy_down{};
    hipo::column_handle h_time_up{};
    hipo::column_handle h_time_down{};
    hipo::column_handle h_energy_up_unc{};
    hipo::column_handle h_energy_down_unc{};
    hipo::column_handle h_time_up_unc{};
    hipo::column_handle h_time_down_unc{};

    friend class Row;
};

/// Lightweight row view for CTOF::rawhits. Stores a pointer + row index only.
class CTOF_rawhits::Row {
 public:
    Row(const CTOF_rawhits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy_up() const noexcept { return m_bank->energy_up(m_row); }
    [[nodiscard]] float energy_down() const noexcept { return m_bank->energy_down(m_row); }
    [[nodiscard]] float time_up() const noexcept { return m_bank->time_up(m_row); }
    [[nodiscard]] float time_down() const noexcept { return m_bank->time_down(m_row); }
    [[nodiscard]] float energy_up_unc() const noexcept { return m_bank->energy_up_unc(m_row); }
    [[nodiscard]] float energy_down_unc() const noexcept { return m_bank->energy_down_unc(m_row); }
    [[nodiscard]] float time_up_unc() const noexcept { return m_bank->time_up_unc(m_row); }
    [[nodiscard]] float time_down_unc() const noexcept { return m_bank->time_down_unc(m_row); }
 private:
    const CTOF_rawhits* m_bank;
    int m_row;
};

inline CTOF_rawhits::Row CTOF_rawhits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
