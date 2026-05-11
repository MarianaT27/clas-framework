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


/// HIPO bank wrapper for `RICH::Photon` (group 21800, item 39).
/// Reconstructed Photons in RICH
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RICH_Photon bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Photon")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Photon {
 public:
    static constexpr std::string_view bank_name = "RICH::Photon";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 39;

    class Row;

    RICH_Photon() = default;

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
            int i_hindex = -1;
            int i_pindex = -1;
            int i_type = -1;
            int i_used = -1;
            int i_start_time = -1;
            int i_traced_the = -1;
            int i_traced_phi = -1;
            int i_traced_hitx = -1;
            int i_traced_hity = -1;
            int i_traced_hitz = -1;
            int i_traced_path = -1;
            int i_traced_time = -1;
            int i_traced_stime = -1;
            int i_traced_nrfl = -1;
            int i_traced_nrfr = -1;
            int i_traced_1rfl = -1;
            int i_traced_layers = -1;
            int i_traced_compos = -1;
            int i_traced_etaC = -1;
            int i_traced_aeron = -1;
            int i_traced_dthe = -1;
            int i_traced_dphi = -1;
            int i_eff = -1;
            int i_back = -1;
            int i_hypo_pid = -1;
            int i_etac_ref = -1;
            int i_etac_rms = -1;
            int i_prob = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_hindex = schema.exists("hindex") ? schema.getEntryOrder("hindex") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_type = schema.exists("type") ? schema.getEntryOrder("type") : -1;
            cache.i_used = schema.exists("used") ? schema.getEntryOrder("used") : -1;
            cache.i_start_time = schema.exists("start_time") ? schema.getEntryOrder("start_time") : -1;
            cache.i_traced_the = schema.exists("traced_the") ? schema.getEntryOrder("traced_the") : -1;
            cache.i_traced_phi = schema.exists("traced_phi") ? schema.getEntryOrder("traced_phi") : -1;
            cache.i_traced_hitx = schema.exists("traced_hitx") ? schema.getEntryOrder("traced_hitx") : -1;
            cache.i_traced_hity = schema.exists("traced_hity") ? schema.getEntryOrder("traced_hity") : -1;
            cache.i_traced_hitz = schema.exists("traced_hitz") ? schema.getEntryOrder("traced_hitz") : -1;
            cache.i_traced_path = schema.exists("traced_path") ? schema.getEntryOrder("traced_path") : -1;
            cache.i_traced_time = schema.exists("traced_time") ? schema.getEntryOrder("traced_time") : -1;
            cache.i_traced_stime = schema.exists("traced_stime") ? schema.getEntryOrder("traced_stime") : -1;
            cache.i_traced_nrfl = schema.exists("traced_nrfl") ? schema.getEntryOrder("traced_nrfl") : -1;
            cache.i_traced_nrfr = schema.exists("traced_nrfr") ? schema.getEntryOrder("traced_nrfr") : -1;
            cache.i_traced_1rfl = schema.exists("traced_1rfl") ? schema.getEntryOrder("traced_1rfl") : -1;
            cache.i_traced_layers = schema.exists("traced_layers") ? schema.getEntryOrder("traced_layers") : -1;
            cache.i_traced_compos = schema.exists("traced_compos") ? schema.getEntryOrder("traced_compos") : -1;
            cache.i_traced_etaC = schema.exists("traced_etaC") ? schema.getEntryOrder("traced_etaC") : -1;
            cache.i_traced_aeron = schema.exists("traced_aeron") ? schema.getEntryOrder("traced_aeron") : -1;
            cache.i_traced_dthe = schema.exists("traced_dthe") ? schema.getEntryOrder("traced_dthe") : -1;
            cache.i_traced_dphi = schema.exists("traced_dphi") ? schema.getEntryOrder("traced_dphi") : -1;
            cache.i_eff = schema.exists("eff") ? schema.getEntryOrder("eff") : -1;
            cache.i_back = schema.exists("back") ? schema.getEntryOrder("back") : -1;
            cache.i_hypo_pid = schema.exists("hypo_pid") ? schema.getEntryOrder("hypo_pid") : -1;
            cache.i_etac_ref = schema.exists("etac_ref") ? schema.getEntryOrder("etac_ref") : -1;
            cache.i_etac_rms = schema.exists("etac_rms") ? schema.getEntryOrder("etac_rms") : -1;
            cache.i_prob = schema.exists("prob") ? schema.getEntryOrder("prob") : -1;
        }
        i_id = cache.i_id;
        i_hindex = cache.i_hindex;
        i_pindex = cache.i_pindex;
        i_type = cache.i_type;
        i_used = cache.i_used;
        i_start_time = cache.i_start_time;
        i_traced_the = cache.i_traced_the;
        i_traced_phi = cache.i_traced_phi;
        i_traced_hitx = cache.i_traced_hitx;
        i_traced_hity = cache.i_traced_hity;
        i_traced_hitz = cache.i_traced_hitz;
        i_traced_path = cache.i_traced_path;
        i_traced_time = cache.i_traced_time;
        i_traced_stime = cache.i_traced_stime;
        i_traced_nrfl = cache.i_traced_nrfl;
        i_traced_nrfr = cache.i_traced_nrfr;
        i_traced_1rfl = cache.i_traced_1rfl;
        i_traced_layers = cache.i_traced_layers;
        i_traced_compos = cache.i_traced_compos;
        i_traced_etaC = cache.i_traced_etaC;
        i_traced_aeron = cache.i_traced_aeron;
        i_traced_dthe = cache.i_traced_dthe;
        i_traced_dphi = cache.i_traced_dphi;
        i_eff = cache.i_eff;
        i_back = cache.i_back;
        i_hypo_pid = cache.i_hypo_pid;
        i_etac_ref = cache.i_etac_ref;
        i_etac_rms = cache.i_etac_rms;
        i_prob = cache.i_prob;
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
    /// type: hypo (0=ele, 1=pion, 2=kaon, 3=proto) + reco (0=real hit, 10=trial)
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_type, row));
    }
    /// used: eligible for PID in time (1=yes, 0=no) angle (10=yes, 0=no) hypo (100=by other)
    [[nodiscard]] int used(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_used, row));
    }
    /// start_time: time at photon emission point
    [[nodiscard]] float start_time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_start_time, row));
    }
    /// traced_the: Lab theta angle for traced solution
    [[nodiscard]] float traced_the(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_the, row));
    }
    /// traced_phi: Lab phi angle for traced solution
    [[nodiscard]] float traced_phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_phi, row));
    }
    /// traced_hitx: Lab hitx angle for traced solution
    [[nodiscard]] float traced_hitx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_hitx, row));
    }
    /// traced_hity: Lab hity angle for traced solution
    [[nodiscard]] float traced_hity(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_hity, row));
    }
    /// traced_hitz: Lab hitz angle for traced solution
    [[nodiscard]] float traced_hitz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_hitz, row));
    }
    /// traced_path: path for traced solution
    [[nodiscard]] float traced_path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_path, row));
    }
    /// traced_time: time for traced solution
    [[nodiscard]] float traced_time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_time, row));
    }
    /// traced_stime: time rms for traced solution
    [[nodiscard]] float traced_stime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_stime, row));
    }
    /// traced_nrfl: n reflections for traced solution
    [[nodiscard]] int traced_nrfl(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_traced_nrfl, row));
    }
    /// traced_nrfr: n refractions for traced solution
    [[nodiscard]] int traced_nrfr(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_traced_nrfr, row));
    }
    /// traced_1rfl: first reflection type for traced solution
    [[nodiscard]] int traced_1rfl(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_traced_1rfl, row));
    }
    /// traced_layers: sequence of hit layers (reflections)
    [[nodiscard]] int traced_layers(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_traced_layers, row));
    }
    /// traced_compos: sequence of hit components (reflections)
    [[nodiscard]] int traced_compos(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_traced_compos, row));
    }
    /// traced_etaC: etaC angle for traced solution
    [[nodiscard]] float traced_etaC(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_etaC, row));
    }
    /// traced_aeron: n for traced solution
    [[nodiscard]] float traced_aeron(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_aeron, row));
    }
    /// traced_dthe: pixel equivalent dthe for traced solution
    [[nodiscard]] float traced_dthe(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_dthe, row));
    }
    /// traced_dphi: pixel equivalent dphi for traced solution
    [[nodiscard]] float traced_dphi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_dphi, row));
    }
    /// eff: photon detection efficiency for likelihood
    [[nodiscard]] float eff(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_eff, row));
    }
    /// back: background level for likelihood
    [[nodiscard]] float back(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_back, row));
    }
    /// hypo_pid: PID hypothesis for photon reconstruction
    [[nodiscard]] int hypo_pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_hypo_pid, row));
    }
    /// etac_ref: Cherenkov angle expected value
    [[nodiscard]] float etac_ref(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_etac_ref, row));
    }
    /// etac_rms: Cherenkov angle expected resolution
    [[nodiscard]] float etac_rms(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_etac_rms, row));
    }
    /// prob: probability for traced solution with given hypo
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob, row));
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
    int i_type = -1;
    int i_used = -1;
    int i_start_time = -1;
    int i_traced_the = -1;
    int i_traced_phi = -1;
    int i_traced_hitx = -1;
    int i_traced_hity = -1;
    int i_traced_hitz = -1;
    int i_traced_path = -1;
    int i_traced_time = -1;
    int i_traced_stime = -1;
    int i_traced_nrfl = -1;
    int i_traced_nrfr = -1;
    int i_traced_1rfl = -1;
    int i_traced_layers = -1;
    int i_traced_compos = -1;
    int i_traced_etaC = -1;
    int i_traced_aeron = -1;
    int i_traced_dthe = -1;
    int i_traced_dphi = -1;
    int i_eff = -1;
    int i_back = -1;
    int i_hypo_pid = -1;
    int i_etac_ref = -1;
    int i_etac_rms = -1;
    int i_prob = -1;

    friend class Row;
};

