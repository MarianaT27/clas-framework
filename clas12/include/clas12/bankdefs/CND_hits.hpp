// =============================================================================
// Auto-generated from cnd.json - do not edit by hand.
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


/// HIPO bank wrapper for `CND::hits` (group 20300, item 21).
/// reconstructed hit info from CND
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CND_hits bank;
/// if (auto sc = bank.bind(event.get_bank("CND::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CND_hits {
 public:
    static constexpr std::string_view bank_name = "CND::hits";
    static constexpr int bank_group = 20300;
    static constexpr int bank_item  = 21;

    class Row;

    CND_hits() = default;

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
            hipo::column_handle h_status;
            hipo::column_handle h_trkID;
            hipo::column_handle h_clusterid;
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_component;
            hipo::column_handle h_energy;
            hipo::column_handle h_time;
            hipo::column_handle h_energy_unc;
            hipo::column_handle h_time_unc;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_x_unc;
            hipo::column_handle h_y_unc;
            hipo::column_handle h_z_unc;
            hipo::column_handle h_tx;
            hipo::column_handle h_ty;
            hipo::column_handle h_tz;
            hipo::column_handle h_tlength;
            hipo::column_handle h_pathlength;
            hipo::column_handle h_indexLadc;
            hipo::column_handle h_indexRadc;
            hipo::column_handle h_indexLtdc;
            hipo::column_handle h_indexRtdc;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("trkID"); h) cache.h_trkID = *h;
            else cache.h_trkID = {};
            if (auto h = schema.get_handle("clusterid"); h) cache.h_clusterid = *h;
            else cache.h_clusterid = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("component"); h) cache.h_component = *h;
            else cache.h_component = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("energy_unc"); h) cache.h_energy_unc = *h;
            else cache.h_energy_unc = {};
            if (auto h = schema.get_handle("time_unc"); h) cache.h_time_unc = *h;
            else cache.h_time_unc = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("x_unc"); h) cache.h_x_unc = *h;
            else cache.h_x_unc = {};
            if (auto h = schema.get_handle("y_unc"); h) cache.h_y_unc = *h;
            else cache.h_y_unc = {};
            if (auto h = schema.get_handle("z_unc"); h) cache.h_z_unc = *h;
            else cache.h_z_unc = {};
            if (auto h = schema.get_handle("tx"); h) cache.h_tx = *h;
            else cache.h_tx = {};
            if (auto h = schema.get_handle("ty"); h) cache.h_ty = *h;
            else cache.h_ty = {};
            if (auto h = schema.get_handle("tz"); h) cache.h_tz = *h;
            else cache.h_tz = {};
            if (auto h = schema.get_handle("tlength"); h) cache.h_tlength = *h;
            else cache.h_tlength = {};
            if (auto h = schema.get_handle("pathlength"); h) cache.h_pathlength = *h;
            else cache.h_pathlength = {};
            if (auto h = schema.get_handle("indexLadc"); h) cache.h_indexLadc = *h;
            else cache.h_indexLadc = {};
            if (auto h = schema.get_handle("indexRadc"); h) cache.h_indexRadc = *h;
            else cache.h_indexRadc = {};
            if (auto h = schema.get_handle("indexLtdc"); h) cache.h_indexLtdc = *h;
            else cache.h_indexLtdc = {};
            if (auto h = schema.get_handle("indexRtdc"); h) cache.h_indexRtdc = *h;
            else cache.h_indexRtdc = {};
        }
        h_id = cache.h_id;
        h_status = cache.h_status;
        h_trkID = cache.h_trkID;
        h_clusterid = cache.h_clusterid;
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_energy = cache.h_energy;
        h_time = cache.h_time;
        h_energy_unc = cache.h_energy_unc;
        h_time_unc = cache.h_time_unc;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_x_unc = cache.h_x_unc;
        h_y_unc = cache.h_y_unc;
        h_z_unc = cache.h_z_unc;
        h_tx = cache.h_tx;
        h_ty = cache.h_ty;
        h_tz = cache.h_tz;
        h_tlength = cache.h_tlength;
        h_pathlength = cache.h_pathlength;
        h_indexLadc = cache.h_indexLadc;
        h_indexRadc = cache.h_indexRadc;
        h_indexLtdc = cache.h_indexLtdc;
        h_indexRtdc = cache.h_indexRtdc;
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
    /// status: status of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// trkID: match CVT track index
    [[nodiscard]] int trkID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_trkID, row));
    }
    /// clusterid: pointer to the cluster
    [[nodiscard]] int clusterid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusterid, row));
    }
    /// sector: sector of CND
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: panel id of CND
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: paddle id of CND
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// energy: E dep (MeV) of hit
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// time: Hit time (ns)
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// energy_unc: E dep unc (MeV) of hit
    [[nodiscard]] float energy_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy_unc, row));
    }
    /// time_unc: Hit time unc (ns)
    [[nodiscard]] float time_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time_unc, row));
    }
    /// x: Global X coor (cm) of hit
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Global Y coor (cm) of hit
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Global Z coor (cm) of hit
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// x_unc: Global X coor unc (cm) of hit
    [[nodiscard]] float x_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x_unc, row));
    }
    /// y_unc: Global Y coor unc (cm) of hit
    [[nodiscard]] float y_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y_unc, row));
    }
    /// z_unc: Global Z coor unc (cm) of hit
    [[nodiscard]] float z_unc(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z_unc, row));
    }
    /// tx: Global X coor (cm) of hit from CVT info -trkID index
    [[nodiscard]] float tx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tx, row));
    }
    /// ty: Global Y coor (cm) of hit from CVT info - trkID index
    [[nodiscard]] float ty(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ty, row));
    }
    /// tz: Global Z coor (cm) of hit from CVT info - trkID index
    [[nodiscard]] float tz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tz, row));
    }
    /// tlength: pathlength of the track from the entrance point to the exit point through the hit bar
    [[nodiscard]] float tlength(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tlength, row));
    }
    /// pathlength: pathlength of the track from the vertex (doca point to the beamline to the midpoint between the entrance and exit of the hit bar
    [[nodiscard]] float pathlength(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathlength, row));
    }
    /// indexLadc: index of the left adc
    [[nodiscard]] int indexLadc(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexLadc, row));
    }
    /// indexRadc: index of the right adc
    [[nodiscard]] int indexRadc(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexRadc, row));
    }
    /// indexLtdc: index of the left tdc
    [[nodiscard]] int indexLtdc(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexLtdc, row));
    }
    /// indexRtdc: index of the right tdc
    [[nodiscard]] int indexRtdc(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexRtdc, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// trkID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> trkID_col() const noexcept {
        return m_view.column<int16_t>(h_trkID);
    }
    /// clusterid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusterid_col() const noexcept {
        return m_view.column<int16_t>(h_clusterid);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// component column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> component_col() const noexcept {
        return m_view.column<int16_t>(h_component);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// energy_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_unc_col() const noexcept {
        return m_view.column<float>(h_energy_unc);
    }
    /// time_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> time_unc_col() const noexcept {
        return m_view.column<float>(h_time_unc);
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
    /// x_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> x_unc_col() const noexcept {
        return m_view.column<float>(h_x_unc);
    }
    /// y_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> y_unc_col() const noexcept {
        return m_view.column<float>(h_y_unc);
    }
    /// z_unc column as a contiguous span.
    [[nodiscard]] std::span<const float> z_unc_col() const noexcept {
        return m_view.column<float>(h_z_unc);
    }
    /// tx column as a contiguous span.
    [[nodiscard]] std::span<const float> tx_col() const noexcept {
        return m_view.column<float>(h_tx);
    }
    /// ty column as a contiguous span.
    [[nodiscard]] std::span<const float> ty_col() const noexcept {
        return m_view.column<float>(h_ty);
    }
    /// tz column as a contiguous span.
    [[nodiscard]] std::span<const float> tz_col() const noexcept {
        return m_view.column<float>(h_tz);
    }
    /// tlength column as a contiguous span.
    [[nodiscard]] std::span<const float> tlength_col() const noexcept {
        return m_view.column<float>(h_tlength);
    }
    /// pathlength column as a contiguous span.
    [[nodiscard]] std::span<const float> pathlength_col() const noexcept {
        return m_view.column<float>(h_pathlength);
    }
    /// indexLadc column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexLadc_col() const noexcept {
        return m_view.column<int16_t>(h_indexLadc);
    }
    /// indexRadc column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexRadc_col() const noexcept {
        return m_view.column<int16_t>(h_indexRadc);
    }
    /// indexLtdc column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexLtdc_col() const noexcept {
        return m_view.column<int16_t>(h_indexLtdc);
    }
    /// indexRtdc column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexRtdc_col() const noexcept {
        return m_view.column<int16_t>(h_indexRtdc);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_status{};
    hipo::column_handle h_trkID{};
    hipo::column_handle h_clusterid{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_time{};
    hipo::column_handle h_energy_unc{};
    hipo::column_handle h_time_unc{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_x_unc{};
    hipo::column_handle h_y_unc{};
    hipo::column_handle h_z_unc{};
    hipo::column_handle h_tx{};
    hipo::column_handle h_ty{};
    hipo::column_handle h_tz{};
    hipo::column_handle h_tlength{};
    hipo::column_handle h_pathlength{};
    hipo::column_handle h_indexLadc{};
    hipo::column_handle h_indexRadc{};
    hipo::column_handle h_indexLtdc{};
    hipo::column_handle h_indexRtdc{};

    friend class Row;
};

/// Lightweight row view for CND::hits. Stores a pointer + row index only.
class CND_hits::Row {
 public:
    Row(const CND_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int trkID() const noexcept { return m_bank->trkID(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float energy_unc() const noexcept { return m_bank->energy_unc(m_row); }
    [[nodiscard]] float time_unc() const noexcept { return m_bank->time_unc(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float x_unc() const noexcept { return m_bank->x_unc(m_row); }
    [[nodiscard]] float y_unc() const noexcept { return m_bank->y_unc(m_row); }
    [[nodiscard]] float z_unc() const noexcept { return m_bank->z_unc(m_row); }
    [[nodiscard]] float tx() const noexcept { return m_bank->tx(m_row); }
    [[nodiscard]] float ty() const noexcept { return m_bank->ty(m_row); }
    [[nodiscard]] float tz() const noexcept { return m_bank->tz(m_row); }
    [[nodiscard]] float tlength() const noexcept { return m_bank->tlength(m_row); }
    [[nodiscard]] float pathlength() const noexcept { return m_bank->pathlength(m_row); }
    [[nodiscard]] int indexLadc() const noexcept { return m_bank->indexLadc(m_row); }
    [[nodiscard]] int indexRadc() const noexcept { return m_bank->indexRadc(m_row); }
    [[nodiscard]] int indexLtdc() const noexcept { return m_bank->indexLtdc(m_row); }
    [[nodiscard]] int indexRtdc() const noexcept { return m_bank->indexRtdc(m_row); }
 private:
    const CND_hits* m_bank;
    int m_row;
};

inline CND_hits::Row CND_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
