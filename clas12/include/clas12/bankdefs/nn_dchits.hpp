// =============================================================================
// Auto-generated from neuralnetwork.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `nn::dchits` (group 4200, item 2).
/// index if the track hits in DC tdc bank.
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// nn_dchits bank;
/// if (auto sc = bank.bind(event.get_bank("nn::dchits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class nn_dchits {
 public:
    static constexpr std::string_view bank_name = "nn::dchits";
    static constexpr int bank_group = 4200;
    static constexpr int bank_item  = 2;

    class Row;

    nn_dchits() = default;

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
            int i_index = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
        }
        i_id = cache.i_id;
        i_index = cache.i_index;
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
    /// id: track id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_id, row));
    }
    /// index: index of the row for DC::tdc bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int index_handle() const noexcept { return i_index; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_index = -1;

    friend class Row;
};

/// Lightweight row view for nn::dchits. Stores a pointer + row index only.
class nn_dchits::Row {
 public:
    Row(const nn_dchits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
 private:
    const nn_dchits* m_bank;
    int m_row;
};

inline nn_dchits::Row nn_dchits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
