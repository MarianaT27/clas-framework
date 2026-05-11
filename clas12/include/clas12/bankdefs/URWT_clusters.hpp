// =============================================================================
// Auto-generated from urwell.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `URWT::clusters` (group 22300, item 22).
/// reconstructed clusters from URWT
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// URWT_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("URWT::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class URWT_clusters {
 public:
    static constexpr std::string_view bank_name = "URWT::clusters";
    static constexpr int bank_group = 22300;
    static constexpr int bank_item  = 22;

    class Row;

    URWT_clusters() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_strip = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_xo = -1;
            int i_yo = -1;
            int i_zo = -1;
            int i_xe = -1;
            int i_ye = -1;
            int i_ze = -1;
            int i_size = -1;
            int i_status = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_strip = schema.exists("strip") ? schema.getEntryOrder("strip") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_xo = schema.exists("xo") ? schema.getEntryOrder("xo") : -1;
            cache.i_yo = schema.exists("yo") ? schema.getEntryOrder("yo") : -1;
            cache.i_zo = schema.exists("zo") ? schema.getEntryOrder("zo") : -1;
            cache.i_xe = schema.exists("xe") ? schema.getEntryOrder("xe") : -1;
            cache.i_ye = schema.exists("ye") ? schema.getEntryOrder("ye") : -1;
            cache.i_ze = schema.exists("ze") ? schema.getEntryOrder("ze") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
        }
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_strip = cache.i_strip;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_xo = cache.i_xo;
        i_yo = cache.i_yo;
        i_zo = cache.i_zo;
        i_xe = cache.i_xe;
        i_ye = cache.i_ye;
        i_ze = cache.i_ze;
        i_size = cache.i_size;
        i_status = cache.i_status;
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
    /// id: id of the cluster
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// sector: sector of URWT
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer of URWT
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// strip: seed strip
    [[nodiscard]] int strip(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_strip, row));
    }
    /// energy: energy of the cluster (eV)
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: time of the cluster (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// xo: strip origin X coordinate (cm)
    [[nodiscard]] float xo(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_xo, row));
    }
    /// yo: strip origin Y coordinate (cm)
    [[nodiscard]] float yo(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_yo, row));
    }
    /// zo: strip origin Z coordinate (cm)
    [[nodiscard]] float zo(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_zo, row));
    }
    /// xe: strip end    X coordinate (cm)
    [[nodiscard]] float xe(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_xe, row));
    }
    /// ye: strip end    Y coordinate (cm)
    [[nodiscard]] float ye(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ye, row));
    }
    /// ze: strip end    Z coordinate (cm)
    [[nodiscard]] float ze(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ze, row));
    }
    /// size: size of the cluster
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_size, row));
    }
    /// status: status of the cluster
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_strip = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_xo = -1;
    int i_yo = -1;
    int i_zo = -1;
    int i_xe = -1;
    int i_ye = -1;
    int i_ze = -1;
    int i_size = -1;
    int i_status = -1;

    friend class Row;
};

/// Lightweight row view for URWT::clusters. Stores a pointer + row index only.
class URWT_clusters::Row {
 public:
    Row(const URWT_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int strip() const noexcept { return m_bank->strip(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float xo() const noexcept { return m_bank->xo(m_row); }
    [[nodiscard]] float yo() const noexcept { return m_bank->yo(m_row); }
    [[nodiscard]] float zo() const noexcept { return m_bank->zo(m_row); }
    [[nodiscard]] float xe() const noexcept { return m_bank->xe(m_row); }
    [[nodiscard]] float ye() const noexcept { return m_bank->ye(m_row); }
    [[nodiscard]] float ze() const noexcept { return m_bank->ze(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const URWT_clusters* m_bank;
    int m_row;
};

inline URWT_clusters::Row URWT_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
