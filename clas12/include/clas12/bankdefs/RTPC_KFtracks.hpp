// =============================================================================
// Auto-generated from rtpc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RTPC::KFtracks` (group 21900, item 23).
/// Kalman Fitter track info
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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
            int i_trkID = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_vz = -1;
            int i_dEdx = -1;
            int i_p_drift = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_trkID = schema.exists("trkID") ? schema.getEntryOrder("trkID") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
            cache.i_dEdx = schema.exists("dEdx") ? schema.getEntryOrder("dEdx") : -1;
            cache.i_p_drift = schema.exists("p_drift") ? schema.getEntryOrder("p_drift") : -1;
        }
        i_trkID = cache.i_trkID;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_vz = cache.i_vz;
        i_dEdx = cache.i_dEdx;
        i_p_drift = cache.i_p_drift;
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
    /// trkID: Track ID of the hit
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_trkID, row));
    }
    /// px: X component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: Y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: Z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// vz: Z component of the reconstructed vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }
    /// dEdx: dEdx for the Track (MeV/mm)
    [[nodiscard]] float dEdx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dEdx, row));
    }
    /// p_drift: Momentum in the drift region (GeV)
    [[nodiscard]] float p_drift(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p_drift, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_trkID = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_vz = -1;
    int i_dEdx = -1;
    int i_p_drift = -1;

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

}  // namespace bankdefs
