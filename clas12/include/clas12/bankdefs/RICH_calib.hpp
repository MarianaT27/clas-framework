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


/// HIPO bank wrapper for `RICH::calib` (group 21800, item 51).
/// Reconstructed Photons in RICH selected for detector calibration
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    class Row;

    RICH_calib() = default;

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
            hipo::column_handle h_hindex;
            hipo::column_handle h_pindex;
            hipo::column_handle h_sector;
            hipo::column_handle h_pmt;
            hipo::column_handle h_anode;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_time;
            hipo::column_handle h_rawtime;
            hipo::column_handle h_duration;
            hipo::column_handle h_status;
            hipo::column_handle h_used;
            hipo::column_handle h_emilay;
            hipo::column_handle h_emico;
            hipo::column_handle h_enico;
            hipo::column_handle h_emqua;
            hipo::column_handle h_start_time;
            hipo::column_handle h_traced_the;
            hipo::column_handle h_traced_phi;
            hipo::column_handle h_traced_hitx;
            hipo::column_handle h_traced_hity;
            hipo::column_handle h_traced_hitz;
            hipo::column_handle h_traced_path;
            hipo::column_handle h_traced_time;
            hipo::column_handle h_traced_nrfl;
            hipo::column_handle h_traced_nrfr;
            hipo::column_handle h_traced_1rfl;
            hipo::column_handle h_traced_layers;
            hipo::column_handle h_traced_compos;
            hipo::column_handle h_traced_etaC;
            hipo::column_handle h_etac_ref;
            hipo::column_handle h_etac_rms;
            hipo::column_handle h_prob;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("hindex"); h) cache.h_hindex = *h;
            else cache.h_hindex = {};
            if (auto h = schema.get_handle("pindex"); h) cache.h_pindex = *h;
            else cache.h_pindex = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("pmt"); h) cache.h_pmt = *h;
            else cache.h_pmt = {};
            if (auto h = schema.get_handle("anode"); h) cache.h_anode = *h;
            else cache.h_anode = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("rawtime"); h) cache.h_rawtime = *h;
            else cache.h_rawtime = {};
            if (auto h = schema.get_handle("duration"); h) cache.h_duration = *h;
            else cache.h_duration = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("used"); h) cache.h_used = *h;
            else cache.h_used = {};
            if (auto h = schema.get_handle("emilay"); h) cache.h_emilay = *h;
            else cache.h_emilay = {};
            if (auto h = schema.get_handle("emico"); h) cache.h_emico = *h;
            else cache.h_emico = {};
            if (auto h = schema.get_handle("enico"); h) cache.h_enico = *h;
            else cache.h_enico = {};
            if (auto h = schema.get_handle("emqua"); h) cache.h_emqua = *h;
            else cache.h_emqua = {};
            if (auto h = schema.get_handle("start_time"); h) cache.h_start_time = *h;
            else cache.h_start_time = {};
            if (auto h = schema.get_handle("traced_the"); h) cache.h_traced_the = *h;
            else cache.h_traced_the = {};
            if (auto h = schema.get_handle("traced_phi"); h) cache.h_traced_phi = *h;
            else cache.h_traced_phi = {};
            if (auto h = schema.get_handle("traced_hitx"); h) cache.h_traced_hitx = *h;
            else cache.h_traced_hitx = {};
            if (auto h = schema.get_handle("traced_hity"); h) cache.h_traced_hity = *h;
            else cache.h_traced_hity = {};
            if (auto h = schema.get_handle("traced_hitz"); h) cache.h_traced_hitz = *h;
            else cache.h_traced_hitz = {};
            if (auto h = schema.get_handle("traced_path"); h) cache.h_traced_path = *h;
            else cache.h_traced_path = {};
            if (auto h = schema.get_handle("traced_time"); h) cache.h_traced_time = *h;
            else cache.h_traced_time = {};
            if (auto h = schema.get_handle("traced_nrfl"); h) cache.h_traced_nrfl = *h;
            else cache.h_traced_nrfl = {};
            if (auto h = schema.get_handle("traced_nrfr"); h) cache.h_traced_nrfr = *h;
            else cache.h_traced_nrfr = {};
            if (auto h = schema.get_handle("traced_1rfl"); h) cache.h_traced_1rfl = *h;
            else cache.h_traced_1rfl = {};
            if (auto h = schema.get_handle("traced_layers"); h) cache.h_traced_layers = *h;
            else cache.h_traced_layers = {};
            if (auto h = schema.get_handle("traced_compos"); h) cache.h_traced_compos = *h;
            else cache.h_traced_compos = {};
            if (auto h = schema.get_handle("traced_etaC"); h) cache.h_traced_etaC = *h;
            else cache.h_traced_etaC = {};
            if (auto h = schema.get_handle("etac_ref"); h) cache.h_etac_ref = *h;
            else cache.h_etac_ref = {};
            if (auto h = schema.get_handle("etac_rms"); h) cache.h_etac_rms = *h;
            else cache.h_etac_rms = {};
            if (auto h = schema.get_handle("prob"); h) cache.h_prob = *h;
            else cache.h_prob = {};
        }
        h_hindex = cache.h_hindex;
        h_pindex = cache.h_pindex;
        h_sector = cache.h_sector;
        h_pmt = cache.h_pmt;
        h_anode = cache.h_anode;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_time = cache.h_time;
        h_rawtime = cache.h_rawtime;
        h_duration = cache.h_duration;
        h_status = cache.h_status;
        h_used = cache.h_used;
        h_emilay = cache.h_emilay;
        h_emico = cache.h_emico;
        h_enico = cache.h_enico;
        h_emqua = cache.h_emqua;
        h_start_time = cache.h_start_time;
        h_traced_the = cache.h_traced_the;
        h_traced_phi = cache.h_traced_phi;
        h_traced_hitx = cache.h_traced_hitx;
        h_traced_hity = cache.h_traced_hity;
        h_traced_hitz = cache.h_traced_hitz;
        h_traced_path = cache.h_traced_path;
        h_traced_time = cache.h_traced_time;
        h_traced_nrfl = cache.h_traced_nrfl;
        h_traced_nrfr = cache.h_traced_nrfr;
        h_traced_1rfl = cache.h_traced_1rfl;
        h_traced_layers = cache.h_traced_layers;
        h_traced_compos = cache.h_traced_compos;
        h_traced_etaC = cache.h_traced_etaC;
        h_etac_ref = cache.h_etac_ref;
        h_etac_rms = cache.h_etac_rms;
        h_prob = cache.h_prob;
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
    /// hindex: related row in the RICH::hits bank
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_hindex, row));
    }
    /// pindex: related row in the REC::Particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pindex, row));
    }
    /// sector: Hit sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// pmt: Hit pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pmt, row));
    }
    /// anode: Hit anode
    [[nodiscard]] int anode(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_anode, row));
    }
    /// x: Hit x
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Hit y
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Hit z
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// time: Hit time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// rawtime: Hit rawtime
    [[nodiscard]] float rawtime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_rawtime, row));
    }
    /// duration: Hit duration
    [[nodiscard]] int duration(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_duration, row));
    }
    /// status: Hit status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// used: eligible for PID in time (1=yes, 0=no) angle (10=yes, 0=no) hypo (100=by other) or cluster (2)
    [[nodiscard]] int used(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_used, row));
    }
    /// emilay: aerogel layer of photon emission
    [[nodiscard]] int emilay(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_emilay, row));
    }
    /// emico: aerogel component of photon emission
    [[nodiscard]] int emico(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_emico, row));
    }
    /// enico: aerogel component of photon entrance point
    [[nodiscard]] int enico(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_enico, row));
    }
    /// emqua: aerogel quadrant of photon emission
    [[nodiscard]] int emqua(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_emqua, row));
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
    /// hindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> hindex_col() const noexcept {
        return m_view.column<int16_t>(h_hindex);
    }
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> pindex_col() const noexcept {
        return m_view.column<int8_t>(h_pindex);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// pmt column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pmt_col() const noexcept {
        return m_view.column<int16_t>(h_pmt);
    }
    /// anode column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> anode_col() const noexcept {
        return m_view.column<int16_t>(h_anode);
    }
    /// x column as a contiguous span.
    [[nodiscard]] std::span<const float> x_col() const noexcept {
        return m_view.column<float>(h_x);
    }
    /// y column as a contiguous span.
    [[nodiscard]] std::span<const float> y_col() const noexcept {
        return m_view.column<float>(h_y);
    }
    /// z column as a contiguous span.
    [[nodiscard]] std::span<const float> z_col() const noexcept {
        return m_view.column<float>(h_z);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// rawtime column as a contiguous span.
    [[nodiscard]] std::span<const float> rawtime_col() const noexcept {
        return m_view.column<float>(h_rawtime);
    }
    /// duration column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> duration_col() const noexcept {
        return m_view.column<int16_t>(h_duration);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// used column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> used_col() const noexcept {
        return m_view.column<int8_t>(h_used);
    }
    /// emilay column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> emilay_col() const noexcept {
        return m_view.column<int8_t>(h_emilay);
    }
    /// emico column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> emico_col() const noexcept {
        return m_view.column<int8_t>(h_emico);
    }
    /// enico column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> enico_col() const noexcept {
        return m_view.column<int8_t>(h_enico);
    }
    /// emqua column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> emqua_col() const noexcept {
        return m_view.column<int16_t>(h_emqua);
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
    hipo::column_handle h_hindex{};
    hipo::column_handle h_pindex{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_pmt{};
    hipo::column_handle h_anode{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_time{};
    hipo::column_handle h_rawtime{};
    hipo::column_handle h_duration{};
    hipo::column_handle h_status{};
    hipo::column_handle h_used{};
    hipo::column_handle h_emilay{};
    hipo::column_handle h_emico{};
    hipo::column_handle h_enico{};
    hipo::column_handle h_emqua{};
    hipo::column_handle h_start_time{};
    hipo::column_handle h_traced_the{};
    hipo::column_handle h_traced_phi{};
    hipo::column_handle h_traced_hitx{};
    hipo::column_handle h_traced_hity{};
    hipo::column_handle h_traced_hitz{};
    hipo::column_handle h_traced_path{};
    hipo::column_handle h_traced_time{};
    hipo::column_handle h_traced_nrfl{};
    hipo::column_handle h_traced_nrfr{};
    hipo::column_handle h_traced_1rfl{};
    hipo::column_handle h_traced_layers{};
    hipo::column_handle h_traced_compos{};
    hipo::column_handle h_traced_etaC{};
    hipo::column_handle h_etac_ref{};
    hipo::column_handle h_etac_rms{};
    hipo::column_handle h_prob{};

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

}  // namespace clas12::bankdefs
