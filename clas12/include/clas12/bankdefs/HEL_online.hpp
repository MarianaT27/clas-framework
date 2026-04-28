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


/// HIPO bank wrapper for `HEL::online` (group 22000, item 13).
/// Helicity with online delay-correction in Level3 trigger
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HEL_online bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::online")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto helicity = row.helicity();
/// }
/// @endcode
class HEL_online {
 public:
    static constexpr std::string_view bank_name = "HEL::online";
    static constexpr int bank_group = 22000;
    static constexpr int bank_item  = 13;

    class Row;

    HEL_online() = default;

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
            hipo::column_handle h_helicity;
            hipo::column_handle h_helicityRaw;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("helicity"); h) cache.h_helicity = *h;
            else cache.h_helicity = {};
            if (auto h = schema.get_handle("helicityRaw"); h) cache.h_helicityRaw = *h;
            else cache.h_helicityRaw = {};
        }
        h_helicity = cache.h_helicity;
        h_helicityRaw = cache.h_helicityRaw;
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
    /// helicity: online-delay-corrected helicity, with HWP-correction (0=UDF)
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicity, row));
    }
    /// helicityRaw: online-delay-corrected helicity (0=UDF)
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicityRaw, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// helicity column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicity_col() const noexcept {
        return m_view.column<int8_t>(h_helicity);
    }
    /// helicityRaw column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicityRaw_col() const noexcept {
        return m_view.column<int8_t>(h_helicityRaw);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_helicity{};
    hipo::column_handle h_helicityRaw{};

    friend class Row;
};

/// Lightweight row view for HEL::online. Stores a pointer + row index only.
class HEL_online::Row {
 public:
    Row(const HEL_online* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int helicityRaw() const noexcept { return m_bank->helicityRaw(m_row); }
 private:
    const HEL_online* m_bank;
    int m_row;
};

inline HEL_online::Row HEL_online::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
