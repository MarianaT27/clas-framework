// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `RECAIFT::Particle` (group 400, item 42).
/// Reconstructed Particle Information, shadowing RECAI::Particle with FT-based start time
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECAIFT_Particle bank;
/// if (auto sc = bank.bind(event.get_bank("RECAIFT::Particle")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto pid = row.pid();
/// }
/// @endcode
class RECAIFT_Particle {
 public:
    static constexpr std::string_view bank_name = "RECAIFT::Particle";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 42;

    class Row;

    RECAIFT_Particle() = default;

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
            int i_pid = -1;
            int i_vt = -1;
            int i_beta = -1;
            int i_chi2pid = -1;
            int i_status = -1;
            int i_px = -1;
            int i_py = -1;
            int i_pz = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_pid = schema.exists("pid") ? schema.getEntryOrder("pid") : -1;
            cache.i_vt = schema.exists("vt") ? schema.getEntryOrder("vt") : -1;
            cache.i_beta = schema.exists("beta") ? schema.getEntryOrder("beta") : -1;
            cache.i_chi2pid = schema.exists("chi2pid") ? schema.getEntryOrder("chi2pid") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_px = schema.exists("px") ? schema.getEntryOrder("px") : -1;
            cache.i_py = schema.exists("py") ? schema.getEntryOrder("py") : -1;
            cache.i_pz = schema.exists("pz") ? schema.getEntryOrder("pz") : -1;
        }
        i_pid = cache.i_pid;
        i_vt = cache.i_vt;
        i_beta = cache.i_beta;
        i_chi2pid = cache.i_chi2pid;
        i_status = cache.i_status;
        i_px = cache.i_px;
        i_py = cache.i_py;
        i_pz = cache.i_pz;
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
    /// pid: particle id in LUND conventions
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pid, row));
    }
    /// vt: RF and z corrected vertex time (ns)
    [[nodiscard]] float vt(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vt, row));
    }
    /// beta: particle beta measured by TOF
    [[nodiscard]] float beta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_beta, row));
    }
    /// chi2pid: Chi2 of assigned PID
    [[nodiscard]] float chi2pid(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_chi2pid, row));
    }
    /// status: particle status (represents detector collection it passed)
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
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

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_pid = -1;
    int i_vt = -1;
    int i_beta = -1;
    int i_chi2pid = -1;
    int i_status = -1;
    int i_px = -1;
    int i_py = -1;
    int i_pz = -1;

    friend class Row;
};

/// Lightweight row view for RECAIFT::Particle. Stores a pointer + row index only.
class RECAIFT_Particle::Row {
 public:
    Row(const RECAIFT_Particle* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] float vt() const noexcept { return m_bank->vt(m_row); }
    [[nodiscard]] float beta() const noexcept { return m_bank->beta(m_row); }
    [[nodiscard]] float chi2pid() const noexcept { return m_bank->chi2pid(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] float px() const noexcept { return m_bank->px(m_row); }
    [[nodiscard]] float py() const noexcept { return m_bank->py(m_row); }
    [[nodiscard]] float pz() const noexcept { return m_bank->pz(m_row); }
 private:
    const RECAIFT_Particle* m_bank;
    int m_row;
};

inline RECAIFT_Particle::Row RECAIFT_Particle::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
