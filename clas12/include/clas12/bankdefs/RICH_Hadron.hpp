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


/// HIPO bank wrapper for `RICH::Hadron` (group 21800, item 38).
/// Reconstructed Hadrons in RICH
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RICH_Hadron bank;
/// if (auto sc = bank.bind(event.get_bank("RICH::Hadron")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class RICH_Hadron {
 public:
    static constexpr std::string_view bank_name = "RICH::Hadron";
    static constexpr int bank_group = 21800;
    static constexpr int bank_item  = 38;

    class Row;

    RICH_Hadron() = default;

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
            hipo::column_handle h_traced_the;
            hipo::column_handle h_traced_phi;
            hipo::column_handle h_traced_hitx;
            hipo::column_handle h_traced_hity;
            hipo::column_handle h_traced_hitz;
            hipo::column_handle h_traced_time;
            hipo::column_handle h_traced_path;
            hipo::column_handle h_traced_mchi2;
            hipo::column_handle h_traced_ilay;
            hipo::column_handle h_traced_ico;
            hipo::column_handle h_traced_emix;
            hipo::column_handle h_traced_emiy;
            hipo::column_handle h_traced_emiz;
            hipo::column_handle h_etaC_dir;
            hipo::column_handle h_etaC_lat;
            hipo::column_handle h_etaC_sphe;
            hipo::column_handle h_etaC_rms;
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
            if (auto h = schema.get_handle("traced_time"); h) cache.h_traced_time = *h;
            else cache.h_traced_time = {};
            if (auto h = schema.get_handle("traced_path"); h) cache.h_traced_path = *h;
            else cache.h_traced_path = {};
            if (auto h = schema.get_handle("traced_mchi2"); h) cache.h_traced_mchi2 = *h;
            else cache.h_traced_mchi2 = {};
            if (auto h = schema.get_handle("traced_ilay"); h) cache.h_traced_ilay = *h;
            else cache.h_traced_ilay = {};
            if (auto h = schema.get_handle("traced_ico"); h) cache.h_traced_ico = *h;
            else cache.h_traced_ico = {};
            if (auto h = schema.get_handle("traced_emix"); h) cache.h_traced_emix = *h;
            else cache.h_traced_emix = {};
            if (auto h = schema.get_handle("traced_emiy"); h) cache.h_traced_emiy = *h;
            else cache.h_traced_emiy = {};
            if (auto h = schema.get_handle("traced_emiz"); h) cache.h_traced_emiz = *h;
            else cache.h_traced_emiz = {};
            if (auto h = schema.get_handle("etaC_dir"); h) cache.h_etaC_dir = *h;
            else cache.h_etaC_dir = {};
            if (auto h = schema.get_handle("etaC_lat"); h) cache.h_etaC_lat = *h;
            else cache.h_etaC_lat = {};
            if (auto h = schema.get_handle("etaC_sphe"); h) cache.h_etaC_sphe = *h;
            else cache.h_etaC_sphe = {};
            if (auto h = schema.get_handle("etaC_rms"); h) cache.h_etaC_rms = *h;
            else cache.h_etaC_rms = {};
        }
        h_id = cache.h_id;
        h_hindex = cache.h_hindex;
        h_pindex = cache.h_pindex;
        h_sector = cache.h_sector;
        h_traced_the = cache.h_traced_the;
        h_traced_phi = cache.h_traced_phi;
        h_traced_hitx = cache.h_traced_hitx;
        h_traced_hity = cache.h_traced_hity;
        h_traced_hitz = cache.h_traced_hitz;
        h_traced_time = cache.h_traced_time;
        h_traced_path = cache.h_traced_path;
        h_traced_mchi2 = cache.h_traced_mchi2;
        h_traced_ilay = cache.h_traced_ilay;
        h_traced_ico = cache.h_traced_ico;
        h_traced_emix = cache.h_traced_emix;
        h_traced_emiy = cache.h_traced_emiy;
        h_traced_emiz = cache.h_traced_emiz;
        h_etaC_dir = cache.h_etaC_dir;
        h_etaC_lat = cache.h_etaC_lat;
        h_etaC_sphe = cache.h_etaC_sphe;
        h_etaC_rms = cache.h_etaC_rms;
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
    /// id: hadron id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// hindex: row in the RICH clusters bank
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_hindex, row));
    }
    /// pindex: row in the REC  particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_pindex, row));
    }
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
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
    /// traced_time: time for traced solution
    [[nodiscard]] float traced_time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_time, row));
    }
    /// traced_path: path for traced solution
    [[nodiscard]] float traced_path(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_path, row));
    }
    /// traced_mchi2: mchi2 for traced solution
    [[nodiscard]] float traced_mchi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_mchi2, row));
    }
    /// traced_ilay: emission aero layer for traced solution
    [[nodiscard]] int traced_ilay(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_traced_ilay, row));
    }
    /// traced_ico: emission aero component for traced solution
    [[nodiscard]] int traced_ico(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_traced_ico, row));
    }
    /// traced_emix: emission point x for traced solution
    [[nodiscard]] float traced_emix(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_emix, row));
    }
    /// traced_emiy: emission point y for traced solution
    [[nodiscard]] float traced_emiy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_emiy, row));
    }
    /// traced_emiz: emission point z for traced solution
    [[nodiscard]] float traced_emiz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_traced_emiz, row));
    }
    /// etaC_dir: expected etaC for electron (direct photons)
    [[nodiscard]] float etaC_dir(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_etaC_dir, row));
    }
    /// etaC_lat: expected etaC for electron (after lateral reflection)
    [[nodiscard]] float etaC_lat(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_etaC_lat, row));
    }
    /// etaC_sphe: expected etaC for electron (after spherical reflection)
    [[nodiscard]] float etaC_sphe(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_etaC_sphe, row));
    }
    /// etaC_rms: expected etaC resolution
    [[nodiscard]] float etaC_rms(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_etaC_rms, row));
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
    /// traced_time column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_time_col() const noexcept {
        return m_view.column<float>(h_traced_time);
    }
    /// traced_path column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_path_col() const noexcept {
        return m_view.column<float>(h_traced_path);
    }
    /// traced_mchi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_mchi2_col() const noexcept {
        return m_view.column<float>(h_traced_mchi2);
    }
    /// traced_ilay column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> traced_ilay_col() const noexcept {
        return m_view.column<int16_t>(h_traced_ilay);
    }
    /// traced_ico column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> traced_ico_col() const noexcept {
        return m_view.column<int16_t>(h_traced_ico);
    }
    /// traced_emix column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_emix_col() const noexcept {
        return m_view.column<float>(h_traced_emix);
    }
    /// traced_emiy column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_emiy_col() const noexcept {
        return m_view.column<float>(h_traced_emiy);
    }
    /// traced_emiz column as a contiguous span.
    [[nodiscard]] std::span<const float> traced_emiz_col() const noexcept {
        return m_view.column<float>(h_traced_emiz);
    }
    /// etaC_dir column as a contiguous span.
    [[nodiscard]] std::span<const float> etaC_dir_col() const noexcept {
        return m_view.column<float>(h_etaC_dir);
    }
    /// etaC_lat column as a contiguous span.
    [[nodiscard]] std::span<const float> etaC_lat_col() const noexcept {
        return m_view.column<float>(h_etaC_lat);
    }
    /// etaC_sphe column as a contiguous span.
    [[nodiscard]] std::span<const float> etaC_sphe_col() const noexcept {
        return m_view.column<float>(h_etaC_sphe);
    }
    /// etaC_rms column as a contiguous span.
    [[nodiscard]] std::span<const float> etaC_rms_col() const noexcept {
        return m_view.column<float>(h_etaC_rms);
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
    hipo::column_handle h_traced_the{};
    hipo::column_handle h_traced_phi{};
    hipo::column_handle h_traced_hitx{};
    hipo::column_handle h_traced_hity{};
    hipo::column_handle h_traced_hitz{};
    hipo::column_handle h_traced_time{};
    hipo::column_handle h_traced_path{};
    hipo::column_handle h_traced_mchi2{};
    hipo::column_handle h_traced_ilay{};
    hipo::column_handle h_traced_ico{};
    hipo::column_handle h_traced_emix{};
    hipo::column_handle h_traced_emiy{};
    hipo::column_handle h_traced_emiz{};
    hipo::column_handle h_etaC_dir{};
    hipo::column_handle h_etaC_lat{};
    hipo::column_handle h_etaC_sphe{};
    hipo::column_handle h_etaC_rms{};

    friend class Row;
};

