// =============================================================================
// Auto-generated from pi0skim.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `PI0::event` (group 30000, item 1).
/// Slim pi0 skim: per-event DIS kinematics + trigger electron 4-vector
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// PI0_event bank;
/// if (auto sc = bank.bind(event.get_bank("PI0::event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto run = row.run();
/// }
/// @endcode
class PI0_event {
 public:
    static constexpr std::string_view bank_name = "PI0::event";
    static constexpr int bank_group = 30000;
    static constexpr int bank_item  = 1;
    static constexpr std::string_view bank_schema = "run/I,event/L,helicity/I,Q2/F,xB/F,nu/F,W/F,y/F,ex/F,ey/F,ez/F,eE/F";

    class Row;

    PI0_event() = default;

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
            int i_run = -1;
            int i_event = -1;
            int i_helicity = -1;
            int i_Q2 = -1;
            int i_xB = -1;
            int i_nu = -1;
            int i_W = -1;
            int i_y = -1;
            int i_ex = -1;
            int i_ey = -1;
            int i_ez = -1;
            int i_eE = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_run = schema.exists("run") ? schema.getEntryOrder("run") : -1;
            cache.i_event = schema.exists("event") ? schema.getEntryOrder("event") : -1;
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
            cache.i_Q2 = schema.exists("Q2") ? schema.getEntryOrder("Q2") : -1;
            cache.i_xB = schema.exists("xB") ? schema.getEntryOrder("xB") : -1;
            cache.i_nu = schema.exists("nu") ? schema.getEntryOrder("nu") : -1;
            cache.i_W = schema.exists("W") ? schema.getEntryOrder("W") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_ex = schema.exists("ex") ? schema.getEntryOrder("ex") : -1;
            cache.i_ey = schema.exists("ey") ? schema.getEntryOrder("ey") : -1;
            cache.i_ez = schema.exists("ez") ? schema.getEntryOrder("ez") : -1;
            cache.i_eE = schema.exists("eE") ? schema.getEntryOrder("eE") : -1;
        }
        i_run = cache.i_run;
        i_event = cache.i_event;
        i_helicity = cache.i_helicity;
        i_Q2 = cache.i_Q2;
        i_xB = cache.i_xB;
        i_nu = cache.i_nu;
        i_W = cache.i_W;
        i_y = cache.i_y;
        i_ex = cache.i_ex;
        i_ey = cache.i_ey;
        i_ez = cache.i_ez;
        i_eE = cache.i_eE;
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
    /// run: run number
    [[nodiscard]] int run(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_run, row));
    }
    /// event: event number
    [[nodiscard]] int64_t event(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_event, row));
    }
    /// helicity: beam helicity (HWP-corrected, +/-1)
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_helicity, row));
    }
    /// Q2: DIS Q^2 (GeV^2)
    [[nodiscard]] float Q2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_Q2, row));
    }
    /// xB: DIS Bjorken x
    [[nodiscard]] float xB(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_xB, row));
    }
    /// nu: DIS nu (GeV)
    [[nodiscard]] float nu(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_nu, row));
    }
    /// W: DIS W (GeV)
    [[nodiscard]] float W(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_W, row));
    }
    /// y: DIS inelasticity y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// ex: trigger electron px (GeV)
    [[nodiscard]] float ex(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ex, row));
    }
    /// ey: trigger electron py (GeV)
    [[nodiscard]] float ey(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ey, row));
    }
    /// ez: trigger electron pz (GeV)
    [[nodiscard]] float ez(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ez, row));
    }
    /// eE: trigger electron E (GeV)
    [[nodiscard]] float eE(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_eE, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_run = -1;
    int i_event = -1;
    int i_helicity = -1;
    int i_Q2 = -1;
    int i_xB = -1;
    int i_nu = -1;
    int i_W = -1;
    int i_y = -1;
    int i_ex = -1;
    int i_ey = -1;
    int i_ez = -1;
    int i_eE = -1;

    friend class Row;
};

/// Lightweight row view for PI0::event. Stores a pointer + row index only.
class PI0_event::Row {
 public:
    Row(const PI0_event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int run() const noexcept { return m_bank->run(m_row); }
    [[nodiscard]] int64_t event() const noexcept { return m_bank->event(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] float Q2() const noexcept { return m_bank->Q2(m_row); }
    [[nodiscard]] float xB() const noexcept { return m_bank->xB(m_row); }
    [[nodiscard]] float nu() const noexcept { return m_bank->nu(m_row); }
    [[nodiscard]] float W() const noexcept { return m_bank->W(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float ex() const noexcept { return m_bank->ex(m_row); }
    [[nodiscard]] float ey() const noexcept { return m_bank->ey(m_row); }
    [[nodiscard]] float ez() const noexcept { return m_bank->ez(m_row); }
    [[nodiscard]] float eE() const noexcept { return m_bank->eE(m_row); }
 private:
    const PI0_event* m_bank;
    int m_row;
};

inline PI0_event::Row PI0_event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
