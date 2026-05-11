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


/// HIPO bank wrapper for `MC::True` (group 40, item 4).
/// True detector information from GEANT4
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// MC_True bank;
/// if (auto sc = bank.bind(event.get_bank("MC::True")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto detector = row.detector();
/// }
/// @endcode
class MC_True {
 public:
    static constexpr std::string_view bank_name = "MC::True";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 4;

    class Row;

    MC_True() = default;

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
            int i_detector = -1;
            int i_pid = -1;
            int i_mpid = -1;
            int i_tid = -1;
            int i_mtid = -1;
            int i_otid = -1;
            int i_trackE = -1;
            int i_totEdep = -1;
            int i_avgX = -1;
            int i_avgY = -1;
            int i_avgZ = -1;
            int i_avgLx = -1;
            int i_avgLy = -1;
            int i_avgLz = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_vx = -1;
            int i_vy = -1;
            int i_vz = -1;
            int i_mvx = -1;
            int i_mvy = -1;
            int i_mvz = -1;
            int i_avgT = -1;
            int i_nsteps = -1;
            int i_procID = -1;
            int i_hitn = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_detector = schema.exists("detector") ? schema.getEntryOrder("detector") : -1;
            cache.i_pid = schema.exists("pid") ? schema.getEntryOrder("pid") : -1;
            cache.i_mpid = schema.exists("mpid") ? schema.getEntryOrder("mpid") : -1;
            cache.i_tid = schema.exists("tid") ? schema.getEntryOrder("tid") : -1;
            cache.i_mtid = schema.exists("mtid") ? schema.getEntryOrder("mtid") : -1;
            cache.i_otid = schema.exists("otid") ? schema.getEntryOrder("otid") : -1;
            cache.i_trackE = schema.exists("trackE") ? schema.getEntryOrder("trackE") : -1;
            cache.i_totEdep = schema.exists("totEdep") ? schema.getEntryOrder("totEdep") : -1;
            cache.i_avgX = schema.exists("avgX") ? schema.getEntryOrder("avgX") : -1;
            cache.i_avgY = schema.exists("avgY") ? schema.getEntryOrder("avgY") : -1;
            cache.i_avgZ = schema.exists("avgZ") ? schema.getEntryOrder("avgZ") : -1;
            cache.i_avgLx = schema.exists("avgLx") ? schema.getEntryOrder("avgLx") : -1;
            cache.i_avgLy = schema.exists("avgLy") ? schema.getEntryOrder("avgLy") : -1;
            cache.i_avgLz = schema.exists("avgLz") ? schema.getEntryOrder("avgLz") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_vx = schema.exists("vx") ? schema.getEntryOrder("vx") : -1;
            cache.i_vy = schema.exists("vy") ? schema.getEntryOrder("vy") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
            cache.i_mvx = schema.exists("mvx") ? schema.getEntryOrder("mvx") : -1;
            cache.i_mvy = schema.exists("mvy") ? schema.getEntryOrder("mvy") : -1;
            cache.i_mvz = schema.exists("mvz") ? schema.getEntryOrder("mvz") : -1;
            cache.i_avgT = schema.exists("avgT") ? schema.getEntryOrder("avgT") : -1;
            cache.i_nsteps = schema.exists("nsteps") ? schema.getEntryOrder("nsteps") : -1;
            cache.i_procID = schema.exists("procID") ? schema.getEntryOrder("procID") : -1;
            cache.i_hitn = schema.exists("hitn") ? schema.getEntryOrder("hitn") : -1;
        }
        i_detector = cache.i_detector;
        i_pid = cache.i_pid;
        i_mpid = cache.i_mpid;
        i_tid = cache.i_tid;
        i_mtid = cache.i_mtid;
        i_otid = cache.i_otid;
        i_trackE = cache.i_trackE;
        i_totEdep = cache.i_totEdep;
        i_avgX = cache.i_avgX;
        i_avgY = cache.i_avgY;
        i_avgZ = cache.i_avgZ;
        i_avgLx = cache.i_avgLx;
        i_avgLy = cache.i_avgLy;
        i_avgLz = cache.i_avgLz;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_vx = cache.i_vx;
        i_vy = cache.i_vy;
        i_vz = cache.i_vz;
        i_mvx = cache.i_mvx;
        i_mvy = cache.i_mvy;
        i_mvz = cache.i_mvz;
        i_avgT = cache.i_avgT;
        i_nsteps = cache.i_nsteps;
        i_procID = cache.i_procID;
        i_hitn = cache.i_hitn;
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
    /// detector: detector ID
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_detector, row));
    }
    /// pid: ID of the first particle entering the sensitive volume
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pid, row));
    }
    /// mpid: ID of the mother of the first particle entering the sensitive volume
    [[nodiscard]] int mpid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_mpid, row));
    }
    /// tid: Track ID of the first particle entering the sensitive volume
    [[nodiscard]] int tid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tid, row));
    }
    /// mtid: Track ID of the mother of the first particle entering the sensitive volume
    [[nodiscard]] int mtid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_mtid, row));
    }
    /// otid: Track ID of the original track that generated the first particle entering the sensitive volume
    [[nodiscard]] int otid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_otid, row));
    }
    /// trackE: Energy of the track
    [[nodiscard]] float trackE(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_trackE, row));
    }
    /// totEdep: Total Energy Deposited
    [[nodiscard]] float totEdep(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_totEdep, row));
    }
    /// avgX: Average X position in global reference system
    [[nodiscard]] float avgX(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgX, row));
    }
    /// avgY: Average Y position in global reference system
    [[nodiscard]] float avgY(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgY, row));
    }
    /// avgZ: Average Z position in global reference system
    [[nodiscard]] float avgZ(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgZ, row));
    }
    /// avgLx: Average X position in local reference system
    [[nodiscard]] float avgLx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgLx, row));
    }
    /// avgLy: Average Y position in local reference system
    [[nodiscard]] float avgLy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgLy, row));
    }
    /// avgLz: Average Z position in local reference system
    [[nodiscard]] float avgLz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgLz, row));
    }
    /// px: x component of momentum of the particle entering the sensitive volume
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: y component of momentum of the particle entering the sensitive volume
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: z component of momentum of the particle entering the sensitive volume
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// vx: x component of primary vertex of the particle entering the sensitive volume
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vx, row));
    }
    /// vy: y component of primary vertex of the particle entering the sensitive volume
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vy, row));
    }
    /// vz: z component of primary vertex of the particle entering the sensitive volume
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }
    /// mvx: x component of primary vertex of the mother of the particle entering the sensitive volume
    [[nodiscard]] float mvx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_mvx, row));
    }
    /// mvy: y component of primary vertex of the mother of the particle entering the sensitive volume
    [[nodiscard]] float mvy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_mvy, row));
    }
    /// mvz: z component of primary vertex of the mother of the particle entering the sensitive volume
    [[nodiscard]] float mvz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_mvz, row));
    }
    /// avgT: Average time
    [[nodiscard]] float avgT(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_avgT, row));
    }
    /// nsteps: Number of geant4 steps
    [[nodiscard]] int nsteps(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_nsteps, row));
    }
    /// procID: Process that created the FP. see gemc.jlab.org
    [[nodiscard]] int procID(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_procID, row));
    }
    /// hitn: Hit number
    [[nodiscard]] int hitn(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_hitn, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_detector = -1;
    int i_pid = -1;
    int i_mpid = -1;
    int i_tid = -1;
    int i_mtid = -1;
    int i_otid = -1;
    int i_trackE = -1;
    int i_totEdep = -1;
    int i_avgX = -1;
    int i_avgY = -1;
    int i_avgZ = -1;
    int i_avgLx = -1;
    int i_avgLy = -1;
    int i_avgLz = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_vx = -1;
    int i_vy = -1;
    int i_vz = -1;
    int i_mvx = -1;
    int i_mvy = -1;
    int i_mvz = -1;
    int i_avgT = -1;
    int i_nsteps = -1;
    int i_procID = -1;
    int i_hitn = -1;

    friend class Row;
};

