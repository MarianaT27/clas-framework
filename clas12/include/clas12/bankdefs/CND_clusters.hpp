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

namespace clas12::bankdefs {


/// HIPO bank wrapper for `CND::clusters` (group 20300, item 22).
/// reconstructed cluster info from CND and CTOF
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CND_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("CND::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CND_clusters {
 public:
    static constexpr std::string_view bank_name = "CND::clusters";
    static constexpr int bank_group = 20300;
    static constexpr int bank_item  = 22;

    class Row;

    CND_clusters() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("layer"); h) h_layer = *h;
        else h_layer = {};
        if (auto h = schema.get_handle("component"); h) h_component = *h;
        else h_component = {};
        if (auto h = schema.get_handle("size"); h) h_size = *h;
        else h_size = {};
        if (auto h = schema.get_handle("energy"); h) h_energy = *h;
        else h_energy = {};
        if (auto h = schema.get_handle("x"); h) h_x = *h;
        else h_x = {};
        if (auto h = schema.get_handle("y"); h) h_y = *h;
        else h_y = {};
        if (auto h = schema.get_handle("z"); h) h_z = *h;
        else h_z = {};
        if (auto h = schema.get_handle("time"); h) h_time = *h;
        else h_time = {};
        if (auto h = schema.get_handle("status"); h) h_status = *h;
        else h_status = {};
        if (auto h = schema.get_handle("layermult"); h) h_layermult = *h;
        else h_layermult = {};
        if (auto h = schema.get_handle("layer1"); h) h_layer1 = *h;
        else h_layer1 = {};
        if (auto h = schema.get_handle("layer2"); h) h_layer2 = *h;
        else h_layer2 = {};
        if (auto h = schema.get_handle("layer3"); h) h_layer3 = *h;
        else h_layer3 = {};
        if (auto h = schema.get_handle("pathLengthThruBar"); h) h_pathLengthThruBar = *h;
        else h_pathLengthThruBar = {};
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
    /// sector: sector of the cluster's seed hit
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer of the cluster's seed hit
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: component of the cluster's seed hit
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// size: number of hits of the cluster
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// energy: Sum of E dep (MeV) of the hits
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// x: center of X coor (cm) of the hits
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: center of Y coor (cm) of the hits
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: center of Z coor (cm) of the hits
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// time: center of time (ns) of the hits
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// status: cluster status
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// layermult: layer multiplicity
    [[nodiscard]] int layermult(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layermult, row));
    }
    /// layer1: firt layer hit
    [[nodiscard]] int layer1(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer1, row));
    }
    /// layer2: second layer hit
    [[nodiscard]] int layer2(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer2, row));
    }
    /// layer3: third layer hit
    [[nodiscard]] int layer3(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer3, row));
    }
    /// pathLengthThruBar: pathlength of the track from the entrance point to the exit point through the cluster bars 
    [[nodiscard]] float pathLengthThruBar(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pathLengthThruBar, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
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
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
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
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// layermult column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layermult_col() const noexcept {
        return m_view.column<int8_t>(h_layermult);
    }
    /// layer1 column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer1_col() const noexcept {
        return m_view.column<int8_t>(h_layer1);
    }
    /// layer2 column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer2_col() const noexcept {
        return m_view.column<int8_t>(h_layer2);
    }
    /// layer3 column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer3_col() const noexcept {
        return m_view.column<int8_t>(h_layer3);
    }
    /// pathLengthThruBar column as a contiguous span.
    [[nodiscard]] std::span<const float> pathLengthThruBar_col() const noexcept {
        return m_view.column<float>(h_pathLengthThruBar);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_size{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_time{};
    hipo::column_handle h_status{};
    hipo::column_handle h_layermult{};
    hipo::column_handle h_layer1{};
    hipo::column_handle h_layer2{};
    hipo::column_handle h_layer3{};
    hipo::column_handle h_pathLengthThruBar{};

    friend class Row;
};

/// Lightweight row view for CND::clusters. Stores a pointer + row index only.
class CND_clusters::Row {
 public:
    Row(const CND_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int layermult() const noexcept { return m_bank->layermult(m_row); }
    [[nodiscard]] int layer1() const noexcept { return m_bank->layer1(m_row); }
    [[nodiscard]] int layer2() const noexcept { return m_bank->layer2(m_row); }
    [[nodiscard]] int layer3() const noexcept { return m_bank->layer3(m_row); }
    [[nodiscard]] float pathLengthThruBar() const noexcept { return m_bank->pathLengthThruBar(m_row); }
 private:
    const CND_clusters* m_bank;
    int m_row;
};

inline CND_clusters::Row CND_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
