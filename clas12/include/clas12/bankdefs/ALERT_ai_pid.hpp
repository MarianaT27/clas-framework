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


/// HIPO bank wrapper for `ALERT::ai:pid` (group 23000, item 34).
/// AI-assisted PID for ALERT
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ALERT_ai_pid bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::ai:pid")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trackid = row.trackid();
/// }
/// @endcode
class ALERT_ai_pid {
 public:
    static constexpr std::string_view bank_name = "ALERT::ai:pid";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 34;

    class Row;

    ALERT_ai_pid() = default;

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
            int i_pid = -1;
            int i_prob_2212 = -1;
            int i_prob_45 = -1;
            int i_prob_46 = -1;
            int i_prob_47 = -1;
            int i_prob_49 = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_trackid = schema.exists("trackid") ? schema.getEntryOrder("trackid") : -1;
            cache.i_clusterid = schema.exists("clusterid") ? schema.getEntryOrder("clusterid") : -1;
            cache.i_pid = schema.exists("pid") ? schema.getEntryOrder("pid") : -1;
            cache.i_prob_2212 = schema.exists("prob_2212") ? schema.getEntryOrder("prob_2212") : -1;
            cache.i_prob_45 = schema.exists("prob_45") ? schema.getEntryOrder("prob_45") : -1;
            cache.i_prob_46 = schema.exists("prob_46") ? schema.getEntryOrder("prob_46") : -1;
            cache.i_prob_47 = schema.exists("prob_47") ? schema.getEntryOrder("prob_47") : -1;
            cache.i_prob_49 = schema.exists("prob_49") ? schema.getEntryOrder("prob_49") : -1;
        }
        i_trackid = cache.i_trackid;
        i_clusterid = cache.i_clusterid;
        i_pid = cache.i_pid;
        i_prob_2212 = cache.i_prob_2212;
        i_prob_45 = cache.i_prob_45;
        i_prob_46 = cache.i_prob_46;
        i_prob_47 = cache.i_prob_47;
        i_prob_49 = cache.i_prob_49;
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
    /// pid: argmax PID
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pid, row));
    }
    /// prob_2212: P(pid=2212)
    [[nodiscard]] float prob_2212(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob_2212, row));
    }
    /// prob_45: P(pid=45)
    [[nodiscard]] float prob_45(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob_45, row));
    }
    /// prob_46: P(pid=46)
    [[nodiscard]] float prob_46(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob_46, row));
    }
    /// prob_47: P(pid=47)
    [[nodiscard]] float prob_47(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob_47, row));
    }
    /// prob_49: P(pid=49)
    [[nodiscard]] float prob_49(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob_49, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_trackid = -1;
    int i_clusterid = -1;
    int i_pid = -1;
    int i_prob_2212 = -1;
    int i_prob_45 = -1;
    int i_prob_46 = -1;
    int i_prob_47 = -1;
    int i_prob_49 = -1;

    friend class Row;
};

/// Lightweight row view for ALERT::ai:pid. Stores a pointer + row index only.
class ALERT_ai_pid::Row {
 public:
    Row(const ALERT_ai_pid* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] float prob_2212() const noexcept { return m_bank->prob_2212(m_row); }
    [[nodiscard]] float prob_45() const noexcept { return m_bank->prob_45(m_row); }
    [[nodiscard]] float prob_46() const noexcept { return m_bank->prob_46(m_row); }
    [[nodiscard]] float prob_47() const noexcept { return m_bank->prob_47(m_row); }
    [[nodiscard]] float prob_49() const noexcept { return m_bank->prob_49(m_row); }
 private:
    const ALERT_ai_pid* m_bank;
    int m_row;
};

inline ALERT_ai_pid::Row ALERT_ai_pid::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
