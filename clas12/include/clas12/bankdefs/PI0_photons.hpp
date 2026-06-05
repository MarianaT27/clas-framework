// =============================================================================
// Auto-generated from pi0skim.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `PI0::photons` (group 30000, item 2).
/// Slim pi0 skim: selected photon 4-momenta (one row per photon)
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// PI0_photons bank;
/// if (auto sc = bank.bind(event.get_bank("PI0::photons")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto px = row.px();
/// }
/// @endcode
class PI0_photons {
 public:
    static constexpr std::string_view bank_name = "PI0::photons";
    static constexpr int bank_group = 30000;
    static constexpr int bank_item  = 2;
    static constexpr std::string_view bank_schema = "px/F,py/F,pz/F,E/F";

    class Row;

    PI0_photons() = default;

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
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_E = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_E = schema.exists("E") ? schema.getEntryOrder("E") : -1;
        }
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_E = cache.i_E;
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
    /// px: photon px (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: photon py (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: photon pz (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// E: photon energy (GeV)
    [[nodiscard]] float E(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_E, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_E = -1;

    friend class Row;
};

/// Lightweight row view for PI0::photons. Stores a pointer + row index only.
class PI0_photons::Row {
 public:
    Row(const PI0_photons* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float E() const noexcept { return m_bank->E(m_row); }
 private:
    const PI0_photons* m_bank;
    int m_row;
};

inline PI0_photons::Row PI0_photons::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
