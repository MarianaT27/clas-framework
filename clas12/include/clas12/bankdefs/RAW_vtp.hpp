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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `RAW::vtp` (group 20000, item 14).
/// VTP bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RAW_vtp bank;
/// if (auto sc = bank.bind(event.get_bank("RAW::vtp")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto crate = row.crate();
/// }
/// @endcode
class RAW_vtp {
 public:
    static constexpr std::string_view bank_name = "RAW::vtp";
    static constexpr int bank_group = 20000;
    static constexpr int bank_item  = 14;

    class Row;

    RAW_vtp() = default;

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
            hipo::column_handle h_word;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("crate"); h) cache.h_crate = *h;
            else cache.h_crate = {};
            if (auto h = schema.get_handle("word"); h) cache.h_word = *h;
            else cache.h_word = {};
        }
        h_crate = cache.h_crate;
        h_word = cache.h_word;
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
    /// crate: crate number
    [[nodiscard]] int crate(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_crate, row));
    }
    /// word: vtp value
    [[nodiscard]] int word(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_word, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// crate column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> crate_col() const noexcept {
        return m_view.column<int8_t>(h_crate);
    }
    /// word column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> word_col() const noexcept {
        return m_view.column<int32_t>(h_word);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_crate{};
    hipo::column_handle h_word{};

    friend class Row;
};

/// Lightweight row view for RAW::vtp. Stores a pointer + row index only.
class RAW_vtp::Row {
 public:
    Row(const RAW_vtp* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int crate() const noexcept { return m_bank->crate(m_row); }
    [[nodiscard]] int word() const noexcept { return m_bank->word(m_row); }
 private:
    const RAW_vtp* m_bank;
    int m_row;
};

inline RAW_vtp::Row RAW_vtp::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
