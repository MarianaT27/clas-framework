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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `MC::User` (group 40, item 5).
/// User-defined information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MC_User bank;
/// if (auto sc = bank.bind(event.get_bank("MC::User")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto userVar = row.userVar();
/// }
/// @endcode
class MC_User {
 public:
    static constexpr std::string_view bank_name = "MC::User";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 5;

    class Row;

    MC_User() = default;

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
            hipo::column_handle h_userVar;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("userVar"); h) cache.h_userVar = *h;
            else cache.h_userVar = {};
        }
        h_userVar = cache.h_userVar;
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
    /// userVar: user-defined variable
    [[nodiscard]] float userVar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_userVar, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// userVar column as a contiguous span.
    [[nodiscard]] std::span<const float> userVar_col() const noexcept {
        return m_view.column<float>(h_userVar);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_userVar{};

    friend class Row;
};

/// Lightweight row view for MC::User. Stores a pointer + row index only.
class MC_User::Row {
 public:
    Row(const MC_User* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float userVar() const noexcept { return m_bank->userVar(m_row); }
 private:
    const MC_User* m_bank;
    int m_row;
};

inline MC_User::Row MC_User::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
