// =============================================================================
// Auto-generated from htcc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `HTCC::rec` (group 21500, item 22).
/// reconstructed clusters in HTCC
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// HTCC_rec bank;
/// if (auto sc = bank.bind(event.get_bank("HTCC::rec")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HTCC_rec {
 public:
    static constexpr std::string_view bank_name = "HTCC::rec";
    static constexpr int bank_group = 21500;
    static constexpr int bank_item  = 22;
    static constexpr std::string_view bank_schema = "id/S,nhits/S,nphe/F,ntheta/S,nphi/S,mintheta/S,maxtheta/S,minphi/S,maxphi/S,time/F,theta/F,dtheta/F,phi/F,dphi/F,x/F,y/F,z/F";

    class Row;

    HTCC_rec() = default;

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
            int i_id = -1;
            int i_nhits = -1;
            int i_nphe = -1;
            int i_ntheta = -1;
            int i_nphi = -1;
            int i_mintheta = -1;
            int i_maxtheta = -1;
            int i_minphi = -1;
            int i_maxphi = -1;
            int i_time = -1;
            int i_theta = -1;
            int i_dtheta = -1;
            int i_phi = -1;
            int i_dphi = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_nhits = schema.exists("nhits") ? schema.getEntryOrder("nhits") : -1;
            cache.i_nphe = schema.exists("nphe") ? schema.getEntryOrder("nphe") : -1;
            cache.i_ntheta = schema.exists("ntheta") ? schema.getEntryOrder("ntheta") : -1;
            cache.i_nphi = schema.exists("nphi") ? schema.getEntryOrder("nphi") : -1;
            cache.i_mintheta = schema.exists("mintheta") ? schema.getEntryOrder("mintheta") : -1;
            cache.i_maxtheta = schema.exists("maxtheta") ? schema.getEntryOrder("maxtheta") : -1;
            cache.i_minphi = schema.exists("minphi") ? schema.getEntryOrder("minphi") : -1;
            cache.i_maxphi = schema.exists("maxphi") ? schema.getEntryOrder("maxphi") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
            cache.i_theta = schema.exists("theta") ? schema.getEntryOrder("theta") : -1;
            cache.i_dtheta = schema.exists("dtheta") ? schema.getEntryOrder("dtheta") : -1;
            cache.i_phi = schema.exists("phi") ? schema.getEntryOrder("phi") : -1;
            cache.i_dphi = schema.exists("dphi") ? schema.getEntryOrder("dphi") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
        }
        i_id = cache.i_id;
        i_nhits = cache.i_nhits;
        i_nphe = cache.i_nphe;
        i_ntheta = cache.i_ntheta;
        i_nphi = cache.i_nphi;
        i_mintheta = cache.i_mintheta;
        i_maxtheta = cache.i_maxtheta;
        i_minphi = cache.i_minphi;
        i_maxphi = cache.i_maxphi;
        i_time = cache.i_time;
        i_theta = cache.i_theta;
        i_dtheta = cache.i_dtheta;
        i_phi = cache.i_phi;
        i_dphi = cache.i_dphi;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// nhits: number of hits in cluster
    [[nodiscard]] int nhits(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_nhits, row));
    }
    /// nphe: number of photoelectrons in cluster
    [[nodiscard]] float nphe(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_nphe, row));
    }
    /// ntheta: ntheta
    [[nodiscard]] int ntheta(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_ntheta, row));
    }
    /// nphi: nphi
    [[nodiscard]] int nphi(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_nphi, row));
    }
    /// mintheta: Minimum theta angle
    [[nodiscard]] int mintheta(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_mintheta, row));
    }
    /// maxtheta: Maximum theta angle
    [[nodiscard]] int maxtheta(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_maxtheta, row));
    }
    /// minphi: Minimum phi angle
    [[nodiscard]] int minphi(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_minphi, row));
    }
    /// maxphi: Maximum phi angle
    [[nodiscard]] int maxphi(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_maxphi, row));
    }
    /// time: Time at the vertex
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_time, row));
    }
    /// theta: Theta of the cluster
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_theta, row));
    }
    /// dtheta: Error of the theta
    [[nodiscard]] float dtheta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dtheta, row));
    }
    /// phi: Phi of the cluster
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_phi, row));
    }
    /// dphi: Error of the phi
    [[nodiscard]] float dphi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_dphi, row));
    }
    /// x: X coordinate of the hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: Y coordinate of the hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: Z coordinate of the hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_nhits = -1;
    int i_nphe = -1;
    int i_ntheta = -1;
    int i_nphi = -1;
    int i_mintheta = -1;
    int i_maxtheta = -1;
    int i_minphi = -1;
    int i_maxphi = -1;
    int i_time = -1;
    int i_theta = -1;
    int i_dtheta = -1;
    int i_phi = -1;
    int i_dphi = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;

    friend class Row;
};

/// Lightweight row view for HTCC::rec. Stores a pointer + row index only.
class HTCC_rec::Row {
 public:
    Row(const HTCC_rec* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int nhits() const noexcept { return m_bank->nhits(m_row); }
    [[nodiscard]] float nphe() const noexcept { return m_bank->nphe(m_row); }
    [[nodiscard]] int ntheta() const noexcept { return m_bank->ntheta(m_row); }
    [[nodiscard]] int nphi() const noexcept { return m_bank->nphi(m_row); }
    [[nodiscard]] int mintheta() const noexcept { return m_bank->mintheta(m_row); }
    [[nodiscard]] int maxtheta() const noexcept { return m_bank->maxtheta(m_row); }
    [[nodiscard]] int minphi() const noexcept { return m_bank->minphi(m_row); }
    [[nodiscard]] int maxphi() const noexcept { return m_bank->maxphi(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float theta() const noexcept { return m_bank->theta(m_row); }
    [[nodiscard]] float dtheta() const noexcept { return m_bank->dtheta(m_row); }
    [[nodiscard]] float phi() const noexcept { return m_bank->phi(m_row); }
    [[nodiscard]] float dphi() const noexcept { return m_bank->dphi(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
 private:
    const HTCC_rec* m_bank;
    int m_row;
};

inline HTCC_rec::Row HTCC_rec::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
