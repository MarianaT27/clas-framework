// =============================================================================
// Auto-generated from ecal.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `ECAL::clusters` (group 20700, item 23).
/// reconstructed clusters in ECAL
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ECAL_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("ECAL::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ECAL_clusters {
 public:
    static constexpr std::string_view bank_name = "ECAL::clusters";
    static constexpr int bank_group = 20700;
    static constexpr int bank_item  = 23;

    class Row;

    ECAL_clusters() = default;

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
            int i_status = -1;
            int i_sector = -1;
            int i_layer = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_energy = -1;
            int i_time = -1;
            int i_widthU = -1;
            int i_widthV = -1;
            int i_widthW = -1;
            int i_idU = -1;
            int i_idV = -1;
            int i_idW = -1;
            int i_coordU = -1;
            int i_coordV = -1;
            int i_coordW = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_widthU = schema.exists("widthU") ? schema.getEntryOrder("widthU") : -1;
            cache.i_widthV = schema.exists("widthV") ? schema.getEntryOrder("widthV") : -1;
            cache.i_widthW = schema.exists("widthW") ? schema.getEntryOrder("widthW") : -1;
            cache.i_idU = schema.exists("idU") ? schema.getEntryOrder("idU") : -1;
            cache.i_idV = schema.exists("idV") ? schema.getEntryOrder("idV") : -1;
            cache.i_idW = schema.exists("idW") ? schema.getEntryOrder("idW") : -1;
            cache.i_coordU = schema.exists("coordU") ? schema.getEntryOrder("coordU") : -1;
            cache.i_coordV = schema.exists("coordV") ? schema.getEntryOrder("coordV") : -1;
            cache.i_coordW = schema.exists("coordW") ? schema.getEntryOrder("coordW") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_energy = cache.i_energy;
        i_time = cache.i_time;
        i_widthU = cache.i_widthU;
        i_widthV = cache.i_widthV;
        i_widthW = cache.i_widthW;
        i_idU = cache.i_idU;
        i_idV = cache.i_idV;
        i_idW = cache.i_idW;
        i_coordU = cache.i_coordU;
        i_coordV = cache.i_coordV;
        i_coordW = cache.i_coordW;
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
    /// status: status of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// sector: sector of ECAL
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: Layer of ECAL (1-3:PCAL, 4-6:ECIN, 7-9:ECOUT
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// x: X coordinate of the hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Y coordinate of the hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Z coordinate of the hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// energy: Energy of the hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// time: Energy of the hit
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// widthU: width of U peak
    [[nodiscard]] float widthU(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_widthU, row));
    }
    /// widthV: width of V peak
    [[nodiscard]] float widthV(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_widthV, row));
    }
    /// widthW: width of W peak
    [[nodiscard]] float widthW(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_widthW, row));
    }
    /// idU: id of U peak
    [[nodiscard]] int idU(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_idU, row));
    }
    /// idV: id of V peak
    [[nodiscard]] int idV(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_idV, row));
    }
    /// idW: id of W peak
    [[nodiscard]] int idW(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_idW, row));
    }
    /// coordU: U coordinate 
    [[nodiscard]] int coordU(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_coordU, row));
    }
    /// coordV: V coordinate
    [[nodiscard]] int coordV(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_coordV, row));
    }
    /// coordW: W coordinate
    [[nodiscard]] int coordW(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_coordW, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_layer = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_energy = -1;
    int i_time = -1;
    int i_widthU = -1;
    int i_widthV = -1;
    int i_widthW = -1;
    int i_idU = -1;
    int i_idV = -1;
    int i_idW = -1;
    int i_coordU = -1;
    int i_coordV = -1;
    int i_coordW = -1;

    friend class Row;
};

/// Lightweight row view for ECAL::clusters. Stores a pointer + row index only.
class ECAL_clusters::Row {
 public:
    Row(const ECAL_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float widthU() const noexcept { return m_bank->widthU(m_row); }
    [[nodiscard]] float widthV() const noexcept { return m_bank->widthV(m_row); }
    [[nodiscard]] float widthW() const noexcept { return m_bank->widthW(m_row); }
    [[nodiscard]] int idU() const noexcept { return m_bank->idU(m_row); }
    [[nodiscard]] int idV() const noexcept { return m_bank->idV(m_row); }
    [[nodiscard]] int idW() const noexcept { return m_bank->idW(m_row); }
    [[nodiscard]] int coordU() const noexcept { return m_bank->coordU(m_row); }
    [[nodiscard]] int coordV() const noexcept { return m_bank->coordV(m_row); }
    [[nodiscard]] int coordW() const noexcept { return m_bank->coordW(m_row); }
 private:
    const ECAL_clusters* m_bank;
    int m_row;
};

inline ECAL_clusters::Row ECAL_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
