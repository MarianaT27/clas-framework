// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `HitBasedTrkg::AIHitTrkId` (group 20600, item 67).
/// pointers between track and hit banks
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
            int i_id = -1;
            int i_tid = -1;
            int i_B = -1;
            int i_TProp = -1;
            int i_TFlight = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_tid = schema.exists("tid") ? schema.getEntryOrder("tid") : -1;
            cache.i_B = schema.exists("B") ? schema.getEntryOrder("B") : -1;
            cache.i_TProp = schema.exists("TProp") ? schema.getEntryOrder("TProp") : -1;
            cache.i_TFlight = schema.exists("TFlight") ? schema.getEntryOrder("TFlight") : -1;
        }
        i_id = cache.i_id;
        i_tid = cache.i_tid;
        i_B = cache.i_B;
        i_TProp = cache.i_TProp;
        i_TFlight = cache.i_TFlight;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// tid: id of the track
    [[nodiscard]] int tid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_tid, row));
    }
    /// B: B-field intensity at hit position in tilted-sector system
    [[nodiscard]] float B(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_B, row));
    }
    /// TProp: t propagation along the wire
    [[nodiscard]] float TProp(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_TProp, row));
    }
    /// TFlight: time of flight correction
    [[nodiscard]] float TFlight(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_TFlight, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_tid = -1;
    int i_B = -1;
    int i_TProp = -1;
    int i_TFlight = -1;

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
