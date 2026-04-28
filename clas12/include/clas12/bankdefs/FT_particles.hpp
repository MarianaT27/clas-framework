// =============================================================================
// Auto-generated from ft.json - do not edit by hand.
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


/// HIPO bank wrapper for `FT::particles` (group 20900, item 24).
/// Reconstructed Particles in FT
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FT_particles bank;
/// if (auto sc = bank.bind(event.get_bank("FT::particles")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class FT_particles {
 public:
    static constexpr std::string_view bank_name = "FT::particles";
    static constexpr int bank_group = 20900;
    static constexpr int bank_item  = 24;

    class Row;

    FT_particles() = default;

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
            hipo::column_handle h_charge;
            hipo::column_handle h_energy;
            hipo::column_handle h_cx;
            hipo::column_handle h_cy;
            hipo::column_handle h_cz;
            hipo::column_handle h_vx;
            hipo::column_handle h_vy;
            hipo::column_handle h_vz;
            hipo::column_handle h_time;
            hipo::column_handle h_calID;
            hipo::column_handle h_hodoID;
            hipo::column_handle h_trk0ID;
            hipo::column_handle h_trk1ID;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("charge"); h) cache.h_charge = *h;
            else cache.h_charge = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("cx"); h) cache.h_cx = *h;
            else cache.h_cx = {};
            if (auto h = schema.get_handle("cy"); h) cache.h_cy = *h;
            else cache.h_cy = {};
            if (auto h = schema.get_handle("cz"); h) cache.h_cz = *h;
            else cache.h_cz = {};
            if (auto h = schema.get_handle("vx"); h) cache.h_vx = *h;
            else cache.h_vx = {};
            if (auto h = schema.get_handle("vy"); h) cache.h_vy = *h;
            else cache.h_vy = {};
            if (auto h = schema.get_handle("vz"); h) cache.h_vz = *h;
            else cache.h_vz = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("calID"); h) cache.h_calID = *h;
            else cache.h_calID = {};
            if (auto h = schema.get_handle("hodoID"); h) cache.h_hodoID = *h;
            else cache.h_hodoID = {};
            if (auto h = schema.get_handle("trk0ID"); h) cache.h_trk0ID = *h;
            else cache.h_trk0ID = {};
            if (auto h = schema.get_handle("trk1ID"); h) cache.h_trk1ID = *h;
            else cache.h_trk1ID = {};
        }
        h_id = cache.h_id;
        h_charge = cache.h_charge;
        h_energy = cache.h_energy;
        h_cx = cache.h_cx;
        h_cy = cache.h_cy;
        h_cz = cache.h_cz;
        h_vx = cache.h_vx;
        h_vy = cache.h_vy;
        h_vz = cache.h_vz;
        h_time = cache.h_time;
        h_calID = cache.h_calID;
        h_hodoID = cache.h_hodoID;
        h_trk0ID = cache.h_trk0ID;
        h_trk1ID = cache.h_trk1ID;
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
    /// id: track ID
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// charge: track Charge
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_charge, row));
    }
    /// energy: track Energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// cx: track Cx
    [[nodiscard]] float cx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cx, row));
    }
    /// cy: track Cy
    [[nodiscard]] float cy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cy, row));
    }
    /// cz: track Cz
    [[nodiscard]] float cz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cz, row));
    }
    /// vx: track vertex x coordinate
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vx, row));
    }
    /// vy: track vertex y coordinate
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vy, row));
    }
    /// vz: track vertex z coordinate
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }
    /// time: track Time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// calID: pointer to cluster bank
    [[nodiscard]] int calID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_calID, row));
    }
    /// hodoID: pointer to signal bank
    [[nodiscard]] int hodoID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_hodoID, row));
    }
    /// trk0ID: pointer to first cross bank
    [[nodiscard]] int trk0ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trk0ID, row));
    }
    /// trk1ID: pointer to second cross bank
    [[nodiscard]] int trk1ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trk1ID, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// charge column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> charge_col() const noexcept {
        return m_view.column<int8_t>(h_charge);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// cx column as a contiguous span.
    [[nodiscard]] std::span<const float> cx_col() const noexcept {
        return m_view.column<float>(h_cx);
    }
    /// cy column as a contiguous span.
    [[nodiscard]] std::span<const float> cy_col() const noexcept {
        return m_view.column<float>(h_cy);
    }
    /// cz column as a contiguous span.
    [[nodiscard]] std::span<const float> cz_col() const noexcept {
        return m_view.column<float>(h_cz);
    }
    /// vx column as a contiguous span.
    [[nodiscard]] std::span<const float> vx_col() const noexcept {
        return m_view.column<float>(h_vx);
    }
    /// vy column as a contiguous span.
    [[nodiscard]] std::span<const float> vy_col() const noexcept {
        return m_view.column<float>(h_vy);
    }
    /// vz column as a contiguous span.
    [[nodiscard]] std::span<const float> vz_col() const noexcept {
        return m_view.column<float>(h_vz);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// calID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> calID_col() const noexcept {
        return m_view.column<int16_t>(h_calID);
    }
    /// hodoID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> hodoID_col() const noexcept {
        return m_view.column<int16_t>(h_hodoID);
    }
    /// trk0ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trk0ID_col() const noexcept {
        return m_view.column<int16_t>(h_trk0ID);
    }
    /// trk1ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trk1ID_col() const noexcept {
        return m_view.column<int16_t>(h_trk1ID);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_charge{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_cx{};
    hipo::column_handle h_cy{};
    hipo::column_handle h_cz{};
    hipo::column_handle h_vx{};
    hipo::column_handle h_vy{};
    hipo::column_handle h_vz{};
    hipo::column_handle h_time{};
    hipo::column_handle h_calID{};
    hipo::column_handle h_hodoID{};
    hipo::column_handle h_trk0ID{};
    hipo::column_handle h_trk1ID{};

    friend class Row;
};

/// Lightweight row view for FT::particles. Stores a pointer + row index only.
class FT_particles::Row {
 public:
    Row(const FT_particles* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float cx() const noexcept { return m_bank->cx(m_row); }
    [[nodiscard]] float cy() const noexcept { return m_bank->cy(m_row); }
    [[nodiscard]] float cz() const noexcept { return m_bank->cz(m_row); }
    [[nodiscard]] float vx() const noexcept { return m_bank->vx(m_row); }
    [[nodiscard]] float vy() const noexcept { return m_bank->vy(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int calID() const noexcept { return m_bank->calID(m_row); }
    [[nodiscard]] int hodoID() const noexcept { return m_bank->hodoID(m_row); }
    [[nodiscard]] int trk0ID() const noexcept { return m_bank->trk0ID(m_row); }
    [[nodiscard]] int trk1ID() const noexcept { return m_bank->trk1ID(m_row); }
 private:
    const FT_particles* m_bank;
    int m_row;
};

inline FT_particles::Row FT_particles::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
