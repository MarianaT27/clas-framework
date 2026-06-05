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


/// HIPO bank wrapper for `HitBasedTrkg::AIHits` (group 20600, item 61).
/// reconstructed hits using DC wire positions
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// HitBasedTrkg_AIHits bank;
/// if (auto sc = bank.bind(event.get_bank("HitBasedTrkg::AIHits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HitBasedTrkg_AIHits {
 public:
    static constexpr std::string_view bank_name = "HitBasedTrkg::AIHits";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 61;
    static constexpr std::string_view bank_schema = "id/S,indexTDC/S,status/S,sector/B,superlayer/B,layer/B,wire/S,TDC/I,jitter/B,trkDoca/F,docaError/F,LR/B,LocX/F,LocY/F,X/F,Z/F,clusterID/S";

    class Row;

    HitBasedTrkg_AIHits() = default;

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
            int i_indexTDC = -1;
            int i_status = -1;
            int i_sector = -1;
            int i_superlayer = -1;
            int i_layer = -1;
            int i_wire = -1;
            int i_TDC = -1;
            int i_jitter = -1;
            int i_trkDoca = -1;
            int i_docaError = -1;
            int i_LR = -1;
            int i_LocX = -1;
            int i_LocY = -1;
            int i_X = -1;
            int i_Z = -1;
            int i_clusterID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_indexTDC = schema.exists("indexTDC") ? schema.getEntryOrder("indexTDC") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_superlayer = schema.exists("superlayer") ? schema.getEntryOrder("superlayer") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_wire = schema.exists("wire") ? schema.getEntryOrder("wire") : -1;
            cache.i_TDC = schema.exists("TDC") ? schema.getEntryOrder("TDC") : -1;
            cache.i_jitter = schema.exists("jitter") ? schema.getEntryOrder("jitter") : -1;
            cache.i_trkDoca = schema.exists("trkDoca") ? schema.getEntryOrder("trkDoca") : -1;
            cache.i_docaError = schema.exists("docaError") ? schema.getEntryOrder("docaError") : -1;
            cache.i_LR = schema.exists("LR") ? schema.getEntryOrder("LR") : -1;
            cache.i_LocX = schema.exists("LocX") ? schema.getEntryOrder("LocX") : -1;
            cache.i_LocY = schema.exists("LocY") ? schema.getEntryOrder("LocY") : -1;
            cache.i_X = schema.exists("X") ? schema.getEntryOrder("X") : -1;
            cache.i_Z = schema.exists("Z") ? schema.getEntryOrder("Z") : -1;
            cache.i_clusterID = schema.exists("clusterID") ? schema.getEntryOrder("clusterID") : -1;
        }
        i_id = cache.i_id;
        i_indexTDC = cache.i_indexTDC;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_superlayer = cache.i_superlayer;
        i_layer = cache.i_layer;
        i_wire = cache.i_wire;
        i_TDC = cache.i_TDC;
        i_jitter = cache.i_jitter;
        i_trkDoca = cache.i_trkDoca;
        i_docaError = cache.i_docaError;
        i_LR = cache.i_LR;
        i_LocX = cache.i_LocX;
        i_LocY = cache.i_LocY;
        i_X = cache.i_X;
        i_Z = cache.i_Z;
        i_clusterID = cache.i_clusterID;
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
    /// indexTDC: index in the bank DC::TDC
    [[nodiscard]] int indexTDC(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_indexTDC, row));
    }
    /// status: id of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// sector: DC sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// superlayer: DC superlayer (1...6)
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_superlayer, row));
    }
    /// layer: DC layer in superlayer (1...6)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// wire: wire id of DC
    [[nodiscard]] int wire(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_wire, row));
    }
    /// TDC: raw time of the hit
    [[nodiscard]] int TDC(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_TDC, row));
    }
    /// jitter: time jitter (ns)
    [[nodiscard]] int jitter(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_jitter, row));
    }
    /// trkDoca: track doca of the hit (in cm)
    [[nodiscard]] float trkDoca(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trkDoca, row));
    }
    /// docaError: error on track doca of the hit
    [[nodiscard]] float docaError(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_docaError, row));
    }
    /// LR: Left/Right ambiguity of the hit
    [[nodiscard]] int LR(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_LR, row));
    }
    /// LocX: x in planar local coordinate system
    [[nodiscard]] float LocX(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_LocX, row));
    }
    /// LocY: y in planar local coordinate system
    [[nodiscard]] float LocY(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_LocY, row));
    }
    /// X: wire x-coordinate  in tilted-sector
    [[nodiscard]] float X(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_X, row));
    }
    /// Z: wire z-coordinate  in tilted-sector
    [[nodiscard]] float Z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_Z, row));
    }
    /// clusterID: ID of associated cluster
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusterID, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_indexTDC = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_superlayer = -1;
    int i_layer = -1;
    int i_wire = -1;
    int i_TDC = -1;
    int i_jitter = -1;
    int i_trkDoca = -1;
    int i_docaError = -1;
    int i_LR = -1;
    int i_LocX = -1;
    int i_LocY = -1;
    int i_X = -1;
    int i_Z = -1;
    int i_clusterID = -1;

    friend class Row;
};

/// Lightweight row view for HitBasedTrkg::AIHits. Stores a pointer + row index only.
class HitBasedTrkg_AIHits::Row {
 public:
    Row(const HitBasedTrkg_AIHits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int indexTDC() const noexcept { return m_bank->indexTDC(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int wire() const noexcept { return m_bank->wire(m_row); }
    [[nodiscard]] int TDC() const noexcept { return m_bank->TDC(m_row); }
    [[nodiscard]] int jitter() const noexcept { return m_bank->jitter(m_row); }
    [[nodiscard]] float trkDoca() const noexcept { return m_bank->trkDoca(m_row); }
    [[nodiscard]] float docaError() const noexcept { return m_bank->docaError(m_row); }
    [[nodiscard]] int LR() const noexcept { return m_bank->LR(m_row); }
    [[nodiscard]] float LocX() const noexcept { return m_bank->LocX(m_row); }
    [[nodiscard]] float LocY() const noexcept { return m_bank->LocY(m_row); }
    [[nodiscard]] float X() const noexcept { return m_bank->X(m_row); }
    [[nodiscard]] float Z() const noexcept { return m_bank->Z(m_row); }
    [[nodiscard]] int clusterID() const noexcept { return m_bank->clusterID(m_row); }
 private:
    const HitBasedTrkg_AIHits* m_bank;
    int m_row;
};

inline HitBasedTrkg_AIHits::Row HitBasedTrkg_AIHits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
