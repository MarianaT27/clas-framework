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


/// HIPO bank wrapper for `RICH::Particle` (group 21800, item 37).
/// Reconstructed Cherenov information per track
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RICH_Particle bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Particle")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Particle {
 public:
    static constexpr std::string_view bank_name = "RICH::Particle";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 37;

    class Row;

    RICH_Particle() = default;

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
            hipo::column_handle h_id;
            hipo::column_handle h_hindex;
            hipo::column_handle h_pindex;
            hipo::column_handle h_emilay;
            hipo::column_handle h_emico;
            hipo::column_handle h_enico;
            hipo::column_handle h_emqua;
            hipo::column_handle h_mchi2;
            hipo::column_handle h_mass;
            hipo::column_handle h_best_PID;
            hipo::column_handle h_RQ;
            hipo::column_handle h_ReQ;
            hipo::column_handle h_el_logl;
            hipo::column_handle h_pi_logl;
            hipo::column_handle h_k_logl;
            hipo::column_handle h_pr_logl;
            hipo::column_handle h_best_ch;
            hipo::column_handle h_best_c2;
            hipo::column_handle h_best_RL;
            hipo::column_handle h_best_ntot;
            hipo::column_handle h_best_mass;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("hindex"); h) cache.h_hindex = *h;
            else cache.h_hindex = {};
            if (auto h = schema.get_handle("pindex"); h) cache.h_pindex = *h;
            else cache.h_pindex = {};
            if (auto h = schema.get_handle("emilay"); h) cache.h_emilay = *h;
            else cache.h_emilay = {};
            if (auto h = schema.get_handle("emico"); h) cache.h_emico = *h;
            else cache.h_emico = {};
            if (auto h = schema.get_handle("enico"); h) cache.h_enico = *h;
            else cache.h_enico = {};
            if (auto h = schema.get_handle("emqua"); h) cache.h_emqua = *h;
            else cache.h_emqua = {};
            if (auto h = schema.get_handle("mchi2"); h) cache.h_mchi2 = *h;
            else cache.h_mchi2 = {};
            if (auto h = schema.get_handle("mass"); h) cache.h_mass = *h;
            else cache.h_mass = {};
            if (auto h = schema.get_handle("best_PID"); h) cache.h_best_PID = *h;
            else cache.h_best_PID = {};
            if (auto h = schema.get_handle("RQ"); h) cache.h_RQ = *h;
            else cache.h_RQ = {};
            if (auto h = schema.get_handle("ReQ"); h) cache.h_ReQ = *h;
            else cache.h_ReQ = {};
            if (auto h = schema.get_handle("el_logl"); h) cache.h_el_logl = *h;
            else cache.h_el_logl = {};
            if (auto h = schema.get_handle("pi_logl"); h) cache.h_pi_logl = *h;
            else cache.h_pi_logl = {};
            if (auto h = schema.get_handle("k_logl"); h) cache.h_k_logl = *h;
            else cache.h_k_logl = {};
            if (auto h = schema.get_handle("pr_logl"); h) cache.h_pr_logl = *h;
            else cache.h_pr_logl = {};
            if (auto h = schema.get_handle("best_ch"); h) cache.h_best_ch = *h;
            else cache.h_best_ch = {};
            if (auto h = schema.get_handle("best_c2"); h) cache.h_best_c2 = *h;
            else cache.h_best_c2 = {};
            if (auto h = schema.get_handle("best_RL"); h) cache.h_best_RL = *h;
            else cache.h_best_RL = {};
            if (auto h = schema.get_handle("best_ntot"); h) cache.h_best_ntot = *h;
            else cache.h_best_ntot = {};
            if (auto h = schema.get_handle("best_mass"); h) cache.h_best_mass = *h;
            else cache.h_best_mass = {};
        }
        h_id = cache.h_id;
        h_hindex = cache.h_hindex;
        h_pindex = cache.h_pindex;
        h_emilay = cache.h_emilay;
        h_emico = cache.h_emico;
        h_enico = cache.h_enico;
        h_emqua = cache.h_emqua;
        h_mchi2 = cache.h_mchi2;
        h_mass = cache.h_mass;
        h_best_PID = cache.h_best_PID;
        h_RQ = cache.h_RQ;
        h_ReQ = cache.h_ReQ;
        h_el_logl = cache.h_el_logl;
        h_pi_logl = cache.h_pi_logl;
        h_k_logl = cache.h_k_logl;
        h_pr_logl = cache.h_pr_logl;
        h_best_ch = cache.h_best_ch;
        h_best_c2 = cache.h_best_c2;
        h_best_RL = cache.h_best_RL;
        h_best_ntot = cache.h_best_ntot;
        h_best_mass = cache.h_best_mass;
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
        return static_cast<int>(m_view.get<int8_t>(h_id, row));
    }
    /// hindex: related row in the RICH::clusters bank (if any)
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_hindex, row));
    }
    /// pindex: related row in the REC::Particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pindex, row));
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
    /// mchi2: track-cluster matching chi2 (if any)
    [[nodiscard]] float mchi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_mchi2, row));
    }
    /// mass: reconstructed mass for best hypo
    [[nodiscard]] float mass(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_mass, row));
    }
    /// best_PID: most probable PID choice
    [[nodiscard]] int best_PID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_best_PID, row));
    }
    /// RQ: goodness of hadron choice parameter (1=anambiguos, 0=random)
    [[nodiscard]] float RQ(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_RQ, row));
    }
    /// ReQ: goodness of elecgtron choice parameter (1=anambiguos, 0=random)
    [[nodiscard]] float ReQ(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ReQ, row));
    }
    /// el_logl: probability to be an electron
    [[nodiscard]] float el_logl(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_el_logl, row));
    }
    /// pi_logl: probability to be an pion
    [[nodiscard]] float pi_logl(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pi_logl, row));
    }
    /// k_logl: probability to be an kaon
    [[nodiscard]] float k_logl(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_k_logl, row));
    }
    /// pr_logl: probability to be an proton
    [[nodiscard]] float pr_logl(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pr_logl, row));
    }
    /// best_ch: Average etaC for best hypothesis
    [[nodiscard]] float best_ch(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_best_ch, row));
    }
    /// best_c2: chi2 for best hypothesis
    [[nodiscard]] float best_c2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_best_c2, row));
    }
    /// best_RL: Likelihood ratio for best hypothesis
    [[nodiscard]] float best_RL(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_best_RL, row));
    }
    /// best_ntot: Number of photon used for likelihood
    [[nodiscard]] float best_ntot(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_best_ntot, row));
    }
    /// best_mass: Reconstructed mass for best hypothesis
    [[nodiscard]] float best_mass(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_best_mass, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> id_col() const noexcept {
        return m_view.column<int8_t>(h_id);
    }
    /// hindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> hindex_col() const noexcept {
        return m_view.column<int16_t>(h_hindex);
    }
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> pindex_col() const noexcept {
        return m_view.column<int8_t>(h_pindex);
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
    /// mchi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> mchi2_col() const noexcept {
        return m_view.column<float>(h_mchi2);
    }
    /// mass column as a contiguous span.
    [[nodiscard]] std::span<const float> mass_col() const noexcept {
        return m_view.column<float>(h_mass);
    }
    /// best_PID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> best_PID_col() const noexcept {
        return m_view.column<int16_t>(h_best_PID);
    }
    /// RQ column as a contiguous span.
    [[nodiscard]] std::span<const float> RQ_col() const noexcept {
        return m_view.column<float>(h_RQ);
    }
    /// ReQ column as a contiguous span.
    [[nodiscard]] std::span<const float> ReQ_col() const noexcept {
        return m_view.column<float>(h_ReQ);
    }
    /// el_logl column as a contiguous span.
    [[nodiscard]] std::span<const float> el_logl_col() const noexcept {
        return m_view.column<float>(h_el_logl);
    }
    /// pi_logl column as a contiguous span.
    [[nodiscard]] std::span<const float> pi_logl_col() const noexcept {
        return m_view.column<float>(h_pi_logl);
    }
    /// k_logl column as a contiguous span.
    [[nodiscard]] std::span<const float> k_logl_col() const noexcept {
        return m_view.column<float>(h_k_logl);
    }
    /// pr_logl column as a contiguous span.
    [[nodiscard]] std::span<const float> pr_logl_col() const noexcept {
        return m_view.column<float>(h_pr_logl);
    }
    /// best_ch column as a contiguous span.
    [[nodiscard]] std::span<const float> best_ch_col() const noexcept {
        return m_view.column<float>(h_best_ch);
    }
    /// best_c2 column as a contiguous span.
    [[nodiscard]] std::span<const float> best_c2_col() const noexcept {
        return m_view.column<float>(h_best_c2);
    }
    /// best_RL column as a contiguous span.
    [[nodiscard]] std::span<const float> best_RL_col() const noexcept {
        return m_view.column<float>(h_best_RL);
    }
    /// best_ntot column as a contiguous span.
    [[nodiscard]] std::span<const float> best_ntot_col() const noexcept {
        return m_view.column<float>(h_best_ntot);
    }
    /// best_mass column as a contiguous span.
    [[nodiscard]] std::span<const float> best_mass_col() const noexcept {
        return m_view.column<float>(h_best_mass);
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
    hipo::column_handle h_emilay{};
    hipo::column_handle h_emico{};
    hipo::column_handle h_enico{};
    hipo::column_handle h_emqua{};
    hipo::column_handle h_mchi2{};
    hipo::column_handle h_mass{};
    hipo::column_handle h_best_PID{};
    hipo::column_handle h_RQ{};
    hipo::column_handle h_ReQ{};
    hipo::column_handle h_el_logl{};
    hipo::column_handle h_pi_logl{};
    hipo::column_handle h_k_logl{};
    hipo::column_handle h_pr_logl{};
    hipo::column_handle h_best_ch{};
    hipo::column_handle h_best_c2{};
    hipo::column_handle h_best_RL{};
    hipo::column_handle h_best_ntot{};
    hipo::column_handle h_best_mass{};

    friend class Row;
};

