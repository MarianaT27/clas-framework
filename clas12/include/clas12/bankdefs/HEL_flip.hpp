// =============================================================================
// Auto-generated from data.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `HEL::flip` (group 22000, item 12).
/// Helicity flips from ADC bank.  State values are -1/0/+1, where 0 is invalid.
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// HEL_flip bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::flip")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto run = row.run();
/// }
/// @endcode
class HEL_flip {
 public:
    static constexpr std::string_view bank_name = "HEL::flip";
    static constexpr int bank_group = 22000;
    static constexpr int bank_item  = 12;
    static constexpr std::string_view bank_schema = "run/I,event/I,timestamp/L,helicity/B,helicityRaw/B,pair/B,pattern/B,status/B";

    class Row;

    HEL_flip() = default;

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
            int i_run = -1;
            int i_event = -1;
            int i_timestamp = -1;
            int i_helicity = -1;
            int i_helicityRaw = -1;
            int i_pair = -1;
            int i_pattern = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_run = schema.exists("run") ? schema.getEntryOrder("run") : -1;
            cache.i_event = schema.exists("event") ? schema.getEntryOrder("event") : -1;
            cache.i_timestamp = schema.exists("timestamp") ? schema.getEntryOrder("timestamp") : -1;
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
            cache.i_helicityRaw = schema.exists("helicityRaw") ? schema.getEntryOrder("helicityRaw") : -1;
            cache.i_pair = schema.exists("pair") ? schema.getEntryOrder("pair") : -1;
            cache.i_pattern = schema.exists("pattern") ? schema.getEntryOrder("pattern") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_run = cache.i_run;
        i_event = cache.i_event;
        i_timestamp = cache.i_timestamp;
        i_helicity = cache.i_helicity;
        i_helicityRaw = cache.i_helicityRaw;
        i_pair = cache.i_pair;
        i_pattern = cache.i_pattern;
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
    /// run: run number
    [[nodiscard]] int run(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_run, row));
    }
    /// event: event number
    [[nodiscard]] int event(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_event, row));
    }
    /// timestamp: timestamp (4ns)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_timestamp, row));
    }
    /// helicity: helicity state with half-wave plate correction
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicity, row));
    }
    /// helicityRaw: helicity state without half-wave plate correction
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicityRaw, row));
    }
    /// pair: pair sync state
    [[nodiscard]] int pair(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pair, row));
    }
    /// pattern: pattern sync state
    [[nodiscard]] int pattern(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pattern, row));
    }
    /// status: status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_run = -1;
    int i_event = -1;
    int i_timestamp = -1;
    int i_helicity = -1;
    int i_helicityRaw = -1;
    int i_pair = -1;
    int i_pattern = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for HEL::flip. Stores a pointer + row index only.
class HEL_flip::Row {
 public:
    Row(const HEL_flip* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int run() const noexcept { return m_bank->run(m_row); }
    [[nodiscard]] int event() const noexcept { return m_bank->event(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int helicityRaw() const noexcept { return m_bank->helicityRaw(m_row); }
    [[nodiscard]] int pair() const noexcept { return m_bank->pair(m_row); }
    [[nodiscard]] int pattern() const noexcept { return m_bank->pattern(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const HEL_flip* m_bank;
    int m_row;
};

inline HEL_flip::Row HEL_flip::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
