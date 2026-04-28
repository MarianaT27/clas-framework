// =============================================================================
// Auto-generated from data.json - do not edit by hand.
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


/// HIPO bank wrapper for `RAW::scaler` (group 20000, item 13).
/// Scaler bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RAW_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("RAW::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class RAW_scaler {
 public:
    static constexpr std::string_view bank_name = "RAW::scaler";
    static constexpr int bank_group = 20000;
    static constexpr int bank_item  = 13;

    class Row;

    RAW_scaler() = default;

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
            hipo::column_handle h_crate;
            hipo::column_handle h_slot;
            hipo::column_handle h_channel;
            hipo::column_handle h_helicity;
            hipo::column_handle h_quartet;
            hipo::column_handle h_value;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("crate"); h) cache.h_crate = *h;
            else cache.h_crate = {};
            if (auto h = schema.get_handle("slot"); h) cache.h_slot = *h;
            else cache.h_slot = {};
            if (auto h = schema.get_handle("channel"); h) cache.h_channel = *h;
            else cache.h_channel = {};
            if (auto h = schema.get_handle("helicity"); h) cache.h_helicity = *h;
            else cache.h_helicity = {};
            if (auto h = schema.get_handle("quartet"); h) cache.h_quartet = *h;
            else cache.h_quartet = {};
            if (auto h = schema.get_handle("value"); h) cache.h_value = *h;
            else cache.h_value = {};
        }
        h_crate = cache.h_crate;
        h_slot = cache.h_slot;
        h_channel = cache.h_channel;
        h_helicity = cache.h_helicity;
        h_quartet = cache.h_quartet;
        h_value = cache.h_value;
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
    /// crate: scaler crate
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_crate, row));
    }
    /// slot: scaler slot
    [[nodiscard]] int slot(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_slot, row));
    }
    /// channel: scaler channel (0/32 = FCUP, 1/33 = SLM, 2/24 = Clock)
    [[nodiscard]] int channel(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_channel, row));
    }
    /// helicity: helicity
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicity, row));
    }
    /// quartet: quartet
    [[nodiscard]] int quartet(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_quartet, row));
    }
    /// value: scaler value
    [[nodiscard]] int64_t value(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_value, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// crate column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> crate_col() const noexcept {
        return m_view.column<int8_t>(h_crate);
    }
    /// slot column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> slot_col() const noexcept {
        return m_view.column<int8_t>(h_slot);
    }
    /// channel column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> channel_col() const noexcept {
        return m_view.column<int16_t>(h_channel);
    }
    /// helicity column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicity_col() const noexcept {
        return m_view.column<int8_t>(h_helicity);
    }
    /// quartet column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> quartet_col() const noexcept {
        return m_view.column<int8_t>(h_quartet);
    }
    /// value column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> value_col() const noexcept {
        return m_view.column<int64_t>(h_value);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_crate{};
    hipo::column_handle h_slot{};
    hipo::column_handle h_channel{};
    hipo::column_handle h_helicity{};
    hipo::column_handle h_quartet{};
    hipo::column_handle h_value{};

    friend class Row;
};

/// Lightweight row view for RAW::scaler. Stores a pointer + row index only.
class RAW_scaler::Row {
 public:
    Row(const RAW_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int slot() const noexcept { return m_bank->slot(m_row); }
    [[nodiscard]] int channel() const noexcept { return m_bank->channel(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int quartet() const noexcept { return m_bank->quartet(m_row); }
    [[nodiscard]] int64_t value() const noexcept { return m_bank->value(m_row); }
 private:
    const RAW_scaler* m_bank;
    int m_row;
};

inline RAW_scaler::Row RAW_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
