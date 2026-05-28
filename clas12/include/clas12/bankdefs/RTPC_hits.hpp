// =============================================================================
// Auto-generated from rtpc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RTPC::hits` (group 21900, item 21).
/// reconstructed RTPC hits
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
            int i_trkID = -1;
            int i_id = -1;
            int i_time = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_adc = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_adc = schema.exists("adc") ? schema.getEntryOrder("adc") : -1;
        }
        i_trkID = cache.i_trkID;
        i_id = cache.i_id;
        i_time = cache.i_time;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_adc = cache.i_adc;
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
    /// trkID: Track ID of the hit
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trkID, row));
    }
    /// id: cell id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_id, row));
    }
    /// time: Time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// x: x pos (mm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: y pos (mm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: z pos (mm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// adc: adc
    [[nodiscard]] float adc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_adc, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_trkID = -1;
    int i_id = -1;
    int i_time = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_adc = -1;

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

}  // namespace bankdefs
