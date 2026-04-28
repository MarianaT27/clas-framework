// =============================================================================
// Auto-generated from ecal.json - do not edit by hand.
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


/// HIPO bank wrapper for `ECAL::trigger` (group 20700, item 27).
/// Occupancy bank for trigger bits
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ECAL_trigger bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::trigger")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto bit = row.bit();
/// }
/// @endcode
class ECAL_trigger {
 public:
    static constexpr std::string_view bank_name = "ECAL::trigger";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 27;

    class Row;

    ECAL_trigger() = default;

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
            hipo::column_handle h_bit;
            hipo::column_handle h_counts;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("bit"); h) cache.h_bit = *h;
            else cache.h_bit = {};
            if (auto h = schema.get_handle("counts"); h) cache.h_counts = *h;
            else cache.h_counts = {};
        }
        h_bit = cache.h_bit;
        h_counts = cache.h_counts;
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
    /// bit: trigger bit
    [[nodiscard]] int bit(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_bit, row));
    }
    /// counts: bit count
    [[nodiscard]] int counts(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_counts, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// bit column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> bit_col() const noexcept {
        return m_view.column<int16_t>(h_bit);
    }
    /// counts column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> counts_col() const noexcept {
        return m_view.column<int32_t>(h_counts);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_bit{};
    hipo::column_handle h_counts{};

    friend class Row;
};

/// Lightweight row view for ECAL::trigger. Stores a pointer + row index only.
class ECAL_trigger::Row {
 public:
    Row(const ECAL_trigger* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int bit() const noexcept { return m_bank->bit(m_row); }
    [[nodiscard]] int counts() const noexcept { return m_bank->counts(m_row); }
 private:
    const ECAL_trigger* m_bank;
    int m_row;
};

inline ECAL_trigger::Row ECAL_trigger::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
