// =============================================================================
// Auto-generated from event.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `REC::CaloExtras` (group 300, item 46).
/// Calorimeter Cluster Parameters
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// REC_CaloExtras bank;
/// if (auto sc = bank.bind(event.get_bank("REC::CaloExtras")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto dbstU = row.dbstU();
/// }
/// @endcode
class REC_CaloExtras {
 public:
    static constexpr std::string_view bank_name = "REC::CaloExtras";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 46;
    static constexpr std::string_view bank_schema = "dbstU/S,dbstV/S,dbstW/S,rawEU/F,rawEV/F,rawEW/F,recEU/F,recEV/F,recEW/F,recDTU/F,recDTV/F,recDTW/F,recFTU/F,recFTV/F,recFTW/F,size/F";

    class Row;

    REC_CaloExtras() = default;

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
            int i_dbstU = -1;
            int i_dbstV = -1;
            int i_dbstW = -1;
            int i_rawEU = -1;
            int i_rawEV = -1;
            int i_rawEW = -1;
            int i_recEU = -1;
            int i_recEV = -1;
            int i_recEW = -1;
            int i_recDTU = -1;
            int i_recDTV = -1;
            int i_recDTW = -1;
            int i_recFTU = -1;
            int i_recFTV = -1;
            int i_recFTW = -1;
            int i_size = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_dbstU = schema.exists("dbstU") ? schema.getEntryOrder("dbstU") : -1;
            cache.i_dbstV = schema.exists("dbstV") ? schema.getEntryOrder("dbstV") : -1;
            cache.i_dbstW = schema.exists("dbstW") ? schema.getEntryOrder("dbstW") : -1;
            cache.i_rawEU = schema.exists("rawEU") ? schema.getEntryOrder("rawEU") : -1;
            cache.i_rawEV = schema.exists("rawEV") ? schema.getEntryOrder("rawEV") : -1;
            cache.i_rawEW = schema.exists("rawEW") ? schema.getEntryOrder("rawEW") : -1;
            cache.i_recEU = schema.exists("recEU") ? schema.getEntryOrder("recEU") : -1;
            cache.i_recEV = schema.exists("recEV") ? schema.getEntryOrder("recEV") : -1;
            cache.i_recEW = schema.exists("recEW") ? schema.getEntryOrder("recEW") : -1;
            cache.i_recDTU = schema.exists("recDTU") ? schema.getEntryOrder("recDTU") : -1;
            cache.i_recDTV = schema.exists("recDTV") ? schema.getEntryOrder("recDTV") : -1;
            cache.i_recDTW = schema.exists("recDTW") ? schema.getEntryOrder("recDTW") : -1;
            cache.i_recFTU = schema.exists("recFTU") ? schema.getEntryOrder("recFTU") : -1;
            cache.i_recFTV = schema.exists("recFTV") ? schema.getEntryOrder("recFTV") : -1;
            cache.i_recFTW = schema.exists("recFTW") ? schema.getEntryOrder("recFTW") : -1;
            cache.i_size = schema.exists("size") ? schema.getEntryOrder("size") : -1;
        }
        i_dbstU = cache.i_dbstU;
        i_dbstV = cache.i_dbstV;
        i_dbstW = cache.i_dbstW;
        i_rawEU = cache.i_rawEU;
        i_rawEV = cache.i_rawEV;
        i_rawEW = cache.i_rawEW;
        i_recEU = cache.i_recEU;
        i_recEV = cache.i_recEV;
        i_recEW = cache.i_recEW;
        i_recDTU = cache.i_recDTU;
        i_recDTV = cache.i_recDTV;
        i_recDTW = cache.i_recDTW;
        i_recFTU = cache.i_recFTU;
        i_recFTV = cache.i_recFTV;
        i_recFTW = cache.i_recFTW;
        i_size = cache.i_size;
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
    /// dbstU: DB status + U strip*10
    [[nodiscard]] int dbstU(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_dbstU, row));
    }
    /// dbstV: DB status + V strip*10
    [[nodiscard]] int dbstV(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_dbstV, row));
    }
    /// dbstW: DB status + W strip*10
    [[nodiscard]] int dbstW(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_dbstW, row));
    }
    /// rawEU: raw U peak energy
    [[nodiscard]] float rawEU(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_rawEU, row));
    }
    /// rawEV: raw V peak energy
    [[nodiscard]] float rawEV(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_rawEV, row));
    }
    /// rawEW: raw W peak energy
    [[nodiscard]] float rawEW(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_rawEW, row));
    }
    /// recEU: recon U peak energy (GeV)
    [[nodiscard]] float recEU(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recEU, row));
    }
    /// recEV: recon V peak energy (GeV)
    [[nodiscard]] float recEV(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recEV, row));
    }
    /// recEW: recon W peak energy (GeV)
    [[nodiscard]] float recEW(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recEW, row));
    }
    /// recDTU: recon U peak TDC time (ns)
    [[nodiscard]] float recDTU(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recDTU, row));
    }
    /// recDTV: recon V peak TDC time (ns)
    [[nodiscard]] float recDTV(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recDTV, row));
    }
    /// recDTW: recon W peak TDC time (ns)
    [[nodiscard]] float recDTW(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recDTW, row));
    }
    /// recFTU: recon U peak FADC time (ns)
    [[nodiscard]] float recFTU(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recFTU, row));
    }
    /// recFTV: recon V peak FADC time (ns)
    [[nodiscard]] float recFTV(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recFTV, row));
    }
    /// recFTW: recon W peak FADC time (ns)
    [[nodiscard]] float recFTW(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_recFTW, row));
    }
    /// size: cluster size
    [[nodiscard]] float size(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_size, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_dbstU = -1;
    int i_dbstV = -1;
    int i_dbstW = -1;
    int i_rawEU = -1;
    int i_rawEV = -1;
    int i_rawEW = -1;
    int i_recEU = -1;
    int i_recEV = -1;
    int i_recEW = -1;
    int i_recDTU = -1;
    int i_recDTV = -1;
    int i_recDTW = -1;
    int i_recFTU = -1;
    int i_recFTV = -1;
    int i_recFTW = -1;
    int i_size = -1;

    friend class Row;
};

