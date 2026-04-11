// =============================================================================
// Auto-generated from rich.json - do not edit by hand.
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


/// HIPO bank wrapper for `RICH::Photon` (group 21800, item 39).
/// Reconstructed Photons in RICH
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("hindex"); h) h_hindex = *h;
        else h_hindex = {};
        if (auto h = schema.get_handle("pindex"); h) h_pindex = *h;
        else h_pindex = {};
        if (auto h = schema.get_handle("type"); h) h_type = *h;
        else h_type = {};
        if (auto h = schema.get_handle("used"); h) h_used = *h;
        else h_used = {};
        if (auto h = schema.get_handle("start_time"); h) h_start_time = *h;
        else h_start_time = {};
        if (auto h = schema.get_handle("traced_the"); h) h_traced_the = *h;
        else h_traced_the = {};
        if (auto h = schema.get_handle("traced_phi"); h) h_traced_phi = *h;
        else h_traced_phi = {};
        if (auto h = schema.get_handle("traced_hitx"); h) h_traced_hitx = *h;
        else h_traced_hitx = {};
        if (auto h = schema.get_handle("traced_hity"); h) h_traced_hity = *h;
        else h_traced_hity = {};
        if (auto h = schema.get_handle("traced_hitz"); h) h_traced_hitz = *h;
        else h_traced_hitz = {};
        if (auto h = schema.get_handle("traced_path"); h) h_traced_path = *h;
        else h_traced_path = {};
        if (auto h = schema.get_handle("traced_time"); h) h_traced_time = *h;
        else h_traced_time = {};
        if (auto h = schema.get_handle("traced_stime"); h) h_traced_stime = *h;
        else h_traced_stime = {};
        if (auto h = schema.get_handle("traced_nrfl"); h) h_traced_nrfl = *h;
        else h_traced_nrfl = {};
        if (auto h = schema.get_handle("traced_nrfr"); h) h_traced_nrfr = *h;
        else h_traced_nrfr = {};
        if (auto h = schema.get_handle("traced_1rfl"); h) h_traced_1rfl = *h;
        else h_traced_1rfl = {};
        if (auto h = schema.get_handle("traced_layers"); h) h_traced_layers = *h;
        else h_traced_layers = {};
        if (auto h = schema.get_handle("traced_compos"); h) h_traced_compos = *h;
        else h_traced_compos = {};
        if (auto h = schema.get_handle("traced_etaC"); h) h_traced_etaC = *h;
        else h_traced_etaC = {};
        if (auto h = schema.get_handle("traced_aeron"); h) h_traced_aeron = *h;
        else h_traced_aeron = {};
        if (auto h = schema.get_handle("traced_dthe"); h) h_traced_dthe = *h;
        else h_traced_dthe = {};
        if (auto h = schema.get_handle("traced_dphi"); h) h_traced_dphi = *h;
        else h_traced_dphi = {};
        if (auto h = schema.get_handle("eff"); h) h_eff = *h;
        else h_eff = {};
        if (auto h = schema.get_handle("back"); h) h_back = *h;
        else h_back = {};
        if (auto h = schema.get_handle("hypo_pid"); h) h_hypo_pid = *h;
        else h_hypo_pid = {};
        if (auto h = schema.get_handle("etac_ref"); h) h_etac_ref = *h;
        else h_etac_ref = {};
        if (auto h = schema.get_handle("etac_rms"); h) h_etac_rms = *h;
        else h_etac_rms = {};
        if (auto h = schema.get_handle("prob"); h) h_prob = *h;
        else h_prob = {};
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
    /// id: id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// hindex: related row in the RICH::hits bank
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_hindex, row));
    }
    /// pindex: related row in the REC::Particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pindex, row));
    }
    /// type: hypo (0=ele, 1=pion, 2=kaon, 3=proto) + reco (0=real hit, 10=trial)
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_type, row));
    }
    /// used: eligible for PID in time (1=yes, 0=no) angle (10=yes, 0=no) hypo (100=by other)
    [[nodiscard]] int used(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_used, row));
    }
    /// start_time: time at photon emission point
    [[nodiscard]] float start_time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_start_time, row));
    }
    /// traced_the: Lab theta angle for traced solution
    [[nodiscard]] float traced_the(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_the, row));
    }
    /// traced_phi: Lab phi angle for traced solution
    [[nodiscard]] float traced_phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_phi, row));
    }
    /// traced_hitx: Lab hitx angle for traced solution
    [[nodiscard]] float traced_hitx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_hitx, row));
    }
    /// traced_hity: Lab hity angle for traced solution
    [[nodiscard]] float traced_hity(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_hity, row));
    }
    /// traced_hitz: Lab hitz angle for traced solution
    [[nodiscard]] float traced_hitz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_hitz, row));
    }
    /// traced_path: path for traced solution
    [[nodiscard]] float traced_path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_path, row));
    }
    /// traced_time: time for traced solution
    [[nodiscard]] float traced_time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_time, row));
    }
    /// traced_stime: time rms for traced solution
    [[nodiscard]] float traced_stime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_stime, row));
    }
    /// traced_nrfl: n reflections for traced solution
    [[nodiscard]] int traced_nrfl(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_traced_nrfl, row));
    }
    /// traced_nrfr: n refractions for traced solution
    [[nodiscard]] int traced_nrfr(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_traced_nrfr, row));
    }
    /// traced_1rfl: first reflection type for traced solution
    [[nodiscard]] int traced_1rfl(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_traced_1rfl, row));
    }
    /// traced_layers: sequence of hit layers (reflections)
    [[nodiscard]] int traced_layers(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_traced_layers, row));
    }
    /// traced_compos: sequence of hit components (reflections)
    [[nodiscard]] int traced_compos(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_traced_compos, row));
    }
    /// traced_etaC: etaC angle for traced solution
    [[nodiscard]] float traced_etaC(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_etaC, row));
    }
    /// traced_aeron: n for traced solution
    [[nodiscard]] float traced_aeron(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_aeron, row));
    }
    /// traced_dthe: pixel equivalent dthe for traced solution
    [[nodiscard]] float traced_dthe(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_dthe, row));
    }
    /// traced_dphi: pixel equivalent dphi for traced solution
    [[nodiscard]] float traced_dphi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_dphi, row));
    }
    /// eff: photon detection efficiency for likelihood
    [[nodiscard]] float eff(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_eff, row));
    }
    /// back: background level for likelihood
    [[nodiscard]] float back(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_back, row));
    }
    /// hypo_pid: PID hypothesis for photon reconstruction
    [[nodiscard]] int hypo_pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_hypo_pid, row));
    }
    /// etac_ref: Cherenkov angle expected value
    [[nodiscard]] float etac_ref(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_etac_ref, row));
    }
    /// etac_rms: Cherenkov angle expected resolution
    [[nodiscard]] float etac_rms(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_etac_rms, row));
    }
    /// prob: probability for traced solution with given hypo
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// hindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> hindex_col() const noexcept {
        return m_view.column<int16_t>(h_hindex);
    }
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> pindex_col() const noexcept {
        return m_view.column<int8_t>(h_pindex);
    }
    /// type column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> type_col() const noexcept {
        return m_view.column<int16_t>(h_type);
    }
    /// used column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> used_col() const noexcept {
        return m_view.column<int8_t>(h_used);
    }
    /// start_time column as a contiguous span.
    [[nodiscard]] std::span<const float> start_time_col() const noexcept {
        return m_view.column<float>(h_start_time);
    }
    /// traced_the column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_the_col() const noexcept {
        return m_view.column<float>(h_traced_the);
    }
    /// traced_phi column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_phi_col() const noexcept {
        return m_view.column<float>(h_traced_phi);
    }
    /// traced_hitx column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_hitx_col() const noexcept {
        return m_view.column<float>(h_traced_hitx);
    }
    /// traced_hity column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_hity_col() const noexcept {
        return m_view.column<float>(h_traced_hity);
    }
    /// traced_hitz column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_hitz_col() const noexcept {
        return m_view.column<float>(h_traced_hitz);
    }
    /// traced_path column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_path_col() const noexcept {
        return m_view.column<float>(h_traced_path);
    }
    /// traced_time column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_time_col() const noexcept {
        return m_view.column<float>(h_traced_time);
    }
    /// traced_stime column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_stime_col() const noexcept {
        return m_view.column<float>(h_traced_stime);
    }
    /// traced_nrfl column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> traced_nrfl_col() const noexcept {
        return m_view.column<int16_t>(h_traced_nrfl);
    }
    /// traced_nrfr column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> traced_nrfr_col() const noexcept {
        return m_view.column<int16_t>(h_traced_nrfr);
    }
    /// traced_1rfl column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> traced_1rfl_col() const noexcept {
        return m_view.column<int16_t>(h_traced_1rfl);
    }
    /// traced_layers column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> traced_layers_col() const noexcept {
        return m_view.column<int32_t>(h_traced_layers);
    }
    /// traced_compos column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> traced_compos_col() const noexcept {
        return m_view.column<int32_t>(h_traced_compos);
    }
    /// traced_etaC column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_etaC_col() const noexcept {
        return m_view.column<float>(h_traced_etaC);
    }
    /// traced_aeron column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_aeron_col() const noexcept {
        return m_view.column<float>(h_traced_aeron);
    }
    /// traced_dthe column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_dthe_col() const noexcept {
        return m_view.column<float>(h_traced_dthe);
    }
    /// traced_dphi column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_dphi_col() const noexcept {
        return m_view.column<float>(h_traced_dphi);
    }
    /// eff column as a contiguous span.
    [[nodiscard]] std::span<const float> eff_col() const noexcept {
        return m_view.column<float>(h_eff);
    }
    /// back column as a contiguous span.
    [[nodiscard]] std::span<const float> back_col() const noexcept {
        return m_view.column<float>(h_back);
    }
    /// hypo_pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> hypo_pid_col() const noexcept {
        return m_view.column<int32_t>(h_hypo_pid);
    }
    /// etac_ref column as a contiguous span.
    [[nodiscard]] std::span<const float> etac_ref_col() const noexcept {
        return m_view.column<float>(h_etac_ref);
    }
    /// etac_rms column as a contiguous span.
    [[nodiscard]] std::span<const float> etac_rms_col() const noexcept {
        return m_view.column<float>(h_etac_rms);
    }
    /// prob column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_col() const noexcept {
        return m_view.column<float>(h_prob);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle hindex_handle() const noexcept { return h_hindex; }
    [[nodiscard]] hipo::column_handle pindex_handle() const noexcept { return h_pindex; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_hindex{};
    hipo::column_handle h_pindex{};
    hipo::column_handle h_type{};
    hipo::column_handle h_used{};
    hipo::column_handle h_start_time{};
    hipo::column_handle h_traced_the{};
    hipo::column_handle h_traced_phi{};
    hipo::column_handle h_traced_hitx{};
    hipo::column_handle h_traced_hity{};
    hipo::column_handle h_traced_hitz{};
    hipo::column_handle h_traced_path{};
    hipo::column_handle h_traced_time{};
    hipo::column_handle h_traced_stime{};
    hipo::column_handle h_traced_nrfl{};
    hipo::column_handle h_traced_nrfr{};
    hipo::column_handle h_traced_1rfl{};
    hipo::column_handle h_traced_layers{};
    hipo::column_handle h_traced_compos{};
    hipo::column_handle h_traced_etaC{};
    hipo::column_handle h_traced_aeron{};
    hipo::column_handle h_traced_dthe{};
    hipo::column_handle h_traced_dphi{};
    hipo::column_handle h_eff{};
    hipo::column_handle h_back{};
    hipo::column_handle h_hypo_pid{};
    hipo::column_handle h_etac_ref{};
    hipo::column_handle h_etac_rms{};
    hipo::column_handle h_prob{};

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

}  // namespace clas12::bankdefs
