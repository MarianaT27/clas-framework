// =============================================================================
// Auto-generated from ltcc.json - do not edit by hand.
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


/// HIPO bank wrapper for `LTCC::clusters` (group 21600, item 22).
/// reconstructed clusters from the LTCC
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// LTCC_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("LTCC::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class LTCC_clusters {
 public:
    static constexpr std::string_view bank_name = "LTCC::clusters";
    static constexpr int bank_group = 21600;
    static constexpr int bank_item  = 22;

    class Row;

    LTCC_clusters() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("segment"); h) h_segment = *h;
        else h_segment = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("nphe"); h) h_nphe = *h;
        else h_nphe = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("nHits"); h) h_nHits = *h;
        else h_nHits = {};
        if (auto h = schema.get_handle("minTheta"); h) h_minTheta = *h;
        else h_minTheta = {};
        if (auto h = schema.get_handle("maxTheta"); h) h_maxTheta = *h;
        else h_maxTheta = {};
        if (auto h = schema.get_handle("minPhi"); h) h_minPhi = *h;
        else h_minPhi = {};
        if (auto h = schema.get_handle("maxPhi"); h) h_maxPhi = *h;
        else h_maxPhi = {};
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
    /// id: id of the cluster
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// status: good (1) or bad (0)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_status, row));
    }
    /// sector: sector of LTCC
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// segment: weighted central cluster segment
    [[nodiscard]] int segment(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_segment, row));
    }
    /// x: crude cluster center (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: crude cluster center (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: crude cluster center (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// nphe: number of photo-electrons
    [[nodiscard]] float nphe(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_nphe, row));
    }
    /// time: cluster time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// nHits: number of hits in this cluster
    [[nodiscard]] int nHits(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_nHits, row));
    }
    /// minTheta: minimum theta angle of the cluster (deg)
    [[nodiscard]] float minTheta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_minTheta, row));
    }
    /// maxTheta: maximum theta angle of the cluster (deg)
    [[nodiscard]] float maxTheta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_maxTheta, row));
    }
    /// minPhi: minimum phi angle of the cluster (deg)
    [[nodiscard]] float minPhi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_minPhi, row));
    }
    /// maxPhi: maximum phi angle of the cluster (deg)
    [[nodiscard]] float maxPhi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_maxPhi, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> status_col() const noexcept {
        return m_view.column<int8_t>(h_status);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// segment column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> segment_col() const noexcept {
        return m_view.column<int16_t>(h_segment);
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
    /// nphe column as a contiguous span.
    [[nodiscard]] std::span<const float> nphe_col() const noexcept {
        return m_view.column<float>(h_nphe);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// nHits column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> nHits_col() const noexcept {
        return m_view.column<int16_t>(h_nHits);
    }
    /// minTheta column as a contiguous span.
    [[nodiscard]] std::span<const float> minTheta_col() const noexcept {
        return m_view.column<float>(h_minTheta);
    }
    /// maxTheta column as a contiguous span.
    [[nodiscard]] std::span<const float> maxTheta_col() const noexcept {
        return m_view.column<float>(h_maxTheta);
    }
    /// minPhi column as a contiguous span.
    [[nodiscard]] std::span<const float> minPhi_col() const noexcept {
        return m_view.column<float>(h_minPhi);
    }
    /// maxPhi column as a contiguous span.
    [[nodiscard]] std::span<const float> maxPhi_col() const noexcept {
        return m_view.column<float>(h_maxPhi);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_segment{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_nphe{};
    hipo::column_handle h_time{};
    hipo::column_handle h_nHits{};
    hipo::column_handle h_minTheta{};
    hipo::column_handle h_maxTheta{};
    hipo::column_handle h_minPhi{};
    hipo::column_handle h_maxPhi{};

    friend class Row;
};

/// Lightweight row view for LTCC::clusters. Stores a pointer + row index only.
class LTCC_clusters::Row {
 public:
    Row(const LTCC_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int segment() const noexcept { return m_bank->segment(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float nphe() const noexcept { return m_bank->nphe(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int nHits() const noexcept { return m_bank->nHits(m_row); }
    [[nodiscard]] float minTheta() const noexcept { return m_bank->minTheta(m_row); }
    [[nodiscard]] float maxTheta() const noexcept { return m_bank->maxTheta(m_row); }
    [[nodiscard]] float minPhi() const noexcept { return m_bank->minPhi(m_row); }
    [[nodiscard]] float maxPhi() const noexcept { return m_bank->maxPhi(m_row); }
 private:
    const LTCC_clusters* m_bank;
    int m_row;
};

inline LTCC_clusters::Row LTCC_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
