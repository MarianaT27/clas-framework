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

namespace bankdefs {


/// HIPO bank wrapper for `HEL::scaler` (group 10000, item 16).
/// Helicity-gated, analyzed scalers.
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HEL_scaler bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::scaler")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto fcupgated = row.fcupgated();
/// }
/// @endcode
class HEL_scaler {
 public:
    static constexpr std::string_view bank_name = "HEL::scaler";
    static constexpr int bank_group = 10000;
    static constexpr int bank_item  = 16;

    class Row;

    HEL_scaler() = default;

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
            hipo::column_handle h_slmgated;
            hipo::column_handle h_slm;
            hipo::column_handle h_clockgated;
            hipo::column_handle h_clock;
            hipo::column_handle h_helicity;
            hipo::column_handle h_helicityRaw;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("fcupgated"); h) cache.h_fcupgated = *h;
            else cache.h_fcupgated = {};
            if (auto h = schema.get_handle("fcup"); h) cache.h_fcup = *h;
            else cache.h_fcup = {};
            if (auto h = schema.get_handle("slmgated"); h) cache.h_slmgated = *h;
            else cache.h_slmgated = {};
            if (auto h = schema.get_handle("slm"); h) cache.h_slm = *h;
            else cache.h_slm = {};
            if (auto h = schema.get_handle("clockgated"); h) cache.h_clockgated = *h;
            else cache.h_clockgated = {};
            if (auto h = schema.get_handle("clock"); h) cache.h_clock = *h;
            else cache.h_clock = {};
            if (auto h = schema.get_handle("helicity"); h) cache.h_helicity = *h;
            else cache.h_helicity = {};
            if (auto h = schema.get_handle("helicityRaw"); h) cache.h_helicityRaw = *h;
            else cache.h_helicityRaw = {};
        }
        h_fcupgated = cache.h_fcupgated;
        h_fcup = cache.h_fcup;
        h_slmgated = cache.h_slmgated;
        h_slm = cache.h_slm;
        h_clockgated = cache.h_clockgated;
        h_clock = cache.h_clock;
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
    /// fcupgated: Beam charge from Faraday cup, DAQ-gated (nano-Coulomb)
    [[nodiscard]] float fcupgated(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fcupgated, row));
    }
    /// fcup: Beam charge from Faraday cup, (nano-Coulomb)
    [[nodiscard]] float fcup(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_fcup, row));
    }
    /// slmgated: Beam charge from SLM, DAQ-gated (nano-Coulomb)
    [[nodiscard]] float slmgated(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_slmgated, row));
    }
    /// slm: Beam charge from SLM (nano-Coulomb)
    [[nodiscard]] float slm(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_slm, row));
    }
    /// clockgated: Clock, DAQ-gated
    [[nodiscard]] float clockgated(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_clockgated, row));
    }
    /// clock: Clock
    [[nodiscard]] float clock(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_clock, row));
    }
    /// helicity: Helicity state, HWP-corrected
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicity, row));
    }
    /// helicityRaw: Helicity state
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicityRaw, row));
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
    /// slmgated column as a contiguous span.
    [[nodiscard]] std::span<const float> slmgated_col() const noexcept {
        return m_view.column<float>(h_slmgated);
    }
    /// slm column as a contiguous span.
    [[nodiscard]] std::span<const float> slm_col() const noexcept {
        return m_view.column<float>(h_slm);
    }
    /// clockgated column as a contiguous span.
    [[nodiscard]] std::span<const float> clockgated_col() const noexcept {
        return m_view.column<float>(h_clockgated);
    }
    /// clock column as a contiguous span.
    [[nodiscard]] std::span<const float> clock_col() const noexcept {
        return m_view.column<float>(h_clock);
    }
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
    hipo::column_handle h_fcupgated{};
    hipo::column_handle h_fcup{};
    hipo::column_handle h_slmgated{};
    hipo::column_handle h_slm{};
    hipo::column_handle h_clockgated{};
    hipo::column_handle h_clock{};
    hipo::column_handle h_helicity{};
    hipo::column_handle h_helicityRaw{};

    friend class Row;
};

/// Lightweight row view for HEL::scaler. Stores a pointer + row index only.
class HEL_scaler::Row {
 public:
    Row(const HEL_scaler* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float fcupgated() const noexcept { return m_bank->fcupgated(m_row); }
    [[nodiscard]] float fcup() const noexcept { return m_bank->fcup(m_row); }
    [[nodiscard]] float slmgated() const noexcept { return m_bank->slmgated(m_row); }
    [[nodiscard]] float slm() const noexcept { return m_bank->slm(m_row); }
    [[nodiscard]] float clockgated() const noexcept { return m_bank->clockgated(m_row); }
    [[nodiscard]] float clock() const noexcept { return m_bank->clock(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int helicityRaw() const noexcept { return m_bank->helicityRaw(m_row); }
 private:
    const HEL_scaler* m_bank;
    int m_row;
};

inline HEL_scaler::Row HEL_scaler::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
