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


/// HIPO bank wrapper for `DC::doca` (group 20600, item 14).
/// Drift Chamber DOCA bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// DC_doca bank;
/// if (auto sc = bank.bind(event.get_bank("DC::doca")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto LR = row.LR();
/// }
/// @endcode
class DC_doca {
 public:
    static constexpr std::string_view bank_name = "DC::doca";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 14;

    class Row;

    DC_doca() = default;

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
            hipo::column_handle h_LR;
            hipo::column_handle h_doca;
            hipo::column_handle h_sdoca;
            hipo::column_handle h_time;
            hipo::column_handle h_stime;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("LR"); h) cache.h_LR = *h;
            else cache.h_LR = {};
            if (auto h = schema.get_handle("doca"); h) cache.h_doca = *h;
            else cache.h_doca = {};
            if (auto h = schema.get_handle("sdoca"); h) cache.h_sdoca = *h;
            else cache.h_sdoca = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("stime"); h) cache.h_stime = *h;
            else cache.h_stime = {};
        }
        h_LR = cache.h_LR;
        h_doca = cache.h_doca;
        h_sdoca = cache.h_sdoca;
        h_time = cache.h_time;
        h_stime = cache.h_stime;
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
    /// LR: left right ambiguity
    [[nodiscard]] int LR(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_LR, row));
    }
    /// doca: doca for the hit
    [[nodiscard]] float doca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_doca, row));
    }
    /// sdoca: smeared doca
    [[nodiscard]] float sdoca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_sdoca, row));
    }
    /// time: time for the hit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// stime: smeared time
    [[nodiscard]] float stime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_stime, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// LR column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> LR_col() const noexcept {
        return m_view.column<int8_t>(h_LR);
    }
    /// doca column as a contiguous span.
    [[nodiscard]] std::span<const float> doca_col() const noexcept {
        return m_view.column<float>(h_doca);
    }
    /// sdoca column as a contiguous span.
    [[nodiscard]] std::span<const float> sdoca_col() const noexcept {
        return m_view.column<float>(h_sdoca);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// stime column as a contiguous span.
    [[nodiscard]] std::span<const float> stime_col() const noexcept {
        return m_view.column<float>(h_stime);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_LR{};
    hipo::column_handle h_doca{};
    hipo::column_handle h_sdoca{};
    hipo::column_handle h_time{};
    hipo::column_handle h_stime{};

    friend class Row;
};

/// Lightweight row view for DC::doca. Stores a pointer + row index only.
class DC_doca::Row {
 public:
    Row(const DC_doca* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int LR() const noexcept { return m_bank->LR(m_row); }
    [[nodiscard]] float doca() const noexcept { return m_bank->doca(m_row); }
    [[nodiscard]] float sdoca() const noexcept { return m_bank->sdoca(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float stime() const noexcept { return m_bank->stime(m_row); }
 private:
    const DC_doca* m_bank;
    int m_row;
};

inline DC_doca::Row DC_doca::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
