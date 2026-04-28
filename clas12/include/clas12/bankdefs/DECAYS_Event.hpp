// =============================================================================
// Auto-generated from decays.json - do not edit by hand.
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


/// HIPO bank wrapper for `DECAYS::Event` (group 900, item 10).
/// Event Header Bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// DECAYS_Event bank;
/// if (auto sc = bank.bind(event.get_bank("DECAYS::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto tarmass = row.tarmass();
/// }
/// @endcode
class DECAYS_Event {
 public:
    static constexpr std::string_view bank_name = "DECAYS::Event";
    static constexpr int bank_group = 900;
    static constexpr int bank_item  = 10;

    class Row;

    DECAYS_Event() = default;

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
            hipo::column_handle h_tarmass;
            hipo::column_handle h_beamenergy;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("tarmass"); h) cache.h_tarmass = *h;
            else cache.h_tarmass = {};
            if (auto h = schema.get_handle("beamenergy"); h) cache.h_beamenergy = *h;
            else cache.h_beamenergy = {};
        }
        h_tarmass = cache.h_tarmass;
        h_beamenergy = cache.h_beamenergy;
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
    /// tarmass: target mass
    [[nodiscard]] float tarmass(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tarmass, row));
    }
    /// beamenergy: beam energy
    [[nodiscard]] float beamenergy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_beamenergy, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// tarmass column as a contiguous span.
    [[nodiscard]] std::span<const float> tarmass_col() const noexcept {
        return m_view.column<float>(h_tarmass);
    }
    /// beamenergy column as a contiguous span.
    [[nodiscard]] std::span<const float> beamenergy_col() const noexcept {
        return m_view.column<float>(h_beamenergy);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_tarmass{};
    hipo::column_handle h_beamenergy{};

    friend class Row;
};

/// Lightweight row view for DECAYS::Event. Stores a pointer + row index only.
class DECAYS_Event::Row {
 public:
    Row(const DECAYS_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float tarmass() const noexcept { return m_bank->tarmass(m_row); }
    [[nodiscard]] float beamenergy() const noexcept { return m_bank->beamenergy(m_row); }
 private:
    const DECAYS_Event* m_bank;
    int m_row;
};

inline DECAYS_Event::Row DECAYS_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
