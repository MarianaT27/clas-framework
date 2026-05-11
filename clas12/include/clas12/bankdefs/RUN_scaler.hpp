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


/// HIPO bank wrapper for `RUN::scaler` (group 10000, item 14).
/// Run-integrated, analyzed scalers.
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RUN_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("RUN::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto fcupgated = row.fcupgated();
/// }
/// @endcode
class RUN_scaler {
 public:
    static constexpr std::string_view bank_name = "RUN::scaler";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 14;

    class Row;

    RUN_scaler() = default;

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
            int i_fcupgated = -1;
            int i_fcup = -1;
            int i_livetime = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_fcupgated = schema.exists("fcupgated") ? schema.getEntryOrder("fcupgated") : -1;
            cache.i_fcup = schema.exists("fcup") ? schema.getEntryOrder("fcup") : -1;
            cache.i_livetime = schema.exists("livetime") ? schema.getEntryOrder("livetime") : -1;
        }
        i_fcupgated = cache.i_fcupgated;
        i_fcup = cache.i_fcup;
        i_livetime = cache.i_livetime;
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
    /// fcupgated: Beam charge, integrated from beginning of run, DAQ-gated (nano-Coulomb)
    [[nodiscard]] float fcupgated(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fcupgated, row));
    }
    /// fcup: Beam charge, integrated from beginning of run, ungated (nano-Coulomb)
    [[nodiscard]] float fcup(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_fcup, row));
    }
    /// livetime: DAQ livetime during one scaler period
    [[nodiscard]] float livetime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_livetime, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_fcupgated = -1;
    int i_fcup = -1;
    int i_livetime = -1;

    friend class Row;
};

/// Lightweight row view for RUN::scaler. Stores a pointer + row index only.
class RUN_scaler::Row {
 public:
    Row(const RUN_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float fcupgated() const noexcept { return m_bank->fcupgated(m_row); }
    [[nodiscard]] float fcup() const noexcept { return m_bank->fcup(m_row); }
    [[nodiscard]] float livetime() const noexcept { return m_bank->livetime(m_row); }
 private:
    const RUN_scaler* m_bank;
    int m_row;
};

inline RUN_scaler::Row RUN_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
