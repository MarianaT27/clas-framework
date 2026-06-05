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


/// HIPO bank wrapper for `RICH::calib` (group 21800, item 51).
/// Reconstructed Photons in RICH selected for detector calibration
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RICH_calib bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::calib")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto hindex = row.hindex();
/// }
/// @endcode
class RICH_calib {
 public:
    static constexpr std::string_view bank_name = "RICH::calib";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 51;
    static constexpr std::string_view bank_schema = "hindex/S,pindex/B,sector/B,pmt/S,anode/S,x/F,y/F,z/F,time/F,rawtime/F,duration/S,status/S,used/B,emilay/B,emico/B,enico/B,emqua/S,start_time/F,traced_the/F,traced_phi/F,traced_hitx/F,traced_hity/F,traced_hitz/F,traced_path/F,traced_time/F,traced_nrfl/S,traced_nrfr/S,traced_1rfl/S,traced_layers/I,traced_compos/I,traced_etaC/F,etac_ref/F,etac_rms/F,prob/F";

    class Row;

    RICH_calib() = default;

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
            int i_hindex = -1;
            int i_pindex = -1;
            int i_sector = -1;
            int i_pmt = -1;
            int i_anode = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_time = -1;
            int i_rawtime = -1;
            int i_duration = -1;
            int i_status = -1;
            int i_used = -1;
            int i_emilay = -1;
            int i_emico = -1;
            int i_enico = -1;
            int i_emqua = -1;
            int i_start_time = -1;
            int i_traced_the = -1;
            int i_traced_phi = -1;
            int i_traced_hitx = -1;
            int i_traced_hity = -1;
            int i_traced_hitz = -1;
            int i_traced_path = -1;
            int i_traced_time = -1;
            int i_traced_nrfl = -1;
            int i_traced_nrfr = -1;
            int i_traced_1rfl = -1;
            int i_traced_layers = -1;
            int i_traced_compos = -1;
            int i_traced_etaC = -1;
            int i_etac_ref = -1;
            int i_etac_rms = -1;
            int i_prob = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_hindex = schema.exists("hindex") ? schema.getEntryOrder("hindex") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_pmt = schema.exists("pmt") ? schema.getEntryOrder("pmt") : -1;
            cache.i_anode = schema.exists("anode") ? schema.getEntryOrder("anode") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_rawtime = schema.exists("rawtime") ? schema.getEntryOrder("rawtime") : -1;
            cache.i_duration = schema.exists("duration") ? schema.getEntryOrder("duration") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_used = schema.exists("used") ? schema.getEntryOrder("used") : -1;
            cache.i_emilay = schema.exists("emilay") ? schema.getEntryOrder("emilay") : -1;
            cache.i_emico = schema.exists("emico") ? schema.getEntryOrder("emico") : -1;
            cache.i_enico = schema.exists("enico") ? schema.getEntryOrder("enico") : -1;
            cache.i_emqua = schema.exists("emqua") ? schema.getEntryOrder("emqua") : -1;
            cache.i_start_time = schema.exists("start_time") ? schema.getEntryOrder("start_time") : -1;
            cache.i_traced_the = schema.exists("traced_the") ? schema.getEntryOrder("traced_the") : -1;
            cache.i_traced_phi = schema.exists("traced_phi") ? schema.getEntryOrder("traced_phi") : -1;
            cache.i_traced_hitx = schema.exists("traced_hitx") ? schema.getEntryOrder("traced_hitx") : -1;
            cache.i_traced_hity = schema.exists("traced_hity") ? schema.getEntryOrder("traced_hity") : -1;
            cache.i_traced_hitz = schema.exists("traced_hitz") ? schema.getEntryOrder("traced_hitz") : -1;
            cache.i_traced_path = schema.exists("traced_path") ? schema.getEntryOrder("traced_path") : -1;
            cache.i_traced_time = schema.exists("traced_time") ? schema.getEntryOrder("traced_time") : -1;
            cache.i_traced_nrfl = schema.exists("traced_nrfl") ? schema.getEntryOrder("traced_nrfl") : -1;
            cache.i_traced_nrfr = schema.exists("traced_nrfr") ? schema.getEntryOrder("traced_nrfr") : -1;
            cache.i_traced_1rfl = schema.exists("traced_1rfl") ? schema.getEntryOrder("traced_1rfl") : -1;
            cache.i_traced_layers = schema.exists("traced_layers") ? schema.getEntryOrder("traced_layers") : -1;
            cache.i_traced_compos = schema.exists("traced_compos") ? schema.getEntryOrder("traced_compos") : -1;
            cache.i_traced_etaC = schema.exists("traced_etaC") ? schema.getEntryOrder("traced_etaC") : -1;
            cache.i_etac_ref = schema.exists("etac_ref") ? schema.getEntryOrder("etac_ref") : -1;
            cache.i_etac_rms = schema.exists("etac_rms") ? schema.getEntryOrder("etac_rms") : -1;
            cache.i_prob = schema.exists("prob") ? schema.getEntryOrder("prob") : -1;
        }
        i_hindex = cache.i_hindex;
        i_pindex = cache.i_pindex;
        i_sector = cache.i_sector;
        i_pmt = cache.i_pmt;
        i_anode = cache.i_anode;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_time = cache.i_time;
        i_rawtime = cache.i_rawtime;
        i_duration = cache.i_duration;
        i_status = cache.i_status;
        i_used = cache.i_used;
        i_emilay = cache.i_emilay;
        i_emico = cache.i_emico;
        i_enico = cache.i_enico;
        i_emqua = cache.i_emqua;
        i_start_time = cache.i_start_time;
        i_traced_the = cache.i_traced_the;
        i_traced_phi = cache.i_traced_phi;
        i_traced_hitx = cache.i_traced_hitx;
        i_traced_hity = cache.i_traced_hity;
        i_traced_hitz = cache.i_traced_hitz;
        i_traced_path = cache.i_traced_path;
        i_traced_time = cache.i_traced_time;
        i_traced_nrfl = cache.i_traced_nrfl;
        i_traced_nrfr = cache.i_traced_nrfr;
        i_traced_1rfl = cache.i_traced_1rfl;
        i_traced_layers = cache.i_traced_layers;
        i_traced_compos = cache.i_traced_compos;
        i_traced_etaC = cache.i_traced_etaC;
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
    /// hindex: related row in the RICH::hits bank
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_hindex, row));
    }
    /// pindex: related row in the REC::Particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pindex, row));
    }
    /// sector: Hit sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// pmt: Hit pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pmt, row));
    }
    /// anode: Hit anode
    [[nodiscard]] int anode(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_anode, row));
    }
    /// x: Hit x
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Hit y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Hit z
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// time: Hit time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// rawtime: Hit rawtime
    [[nodiscard]] float rawtime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_rawtime, row));
    }
    /// duration: Hit duration
    [[nodiscard]] int duration(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_duration, row));
    }
    /// status: Hit status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// used: eligible for PID in time (1=yes, 0=no) angle (10=yes, 0=no) hypo (100=by other) or cluster (2)
    [[nodiscard]] int used(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_used, row));
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
    int i_hindex = -1;
    int i_pindex = -1;
    int i_sector = -1;
    int i_pmt = -1;
    int i_anode = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_time = -1;
    int i_rawtime = -1;
    int i_duration = -1;
    int i_status = -1;
    int i_used = -1;
    int i_emilay = -1;
    int i_emico = -1;
    int i_enico = -1;
    int i_emqua = -1;
    int i_start_time = -1;
    int i_traced_the = -1;
    int i_traced_phi = -1;
    int i_traced_hitx = -1;
    int i_traced_hity = -1;
    int i_traced_hitz = -1;
    int i_traced_path = -1;
    int i_traced_time = -1;
    int i_traced_nrfl = -1;
    int i_traced_nrfr = -1;
    int i_traced_1rfl = -1;
    int i_traced_layers = -1;
    int i_traced_compos = -1;
    int i_traced_etaC = -1;
    int i_etac_ref = -1;
    int i_etac_rms = -1;
    int i_prob = -1;

    friend class Row;
};

