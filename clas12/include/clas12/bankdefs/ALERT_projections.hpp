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


/// HIPO bank wrapper for `ALERT::projections` (group 23000, item 31).
/// Track Projections to ATOF
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ALERT_projections bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::projections")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ALERT_projections {
 public:
    static constexpr std::string_view bank_name = "ALERT::projections";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 31;

    class Row;

    ALERT_projections() = default;

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
            int i_trackid = -1;
            int i_x_at_bar = -1;
            int i_y_at_bar = -1;
            int i_z_at_bar = -1;
            int i_l_at_bar = -1;
            int i_l_in_bar = -1;
            int i_x_at_wedge = -1;
            int i_y_at_wedge = -1;
            int i_z_at_wedge = -1;
            int i_l_at_wedge = -1;
            int i_l_in_wedge = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
            cache.i_x_at_bar = schema.exists("x_at_bar") ? schema.getEntryOrder("x_at_bar") : -1;
            cache.i_y_at_bar = schema.exists("y_at_bar") ? schema.getEntryOrder("y_at_bar") : -1;
            cache.i_z_at_bar = schema.exists("z_at_bar") ? schema.getEntryOrder("z_at_bar") : -1;
            cache.i_l_at_bar = schema.exists("l_at_bar") ? schema.getEntryOrder("l_at_bar") : -1;
            cache.i_l_in_bar = schema.exists("l_in_bar") ? schema.getEntryOrder("l_in_bar") : -1;
            cache.i_x_at_wedge = schema.exists("x_at_wedge") ? schema.getEntryOrder("x_at_wedge") : -1;
            cache.i_y_at_wedge = schema.exists("y_at_wedge") ? schema.getEntryOrder("y_at_wedge") : -1;
            cache.i_z_at_wedge = schema.exists("z_at_wedge") ? schema.getEntryOrder("z_at_wedge") : -1;
            cache.i_l_at_wedge = schema.exists("l_at_wedge") ? schema.getEntryOrder("l_at_wedge") : -1;
            cache.i_l_in_wedge = schema.exists("l_in_wedge") ? schema.getEntryOrder("l_in_wedge") : -1;
        }
        i_id = cache.i_id;
        i_trackid = cache.i_trackid;
        i_x_at_bar = cache.i_x_at_bar;
        i_y_at_bar = cache.i_y_at_bar;
        i_z_at_bar = cache.i_z_at_bar;
        i_l_at_bar = cache.i_l_at_bar;
        i_l_in_bar = cache.i_l_in_bar;
        i_x_at_wedge = cache.i_x_at_wedge;
        i_y_at_wedge = cache.i_y_at_wedge;
        i_z_at_wedge = cache.i_z_at_wedge;
        i_l_at_wedge = cache.i_l_at_wedge;
        i_l_in_wedge = cache.i_l_in_wedge;
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
    /// id: projection id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// trackid: id of the track that was projected
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_trackid, row));
    }
    /// x_at_bar: x position at atof bar (middle surface) in mm
    [[nodiscard]] float x_at_bar(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x_at_bar, row));
    }
    /// y_at_bar: y position at atof bar (middle surface) in mm
    [[nodiscard]] float y_at_bar(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y_at_bar, row));
    }
    /// z_at_bar: z position at atof bar (middle surface) in mm
    [[nodiscard]] float z_at_bar(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z_at_bar, row));
    }
    /// l_at_bar: path length at atof bar (inner surface) in mm
    [[nodiscard]] float l_at_bar(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_l_at_bar, row));
    }
    /// l_in_bar: path length inside atof bar in mm
    [[nodiscard]] float l_in_bar(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_l_in_bar, row));
    }
    /// x_at_wedge: x position at atof wedge (middle surface) in mm
    [[nodiscard]] float x_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x_at_wedge, row));
    }
    /// y_at_wedge: y position at atof wedge (middle surface) in mm
    [[nodiscard]] float y_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y_at_wedge, row));
    }
    /// z_at_wedge: z position at atof wedge (middle surface) in mm
    [[nodiscard]] float z_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z_at_wedge, row));
    }
    /// l_at_wedge: path length at atof wedge (inner surface) in mm
    [[nodiscard]] float l_at_wedge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_l_at_wedge, row));
    }
    /// l_in_wedge: path length inside atof wedge in mm
    [[nodiscard]] float l_in_wedge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_l_in_wedge, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_trackid = -1;
    int i_x_at_bar = -1;
    int i_y_at_bar = -1;
    int i_z_at_bar = -1;
    int i_l_at_bar = -1;
    int i_l_in_bar = -1;
    int i_x_at_wedge = -1;
    int i_y_at_wedge = -1;
    int i_z_at_wedge = -1;
    int i_l_at_wedge = -1;
    int i_l_in_wedge = -1;

    friend class Row;
};

/// Lightweight row view for ALERT::projections. Stores a pointer + row index only.
class ALERT_projections::Row {
 public:
    Row(const ALERT_projections* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] float x_at_bar() const noexcept { return m_bank->x_at_bar(m_row); }
    [[nodiscard]] float y_at_bar() const noexcept { return m_bank->y_at_bar(m_row); }
    [[nodiscard]] float z_at_bar() const noexcept { return m_bank->z_at_bar(m_row); }
    [[nodiscard]] float l_at_bar() const noexcept { return m_bank->l_at_bar(m_row); }
    [[nodiscard]] float l_in_bar() const noexcept { return m_bank->l_in_bar(m_row); }
    [[nodiscard]] float x_at_wedge() const noexcept { return m_bank->x_at_wedge(m_row); }
    [[nodiscard]] float y_at_wedge() const noexcept { return m_bank->y_at_wedge(m_row); }
    [[nodiscard]] float z_at_wedge() const noexcept { return m_bank->z_at_wedge(m_row); }
    [[nodiscard]] float l_at_wedge() const noexcept { return m_bank->l_at_wedge(m_row); }
    [[nodiscard]] float l_in_wedge() const noexcept { return m_bank->l_in_wedge(m_row); }
 private:
    const ALERT_projections* m_bank;
    int m_row;
};

inline ALERT_projections::Row ALERT_projections::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
