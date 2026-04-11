// =============================================================================
// Auto-generated from bmt.json - do not edit by hand.
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


/// HIPO bank wrapper for `BMTRec::Clusters` (group 20100, item 22).
/// first pass reconstructed BMT clusters
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// BMTRec_Clusters bank;
/// if (auto sc = bank.bind(event.get_bank("BMTRec::Clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto ID = row.ID();
/// }
/// @endcode
class BMTRec_Clusters {
 public:
    static constexpr std::string_view bank_name = "BMTRec::Clusters";
    static constexpr int bank_group = 20100;
    static constexpr int bank_item  = 22;

    class Row;

    BMTRec_Clusters() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("ID"); h) h_ID = *h;
        else h_ID = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("size"); h) h_size = *h;
        else h_size = {};
        if (auto h = schema.get_handle("ETot"); h) h_ETot = *h;
        else h_ETot = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("seedE"); h) h_seedE = *h;
        else h_seedE = {};
        if (auto h = schema.get_handle("seedStrip"); h) h_seedStrip = *h;
        else h_seedStrip = {};
        if (auto h = schema.get_handle("centroid"); h) h_centroid = *h;
        else h_centroid = {};
        if (auto h = schema.get_handle("centroidValue"); h) h_centroidValue = *h;
        else h_centroidValue = {};
        if (auto h = schema.get_handle("centroidError"); h) h_centroidError = *h;
        else h_centroidError = {};
        if (auto h = schema.get_handle("centroidResidual"); h) h_centroidResidual = *h;
        else h_centroidResidual = {};
        if (auto h = schema.get_handle("seedResidual"); h) h_seedResidual = *h;
        else h_seedResidual = {};
        if (auto h = schema.get_handle("Hit1_ID"); h) h_Hit1_ID = *h;
        else h_Hit1_ID = {};
        if (auto h = schema.get_handle("Hit2_ID"); h) h_Hit2_ID = *h;
        else h_Hit2_ID = {};
        if (auto h = schema.get_handle("Hit3_ID"); h) h_Hit3_ID = *h;
        else h_Hit3_ID = {};
        if (auto h = schema.get_handle("Hit4_ID"); h) h_Hit4_ID = *h;
        else h_Hit4_ID = {};
        if (auto h = schema.get_handle("Hit5_ID"); h) h_Hit5_ID = *h;
        else h_Hit5_ID = {};
        if (auto h = schema.get_handle("trkID"); h) h_trkID = *h;
        else h_trkID = {};
        if (auto h = schema.get_handle("x1"); h) h_x1 = *h;
        else h_x1 = {};
        if (auto h = schema.get_handle("y1"); h) h_y1 = *h;
        else h_y1 = {};
        if (auto h = schema.get_handle("z1"); h) h_z1 = *h;
        else h_z1 = {};
        if (auto h = schema.get_handle("x2"); h) h_x2 = *h;
        else h_x2 = {};
        if (auto h = schema.get_handle("y2"); h) h_y2 = *h;
        else h_y2 = {};
        if (auto h = schema.get_handle("z2"); h) h_z2 = *h;
        else h_z2 = {};
        if (auto h = schema.get_handle("cx"); h) h_cx = *h;
        else h_cx = {};
        if (auto h = schema.get_handle("cy"); h) h_cy = *h;
        else h_cy = {};
        if (auto h = schema.get_handle("cz"); h) h_cz = *h;
        else h_cz = {};
        if (auto h = schema.get_handle("theta"); h) h_theta = *h;
        else h_theta = {};
        if (auto h = schema.get_handle("ax1"); h) h_ax1 = *h;
        else h_ax1 = {};
        if (auto h = schema.get_handle("ay1"); h) h_ay1 = *h;
        else h_ay1 = {};
        if (auto h = schema.get_handle("az1"); h) h_az1 = *h;
        else h_az1 = {};
        if (auto h = schema.get_handle("ax2"); h) h_ax2 = *h;
        else h_ax2 = {};
        if (auto h = schema.get_handle("ay2"); h) h_ay2 = *h;
        else h_ay2 = {};
        if (auto h = schema.get_handle("az2"); h) h_az2 = *h;
        else h_az2 = {};
        if (auto h = schema.get_handle("lx"); h) h_lx = *h;
        else h_lx = {};
        if (auto h = schema.get_handle("ly"); h) h_ly = *h;
        else h_ly = {};
        if (auto h = schema.get_handle("lz"); h) h_lz = *h;
        else h_lz = {};
        if (auto h = schema.get_handle("sx"); h) h_sx = *h;
        else h_sx = {};
        if (auto h = schema.get_handle("sy"); h) h_sy = *h;
        else h_sy = {};
        if (auto h = schema.get_handle("sz"); h) h_sz = *h;
        else h_sz = {};
        if (auto h = schema.get_handle("nx"); h) h_nx = *h;
        else h_nx = {};
        if (auto h = schema.get_handle("ny"); h) h_ny = *h;
        else h_ny = {};
        if (auto h = schema.get_handle("nz"); h) h_nz = *h;
        else h_nz = {};
        if (auto h = schema.get_handle("e"); h) h_e = *h;
        else h_e = {};
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
    /// ID: ID
    [[nodiscard]] int ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ID, row));
    }
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// size: cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// ETot: cluster total energy
    [[nodiscard]] float ETot(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ETot, row));
    }
    /// time: cluster average time
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// seedE: energy of the seed 
    [[nodiscard]] float seedE(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_seedE, row));
    }
    /// seedStrip: seed strip
    [[nodiscard]] int seedStrip(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_seedStrip, row));
    }
    /// centroid: centroid strip number
    [[nodiscard]] float centroid(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroid, row));
    }
    /// centroidValue: centroid value (z or phi)
    [[nodiscard]] float centroidValue(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroidValue, row));
    }
    /// centroidError: centroid value error
    [[nodiscard]] float centroidError(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroidError, row));
    }
    /// centroidResidual: centroid residual
    [[nodiscard]] float centroidResidual(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_centroidResidual, row));
    }
    /// seedResidual: seed residual
    [[nodiscard]] float seedResidual(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_seedResidual, row));
    }
    /// Hit1_ID: Index of hit 1 in cluster
    [[nodiscard]] int Hit1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit1_ID, row));
    }
    /// Hit2_ID: Index of hit 2 in cluster
    [[nodiscard]] int Hit2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit2_ID, row));
    }
    /// Hit3_ID: Index of hit 3 in cluster
    [[nodiscard]] int Hit3_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit3_ID, row));
    }
    /// Hit4_ID: Index of hit 4 in cluster
    [[nodiscard]] int Hit4_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit4_ID, row));
    }
    /// Hit5_ID: Index of hit 5 in cluster
    [[nodiscard]] int Hit5_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Hit5_ID, row));
    }
    /// trkID: associated track ID
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trkID, row));
    }
    /// x1: bmt-Z cluster first end point x
    [[nodiscard]] float x1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x1, row));
    }
    /// y1: bmt-Z cluster first end point y
    [[nodiscard]] float y1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y1, row));
    }
    /// z1: bmt-Z cluster first end point z
    [[nodiscard]] float z1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z1, row));
    }
    /// x2: bmt-Z cluster second end point x
    [[nodiscard]] float x2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x2, row));
    }
    /// y2: bmt-Z cluster second end point y
    [[nodiscard]] float y2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y2, row));
    }
    /// z2: bmt-Z cluster second end point z
    [[nodiscard]] float z2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z2, row));
    }
    /// cx: bmt-C cluster arc center x
    [[nodiscard]] float cx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cx, row));
    }
    /// cy: bmt-C cluster arc center y
    [[nodiscard]] float cy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cy, row));
    }
    /// cz: bmt-C cluster arc center z
    [[nodiscard]] float cz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_cz, row));
    }
    /// theta: cluster arc theta
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_theta, row));
    }
    /// ax1: bmt cylinder axis first end point x
    [[nodiscard]] float ax1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ax1, row));
    }
    /// ay1: bmt cylinder axis first end point y
    [[nodiscard]] float ay1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ay1, row));
    }
    /// az1: bmt cylinder axis first end point z
    [[nodiscard]] float az1(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_az1, row));
    }
    /// ax2: bmt cylinder axis second end point x
    [[nodiscard]] float ax2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ax2, row));
    }
    /// ay2: bmt cylinder axis second end point y
    [[nodiscard]] float ay2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ay2, row));
    }
    /// az2: bmt cylinder axis second end point z
    [[nodiscard]] float az2(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_az2, row));
    }
    /// lx: vector x comp  along cluster pseudo-strip direction
    [[nodiscard]] float lx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lx, row));
    }
    /// ly: vector y comp  along cluster pseudo-strip direction
    [[nodiscard]] float ly(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ly, row));
    }
    /// lz: vector z comp  along cluster pseudo-strip direction
    [[nodiscard]] float lz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lz, row));
    }
    /// sx: vector x comp perpendicular to cluster pseudo-strip direction in the module plane
    [[nodiscard]] float sx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_sx, row));
    }
    /// sy: vector y comp perpendicular to cluster pseudo-strip direction in the module plane
    [[nodiscard]] float sy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_sy, row));
    }
    /// sz: vector z comp perpendicular to cluster pseudo-strip direction in the module plane
    [[nodiscard]] float sz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_sz, row));
    }
    /// nx: vector x comp normal to the cluster module plane
    [[nodiscard]] float nx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_nx, row));
    }
    /// ny: vector y comp normal to the cluster module plane
    [[nodiscard]] float ny(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ny, row));
    }
    /// nz: vector z comp normal to the cluster module plane
    [[nodiscard]] float nz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_nz, row));
    }
    /// e: cluster spacial uncertainty
    [[nodiscard]] float e(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_e, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ID_col() const noexcept {
        return m_view.column<int16_t>(h_ID);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// ETot column as a contiguous span.
    [[nodiscard]] std::span<const float> ETot_col() const noexcept {
        return m_view.column<float>(h_ETot);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// seedE column as a contiguous span.
    [[nodiscard]] std::span<const float> seedE_col() const noexcept {
        return m_view.column<float>(h_seedE);
    }
    /// seedStrip column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> seedStrip_col() const noexcept {
        return m_view.column<int16_t>(h_seedStrip);
    }
    /// centroid column as a contiguous span.
    [[nodiscard]] std::span<const float> centroid_col() const noexcept {
        return m_view.column<float>(h_centroid);
    }
    /// centroidValue column as a contiguous span.
    [[nodiscard]] std::span<const float> centroidValue_col() const noexcept {
        return m_view.column<float>(h_centroidValue);
    }
    /// centroidError column as a contiguous span.
    [[nodiscard]] std::span<const float> centroidError_col() const noexcept {
        return m_view.column<float>(h_centroidError);
    }
    /// centroidResidual column as a contiguous span.
    [[nodiscard]] std::span<const float> centroidResidual_col() const noexcept {
        return m_view.column<float>(h_centroidResidual);
    }
    /// seedResidual column as a contiguous span.
    [[nodiscard]] std::span<const float> seedResidual_col() const noexcept {
        return m_view.column<float>(h_seedResidual);
    }
    /// Hit1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit1_ID);
    }
    /// Hit2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit2_ID);
    }
    /// Hit3_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit3_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit3_ID);
    }
    /// Hit4_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit4_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit4_ID);
    }
    /// Hit5_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Hit5_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Hit5_ID);
    }
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trkID_col() const noexcept {
        return m_view.column<int16_t>(h_trkID);
    }
    /// x1 column as a contiguous span.
    [[nodiscard]] std::span<const float> x1_col() const noexcept {
        return m_view.column<float>(h_x1);
    }
    /// y1 column as a contiguous span.
    [[nodiscard]] std::span<const float> y1_col() const noexcept {
        return m_view.column<float>(h_y1);
    }
    /// z1 column as a contiguous span.
    [[nodiscard]] std::span<const float> z1_col() const noexcept {
        return m_view.column<float>(h_z1);
    }
    /// x2 column as a contiguous span.
    [[nodiscard]] std::span<const float> x2_col() const noexcept {
        return m_view.column<float>(h_x2);
    }
    /// y2 column as a contiguous span.
    [[nodiscard]] std::span<const float> y2_col() const noexcept {
        return m_view.column<float>(h_y2);
    }
    /// z2 column as a contiguous span.
    [[nodiscard]] std::span<const float> z2_col() const noexcept {
        return m_view.column<float>(h_z2);
    }
    /// cx column as a contiguous span.
    [[nodiscard]] std::span<const float> cx_col() const noexcept {
        return m_view.column<float>(h_cx);
    }
    /// cy column as a contiguous span.
    [[nodiscard]] std::span<const float> cy_col() const noexcept {
        return m_view.column<float>(h_cy);
    }
    /// cz column as a contiguous span.
    [[nodiscard]] std::span<const float> cz_col() const noexcept {
        return m_view.column<float>(h_cz);
    }
    /// theta column as a contiguous span.
    [[nodiscard]] std::span<const float> theta_col() const noexcept {
        return m_view.column<float>(h_theta);
    }
    /// ax1 column as a contiguous span.
    [[nodiscard]] std::span<const float> ax1_col() const noexcept {
        return m_view.column<float>(h_ax1);
    }
    /// ay1 column as a contiguous span.
    [[nodiscard]] std::span<const float> ay1_col() const noexcept {
        return m_view.column<float>(h_ay1);
    }
    /// az1 column as a contiguous span.
    [[nodiscard]] std::span<const float> az1_col() const noexcept {
        return m_view.column<float>(h_az1);
    }
    /// ax2 column as a contiguous span.
    [[nodiscard]] std::span<const float> ax2_col() const noexcept {
        return m_view.column<float>(h_ax2);
    }
    /// ay2 column as a contiguous span.
    [[nodiscard]] std::span<const float> ay2_col() const noexcept {
        return m_view.column<float>(h_ay2);
    }
    /// az2 column as a contiguous span.
    [[nodiscard]] std::span<const float> az2_col() const noexcept {
        return m_view.column<float>(h_az2);
    }
    /// lx column as a contiguous span.
    [[nodiscard]] std::span<const float> lx_col() const noexcept {
        return m_view.column<float>(h_lx);
    }
    /// ly column as a contiguous span.
    [[nodiscard]] std::span<const float> ly_col() const noexcept {
        return m_view.column<float>(h_ly);
    }
    /// lz column as a contiguous span.
    [[nodiscard]] std::span<const float> lz_col() const noexcept {
        return m_view.column<float>(h_lz);
    }
    /// sx column as a contiguous span.
    [[nodiscard]] std::span<const float> sx_col() const noexcept {
        return m_view.column<float>(h_sx);
    }
    /// sy column as a contiguous span.
    [[nodiscard]] std::span<const float> sy_col() const noexcept {
        return m_view.column<float>(h_sy);
    }
    /// sz column as a contiguous span.
    [[nodiscard]] std::span<const float> sz_col() const noexcept {
        return m_view.column<float>(h_sz);
    }
    /// nx column as a contiguous span.
    [[nodiscard]] std::span<const float> nx_col() const noexcept {
        return m_view.column<float>(h_nx);
    }
    /// ny column as a contiguous span.
    [[nodiscard]] std::span<const float> ny_col() const noexcept {
        return m_view.column<float>(h_ny);
    }
    /// nz column as a contiguous span.
    [[nodiscard]] std::span<const float> nz_col() const noexcept {
        return m_view.column<float>(h_nz);
    }
    /// e column as a contiguous span.
    [[nodiscard]] std::span<const float> e_col() const noexcept {
        return m_view.column<float>(h_e);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_ID{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_size{};
    hipo::column_handle h_ETot{};
    hipo::column_handle h_time{};
    hipo::column_handle h_seedE{};
    hipo::column_handle h_seedStrip{};
    hipo::column_handle h_centroid{};
    hipo::column_handle h_centroidValue{};
    hipo::column_handle h_centroidError{};
    hipo::column_handle h_centroidResidual{};
    hipo::column_handle h_seedResidual{};
    hipo::column_handle h_Hit1_ID{};
    hipo::column_handle h_Hit2_ID{};
    hipo::column_handle h_Hit3_ID{};
    hipo::column_handle h_Hit4_ID{};
    hipo::column_handle h_Hit5_ID{};
    hipo::column_handle h_trkID{};
    hipo::column_handle h_x1{};
    hipo::column_handle h_y1{};
    hipo::column_handle h_z1{};
    hipo::column_handle h_x2{};
    hipo::column_handle h_y2{};
    hipo::column_handle h_z2{};
    hipo::column_handle h_cx{};
    hipo::column_handle h_cy{};
    hipo::column_handle h_cz{};
    hipo::column_handle h_theta{};
    hipo::column_handle h_ax1{};
    hipo::column_handle h_ay1{};
    hipo::column_handle h_az1{};
    hipo::column_handle h_ax2{};
    hipo::column_handle h_ay2{};
    hipo::column_handle h_az2{};
    hipo::column_handle h_lx{};
    hipo::column_handle h_ly{};
    hipo::column_handle h_lz{};
    hipo::column_handle h_sx{};
    hipo::column_handle h_sy{};
    hipo::column_handle h_sz{};
    hipo::column_handle h_nx{};
    hipo::column_handle h_ny{};
    hipo::column_handle h_nz{};
    hipo::column_handle h_e{};

    friend class Row;
};

/// Lightweight row view for BMTRec::Clusters. Stores a pointer + row index only.
class BMTRec_Clusters::Row {
 public:
    Row(const BMTRec_Clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int ID() const noexcept { return m_bank->ID(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] float ETot() const noexcept { return m_bank->ETot(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float seedE() const noexcept { return m_bank->seedE(m_row); }
    [[nodiscard]] int seedStrip() const noexcept { return m_bank->seedStrip(m_row); }
    [[nodiscard]] float centroid() const noexcept { return m_bank->centroid(m_row); }
    [[nodiscard]] float centroidValue() const noexcept { return m_bank->centroidValue(m_row); }
    [[nodiscard]] float centroidError() const noexcept { return m_bank->centroidError(m_row); }
    [[nodiscard]] float centroidResidual() const noexcept { return m_bank->centroidResidual(m_row); }
    [[nodiscard]] float seedResidual() const noexcept { return m_bank->seedResidual(m_row); }
    [[nodiscard]] int Hit1_ID() const noexcept { return m_bank->Hit1_ID(m_row); }
    [[nodiscard]] int Hit2_ID() const noexcept { return m_bank->Hit2_ID(m_row); }
    [[nodiscard]] int Hit3_ID() const noexcept { return m_bank->Hit3_ID(m_row); }
    [[nodiscard]] int Hit4_ID() const noexcept { return m_bank->Hit4_ID(m_row); }
    [[nodiscard]] int Hit5_ID() const noexcept { return m_bank->Hit5_ID(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] float x1() const noexcept { return m_bank->x1(m_row); }
    [[nodiscard]] float y1() const noexcept { return m_bank->y1(m_row); }
    [[nodiscard]] float z1() const noexcept { return m_bank->z1(m_row); }
    [[nodiscard]] float x2() const noexcept { return m_bank->x2(m_row); }
    [[nodiscard]] float y2() const noexcept { return m_bank->y2(m_row); }
    [[nodiscard]] float z2() const noexcept { return m_bank->z2(m_row); }
    [[nodiscard]] float cx() const noexcept { return m_bank->cx(m_row); }
    [[nodiscard]] float cy() const noexcept { return m_bank->cy(m_row); }
    [[nodiscard]] float cz() const noexcept { return m_bank->cz(m_row); }
    [[nodiscard]] float theta() const noexcept { return m_bank->theta(m_row); }
    [[nodiscard]] float ax1() const noexcept { return m_bank->ax1(m_row); }
    [[nodiscard]] float ay1() const noexcept { return m_bank->ay1(m_row); }
    [[nodiscard]] float az1() const noexcept { return m_bank->az1(m_row); }
    [[nodiscard]] float ax2() const noexcept { return m_bank->ax2(m_row); }
    [[nodiscard]] float ay2() const noexcept { return m_bank->ay2(m_row); }
    [[nodiscard]] float az2() const noexcept { return m_bank->az2(m_row); }
    [[nodiscard]] float lx() const noexcept { return m_bank->lx(m_row); }
    [[nodiscard]] float ly() const noexcept { return m_bank->ly(m_row); }
    [[nodiscard]] float lz() const noexcept { return m_bank->lz(m_row); }
    [[nodiscard]] float sx() const noexcept { return m_bank->sx(m_row); }
    [[nodiscard]] float sy() const noexcept { return m_bank->sy(m_row); }
    [[nodiscard]] float sz() const noexcept { return m_bank->sz(m_row); }
    [[nodiscard]] float nx() const noexcept { return m_bank->nx(m_row); }
    [[nodiscard]] float ny() const noexcept { return m_bank->ny(m_row); }
    [[nodiscard]] float nz() const noexcept { return m_bank->nz(m_row); }
    [[nodiscard]] float e() const noexcept { return m_bank->e(m_row); }
 private:
    const BMTRec_Clusters* m_bank;
    int m_row;
};

inline BMTRec_Clusters::Row BMTRec_Clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
