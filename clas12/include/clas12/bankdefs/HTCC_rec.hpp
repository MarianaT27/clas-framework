// =============================================================================
// Auto-generated from htcc.json - do not edit by hand.
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

namespace bankdefs {


/// HIPO bank wrapper for `HTCC::rec` (group 21500, item 22).
/// reconstructed clusters in HTCC
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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

    class Row;

    HTCC_rec() = default;

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
            hipo::column_handle h_nhits;
            hipo::column_handle h_nphe;
            hipo::column_handle h_ntheta;
            hipo::column_handle h_nphi;
            hipo::column_handle h_mintheta;
            hipo::column_handle h_maxtheta;
            hipo::column_handle h_minphi;
            hipo::column_handle h_maxphi;
            hipo::column_handle h_time;
            hipo::column_handle h_theta;
            hipo::column_handle h_dtheta;
            hipo::column_handle h_phi;
            hipo::column_handle h_dphi;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("nhits"); h) cache.h_nhits = *h;
            else cache.h_nhits = {};
            if (auto h = schema.get_handle("nphe"); h) cache.h_nphe = *h;
            else cache.h_nphe = {};
            if (auto h = schema.get_handle("ntheta"); h) cache.h_ntheta = *h;
            else cache.h_ntheta = {};
            if (auto h = schema.get_handle("nphi"); h) cache.h_nphi = *h;
            else cache.h_nphi = {};
            if (auto h = schema.get_handle("mintheta"); h) cache.h_mintheta = *h;
            else cache.h_mintheta = {};
            if (auto h = schema.get_handle("maxtheta"); h) cache.h_maxtheta = *h;
            else cache.h_maxtheta = {};
            if (auto h = schema.get_handle("minphi"); h) cache.h_minphi = *h;
            else cache.h_minphi = {};
            if (auto h = schema.get_handle("maxphi"); h) cache.h_maxphi = *h;
            else cache.h_maxphi = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("theta"); h) cache.h_theta = *h;
            else cache.h_theta = {};
            if (auto h = schema.get_handle("dtheta"); h) cache.h_dtheta = *h;
            else cache.h_dtheta = {};
            if (auto h = schema.get_handle("phi"); h) cache.h_phi = *h;
            else cache.h_phi = {};
            if (auto h = schema.get_handle("dphi"); h) cache.h_dphi = *h;
            else cache.h_dphi = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
        }
        h_id = cache.h_id;
        h_nhits = cache.h_nhits;
        h_nphe = cache.h_nphe;
        h_ntheta = cache.h_ntheta;
        h_nphi = cache.h_nphi;
        h_mintheta = cache.h_mintheta;
        h_maxtheta = cache.h_maxtheta;
        h_minphi = cache.h_minphi;
        h_maxphi = cache.h_maxphi;
        h_time = cache.h_time;
        h_theta = cache.h_theta;
        h_dtheta = cache.h_dtheta;
        h_phi = cache.h_phi;
        h_dphi = cache.h_dphi;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// nhits: number of hits in cluster
    [[nodiscard]] int nhits(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_nhits, row));
    }
    /// nphe: number of photoelectrons in cluster
    [[nodiscard]] float nphe(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_nphe, row));
    }
    /// ntheta: ntheta
    [[nodiscard]] int ntheta(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ntheta, row));
    }
    /// nphi: nphi
    [[nodiscard]] int nphi(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_nphi, row));
    }
    /// mintheta: Minimum theta angle
    [[nodiscard]] int mintheta(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_mintheta, row));
    }
    /// maxtheta: Maximum theta angle
    [[nodiscard]] int maxtheta(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_maxtheta, row));
    }
    /// minphi: Minimum phi angle
    [[nodiscard]] int minphi(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_minphi, row));
    }
    /// maxphi: Maximum phi angle
    [[nodiscard]] int maxphi(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_maxphi, row));
    }
    /// time: Time at the vertex
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// theta: Theta of the cluster
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_theta, row));
    }
    /// dtheta: Error of the theta
    [[nodiscard]] float dtheta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dtheta, row));
    }
    /// phi: Phi of the cluster
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_phi, row));
    }
    /// dphi: Error of the phi
    [[nodiscard]] float dphi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_dphi, row));
    }
    /// x: X coordinate of the hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Y coordinate of the hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Z coordinate of the hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// nhits column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> nhits_col() const noexcept {
        return m_view.column<int16_t>(h_nhits);
    }
    /// nphe column as a contiguous span.
    [[nodiscard]] std::span<const float> nphe_col() const noexcept {
        return m_view.column<float>(h_nphe);
    }
    /// ntheta column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ntheta_col() const noexcept {
        return m_view.column<int16_t>(h_ntheta);
    }
    /// nphi column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> nphi_col() const noexcept {
        return m_view.column<int16_t>(h_nphi);
    }
    /// mintheta column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> mintheta_col() const noexcept {
        return m_view.column<int16_t>(h_mintheta);
    }
    /// maxtheta column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> maxtheta_col() const noexcept {
        return m_view.column<int16_t>(h_maxtheta);
    }
    /// minphi column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> minphi_col() const noexcept {
        return m_view.column<int16_t>(h_minphi);
    }
    /// maxphi column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> maxphi_col() const noexcept {
        return m_view.column<int16_t>(h_maxphi);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// theta column as a contiguous span.
    [[nodiscard]] std::span<const float> theta_col() const noexcept {
        return m_view.column<float>(h_theta);
    }
    /// dtheta column as a contiguous span.
    [[nodiscard]] std::span<const float> dtheta_col() const noexcept {
        return m_view.column<float>(h_dtheta);
    }
    /// phi column as a contiguous span.
    [[nodiscard]] std::span<const float> phi_col() const noexcept {
        return m_view.column<float>(h_phi);
    }
    /// dphi column as a contiguous span.
    [[nodiscard]] std::span<const float> dphi_col() const noexcept {
        return m_view.column<float>(h_dphi);
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

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_nhits{};
    hipo::column_handle h_nphe{};
    hipo::column_handle h_ntheta{};
    hipo::column_handle h_nphi{};
    hipo::column_handle h_mintheta{};
    hipo::column_handle h_maxtheta{};
    hipo::column_handle h_minphi{};
    hipo::column_handle h_maxphi{};
    hipo::column_handle h_time{};
    hipo::column_handle h_theta{};
    hipo::column_handle h_dtheta{};
    hipo::column_handle h_phi{};
    hipo::column_handle h_dphi{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};

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
