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


/// HIPO bank wrapper for `RECAI::CovMat` (group 400, item 38).
/// reconstructed track covariance matrix
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECAI_CovMat bank;
/// if (auto sc = bank.bind(event.get_bank("RECAI::CovMat")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class RECAI_CovMat {
 public:
    static constexpr std::string_view bank_name = "RECAI::CovMat";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 38;

    class Row;

    RECAI_CovMat() = default;

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
            int i_index = -1;
            int i_pindex = -1;
            int i_C11 = -1;
            int i_C12 = -1;
            int i_C13 = -1;
            int i_C14 = -1;
            int i_C15 = -1;
            int i_C16 = -1;
            int i_C22 = -1;
            int i_C23 = -1;
            int i_C24 = -1;
            int i_C25 = -1;
            int i_C26 = -1;
            int i_C33 = -1;
            int i_C34 = -1;
            int i_C35 = -1;
            int i_C36 = -1;
            int i_C44 = -1;
            int i_C45 = -1;
            int i_C46 = -1;
            int i_C55 = -1;
            int i_C56 = -1;
            int i_C66 = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_index = schema.exists("index") ? schema.getEntryOrder("index") : -1;
            cache.i_pindex = schema.exists("pindex") ? schema.getEntryOrder("pindex") : -1;
            cache.i_C11 = schema.exists("C11") ? schema.getEntryOrder("C11") : -1;
            cache.i_C12 = schema.exists("C12") ? schema.getEntryOrder("C12") : -1;
            cache.i_C13 = schema.exists("C13") ? schema.getEntryOrder("C13") : -1;
            cache.i_C14 = schema.exists("C14") ? schema.getEntryOrder("C14") : -1;
            cache.i_C15 = schema.exists("C15") ? schema.getEntryOrder("C15") : -1;
            cache.i_C16 = schema.exists("C16") ? schema.getEntryOrder("C16") : -1;
            cache.i_C22 = schema.exists("C22") ? schema.getEntryOrder("C22") : -1;
            cache.i_C23 = schema.exists("C23") ? schema.getEntryOrder("C23") : -1;
            cache.i_C24 = schema.exists("C24") ? schema.getEntryOrder("C24") : -1;
            cache.i_C25 = schema.exists("C25") ? schema.getEntryOrder("C25") : -1;
            cache.i_C26 = schema.exists("C26") ? schema.getEntryOrder("C26") : -1;
            cache.i_C33 = schema.exists("C33") ? schema.getEntryOrder("C33") : -1;
            cache.i_C34 = schema.exists("C34") ? schema.getEntryOrder("C34") : -1;
            cache.i_C35 = schema.exists("C35") ? schema.getEntryOrder("C35") : -1;
            cache.i_C36 = schema.exists("C36") ? schema.getEntryOrder("C36") : -1;
            cache.i_C44 = schema.exists("C44") ? schema.getEntryOrder("C44") : -1;
            cache.i_C45 = schema.exists("C45") ? schema.getEntryOrder("C45") : -1;
            cache.i_C46 = schema.exists("C46") ? schema.getEntryOrder("C46") : -1;
            cache.i_C55 = schema.exists("C55") ? schema.getEntryOrder("C55") : -1;
            cache.i_C56 = schema.exists("C56") ? schema.getEntryOrder("C56") : -1;
            cache.i_C66 = schema.exists("C66") ? schema.getEntryOrder("C66") : -1;
        }
        i_index = cache.i_index;
        i_pindex = cache.i_pindex;
        i_C11 = cache.i_C11;
        i_C12 = cache.i_C12;
        i_C13 = cache.i_C13;
        i_C14 = cache.i_C14;
        i_C15 = cache.i_C15;
        i_C16 = cache.i_C16;
        i_C22 = cache.i_C22;
        i_C23 = cache.i_C23;
        i_C24 = cache.i_C24;
        i_C25 = cache.i_C25;
        i_C26 = cache.i_C26;
        i_C33 = cache.i_C33;
        i_C34 = cache.i_C34;
        i_C35 = cache.i_C35;
        i_C36 = cache.i_C36;
        i_C44 = cache.i_C44;
        i_C45 = cache.i_C45;
        i_C46 = cache.i_C46;
        i_C55 = cache.i_C55;
        i_C56 = cache.i_C56;
        i_C66 = cache.i_C66;
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
    /// index: index of the track in Tracks bank
    [[nodiscard]] int index(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_index, row));
    }
    /// pindex: row number in the particle bank hit is associated with
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_pindex, row));
    }
    /// C11: var(x) (cm^2)
    [[nodiscard]] float C11(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C11, row));
    }
    /// C12: cov(x,y)
    [[nodiscard]] float C12(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C12, row));
    }
    /// C13: cov(x,z)
    [[nodiscard]] float C13(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C13, row));
    }
    /// C14: cov(x,theta)
    [[nodiscard]] float C14(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C14, row));
    }
    /// C15: cov(x,phi)
    [[nodiscard]] float C15(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C15, row));
    }
    /// C16: cov(x,p)
    [[nodiscard]] float C16(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C16, row));
    }
    /// C22: var(y) (cm^2)
    [[nodiscard]] float C22(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C22, row));
    }
    /// C23: cov(y,z)
    [[nodiscard]] float C23(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C23, row));
    }
    /// C24: cov(y,theta)
    [[nodiscard]] float C24(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C24, row));
    }
    /// C25: cov(y,phi)
    [[nodiscard]] float C25(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C25, row));
    }
    /// C26: cov(y,p)
    [[nodiscard]] float C26(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C26, row));
    }
    /// C33: var(z) (cm^2)
    [[nodiscard]] float C33(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C33, row));
    }
    /// C34: cov(z,theta)
    [[nodiscard]] float C34(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C34, row));
    }
    /// C35: cov(z,phi)
    [[nodiscard]] float C35(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C35, row));
    }
    /// C36: cov(z,p)
    [[nodiscard]] float C36(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C36, row));
    }
    /// C44: var(theta) (rad^2)
    [[nodiscard]] float C44(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C44, row));
    }
    /// C45: cov(theta,phi)
    [[nodiscard]] float C45(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C45, row));
    }
    /// C46: cov(theta,p)
    [[nodiscard]] float C46(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C46, row));
    }
    /// C55: var(phi) (rad^2)
    [[nodiscard]] float C55(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C55, row));
    }
    /// C56: cov(phi,p)
    [[nodiscard]] float C56(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C56, row));
    }
    /// C66: var(p) (GeV^2)
    [[nodiscard]] float C66(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C66, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column item indices (used by PindexLinker to scan the
    // pindex/index column without re-resolving by name per call)
    // -------------------------------------------------------------------
    [[nodiscard]] int index_handle() const noexcept { return i_index; }
    [[nodiscard]] int pindex_handle() const noexcept { return i_pindex; }

 private:
    const hipo::bank* m_view = nullptr;
    int i_index = -1;
    int i_pindex = -1;
    int i_C11 = -1;
    int i_C12 = -1;
    int i_C13 = -1;
    int i_C14 = -1;
    int i_C15 = -1;
    int i_C16 = -1;
    int i_C22 = -1;
    int i_C23 = -1;
    int i_C24 = -1;
    int i_C25 = -1;
    int i_C26 = -1;
    int i_C33 = -1;
    int i_C34 = -1;
    int i_C35 = -1;
    int i_C36 = -1;
    int i_C44 = -1;
    int i_C45 = -1;
    int i_C46 = -1;
    int i_C55 = -1;
    int i_C56 = -1;
    int i_C66 = -1;

    friend class Row;
};

/// Lightweight row view for RECAI::CovMat. Stores a pointer + row index only.
class RECAI_CovMat::Row {
 public:
    Row(const RECAI_CovMat* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] float C11() const noexcept { return m_bank->C11(m_row); }
    [[nodiscard]] float C12() const noexcept { return m_bank->C12(m_row); }
    [[nodiscard]] float C13() const noexcept { return m_bank->C13(m_row); }
    [[nodiscard]] float C14() const noexcept { return m_bank->C14(m_row); }
    [[nodiscard]] float C15() const noexcept { return m_bank->C15(m_row); }
    [[nodiscard]] float C16() const noexcept { return m_bank->C16(m_row); }
    [[nodiscard]] float C22() const noexcept { return m_bank->C22(m_row); }
    [[nodiscard]] float C23() const noexcept { return m_bank->C23(m_row); }
    [[nodiscard]] float C24() const noexcept { return m_bank->C24(m_row); }
    [[nodiscard]] float C25() const noexcept { return m_bank->C25(m_row); }
    [[nodiscard]] float C26() const noexcept { return m_bank->C26(m_row); }
    [[nodiscard]] float C33() const noexcept { return m_bank->C33(m_row); }
    [[nodiscard]] float C34() const noexcept { return m_bank->C34(m_row); }
    [[nodiscard]] float C35() const noexcept { return m_bank->C35(m_row); }
    [[nodiscard]] float C36() const noexcept { return m_bank->C36(m_row); }
    [[nodiscard]] float C44() const noexcept { return m_bank->C44(m_row); }
    [[nodiscard]] float C45() const noexcept { return m_bank->C45(m_row); }
    [[nodiscard]] float C46() const noexcept { return m_bank->C46(m_row); }
    [[nodiscard]] float C55() const noexcept { return m_bank->C55(m_row); }
    [[nodiscard]] float C56() const noexcept { return m_bank->C56(m_row); }
    [[nodiscard]] float C66() const noexcept { return m_bank->C66(m_row); }
 private:
    const RECAI_CovMat* m_bank;
    int m_row;
};

inline RECAI_CovMat::Row RECAI_CovMat::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
