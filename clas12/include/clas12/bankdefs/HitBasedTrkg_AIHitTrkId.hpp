// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
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


/// HIPO bank wrapper for `HitBasedTrkg::AIHitTrkId` (group 20600, item 67).
/// pointers between track and hit banks
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HitBasedTrkg_AIHitTrkId bank;
/// if (auto sc = bank.bind(event.get_bank("HitBasedTrkg::AIHitTrkId")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HitBasedTrkg_AIHitTrkId {
 public:
    static constexpr std::string_view bank_name = "HitBasedTrkg::AIHitTrkId";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 67;

    class Row;

    HitBasedTrkg_AIHitTrkId() = default;

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
            hipo::column_handle h_id;
            hipo::column_handle h_tid;
            hipo::column_handle h_B;
            hipo::column_handle h_TProp;
            hipo::column_handle h_TFlight;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("tid"); h) cache.h_tid = *h;
            else cache.h_tid = {};
            if (auto h = schema.get_handle("B"); h) cache.h_B = *h;
            else cache.h_B = {};
            if (auto h = schema.get_handle("TProp"); h) cache.h_TProp = *h;
            else cache.h_TProp = {};
            if (auto h = schema.get_handle("TFlight"); h) cache.h_TFlight = *h;
            else cache.h_TFlight = {};
        }
        h_id = cache.h_id;
        h_tid = cache.h_tid;
        h_B = cache.h_B;
        h_TProp = cache.h_TProp;
        h_TFlight = cache.h_TFlight;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// tid: id of the track
    [[nodiscard]] int tid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_tid, row));
    }
    /// B: B-field intensity at hit position in tilted-sector system
    [[nodiscard]] float B(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_B, row));
    }
    /// TProp: t propagation along the wire
    [[nodiscard]] float TProp(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_TProp, row));
    }
    /// TFlight: time of flight correction
    [[nodiscard]] float TFlight(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_TFlight, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// tid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> tid_col() const noexcept {
        return m_view.column<int16_t>(h_tid);
    }
    /// B column as a contiguous span.
    [[nodiscard]] std::span<const float> B_col() const noexcept {
        return m_view.column<float>(h_B);
    }
    /// TProp column as a contiguous span.
    [[nodiscard]] std::span<const float> TProp_col() const noexcept {
        return m_view.column<float>(h_TProp);
    }
    /// TFlight column as a contiguous span.
    [[nodiscard]] std::span<const float> TFlight_col() const noexcept {
        return m_view.column<float>(h_TFlight);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_tid{};
    hipo::column_handle h_B{};
    hipo::column_handle h_TProp{};
    hipo::column_handle h_TFlight{};

    friend class Row;
};

/// Lightweight row view for HitBasedTrkg::AIHitTrkId. Stores a pointer + row index only.
class HitBasedTrkg_AIHitTrkId::Row {
 public:
    Row(const HitBasedTrkg_AIHitTrkId* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int tid() const noexcept { return m_bank->tid(m_row); }
    [[nodiscard]] float B() const noexcept { return m_bank->B(m_row); }
    [[nodiscard]] float TProp() const noexcept { return m_bank->TProp(m_row); }
    [[nodiscard]] float TFlight() const noexcept { return m_bank->TFlight(m_row); }
 private:
    const HitBasedTrkg_AIHitTrkId* m_bank;
    int m_row;
};

inline HitBasedTrkg_AIHitTrkId::Row HitBasedTrkg_AIHitTrkId::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
