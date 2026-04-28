// =============================================================================
// Auto-generated from data.json - do not edit by hand.
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


/// HIPO bank wrapper for `RTPC::pos` (group 21900, item 14).
/// Digitized bank for the Radial TPC
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RTPC_pos bank;
/// if (auto sc = bank.bind(event.get_bank("RTPC::pos")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto step = row.step();
/// }
/// @endcode
class RTPC_pos {
 public:
    static constexpr std::string_view bank_name = "RTPC::pos";
    static constexpr int bank_group = 21900;
    static constexpr int bank_item  = 14;

    class Row;

    RTPC_pos() = default;

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
            hipo::column_handle h_step;
            hipo::column_handle h_time;
            hipo::column_handle h_energy;
            hipo::column_handle h_posx;
            hipo::column_handle h_posy;
            hipo::column_handle h_posz;
            hipo::column_handle h_phi;
            hipo::column_handle h_tid;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("step"); h) cache.h_step = *h;
            else cache.h_step = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("posx"); h) cache.h_posx = *h;
            else cache.h_posx = {};
            if (auto h = schema.get_handle("posy"); h) cache.h_posy = *h;
            else cache.h_posy = {};
            if (auto h = schema.get_handle("posz"); h) cache.h_posz = *h;
            else cache.h_posz = {};
            if (auto h = schema.get_handle("phi"); h) cache.h_phi = *h;
            else cache.h_phi = {};
            if (auto h = schema.get_handle("tid"); h) cache.h_tid = *h;
            else cache.h_tid = {};
        }
        h_step = cache.h_step;
        h_time = cache.h_time;
        h_energy = cache.h_energy;
        h_posx = cache.h_posx;
        h_posy = cache.h_posy;
        h_posz = cache.h_posz;
        h_phi = cache.h_phi;
        h_tid = cache.h_tid;
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
    /// step: step
    [[nodiscard]] int step(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_step, row));
    }
    /// time: time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// energy: energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// posx: x position (mm)
    [[nodiscard]] float posx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_posx, row));
    }
    /// posy: y position (mm)
    [[nodiscard]] float posy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_posy, row));
    }
    /// posz: z position (mm)
    [[nodiscard]] float posz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_posz, row));
    }
    /// phi: phi (rad)
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_phi, row));
    }
    /// tid: track id
    [[nodiscard]] int tid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tid, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// step column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> step_col() const noexcept {
        return m_view.column<int32_t>(h_step);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// posx column as a contiguous span.
    [[nodiscard]] std::span<const float> posx_col() const noexcept {
        return m_view.column<float>(h_posx);
    }
    /// posy column as a contiguous span.
    [[nodiscard]] std::span<const float> posy_col() const noexcept {
        return m_view.column<float>(h_posy);
    }
    /// posz column as a contiguous span.
    [[nodiscard]] std::span<const float> posz_col() const noexcept {
        return m_view.column<float>(h_posz);
    }
    /// phi column as a contiguous span.
    [[nodiscard]] std::span<const float> phi_col() const noexcept {
        return m_view.column<float>(h_phi);
    }
    /// tid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tid_col() const noexcept {
        return m_view.column<int32_t>(h_tid);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_step{};
    hipo::column_handle h_time{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_posx{};
    hipo::column_handle h_posy{};
    hipo::column_handle h_posz{};
    hipo::column_handle h_phi{};
    hipo::column_handle h_tid{};

    friend class Row;
};

/// Lightweight row view for RTPC::pos. Stores a pointer + row index only.
class RTPC_pos::Row {
 public:
    Row(const RTPC_pos* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int step() const noexcept { return m_bank->step(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float posx() const noexcept { return m_bank->posx(m_row); }
    [[nodiscard]] float posy() const noexcept { return m_bank->posy(m_row); }
    [[nodiscard]] float posz() const noexcept { return m_bank->posz(m_row); }
    [[nodiscard]] float phi() const noexcept { return m_bank->phi(m_row); }
    [[nodiscard]] int tid() const noexcept { return m_bank->tid(m_row); }
 private:
    const RTPC_pos* m_bank;
    int m_row;
};

inline RTPC_pos::Row RTPC_pos::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
