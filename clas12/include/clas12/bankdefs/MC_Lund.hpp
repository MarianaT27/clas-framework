// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
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


/// HIPO bank wrapper for `MC::Lund` (group 40, item 3).
/// Generated Particle information from Lund
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MC_Lund bank;
/// if (auto sc = bank.bind(event.get_bank("MC::Lund")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class MC_Lund {
 public:
    static constexpr std::string_view bank_name = "MC::Lund";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 3;

    class Row;

    MC_Lund() = default;

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
            hipo::column_handle h_lifetime;
            hipo::column_handle h_type;
            hipo::column_handle h_pid;
            hipo::column_handle h_parent;
            hipo::column_handle h_daughter;
            hipo::column_handle h_px;
            hipo::column_handle h_py;
            hipo::column_handle h_pz;
            hipo::column_handle h_energy;
            hipo::column_handle h_mass;
            hipo::column_handle h_vx;
            hipo::column_handle h_vy;
            hipo::column_handle h_vz;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("index"); h) cache.h_index = *h;
            else cache.h_index = {};
            if (auto h = schema.get_handle("lifetime"); h) cache.h_lifetime = *h;
            else cache.h_lifetime = {};
            if (auto h = schema.get_handle("type"); h) cache.h_type = *h;
            else cache.h_type = {};
            if (auto h = schema.get_handle("pid"); h) cache.h_pid = *h;
            else cache.h_pid = {};
            if (auto h = schema.get_handle("parent"); h) cache.h_parent = *h;
            else cache.h_parent = {};
            if (auto h = schema.get_handle("daughter"); h) cache.h_daughter = *h;
            else cache.h_daughter = {};
            if (auto h = schema.get_handle("px"); h) cache.h_px = *h;
            else cache.h_px = {};
            if (auto h = schema.get_handle("py"); h) cache.h_py = *h;
            else cache.h_py = {};
            if (auto h = schema.get_handle("pz"); h) cache.h_pz = *h;
            else cache.h_pz = {};
            if (auto h = schema.get_handle("energy"); h) cache.h_energy = *h;
            else cache.h_energy = {};
            if (auto h = schema.get_handle("mass"); h) cache.h_mass = *h;
            else cache.h_mass = {};
            if (auto h = schema.get_handle("vx"); h) cache.h_vx = *h;
            else cache.h_vx = {};
            if (auto h = schema.get_handle("vy"); h) cache.h_vy = *h;
            else cache.h_vy = {};
            if (auto h = schema.get_handle("vz"); h) cache.h_vz = *h;
            else cache.h_vz = {};
        }
        h_index = cache.h_index;
        h_lifetime = cache.h_lifetime;
        h_type = cache.h_type;
        h_pid = cache.h_pid;
        h_parent = cache.h_parent;
        h_daughter = cache.h_daughter;
        h_px = cache.h_px;
        h_py = cache.h_py;
        h_pz = cache.h_pz;
        h_energy = cache.h_energy;
        h_mass = cache.h_mass;
        h_vx = cache.h_vx;
        h_vy = cache.h_vy;
        h_vz = cache.h_vz;
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
    /// index: index
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_index, row));
    }
    /// lifetime: particle lifetime (ns) (User Defined)
    [[nodiscard]] float lifetime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_lifetime, row));
    }
    /// type: particle type (1 is active)
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_type, row));
    }
    /// pid: particle id
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pid, row));
    }
    /// parent: index of the parent (User Defined)
    [[nodiscard]] int parent(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_parent, row));
    }
    /// daughter: index of the first daughter (User Defined)
    [[nodiscard]] int daughter(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_daughter, row));
    }
    /// px: x component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_px, row));
    }
    /// py: y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_py, row));
    }
    /// pz: z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pz, row));
    }
    /// energy: Energy of the particle (GeV) (User Defined)
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_energy, row));
    }
    /// mass: mass of the particle (GeV) (User Defined)
    [[nodiscard]] float mass(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_mass, row));
    }
    /// vx: x component of the vertex (cm)
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vx, row));
    }
    /// vy: y component of the vertex (cm)
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vy, row));
    }
    /// vz: z component of the vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// index column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> index_col() const noexcept {
        return m_view.column<int8_t>(h_index);
    }
    /// lifetime column as a contiguous span.
    [[nodiscard]] std::span<const float> lifetime_col() const noexcept {
        return m_view.column<float>(h_lifetime);
    }
    /// type column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> type_col() const noexcept {
        return m_view.column<int8_t>(h_type);
    }
    /// pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pid_col() const noexcept {
        return m_view.column<int32_t>(h_pid);
    }
    /// parent column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> parent_col() const noexcept {
        return m_view.column<int8_t>(h_parent);
    }
    /// daughter column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> daughter_col() const noexcept {
        return m_view.column<int8_t>(h_daughter);
    }
    /// px column as a contiguous span.
    [[nodiscard]] std::span<const float> px_col() const noexcept {
        return m_view.column<float>(h_px);
    }
    /// py column as a contiguous span.
    [[nodiscard]] std::span<const float> py_col() const noexcept {
        return m_view.column<float>(h_py);
    }
    /// pz column as a contiguous span.
    [[nodiscard]] std::span<const float> pz_col() const noexcept {
        return m_view.column<float>(h_pz);
    }
    /// energy column as a contiguous span.
    [[nodiscard]] std::span<const float> energy_col() const noexcept {
        return m_view.column<float>(h_energy);
    }
    /// mass column as a contiguous span.
    [[nodiscard]] std::span<const float> mass_col() const noexcept {
        return m_view.column<float>(h_mass);
    }
    /// vx column as a contiguous span.
    [[nodiscard]] std::span<const float> vx_col() const noexcept {
        return m_view.column<float>(h_vx);
    }
    /// vy column as a contiguous span.
    [[nodiscard]] std::span<const float> vy_col() const noexcept {
        return m_view.column<float>(h_vy);
    }
    /// vz column as a contiguous span.
    [[nodiscard]] std::span<const float> vz_col() const noexcept {
        return m_view.column<float>(h_vz);
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
    hipo::column_handle h_lifetime{};
    hipo::column_handle h_type{};
    hipo::column_handle h_pid{};
    hipo::column_handle h_parent{};
    hipo::column_handle h_daughter{};
    hipo::column_handle h_px{};
    hipo::column_handle h_py{};
    hipo::column_handle h_pz{};
    hipo::column_handle h_energy{};
    hipo::column_handle h_mass{};
    hipo::column_handle h_vx{};
    hipo::column_handle h_vy{};
    hipo::column_handle h_vz{};

    friend class Row;
};

/// Lightweight row view for MC::Lund. Stores a pointer + row index only.
class MC_Lund::Row {
 public:
    Row(const MC_Lund* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] float lifetime() const noexcept { return m_bank->lifetime(m_row); }
    [[nodiscard]] int type() const noexcept { return m_bank->type(m_row); }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] int parent() const noexcept { return m_bank->parent(m_row); }
    [[nodiscard]] int daughter() const noexcept { return m_bank->daughter(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
    [[nodiscard]] float energy() const noexcept { return m_bank->energy(m_row); }
    [[nodiscard]] float mass() const noexcept { return m_bank->mass(m_row); }
    [[nodiscard]] float vx() const noexcept { return m_bank->vx(m_row); }
    [[nodiscard]] float vy() const noexcept { return m_bank->vy(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
 private:
    const MC_Lund* m_bank;
    int m_row;
};

inline MC_Lund::Row MC_Lund::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
