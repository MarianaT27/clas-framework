// =============================================================================
// Auto-generated from rich.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RICH::Signal` (group 21800, item 41).
/// Reconstructed Signals in RICH
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RICH_Signal bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Signal")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Signal {
 public:
    static constexpr std::string_view bank_name = "RICH::Signal";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 41;

    class Row;

    RICH_Signal() = default;

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
            int i_hindex = -1;
            int i_sector = -1;
            int i_pmt = -1;
            int i_anode = -1;
            int i_size = -1;
            int i_status = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_time = -1;
            int i_rawtime = -1;
            int i_charge = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_hindex = schema.exists("hindex") ? schema.getEntryOrder("hindex") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_pmt = schema.exists("pmt") ? schema.getEntryOrder("pmt") : -1;
            cache.i_anode = schema.exists("anode") ? schema.getEntryOrder("anode") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_rawtime = schema.exists("rawtime") ? schema.getEntryOrder("rawtime") : -1;
            cache.i_charge = schema.exists("charge") ? schema.getEntryOrder("charge") : -1;
        }
        i_id = cache.i_id;
        i_hindex = cache.i_hindex;
        i_sector = cache.i_sector;
        i_pmt = cache.i_pmt;
        i_anode = cache.i_anode;
        i_size = cache.i_size;
        i_status = cache.i_status;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_time = cache.i_time;
        i_rawtime = cache.i_rawtime;
        i_charge = cache.i_charge;
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
    /// id: Signal id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// hindex: Signal hindex
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_hindex, row));
    }
    /// sector: Signal sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_sector, row));
    }
    /// pmt: Signal pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pmt, row));
    }
    /// anode: Signal anode
    [[nodiscard]] int anode(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_anode, row));
    }
    /// size: Signal size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// status: Signal status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// x: Signal x
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Signal y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Signal z
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// time: Signal time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// rawtime: Signal rawtime
    [[nodiscard]] float rawtime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_rawtime, row));
    }
    /// charge: Signal charge
    [[nodiscard]] float charge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_charge, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int hindex_handle() const noexcept { return i_hindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_hindex = -1;
    int i_sector = -1;
    int i_pmt = -1;
    int i_anode = -1;
    int i_size = -1;
    int i_status = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_time = -1;
    int i_rawtime = -1;
    int i_charge = -1;

    friend class Row;
};

/// Lightweight row view for RICH::Signal. Stores a pointer + row index only.
class RICH_Signal::Row {
 public:
    Row(const RICH_Signal* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] int anode() const noexcept { return m_bank->anode(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float rawtime() const noexcept { return m_bank->rawtime(m_row); }
    [[nodiscard]] float charge() const noexcept { return m_bank->charge(m_row); }
 private:
    const RICH_Signal* m_bank;
    int m_row;
};

inline RICH_Signal::Row RICH_Signal::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
