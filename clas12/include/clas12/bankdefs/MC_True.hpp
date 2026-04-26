// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include <bank_view.h>
#include <schema.h>

namespace clas12::bankdefs {


/// HIPO bank wrapper for `MC::True` (group 40, item 4).
/// True detector information from GEANT4
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    /// Bind to a live bank_view. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column handle by name; all
    /// subsequent bind()s for the same schema hit the cache and copy 25-30
    /// trivially-copyable column_handle words.
    /// Returns false if the view is not valid.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        struct HandleCache {
            const hipo::schema* schema_ptr = nullptr;
            hipo::column_handle h_detector;
            hipo::column_handle h_pid;
            hipo::column_handle h_mpid;
            hipo::column_handle h_tid;
            hipo::column_handle h_mtid;
            hipo::column_handle h_otid;
            hipo::column_handle h_trackE;
            hipo::column_handle h_totEdep;
            hipo::column_handle h_avgX;
            hipo::column_handle h_avgY;
            hipo::column_handle h_avgZ;
            hipo::column_handle h_avgLx;
            hipo::column_handle h_avgLy;
            hipo::column_handle h_avgLz;
            hipo::column_handle h_px;
            hipo::column_handle h_py;
            hipo::column_handle h_pz;
            hipo::column_handle h_vx;
            hipo::column_handle h_vy;
            hipo::column_handle h_vz;
            hipo::column_handle h_mvx;
            hipo::column_handle h_mvy;
            hipo::column_handle h_mvz;
            hipo::column_handle h_avgT;
            hipo::column_handle h_nsteps;
            hipo::column_handle h_procID;
            hipo::column_handle h_hitn;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("detector"); h) cache.h_detector = *h;
            else cache.h_detector = {};
            if (auto h = schema.get_handle("pid"); h) cache.h_pid = *h;
            else cache.h_pid = {};
            if (auto h = schema.get_handle("mpid"); h) cache.h_mpid = *h;
            else cache.h_mpid = {};
            if (auto h = schema.get_handle("tid"); h) cache.h_tid = *h;
            else cache.h_tid = {};
            if (auto h = schema.get_handle("mtid"); h) cache.h_mtid = *h;
            else cache.h_mtid = {};
            if (auto h = schema.get_handle("otid"); h) cache.h_otid = *h;
            else cache.h_otid = {};
            if (auto h = schema.get_handle("trackE"); h) cache.h_trackE = *h;
            else cache.h_trackE = {};
            if (auto h = schema.get_handle("totEdep"); h) cache.h_totEdep = *h;
            else cache.h_totEdep = {};
            if (auto h = schema.get_handle("avgX"); h) cache.h_avgX = *h;
            else cache.h_avgX = {};
            if (auto h = schema.get_handle("avgY"); h) cache.h_avgY = *h;
            else cache.h_avgY = {};
            if (auto h = schema.get_handle("avgZ"); h) cache.h_avgZ = *h;
            else cache.h_avgZ = {};
            if (auto h = schema.get_handle("avgLx"); h) cache.h_avgLx = *h;
            else cache.h_avgLx = {};
            if (auto h = schema.get_handle("avgLy"); h) cache.h_avgLy = *h;
            else cache.h_avgLy = {};
            if (auto h = schema.get_handle("avgLz"); h) cache.h_avgLz = *h;
            else cache.h_avgLz = {};
            if (auto h = schema.get_handle("px"); h) cache.h_px = *h;
            else cache.h_px = {};
            if (auto h = schema.get_handle("py"); h) cache.h_py = *h;
            else cache.h_py = {};
            if (auto h = schema.get_handle("pz"); h) cache.h_pz = *h;
            else cache.h_pz = {};
            if (auto h = schema.get_handle("vx"); h) cache.h_vx = *h;
            else cache.h_vx = {};
            if (auto h = schema.get_handle("vy"); h) cache.h_vy = *h;
            else cache.h_vy = {};
            if (auto h = schema.get_handle("vz"); h) cache.h_vz = *h;
            else cache.h_vz = {};
            if (auto h = schema.get_handle("mvx"); h) cache.h_mvx = *h;
            else cache.h_mvx = {};
            if (auto h = schema.get_handle("mvy"); h) cache.h_mvy = *h;
            else cache.h_mvy = {};
            if (auto h = schema.get_handle("mvz"); h) cache.h_mvz = *h;
            else cache.h_mvz = {};
            if (auto h = schema.get_handle("avgT"); h) cache.h_avgT = *h;
            else cache.h_avgT = {};
            if (auto h = schema.get_handle("nsteps"); h) cache.h_nsteps = *h;
            else cache.h_nsteps = {};
            if (auto h = schema.get_handle("procID"); h) cache.h_procID = *h;
            else cache.h_procID = {};
            if (auto h = schema.get_handle("hitn"); h) cache.h_hitn = *h;
            else cache.h_hitn = {};
        }
        h_detector = cache.h_detector;
        h_pid = cache.h_pid;
        h_mpid = cache.h_mpid;
        h_tid = cache.h_tid;
        h_mtid = cache.h_mtid;
        h_otid = cache.h_otid;
        h_trackE = cache.h_trackE;
        h_totEdep = cache.h_totEdep;
        h_avgX = cache.h_avgX;
        h_avgY = cache.h_avgY;
        h_avgZ = cache.h_avgZ;
        h_avgLx = cache.h_avgLx;
        h_avgLy = cache.h_avgLy;
        h_avgLz = cache.h_avgLz;
        h_px = cache.h_px;
        h_py = cache.h_py;
        h_pz = cache.h_pz;
        h_vx = cache.h_vx;
        h_vy = cache.h_vy;
        h_vz = cache.h_vz;
        h_mvx = cache.h_mvx;
        h_mvy = cache.h_mvy;
        h_mvz = cache.h_mvz;
        h_avgT = cache.h_avgT;
        h_nsteps = cache.h_nsteps;
        h_procID = cache.h_procID;
        h_hitn = cache.h_hitn;
        return true;
    }

    /// Current row count of the bound bank view.
    [[nodiscard]] int rows() const noexcept { return m_view.rows(); }

    /// True if bind() has been called with a valid view.
    [[nodiscard]] bool valid() const noexcept { return m_view.valid(); }

    /// Access the underlying bank_view (for advanced use).
    [[nodiscard]] const hipo::bank_view& view() const noexcept { return m_view; }

    // -------------------------------------------------------------------
    // Per-row scalar getters (handle-based, zero string lookup)
    // -------------------------------------------------------------------
    /// detector: detector ID
    [[nodiscard]] int detector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_detector, row));
    }
    /// pid: ID of the first particle entering the sensitive volume
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pid, row));
    }
    /// mpid: ID of the mother of the first particle entering the sensitive volume
    [[nodiscard]] int mpid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_mpid, row));
    }
    /// tid: Track ID of the first particle entering the sensitive volume
    [[nodiscard]] int tid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_tid, row));
    }
    /// mtid: Track ID of the mother of the first particle entering the sensitive volume
    [[nodiscard]] int mtid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_mtid, row));
    }
    /// otid: Track ID of the original track that generated the first particle entering the sensitive volume
    [[nodiscard]] int otid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_otid, row));
    }
    /// trackE: Energy of the track
    [[nodiscard]] float trackE(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trackE, row));
    }
    /// totEdep: Total Energy Deposited
    [[nodiscard]] float totEdep(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_totEdep, row));
    }
    /// avgX: Average X position in global reference system
    [[nodiscard]] float avgX(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgX, row));
    }
    /// avgY: Average Y position in global reference system
    [[nodiscard]] float avgY(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgY, row));
    }
    /// avgZ: Average Z position in global reference system
    [[nodiscard]] float avgZ(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgZ, row));
    }
    /// avgLx: Average X position in local reference system
    [[nodiscard]] float avgLx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgLx, row));
    }
    /// avgLy: Average Y position in local reference system
    [[nodiscard]] float avgLy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgLy, row));
    }
    /// avgLz: Average Z position in local reference system
    [[nodiscard]] float avgLz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgLz, row));
    }
    /// px: x component of momentum of the particle entering the sensitive volume
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: y component of momentum of the particle entering the sensitive volume
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: z component of momentum of the particle entering the sensitive volume
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// vx: x component of primary vertex of the particle entering the sensitive volume
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vx, row));
    }
    /// vy: y component of primary vertex of the particle entering the sensitive volume
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vy, row));
    }
    /// vz: z component of primary vertex of the particle entering the sensitive volume
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }
    /// mvx: x component of primary vertex of the mother of the particle entering the sensitive volume
    [[nodiscard]] float mvx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_mvx, row));
    }
    /// mvy: y component of primary vertex of the mother of the particle entering the sensitive volume
    [[nodiscard]] float mvy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_mvy, row));
    }
    /// mvz: z component of primary vertex of the mother of the particle entering the sensitive volume
    [[nodiscard]] float mvz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_mvz, row));
    }
    /// avgT: Average time
    [[nodiscard]] float avgT(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_avgT, row));
    }
    /// nsteps: Number of geant4 steps
    [[nodiscard]] int nsteps(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_nsteps, row));
    }
    /// procID: Process that created the FP. see gemc.jlab.org
    [[nodiscard]] int procID(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_procID, row));
    }
    /// hitn: Hit number
    [[nodiscard]] int hitn(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_hitn, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// detector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> detector_col() const noexcept {
        return m_view.column<int8_t>(h_detector);
    }
    /// pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pid_col() const noexcept {
        return m_view.column<int32_t>(h_pid);
    }
    /// mpid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> mpid_col() const noexcept {
        return m_view.column<int32_t>(h_mpid);
    }
    /// tid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> tid_col() const noexcept {
        return m_view.column<int32_t>(h_tid);
    }
    /// mtid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> mtid_col() const noexcept {
        return m_view.column<int32_t>(h_mtid);
    }
    /// otid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> otid_col() const noexcept {
        return m_view.column<int32_t>(h_otid);
    }
    /// trackE column as a contiguous span.
    [[nodiscard]] std::span<const float> trackE_col() const noexcept {
        return m_view.column<float>(h_trackE);
    }
    /// totEdep column as a contiguous span.
    [[nodiscard]] std::span<const float> totEdep_col() const noexcept {
        return m_view.column<float>(h_totEdep);
    }
    /// avgX column as a contiguous span.
    [[nodiscard]] std::span<const float> avgX_col() const noexcept {
        return m_view.column<float>(h_avgX);
    }
    /// avgY column as a contiguous span.
    [[nodiscard]] std::span<const float> avgY_col() const noexcept {
        return m_view.column<float>(h_avgY);
    }
    /// avgZ column as a contiguous span.
    [[nodiscard]] std::span<const float> avgZ_col() const noexcept {
        return m_view.column<float>(h_avgZ);
    }
    /// avgLx column as a contiguous span.
    [[nodiscard]] std::span<const float> avgLx_col() const noexcept {
        return m_view.column<float>(h_avgLx);
    }
    /// avgLy column as a contiguous span.
    [[nodiscard]] std::span<const float> avgLy_col() const noexcept {
        return m_view.column<float>(h_avgLy);
    }
    /// avgLz column as a contiguous span.
    [[nodiscard]] std::span<const float> avgLz_col() const noexcept {
        return m_view.column<float>(h_avgLz);
    }
    /// px column as a contiguous span.
    [[nodiscard]] std::span<const float> px_col() const noexcept {
        return m_view.column<float>(h_px);
    }
    /// py column as a contiguous span.
    [[nodiscard]] std::span<const float> py_col() const noexcept {
        return m_view.column<float>(h_py);
    }
    /// pz column as a contiguous span.
    [[nodiscard]] std::span<const float> pz_col() const noexcept {
        return m_view.column<float>(h_pz);
    }
    /// vx column as a contiguous span.
    [[nodiscard]] std::span<const float> vx_col() const noexcept {
        return m_view.column<float>(h_vx);
    }
    /// vy column as a contiguous span.
    [[nodiscard]] std::span<const float> vy_col() const noexcept {
        return m_view.column<float>(h_vy);
    }
    /// vz column as a contiguous span.
    [[nodiscard]] std::span<const float> vz_col() const noexcept {
        return m_view.column<float>(h_vz);
    }
    /// mvx column as a contiguous span.
    [[nodiscard]] std::span<const float> mvx_col() const noexcept {
        return m_view.column<float>(h_mvx);
    }
    /// mvy column as a contiguous span.
    [[nodiscard]] std::span<const float> mvy_col() const noexcept {
        return m_view.column<float>(h_mvy);
    }
    /// mvz column as a contiguous span.
    [[nodiscard]] std::span<const float> mvz_col() const noexcept {
        return m_view.column<float>(h_mvz);
    }
    /// avgT column as a contiguous span.
    [[nodiscard]] std::span<const float> avgT_col() const noexcept {
        return m_view.column<float>(h_avgT);
    }
    /// nsteps column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> nsteps_col() const noexcept {
        return m_view.column<int32_t>(h_nsteps);
    }
    /// procID column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> procID_col() const noexcept {
        return m_view.column<int32_t>(h_procID);
    }
    /// hitn column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> hitn_col() const noexcept {
        return m_view.column<int32_t>(h_hitn);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_detector{};
    hipo::column_handle h_pid{};
    hipo::column_handle h_mpid{};
    hipo::column_handle h_tid{};
    hipo::column_handle h_mtid{};
    hipo::column_handle h_otid{};
    hipo::column_handle h_trackE{};
    hipo::column_handle h_totEdep{};
    hipo::column_handle h_avgX{};
    hipo::column_handle h_avgY{};
    hipo::column_handle h_avgZ{};
    hipo::column_handle h_avgLx{};
    hipo::column_handle h_avgLy{};
    hipo::column_handle h_avgLz{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_vx{};
    hipo::column_handle h_vy{};
    hipo::column_handle h_vz{};
    hipo::column_handle h_mvx{};
    hipo::column_handle h_mvy{};
    hipo::column_handle h_mvz{};
    hipo::column_handle h_avgT{};
    hipo::column_handle h_nsteps{};
    hipo::column_handle h_procID{};
    hipo::column_handle h_hitn{};

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

}  // namespace clas12::bankdefs
