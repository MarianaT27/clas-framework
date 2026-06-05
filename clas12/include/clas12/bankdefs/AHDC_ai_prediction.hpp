// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `AHDC::ai:prediction` (group 23000, item 30).
/// Prediction given by AI
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
    static constexpr std::string_view bank_schema = "x1/F,y1/F,x2/F,y2/F,x3/F,y3/F,x4/F,y4/F,x5/F,y5/F,pred/F";

    class Row;

    AHDC_ai_prediction() = default;

    /// Bind to a live hipo::bank. Per-thread cache keyed by the schema's
    /// (group, item): the first bind() in a thread resolves every column
    /// name to its item index via schema.getEntryOrder(); subsequent
    /// bind()s hit the cache. Keying by (group,item) rather than the schema
    /// POINTER is critical: the framework constructs a fresh hipo::bank
    /// (copying its schema to a new address) every event, so a pointer key
    /// would miss every event and re-resolve all columns by string. (group,
    /// item) is stable across those copies, so resolution is paid once per
    /// thread per bank type. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            int gi_group = -1;
            int gi_item  = -2;
            int i_x1 = -1;
            int i_y1 = -1;
            int i_x2 = -1;
            int i_y2 = -1;
            int i_x3 = -1;
            int i_y3 = -1;
            int i_x4 = -1;
            int i_y4 = -1;
            int i_x5 = -1;
            int i_y5 = -1;
            int i_pred = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_x1 = schema.exists("x1") ? schema.getEntryOrder("x1") : -1;
            cache.i_y1 = schema.exists("y1") ? schema.getEntryOrder("y1") : -1;
            cache.i_x2 = schema.exists("x2") ? schema.getEntryOrder("x2") : -1;
            cache.i_y2 = schema.exists("y2") ? schema.getEntryOrder("y2") : -1;
            cache.i_x3 = schema.exists("x3") ? schema.getEntryOrder("x3") : -1;
            cache.i_y3 = schema.exists("y3") ? schema.getEntryOrder("y3") : -1;
            cache.i_x4 = schema.exists("x4") ? schema.getEntryOrder("x4") : -1;
            cache.i_y4 = schema.exists("y4") ? schema.getEntryOrder("y4") : -1;
            cache.i_x5 = schema.exists("x5") ? schema.getEntryOrder("x5") : -1;
            cache.i_y5 = schema.exists("y5") ? schema.getEntryOrder("y5") : -1;
            cache.i_pred = schema.exists("pred") ? schema.getEntryOrder("pred") : -1;
        }
        i_x1 = cache.i_x1;
        i_y1 = cache.i_y1;
        i_x2 = cache.i_x2;
        i_y2 = cache.i_y2;
        i_x3 = cache.i_x3;
        i_y3 = cache.i_y3;
        i_x4 = cache.i_x4;
        i_y4 = cache.i_y4;
        i_x5 = cache.i_x5;
        i_y5 = cache.i_y5;
        i_pred = cache.i_pred;
        return true;
    }

    /// Current row count of the bound bank (0 if unbound).
    [[nodiscard]] int rows() const noexcept { return m_view ? m_view->getRows() : 0; }

    /// True if bind() has been called.
    [[nodiscard]] bool valid() const noexcept { return m_view != nullptr; }

    /// Access the underlying hipo::bank (for advanced use).
    [[nodiscard]] const hipo::bank& view() const noexcept { return *m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (item-index based, no string lookup)
    // -------------------------------------------------------------------
    /// x1: X1 position of the 1th superprecluster (mm)
    [[nodiscard]] float x1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x1, row));
    }
    /// y1: Y1 position of the 1th superprecluster (mm)
    [[nodiscard]] float y1(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y1, row));
    }
    /// x2: X2 position of the 2nd superprecluster (mm)
    [[nodiscard]] float x2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x2, row));
    }
    /// y2: Y2 position of the 2nd superprecluster (mm)
    [[nodiscard]] float y2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y2, row));
    }
    /// x3: X3 position of the 3rd superprecluster (mm)
    [[nodiscard]] float x3(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x3, row));
    }
    /// y3: Y3 position of the 3rd superprecluster (mm)
    [[nodiscard]] float y3(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y3, row));
    }
    /// x4: X4 position of the 4th superprecluster (mm)
    [[nodiscard]] float x4(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x4, row));
    }
    /// y4: Y4 position of the 4th superprecluster (mm)
    [[nodiscard]] float y4(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y4, row));
    }
    /// x5: X5 position of the 5th superprecluster (mm)
    [[nodiscard]] float x5(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x5, row));
    }
    /// y5: Y5 position of the 5th superprecluster (mm)
    [[nodiscard]] float y5(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y5, row));
    }
    /// pred: Prediction of the model: 0 mean bad track; 1 mean good track
    [[nodiscard]] float pred(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pred, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_x1 = -1;
    int i_y1 = -1;
    int i_x2 = -1;
    int i_y2 = -1;
    int i_x3 = -1;
    int i_y3 = -1;
    int i_x4 = -1;
    int i_y4 = -1;
    int i_x5 = -1;
    int i_y5 = -1;
    int i_pred = -1;

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

}  // namespace bankdefs
