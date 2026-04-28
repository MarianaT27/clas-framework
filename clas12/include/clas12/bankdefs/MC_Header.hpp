// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
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

namespace bankdefs {


/// HIPO bank wrapper for `MC::Header` (group 40, item 0).
/// Head bank for the generated event
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MC_Header bank;
/// if (auto sc = bank.bind(event.get_bank("MC::Header")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto run = row.run();
/// }
/// @endcode
class MC_Header {
 public:
    static constexpr std::string_view bank_name = "MC::Header";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 0;

    class Row;

    MC_Header() = default;

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
            hipo::column_handle h_type;
            hipo::column_handle h_helicity;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("run"); h) cache.h_run = *h;
            else cache.h_run = {};
            if (auto h = schema.get_handle("event"); h) cache.h_event = *h;
            else cache.h_event = {};
            if (auto h = schema.get_handle("type"); h) cache.h_type = *h;
            else cache.h_type = {};
            if (auto h = schema.get_handle("helicity"); h) cache.h_helicity = *h;
            else cache.h_helicity = {};
        }
        h_run = cache.h_run;
        h_event = cache.h_event;
        h_type = cache.h_type;
        h_helicity = cache.h_helicity;
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
    /// run: Run number
    [[nodiscard]] int run(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_run, row));
    }
    /// event: Event number
    [[nodiscard]] int event(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_event, row));
    }
    /// type: Event type
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_type, row));
    }
    /// helicity: Beam helicity
    [[nodiscard]] float helicity(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_helicity, row));
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
    /// type column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> type_col() const noexcept {
        return m_view.column<int8_t>(h_type);
    }
    /// helicity column as a contiguous span.
    [[nodiscard]] std::span<const float> helicity_col() const noexcept {
        return m_view.column<float>(h_helicity);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_run{};
    hipo::column_handle h_event{};
    hipo::column_handle h_type{};
    hipo::column_handle h_helicity{};

    friend class Row;
};

/// Lightweight row view for MC::Header. Stores a pointer + row index only.
class MC_Header::Row {
 public:
    Row(const MC_Header* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int run() const noexcept { return m_bank->run(m_row); }
    [[nodiscard]] int event() const noexcept { return m_bank->event(m_row); }
    [[nodiscard]] int type() const noexcept { return m_bank->type(m_row); }
    [[nodiscard]] float helicity() const noexcept { return m_bank->helicity(m_row); }
 private:
    const MC_Header* m_bank;
    int m_row;
};

inline MC_Header::Row MC_Header::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
