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


/// HIPO bank wrapper for `ALERT::ai:projections` (group 23000, item 32).
/// Track Projections to ATOF given by AI
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ALERT_ai_projections bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::ai:projections")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trackid = row.trackid();
/// }
/// @endcode
class ALERT_ai_projections {
 public:
    static constexpr std::string_view bank_name = "ALERT::ai:projections";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 32;
    static constexpr std::string_view bank_schema = "trackid/I,matched_atof_hit_id/I";

    class Row;

    ALERT_ai_projections() = default;

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
            int i_trackid = -1;
            int i_matched_atof_hit_id = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
            cache.i_matched_atof_hit_id = schema.exists("matched_atof_hit_id") ? schema.getEntryOrder("matched_atof_hit_id") : -1;
        }
        i_trackid = cache.i_trackid;
        i_matched_atof_hit_id = cache.i_matched_atof_hit_id;
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
    /// trackid: track id
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trackid, row));
    }
    /// matched_atof_hit_id: id of the matched ATOF hit, -1 if no hit was matched
    [[nodiscard]] int matched_atof_hit_id(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_matched_atof_hit_id, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_trackid = -1;
    int i_matched_atof_hit_id = -1;

    friend class Row;
};

/// Lightweight row view for ALERT::ai:projections. Stores a pointer + row index only.
class ALERT_ai_projections::Row {
 public:
    Row(const ALERT_ai_projections* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int matched_atof_hit_id() const noexcept { return m_bank->matched_atof_hit_id(m_row); }
 private:
    const ALERT_ai_projections* m_bank;
    int m_row;
};

inline ALERT_ai_projections::Row ALERT_ai_projections::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
