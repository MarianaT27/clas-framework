// =============================================================================
// Auto-generated from decays.json - do not edit by hand.
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


/// HIPO bank wrapper for `DECAYS::Particle` (group 900, item 11).
/// Reconstructed Particle Information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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
            hipo::column_handle h_idx;
            hipo::column_handle h_pid;
            hipo::column_handle h_emc;
            hipo::column_handle h_erec;
            hipo::column_handle h_e;
            hipo::column_handle h_ovx;
            hipo::column_handle h_ovy;
            hipo::column_handle h_ovz;
            hipo::column_handle h_px;
            hipo::column_handle h_py;
            hipo::column_handle h_pz;
            hipo::column_handle h_upx;
            hipo::column_handle h_upy;
            hipo::column_handle h_upz;
            hipo::column_handle h_vx;
            hipo::column_handle h_vy;
            hipo::column_handle h_vz;
            hipo::column_handle h_r;
            hipo::column_handle h_charge;
            hipo::column_handle h_mass;
            hipo::column_handle h_umass;
            hipo::column_handle h_ndau;
            hipo::column_handle h_dau1idx;
            hipo::column_handle h_dau2idx;
            hipo::column_handle h_dau3idx;
            hipo::column_handle h_det;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("idx"); h) cache.h_idx = *h;
            else cache.h_idx = {};
            if (auto h = schema.get_handle("pid"); h) cache.h_pid = *h;
            else cache.h_pid = {};
            if (auto h = schema.get_handle("emc"); h) cache.h_emc = *h;
            else cache.h_emc = {};
            if (auto h = schema.get_handle("erec"); h) cache.h_erec = *h;
            else cache.h_erec = {};
            if (auto h = schema.get_handle("e"); h) cache.h_e = *h;
            else cache.h_e = {};
            if (auto h = schema.get_handle("ovx"); h) cache.h_ovx = *h;
            else cache.h_ovx = {};
            if (auto h = schema.get_handle("ovy"); h) cache.h_ovy = *h;
            else cache.h_ovy = {};
            if (auto h = schema.get_handle("ovz"); h) cache.h_ovz = *h;
            else cache.h_ovz = {};
            if (auto h = schema.get_handle("px"); h) cache.h_px = *h;
            else cache.h_px = {};
            if (auto h = schema.get_handle("py"); h) cache.h_py = *h;
            else cache.h_py = {};
            if (auto h = schema.get_handle("pz"); h) cache.h_pz = *h;
            else cache.h_pz = {};
            if (auto h = schema.get_handle("upx"); h) cache.h_upx = *h;
            else cache.h_upx = {};
            if (auto h = schema.get_handle("upy"); h) cache.h_upy = *h;
            else cache.h_upy = {};
            if (auto h = schema.get_handle("upz"); h) cache.h_upz = *h;
            else cache.h_upz = {};
            if (auto h = schema.get_handle("vx"); h) cache.h_vx = *h;
            else cache.h_vx = {};
            if (auto h = schema.get_handle("vy"); h) cache.h_vy = *h;
            else cache.h_vy = {};
            if (auto h = schema.get_handle("vz"); h) cache.h_vz = *h;
            else cache.h_vz = {};
            if (auto h = schema.get_handle("r"); h) cache.h_r = *h;
            else cache.h_r = {};
            if (auto h = schema.get_handle("charge"); h) cache.h_charge = *h;
            else cache.h_charge = {};
            if (auto h = schema.get_handle("mass"); h) cache.h_mass = *h;
            else cache.h_mass = {};
            if (auto h = schema.get_handle("umass"); h) cache.h_umass = *h;
            else cache.h_umass = {};
            if (auto h = schema.get_handle("ndau"); h) cache.h_ndau = *h;
            else cache.h_ndau = {};
            if (auto h = schema.get_handle("dau1idx"); h) cache.h_dau1idx = *h;
            else cache.h_dau1idx = {};
            if (auto h = schema.get_handle("dau2idx"); h) cache.h_dau2idx = *h;
            else cache.h_dau2idx = {};
            if (auto h = schema.get_handle("dau3idx"); h) cache.h_dau3idx = *h;
            else cache.h_dau3idx = {};
            if (auto h = schema.get_handle("det"); h) cache.h_det = *h;
            else cache.h_det = {};
        }
        h_idx = cache.h_idx;
        h_pid = cache.h_pid;
        h_emc = cache.h_emc;
        h_erec = cache.h_erec;
        h_e = cache.h_e;
        h_ovx = cache.h_ovx;
        h_ovy = cache.h_ovy;
        h_ovz = cache.h_ovz;
        h_px = cache.h_px;
        h_py = cache.h_py;
        h_pz = cache.h_pz;
        h_upx = cache.h_upx;
        h_upy = cache.h_upy;
        h_upz = cache.h_upz;
        h_vx = cache.h_vx;
        h_vy = cache.h_vy;
        h_vz = cache.h_vz;
        h_r = cache.h_r;
        h_charge = cache.h_charge;
        h_mass = cache.h_mass;
        h_umass = cache.h_umass;
        h_ndau = cache.h_ndau;
        h_dau1idx = cache.h_dau1idx;
        h_dau2idx = cache.h_dau2idx;
        h_dau3idx = cache.h_dau3idx;
        h_det = cache.h_det;
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
    /// idx: particle idx
    [[nodiscard]] int idx(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_idx, row));
    }
    /// pid: particle id in LUND conventions
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pid, row));
    }
    /// emc: particle mass constrained energy
    [[nodiscard]] float emc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_emc, row));
    }
    /// erec: reconstructed particle energy
    [[nodiscard]] float erec(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_erec, row));
    }
    /// e: analysis particle energy (sum of constrained energies or daughters if mass constrain flag is on
    [[nodiscard]] float e(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_e, row));
    }
    /// ovx: x component of the vertex at the production vertex (cm)
    [[nodiscard]] float ovx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ovx, row));
    }
    /// ovy: y component of the vertex at the production vertex (cm)
    [[nodiscard]] float ovy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ovy, row));
    }
    /// ovz: z component of the vertex at the production vertex (cm)
    [[nodiscard]] float ovz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ovz, row));
    }
    /// px: x component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// upx: x component of the uncorrected momentum at the vertex (GeV)
    [[nodiscard]] float upx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_upx, row));
    }
    /// upy: y component of the uncorrected momentum at the vertex (GeV)
    [[nodiscard]] float upy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_upy, row));
    }
    /// upz: z component of the uncorrected momentum at the vertex (GeV)
    [[nodiscard]] float upz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_upz, row));
    }
    /// vx: x component of the vertex (cm)
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vx, row));
    }
    /// vy: y component of the vertex (cm)
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vy, row));
    }
    /// vz: z component of the vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }
    /// r: distance between helices for 2-particle decay (cm)
    [[nodiscard]] float r(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_r, row));
    }
    /// charge: particle charge
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_charge, row));
    }
    /// mass: particle mass (GeV)
    [[nodiscard]] float mass(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_mass, row));
    }
    /// umass: particle uncorrected mass (GeV)
    [[nodiscard]] float umass(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_umass, row));
    }
    /// ndau: number of daughters
    [[nodiscard]] int ndau(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_ndau, row));
    }
    /// dau1idx: daughter 1 idx
    [[nodiscard]] int dau1idx(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_dau1idx, row));
    }
    /// dau2idx: daughter 2 idx
    [[nodiscard]] int dau2idx(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_dau2idx, row));
    }
    /// dau3idx: daughter 3 idx
    [[nodiscard]] int dau3idx(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_dau3idx, row));
    }
    /// det: det=0 for central, det=1 for forward tracker
    [[nodiscard]] int det(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_det, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// idx column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> idx_col() const noexcept {
        return m_view.column<int16_t>(h_idx);
    }
    /// pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pid_col() const noexcept {
        return m_view.column<int32_t>(h_pid);
    }
    /// emc column as a contiguous span.
    [[nodiscard]] std::span<const float> emc_col() const noexcept {
        return m_view.column<float>(h_emc);
    }
    /// erec column as a contiguous span.
    [[nodiscard]] std::span<const float> erec_col() const noexcept {
        return m_view.column<float>(h_erec);
    }
    /// e column as a contiguous span.
    [[nodiscard]] std::span<const float> e_col() const noexcept {
        return m_view.column<float>(h_e);
    }
    /// ovx column as a contiguous span.
    [[nodiscard]] std::span<const float> ovx_col() const noexcept {
        return m_view.column<float>(h_ovx);
    }
    /// ovy column as a contiguous span.
    [[nodiscard]] std::span<const float> ovy_col() const noexcept {
        return m_view.column<float>(h_ovy);
    }
    /// ovz column as a contiguous span.
    [[nodiscard]] std::span<const float> ovz_col() const noexcept {
        return m_view.column<float>(h_ovz);
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
    /// upx column as a contiguous span.
    [[nodiscard]] std::span<const float> upx_col() const noexcept {
        return m_view.column<float>(h_upx);
    }
    /// upy column as a contiguous span.
    [[nodiscard]] std::span<const float> upy_col() const noexcept {
        return m_view.column<float>(h_upy);
    }
    /// upz column as a contiguous span.
    [[nodiscard]] std::span<const float> upz_col() const noexcept {
        return m_view.column<float>(h_upz);
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
    /// r column as a contiguous span.
    [[nodiscard]] std::span<const float> r_col() const noexcept {
        return m_view.column<float>(h_r);
    }
    /// charge column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> charge_col() const noexcept {
        return m_view.column<int8_t>(h_charge);
    }
    /// mass column as a contiguous span.
    [[nodiscard]] std::span<const float> mass_col() const noexcept {
        return m_view.column<float>(h_mass);
    }
    /// umass column as a contiguous span.
    [[nodiscard]] std::span<const float> umass_col() const noexcept {
        return m_view.column<float>(h_umass);
    }
    /// ndau column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> ndau_col() const noexcept {
        return m_view.column<int8_t>(h_ndau);
    }
    /// dau1idx column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> dau1idx_col() const noexcept {
        return m_view.column<int16_t>(h_dau1idx);
    }
    /// dau2idx column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> dau2idx_col() const noexcept {
        return m_view.column<int16_t>(h_dau2idx);
    }
    /// dau3idx column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> dau3idx_col() const noexcept {
        return m_view.column<int16_t>(h_dau3idx);
    }
    /// det column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> det_col() const noexcept {
        return m_view.column<int8_t>(h_det);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_idx{};
    hipo::column_handle h_pid{};
    hipo::column_handle h_emc{};
    hipo::column_handle h_erec{};
    hipo::column_handle h_e{};
    hipo::column_handle h_ovx{};
    hipo::column_handle h_ovy{};
    hipo::column_handle h_ovz{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_upx{};
    hipo::column_handle h_upy{};
    hipo::column_handle h_upz{};
    hipo::column_handle h_vx{};
    hipo::column_handle h_vy{};
    hipo::column_handle h_vz{};
    hipo::column_handle h_r{};
    hipo::column_handle h_charge{};
    hipo::column_handle h_mass{};
    hipo::column_handle h_umass{};
    hipo::column_handle h_ndau{};
    hipo::column_handle h_dau1idx{};
    hipo::column_handle h_dau2idx{};
    hipo::column_handle h_dau3idx{};
    hipo::column_handle h_det{};

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

}  // namespace clas12::bankdefs