/// Lightweight row view for REC::CaloExtras. Stores a pointer + row index only.
class REC_CaloExtras::Row {
 public:
    Row(const REC_CaloExtras* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int dbstU() const noexcept { return m_bank->dbstU(m_row); }
    [[nodiscard]] int dbstV() const noexcept { return m_bank->dbstV(m_row); }
    [[nodiscard]] int dbstW() const noexcept { return m_bank->dbstW(m_row); }
    [[nodiscard]] float rawEU() const noexcept { return m_bank->rawEU(m_row); }
    [[nodiscard]] float rawEV() const noexcept { return m_bank->rawEV(m_row); }
    [[nodiscard]] float rawEW() const noexcept { return m_bank->rawEW(m_row); }
    [[nodiscard]] float recEU() const noexcept { return m_bank->recEU(m_row); }
    [[nodiscard]] float recEV() const noexcept { return m_bank->recEV(m_row); }
    [[nodiscard]] float recEW() const noexcept { return m_bank->recEW(m_row); }
    [[nodiscard]] float recDTU() const noexcept { return m_bank->recDTU(m_row); }
    [[nodiscard]] float recDTV() const noexcept { return m_bank->recDTV(m_row); }
    [[nodiscard]] float recDTW() const noexcept { return m_bank->recDTW(m_row); }
    [[nodiscard]] float recFTU() const noexcept { return m_bank->recFTU(m_row); }
    [[nodiscard]] float recFTV() const noexcept { return m_bank->recFTV(m_row); }
    [[nodiscard]] float recFTW() const noexcept { return m_bank->recFTW(m_row); }
    [[nodiscard]] float size() const noexcept { return m_bank->size(m_row); }
 private:
    const REC_CaloExtras* m_bank;
    int m_row;
};

inline REC_CaloExtras::Row REC_CaloExtras::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