/// Lightweight row view for MC::True. Stores a pointer + row index only.
class MC_True::Row {
 public:
    Row(const MC_True* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int detector() const noexcept { return m_bank->detector(m_row); }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] int mpid() const noexcept { return m_bank->mpid(m_row); }
    [[nodiscard]] int tid() const noexcept { return m_bank->tid(m_row); }
    [[nodiscard]] int mtid() const noexcept { return m_bank->mtid(m_row); }
    [[nodiscard]] int otid() const noexcept { return m_bank->otid(m_row); }
    [[nodiscard]] float trackE() const noexcept { return m_bank->trackE(m_row); }
    [[nodiscard]] float totEdep() const noexcept { return m_bank->totEdep(m_row); }
    [[nodiscard]] float avgX() const noexcept { return m_bank->avgX(m_row); }
    [[nodiscard]] float avgY() const noexcept { return m_bank->avgY(m_row); }
    [[nodiscard]] float avgZ() const noexcept { return m_bank->avgZ(m_row); }
    [[nodiscard]] float avgLx() const noexcept { return m_bank->avgLx(m_row); }
    [[nodiscard]] float avgLy() const noexcept { return m_bank->avgLy(m_row); }
    [[nodiscard]] float avgLz() const noexcept { return m_bank->avgLz(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float vx() const noexcept { return m_bank->vx(m_row); }
    [[nodiscard]] float vy() const noexcept { return m_bank->vy(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float mvx() const noexcept { return m_bank->mvx(m_row); }
    [[nodiscard]] float mvy() const noexcept { return m_bank->mvy(m_row); }
    [[nodiscard]] float mvz() const noexcept { return m_bank->mvz(m_row); }
    [[nodiscard]] float avgT() const noexcept { return m_bank->avgT(m_row); }
    [[nodiscard]] int nsteps() const noexcept { return m_bank->nsteps(m_row); }
    [[nodiscard]] int procID() const noexcept { return m_bank->procID(m_row); }
    [[nodiscard]] int hitn() const noexcept { return m_bank->hitn(m_row); }
 private:
    const MC_True* m_bank;
    int m_row;
};

inline MC_True::Row MC_True::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
