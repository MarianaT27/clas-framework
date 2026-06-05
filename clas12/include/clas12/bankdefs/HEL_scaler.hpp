// =============================================================================
// Auto-generated from header.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `HEL::scaler` (group 10000, item 16).
/// Helicity-gated, analyzed scalers.
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// HEL_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto fcupgated = row.fcupgated();
/// }
/// @endcode
class HEL_scaler {
 public:
    static constexpr std::string_view bank_name = "HEL::scaler";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 16;
    static constexpr std::string_view bank_schema = "fcupgated/F,fcup/F,slmgated/F,slm/F,clockgated/F,clock/F,helicity/B,helicityRaw/B";

    class Row;

    HEL_scaler() = default;

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
            int i_fcupgated = -1;
            int i_fcup = -1;
            int i_slmgated = -1;
            int i_slm = -1;
            int i_clockgated = -1;
            int i_clock = -1;
            int i_helicity = -1;
            int i_helicityRaw = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_fcupgated = schema.exists("fcupgated") ? schema.getEntryOrder("fcupgated") : -1;
            cache.i_fcup = schema.exists("fcup") ? schema.getEntryOrder("fcup") : -1;
            cache.i_slmgated = schema.exists("slmgated") ? schema.getEntryOrder("slmgated") : -1;
            cache.i_slm = schema.exists("slm") ? schema.getEntryOrder("slm") : -1;
            cache.i_clockgated = schema.exists("clockgated") ? schema.getEntryOrder("clockgated") : -1;
            cache.i_clock = schema.exists("clock") ? schema.getEntryOrder("clock") : -1;
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
            cache.i_helicityRaw = schema.exists("helicityRaw") ? schema.getEntryOrder("helicityRaw") : -1;
        }
        i_fcupgated = cache.i_fcupgated;
        i_fcup = cache.i_fcup;
        i_slmgated = cache.i_slmgated;
        i_slm = cache.i_slm;
        i_clockgated = cache.i_clockgated;
        i_clock = cache.i_clock;
        i_helicity = cache.i_helicity;
        i_helicityRaw = cache.i_helicityRaw;
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
    /// fcupgated: Beam charge from Faraday cup, DAQ-gated (nano-Coulomb)
    [[nodiscard]] float fcupgated(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fcupgated, row));
    }
    /// fcup: Beam charge from Faraday cup, (nano-Coulomb)
    [[nodiscard]] float fcup(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fcup, row));
    }
    /// slmgated: Beam charge from SLM, DAQ-gated (nano-Coulomb)
    [[nodiscard]] float slmgated(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_slmgated, row));
    }
    /// slm: Beam charge from SLM (nano-Coulomb)
    [[nodiscard]] float slm(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_slm, row));
    }
    /// clockgated: Clock, DAQ-gated
    [[nodiscard]] float clockgated(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_clockgated, row));
    }
    /// clock: Clock
    [[nodiscard]] float clock(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_clock, row));
    }
    /// helicity: Helicity state, HWP-corrected
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicity, row));
    }
    /// helicityRaw: Helicity state
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicityRaw, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_fcupgated = -1;
    int i_fcup = -1;
    int i_slmgated = -1;
    int i_slm = -1;
    int i_clockgated = -1;
    int i_clock = -1;
    int i_helicity = -1;
    int i_helicityRaw = -1;

    friend class Row;
};

/// Lightweight row view for HEL::scaler. Stores a pointer + row index only.
class HEL_scaler::Row {
 public:
    Row(const HEL_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float fcupgated() const noexcept { return m_bank->fcupgated(m_row); }
    [[nodiscard]] float fcup() const noexcept { return m_bank->fcup(m_row); }
    [[nodiscard]] float slmgated() const noexcept { return m_bank->slmgated(m_row); }
    [[nodiscard]] float slm() const noexcept { return m_bank->slm(m_row); }
    [[nodiscard]] float clockgated() const noexcept { return m_bank->clockgated(m_row); }
    [[nodiscard]] float clock() const noexcept { return m_bank->clock(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int helicityRaw() const noexcept { return m_bank->helicityRaw(m_row); }
 private:
    const HEL_scaler* m_bank;
    int m_row;
};

inline HEL_scaler::Row HEL_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
