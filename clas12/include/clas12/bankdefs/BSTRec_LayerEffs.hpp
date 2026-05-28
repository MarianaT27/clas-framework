// =============================================================================
// Auto-generated from bst.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `BSTRec::LayerEffs` (group 20200, item 29).
/// layer efficiencies
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// BSTRec_LayerEffs bank;
/// if (auto sc = bank.bind(event.get_bank("BSTRec::LayerEffs")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class BSTRec_LayerEffs {
 public:
    static constexpr std::string_view bank_name = "BSTRec::LayerEffs";
    static constexpr int bank_group = 20200;
    static constexpr int bank_item  = 29;

    class Row;

    BSTRec_LayerEffs() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_residual = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_residual = schema.exists("residual") ? schema.getEntryOrder("residual") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_residual = cache.i_residual;
        i_status = cache.i_status;
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
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// residual: excluded layer residual of the matched cluster centroid
    [[nodiscard]] float residual(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_residual, row));
    }
    /// status: status (-1: not used, i.e. no track; 0: inefficient; 1: efficient
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_sector = -1;
    int i_layer = -1;
    int i_residual = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for BSTRec::LayerEffs. Stores a pointer + row index only.
class BSTRec_LayerEffs::Row {
 public:
    Row(const BSTRec_LayerEffs* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float residual() const noexcept { return m_bank->residual(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const BSTRec_LayerEffs* m_bank;
    int m_row;
};

inline BSTRec_LayerEffs::Row BSTRec_LayerEffs::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
