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


/// HIPO bank wrapper for `AHDC::mc` (group 23000, item 22).
/// MC Tracks
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// AHDC_mc bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::mc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto x = row.x();
/// }
/// @endcode
class AHDC_mc {
 public:
    static constexpr std::string_view bank_name = "AHDC::mc";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 22;

    class Row;

    AHDC_mc() = default;

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
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
        }
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
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
    /// x: x position in mm
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y position in mm
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: z position in mm
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// px: px in MeV
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: py in MeV
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: pz in MeV
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;

    friend class Row;
};

/// Lightweight row view for AHDC::mc. Stores a pointer + row index only.
class AHDC_mc::Row {
 public:
    Row(const AHDC_mc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
 private:
    const AHDC_mc* m_bank;
    int m_row;
};

inline AHDC_mc::Row AHDC_mc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