/// Lightweight row view for RICH::Photon. Stores a pointer + row index only.
class RICH_Photon::Row {
 public:
    Row(const RICH_Photon* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int type() const noexcept { return m_bank->type(m_row); }
    [[nodiscard]] int used() const noexcept { return m_bank->used(m_row); }
    [[nodiscard]] float start_time() const noexcept { return m_bank->start_time(m_row); }
    [[nodiscard]] float traced_the() const noexcept { return m_bank->traced_the(m_row); }
    [[nodiscard]] float traced_phi() const noexcept { return m_bank->traced_phi(m_row); }
    [[nodiscard]] float traced_hitx() const noexcept { return m_bank->traced_hitx(m_row); }
    [[nodiscard]] float traced_hity() const noexcept { return m_bank->traced_hity(m_row); }
    [[nodiscard]] float traced_hitz() const noexcept { return m_bank->traced_hitz(m_row); }
    [[nodiscard]] float traced_path() const noexcept { return m_bank->traced_path(m_row); }
    [[nodiscard]] float traced_time() const noexcept { return m_bank->traced_time(m_row); }
    [[nodiscard]] float traced_stime() const noexcept { return m_bank->traced_stime(m_row); }
    [[nodiscard]] int traced_nrfl() const noexcept { return m_bank->traced_nrfl(m_row); }
    [[nodiscard]] int traced_nrfr() const noexcept { return m_bank->traced_nrfr(m_row); }
    [[nodiscard]] int traced_1rfl() const noexcept { return m_bank->traced_1rfl(m_row); }
    [[nodiscard]] int traced_layers() const noexcept { return m_bank->traced_layers(m_row); }
    [[nodiscard]] int traced_compos() const noexcept { return m_bank->traced_compos(m_row); }
    [[nodiscard]] float traced_etaC() const noexcept { return m_bank->traced_etaC(m_row); }
    [[nodiscard]] float traced_aeron() const noexcept { return m_bank->traced_aeron(m_row); }
    [[nodiscard]] float traced_dthe() const noexcept { return m_bank->traced_dthe(m_row); }
    [[nodiscard]] float traced_dphi() const noexcept { return m_bank->traced_dphi(m_row); }
    [[nodiscard]] float eff() const noexcept { return m_bank->eff(m_row); }
    [[nodiscard]] float back() const noexcept { return m_bank->back(m_row); }
    [[nodiscard]] int hypo_pid() const noexcept { return m_bank->hypo_pid(m_row); }
    [[nodiscard]] float etac_ref() const noexcept { return m_bank->etac_ref(m_row); }
    [[nodiscard]] float etac_rms() const noexcept { return m_bank->etac_rms(m_row); }
    [[nodiscard]] float prob() const noexcept { return m_bank->prob(m_row); }
 private:
    const RICH_Photon* m_bank;
    int m_row;
};

inline RICH_Photon::Row RICH_Photon::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
