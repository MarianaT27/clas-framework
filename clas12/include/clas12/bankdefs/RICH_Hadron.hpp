// =============================================================================
// Auto-generated from rich.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RICH::Hadron` (group 21800, item 38).
/// Reconstructed Hadrons in RICH
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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

    /// Bind to a live hipo::bank. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column name to its item
    /// index via schema.getEntryOrder(); subsequent bind()s for the same
    /// schema hit the cache. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            const hipo::schema* schema_ptr = nullptr;
            int i_id = -1;
            int i_hindex = -1;
            int i_pindex = -1;
            int i_sector = -1;
            int i_traced_the = -1;
            int i_traced_phi = -1;
            int i_traced_hitx = -1;
            int i_traced_hity = -1;
            int i_traced_hitz = -1;
            int i_traced_time = -1;
            int i_traced_path = -1;
            int i_traced_mchi2 = -1;
            int i_traced_ilay = -1;
            int i_traced_ico = -1;
            int i_traced_emix = -1;
            int i_traced_emiy = -1;
            int i_traced_emiz = -1;
            int i_etaC_dir = -1;
            int i_etaC_lat = -1;
            int i_etaC_sphe = -1;
            int i_etaC_rms = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_hindex = schema.exists("hindex") ? schema.getEntryOrder("hindex") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_traced_the = schema.exists("traced_the") ? schema.getEntryOrder("traced_the") : -1;
            cache.i_traced_phi = schema.exists("traced_phi") ? schema.getEntryOrder("traced_phi") : -1;
            cache.i_traced_hitx = schema.exists("traced_hitx") ? schema.getEntryOrder("traced_hitx") : -1;
            cache.i_traced_hity = schema.exists("traced_hity") ? schema.getEntryOrder("traced_hity") : -1;
            cache.i_traced_hitz = schema.exists("traced_hitz") ? schema.getEntryOrder("traced_hitz") : -1;
            cache.i_traced_time = schema.exists("traced_time") ? schema.getEntryOrder("traced_time") : -1;
            cache.i_traced_path = schema.exists("traced_path") ? schema.getEntryOrder("traced_path") : -1;
            cache.i_traced_mchi2 = schema.exists("traced_mchi2") ? schema.getEntryOrder("traced_mchi2") : -1;
            cache.i_traced_ilay = schema.exists("traced_ilay") ? schema.getEntryOrder("traced_ilay") : -1;
            cache.i_traced_ico = schema.exists("traced_ico") ? schema.getEntryOrder("traced_ico") : -1;
            cache.i_traced_emix = schema.exists("traced_emix") ? schema.getEntryOrder("traced_emix") : -1;
            cache.i_traced_emiy = schema.exists("traced_emiy") ? schema.getEntryOrder("traced_emiy") : -1;
            cache.i_traced_emiz = schema.exists("traced_emiz") ? schema.getEntryOrder("traced_emiz") : -1;
            cache.i_etaC_dir = schema.exists("etaC_dir") ? schema.getEntryOrder("etaC_dir") : -1;
            cache.i_etaC_lat = schema.exists("etaC_lat") ? schema.getEntryOrder("etaC_lat") : -1;
            cache.i_etaC_sphe = schema.exists("etaC_sphe") ? schema.getEntryOrder("etaC_sphe") : -1;
            cache.i_etaC_rms = schema.exists("etaC_rms") ? schema.getEntryOrder("etaC_rms") : -1;
        }
        i_id = cache.i_id;
        i_hindex = cache.i_hindex;
        i_pindex = cache.i_pindex;
        i_sector = cache.i_sector;
        i_traced_the = cache.i_traced_the;
        i_traced_phi = cache.i_traced_phi;
        i_traced_hitx = cache.i_traced_hitx;
        i_traced_hity = cache.i_traced_hity;
        i_traced_hitz = cache.i_traced_hitz;
        i_traced_time = cache.i_traced_time;
        i_traced_path = cache.i_traced_path;
        i_traced_mchi2 = cache.i_traced_mchi2;
        i_traced_ilay = cache.i_traced_ilay;
        i_traced_ico = cache.i_traced_ico;
        i_traced_emix = cache.i_traced_emix;
        i_traced_emiy = cache.i_traced_emiy;
        i_traced_emiz = cache.i_traced_emiz;
        i_etaC_dir = cache.i_etaC_dir;
        i_etaC_lat = cache.i_etaC_lat;
        i_etaC_sphe = cache.i_etaC_sphe;
        i_etaC_rms = cache.i_etaC_rms;
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
    /// id: hadron id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// hindex: row in the RICH clusters bank
    [[nodiscard]] int hindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_hindex, row));
    }
    /// pindex: row in the REC  particle bank
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pindex, row));
    }
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// traced_the: Lab theta angle for traced solution
    [[nodiscard]] float traced_the(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_the, row));
    }
    /// traced_phi: Lab phi angle for traced solution
    [[nodiscard]] float traced_phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_phi, row));
    }
    /// traced_hitx: Lab hitx angle for traced solution
    [[nodiscard]] float traced_hitx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_hitx, row));
    }
    /// traced_hity: Lab hity angle for traced solution
    [[nodiscard]] float traced_hity(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_hity, row));
    }
    /// traced_hitz: Lab hitz angle for traced solution
    [[nodiscard]] float traced_hitz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_hitz, row));
    }
    /// traced_time: time for traced solution
    [[nodiscard]] float traced_time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_time, row));
    }
    /// traced_path: path for traced solution
    [[nodiscard]] float traced_path(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_path, row));
    }
    /// traced_mchi2: mchi2 for traced solution
    [[nodiscard]] float traced_mchi2(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_mchi2, row));
    }
    /// traced_ilay: emission aero layer for traced solution
    [[nodiscard]] int traced_ilay(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_traced_ilay, row));
    }
    /// traced_ico: emission aero component for traced solution
    [[nodiscard]] int traced_ico(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_traced_ico, row));
    }
    /// traced_emix: emission point x for traced solution
    [[nodiscard]] float traced_emix(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_emix, row));
    }
    /// traced_emiy: emission point y for traced solution
    [[nodiscard]] float traced_emiy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_emiy, row));
    }
    /// traced_emiz: emission point z for traced solution
    [[nodiscard]] float traced_emiz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_traced_emiz, row));
    }
    /// etaC_dir: expected etaC for electron (direct photons)
    [[nodiscard]] float etaC_dir(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_etaC_dir, row));
    }
    /// etaC_lat: expected etaC for electron (after lateral reflection)
    [[nodiscard]] float etaC_lat(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_etaC_lat, row));
    }
    /// etaC_sphe: expected etaC for electron (after spherical reflection)
    [[nodiscard]] float etaC_sphe(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_etaC_sphe, row));
    }
    /// etaC_rms: expected etaC resolution
    [[nodiscard]] float etaC_rms(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_etaC_rms, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int hindex_handle() const noexcept { return i_hindex; }
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_hindex = -1;
    int i_pindex = -1;
    int i_sector = -1;
    int i_traced_the = -1;
    int i_traced_phi = -1;
    int i_traced_hitx = -1;
    int i_traced_hity = -1;
    int i_traced_hitz = -1;
    int i_traced_time = -1;
    int i_traced_path = -1;
    int i_traced_mchi2 = -1;
    int i_traced_ilay = -1;
    int i_traced_ico = -1;
    int i_traced_emix = -1;
    int i_traced_emiy = -1;
    int i_traced_emiz = -1;
    int i_etaC_dir = -1;
    int i_etaC_lat = -1;
    int i_etaC_sphe = -1;
    int i_etaC_rms = -1;

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

}  // namespace bankdefs
