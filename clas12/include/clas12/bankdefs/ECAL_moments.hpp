// =============================================================================
// Auto-generated from ecal.json - do not edit by hand.
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


/// HIPO bank wrapper for `ECAL::moments` (group 20700, item 25).
/// ECCAL bank for clusters containing distances an moments
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ECAL_moments bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::moments")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto distU = row.distU();
/// }
/// @endcode
class ECAL_moments {
 public:
    static constexpr std::string_view bank_name = "ECAL::moments";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 25;

    class Row;

    ECAL_moments() = default;

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
            hipo::column_handle h_distU;
            hipo::column_handle h_distV;
            hipo::column_handle h_distW;
            hipo::column_handle h_m1u;
            hipo::column_handle h_m1v;
            hipo::column_handle h_m1w;
            hipo::column_handle h_m2u;
            hipo::column_handle h_m2v;
            hipo::column_handle h_m2w;
            hipo::column_handle h_m3u;
            hipo::column_handle h_m3v;
            hipo::column_handle h_m3w;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("distU"); h) cache.h_distU = *h;
            else cache.h_distU = {};
            if (auto h = schema.get_handle("distV"); h) cache.h_distV = *h;
            else cache.h_distV = {};
            if (auto h = schema.get_handle("distW"); h) cache.h_distW = *h;
            else cache.h_distW = {};
            if (auto h = schema.get_handle("m1u"); h) cache.h_m1u = *h;
            else cache.h_m1u = {};
            if (auto h = schema.get_handle("m1v"); h) cache.h_m1v = *h;
            else cache.h_m1v = {};
            if (auto h = schema.get_handle("m1w"); h) cache.h_m1w = *h;
            else cache.h_m1w = {};
            if (auto h = schema.get_handle("m2u"); h) cache.h_m2u = *h;
            else cache.h_m2u = {};
            if (auto h = schema.get_handle("m2v"); h) cache.h_m2v = *h;
            else cache.h_m2v = {};
            if (auto h = schema.get_handle("m2w"); h) cache.h_m2w = *h;
            else cache.h_m2w = {};
            if (auto h = schema.get_handle("m3u"); h) cache.h_m3u = *h;
            else cache.h_m3u = {};
            if (auto h = schema.get_handle("m3v"); h) cache.h_m3v = *h;
            else cache.h_m3v = {};
            if (auto h = schema.get_handle("m3w"); h) cache.h_m3w = *h;
            else cache.h_m3w = {};
        }
        h_distU = cache.h_distU;
        h_distV = cache.h_distV;
        h_distW = cache.h_distW;
        h_m1u = cache.h_m1u;
        h_m1v = cache.h_m1v;
        h_m1w = cache.h_m1w;
        h_m2u = cache.h_m2u;
        h_m2v = cache.h_m2v;
        h_m2w = cache.h_m2w;
        h_m3u = cache.h_m3u;
        h_m3v = cache.h_m3v;
        h_m3w = cache.h_m3w;
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
    /// distU: distance fomr u edge
    [[nodiscard]] float distU(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_distU, row));
    }
    /// distV: distance from v edge
    [[nodiscard]] float distV(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_distV, row));
    }
    /// distW: distance from w edge
    [[nodiscard]] float distW(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_distW, row));
    }
    /// m1u: second moment
    [[nodiscard]] float m1u(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m1u, row));
    }
    /// m1v: second moment
    [[nodiscard]] float m1v(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m1v, row));
    }
    /// m1w: second moment
    [[nodiscard]] float m1w(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m1w, row));
    }
    /// m2u: second moment
    [[nodiscard]] float m2u(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m2u, row));
    }
    /// m2v: second moment
    [[nodiscard]] float m2v(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m2v, row));
    }
    /// m2w: second moment
    [[nodiscard]] float m2w(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m2w, row));
    }
    /// m3u: third moment
    [[nodiscard]] float m3u(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m3u, row));
    }
    /// m3v: third moment
    [[nodiscard]] float m3v(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m3v, row));
    }
    /// m3w: third moment
    [[nodiscard]] float m3w(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_m3w, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// distU column as a contiguous span.
    [[nodiscard]] std::span<const float> distU_col() const noexcept {
        return m_view.column<float>(h_distU);
    }
    /// distV column as a contiguous span.
    [[nodiscard]] std::span<const float> distV_col() const noexcept {
        return m_view.column<float>(h_distV);
    }
    /// distW column as a contiguous span.
    [[nodiscard]] std::span<const float> distW_col() const noexcept {
        return m_view.column<float>(h_distW);
    }
    /// m1u column as a contiguous span.
    [[nodiscard]] std::span<const float> m1u_col() const noexcept {
        return m_view.column<float>(h_m1u);
    }
    /// m1v column as a contiguous span.
    [[nodiscard]] std::span<const float> m1v_col() const noexcept {
        return m_view.column<float>(h_m1v);
    }
    /// m1w column as a contiguous span.
    [[nodiscard]] std::span<const float> m1w_col() const noexcept {
        return m_view.column<float>(h_m1w);
    }
    /// m2u column as a contiguous span.
    [[nodiscard]] std::span<const float> m2u_col() const noexcept {
        return m_view.column<float>(h_m2u);
    }
    /// m2v column as a contiguous span.
    [[nodiscard]] std::span<const float> m2v_col() const noexcept {
        return m_view.column<float>(h_m2v);
    }
    /// m2w column as a contiguous span.
    [[nodiscard]] std::span<const float> m2w_col() const noexcept {
        return m_view.column<float>(h_m2w);
    }
    /// m3u column as a contiguous span.
    [[nodiscard]] std::span<const float> m3u_col() const noexcept {
        return m_view.column<float>(h_m3u);
    }
    /// m3v column as a contiguous span.
    [[nodiscard]] std::span<const float> m3v_col() const noexcept {
        return m_view.column<float>(h_m3v);
    }
    /// m3w column as a contiguous span.
    [[nodiscard]] std::span<const float> m3w_col() const noexcept {
        return m_view.column<float>(h_m3w);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_distU{};
    hipo::column_handle h_distV{};
    hipo::column_handle h_distW{};
    hipo::column_handle h_m1u{};
    hipo::column_handle h_m1v{};
    hipo::column_handle h_m1w{};
    hipo::column_handle h_m2u{};
    hipo::column_handle h_m2v{};
    hipo::column_handle h_m2w{};
    hipo::column_handle h_m3u{};
    hipo::column_handle h_m3v{};
    hipo::column_handle h_m3w{};

    friend class Row;
};

/// Lightweight row view for ECAL::moments. Stores a pointer + row index only.
class ECAL_moments::Row {
 public:
    Row(const ECAL_moments* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float distU() const noexcept { return m_bank->distU(m_row); }
    [[nodiscard]] float distV() const noexcept { return m_bank->distV(m_row); }
    [[nodiscard]] float distW() const noexcept { return m_bank->distW(m_row); }
    [[nodiscard]] float m1u() const noexcept { return m_bank->m1u(m_row); }
    [[nodiscard]] float m1v() const noexcept { return m_bank->m1v(m_row); }
    [[nodiscard]] float m1w() const noexcept { return m_bank->m1w(m_row); }
    [[nodiscard]] float m2u() const noexcept { return m_bank->m2u(m_row); }
    [[nodiscard]] float m2v() const noexcept { return m_bank->m2v(m_row); }
    [[nodiscard]] float m2w() const noexcept { return m_bank->m2w(m_row); }
    [[nodiscard]] float m3u() const noexcept { return m_bank->m3u(m_row); }
    [[nodiscard]] float m3v() const noexcept { return m_bank->m3v(m_row); }
    [[nodiscard]] float m3w() const noexcept { return m_bank->m3w(m_row); }
 private:
    const ECAL_moments* m_bank;
    int m_row;
};

inline ECAL_moments::Row ECAL_moments::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
