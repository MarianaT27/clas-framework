// =============================================================================
// Auto-generated from rtpc.json - do not edit by hand.
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


/// HIPO bank wrapper for `RTPC::KFtracks` (group 21900, item 23).
/// Kalman Fitter track info
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RTPC_KFtracks bank;
/// if (auto sc = bank.bind(event.get_bank("RTPC::KFtracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trkID = row.trkID();
/// }
/// @endcode
class RTPC_KFtracks {
 public:
    static constexpr std::string_view bank_name = "RTPC::KFtracks";
    static constexpr int bank_group = 21900;
    static constexpr int bank_item  = 23;

    class Row;

    RTPC_KFtracks() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("trkID"); h) h_trkID = *h;
        else h_trkID = {};
        if (auto h = schema.get_handle("px"); h) h_px = *h;
        else h_px = {};
        if (auto h = schema.get_handle("py"); h) h_py = *h;
        else h_py = {};
        if (auto h = schema.get_handle("pz"); h) h_pz = *h;
        else h_pz = {};
        if (auto h = schema.get_handle("vz"); h) h_vz = *h;
        else h_vz = {};
        if (auto h = schema.get_handle("dEdx"); h) h_dEdx = *h;
        else h_dEdx = {};
        if (auto h = schema.get_handle("p_drift"); h) h_p_drift = *h;
        else h_p_drift = {};
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
    /// trkID: Track ID of the hit
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trkID, row));
    }
    /// px: X component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: Y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: Z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// vz: Z component of the reconstructed vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }
    /// dEdx: dEdx for the Track (MeV/mm)
    [[nodiscard]] float dEdx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dEdx, row));
    }
    /// p_drift: Momentum in the drift region (GeV)
    [[nodiscard]] float p_drift(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p_drift, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trkID_col() const noexcept {
        return m_view.column<int32_t>(h_trkID);
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
    /// vz column as a contiguous span.
    [[nodiscard]] std::span<const float> vz_col() const noexcept {
        return m_view.column<float>(h_vz);
    }
    /// dEdx column as a contiguous span.
    [[nodiscard]] std::span<const float> dEdx_col() const noexcept {
        return m_view.column<float>(h_dEdx);
    }
    /// p_drift column as a contiguous span.
    [[nodiscard]] std::span<const float> p_drift_col() const noexcept {
        return m_view.column<float>(h_p_drift);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_trkID{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_vz{};
    hipo::column_handle h_dEdx{};
    hipo::column_handle h_p_drift{};

    friend class Row;
};

/// Lightweight row view for RTPC::KFtracks. Stores a pointer + row index only.
class RTPC_KFtracks::Row {
 public:
    Row(const RTPC_KFtracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
    [[nodiscard]] float dEdx() const noexcept { return m_bank->dEdx(m_row); }
    [[nodiscard]] float p_drift() const noexcept { return m_bank->p_drift(m_row); }
 private:
    const RTPC_KFtracks* m_bank;
    int m_row;
};

inline RTPC_KFtracks::Row RTPC_KFtracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
