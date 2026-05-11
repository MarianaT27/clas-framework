// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RECAI::VertDoca` (group 400, item 39).
/// Track Cross information for Particles bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECAI_VertDoca bank;
/// if (auto sc = bank.bind(event.get_bank("RECAI::VertDoca")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index1 = row.index1();
/// }
/// @endcode
class RECAI_VertDoca {
 public:
    static constexpr std::string_view bank_name = "RECAI::VertDoca";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 39;

    class Row;

    RECAI_VertDoca() = default;

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
            int i_index1 = -1;
            int i_index2 = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_x1 = -1;
            int i_y1 = -1;
            int i_z1 = -1;
            int i_cx1 = -1;
            int i_cy1 = -1;
            int i_cz1 = -1;
            int i_x2 = -1;
            int i_y2 = -1;
            int i_z2 = -1;
            int i_cx2 = -1;
            int i_cy2 = -1;
            int i_cz2 = -1;
            int i_r = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_index1 = schema.exists("index1") ? schema.getEntryOrder("index1") : -1;
            cache.i_index2 = schema.exists("index2") ? schema.getEntryOrder("index2") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_x1 = schema.exists("x1") ? schema.getEntryOrder("x1") : -1;
            cache.i_y1 = schema.exists("y1") ? schema.getEntryOrder("y1") : -1;
            cache.i_z1 = schema.exists("z1") ? schema.getEntryOrder("z1") : -1;
            cache.i_cx1 = schema.exists("cx1") ? schema.getEntryOrder("cx1") : -1;
            cache.i_cy1 = schema.exists("cy1") ? schema.getEntryOrder("cy1") : -1;
            cache.i_cz1 = schema.exists("cz1") ? schema.getEntryOrder("cz1") : -1;
            cache.i_x2 = schema.exists("x2") ? schema.getEntryOrder("x2") : -1;
            cache.i_y2 = schema.exists("y2") ? schema.getEntryOrder("y2") : -1;
            cache.i_z2 = schema.exists("z2") ? schema.getEntryOrder("z2") : -1;
            cache.i_cx2 = schema.exists("cx2") ? schema.getEntryOrder("cx2") : -1;
            cache.i_cy2 = schema.exists("cy2") ? schema.getEntryOrder("cy2") : -1;
            cache.i_cz2 = schema.exists("cz2") ? schema.getEntryOrder("cz2") : -1;
            cache.i_r = schema.exists("r") ? schema.getEntryOrder("r") : -1;
        }
        i_index1 = cache.i_index1;
        i_index2 = cache.i_index2;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_x1 = cache.i_x1;
        i_y1 = cache.i_y1;
        i_z1 = cache.i_z1;
        i_cx1 = cache.i_cx1;
        i_cy1 = cache.i_cy1;
        i_cz1 = cache.i_cz1;
        i_x2 = cache.i_x2;
        i_y2 = cache.i_y2;
        i_z2 = cache.i_z2;
        i_cx2 = cache.i_cx2;
        i_cy2 = cache.i_cy2;
        i_cz2 = cache.i_cz2;
        i_r = cache.i_r;
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
    /// index1: index of the first track in Tracks bank
    [[nodiscard]] int index1(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index1, row));
    }
    /// index2: index of the second track in Tracks bank
    [[nodiscard]] int index2(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index2, row));
    }
    /// x: x-position of the common vertex (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y-position of the common vertex (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: z-position of the common vertex (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// x1: x-position of the first track at the DOCA point (cm)
    [[nodiscard]] float x1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x1, row));
    }
    /// y1: y-position of the first track at the DOCA point (cm)
    [[nodiscard]] float y1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y1, row));
    }
    /// z1: z-position of the first track at the DOCA point (cm)
    [[nodiscard]] float z1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z1, row));
    }
    /// cx1: x-direction vector of the first track at the DOCA point (cm)
    [[nodiscard]] float cx1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cx1, row));
    }
    /// cy1: y-direction vector of the first track at the DOCA point (cm)
    [[nodiscard]] float cy1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cy1, row));
    }
    /// cz1: z-direction vector of the first track at the DOCA point (cm)
    [[nodiscard]] float cz1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cz1, row));
    }
    /// x2: x-position of the second track at the DOCA point (cm)
    [[nodiscard]] float x2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x2, row));
    }
    /// y2: y-position of the second track at the DOCA point (cm)
    [[nodiscard]] float y2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y2, row));
    }
    /// z2: z-position of the second track at the DOCA point (cm)
    [[nodiscard]] float z2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z2, row));
    }
    /// cx2: x-direction vector of the second track at the DOCA point (cm)
    [[nodiscard]] float cx2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cx2, row));
    }
    /// cy2: y-direction vector of the second track at the DOCA point (cm)
    [[nodiscard]] float cy2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cy2, row));
    }
    /// cz2: z-direction vector of the second track at the DOCA point (cm)
    [[nodiscard]] float cz2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_cz2, row));
    }
    /// r: The distance between two tracks (cm)
    [[nodiscard]] float r(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_r, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_index1 = -1;
    int i_index2 = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_x1 = -1;
    int i_y1 = -1;
    int i_z1 = -1;
    int i_cx1 = -1;
    int i_cy1 = -1;
    int i_cz1 = -1;
    int i_x2 = -1;
    int i_y2 = -1;
    int i_z2 = -1;
    int i_cx2 = -1;
    int i_cy2 = -1;
    int i_cz2 = -1;
    int i_r = -1;

    friend class Row;
};

/// Lightweight row view for RECAI::VertDoca. Stores a pointer + row index only.
class RECAI_VertDoca::Row {
 public:
    Row(const RECAI_VertDoca* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index1() const noexcept { return m_bank->index1(m_row); }
    [[nodiscard]] int index2() const noexcept { return m_bank->index2(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float x1() const noexcept { return m_bank->x1(m_row); }
    [[nodiscard]] float y1() const noexcept { return m_bank->y1(m_row); }
    [[nodiscard]] float z1() const noexcept { return m_bank->z1(m_row); }
    [[nodiscard]] float cx1() const noexcept { return m_bank->cx1(m_row); }
    [[nodiscard]] float cy1() const noexcept { return m_bank->cy1(m_row); }
    [[nodiscard]] float cz1() const noexcept { return m_bank->cz1(m_row); }
    [[nodiscard]] float x2() const noexcept { return m_bank->x2(m_row); }
    [[nodiscard]] float y2() const noexcept { return m_bank->y2(m_row); }
    [[nodiscard]] float z2() const noexcept { return m_bank->z2(m_row); }
    [[nodiscard]] float cx2() const noexcept { return m_bank->cx2(m_row); }
    [[nodiscard]] float cy2() const noexcept { return m_bank->cy2(m_row); }
    [[nodiscard]] float cz2() const noexcept { return m_bank->cz2(m_row); }
    [[nodiscard]] float r() const noexcept { return m_bank->r(m_row); }
 private:
    const RECAI_VertDoca* m_bank;
    int m_row;
};

inline RECAI_VertDoca::Row RECAI_VertDoca::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
