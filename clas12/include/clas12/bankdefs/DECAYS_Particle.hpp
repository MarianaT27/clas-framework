// =============================================================================
// Auto-generated from decays.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `DECAYS::Particle` (group 900, item 11).
/// Reconstructed Particle Information
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// DECAYS_Particle bank;
/// if (auto sc = bank.bind(event.get_bank("DECAYS::Particle")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto idx = row.idx();
/// }
/// @endcode
class DECAYS_Particle {
 public:
    static constexpr std::string_view bank_name = "DECAYS::Particle";
    static constexpr int bank_group = 900;
    static constexpr int bank_item  = 11;

    class Row;

    DECAYS_Particle() = default;

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
            int i_idx = -1;
            int i_pid = -1;
            int i_emc = -1;
            int i_erec = -1;
            int i_e = -1;
            int i_ovx = -1;
            int i_ovy = -1;
            int i_ovz = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_upx = -1;
            int i_upy = -1;
            int i_upz = -1;
            int i_vx = -1;
            int i_vy = -1;
            int i_vz = -1;
            int i_r = -1;
            int i_charge = -1;
            int i_mass = -1;
            int i_umass = -1;
            int i_ndau = -1;
            int i_dau1idx = -1;
            int i_dau2idx = -1;
            int i_dau3idx = -1;
            int i_det = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_idx = schema.exists("idx") ? schema.getEntryOrder("idx") : -1;
            cache.i_pid = schema.exists("pid") ? schema.getEntryOrder("pid") : -1;
            cache.i_emc = schema.exists("emc") ? schema.getEntryOrder("emc") : -1;
            cache.i_erec = schema.exists("erec") ? schema.getEntryOrder("erec") : -1;
            cache.i_e = schema.exists("e") ? schema.getEntryOrder("e") : -1;
            cache.i_ovx = schema.exists("ovx") ? schema.getEntryOrder("ovx") : -1;
            cache.i_ovy = schema.exists("ovy") ? schema.getEntryOrder("ovy") : -1;
            cache.i_ovz = schema.exists("ovz") ? schema.getEntryOrder("ovz") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_upx = schema.exists("upx") ? schema.getEntryOrder("upx") : -1;
            cache.i_upy = schema.exists("upy") ? schema.getEntryOrder("upy") : -1;
            cache.i_upz = schema.exists("upz") ? schema.getEntryOrder("upz") : -1;
            cache.i_vx = schema.exists("vx") ? schema.getEntryOrder("vx") : -1;
            cache.i_vy = schema.exists("vy") ? schema.getEntryOrder("vy") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
            cache.i_r = schema.exists("r") ? schema.getEntryOrder("r") : -1;
            cache.i_charge = schema.exists("charge") ? schema.getEntryOrder("charge") : -1;
            cache.i_mass = schema.exists("mass") ? schema.getEntryOrder("mass") : -1;
            cache.i_umass = schema.exists("umass") ? schema.getEntryOrder("umass") : -1;
            cache.i_ndau = schema.exists("ndau") ? schema.getEntryOrder("ndau") : -1;
            cache.i_dau1idx = schema.exists("dau1idx") ? schema.getEntryOrder("dau1idx") : -1;
            cache.i_dau2idx = schema.exists("dau2idx") ? schema.getEntryOrder("dau2idx") : -1;
            cache.i_dau3idx = schema.exists("dau3idx") ? schema.getEntryOrder("dau3idx") : -1;
            cache.i_det = schema.exists("det") ? schema.getEntryOrder("det") : -1;
        }
        i_idx = cache.i_idx;
        i_pid = cache.i_pid;
        i_emc = cache.i_emc;
        i_erec = cache.i_erec;
        i_e = cache.i_e;
        i_ovx = cache.i_ovx;
        i_ovy = cache.i_ovy;
        i_ovz = cache.i_ovz;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_upx = cache.i_upx;
        i_upy = cache.i_upy;
        i_upz = cache.i_upz;
        i_vx = cache.i_vx;
        i_vy = cache.i_vy;
        i_vz = cache.i_vz;
        i_r = cache.i_r;
        i_charge = cache.i_charge;
        i_mass = cache.i_mass;
        i_umass = cache.i_umass;
        i_ndau = cache.i_ndau;
        i_dau1idx = cache.i_dau1idx;
        i_dau2idx = cache.i_dau2idx;
        i_dau3idx = cache.i_dau3idx;
        i_det = cache.i_det;
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
    /// idx: particle idx
    [[nodiscard]] int idx(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_idx, row));
    }
    /// pid: particle id in LUND conventions
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pid, row));
    }
    /// emc: particle mass constrained energy
    [[nodiscard]] float emc(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_emc, row));
    }
    /// erec: reconstructed particle energy
    [[nodiscard]] float erec(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_erec, row));
    }
    /// e: analysis particle energy (sum of constrained energies or daughters if mass constrain flag is on
    [[nodiscard]] float e(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_e, row));
    }
    /// ovx: x component of the vertex at the production vertex (cm)
    [[nodiscard]] float ovx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ovx, row));
    }
    /// ovy: y component of the vertex at the production vertex (cm)
    [[nodiscard]] float ovy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ovy, row));
    }
    /// ovz: z component of the vertex at the production vertex (cm)
    [[nodiscard]] float ovz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ovz, row));
    }
    /// px: x component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// upx: x component of the uncorrected momentum at the vertex (GeV)
    [[nodiscard]] float upx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_upx, row));
    }
    /// upy: y component of the uncorrected momentum at the vertex (GeV)
    [[nodiscard]] float upy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_upy, row));
    }
    /// upz: z component of the uncorrected momentum at the vertex (GeV)
    [[nodiscard]] float upz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_upz, row));
    }
    /// vx: x component of the vertex (cm)
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vx, row));
    }
    /// vy: y component of the vertex (cm)
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vy, row));
    }
    /// vz: z component of the vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }
    /// r: distance between helices for 2-particle decay (cm)
    [[nodiscard]] float r(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_r, row));
    }
    /// charge: particle charge
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_charge, row));
    }
    /// mass: particle mass (GeV)
    [[nodiscard]] float mass(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_mass, row));
    }
    /// umass: particle uncorrected mass (GeV)
    [[nodiscard]] float umass(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_umass, row));
    }
    /// ndau: number of daughters
    [[nodiscard]] int ndau(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_ndau, row));
    }
    /// dau1idx: daughter 1 idx
    [[nodiscard]] int dau1idx(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_dau1idx, row));
    }
    /// dau2idx: daughter 2 idx
    [[nodiscard]] int dau2idx(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_dau2idx, row));
    }
    /// dau3idx: daughter 3 idx
    [[nodiscard]] int dau3idx(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_dau3idx, row));
    }
    /// det: det=0 for central, det=1 for forward tracker
    [[nodiscard]] int det(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_det, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_idx = -1;
    int i_pid = -1;
    int i_emc = -1;
    int i_erec = -1;
    int i_e = -1;
    int i_ovx = -1;
    int i_ovy = -1;
    int i_ovz = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_upx = -1;
    int i_upy = -1;
    int i_upz = -1;
    int i_vx = -1;
    int i_vy = -1;
    int i_vz = -1;
    int i_r = -1;
    int i_charge = -1;
    int i_mass = -1;
    int i_umass = -1;
    int i_ndau = -1;
    int i_dau1idx = -1;
    int i_dau2idx = -1;
    int i_dau3idx = -1;
    int i_det = -1;

    friend class Row;
};

