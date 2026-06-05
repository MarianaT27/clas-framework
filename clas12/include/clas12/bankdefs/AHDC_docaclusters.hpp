// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `AHDC::docaclusters` (group 23000, item 126).
/// Doca-refined cluster space points
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// AHDC_docaclusters bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::docaclusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto x = row.x();
/// }
/// @endcode
class AHDC_docaclusters {
 public:
    static constexpr std::string_view bank_name = "AHDC::docaclusters";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 126;
    static constexpr std::string_view bank_schema = "x/F,y/F,z/F,weight/F,pattern/I,idx/I";

    class Row;

    AHDC_docaclusters() = default;

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
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_weight = -1;
            int i_pattern = -1;
            int i_idx = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_weight = schema.exists("weight") ? schema.getEntryOrder("weight") : -1;
            cache.i_pattern = schema.exists("pattern") ? schema.getEntryOrder("pattern") : -1;
            cache.i_idx = schema.exists("idx") ? schema.getEntryOrder("idx") : -1;
        }
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_weight = cache.i_weight;
        i_pattern = cache.i_pattern;
        i_idx = cache.i_idx;
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
    /// x: refined x (mm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: refined y (mm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: refined z (mm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// weight: refined point weight
    [[nodiscard]] float weight(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_weight, row));
    }
    /// pattern: hit pattern: 11,12,21,22
    [[nodiscard]] int pattern(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pattern, row));
    }
    /// idx: index of original AHDC::clusters row
    [[nodiscard]] int idx(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_idx, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_weight = -1;
    int i_pattern = -1;
    int i_idx = -1;

    friend class Row;
};

/// Lightweight row view for AHDC::docaclusters. Stores a pointer + row index only.
class AHDC_docaclusters::Row {
 public:
    Row(const AHDC_docaclusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float weight() const noexcept { return m_bank->weight(m_row); }
    [[nodiscard]] int pattern() const noexcept { return m_bank->pattern(m_row); }
    [[nodiscard]] int idx() const noexcept { return m_bank->idx(m_row); }
 private:
    const AHDC_docaclusters* m_bank;
    int m_row;
};

inline AHDC_docaclusters::Row AHDC_docaclusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