/// Lightweight row view for RICH::Hadron. Stores a pointer + row index only.
class RICH_Hadron::Row {
 public:
    Row(const RICH_Hadron* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int hindex() const noexcept { return m_bank->hindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] float traced_the() const noexcept { return m_bank->traced_the(m_row); }
    [[nodiscard]] float traced_phi() const noexcept { return m_bank->traced_phi(m_row); }
    [[nodiscard]] float traced_hitx() const noexcept { return m_bank->traced_hitx(m_row); }
    [[nodiscard]] float traced_hity() const noexcept { return m_bank->traced_hity(m_row); }
    [[nodiscard]] float traced_hitz() const noexcept { return m_bank->traced_hitz(m_row); }
    [[nodiscard]] float traced_time() const noexcept { return m_bank->traced_time(m_row); }
    [[nodiscard]] float traced_path() const noexcept { return m_bank->traced_path(m_row); }
    [[nodiscard]] float traced_mchi2() const noexcept { return m_bank->traced_mchi2(m_row); }
    [[nodiscard]] int traced_ilay() const noexcept { return m_bank->traced_ilay(m_row); }
    [[nodiscard]] int traced_ico() const noexcept { return m_bank->traced_ico(m_row); }
    [[nodiscard]] float traced_emix() const noexcept { return m_bank->traced_emix(m_row); }
    [[nodiscard]] float traced_emiy() const noexcept { return m_bank->traced_emiy(m_row); }
    [[nodiscard]] float traced_emiz() const noexcept { return m_bank->traced_emiz(m_row); }
    [[nodiscard]] float etaC_dir() const noexcept { return m_bank->etaC_dir(m_row); }
    [[nodiscard]] float etaC_lat() const noexcept { return m_bank->etaC_lat(m_row); }
    [[nodiscard]] float etaC_sphe() const noexcept { return m_bank->etaC_sphe(m_row); }
    [[nodiscard]] float etaC_rms() const noexcept { return m_bank->etaC_rms(m_row); }
 private:
    const RICH_Hadron* m_bank;
    int m_row;
};

inline RICH_Hadron::Row RICH_Hadron::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