/// Lightweight row view for DECAYS::Particle. Stores a pointer + row index only.
class DECAYS_Particle::Row {
 public:
    Row(const DECAYS_Particle* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int idx() const noexcept { return m_bank->idx(m_row); }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] float emc() const noexcept { return m_bank->emc(m_row); }
    [[nodiscard]] float erec() const noexcept { return m_bank->erec(m_row); }
    [[nodiscard]] float e() const noexcept { return m_bank->e(m_row); }
    [[nodiscard]] float ovx() const noexcept { return m_bank->ovx(m_row); }
    [[nodiscard]] float ovy() const noexcept { return m_bank->ovy(m_row); }
    [[nodiscard]] float ovz() const noexcept { return m_bank->ovz(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float upx() const noexcept { return m_bank->upx(m_row); }
    [[nodiscard]] float upy() const noexcept { return m_bank->upy(m_row); }
    [[nodiscard]] float upz() const noexcept { return m_bank->upz(m_row); }
    [[nodiscard]] float vx() const noexcept { return m_bank->vx(m_row); }
    [[nodiscard]] float vy() const noexcept { return m_bank->vy(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float r() const noexcept { return m_bank->r(m_row); }
    [[nodiscard]] int charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float mass() const noexcept { return m_bank->mass(m_row); }
    [[nodiscard]] float umass() const noexcept { return m_bank->umass(m_row); }
    [[nodiscard]] int ndau() const noexcept { return m_bank->ndau(m_row); }
    [[nodiscard]] int dau1idx() const noexcept { return m_bank->dau1idx(m_row); }
    [[nodiscard]] int dau2idx() const noexcept { return m_bank->dau2idx(m_row); }
    [[nodiscard]] int dau3idx() const noexcept { return m_bank->dau3idx(m_row); }
    [[nodiscard]] int det() const noexcept { return m_bank->det(m_row); }
 private:
    const DECAYS_Particle* m_bank;
    int m_row;
};

inline DECAYS_Particle::Row DECAYS_Particle::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
