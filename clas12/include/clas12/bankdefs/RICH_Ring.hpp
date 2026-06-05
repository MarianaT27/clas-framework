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


/// HIPO bank wrapper for `RICH::Ring` (group 21800, item 36).
/// Reconstructed Cherenov information for single photon
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RICH_Ring bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Ring")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Ring {
 public:
    static constexpr std::string_view bank_name = "RICH::Ring";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 36;
    static constexpr std::string_view bank_schema = "id/S,hindex/S,pindex/B,sector/B,pmt/S,anode/B,hypo/I,dtime/F,etaC/F,prob/F,use/B,dangle/F,layers/I,compos/I";

    class Row;

    RICH_Ring() = default;

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
            int i_pindex = -1;
            int i_sector = -1;
            int i_pmt = -1;
            int i_anode = -1;
            int i_hypo = -1;
            int i_dtime = -1;
            int i_etaC = -1;
            int i_prob = -1;
            int i_use = -1;
            int i_dangle = -1;
            int i_layers = -1;
            int i_compos = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_hindex = schema.exists("hindex") ? schema.getEntryOrder("hindex") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_pmt = schema.exists("pmt") ? schema.getEntryOrder("pmt") : -1;
            cache.i_anode = schema.exists("anode") ? schema.getEntryOrder("anode") : -1;
            cache.i_hypo = schema.exists("hypo") ? schema.getEntryOrder("hypo") : -1;
            cache.i_dtime = schema.exists("dtime") ? schema.getEntryOrder("dtime") : -1;
            cache.i_etaC = schema.exists("etaC") ? schema.getEntryOrder("etaC") : -1;
            cache.i_prob = schema.exists("prob") ? schema.getEntryOrder("prob") : -1;
            cache.i_use = schema.exists("use") ? schema.getEntryOrder("use") : -1;
            cache.i_dangle = schema.exists("dangle") ? schema.getEntryOrder("dangle") : -1;
            cache.i_layers = schema.exists("layers") ? schema.getEntryOrder("layers") : -1;
            cache.i_compos = schema.exists("compos") ? schema.getEntryOrder("compos") : -1;
        }
        i_id = cache.i_id;
        i_hindex = cache.i_hindex;
        i_pindex = cache.i_pindex;
        i_sector = cache.i_sector;
        i_pmt = cache.i_pmt;
        i_anode = cache.i_anode;
        i_hypo = cache.i_hypo;
        i_dtime = cache.i_dtime;
        i_etaC = cache.i_etaC;
        i_prob = cache.i_prob;
        i_use = cache.i_use;
        i_dangle = cache.i_dangle;
        i_layers = cache.i_layers;
        i_compos = cache.i_compos;
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
    /// id: id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// hindex: related row in the RICH::hits bank
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_hindex, row));
    }
    /// pindex: related row in the REC::Particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pindex, row));
    }
    /// sector: photon sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// pmt: photon hit pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pmt, row));
    }
    /// anode: photon hit anode
    [[nodiscard]] int anode(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_anode, row));
    }
    /// hypo: PID hypothesis for photon reconstruction
    [[nodiscard]] int hypo(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_hypo, row));
    }
    /// dtime: time mismatch for traced solution
    [[nodiscard]] float dtime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dtime, row));
    }
    /// etaC: Cherenkov angle as traced solution
    [[nodiscard]] float etaC(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_etaC, row));
    }
    /// prob: probability for the given hypothesis
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob, row));
    }
    /// use: flag for use in the likelihood
    [[nodiscard]] int use(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_use, row));
    }
    /// dangle: angular range subtended by the pixel
    [[nodiscard]] float dangle(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dangle, row));
    }
    /// layers: sequence of hit layers (reflections)
    [[nodiscard]] int layers(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_layers, row));
    }
    /// compos: sequence of hit components (reflections)
    [[nodiscard]] int compos(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_compos, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int hindex_handle() const noexcept { return i_hindex; }
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_hindex = -1;
    int i_pindex = -1;
    int i_sector = -1;
    int i_pmt = -1;
    int i_anode = -1;
    int i_hypo = -1;
    int i_dtime = -1;
    int i_etaC = -1;
    int i_prob = -1;
    int i_use = -1;
    int i_dangle = -1;
    int i_layers = -1;
    int i_compos = -1;

    friend class Row;
};

/// Lightweight row view for RICH::Ring. Stores a pointer + row index only.
class RICH_Ring::Row {
 public:
    Row(const RICH_Ring* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] int anode() const noexcept { return m_bank->anode(m_row); }
    [[nodiscard]] int hypo() const noexcept { return m_bank->hypo(m_row); }
    [[nodiscard]] float dtime() const noexcept { return m_bank->dtime(m_row); }
    [[nodiscard]] float etaC() const noexcept { return m_bank->etaC(m_row); }
    [[nodiscard]] float prob() const noexcept { return m_bank->prob(m_row); }
    [[nodiscard]] int use() const noexcept { return m_bank->use(m_row); }
    [[nodiscard]] float dangle() const noexcept { return m_bank->dangle(m_row); }
    [[nodiscard]] int layers() const noexcept { return m_bank->layers(m_row); }
    [[nodiscard]] int compos() const noexcept { return m_bank->compos(m_row); }
 private:
    const RICH_Ring* m_bank;
    int m_row;
};

inline RICH_Ring::Row RICH_Ring::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
