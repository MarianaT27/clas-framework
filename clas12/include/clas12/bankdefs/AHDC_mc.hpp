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


/// HIPO bank wrapper for `AHDC::mc` (group 23000, item 22).
/// MC Tracks
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// AHDC_mc bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::mc")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto x = row.x();
/// }
/// @endcode
class AHDC_mc {
 public:
    static constexpr std::string_view bank_name = "AHDC::mc";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 22;

    class Row;

    AHDC_mc() = default;

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
            hipo::column_handle h_px;
            hipo::column_handle h_py;
            hipo::column_handle h_pz;
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
            if (auto h = schema.get_handle("px"); h) cache.h_px = *h;
            else cache.h_px = {};
            if (auto h = schema.get_handle("py"); h) cache.h_py = *h;
            else cache.h_py = {};
            if (auto h = schema.get_handle("pz"); h) cache.h_pz = *h;
            else cache.h_pz = {};
        }
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_px = cache.h_px;
        h_py = cache.h_py;
        h_pz = cache.h_pz;
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
    /// x: x position in mm
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: y position in mm
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: z position in mm
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// px: px in MeV
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: py in MeV
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: pz in MeV
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
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
    /// px column as a contiguous span.
    [[nodiscard]] std::span<const float> px_col() const noexcept {
        return m_view.column<float>(h_px);
    }
    /// py column as a contiguous span.
    [[nodiscard]] std::span<const float> py_col() const noexcept {
        return m_view.column<float>(h_py);
    }
    /// pz column as a contiguous span.
    [[nodiscard]] std::span<const float> pz_col() const noexcept {
        return m_view.column<float>(h_pz);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};

    friend class Row;
};

/// Lightweight row view for AHDC::mc. Stores a pointer + row index only.
class AHDC_mc::Row {
 public:
    Row(const AHDC_mc* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
 private:
    const AHDC_mc* m_bank;
    int m_row;
};

inline AHDC_mc::Row AHDC_mc::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
