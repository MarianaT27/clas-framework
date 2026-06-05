// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `MC::GenMatch` (group 40, item 6).
/// MC::Particle -> REC::Particle matching
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// MC_GenMatch bank;
/// if (auto sc = bank.bind(event.get_bank("MC::GenMatch")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto mcindex = row.mcindex();
/// }
/// @endcode
class MC_GenMatch {
 public:
    static constexpr std::string_view bank_name = "MC::GenMatch";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 6;
    static constexpr std::string_view bank_schema = "mcindex/S,pindex/S,mclayer1/L,mclayer2/L,player1/L,player2/L,quality/F";

    class Row;

    MC_GenMatch() = default;

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
            int i_mcindex = -1;
            int i_pindex = -1;
            int i_mclayer1 = -1;
            int i_mclayer2 = -1;
            int i_player1 = -1;
            int i_player2 = -1;
            int i_quality = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_mcindex = schema.exists("mcindex") ? schema.getEntryOrder("mcindex") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_mclayer1 = schema.exists("mclayer1") ? schema.getEntryOrder("mclayer1") : -1;
            cache.i_mclayer2 = schema.exists("mclayer2") ? schema.getEntryOrder("mclayer2") : -1;
            cache.i_player1 = schema.exists("player1") ? schema.getEntryOrder("player1") : -1;
            cache.i_player2 = schema.exists("player2") ? schema.getEntryOrder("player2") : -1;
            cache.i_quality = schema.exists("quality") ? schema.getEntryOrder("quality") : -1;
        }
        i_mcindex = cache.i_mcindex;
        i_pindex = cache.i_pindex;
        i_mclayer1 = cache.i_mclayer1;
        i_mclayer2 = cache.i_mclayer2;
        i_player1 = cache.i_player1;
        i_player2 = cache.i_player2;
        i_quality = cache.i_quality;
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
    /// mcindex: MC::Particle index
    [[nodiscard]] int mcindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_mcindex, row));
    }
    /// pindex: REC::Particle index
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pindex, row));
    }
    /// mclayer1: layers from the 1st set of detectors hit by MC particle
    [[nodiscard]] int64_t mclayer1(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_mclayer1, row));
    }
    /// mclayer2: layers from the 2nd set of detectors hit by MC particle
    [[nodiscard]] int64_t mclayer2(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_mclayer2, row));
    }
    /// player1: layers from the 1st set of detectors hit by Recon particle
    [[nodiscard]] int64_t player1(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_player1, row));
    }
    /// player2: layers from the 2nd set of detectors hit by Recon particle
    [[nodiscard]] int64_t player2(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_player2, row));
    }
    /// quality: matching quality parameter
    [[nodiscard]] float quality(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_quality, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int mcindex_handle() const noexcept { return i_mcindex; }
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_mcindex = -1;
    int i_pindex = -1;
    int i_mclayer1 = -1;
    int i_mclayer2 = -1;
    int i_player1 = -1;
    int i_player2 = -1;
    int i_quality = -1;

    friend class Row;
};

/// Lightweight row view for MC::GenMatch. Stores a pointer + row index only.
class MC_GenMatch::Row {
 public:
    Row(const MC_GenMatch* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int mcindex() const noexcept { return m_bank->mcindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int64_t mclayer1() const noexcept { return m_bank->mclayer1(m_row); }
    [[nodiscard]] int64_t mclayer2() const noexcept { return m_bank->mclayer2(m_row); }
    [[nodiscard]] int64_t player1() const noexcept { return m_bank->player1(m_row); }
    [[nodiscard]] int64_t player2() const noexcept { return m_bank->player2(m_row); }
    [[nodiscard]] float quality() const noexcept { return m_bank->quality(m_row); }
 private:
    const MC_GenMatch* m_bank;
    int m_row;
};

inline MC_GenMatch::Row MC_GenMatch::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
