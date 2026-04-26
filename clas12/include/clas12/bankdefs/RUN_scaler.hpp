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


/// HIPO bank wrapper for `RUN::scaler` (group 10000, item 14).
/// Run-integrated, analyzed scalers.
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RUN_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("RUN::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto fcupgated = row.fcupgated();
/// }
/// @endcode
class RUN_scaler {
 public:
    static constexpr std::string_view bank_name = "RUN::scaler";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 14;

    class Row;

    RUN_scaler() = default;

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
            hipo::column_handle h_fcupgated;
            hipo::column_handle h_fcup;
            hipo::column_handle h_livetime;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("fcupgated"); h) cache.h_fcupgated = *h;
            else cache.h_fcupgated = {};
            if (auto h = schema.get_handle("fcup"); h) cache.h_fcup = *h;
            else cache.h_fcup = {};
            if (auto h = schema.get_handle("livetime"); h) cache.h_livetime = *h;
            else cache.h_livetime = {};
        }
        h_fcupgated = cache.h_fcupgated;
        h_fcup = cache.h_fcup;
        h_livetime = cache.h_livetime;
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
    /// fcupgated: Beam charge, integrated from beginning of run, DAQ-gated (nano-Coulomb)
    [[nodiscard]] float fcupgated(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fcupgated, row));
    }
    /// fcup: Beam charge, integrated from beginning of run, ungated (nano-Coulomb)
    [[nodiscard]] float fcup(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fcup, row));
    }
    /// livetime: DAQ livetime during one scaler period
    [[nodiscard]] float livetime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_livetime, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// fcupgated column as a contiguous span.
    [[nodiscard]] std::span<const float> fcupgated_col() const noexcept {
        return m_view.column<float>(h_fcupgated);
    }
    /// fcup column as a contiguous span.
    [[nodiscard]] std::span<const float> fcup_col() const noexcept {
        return m_view.column<float>(h_fcup);
    }
    /// livetime column as a contiguous span.
    [[nodiscard]] std::span<const float> livetime_col() const noexcept {
        return m_view.column<float>(h_livetime);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_fcupgated{};
    hipo::column_handle h_fcup{};
    hipo::column_handle h_livetime{};

    friend class Row;
};

/// Lightweight row view for RUN::scaler. Stores a pointer + row index only.
class RUN_scaler::Row {
 public:
    Row(const RUN_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float fcupgated() const noexcept { return m_bank->fcupgated(m_row); }
    [[nodiscard]] float fcup() const noexcept { return m_bank->fcup(m_row); }
    [[nodiscard]] float livetime() const noexcept { return m_bank->livetime(m_row); }
 private:
    const RUN_scaler* m_bank;
    int m_row;
};

inline RUN_scaler::Row RUN_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
