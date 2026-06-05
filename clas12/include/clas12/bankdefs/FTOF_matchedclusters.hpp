// =============================================================================
// Auto-generated from tof.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `FTOF::matchedclusters` (group 21200, item 33).
/// matched clusters from FTOF
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// FTOF_matchedclusters bank;
/// if (auto sc = bank.bind(event.get_bank("FTOF::matchedclusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class FTOF_matchedclusters {
 public:
    static constexpr std::string_view bank_name = "FTOF::matchedclusters";
    static constexpr int bank_group = 21200;
    static constexpr int bank_item  = 33;
    static constexpr std::string_view bank_schema = "sector/B,paddle_id1A/S,paddle_id1B/S,clus_1Aid/S,clus_1Bid/S,clusSize_1A/S,clusSize_1B/S,tminAlgo_1B_tCorr/F,midbarAlgo_1B_tCorr/F,EmaxAlgo_1B_tCorr/F";

    class Row;

    FTOF_matchedclusters() = default;

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
            int i_sector = -1;
            int i_paddle_id1A = -1;
            int i_paddle_id1B = -1;
            int i_clus_1Aid = -1;
            int i_clus_1Bid = -1;
            int i_clusSize_1A = -1;
            int i_clusSize_1B = -1;
            int i_tminAlgo_1B_tCorr = -1;
            int i_midbarAlgo_1B_tCorr = -1;
            int i_EmaxAlgo_1B_tCorr = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_paddle_id1A = schema.exists("paddle_id1A") ? schema.getEntryOrder("paddle_id1A") : -1;
            cache.i_paddle_id1B = schema.exists("paddle_id1B") ? schema.getEntryOrder("paddle_id1B") : -1;
            cache.i_clus_1Aid = schema.exists("clus_1Aid") ? schema.getEntryOrder("clus_1Aid") : -1;
            cache.i_clus_1Bid = schema.exists("clus_1Bid") ? schema.getEntryOrder("clus_1Bid") : -1;
            cache.i_clusSize_1A = schema.exists("clusSize_1A") ? schema.getEntryOrder("clusSize_1A") : -1;
            cache.i_clusSize_1B = schema.exists("clusSize_1B") ? schema.getEntryOrder("clusSize_1B") : -1;
            cache.i_tminAlgo_1B_tCorr = schema.exists("tminAlgo_1B_tCorr") ? schema.getEntryOrder("tminAlgo_1B_tCorr") : -1;
            cache.i_midbarAlgo_1B_tCorr = schema.exists("midbarAlgo_1B_tCorr") ? schema.getEntryOrder("midbarAlgo_1B_tCorr") : -1;
            cache.i_EmaxAlgo_1B_tCorr = schema.exists("EmaxAlgo_1B_tCorr") ? schema.getEntryOrder("EmaxAlgo_1B_tCorr") : -1;
        }
        i_sector = cache.i_sector;
        i_paddle_id1A = cache.i_paddle_id1A;
        i_paddle_id1B = cache.i_paddle_id1B;
        i_clus_1Aid = cache.i_clus_1Aid;
        i_clus_1Bid = cache.i_clus_1Bid;
        i_clusSize_1A = cache.i_clusSize_1A;
        i_clusSize_1B = cache.i_clusSize_1B;
        i_tminAlgo_1B_tCorr = cache.i_tminAlgo_1B_tCorr;
        i_midbarAlgo_1B_tCorr = cache.i_midbarAlgo_1B_tCorr;
        i_EmaxAlgo_1B_tCorr = cache.i_EmaxAlgo_1B_tCorr;
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
    /// sector: sector of FTOF
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// paddle_id1A: paddle id of hit with lowest paddle id in cluster for panel 1A
    [[nodiscard]] int paddle_id1A(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_paddle_id1A, row));
    }
    /// paddle_id1B: paddle id of hit with lowest paddle id in cluster for panel 1B
    [[nodiscard]] int paddle_id1B(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_paddle_id1B, row));
    }
    /// clus_1Aid: id of cluster in 1A
    [[nodiscard]] int clus_1Aid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clus_1Aid, row));
    }
    /// clus_1Bid: id of cluster in 1B
    [[nodiscard]] int clus_1Bid(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clus_1Bid, row));
    }
    /// clusSize_1A: size of cluster in 1A
    [[nodiscard]] int clusSize_1A(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusSize_1A, row));
    }
    /// clusSize_1B: size of cluster in 1B
    [[nodiscard]] int clusSize_1B(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_clusSize_1B, row));
    }
    /// tminAlgo_1B_tCorr: corrected hit time using tmin algorithm to compute the path length between 1A and 1B
    [[nodiscard]] float tminAlgo_1B_tCorr(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_tminAlgo_1B_tCorr, row));
    }
    /// midbarAlgo_1B_tCorr: corrected hit time using middle of bar algorithm to compute the path length between 1A and 1B
    [[nodiscard]] float midbarAlgo_1B_tCorr(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_midbarAlgo_1B_tCorr, row));
    }
    /// EmaxAlgo_1B_tCorr: corrected hit time using Emax algorithm to compute the path length between 1A and 1B
    [[nodiscard]] float EmaxAlgo_1B_tCorr(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_EmaxAlgo_1B_tCorr, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_sector = -1;
    int i_paddle_id1A = -1;
    int i_paddle_id1B = -1;
    int i_clus_1Aid = -1;
    int i_clus_1Bid = -1;
    int i_clusSize_1A = -1;
    int i_clusSize_1B = -1;
    int i_tminAlgo_1B_tCorr = -1;
    int i_midbarAlgo_1B_tCorr = -1;
    int i_EmaxAlgo_1B_tCorr = -1;

    friend class Row;
};

/// Lightweight row view for FTOF::matchedclusters. Stores a pointer + row index only.
class FTOF_matchedclusters::Row {
 public:
    Row(const FTOF_matchedclusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int paddle_id1A() const noexcept { return m_bank->paddle_id1A(m_row); }
    [[nodiscard]] int paddle_id1B() const noexcept { return m_bank->paddle_id1B(m_row); }
    [[nodiscard]] int clus_1Aid() const noexcept { return m_bank->clus_1Aid(m_row); }
    [[nodiscard]] int clus_1Bid() const noexcept { return m_bank->clus_1Bid(m_row); }
    [[nodiscard]] int clusSize_1A() const noexcept { return m_bank->clusSize_1A(m_row); }
    [[nodiscard]] int clusSize_1B() const noexcept { return m_bank->clusSize_1B(m_row); }
    [[nodiscard]] float tminAlgo_1B_tCorr() const noexcept { return m_bank->tminAlgo_1B_tCorr(m_row); }
    [[nodiscard]] float midbarAlgo_1B_tCorr() const noexcept { return m_bank->midbarAlgo_1B_tCorr(m_row); }
    [[nodiscard]] float EmaxAlgo_1B_tCorr() const noexcept { return m_bank->EmaxAlgo_1B_tCorr(m_row); }
 private:
    const FTOF_matchedclusters* m_bank;
    int m_row;
};

inline FTOF_matchedclusters::Row FTOF_matchedclusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
