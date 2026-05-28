// =============================================================================
// Auto-generated from event.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `REC::ScintExtras` (group 300, item 43).
/// Scintillator Cluster Parameters
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// REC_ScintExtras bank;
/// if (auto sc = bank.bind(event.get_bank("REC::ScintExtras")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto dedx = row.dedx();
/// }
/// @endcode
class REC_ScintExtras {
 public:
    static constexpr std::string_view bank_name = "REC::ScintExtras";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 43;

    class Row;

    REC_ScintExtras() = default;

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
            int i_dedx = -1;
            int i_size = -1;
            int i_layermult = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_dedx = schema.exists("dedx") ? schema.getEntryOrder("dedx") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_layermult = schema.exists("layermult") ? schema.getEntryOrder("layermult") : -1;
        }
        i_dedx = cache.i_dedx;
        i_size = cache.i_size;
        i_layermult = cache.i_layermult;
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
    /// dedx: Energy deposition per distance travelled (MeV/cm)
    [[nodiscard]] float dedx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dedx, row));
    }
    /// size: cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// layermult: cluster layer multiplicity
    [[nodiscard]] int layermult(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layermult, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_dedx = -1;
    int i_size = -1;
    int i_layermult = -1;

    friend class Row;
};

/// Lightweight row view for REC::ScintExtras. Stores a pointer + row index only.
class REC_ScintExtras::Row {
 public:
    Row(const REC_ScintExtras* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float dedx() const noexcept { return m_bank->dedx(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int layermult() const noexcept { return m_bank->layermult(m_row); }
 private:
    const REC_ScintExtras* m_bank;
    int m_row;
};

inline REC_ScintExtras::Row REC_ScintExtras::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
