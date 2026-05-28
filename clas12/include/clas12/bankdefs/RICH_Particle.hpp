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


/// HIPO bank wrapper for `RICH::Particle` (group 21800, item 37).
/// Reconstructed Cherenov information per track
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RICH_Particle bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Particle")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Particle {
 public:
    static constexpr std::string_view bank_name = "RICH::Particle";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 37;

    class Row;

    RICH_Particle() = default;

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
            int i_emilay = -1;
            int i_emico = -1;
            int i_enico = -1;
            int i_emqua = -1;
            int i_mchi2 = -1;
            int i_mass = -1;
            int i_best_PID = -1;
            int i_RQ = -1;
            int i_ReQ = -1;
            int i_el_logl = -1;
            int i_pi_logl = -1;
            int i_k_logl = -1;
            int i_pr_logl = -1;
            int i_best_ch = -1;
            int i_best_c2 = -1;
            int i_best_RL = -1;
            int i_best_ntot = -1;
            int i_best_mass = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_hindex = schema.exists("hindex") ? schema.getEntryOrder("hindex") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_emilay = schema.exists("emilay") ? schema.getEntryOrder("emilay") : -1;
            cache.i_emico = schema.exists("emico") ? schema.getEntryOrder("emico") : -1;
            cache.i_enico = schema.exists("enico") ? schema.getEntryOrder("enico") : -1;
            cache.i_emqua = schema.exists("emqua") ? schema.getEntryOrder("emqua") : -1;
            cache.i_mchi2 = schema.exists("mchi2") ? schema.getEntryOrder("mchi2") : -1;
            cache.i_mass = schema.exists("mass") ? schema.getEntryOrder("mass") : -1;
            cache.i_best_PID = schema.exists("best_PID") ? schema.getEntryOrder("best_PID") : -1;
            cache.i_RQ = schema.exists("RQ") ? schema.getEntryOrder("RQ") : -1;
            cache.i_ReQ = schema.exists("ReQ") ? schema.getEntryOrder("ReQ") : -1;
            cache.i_el_logl = schema.exists("el_logl") ? schema.getEntryOrder("el_logl") : -1;
            cache.i_pi_logl = schema.exists("pi_logl") ? schema.getEntryOrder("pi_logl") : -1;
            cache.i_k_logl = schema.exists("k_logl") ? schema.getEntryOrder("k_logl") : -1;
            cache.i_pr_logl = schema.exists("pr_logl") ? schema.getEntryOrder("pr_logl") : -1;
            cache.i_best_ch = schema.exists("best_ch") ? schema.getEntryOrder("best_ch") : -1;
            cache.i_best_c2 = schema.exists("best_c2") ? schema.getEntryOrder("best_c2") : -1;
            cache.i_best_RL = schema.exists("best_RL") ? schema.getEntryOrder("best_RL") : -1;
            cache.i_best_ntot = schema.exists("best_ntot") ? schema.getEntryOrder("best_ntot") : -1;
            cache.i_best_mass = schema.exists("best_mass") ? schema.getEntryOrder("best_mass") : -1;
        }
        i_id = cache.i_id;
        i_hindex = cache.i_hindex;
        i_pindex = cache.i_pindex;
        i_emilay = cache.i_emilay;
        i_emico = cache.i_emico;
        i_enico = cache.i_enico;
        i_emqua = cache.i_emqua;
        i_mchi2 = cache.i_mchi2;
        i_mass = cache.i_mass;
        i_best_PID = cache.i_best_PID;
        i_RQ = cache.i_RQ;
        i_ReQ = cache.i_ReQ;
        i_el_logl = cache.i_el_logl;
        i_pi_logl = cache.i_pi_logl;
        i_k_logl = cache.i_k_logl;
        i_pr_logl = cache.i_pr_logl;
        i_best_ch = cache.i_best_ch;
        i_best_c2 = cache.i_best_c2;
        i_best_RL = cache.i_best_RL;
        i_best_ntot = cache.i_best_ntot;
        i_best_mass = cache.i_best_mass;
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
        return static_cast<int>(m_view->getByte(i_id, row));
    }
    /// hindex: related row in the RICH::clusters bank (if any)
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_hindex, row));
    }
    /// pindex: related row in the REC::Particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pindex, row));
    }
    /// emilay: aerogel layer of photon emission
    [[nodiscard]] int emilay(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_emilay, row));
    }
    /// emico: aerogel component of photon emission
    [[nodiscard]] int emico(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_emico, row));
    }
    /// enico: aerogel component of photon entrance point
    [[nodiscard]] int enico(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_enico, row));
    }
    /// emqua: aerogel quadrant of photon emission
    [[nodiscard]] int emqua(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_emqua, row));
    }
    /// mchi2: track-cluster matching chi2 (if any)
    [[nodiscard]] float mchi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_mchi2, row));
    }
    /// mass: reconstructed mass for best hypo
    [[nodiscard]] float mass(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_mass, row));
    }
    /// best_PID: most probable PID choice
    [[nodiscard]] int best_PID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_best_PID, row));
    }
    /// RQ: goodness of hadron choice parameter (1=anambiguos, 0=random)
    [[nodiscard]] float RQ(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_RQ, row));
    }
    /// ReQ: goodness of elecgtron choice parameter (1=anambiguos, 0=random)
    [[nodiscard]] float ReQ(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ReQ, row));
    }
    /// el_logl: probability to be an electron
    [[nodiscard]] float el_logl(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_el_logl, row));
    }
    /// pi_logl: probability to be an pion
    [[nodiscard]] float pi_logl(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pi_logl, row));
    }
    /// k_logl: probability to be an kaon
    [[nodiscard]] float k_logl(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_k_logl, row));
    }
    /// pr_logl: probability to be an proton
    [[nodiscard]] float pr_logl(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pr_logl, row));
    }
    /// best_ch: Average etaC for best hypothesis
    [[nodiscard]] float best_ch(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_best_ch, row));
    }
    /// best_c2: chi2 for best hypothesis
    [[nodiscard]] float best_c2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_best_c2, row));
    }
    /// best_RL: Likelihood ratio for best hypothesis
    [[nodiscard]] float best_RL(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_best_RL, row));
    }
    /// best_ntot: Number of photon used for likelihood
    [[nodiscard]] float best_ntot(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_best_ntot, row));
    }
    /// best_mass: Reconstructed mass for best hypothesis
    [[nodiscard]] float best_mass(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_best_mass, row));
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
    int i_emilay = -1;
    int i_emico = -1;
    int i_enico = -1;
    int i_emqua = -1;
    int i_mchi2 = -1;
    int i_mass = -1;
    int i_best_PID = -1;
    int i_RQ = -1;
    int i_ReQ = -1;
    int i_el_logl = -1;
    int i_pi_logl = -1;
    int i_k_logl = -1;
    int i_pr_logl = -1;
    int i_best_ch = -1;
    int i_best_c2 = -1;
    int i_best_RL = -1;
    int i_best_ntot = -1;
    int i_best_mass = -1;

    friend class Row;
};

