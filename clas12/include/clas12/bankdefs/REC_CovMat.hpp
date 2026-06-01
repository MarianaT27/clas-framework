// =============================================================================
// Auto-generated from event.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `REC::CovMat` (group 300, item 38).
/// reconstructed track covariance matrix (5x5 DC Kalman state x, y, tx, ty, q/p at the last superlayer)
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// REC_CovMat bank;
/// if (auto sc = bank.bind(event.get_bank("REC::CovMat")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto index = row.index();
/// }
/// @endcode
class REC_CovMat {
 public:
    static constexpr std::string_view bank_name = "REC::CovMat";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 38;

    class Row;

    REC_CovMat() = default;

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
            int i_C22 = -1;
            int i_C23 = -1;
            int i_C24 = -1;
            int i_C25 = -1;
            int i_C33 = -1;
            int i_C34 = -1;
            int i_C35 = -1;
            int i_C44 = -1;
            int i_C45 = -1;
            int i_C55 = -1;
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
            cache.i_C22 = schema.exists("C22") ? schema.getEntryOrder("C22") : -1;
            cache.i_C23 = schema.exists("C23") ? schema.getEntryOrder("C23") : -1;
            cache.i_C24 = schema.exists("C24") ? schema.getEntryOrder("C24") : -1;
            cache.i_C25 = schema.exists("C25") ? schema.getEntryOrder("C25") : -1;
            cache.i_C33 = schema.exists("C33") ? schema.getEntryOrder("C33") : -1;
            cache.i_C34 = schema.exists("C34") ? schema.getEntryOrder("C34") : -1;
            cache.i_C35 = schema.exists("C35") ? schema.getEntryOrder("C35") : -1;
            cache.i_C44 = schema.exists("C44") ? schema.getEntryOrder("C44") : -1;
            cache.i_C45 = schema.exists("C45") ? schema.getEntryOrder("C45") : -1;
            cache.i_C55 = schema.exists("C55") ? schema.getEntryOrder("C55") : -1;
        }
        i_index = cache.i_index;
        i_pindex = cache.i_pindex;
        i_C11 = cache.i_C11;
        i_C12 = cache.i_C12;
        i_C13 = cache.i_C13;
        i_C14 = cache.i_C14;
        i_C15 = cache.i_C15;
        i_C22 = cache.i_C22;
        i_C23 = cache.i_C23;
        i_C24 = cache.i_C24;
        i_C25 = cache.i_C25;
        i_C33 = cache.i_C33;
        i_C34 = cache.i_C34;
        i_C35 = cache.i_C35;
        i_C44 = cache.i_C44;
        i_C45 = cache.i_C45;
        i_C55 = cache.i_C55;
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
    /// C11: C11 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C11(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C11, row));
    }
    /// C12: C12 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C12(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C12, row));
    }
    /// C13: C13 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C13(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C13, row));
    }
    /// C14: C14 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C14(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C14, row));
    }
    /// C15: C15 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C15(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C15, row));
    }
    /// C22: C22 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C22(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C22, row));
    }
    /// C23: C23 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C23(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C23, row));
    }
    /// C24: C24 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C24(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C24, row));
    }
    /// C25: C25 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C25(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C25, row));
    }
    /// C33: C33 = var(tx) covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C33(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C33, row));
    }
    /// C34: C34 = cov(tx,ty) covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C34(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C34, row));
    }
    /// C35: C35 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C35(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C35, row));
    }
    /// C44: C44 = var(ty) covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C44(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C44, row));
    }
    /// C45: C45 covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C45(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C45, row));
    }
    /// C55: C55 = var(q/p) covariance matrix element at last superlayer used in the fit
    [[nodiscard]] float C55(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C55, row));
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
    int i_C22 = -1;
    int i_C23 = -1;
    int i_C24 = -1;
    int i_C25 = -1;
    int i_C33 = -1;
    int i_C34 = -1;
    int i_C35 = -1;
    int i_C44 = -1;
    int i_C45 = -1;
    int i_C55 = -1;

    friend class Row;
};

/// Lightweight row view for REC::CovMat. Stores a pointer + row index only.
class REC_CovMat::Row {
 public:
    Row(const REC_CovMat* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int index() const noexcept { return m_bank->index(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] float C11() const noexcept { return m_bank->C11(m_row); }
    [[nodiscard]] float C12() const noexcept { return m_bank->C12(m_row); }
    [[nodiscard]] float C13() const noexcept { return m_bank->C13(m_row); }
    [[nodiscard]] float C14() const noexcept { return m_bank->C14(m_row); }
    [[nodiscard]] float C15() const noexcept { return m_bank->C15(m_row); }
    [[nodiscard]] float C22() const noexcept { return m_bank->C22(m_row); }
    [[nodiscard]] float C23() const noexcept { return m_bank->C23(m_row); }
    [[nodiscard]] float C24() const noexcept { return m_bank->C24(m_row); }
    [[nodiscard]] float C25() const noexcept { return m_bank->C25(m_row); }
    [[nodiscard]] float C33() const noexcept { return m_bank->C33(m_row); }
    [[nodiscard]] float C34() const noexcept { return m_bank->C34(m_row); }
    [[nodiscard]] float C35() const noexcept { return m_bank->C35(m_row); }
    [[nodiscard]] float C44() const noexcept { return m_bank->C44(m_row); }
    [[nodiscard]] float C45() const noexcept { return m_bank->C45(m_row); }
    [[nodiscard]] float C55() const noexcept { return m_bank->C55(m_row); }
 private:
    const REC_CovMat* m_bank;
    int m_row;
};

inline REC_CovMat::Row REC_CovMat::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
