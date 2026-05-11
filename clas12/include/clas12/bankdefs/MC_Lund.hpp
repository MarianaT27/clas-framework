// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `MC::Lund` (group 40, item 3).
/// Generated Particle information from Lund
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
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

    /// Bind to a live hipo::bank. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column name to its item
    /// index via schema.getEntryOrder(); subsequent bind()s for the same
    /// schema hit the cache. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            const hipo::schema* schema_ptr = nullptr;
            int i_index = -1;
            int i_lifetime = -1;
            int i_type = -1;
            int i_pid = -1;
            int i_parent = -1;
            int i_daughter = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
            int i_energy = -1;
            int i_mass = -1;
            int i_vx = -1;
            int i_vy = -1;
            int i_vz = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_lifetime = schema.exists("lifetime") ? schema.getEntryOrder("lifetime") : -1;
            cache.i_type = schema.exists("type") ? schema.getEntryOrder("type") : -1;
            cache.i_pid = schema.exists("pid") ? schema.getEntryOrder("pid") : -1;
            cache.i_parent = schema.exists("parent") ? schema.getEntryOrder("parent") : -1;
            cache.i_daughter = schema.exists("daughter") ? schema.getEntryOrder("daughter") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
            cache.i_energy = schema.exists("energy") ? schema.getEntryOrder("energy") : -1;
            cache.i_mass = schema.exists("mass") ? schema.getEntryOrder("mass") : -1;
            cache.i_vx = schema.exists("vx") ? schema.getEntryOrder("vx") : -1;
            cache.i_vy = schema.exists("vy") ? schema.getEntryOrder("vy") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
        }
        i_index = cache.i_index;
        i_lifetime = cache.i_lifetime;
        i_type = cache.i_type;
        i_pid = cache.i_pid;
        i_parent = cache.i_parent;
        i_daughter = cache.i_daughter;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
        i_energy = cache.i_energy;
        i_mass = cache.i_mass;
        i_vx = cache.i_vx;
        i_vy = cache.i_vy;
        i_vz = cache.i_vz;
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
    /// index: index
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_index, row));
    }
    /// lifetime: particle lifetime (ns) (User Defined)
    [[nodiscard]] float lifetime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_lifetime, row));
    }
    /// type: particle type (1 is active)
    [[nodiscard]] int type(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_type, row));
    }
    /// pid: particle id
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pid, row));
    }
    /// parent: index of the parent (User Defined)
    [[nodiscard]] int parent(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_parent, row));
    }
    /// daughter: index of the first daughter (User Defined)
    [[nodiscard]] int daughter(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_daughter, row));
    }
    /// px: x component of the momentum (GeV)
    [[nodiscard]] float px(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_px, row));
    }
    /// py: y component of the momentum (GeV)
    [[nodiscard]] float py(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_py, row));
    }
    /// pz: z component of the momentum (GeV)
    [[nodiscard]] float pz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_pz, row));
    }
    /// energy: Energy of the particle (GeV) (User Defined)
    [[nodiscard]] float energy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_energy, row));
    }
    /// mass: mass of the particle (GeV) (User Defined)
    [[nodiscard]] float mass(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_mass, row));
    }
    /// vx: x component of the vertex (cm)
    [[nodiscard]] float vx(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vx, row));
    }
    /// vy: y component of the vertex (cm)
    [[nodiscard]] float vy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vy, row));
    }
    /// vz: z component of the vertex (cm)
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int index_handle() const noexcept { return i_index; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_index = -1;
    int i_lifetime = -1;
    int i_type = -1;
    int i_pid = -1;
    int i_parent = -1;
    int i_daughter = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;
    int i_energy = -1;
    int i_mass = -1;
    int i_vx = -1;
    int i_vy = -1;
    int i_vz = -1;

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
