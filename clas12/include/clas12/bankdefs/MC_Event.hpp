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


/// HIPO bank wrapper for `MC::Event` (group 40, item 1).
/// Lund header bank for the generated event
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MC_Event bank;
/// if (auto sc = bank.bind(event.get_bank("MC::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto npart = row.npart();
/// }
/// @endcode
class MC_Event {
 public:
    static constexpr std::string_view bank_name = "MC::Event";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 1;

    class Row;

    MC_Event() = default;

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
            hipo::column_handle h_npart;
            hipo::column_handle h_atarget;
            hipo::column_handle h_ztarget;
            hipo::column_handle h_ptarget;
            hipo::column_handle h_pbeam;
            hipo::column_handle h_btype;
            hipo::column_handle h_ebeam;
            hipo::column_handle h_targetid;
            hipo::column_handle h_processid;
            hipo::column_handle h_weight;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("npart"); h) cache.h_npart = *h;
            else cache.h_npart = {};
            if (auto h = schema.get_handle("atarget"); h) cache.h_atarget = *h;
            else cache.h_atarget = {};
            if (auto h = schema.get_handle("ztarget"); h) cache.h_ztarget = *h;
            else cache.h_ztarget = {};
            if (auto h = schema.get_handle("ptarget"); h) cache.h_ptarget = *h;
            else cache.h_ptarget = {};
            if (auto h = schema.get_handle("pbeam"); h) cache.h_pbeam = *h;
            else cache.h_pbeam = {};
            if (auto h = schema.get_handle("btype"); h) cache.h_btype = *h;
            else cache.h_btype = {};
            if (auto h = schema.get_handle("ebeam"); h) cache.h_ebeam = *h;
            else cache.h_ebeam = {};
            if (auto h = schema.get_handle("targetid"); h) cache.h_targetid = *h;
            else cache.h_targetid = {};
            if (auto h = schema.get_handle("processid"); h) cache.h_processid = *h;
            else cache.h_processid = {};
            if (auto h = schema.get_handle("weight"); h) cache.h_weight = *h;
            else cache.h_weight = {};
        }
        h_npart = cache.h_npart;
        h_atarget = cache.h_atarget;
        h_ztarget = cache.h_ztarget;
        h_ptarget = cache.h_ptarget;
        h_pbeam = cache.h_pbeam;
        h_btype = cache.h_btype;
        h_ebeam = cache.h_ebeam;
        h_targetid = cache.h_targetid;
        h_processid = cache.h_processid;
        h_weight = cache.h_weight;
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
    /// npart: number of particles in the event (User Defined)
    [[nodiscard]] int npart(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_npart, row));
    }
    /// atarget: Mass number of the target (User Defined)
    [[nodiscard]] int atarget(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_atarget, row));
    }
    /// ztarget: Atomic number oif the target (User Defined)
    [[nodiscard]] int ztarget(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_ztarget, row));
    }
    /// ptarget: Target polarization (User Defined)
    [[nodiscard]] float ptarget(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ptarget, row));
    }
    /// pbeam: Beam polarization
    [[nodiscard]] float pbeam(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_pbeam, row));
    }
    /// btype: Beam type (User Defined)
    [[nodiscard]] int btype(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_btype, row));
    }
    /// ebeam: Beam energy (GeV) (User Defined)
    [[nodiscard]] float ebeam(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_ebeam, row));
    }
    /// targetid: Interacted nucleon ID (proton=2212, neutron=2112 (User Defined)
    [[nodiscard]] int targetid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_targetid, row));
    }
    /// processid: Process ID (User Defined)
    [[nodiscard]] int processid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_processid, row));
    }
    /// weight: Event weight (User Defined)
    [[nodiscard]] float weight(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_weight, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// npart column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> npart_col() const noexcept {
        return m_view.column<int16_t>(h_npart);
    }
    /// atarget column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> atarget_col() const noexcept {
        return m_view.column<int16_t>(h_atarget);
    }
    /// ztarget column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> ztarget_col() const noexcept {
        return m_view.column<int16_t>(h_ztarget);
    }
    /// ptarget column as a contiguous span.
    [[nodiscard]] std::span<const float> ptarget_col() const noexcept {
        return m_view.column<float>(h_ptarget);
    }
    /// pbeam column as a contiguous span.
    [[nodiscard]] std::span<const float> pbeam_col() const noexcept {
        return m_view.column<float>(h_pbeam);
    }
    /// btype column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> btype_col() const noexcept {
        return m_view.column<int16_t>(h_btype);
    }
    /// ebeam column as a contiguous span.
    [[nodiscard]] std::span<const float> ebeam_col() const noexcept {
        return m_view.column<float>(h_ebeam);
    }
    /// targetid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> targetid_col() const noexcept {
        return m_view.column<int16_t>(h_targetid);
    }
    /// processid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> processid_col() const noexcept {
        return m_view.column<int16_t>(h_processid);
    }
    /// weight column as a contiguous span.
    [[nodiscard]] std::span<const float> weight_col() const noexcept {
        return m_view.column<float>(h_weight);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_npart{};
    hipo::column_handle h_atarget{};
    hipo::column_handle h_ztarget{};
    hipo::column_handle h_ptarget{};
    hipo::column_handle h_pbeam{};
    hipo::column_handle h_btype{};
    hipo::column_handle h_ebeam{};
    hipo::column_handle h_targetid{};
    hipo::column_handle h_processid{};
    hipo::column_handle h_weight{};

    friend class Row;
};

/// Lightweight row view for MC::Event. Stores a pointer + row index only.
class MC_Event::Row {
 public:
    Row(const MC_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int npart() const noexcept { return m_bank->npart(m_row); }
    [[nodiscard]] int atarget() const noexcept { return m_bank->atarget(m_row); }
    [[nodiscard]] int ztarget() const noexcept { return m_bank->ztarget(m_row); }
    [[nodiscard]] float ptarget() const noexcept { return m_bank->ptarget(m_row); }
    [[nodiscard]] float pbeam() const noexcept { return m_bank->pbeam(m_row); }
    [[nodiscard]] int btype() const noexcept { return m_bank->btype(m_row); }
    [[nodiscard]] float ebeam() const noexcept { return m_bank->ebeam(m_row); }
    [[nodiscard]] int targetid() const noexcept { return m_bank->targetid(m_row); }
    [[nodiscard]] int processid() const noexcept { return m_bank->processid(m_row); }
    [[nodiscard]] float weight() const noexcept { return m_bank->weight(m_row); }
 private:
    const MC_Event* m_bank;
    int m_row;
};

inline MC_Event::Row MC_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
