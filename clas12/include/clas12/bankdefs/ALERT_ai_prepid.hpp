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


/// HIPO bank wrapper for `ALERT::ai:prepid` (group 23000, item 33).
/// ALERT AI-assisted PrePID to be used for Kalman Filter
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ALERT_ai_prepid bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::ai:prepid")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trackid = row.trackid();
/// }
/// @endcode
class ALERT_ai_prepid {
 public:
    static constexpr std::string_view bank_name = "ALERT::ai:prepid";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 33;

    class Row;

    ALERT_ai_prepid() = default;

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
            int i_clusterid = -1;
            int i_prepid = -1;
            int i_p2212 = -1;
            int i_p45 = -1;
            int i_p46 = -1;
            int i_p47 = -1;
            int i_p49 = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
            cache.i_clusterid = schema.exists("clusterid") ? schema.getEntryOrder("clusterid") : -1;
            cache.i_prepid = schema.exists("prepid") ? schema.getEntryOrder("prepid") : -1;
            cache.i_p2212 = schema.exists("p2212") ? schema.getEntryOrder("p2212") : -1;
            cache.i_p45 = schema.exists("p45") ? schema.getEntryOrder("p45") : -1;
            cache.i_p46 = schema.exists("p46") ? schema.getEntryOrder("p46") : -1;
            cache.i_p47 = schema.exists("p47") ? schema.getEntryOrder("p47") : -1;
            cache.i_p49 = schema.exists("p49") ? schema.getEntryOrder("p49") : -1;
        }
        i_trackid = cache.i_trackid;
        i_clusterid = cache.i_clusterid;
        i_prepid = cache.i_prepid;
        i_p2212 = cache.i_p2212;
        i_p45 = cache.i_p45;
        i_p46 = cache.i_p46;
        i_p47 = cache.i_p47;
        i_p49 = cache.i_p49;
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
    /// trackid: AHDC trackid
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trackid, row));
    }
    /// clusterid: ATOF cluster id
    [[nodiscard]] int clusterid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_clusterid, row));
    }
    /// prepid: argmax PID
    [[nodiscard]] int prepid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_prepid, row));
    }
    /// p2212: P(pid=2212)
    [[nodiscard]] float p2212(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p2212, row));
    }
    /// p45: P(pid=45)
    [[nodiscard]] float p45(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p45, row));
    }
    /// p46: P(pid=46)
    [[nodiscard]] float p46(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p46, row));
    }
    /// p47: P(pid=47)
    [[nodiscard]] float p47(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p47, row));
    }
    /// p49: P(pid=49)
    [[nodiscard]] float p49(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p49, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_trackid = -1;
    int i_clusterid = -1;
    int i_prepid = -1;
    int i_p2212 = -1;
    int i_p45 = -1;
    int i_p46 = -1;
    int i_p47 = -1;
    int i_p49 = -1;

    friend class Row;
};

/// Lightweight row view for ALERT::ai:prepid. Stores a pointer + row index only.
class ALERT_ai_prepid::Row {
 public:
    Row(const ALERT_ai_prepid* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
    [[nodiscard]] int prepid() const noexcept { return m_bank->prepid(m_row); }
    [[nodiscard]] float p2212() const noexcept { return m_bank->p2212(m_row); }
    [[nodiscard]] float p45() const noexcept { return m_bank->p45(m_row); }
    [[nodiscard]] float p46() const noexcept { return m_bank->p46(m_row); }
    [[nodiscard]] float p47() const noexcept { return m_bank->p47(m_row); }
    [[nodiscard]] float p49() const noexcept { return m_bank->p49(m_row); }
 private:
    const ALERT_ai_prepid* m_bank;
    int m_row;
};

inline ALERT_ai_prepid::Row ALERT_ai_prepid::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