/// Lightweight row view for RICH::Particle. Stores a pointer + row index only.
class RICH_Particle::Row {
 public:
    Row(const RICH_Particle* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int emilay() const noexcept { return m_bank->emilay(m_row); }
    [[nodiscard]] int emico() const noexcept { return m_bank->emico(m_row); }
    [[nodiscard]] int enico() const noexcept { return m_bank->enico(m_row); }
    [[nodiscard]] int emqua() const noexcept { return m_bank->emqua(m_row); }
    [[nodiscard]] float mchi2() const noexcept { return m_bank->mchi2(m_row); }
    [[nodiscard]] float mass() const noexcept { return m_bank->mass(m_row); }
    [[nodiscard]] int best_PID() const noexcept { return m_bank->best_PID(m_row); }
    [[nodiscard]] float RQ() const noexcept { return m_bank->RQ(m_row); }
    [[nodiscard]] float ReQ() const noexcept { return m_bank->ReQ(m_row); }
    [[nodiscard]] float el_logl() const noexcept { return m_bank->el_logl(m_row); }
    [[nodiscard]] float pi_logl() const noexcept { return m_bank->pi_logl(m_row); }
    [[nodiscard]] float k_logl() const noexcept { return m_bank->k_logl(m_row); }
    [[nodiscard]] float pr_logl() const noexcept { return m_bank->pr_logl(m_row); }
    [[nodiscard]] float best_ch() const noexcept { return m_bank->best_ch(m_row); }
    [[nodiscard]] float best_c2() const noexcept { return m_bank->best_c2(m_row); }
    [[nodiscard]] float best_RL() const noexcept { return m_bank->best_RL(m_row); }
    [[nodiscard]] float best_ntot() const noexcept { return m_bank->best_ntot(m_row); }
    [[nodiscard]] float best_mass() const noexcept { return m_bank->best_mass(m_row); }
 private:
    const RICH_Particle* m_bank;
    int m_row;
};

inline RICH_Particle::Row RICH_Particle::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
