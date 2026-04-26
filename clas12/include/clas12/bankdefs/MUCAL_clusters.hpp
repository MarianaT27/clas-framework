// =============================================================================
// Auto-generated from mu.json - do not edit by hand.
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


/// HIPO bank wrapper for `MUCAL::clusters` (group 22800, item 22).
/// Reconstructed Clusters in muCLAS12 calorimeter
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MUCAL_clusters bank;
/// if (auto sc = bank.bind(event.get_bank("MUCAL::clusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto size = row.size();
/// }
/// @endcode
class MUCAL_clusters {
 public:
    static constexpr std::string_view bank_name = "MUCAL::clusters";
    static constexpr int bank_group = 22800;
    static constexpr int bank_item  = 22;

    class Row;

    MUCAL_clusters() = default;

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
            hipo::column_handle h_size;
            hipo::column_handle h_id;
            hipo::column_handle h_x;
            hipo::column_handle h_y;
            hipo::column_handle h_z;
            hipo::column_handle h_widthX;
            hipo::column_handle h_widthY;
            hipo::column_handle h_radius;
            hipo::column_handle h_time;
            hipo::column_handle h_energy;
            hipo::column_handle h_maxEnergy;
            hipo::column_handle h_recEnergy;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("size"); h) cache.h_size = *h;
            else cache.h_size = {};
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("x"); h) cache.h_x = *h;
            else cache.h_x = {};
            if (auto h = schema.get_handle("y"); h) cache.h_y = *h;
            else cache.h_y = {};
            if (auto h = schema.get_handle("z"); h) cache.h_z = *h;
            else cache.h_z = {};
            if (auto h = schema.get_handle("widthX"); h) cache.h_widthX = *h;
            else cache.h_widthX = {};
            if (auto h = schema.get_handle("widthY"); h) cache.h_widthY = *h;
            else cache.h_widthY = {};
            if (auto h = schema.get_handle("radius"); h) cache.h_radius = *h;
            else cache.h_radius = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("maxEnergy"); h) cache.h_maxEnergy = *h;
            else cache.h_maxEnergy = {};
            if (auto h = schema.get_handle("recEnergy"); h) cache.h_recEnergy = *h;
            else cache.h_recEnergy = {};
        }
        h_size = cache.h_size;
        h_id = cache.h_id;
        h_x = cache.h_x;
        h_y = cache.h_y;
        h_z = cache.h_z;
        h_widthX = cache.h_widthX;
        h_widthY = cache.h_widthY;
        h_radius = cache.h_radius;
        h_time = cache.h_time;
        h_energy = cache.h_energy;
        h_maxEnergy = cache.h_maxEnergy;
        h_recEnergy = cache.h_recEnergy;
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
    /// size: Cluster size
    [[nodiscard]] int size(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_size, row));
    }
    /// id: Cluster ID
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// x: Cluster centroid X moment (cm)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_x, row));
    }
    /// y: Cluster centroid Y moment (cm)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_y, row));
    }
    /// z: Cluster centroid Z moment (cm)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_z, row));
    }
    /// widthX: Cluster width in x (cm)
    [[nodiscard]] float widthX(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_widthX, row));
    }
    /// widthY: Cluster width in y (cm)
    [[nodiscard]] float widthY(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_widthY, row));
    }
    /// radius: Cluster radius (cm)
    [[nodiscard]] float radius(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_radius, row));
    }
    /// time: Cluster timing information
    [[nodiscard]] float time(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_time, row));
    }
    /// energy: Cluster total energy
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// maxEnergy: Cluster maximum energy
    [[nodiscard]] float maxEnergy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_maxEnergy, row));
    }
    /// recEnergy: Cluster reconstructed energy
    [[nodiscard]] float recEnergy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recEnergy, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> size_col() const noexcept {
        return m_view.column<int16_t>(h_size);
    }
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
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
    /// widthX column as a contiguous span.
    [[nodiscard]] std::span<const float> widthX_col() const noexcept {
        return m_view.column<float>(h_widthX);
    }
    /// widthY column as a contiguous span.
    [[nodiscard]] std::span<const float> widthY_col() const noexcept {
        return m_view.column<float>(h_widthY);
    }
    /// radius column as a contiguous span.
    [[nodiscard]] std::span<const float> radius_col() const noexcept {
        return m_view.column<float>(h_radius);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const float> time_col() const noexcept {
        return m_view.column<float>(h_time);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// maxEnergy column as a contiguous span.
    [[nodiscard]] std::span<const float> maxEnergy_col() const noexcept {
        return m_view.column<float>(h_maxEnergy);
    }
    /// recEnergy column as a contiguous span.
    [[nodiscard]] std::span<const float> recEnergy_col() const noexcept {
        return m_view.column<float>(h_recEnergy);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_size{};
    hipo::column_handle h_id{};
    hipo::column_handle h_x{};
    hipo::column_handle h_y{};
    hipo::column_handle h_z{};
    hipo::column_handle h_widthX{};
    hipo::column_handle h_widthY{};
    hipo::column_handle h_radius{};
    hipo::column_handle h_time{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_maxEnergy{};
    hipo::column_handle h_recEnergy{};

    friend class Row;
};

/// Lightweight row view for MUCAL::clusters. Stores a pointer + row index only.
class MUCAL_clusters::Row {
 public:
    Row(const MUCAL_clusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int size() const noexcept { return m_bank->size(m_row); }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float widthX() const noexcept { return m_bank->widthX(m_row); }
    [[nodiscard]] float widthY() const noexcept { return m_bank->widthY(m_row); }
    [[nodiscard]] float radius() const noexcept { return m_bank->radius(m_row); }
    [[nodiscard]] float time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float maxEnergy() const noexcept { return m_bank->maxEnergy(m_row); }
    [[nodiscard]] float recEnergy() const noexcept { return m_bank->recEnergy(m_row); }
 private:
    const MUCAL_clusters* m_bank;
    int m_row;
};

inline MUCAL_clusters::Row MUCAL_clusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
