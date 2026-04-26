// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
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


/// HIPO bank wrapper for `RECAI::VertDoca` (group 400, item 39).
/// Track Cross information for Particles bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RECAI_VertDoca bank;
/// if (auto sc = bank.bind(event.get_bank("RECAI::VertDoca")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index1 = row.index1();
/// }
/// @endcode
class RECAI_VertDoca {
 public:
    static constexpr std::string_view bank_name = "RECAI::VertDoca";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 39;

    class Row;

    RECAI_VertDoca() = default;

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
            hipo::column_handle h_index1;
            hipo::column_handle h_index2;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_x1;
            hipo::column_handle h_y1;
            hipo::column_handle h_z1;
            hipo::column_handle h_cx1;
            hipo::column_handle h_cy1;
            hipo::column_handle h_cz1;
            hipo::column_handle h_x2;
            hipo::column_handle h_y2;
            hipo::column_handle h_z2;
            hipo::column_handle h_cx2;
            hipo::column_handle h_cy2;
            hipo::column_handle h_cz2;
            hipo::column_handle h_r;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("index1"); h) cache.h_index1 = *h;
            else cache.h_index1 = {};
            if (auto h = schema.get_handle("index2"); h) cache.h_index2 = *h;
            else cache.h_index2 = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("x1"); h) cache.h_x1 = *h;
            else cache.h_x1 = {};
            if (auto h = schema.get_handle("y1"); h) cache.h_y1 = *h;
            else cache.h_y1 = {};
            if (auto h = schema.get_handle("z1"); h) cache.h_z1 = *h;
            else cache.h_z1 = {};
            if (auto h = schema.get_handle("cx1"); h) cache.h_cx1 = *h;
            else cache.h_cx1 = {};
            if (auto h = schema.get_handle("cy1"); h) cache.h_cy1 = *h;
            else cache.h_cy1 = {};
            if (auto h = schema.get_handle("cz1"); h) cache.h_cz1 = *h;
            else cache.h_cz1 = {};
            if (auto h = schema.get_handle("x2"); h) cache.h_x2 = *h;
            else cache.h_x2 = {};
            if (auto h = schema.get_handle("y2"); h) cache.h_y2 = *h;
            else cache.h_y2 = {};
            if (auto h = schema.get_handle("z2"); h) cache.h_z2 = *h;
            else cache.h_z2 = {};
            if (auto h = schema.get_handle("cx2"); h) cache.h_cx2 = *h;
            else cache.h_cx2 = {};
            if (auto h = schema.get_handle("cy2"); h) cache.h_cy2 = *h;
            else cache.h_cy2 = {};
            if (auto h = schema.get_handle("cz2"); h) cache.h_cz2 = *h;
            else cache.h_cz2 = {};
            if (auto h = schema.get_handle("r"); h) cache.h_r = *h;
            else cache.h_r = {};
        }
        h_index1 = cache.h_index1;
        h_index2 = cache.h_index2;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_x1 = cache.h_x1;
        h_y1 = cache.h_y1;
        h_z1 = cache.h_z1;
        h_cx1 = cache.h_cx1;
        h_cy1 = cache.h_cy1;
        h_cz1 = cache.h_cz1;
        h_x2 = cache.h_x2;
        h_y2 = cache.h_y2;
        h_z2 = cache.h_z2;
        h_cx2 = cache.h_cx2;
        h_cy2 = cache.h_cy2;
        h_cz2 = cache.h_cz2;
        h_r = cache.h_r;
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
    /// index1: index of the first track in Tracks bank
    [[nodiscard]] int index1(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index1, row));
    }
    /// index2: index of the second track in Tracks bank
    [[nodiscard]] int index2(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index2, row));
    }
    /// x: x-position of the common vertex (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: y-position of the common vertex (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: z-position of the common vertex (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// x1: x-position of the first track at the DOCA point (cm)
    [[nodiscard]] float x1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x1, row));
    }
    /// y1: y-position of the first track at the DOCA point (cm)
    [[nodiscard]] float y1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y1, row));
    }
    /// z1: z-position of the first track at the DOCA point (cm)
    [[nodiscard]] float z1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z1, row));
    }
    /// cx1: x-direction vector of the first track at the DOCA point (cm)
    [[nodiscard]] float cx1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cx1, row));
    }
    /// cy1: y-direction vector of the first track at the DOCA point (cm)
    [[nodiscard]] float cy1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cy1, row));
    }
    /// cz1: z-direction vector of the first track at the DOCA point (cm)
    [[nodiscard]] float cz1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cz1, row));
    }
    /// x2: x-position of the second track at the DOCA point (cm)
    [[nodiscard]] float x2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x2, row));
    }
    /// y2: y-position of the second track at the DOCA point (cm)
    [[nodiscard]] float y2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y2, row));
    }
    /// z2: z-position of the second track at the DOCA point (cm)
    [[nodiscard]] float z2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z2, row));
    }
    /// cx2: x-direction vector of the second track at the DOCA point (cm)
    [[nodiscard]] float cx2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cx2, row));
    }
    /// cy2: y-direction vector of the second track at the DOCA point (cm)
    [[nodiscard]] float cy2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cy2, row));
    }
    /// cz2: z-direction vector of the second track at the DOCA point (cm)
    [[nodiscard]] float cz2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cz2, row));
    }
    /// r: The distance between two tracks (cm)
    [[nodiscard]] float r(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_r, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index1 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index1_col() const noexcept {
        return m_view.column<int16_t>(h_index1);
    }
    /// index2 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index2_col() const noexcept {
        return m_view.column<int16_t>(h_index2);
    }
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
    /// x1 column as a contiguous span.
    [[nodiscard]] std::span<const float> x1_col() const noexcept {
        return m_view.column<float>(h_x1);
    }
    /// y1 column as a contiguous span.
    [[nodiscard]] std::span<const float> y1_col() const noexcept {
        return m_view.column<float>(h_y1);
    }
    /// z1 column as a contiguous span.
    [[nodiscard]] std::span<const float> z1_col() const noexcept {
        return m_view.column<float>(h_z1);
    }
    /// cx1 column as a contiguous span.
    [[nodiscard]] std::span<const float> cx1_col() const noexcept {
        return m_view.column<float>(h_cx1);
    }
    /// cy1 column as a contiguous span.
    [[nodiscard]] std::span<const float> cy1_col() const noexcept {
        return m_view.column<float>(h_cy1);
    }
    /// cz1 column as a contiguous span.
    [[nodiscard]] std::span<const float> cz1_col() const noexcept {
        return m_view.column<float>(h_cz1);
    }
    /// x2 column as a contiguous span.
    [[nodiscard]] std::span<const float> x2_col() const noexcept {
        return m_view.column<float>(h_x2);
    }
    /// y2 column as a contiguous span.
    [[nodiscard]] std::span<const float> y2_col() const noexcept {
        return m_view.column<float>(h_y2);
    }
    /// z2 column as a contiguous span.
    [[nodiscard]] std::span<const float> z2_col() const noexcept {
        return m_view.column<float>(h_z2);
    }
    /// cx2 column as a contiguous span.
    [[nodiscard]] std::span<const float> cx2_col() const noexcept {
        return m_view.column<float>(h_cx2);
    }
    /// cy2 column as a contiguous span.
    [[nodiscard]] std::span<const float> cy2_col() const noexcept {
        return m_view.column<float>(h_cy2);
    }
    /// cz2 column as a contiguous span.
    [[nodiscard]] std::span<const float> cz2_col() const noexcept {
        return m_view.column<float>(h_cz2);
    }
    /// r column as a contiguous span.
    [[nodiscard]] std::span<const float> r_col() const noexcept {
        return m_view.column<float>(h_r);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_index1{};
    hipo::column_handle h_index2{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_x1{};
    hipo::column_handle h_y1{};
    hipo::column_handle h_z1{};
    hipo::column_handle h_cx1{};
    hipo::column_handle h_cy1{};
    hipo::column_handle h_cz1{};
    hipo::column_handle h_x2{};
    hipo::column_handle h_y2{};
    hipo::column_handle h_z2{};
    hipo::column_handle h_cx2{};
    hipo::column_handle h_cy2{};
    hipo::column_handle h_cz2{};
    hipo::column_handle h_r{};

    friend class Row;
};

/// Lightweight row view for RECAI::VertDoca. Stores a pointer + row index only.
class RECAI_VertDoca::Row {
 public:
    Row(const RECAI_VertDoca* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index1() const noexcept { return m_bank->index1(m_row); }
    [[nodiscard]] int index2() const noexcept { return m_bank->index2(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float x1() const noexcept { return m_bank->x1(m_row); }
    [[nodiscard]] float y1() const noexcept { return m_bank->y1(m_row); }
    [[nodiscard]] float z1() const noexcept { return m_bank->z1(m_row); }
    [[nodiscard]] float cx1() const noexcept { return m_bank->cx1(m_row); }
    [[nodiscard]] float cy1() const noexcept { return m_bank->cy1(m_row); }
    [[nodiscard]] float cz1() const noexcept { return m_bank->cz1(m_row); }
    [[nodiscard]] float x2() const noexcept { return m_bank->x2(m_row); }
    [[nodiscard]] float y2() const noexcept { return m_bank->y2(m_row); }
    [[nodiscard]] float z2() const noexcept { return m_bank->z2(m_row); }
    [[nodiscard]] float cx2() const noexcept { return m_bank->cx2(m_row); }
    [[nodiscard]] float cy2() const noexcept { return m_bank->cy2(m_row); }
    [[nodiscard]] float cz2() const noexcept { return m_bank->cz2(m_row); }
    [[nodiscard]] float r() const noexcept { return m_bank->r(m_row); }
 private:
    const RECAI_VertDoca* m_bank;
    int m_row;
};

inline RECAI_VertDoca::Row RECAI_VertDoca::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
