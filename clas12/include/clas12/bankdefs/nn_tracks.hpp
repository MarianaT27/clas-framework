// =============================================================================
// Auto-generated from neuralnetwork.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `nn::tracks` (group 4200, item 1).
/// track parameters from neural network
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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

    /// Bind to a live hipo::bank. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column name to its item
    /// index via schema.getEntryOrder(); subsequent bind()s for the same
    /// schema hit the cache. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            const hipo::schema* schema_ptr = nullptr;
            int i_id = -1;
            int i_x = -1;
            int i_y = -1;
            int i_tx = -1;
            int i_ty = -1;
            int i_qp = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_tx = schema.exists("tx") ? schema.getEntryOrder("tx") : -1;
            cache.i_ty = schema.exists("ty") ? schema.getEntryOrder("ty") : -1;
            cache.i_qp = schema.exists("qp") ? schema.getEntryOrder("qp") : -1;
        }
        i_id = cache.i_id;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_tx = cache.i_tx;
        i_ty = cache.i_ty;
        i_qp = cache.i_qp;
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
    /// id: track id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_id, row));
    }
    /// x: x coordinate in tilted coordinates
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y coordinate in tilted coordinates
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// tx: tx
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tx, row));
    }
    /// ty: ty
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ty, row));
    }
    /// qp: q over p
    [[nodiscard]] float qp(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_qp, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_x = -1;
    int i_y = -1;
    int i_tx = -1;
    int i_ty = -1;
    int i_qp = -1;

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

}  // namespace bankdefs
