// =============================================================================
// Auto-generated from fmt.json - do not edit by hand.
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


/// HIPO bank wrapper for `FMT::Tracks` (group 20800, item 36).
/// reconstructed tracks using FMT information
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FMT_Tracks bank;
/// if (auto sc = bank.bind(event.get_bank("FMT::Tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class FMT_Tracks {
 public:
    static constexpr std::string_view bank_name = "FMT::Tracks";
    static constexpr int bank_group = 20800;
    static constexpr int bank_item  = 36;

    class Row;

    FMT_Tracks() = default;

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
            hipo::column_handle h_index;
            hipo::column_handle h_status;
            hipo::column_handle h_sector;
            hipo::column_handle h_Vtx0_x;
            hipo::column_handle h_Vtx0_y;
            hipo::column_handle h_Vtx0_z;
            hipo::column_handle h_p0_x;
            hipo::column_handle h_p0_y;
            hipo::column_handle h_p0_z;
            hipo::column_handle h_q;
            hipo::column_handle h_chi2;
            hipo::column_handle h_NDF;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("index"); h) cache.h_index = *h;
            else cache.h_index = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("Vtx0_x"); h) cache.h_Vtx0_x = *h;
            else cache.h_Vtx0_x = {};
            if (auto h = schema.get_handle("Vtx0_y"); h) cache.h_Vtx0_y = *h;
            else cache.h_Vtx0_y = {};
            if (auto h = schema.get_handle("Vtx0_z"); h) cache.h_Vtx0_z = *h;
            else cache.h_Vtx0_z = {};
            if (auto h = schema.get_handle("p0_x"); h) cache.h_p0_x = *h;
            else cache.h_p0_x = {};
            if (auto h = schema.get_handle("p0_y"); h) cache.h_p0_y = *h;
            else cache.h_p0_y = {};
            if (auto h = schema.get_handle("p0_z"); h) cache.h_p0_z = *h;
            else cache.h_p0_z = {};
            if (auto h = schema.get_handle("q"); h) cache.h_q = *h;
            else cache.h_q = {};
            if (auto h = schema.get_handle("chi2"); h) cache.h_chi2 = *h;
            else cache.h_chi2 = {};
            if (auto h = schema.get_handle("NDF"); h) cache.h_NDF = *h;
            else cache.h_NDF = {};
        }
        h_index = cache.h_index;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_Vtx0_x = cache.h_Vtx0_x;
        h_Vtx0_y = cache.h_Vtx0_y;
        h_Vtx0_z = cache.h_Vtx0_z;
        h_p0_x = cache.h_p0_x;
        h_p0_y = cache.h_p0_y;
        h_p0_z = cache.h_p0_z;
        h_q = cache.h_q;
        h_chi2 = cache.h_chi2;
        h_NDF = cache.h_NDF;
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
    /// index: index of the track in the DC bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_index, row));
    }
    /// status: status of the track (0: refitted using FMT, 1: original DC track)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_status, row));
    }
    /// sector: sector of the track in DC
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// Vtx0_x: Vertex x-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Vtx0_x, row));
    }
    /// Vtx0_y: Vertex y-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Vtx0_y, row));
    }
    /// Vtx0_z: Vertex z-position of the swam track to the DOCA to the beamline (in cm)
    [[nodiscard]] float Vtx0_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Vtx0_z, row));
    }
    /// p0_x: 3-momentum x-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float p0_x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p0_x, row));
    }
    /// p0_y: 3-momentum y-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float p0_y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p0_y, row));
    }
    /// p0_z: 3-momentum z-coordinate of the swam track to the DOCA to the beamline (in GeV)
    [[nodiscard]] float p0_z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p0_z, row));
    }
    /// q: charge of the track
    [[nodiscard]] int q(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_q, row));
    }
    /// chi2: chi^2 of the fit
    [[nodiscard]] float chi2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_chi2, row));
    }
    /// NDF: number of degrees of freedom of the fit
    [[nodiscard]] int NDF(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_NDF, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> index_col() const noexcept {
        return m_view.column<int16_t>(h_index);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> status_col() const noexcept {
        return m_view.column<int8_t>(h_status);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// Vtx0_x column as a contiguous span.
    [[nodiscard]] std::span<const float> Vtx0_x_col() const noexcept {
        return m_view.column<float>(h_Vtx0_x);
    }
    /// Vtx0_y column as a contiguous span.
    [[nodiscard]] std::span<const float> Vtx0_y_col() const noexcept {
        return m_view.column<float>(h_Vtx0_y);
    }
    /// Vtx0_z column as a contiguous span.
    [[nodiscard]] std::span<const float> Vtx0_z_col() const noexcept {
        return m_view.column<float>(h_Vtx0_z);
    }
    /// p0_x column as a contiguous span.
    [[nodiscard]] std::span<const float> p0_x_col() const noexcept {
        return m_view.column<float>(h_p0_x);
    }
    /// p0_y column as a contiguous span.
    [[nodiscard]] std::span<const float> p0_y_col() const noexcept {
        return m_view.column<float>(h_p0_y);
    }
    /// p0_z column as a contiguous span.
    [[nodiscard]] std::span<const float> p0_z_col() const noexcept {
        return m_view.column<float>(h_p0_z);
    }
    /// q column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> q_col() const noexcept {
        return m_view.column<int8_t>(h_q);
    }
    /// chi2 column as a contiguous span.
    [[nodiscard]] std::span<const float> chi2_col() const noexcept {
        return m_view.column<float>(h_chi2);
    }
    /// NDF column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> NDF_col() const noexcept {
        return m_view.column<int8_t>(h_NDF);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle index_handle() const noexcept { return h_index; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_index{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_Vtx0_x{};
    hipo::column_handle h_Vtx0_y{};
    hipo::column_handle h_Vtx0_z{};
    hipo::column_handle h_p0_x{};
    hipo::column_handle h_p0_y{};
    hipo::column_handle h_p0_z{};
    hipo::column_handle h_q{};
    hipo::column_handle h_chi2{};
    hipo::column_handle h_NDF{};

    friend class Row;
};

/// Lightweight row view for FMT::Tracks. Stores a pointer + row index only.
class FMT_Tracks::Row {
 public:
    Row(const FMT_Tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] float Vtx0_x() const noexcept { return m_bank->Vtx0_x(m_row); }
    [[nodiscard]] float Vtx0_y() const noexcept { return m_bank->Vtx0_y(m_row); }
    [[nodiscard]] float Vtx0_z() const noexcept { return m_bank->Vtx0_z(m_row); }
    [[nodiscard]] float p0_x() const noexcept { return m_bank->p0_x(m_row); }
    [[nodiscard]] float p0_y() const noexcept { return m_bank->p0_y(m_row); }
    [[nodiscard]] float p0_z() const noexcept { return m_bank->p0_z(m_row); }
    [[nodiscard]] int q() const noexcept { return m_bank->q(m_row); }
    [[nodiscard]] float chi2() const noexcept { return m_bank->chi2(m_row); }
    [[nodiscard]] int NDF() const noexcept { return m_bank->NDF(m_row); }
 private:
    const FMT_Tracks* m_bank;
    int m_row;
};

inline FMT_Tracks::Row FMT_Tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
