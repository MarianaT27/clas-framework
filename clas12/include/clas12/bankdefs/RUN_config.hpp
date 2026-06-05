// =============================================================================
// Auto-generated from header.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RUN::config` (group 10000, item 11).
/// Run Configuration
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RUN_config bank;
/// if (auto sc = bank.bind(event.get_bank("RUN::config")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto run = row.run();
/// }
/// @endcode
class RUN_config {
 public:
    static constexpr std::string_view bank_name = "RUN::config";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 11;
    static constexpr std::string_view bank_schema = "run/I,event/I,unixtime/I,trigger/L,timestamp/L,type/B,mode/B,torus/F,solenoid/F";

    class Row;

    RUN_config() = default;

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
            int i_unixtime = -1;
            int i_trigger = -1;
            int i_timestamp = -1;
            int i_type = -1;
            int i_mode = -1;
            int i_torus = -1;
            int i_solenoid = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_run = schema.exists("run") ? schema.getEntryOrder("run") : -1;
            cache.i_event = schema.exists("event") ? schema.getEntryOrder("event") : -1;
            cache.i_unixtime = schema.exists("unixtime") ? schema.getEntryOrder("unixtime") : -1;
            cache.i_trigger = schema.exists("trigger") ? schema.getEntryOrder("trigger") : -1;
            cache.i_timestamp = schema.exists("timestamp") ? schema.getEntryOrder("timestamp") : -1;
            cache.i_type = schema.exists("type") ? schema.getEntryOrder("type") : -1;
            cache.i_mode = schema.exists("mode") ? schema.getEntryOrder("mode") : -1;
            cache.i_torus = schema.exists("torus") ? schema.getEntryOrder("torus") : -1;
            cache.i_solenoid = schema.exists("solenoid") ? schema.getEntryOrder("solenoid") : -1;
        }
        i_run = cache.i_run;
        i_event = cache.i_event;
        i_unixtime = cache.i_unixtime;
        i_trigger = cache.i_trigger;
        i_timestamp = cache.i_timestamp;
        i_type = cache.i_type;
        i_mode = cache.i_mode;
        i_torus = cache.i_torus;
        i_solenoid = cache.i_solenoid;
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
    /// run: RUN number from CODA or GEMC
    [[nodiscard]] int run(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_run, row));
    }
    /// event: Event number
    [[nodiscard]] int event(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_event, row));
    }
    /// unixtime: Unix time (seconds)
    [[nodiscard]] int unixtime(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_unixtime, row));
    }
    /// trigger: trigger bits
    [[nodiscard]] int64_t trigger(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_trigger, row));
    }
    /// timestamp: time stamp from Trigger Interface (TI) board (4 nanoseconds)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_timestamp, row));
    }
    /// type: type of the run
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_type, row));
    }
    /// mode: run mode
    [[nodiscard]] int mode(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_mode, row));
    }
    /// torus: torus setting relative value(-1.0 to 1.0)
    [[nodiscard]] float torus(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_torus, row));
    }
    /// solenoid: solenoid field setting (-1.0 to 1.0)
    [[nodiscard]] float solenoid(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_solenoid, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_run = -1;
    int i_event = -1;
    int i_unixtime = -1;
    int i_trigger = -1;
    int i_timestamp = -1;
    int i_type = -1;
    int i_mode = -1;
    int i_torus = -1;
    int i_solenoid = -1;

    friend class Row;
};

/// Lightweight row view for RUN::config. Stores a pointer + row index only.
class RUN_config::Row {
 public:
    Row(const RUN_config* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int run() const noexcept { return m_bank->run(m_row); }
    [[nodiscard]] int event() const noexcept { return m_bank->event(m_row); }
    [[nodiscard]] int unixtime() const noexcept { return m_bank->unixtime(m_row); }
    [[nodiscard]] int64_t trigger() const noexcept { return m_bank->trigger(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
    [[nodiscard]] int type() const noexcept { return m_bank->type(m_row); }
    [[nodiscard]] int mode() const noexcept { return m_bank->mode(m_row); }
    [[nodiscard]] float torus() const noexcept { return m_bank->torus(m_row); }
    [[nodiscard]] float solenoid() const noexcept { return m_bank->solenoid(m_row); }
 private:
    const RUN_config* m_bank;
    int m_row;
};

inline RUN_config::Row RUN_config::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
