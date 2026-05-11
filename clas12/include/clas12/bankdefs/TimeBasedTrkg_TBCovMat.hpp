// =============================================================================
// Auto-generated from dc.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `TimeBasedTrkg::TBCovMat` (group 20600, item 37).
/// reconstructed track covariance matrix in lab frame
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// TimeBasedTrkg_TBCovMat bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::TBCovMat")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_TBCovMat {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::TBCovMat";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 37;

    class Row;

    TimeBasedTrkg_TBCovMat() = default;

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
            int i_id = -1;
            int i_C11 = -1;
            int i_C12 = -1;
            int i_C13 = -1;
            int i_C14 = -1;
            int i_C15 = -1;
            int i_C16 = -1;
            int i_C21 = -1;
            int i_C22 = -1;
            int i_C23 = -1;
            int i_C24 = -1;
            int i_C25 = -1;
            int i_C26 = -1;
            int i_C31 = -1;
            int i_C32 = -1;
            int i_C33 = -1;
            int i_C34 = -1;
            int i_C35 = -1;
            int i_C36 = -1;
            int i_C41 = -1;
            int i_C42 = -1;
            int i_C43 = -1;
            int i_C44 = -1;
            int i_C45 = -1;
            int i_C46 = -1;
            int i_C51 = -1;
            int i_C52 = -1;
            int i_C53 = -1;
            int i_C54 = -1;
            int i_C55 = -1;
            int i_C56 = -1;
            int i_C61 = -1;
            int i_C62 = -1;
            int i_C63 = -1;
            int i_C64 = -1;
            int i_C65 = -1;
            int i_C66 = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_C11 = schema.exists("C11") ? schema.getEntryOrder("C11") : -1;
            cache.i_C12 = schema.exists("C12") ? schema.getEntryOrder("C12") : -1;
            cache.i_C13 = schema.exists("C13") ? schema.getEntryOrder("C13") : -1;
            cache.i_C14 = schema.exists("C14") ? schema.getEntryOrder("C14") : -1;
            cache.i_C15 = schema.exists("C15") ? schema.getEntryOrder("C15") : -1;
            cache.i_C16 = schema.exists("C16") ? schema.getEntryOrder("C16") : -1;
            cache.i_C21 = schema.exists("C21") ? schema.getEntryOrder("C21") : -1;
            cache.i_C22 = schema.exists("C22") ? schema.getEntryOrder("C22") : -1;
            cache.i_C23 = schema.exists("C23") ? schema.getEntryOrder("C23") : -1;
            cache.i_C24 = schema.exists("C24") ? schema.getEntryOrder("C24") : -1;
            cache.i_C25 = schema.exists("C25") ? schema.getEntryOrder("C25") : -1;
            cache.i_C26 = schema.exists("C26") ? schema.getEntryOrder("C26") : -1;
            cache.i_C31 = schema.exists("C31") ? schema.getEntryOrder("C31") : -1;
            cache.i_C32 = schema.exists("C32") ? schema.getEntryOrder("C32") : -1;
            cache.i_C33 = schema.exists("C33") ? schema.getEntryOrder("C33") : -1;
            cache.i_C34 = schema.exists("C34") ? schema.getEntryOrder("C34") : -1;
            cache.i_C35 = schema.exists("C35") ? schema.getEntryOrder("C35") : -1;
            cache.i_C36 = schema.exists("C36") ? schema.getEntryOrder("C36") : -1;
            cache.i_C41 = schema.exists("C41") ? schema.getEntryOrder("C41") : -1;
            cache.i_C42 = schema.exists("C42") ? schema.getEntryOrder("C42") : -1;
            cache.i_C43 = schema.exists("C43") ? schema.getEntryOrder("C43") : -1;
            cache.i_C44 = schema.exists("C44") ? schema.getEntryOrder("C44") : -1;
            cache.i_C45 = schema.exists("C45") ? schema.getEntryOrder("C45") : -1;
            cache.i_C46 = schema.exists("C46") ? schema.getEntryOrder("C46") : -1;
            cache.i_C51 = schema.exists("C51") ? schema.getEntryOrder("C51") : -1;
            cache.i_C52 = schema.exists("C52") ? schema.getEntryOrder("C52") : -1;
            cache.i_C53 = schema.exists("C53") ? schema.getEntryOrder("C53") : -1;
            cache.i_C54 = schema.exists("C54") ? schema.getEntryOrder("C54") : -1;
            cache.i_C55 = schema.exists("C55") ? schema.getEntryOrder("C55") : -1;
            cache.i_C56 = schema.exists("C56") ? schema.getEntryOrder("C56") : -1;
            cache.i_C61 = schema.exists("C61") ? schema.getEntryOrder("C61") : -1;
            cache.i_C62 = schema.exists("C62") ? schema.getEntryOrder("C62") : -1;
            cache.i_C63 = schema.exists("C63") ? schema.getEntryOrder("C63") : -1;
            cache.i_C64 = schema.exists("C64") ? schema.getEntryOrder("C64") : -1;
            cache.i_C65 = schema.exists("C65") ? schema.getEntryOrder("C65") : -1;
            cache.i_C66 = schema.exists("C66") ? schema.getEntryOrder("C66") : -1;
        }
        i_id = cache.i_id;
        i_C11 = cache.i_C11;
        i_C12 = cache.i_C12;
        i_C13 = cache.i_C13;
        i_C14 = cache.i_C14;
        i_C15 = cache.i_C15;
        i_C16 = cache.i_C16;
        i_C21 = cache.i_C21;
        i_C22 = cache.i_C22;
        i_C23 = cache.i_C23;
        i_C24 = cache.i_C24;
        i_C25 = cache.i_C25;
        i_C26 = cache.i_C26;
        i_C31 = cache.i_C31;
        i_C32 = cache.i_C32;
        i_C33 = cache.i_C33;
        i_C34 = cache.i_C34;
        i_C35 = cache.i_C35;
        i_C36 = cache.i_C36;
        i_C41 = cache.i_C41;
        i_C42 = cache.i_C42;
        i_C43 = cache.i_C43;
        i_C44 = cache.i_C44;
        i_C45 = cache.i_C45;
        i_C46 = cache.i_C46;
        i_C51 = cache.i_C51;
        i_C52 = cache.i_C52;
        i_C53 = cache.i_C53;
        i_C54 = cache.i_C54;
        i_C55 = cache.i_C55;
        i_C56 = cache.i_C56;
        i_C61 = cache.i_C61;
        i_C62 = cache.i_C62;
        i_C63 = cache.i_C63;
        i_C64 = cache.i_C64;
        i_C65 = cache.i_C65;
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
    /// id: id of the track
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// C11: var(x) at vertex in lab frame
    [[nodiscard]] float C11(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C11, row));
    }
    /// C12: cov(x, y) at vertex in lab frame
    [[nodiscard]] float C12(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C12, row));
    }
    /// C13: cov(x, z) at vertex in lab frame
    [[nodiscard]] float C13(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C13, row));
    }
    /// C14: cov(x, p_theta) at vertex in lab frame
    [[nodiscard]] float C14(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C14, row));
    }
    /// C15: cov(x, p_phi) at vertex in lab frame
    [[nodiscard]] float C15(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C15, row));
    }
    /// C16: cov(x, p) at vertex in lab frame
    [[nodiscard]] float C16(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C16, row));
    }
    /// C21: cov(y, x) at vertex in lab frame
    [[nodiscard]] float C21(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C21, row));
    }
    /// C22: var(y) at vertex in lab frame
    [[nodiscard]] float C22(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C22, row));
    }
    /// C23: cov(y, z) at vertex in lab frame
    [[nodiscard]] float C23(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C23, row));
    }
    /// C24: cov(y, p_theta) at vertex in lab frame
    [[nodiscard]] float C24(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C24, row));
    }
    /// C25: cov(y, p_phi) at vertex in lab frame
    [[nodiscard]] float C25(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C25, row));
    }
    /// C26: cov(y, p) at vertex in lab frame
    [[nodiscard]] float C26(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C26, row));
    }
    /// C31: cov(z, x) at vertex in lab frame
    [[nodiscard]] float C31(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C31, row));
    }
    /// C32: cov(z, y) at vertex in lab frame
    [[nodiscard]] float C32(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C32, row));
    }
    /// C33: var(z) at vertex in lab frame
    [[nodiscard]] float C33(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C33, row));
    }
    /// C34: cov(z, p_theta) at vertex in lab frame
    [[nodiscard]] float C34(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C34, row));
    }
    /// C35: cov(z, p_phi) at vertex in lab frame
    [[nodiscard]] float C35(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C35, row));
    }
    /// C36: cov(z, p) at vertex in lab frame
    [[nodiscard]] float C36(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C36, row));
    }
    /// C41: cov(p_theta, x) at vertex in lab frame
    [[nodiscard]] float C41(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C41, row));
    }
    /// C42: cov(p_theta, y) at vertex in lab frame
    [[nodiscard]] float C42(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C42, row));
    }
    /// C43: cov(p_theta, z) at vertex in lab frame
    [[nodiscard]] float C43(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C43, row));
    }
    /// C44: var(p_theta) at vertex in lab frame
    [[nodiscard]] float C44(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C44, row));
    }
    /// C45: cov(p_theta, p_phi) at vertex in lab frame
    [[nodiscard]] float C45(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C45, row));
    }
    /// C46: cov(p_theta, p) at vertex in lab frame
    [[nodiscard]] float C46(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C46, row));
    }
    /// C51: cov(p_phi, x) at vertex in lab frame
    [[nodiscard]] float C51(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C51, row));
    }
    /// C52: cov(p_phi, y) at vertex in lab frame
    [[nodiscard]] float C52(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C52, row));
    }
    /// C53: cov(p_phi, z) at vertex in lab frame
    [[nodiscard]] float C53(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C53, row));
    }
    /// C54: cov(p_phi, p_theta) at vertex in lab frame
    [[nodiscard]] float C54(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C54, row));
    }
    /// C55: var(p_phi) at vertex in lab frame
    [[nodiscard]] float C55(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C55, row));
    }
    /// C56: cov(p_phi, p) at vertex in lab frame
    [[nodiscard]] float C56(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C56, row));
    }
    /// C61: cov(p, x) at vertex in lab frame
    [[nodiscard]] float C61(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C61, row));
    }
    /// C62: cov(p, y) at vertex in lab frame
    [[nodiscard]] float C62(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C62, row));
    }
    /// C63: cov(p, z) at vertex in lab frame
    [[nodiscard]] float C63(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C63, row));
    }
    /// C64: cov(p, p_theta) at vertex in lab frame
    [[nodiscard]] float C64(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C64, row));
    }
    /// C65: cov(p, p_phi) at vertex in lab frame
    [[nodiscard]] float C65(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C65, row));
    }
    /// C66: var(p) at vertex in lab frame
    [[nodiscard]] float C66(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_C66, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_C11 = -1;
    int i_C12 = -1;
    int i_C13 = -1;
    int i_C14 = -1;
    int i_C15 = -1;
    int i_C16 = -1;
    int i_C21 = -1;
    int i_C22 = -1;
    int i_C23 = -1;
    int i_C24 = -1;
    int i_C25 = -1;
    int i_C26 = -1;
    int i_C31 = -1;
    int i_C32 = -1;
    int i_C33 = -1;
    int i_C34 = -1;
    int i_C35 = -1;
    int i_C36 = -1;
    int i_C41 = -1;
    int i_C42 = -1;
    int i_C43 = -1;
    int i_C44 = -1;
    int i_C45 = -1;
    int i_C46 = -1;
    int i_C51 = -1;
    int i_C52 = -1;
    int i_C53 = -1;
    int i_C54 = -1;
    int i_C55 = -1;
    int i_C56 = -1;
    int i_C61 = -1;
    int i_C62 = -1;
    int i_C63 = -1;
    int i_C64 = -1;
    int i_C65 = -1;
    int i_C66 = -1;

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::TBCovMat. Stores a pointer + row index only.
class TimeBasedTrkg_TBCovMat::Row {
 public:
    Row(const TimeBasedTrkg_TBCovMat* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] float C11() const noexcept { return m_bank->C11(m_row); }
    [[nodiscard]] float C12() const noexcept { return m_bank->C12(m_row); }
    [[nodiscard]] float C13() const noexcept { return m_bank->C13(m_row); }
    [[nodiscard]] float C14() const noexcept { return m_bank->C14(m_row); }
    [[nodiscard]] float C15() const noexcept { return m_bank->C15(m_row); }
    [[nodiscard]] float C16() const noexcept { return m_bank->C16(m_row); }
    [[nodiscard]] float C21() const noexcept { return m_bank->C21(m_row); }
    [[nodiscard]] float C22() const noexcept { return m_bank->C22(m_row); }
    [[nodiscard]] float C23() const noexcept { return m_bank->C23(m_row); }
    [[nodiscard]] float C24() const noexcept { return m_bank->C24(m_row); }
    [[nodiscard]] float C25() const noexcept { return m_bank->C25(m_row); }
    [[nodiscard]] float C26() const noexcept { return m_bank->C26(m_row); }
    [[nodiscard]] float C31() const noexcept { return m_bank->C31(m_row); }
    [[nodiscard]] float C32() const noexcept { return m_bank->C32(m_row); }
    [[nodiscard]] float C33() const noexcept { return m_bank->C33(m_row); }
    [[nodiscard]] float C34() const noexcept { return m_bank->C34(m_row); }
    [[nodiscard]] float C35() const noexcept { return m_bank->C35(m_row); }
    [[nodiscard]] float C36() const noexcept { return m_bank->C36(m_row); }
    [[nodiscard]] float C41() const noexcept { return m_bank->C41(m_row); }
    [[nodiscard]] float C42() const noexcept { return m_bank->C42(m_row); }
    [[nodiscard]] float C43() const noexcept { return m_bank->C43(m_row); }
    [[nodiscard]] float C44() const noexcept { return m_bank->C44(m_row); }
    [[nodiscard]] float C45() const noexcept { return m_bank->C45(m_row); }
    [[nodiscard]] float C46() const noexcept { return m_bank->C46(m_row); }
    [[nodiscard]] float C51() const noexcept { return m_bank->C51(m_row); }
    [[nodiscard]] float C52() const noexcept { return m_bank->C52(m_row); }
    [[nodiscard]] float C53() const noexcept { return m_bank->C53(m_row); }
    [[nodiscard]] float C54() const noexcept { return m_bank->C54(m_row); }
    [[nodiscard]] float C55() const noexcept { return m_bank->C55(m_row); }
    [[nodiscard]] float C56() const noexcept { return m_bank->C56(m_row); }
    [[nodiscard]] float C61() const noexcept { return m_bank->C61(m_row); }
    [[nodiscard]] float C62() const noexcept { return m_bank->C62(m_row); }
    [[nodiscard]] float C63() const noexcept { return m_bank->C63(m_row); }
    [[nodiscard]] float C64() const noexcept { return m_bank->C64(m_row); }
    [[nodiscard]] float C65() const noexcept { return m_bank->C65(m_row); }
    [[nodiscard]] float C66() const noexcept { return m_bank->C66(m_row); }
 private:
    const TimeBasedTrkg_TBCovMat* m_bank;
    int m_row;
};

inline TimeBasedTrkg_TBCovMat::Row TimeBasedTrkg_TBCovMat::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
