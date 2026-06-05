// =============================================================================
// Auto-generated from ft.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `FT::particles` (group 20900, item 24).
/// Reconstructed Particles in FT
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
    static constexpr std::string_view bank_schema = "id/S,charge/B,energy/F,cx/F,cy/F,cz/F,vx/F,vy/F,vz/F,time/F,calID/S,hodoID/S,trk0ID/S,trk1ID/S";

    class Row;

    FT_particles() = default;

    /// Bind to a live hipo::bank. Per-thread cache keyed by the schema's
    /// (group, item): the first bind() in a thread resolves every column
    /// name to its item index via schema.getEntryOrder(); subsequent
    /// bind()s hit the cache. Keying by (group,item) rather than the schema
    /// POINTER is critical: the framework constructs a fresh hipo::bank
    /// (copying its schema to a new address) every event, so a pointer key
    /// would miss every event and re-resolve all columns by string. (group,
    /// item) is stable across those copies, so resolution is paid once per
    /// thread per bank type. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            int gi_group = -1;
            int gi_item  = -2;
            int i_id = -1;
            int i_charge = -1;
            int i_energy = -1;
            int i_cx = -1;
            int i_cy = -1;
            int i_cz = -1;
            int i_vx = -1;
            int i_vy = -1;
            int i_vz = -1;
            int i_time = -1;
            int i_calID = -1;
            int i_hodoID = -1;
            int i_trk0ID = -1;
            int i_trk1ID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_charge = schema.exists("charge") ? schema.getEntryOrder("charge") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_cx = schema.exists("cx") ? schema.getEntryOrder("cx") : -1;
            cache.i_cy = schema.exists("cy") ? schema.getEntryOrder("cy") : -1;
            cache.i_cz = schema.exists("cz") ? schema.getEntryOrder("cz") : -1;
            cache.i_vx = schema.exists("vx") ? schema.getEntryOrder("vx") : -1;
            cache.i_vy = schema.exists("vy") ? schema.getEntryOrder("vy") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_calID = schema.exists("calID") ? schema.getEntryOrder("calID") : -1;
            cache.i_hodoID = schema.exists("hodoID") ? schema.getEntryOrder("hodoID") : -1;
            cache.i_trk0ID = schema.exists("trk0ID") ? schema.getEntryOrder("trk0ID") : -1;
            cache.i_trk1ID = schema.exists("trk1ID") ? schema.getEntryOrder("trk1ID") : -1;
        }
        i_id = cache.i_id;
        i_charge = cache.i_charge;
        i_energy = cache.i_energy;
        i_cx = cache.i_cx;
        i_cy = cache.i_cy;
        i_cz = cache.i_cz;
        i_vx = cache.i_vx;
        i_vy = cache.i_vy;
        i_vz = cache.i_vz;
        i_time = cache.i_time;
        i_calID = cache.i_calID;
        i_hodoID = cache.i_hodoID;
        i_trk0ID = cache.i_trk0ID;
        i_trk1ID = cache.i_trk1ID;
        return true;
    }

    /// Current row count of the bound bank (0 if unbound).
    [[nodiscard]] int rows() const noexcept { return m_view ? m_view->getRows() : 0; }

    /// True if bind() has been called.
    [[nodiscard]] bool valid() const noexcept { return m_view != nullptr; }

    /// Access the underlying hipo::bank (for advanced use).
    [[nodiscard]] const hipo::bank& view() const noexcept { return *m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (item-index based, no string lookup)
    // -------------------------------------------------------------------
    /// id: track ID
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// charge: track Charge
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_charge, row));
    }
    /// energy: track Energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// cx: track Cx
    [[nodiscard]] float cx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cx, row));
    }
    /// cy: track Cy
    [[nodiscard]] float cy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cy, row));
    }
    /// cz: track Cz
    [[nodiscard]] float cz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cz, row));
    }
    /// vx: track vertex x coordinate
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vx, row));
    }
    /// vy: track vertex y coordinate
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vy, row));
    }
    /// vz: track vertex z coordinate
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }
    /// time: track Time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// calID: pointer to cluster bank
    [[nodiscard]] int calID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_calID, row));
    }
    /// hodoID: pointer to signal bank
    [[nodiscard]] int hodoID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_hodoID, row));
    }
    /// trk0ID: pointer to first cross bank
    [[nodiscard]] int trk0ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trk0ID, row));
    }
    /// trk1ID: pointer to second cross bank
    [[nodiscard]] int trk1ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trk1ID, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_charge = -1;
    int i_energy = -1;
    int i_cx = -1;
    int i_cy = -1;
    int i_cz = -1;
    int i_vx = -1;
    int i_vy = -1;
    int i_vz = -1;
    int i_time = -1;
    int i_calID = -1;
    int i_hodoID = -1;
    int i_trk0ID = -1;
    int i_trk1ID = -1;

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
