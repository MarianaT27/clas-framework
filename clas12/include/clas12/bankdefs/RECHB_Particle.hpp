// =============================================================================
// Auto-generated from event.json - do not edit by hand.
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


/// HIPO bank wrapper for `RECHB::Particle` (group 300, item 11).
/// Reconstructed Particle Information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RECHB_Particle bank;
/// if (auto sc = bank.bind(event.get_bank("RECHB::Particle")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto pid = row.pid();
/// }
/// @endcode
class RECHB_Particle {
 public:
    static constexpr std::string_view bank_name = "RECHB::Particle";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 11;

    class Row;

    RECHB_Particle() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("pid"); h) h_pid = *h;
        else h_pid = {};
        if (auto h = schema.get_handle("px"); h) h_px = *h;
        else h_px = {};
        if (auto h = schema.get_handle("py"); h) h_py = *h;
        else h_py = {};
        if (auto h = schema.get_handle("pz"); h) h_pz = *h;
        else h_pz = {};
        if (auto h = schema.get_handle("vx"); h) h_vx = *h;
        else h_vx = {};
        if (auto h = schema.get_handle("vy"); h) h_vy = *h;
        else h_vy = {};
        if (auto h = schema.get_handle("vz"); h) h_vz = *h;
        else h_vz = {};
        if (auto h = schema.get_handle("vt"); h) h_vt = *h;
        else h_vt = {};
        if (auto h = schema.get_handle("charge"); h) h_charge = *h;
        else h_charge = {};
        if (auto h = schema.get_handle("beta"); h) h_beta = *h;
        else h_beta = {};
        if (auto h = schema.get_handle("chi2pid"); h) h_chi2pid = *h;
        else h_chi2pid = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
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
    /// pid: particle id in LUND conventions
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pid, row));
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
    /// vt: RF and z corrected vertex time (ns)
    [[nodiscard]] float vt(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vt, row));
    }
    /// charge: particle charge
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_charge, row));
    }
    /// beta: particle beta measured by TOF
    [[nodiscard]] float beta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_beta, row));
    }
    /// chi2pid: Chi2 of assigned PID
    [[nodiscard]] float chi2pid(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2pid, row));
    }
    /// status: particle status (represents detector collection it passed)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pid_col() const noexcept {
        return m_view.column<int32_t>(h_pid);
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
    /// vt column as a contiguous span.
    [[nodiscard]] std::span<const float> vt_col() const noexcept {
        return m_view.column<float>(h_vt);
    }
    /// charge column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> charge_col() const noexcept {
        return m_view.column<int8_t>(h_charge);
    }
    /// beta column as a contiguous span.
    [[nodiscard]] std::span<const float> beta_col() const noexcept {
        return m_view.column<float>(h_beta);
    }
    /// chi2pid column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2pid_col() const noexcept {
        return m_view.column<float>(h_chi2pid);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_pid{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_vx{};
    hipo::column_handle h_vy{};
    hipo::column_handle h_vz{};
    hipo::column_handle h_vt{};
    hipo::column_handle h_charge{};
    hipo::column_handle h_beta{};
    hipo::column_handle h_chi2pid{};
    hipo::column_handle h_status{};

    friend class Row;
};

/// Lightweight row view for RECHB::Particle. Stores a pointer + row index only.
class RECHB_Particle::Row {
 public:
    Row(const RECHB_Particle* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float vx() const noexcept { return m_bank->vx(m_row); }
    [[nodiscard]] float vy() const noexcept { return m_bank->vy(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float vt() const noexcept { return m_bank->vt(m_row); }
    [[nodiscard]] int charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float beta() const noexcept { return m_bank->beta(m_row); }
    [[nodiscard]] float chi2pid() const noexcept { return m_bank->chi2pid(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
 private:
    const RECHB_Particle* m_bank;
    int m_row;
};

inline RECHB_Particle::Row RECHB_Particle::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
