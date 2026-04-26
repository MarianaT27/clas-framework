// =============================================================================
// Auto-generated from neuralnetwork.json - do not edit by hand.
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


/// HIPO bank wrapper for `nn::tracks` (group 4200, item 1).
/// track parameters from neural network
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// nn_tracks bank;
/// if (auto sc = bank.bind(event.get_bank("nn::tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class nn_tracks {
 public:
    static constexpr std::string_view bank_name = "nn::tracks";
    static constexpr int bank_group = 4200;
    static constexpr int bank_item  = 1;

    class Row;

    nn_tracks() = default;

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
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_tx;
            hipo::column_handle h_ty;
            hipo::column_handle h_qp;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("tx"); h) cache.h_tx = *h;
            else cache.h_tx = {};
            if (auto h = schema.get_handle("ty"); h) cache.h_ty = *h;
            else cache.h_ty = {};
            if (auto h = schema.get_handle("qp"); h) cache.h_qp = *h;
            else cache.h_qp = {};
        }
        h_id = cache.h_id;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_tx = cache.h_tx;
        h_ty = cache.h_ty;
        h_qp = cache.h_qp;
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
    /// id: track id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_id, row));
    }
    /// x: x coordinate in tilted coordinates
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: y coordinate in tilted coordinates
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// tx: tx
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tx, row));
    }
    /// ty: ty
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ty, row));
    }
    /// qp: q over p
    [[nodiscard]] float qp(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_qp, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> id_col() const noexcept {
        return m_view.column<int8_t>(h_id);
    }
    /// x column as a contiguous span.
    [[nodiscard]] std::span<const float> x_col() const noexcept {
        return m_view.column<float>(h_x);
    }
    /// y column as a contiguous span.
    [[nodiscard]] std::span<const float> y_col() const noexcept {
        return m_view.column<float>(h_y);
    }
    /// tx column as a contiguous span.
    [[nodiscard]] std::span<const float> tx_col() const noexcept {
        return m_view.column<float>(h_tx);
    }
    /// ty column as a contiguous span.
    [[nodiscard]] std::span<const float> ty_col() const noexcept {
        return m_view.column<float>(h_ty);
    }
    /// qp column as a contiguous span.
    [[nodiscard]] std::span<const float> qp_col() const noexcept {
        return m_view.column<float>(h_qp);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_tx{};
    hipo::column_handle h_ty{};
    hipo::column_handle h_qp{};

    friend class Row;
};

/// Lightweight row view for nn::tracks. Stores a pointer + row index only.
class nn_tracks::Row {
 public:
    Row(const nn_tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float qp() const noexcept { return m_bank->qp(m_row); }
 private:
    const nn_tracks* m_bank;
    int m_row;
};

inline nn_tracks::Row nn_tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