/// Lightweight row view for RICH::Particle. Stores a pointer + row index only.
class RICH_Particle::Row {
 public:
    Row(const RICH_Particle* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int emilay() const noexcept { return m_bank->emilay(m_row); }
    [[nodiscard]] int emico() const noexcept { return m_bank->emico(m_row); }
    [[nodiscard]] int enico() const noexcept { return m_bank->enico(m_row); }
    [[nodiscard]] int emqua() const noexcept { return m_bank->emqua(m_row); }
    [[nodiscard]] float mchi2() const noexcept { return m_bank->mchi2(m_row); }
    [[nodiscard]] float mass() const noexcept { return m_bank->mass(m_row); }
    [[nodiscard]] int best_PID() const noexcept { return m_bank->best_PID(m_row); }
    [[nodiscard]] float RQ() const noexcept { return m_bank->RQ(m_row); }
    [[nodiscard]] float ReQ() const noexcept { return m_bank->ReQ(m_row); }
    [[nodiscard]] float el_logl() const noexcept { return m_bank->el_logl(m_row); }
    [[nodiscard]] float pi_logl() const noexcept { return m_bank->pi_logl(m_row); }
    [[nodiscard]] float k_logl() const noexcept { return m_bank->k_logl(m_row); }
    [[nodiscard]] float pr_logl() const noexcept { return m_bank->pr_logl(m_row); }
    [[nodiscard]] float best_ch() const noexcept { return m_bank->best_ch(m_row); }
    [[nodiscard]] float best_c2() const noexcept { return m_bank->best_c2(m_row); }
    [[nodiscard]] float best_RL() const noexcept { return m_bank->best_RL(m_row); }
    [[nodiscard]] float best_ntot() const noexcept { return m_bank->best_ntot(m_row); }
    [[nodiscard]] float best_mass() const noexcept { return m_bank->best_mass(m_row); }
 private:
    const RICH_Particle* m_bank;
    int m_row;
};

inline RICH_Particle::Row RICH_Particle::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
