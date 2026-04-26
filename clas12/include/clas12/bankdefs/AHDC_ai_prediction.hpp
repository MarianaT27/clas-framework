// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
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


/// HIPO bank wrapper for `AHDC::ai:prediction` (group 23000, item 30).
/// Prediction given by AI
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// AHDC_ai_prediction bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::ai:prediction")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto x1 = row.x1();
/// }
/// @endcode
class AHDC_ai_prediction {
 public:
    static constexpr std::string_view bank_name = "AHDC::ai:prediction";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 30;

    class Row;

    AHDC_ai_prediction() = default;

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
            hipo::column_handle h_x1;
            hipo::column_handle h_y1;
            hipo::column_handle h_x2;
            hipo::column_handle h_y2;
            hipo::column_handle h_x3;
            hipo::column_handle h_y3;
            hipo::column_handle h_x4;
            hipo::column_handle h_y4;
            hipo::column_handle h_x5;
            hipo::column_handle h_y5;
            hipo::column_handle h_pred;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("x1"); h) cache.h_x1 = *h;
            else cache.h_x1 = {};
            if (auto h = schema.get_handle("y1"); h) cache.h_y1 = *h;
            else cache.h_y1 = {};
            if (auto h = schema.get_handle("x2"); h) cache.h_x2 = *h;
            else cache.h_x2 = {};
            if (auto h = schema.get_handle("y2"); h) cache.h_y2 = *h;
            else cache.h_y2 = {};
            if (auto h = schema.get_handle("x3"); h) cache.h_x3 = *h;
            else cache.h_x3 = {};
            if (auto h = schema.get_handle("y3"); h) cache.h_y3 = *h;
            else cache.h_y3 = {};
            if (auto h = schema.get_handle("x4"); h) cache.h_x4 = *h;
            else cache.h_x4 = {};
            if (auto h = schema.get_handle("y4"); h) cache.h_y4 = *h;
            else cache.h_y4 = {};
            if (auto h = schema.get_handle("x5"); h) cache.h_x5 = *h;
            else cache.h_x5 = {};
            if (auto h = schema.get_handle("y5"); h) cache.h_y5 = *h;
            else cache.h_y5 = {};
            if (auto h = schema.get_handle("pred"); h) cache.h_pred = *h;
            else cache.h_pred = {};
        }
        h_x1 = cache.h_x1;
        h_y1 = cache.h_y1;
        h_x2 = cache.h_x2;
        h_y2 = cache.h_y2;
        h_x3 = cache.h_x3;
        h_y3 = cache.h_y3;
        h_x4 = cache.h_x4;
        h_y4 = cache.h_y4;
        h_x5 = cache.h_x5;
        h_y5 = cache.h_y5;
        h_pred = cache.h_pred;
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
    /// x1: X1 position of the 1th superprecluster (mm)
    [[nodiscard]] float x1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x1, row));
    }
    /// y1: Y1 position of the 1th superprecluster (mm)
    [[nodiscard]] float y1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y1, row));
    }
    /// x2: X2 position of the 2nd superprecluster (mm)
    [[nodiscard]] float x2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x2, row));
    }
    /// y2: Y2 position of the 2nd superprecluster (mm)
    [[nodiscard]] float y2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y2, row));
    }
    /// x3: X3 position of the 3rd superprecluster (mm)
    [[nodiscard]] float x3(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x3, row));
    }
    /// y3: Y3 position of the 3rd superprecluster (mm)
    [[nodiscard]] float y3(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y3, row));
    }
    /// x4: X4 position of the 4th superprecluster (mm)
    [[nodiscard]] float x4(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x4, row));
    }
    /// y4: Y4 position of the 4th superprecluster (mm)
    [[nodiscard]] float y4(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y4, row));
    }
    /// x5: X5 position of the 5th superprecluster (mm)
    [[nodiscard]] float x5(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x5, row));
    }
    /// y5: Y5 position of the 5th superprecluster (mm)
    [[nodiscard]] float y5(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y5, row));
    }
    /// pred: Prediction of the model: 0 mean bad track; 1 mean good track
    [[nodiscard]] float pred(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pred, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// x1 column as a contiguous span.
    [[nodiscard]] std::span<const float> x1_col() const noexcept {
        return m_view.column<float>(h_x1);
    }
    /// y1 column as a contiguous span.
    [[nodiscard]] std::span<const float> y1_col() const noexcept {
        return m_view.column<float>(h_y1);
    }
    /// x2 column as a contiguous span.
    [[nodiscard]] std::span<const float> x2_col() const noexcept {
        return m_view.column<float>(h_x2);
    }
    /// y2 column as a contiguous span.
    [[nodiscard]] std::span<const float> y2_col() const noexcept {
        return m_view.column<float>(h_y2);
    }
    /// x3 column as a contiguous span.
    [[nodiscard]] std::span<const float> x3_col() const noexcept {
        return m_view.column<float>(h_x3);
    }
    /// y3 column as a contiguous span.
    [[nodiscard]] std::span<const float> y3_col() const noexcept {
        return m_view.column<float>(h_y3);
    }
    /// x4 column as a contiguous span.
    [[nodiscard]] std::span<const float> x4_col() const noexcept {
        return m_view.column<float>(h_x4);
    }
    /// y4 column as a contiguous span.
    [[nodiscard]] std::span<const float> y4_col() const noexcept {
        return m_view.column<float>(h_y4);
    }
    /// x5 column as a contiguous span.
    [[nodiscard]] std::span<const float> x5_col() const noexcept {
        return m_view.column<float>(h_x5);
    }
    /// y5 column as a contiguous span.
    [[nodiscard]] std::span<const float> y5_col() const noexcept {
        return m_view.column<float>(h_y5);
    }
    /// pred column as a contiguous span.
    [[nodiscard]] std::span<const float> pred_col() const noexcept {
        return m_view.column<float>(h_pred);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_x1{};
    hipo::column_handle h_y1{};
    hipo::column_handle h_x2{};
    hipo::column_handle h_y2{};
    hipo::column_handle h_x3{};
    hipo::column_handle h_y3{};
    hipo::column_handle h_x4{};
    hipo::column_handle h_y4{};
    hipo::column_handle h_x5{};
    hipo::column_handle h_y5{};
    hipo::column_handle h_pred{};

    friend class Row;
};

/// Lightweight row view for AHDC::ai:prediction. Stores a pointer + row index only.
class AHDC_ai_prediction::Row {
 public:
    Row(const AHDC_ai_prediction* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float x1() const noexcept { return m_bank->x1(m_row); }
    [[nodiscard]] float y1() const noexcept { return m_bank->y1(m_row); }
    [[nodiscard]] float x2() const noexcept { return m_bank->x2(m_row); }
    [[nodiscard]] float y2() const noexcept { return m_bank->y2(m_row); }
    [[nodiscard]] float x3() const noexcept { return m_bank->x3(m_row); }
    [[nodiscard]] float y3() const noexcept { return m_bank->y3(m_row); }
    [[nodiscard]] float x4() const noexcept { return m_bank->x4(m_row); }
    [[nodiscard]] float y4() const noexcept { return m_bank->y4(m_row); }
    [[nodiscard]] float x5() const noexcept { return m_bank->x5(m_row); }
    [[nodiscard]] float y5() const noexcept { return m_bank->y5(m_row); }
    [[nodiscard]] float pred() const noexcept { return m_bank->pred(m_row); }
 private:
    const AHDC_ai_prediction* m_bank;
    int m_row;
};

inline AHDC_ai_prediction::Row AHDC_ai_prediction::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
