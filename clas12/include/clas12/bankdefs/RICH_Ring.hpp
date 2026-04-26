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


/// HIPO bank wrapper for `RICH::Ring` (group 21800, item 36).
/// Reconstructed Cherenov information for single photon
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RICH_Ring bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Ring")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Ring {
 public:
    static constexpr std::string_view bank_name = "RICH::Ring";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 36;

    class Row;

    RICH_Ring() = default;

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
            hipo::column_handle h_sector;
            hipo::column_handle h_pmt;
            hipo::column_handle h_anode;
            hipo::column_handle h_hypo;
            hipo::column_handle h_dtime;
            hipo::column_handle h_etaC;
            hipo::column_handle h_prob;
            hipo::column_handle h_use;
            hipo::column_handle h_dangle;
            hipo::column_handle h_layers;
            hipo::column_handle h_compos;
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
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("pmt"); h) cache.h_pmt = *h;
            else cache.h_pmt = {};
            if (auto h = schema.get_handle("anode"); h) cache.h_anode = *h;
            else cache.h_anode = {};
            if (auto h = schema.get_handle("hypo"); h) cache.h_hypo = *h;
            else cache.h_hypo = {};
            if (auto h = schema.get_handle("dtime"); h) cache.h_dtime = *h;
            else cache.h_dtime = {};
            if (auto h = schema.get_handle("etaC"); h) cache.h_etaC = *h;
            else cache.h_etaC = {};
            if (auto h = schema.get_handle("prob"); h) cache.h_prob = *h;
            else cache.h_prob = {};
            if (auto h = schema.get_handle("use"); h) cache.h_use = *h;
            else cache.h_use = {};
            if (auto h = schema.get_handle("dangle"); h) cache.h_dangle = *h;
            else cache.h_dangle = {};
            if (auto h = schema.get_handle("layers"); h) cache.h_layers = *h;
            else cache.h_layers = {};
            if (auto h = schema.get_handle("compos"); h) cache.h_compos = *h;
            else cache.h_compos = {};
        }
        h_id = cache.h_id;
        h_hindex = cache.h_hindex;
        h_pindex = cache.h_pindex;
        h_sector = cache.h_sector;
        h_pmt = cache.h_pmt;
        h_anode = cache.h_anode;
        h_hypo = cache.h_hypo;
        h_dtime = cache.h_dtime;
        h_etaC = cache.h_etaC;
        h_prob = cache.h_prob;
        h_use = cache.h_use;
        h_dangle = cache.h_dangle;
        h_layers = cache.h_layers;
        h_compos = cache.h_compos;
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
    /// sector: photon sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// pmt: photon hit pmt
    [[nodiscard]] int pmt(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pmt, row));
    }
    /// anode: photon hit anode
    [[nodiscard]] int anode(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_anode, row));
    }
    /// hypo: PID hypothesis for photon reconstruction
    [[nodiscard]] int hypo(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_hypo, row));
    }
    /// dtime: time mismatch for traced solution
    [[nodiscard]] float dtime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dtime, row));
    }
    /// etaC: Cherenkov angle as traced solution
    [[nodiscard]] float etaC(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_etaC, row));
    }
    /// prob: probability for the given hypothesis
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob, row));
    }
    /// use: flag for use in the likelihood
    [[nodiscard]] int use(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_use, row));
    }
    /// dangle: angular range subtended by the pixel
    [[nodiscard]] float dangle(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dangle, row));
    }
    /// layers: sequence of hit layers (reflections)
    [[nodiscard]] int layers(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_layers, row));
    }
    /// compos: sequence of hit components (reflections)
    [[nodiscard]] int compos(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_compos, row));
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
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// pmt column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pmt_col() const noexcept {
        return m_view.column<int16_t>(h_pmt);
    }
    /// anode column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> anode_col() const noexcept {
        return m_view.column<int8_t>(h_anode);
    }
    /// hypo column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> hypo_col() const noexcept {
        return m_view.column<int32_t>(h_hypo);
    }
    /// dtime column as a contiguous span.
    [[nodiscard]] std::span<const float> dtime_col() const noexcept {
        return m_view.column<float>(h_dtime);
    }
    /// etaC column as a contiguous span.
    [[nodiscard]] std::span<const float> etaC_col() const noexcept {
        return m_view.column<float>(h_etaC);
    }
    /// prob column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_col() const noexcept {
        return m_view.column<float>(h_prob);
    }
    /// use column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> use_col() const noexcept {
        return m_view.column<int8_t>(h_use);
    }
    /// dangle column as a contiguous span.
    [[nodiscard]] std::span<const float> dangle_col() const noexcept {
        return m_view.column<float>(h_dangle);
    }
    /// layers column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> layers_col() const noexcept {
        return m_view.column<int32_t>(h_layers);
    }
    /// compos column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> compos_col() const noexcept {
        return m_view.column<int32_t>(h_compos);
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
    hipo::column_handle h_sector{};
    hipo::column_handle h_pmt{};
    hipo::column_handle h_anode{};
    hipo::column_handle h_hypo{};
    hipo::column_handle h_dtime{};
    hipo::column_handle h_etaC{};
    hipo::column_handle h_prob{};
    hipo::column_handle h_use{};
    hipo::column_handle h_dangle{};
    hipo::column_handle h_layers{};
    hipo::column_handle h_compos{};

    friend class Row;
};

/// Lightweight row view for RICH::Ring. Stores a pointer + row index only.
class RICH_Ring::Row {
 public:
    Row(const RICH_Ring* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int pmt() const noexcept { return m_bank->pmt(m_row); }
    [[nodiscard]] int anode() const noexcept { return m_bank->anode(m_row); }
    [[nodiscard]] int hypo() const noexcept { return m_bank->hypo(m_row); }
    [[nodiscard]] float dtime() const noexcept { return m_bank->dtime(m_row); }
    [[nodiscard]] float etaC() const noexcept { return m_bank->etaC(m_row); }
    [[nodiscard]] float prob() const noexcept { return m_bank->prob(m_row); }
    [[nodiscard]] int use() const noexcept { return m_bank->use(m_row); }
    [[nodiscard]] float dangle() const noexcept { return m_bank->dangle(m_row); }
    [[nodiscard]] int layers() const noexcept { return m_bank->layers(m_row); }
    [[nodiscard]] int compos() const noexcept { return m_bank->compos(m_row); }
 private:
    const RICH_Ring* m_bank;
    int m_row;
};

inline RICH_Ring::Row RICH_Ring::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