/// Lightweight row view for RICH::calib. Stores a pointer + row index only.
class RICH_calib::Row {
 public:
    Row(const RICH_calib* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] int anode() const noexcept { return m_bank->anode(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float rawtime() const noexcept { return m_bank->rawtime(m_row); }
    [[nodiscard]] int duration() const noexcept { return m_bank->duration(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int used() const noexcept { return m_bank->used(m_row); }
    [[nodiscard]] int emilay() const noexcept { return m_bank->emilay(m_row); }
    [[nodiscard]] int emico() const noexcept { return m_bank->emico(m_row); }
    [[nodiscard]] int enico() const noexcept { return m_bank->enico(m_row); }
    [[nodiscard]] int emqua() const noexcept { return m_bank->emqua(m_row); }
    [[nodiscard]] float start_time() const noexcept { return m_bank->start_time(m_row); }
    [[nodiscard]] float traced_the() const noexcept { return m_bank->traced_the(m_row); }
    [[nodiscard]] float traced_phi() const noexcept { return m_bank->traced_phi(m_row); }
    [[nodiscard]] float traced_hitx() const noexcept { return m_bank->traced_hitx(m_row); }
    [[nodiscard]] float traced_hity() const noexcept { return m_bank->traced_hity(m_row); }
    [[nodiscard]] float traced_hitz() const noexcept { return m_bank->traced_hitz(m_row); }
    [[nodiscard]] float traced_path() const noexcept { return m_bank->traced_path(m_row); }
    [[nodiscard]] float traced_time() const noexcept { return m_bank->traced_time(m_row); }
    [[nodiscard]] int traced_nrfl() const noexcept { return m_bank->traced_nrfl(m_row); }
    [[nodiscard]] int traced_nrfr() const noexcept { return m_bank->traced_nrfr(m_row); }
    [[nodiscard]] int traced_1rfl() const noexcept { return m_bank->traced_1rfl(m_row); }
    [[nodiscard]] int traced_layers() const noexcept { return m_bank->traced_layers(m_row); }
    [[nodiscard]] int traced_compos() const noexcept { return m_bank->traced_compos(m_row); }
    [[nodiscard]] float traced_etaC() const noexcept { return m_bank->traced_etaC(m_row); }
    [[nodiscard]] float etac_ref() const noexcept { return m_bank->etac_ref(m_row); }
    [[nodiscard]] float etac_rms() const noexcept { return m_bank->etac_rms(m_row); }
    [[nodiscard]] float prob() const noexcept { return m_bank->prob(m_row); }
 private:
    const RICH_calib* m_bank;
    int m_row;
};

inline RICH_calib::Row RICH_calib::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
