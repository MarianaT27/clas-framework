// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `MC::Event` (group 40, item 1).
/// Lund header bank for the generated event
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// MC_Event bank;
/// if (auto sc = bank.bind(event.get_bank("MC::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto npart = row.npart();
/// }
/// @endcode
class MC_Event {
 public:
    static constexpr std::string_view bank_name = "MC::Event";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 1;

    class Row;

    MC_Event() = default;

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
            int i_npart = -1;
            int i_atarget = -1;
            int i_ztarget = -1;
            int i_ptarget = -1;
            int i_pbeam = -1;
            int i_btype = -1;
            int i_ebeam = -1;
            int i_targetid = -1;
            int i_processid = -1;
            int i_weight = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_npart = schema.exists("npart") ? schema.getEntryOrder("npart") : -1;
            cache.i_atarget = schema.exists("atarget") ? schema.getEntryOrder("atarget") : -1;
            cache.i_ztarget = schema.exists("ztarget") ? schema.getEntryOrder("ztarget") : -1;
            cache.i_ptarget = schema.exists("ptarget") ? schema.getEntryOrder("ptarget") : -1;
            cache.i_pbeam = schema.exists("pbeam") ? schema.getEntryOrder("pbeam") : -1;
            cache.i_btype = schema.exists("btype") ? schema.getEntryOrder("btype") : -1;
            cache.i_ebeam = schema.exists("ebeam") ? schema.getEntryOrder("ebeam") : -1;
            cache.i_targetid = schema.exists("targetid") ? schema.getEntryOrder("targetid") : -1;
            cache.i_processid = schema.exists("processid") ? schema.getEntryOrder("processid") : -1;
            cache.i_weight = schema.exists("weight") ? schema.getEntryOrder("weight") : -1;
        }
        i_npart = cache.i_npart;
        i_atarget = cache.i_atarget;
        i_ztarget = cache.i_ztarget;
        i_ptarget = cache.i_ptarget;
        i_pbeam = cache.i_pbeam;
        i_btype = cache.i_btype;
        i_ebeam = cache.i_ebeam;
        i_targetid = cache.i_targetid;
        i_processid = cache.i_processid;
        i_weight = cache.i_weight;
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
    /// npart: number of particles in the event (User Defined)
    [[nodiscard]] int npart(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_npart, row));
    }
    /// atarget: Mass number of the target (User Defined)
    [[nodiscard]] int atarget(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_atarget, row));
    }
    /// ztarget: Atomic number oif the target (User Defined)
    [[nodiscard]] int ztarget(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ztarget, row));
    }
    /// ptarget: Target polarization (User Defined)
    [[nodiscard]] float ptarget(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ptarget, row));
    }
    /// pbeam: Beam polarization
    [[nodiscard]] float pbeam(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pbeam, row));
    }
    /// btype: Beam type (User Defined)
    [[nodiscard]] int btype(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_btype, row));
    }
    /// ebeam: Beam energy (GeV) (User Defined)
    [[nodiscard]] float ebeam(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ebeam, row));
    }
    /// targetid: Interacted nucleon ID (proton=2212, neutron=2112 (User Defined)
    [[nodiscard]] int targetid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_targetid, row));
    }
    /// processid: Process ID (User Defined)
    [[nodiscard]] int processid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_processid, row));
    }
    /// weight: Event weight (User Defined)
    [[nodiscard]] float weight(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_weight, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_npart = -1;
    int i_atarget = -1;
    int i_ztarget = -1;
    int i_ptarget = -1;
    int i_pbeam = -1;
    int i_btype = -1;
    int i_ebeam = -1;
    int i_targetid = -1;
    int i_processid = -1;
    int i_weight = -1;

    friend class Row;
};

/// Lightweight row view for MC::Event. Stores a pointer + row index only.
class MC_Event::Row {
 public:
    Row(const MC_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int npart() const noexcept { return m_bank->npart(m_row); }
    [[nodiscard]] int atarget() const noexcept { return m_bank->atarget(m_row); }
    [[nodiscard]] int ztarget() const noexcept { return m_bank->ztarget(m_row); }
    [[nodiscard]] float ptarget() const noexcept { return m_bank->ptarget(m_row); }
    [[nodiscard]] float pbeam() const noexcept { return m_bank->pbeam(m_row); }
    [[nodiscard]] int btype() const noexcept { return m_bank->btype(m_row); }
    [[nodiscard]] float ebeam() const noexcept { return m_bank->ebeam(m_row); }
    [[nodiscard]] int targetid() const noexcept { return m_bank->targetid(m_row); }
    [[nodiscard]] int processid() const noexcept { return m_bank->processid(m_row); }
    [[nodiscard]] float weight() const noexcept { return m_bank->weight(m_row); }
 private:
    const MC_Event* m_bank;
    int m_row;
};

inline MC_Event::Row MC_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
