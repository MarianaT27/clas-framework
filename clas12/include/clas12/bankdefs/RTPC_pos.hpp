// =============================================================================
// Auto-generated from data.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RTPC::pos` (group 21900, item 14).
/// Digitized bank for the Radial TPC
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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

    /// Bind to a live hipo::bank. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column name to its item
    /// index via schema.getEntryOrder(); subsequent bind()s for the same
    /// schema hit the cache. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            const hipo::schema* schema_ptr = nullptr;
            int i_step = -1;
            int i_time = -1;
            int i_energy = -1;
            int i_posx = -1;
            int i_posy = -1;
            int i_posz = -1;
            int i_phi = -1;
            int i_tid = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_step = schema.exists("step") ? schema.getEntryOrder("step") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_posx = schema.exists("posx") ? schema.getEntryOrder("posx") : -1;
            cache.i_posy = schema.exists("posy") ? schema.getEntryOrder("posy") : -1;
            cache.i_posz = schema.exists("posz") ? schema.getEntryOrder("posz") : -1;
            cache.i_phi = schema.exists("phi") ? schema.getEntryOrder("phi") : -1;
            cache.i_tid = schema.exists("tid") ? schema.getEntryOrder("tid") : -1;
        }
        i_step = cache.i_step;
        i_time = cache.i_time;
        i_energy = cache.i_energy;
        i_posx = cache.i_posx;
        i_posy = cache.i_posy;
        i_posz = cache.i_posz;
        i_phi = cache.i_phi;
        i_tid = cache.i_tid;
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
    /// step: step
    [[nodiscard]] int step(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_step, row));
    }
    /// time: time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// energy: energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// posx: x position (mm)
    [[nodiscard]] float posx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_posx, row));
    }
    /// posy: y position (mm)
    [[nodiscard]] float posy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_posy, row));
    }
    /// posz: z position (mm)
    [[nodiscard]] float posz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_posz, row));
    }
    /// phi: phi (rad)
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_phi, row));
    }
    /// tid: track id
    [[nodiscard]] int tid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tid, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_step = -1;
    int i_time = -1;
    int i_energy = -1;
    int i_posx = -1;
    int i_posy = -1;
    int i_posz = -1;
    int i_phi = -1;
    int i_tid = -1;

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
