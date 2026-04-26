// =============================================================================
// Auto-generated from rtpc.json - do not edit by hand.
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


/// HIPO bank wrapper for `RTPC::hits` (group 21900, item 21).
/// reconstructed RTPC hits
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RTPC_hits bank;
/// if (auto sc = bank.bind(event.get_bank("RTPC::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trkID = row.trkID();
/// }
/// @endcode
class RTPC_hits {
 public:
    static constexpr std::string_view bank_name = "RTPC::hits";
    static constexpr int bank_group = 21900;
    static constexpr int bank_item  = 21;

    class Row;

    RTPC_hits() = default;

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
            hipo::column_handle h_trkID;
            hipo::column_handle h_id;
            hipo::column_handle h_time;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_adc;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("trkID"); h) cache.h_trkID = *h;
            else cache.h_trkID = {};
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("adc"); h) cache.h_adc = *h;
            else cache.h_adc = {};
        }
        h_trkID = cache.h_trkID;
        h_id = cache.h_id;
        h_time = cache.h_time;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_adc = cache.h_adc;
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
    /// trkID: Track ID of the hit
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trkID, row));
    }
    /// id: cell id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_id, row));
    }
    /// time: Time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// x: x pos (mm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: y pos (mm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: z pos (mm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// adc: adc
    [[nodiscard]] float adc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_adc, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trkID_col() const noexcept {
        return m_view.column<int32_t>(h_trkID);
    }
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> id_col() const noexcept {
        return m_view.column<int32_t>(h_id);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
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
    /// adc column as a contiguous span.
    [[nodiscard]] std::span<const float> adc_col() const noexcept {
        return m_view.column<float>(h_adc);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_trkID{};
    hipo::column_handle h_id{};
    hipo::column_handle h_time{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_adc{};

    friend class Row;
};

/// Lightweight row view for RTPC::hits. Stores a pointer + row index only.
class RTPC_hits::Row {
 public:
    Row(const RTPC_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float adc() const noexcept { return m_bank->adc(m_row); }
 private:
    const RTPC_hits* m_bank;
    int m_row;
};

inline RTPC_hits::Row RTPC_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
