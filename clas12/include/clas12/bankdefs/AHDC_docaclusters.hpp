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


/// HIPO bank wrapper for `AHDC::docaclusters` (group 23000, item 126).
/// Doca-refined cluster space points
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// AHDC_docaclusters bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::docaclusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto x = row.x();
/// }
/// @endcode
class AHDC_docaclusters {
 public:
    static constexpr std::string_view bank_name = "AHDC::docaclusters";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 126;

    class Row;

    AHDC_docaclusters() = default;

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
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_weight;
            hipo::column_handle h_pattern;
            hipo::column_handle h_idx;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("weight"); h) cache.h_weight = *h;
            else cache.h_weight = {};
            if (auto h = schema.get_handle("pattern"); h) cache.h_pattern = *h;
            else cache.h_pattern = {};
            if (auto h = schema.get_handle("idx"); h) cache.h_idx = *h;
            else cache.h_idx = {};
        }
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_weight = cache.h_weight;
        h_pattern = cache.h_pattern;
        h_idx = cache.h_idx;
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
    /// x: refined x (mm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: refined y (mm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: refined z (mm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// weight: refined point weight
    [[nodiscard]] float weight(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_weight, row));
    }
    /// pattern: hit pattern: 11,12,21,22
    [[nodiscard]] int pattern(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pattern, row));
    }
    /// idx: index of original AHDC::clusters row
    [[nodiscard]] int idx(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_idx, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// x column as a contiguous span.
    [[nodiscard]] std::span<const float> x_col() const noexcept {
        return m_view.column<float>(h_x);
    }
    /// y column as a contiguous span.
    [[nodiscard]] std::span<const float> y_col() const noexcept {
        return m_view.column<float>(h_y);
    }
    /// z column as a contiguous span.
    [[nodiscard]] std::span<const float> z_col() const noexcept {
        return m_view.column<float>(h_z);
    }
    /// weight column as a contiguous span.
    [[nodiscard]] std::span<const float> weight_col() const noexcept {
        return m_view.column<float>(h_weight);
    }
    /// pattern column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pattern_col() const noexcept {
        return m_view.column<int32_t>(h_pattern);
    }
    /// idx column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> idx_col() const noexcept {
        return m_view.column<int32_t>(h_idx);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_weight{};
    hipo::column_handle h_pattern{};
    hipo::column_handle h_idx{};

    friend class Row;
};

/// Lightweight row view for AHDC::docaclusters. Stores a pointer + row index only.
class AHDC_docaclusters::Row {
 public:
    Row(const AHDC_docaclusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float weight() const noexcept { return m_bank->weight(m_row); }
    [[nodiscard]] int pattern() const noexcept { return m_bank->pattern(m_row); }
    [[nodiscard]] int idx() const noexcept { return m_bank->idx(m_row); }
 private:
    const AHDC_docaclusters* m_bank;
    int m_row;
};

inline AHDC_docaclusters::Row AHDC_docaclusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
