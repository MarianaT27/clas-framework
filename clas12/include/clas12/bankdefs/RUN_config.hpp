// =============================================================================
// Auto-generated from header.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include <bank_view.h>
#include <schema.h>

namespace clas12::bankdefs {


/// HIPO bank wrapper for `RUN::config` (group 10000, item 11).
/// Run Configuration
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    class Row;

    RUN_config() = default;

    /// Bind to a live bank_view. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column handle by name; all
    /// subsequent bind()s for the same schema hit the cache and copy 25-30
    /// trivially-copyable column_handle words.
    /// Returns false if the view is not valid.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        struct HandleCache {
            const hipo::schema* schema_ptr = nullptr;
            hipo::column_handle h_run;
            hipo::column_handle h_event;
            hipo::column_handle h_unixtime;
            hipo::column_handle h_trigger;
            hipo::column_handle h_timestamp;
            hipo::column_handle h_type;
            hipo::column_handle h_mode;
            hipo::column_handle h_torus;
            hipo::column_handle h_solenoid;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("run"); h) cache.h_run = *h;
            else cache.h_run = {};
            if (auto h = schema.get_handle("event"); h) cache.h_event = *h;
            else cache.h_event = {};
            if (auto h = schema.get_handle("unixtime"); h) cache.h_unixtime = *h;
            else cache.h_unixtime = {};
            if (auto h = schema.get_handle("trigger"); h) cache.h_trigger = *h;
            else cache.h_trigger = {};
            if (auto h = schema.get_handle("timestamp"); h) cache.h_timestamp = *h;
            else cache.h_timestamp = {};
            if (auto h = schema.get_handle("type"); h) cache.h_type = *h;
            else cache.h_type = {};
            if (auto h = schema.get_handle("mode"); h) cache.h_mode = *h;
            else cache.h_mode = {};
            if (auto h = schema.get_handle("torus"); h) cache.h_torus = *h;
            else cache.h_torus = {};
            if (auto h = schema.get_handle("solenoid"); h) cache.h_solenoid = *h;
            else cache.h_solenoid = {};
        }
        h_run = cache.h_run;
        h_event = cache.h_event;
        h_unixtime = cache.h_unixtime;
        h_trigger = cache.h_trigger;
        h_timestamp = cache.h_timestamp;
        h_type = cache.h_type;
        h_mode = cache.h_mode;
        h_torus = cache.h_torus;
        h_solenoid = cache.h_solenoid;
        return true;
    }

    /// Current row count of the bound bank view.
    [[nodiscard]] int rows() const noexcept { return m_view.rows(); }

    /// True if bind() has been called with a valid view.
    [[nodiscard]] bool valid() const noexcept { return m_view.valid(); }

    /// Access the underlying bank_view (for advanced use).
    [[nodiscard]] const hipo::bank_view& view() const noexcept { return m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (handle-based, zero string lookup)
    // -------------------------------------------------------------------
    /// run: RUN number from CODA or GEMC
    [[nodiscard]] int run(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_run, row));
    }
    /// event: Event number
    [[nodiscard]] int event(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_event, row));
    }
    /// unixtime: Unix time (seconds)
    [[nodiscard]] int unixtime(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_unixtime, row));
    }
    /// trigger: trigger bits
    [[nodiscard]] int64_t trigger(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_trigger, row));
    }
    /// timestamp: time stamp from Trigger Interface (TI) board (4 nanoseconds)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_timestamp, row));
    }
    /// type: type of the run
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_type, row));
    }
    /// mode: run mode
    [[nodiscard]] int mode(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_mode, row));
    }
    /// torus: torus setting relative value(-1.0 to 1.0)
    [[nodiscard]] float torus(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_torus, row));
    }
    /// solenoid: solenoid field setting (-1.0 to 1.0)
    [[nodiscard]] float solenoid(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_solenoid, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// run column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> run_col() const noexcept {
        return m_view.column<int32_t>(h_run);
    }
    /// event column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> event_col() const noexcept {
        return m_view.column<int32_t>(h_event);
    }
    /// unixtime column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> unixtime_col() const noexcept {
        return m_view.column<int32_t>(h_unixtime);
    }
    /// trigger column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> trigger_col() const noexcept {
        return m_view.column<int64_t>(h_trigger);
    }
    /// timestamp column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> timestamp_col() const noexcept {
        return m_view.column<int64_t>(h_timestamp);
    }
    /// type column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> type_col() const noexcept {
        return m_view.column<int8_t>(h_type);
    }
    /// mode column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> mode_col() const noexcept {
        return m_view.column<int8_t>(h_mode);
    }
    /// torus column as a contiguous span.
    [[nodiscard]] std::span<const float> torus_col() const noexcept {
        return m_view.column<float>(h_torus);
    }
    /// solenoid column as a contiguous span.
    [[nodiscard]] std::span<const float> solenoid_col() const noexcept {
        return m_view.column<float>(h_solenoid);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_run{};
    hipo::column_handle h_event{};
    hipo::column_handle h_unixtime{};
    hipo::column_handle h_trigger{};
    hipo::column_handle h_timestamp{};
    hipo::column_handle h_type{};
    hipo::column_handle h_mode{};
    hipo::column_handle h_torus{};
    hipo::column_handle h_solenoid{};

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

}  // namespace clas12::bankdefs
