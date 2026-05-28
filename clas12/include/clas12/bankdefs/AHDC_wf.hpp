// =============================================================================
// Auto-generated from data.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `AHDC::wf` (group 22400, item 10).
/// Waveform bank for the AHDC
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// AHDC_wf bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::wf")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class AHDC_wf {
 public:
    static constexpr std::string_view bank_name = "AHDC::wf";
    static constexpr int bank_group = 22400;
    static constexpr int bank_item  = 10;

    class Row;

    AHDC_wf() = default;

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
            int i_sector = -1;
            int i_layer = -1;
            int i_component = -1;
            int i_order = -1;
            int i_timestamp = -1;
            int i_s1 = -1;
            int i_s2 = -1;
            int i_s3 = -1;
            int i_s4 = -1;
            int i_s5 = -1;
            int i_s6 = -1;
            int i_s7 = -1;
            int i_s8 = -1;
            int i_s9 = -1;
            int i_s10 = -1;
            int i_s11 = -1;
            int i_s12 = -1;
            int i_s13 = -1;
            int i_s14 = -1;
            int i_s15 = -1;
            int i_s16 = -1;
            int i_s17 = -1;
            int i_s18 = -1;
            int i_s19 = -1;
            int i_s20 = -1;
            int i_s21 = -1;
            int i_s22 = -1;
            int i_s23 = -1;
            int i_s24 = -1;
            int i_s25 = -1;
            int i_s26 = -1;
            int i_s27 = -1;
            int i_s28 = -1;
            int i_s29 = -1;
            int i_s30 = -1;
            int i_time = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_layer = schema.exists("layer") ? schema.getEntryOrder("layer") : -1;
            cache.i_component = schema.exists("component") ? schema.getEntryOrder("component") : -1;
            cache.i_order = schema.exists("order") ? schema.getEntryOrder("order") : -1;
            cache.i_timestamp = schema.exists("timestamp") ? schema.getEntryOrder("timestamp") : -1;
            cache.i_s1 = schema.exists("s1") ? schema.getEntryOrder("s1") : -1;
            cache.i_s2 = schema.exists("s2") ? schema.getEntryOrder("s2") : -1;
            cache.i_s3 = schema.exists("s3") ? schema.getEntryOrder("s3") : -1;
            cache.i_s4 = schema.exists("s4") ? schema.getEntryOrder("s4") : -1;
            cache.i_s5 = schema.exists("s5") ? schema.getEntryOrder("s5") : -1;
            cache.i_s6 = schema.exists("s6") ? schema.getEntryOrder("s6") : -1;
            cache.i_s7 = schema.exists("s7") ? schema.getEntryOrder("s7") : -1;
            cache.i_s8 = schema.exists("s8") ? schema.getEntryOrder("s8") : -1;
            cache.i_s9 = schema.exists("s9") ? schema.getEntryOrder("s9") : -1;
            cache.i_s10 = schema.exists("s10") ? schema.getEntryOrder("s10") : -1;
            cache.i_s11 = schema.exists("s11") ? schema.getEntryOrder("s11") : -1;
            cache.i_s12 = schema.exists("s12") ? schema.getEntryOrder("s12") : -1;
            cache.i_s13 = schema.exists("s13") ? schema.getEntryOrder("s13") : -1;
            cache.i_s14 = schema.exists("s14") ? schema.getEntryOrder("s14") : -1;
            cache.i_s15 = schema.exists("s15") ? schema.getEntryOrder("s15") : -1;
            cache.i_s16 = schema.exists("s16") ? schema.getEntryOrder("s16") : -1;
            cache.i_s17 = schema.exists("s17") ? schema.getEntryOrder("s17") : -1;
            cache.i_s18 = schema.exists("s18") ? schema.getEntryOrder("s18") : -1;
            cache.i_s19 = schema.exists("s19") ? schema.getEntryOrder("s19") : -1;
            cache.i_s20 = schema.exists("s20") ? schema.getEntryOrder("s20") : -1;
            cache.i_s21 = schema.exists("s21") ? schema.getEntryOrder("s21") : -1;
            cache.i_s22 = schema.exists("s22") ? schema.getEntryOrder("s22") : -1;
            cache.i_s23 = schema.exists("s23") ? schema.getEntryOrder("s23") : -1;
            cache.i_s24 = schema.exists("s24") ? schema.getEntryOrder("s24") : -1;
            cache.i_s25 = schema.exists("s25") ? schema.getEntryOrder("s25") : -1;
            cache.i_s26 = schema.exists("s26") ? schema.getEntryOrder("s26") : -1;
            cache.i_s27 = schema.exists("s27") ? schema.getEntryOrder("s27") : -1;
            cache.i_s28 = schema.exists("s28") ? schema.getEntryOrder("s28") : -1;
            cache.i_s29 = schema.exists("s29") ? schema.getEntryOrder("s29") : -1;
            cache.i_s30 = schema.exists("s30") ? schema.getEntryOrder("s30") : -1;
            cache.i_time = schema.exists("time") ? schema.getEntryOrder("time") : -1;
        }
        i_sector = cache.i_sector;
        i_layer = cache.i_layer;
        i_component = cache.i_component;
        i_order = cache.i_order;
        i_timestamp = cache.i_timestamp;
        i_s1 = cache.i_s1;
        i_s2 = cache.i_s2;
        i_s3 = cache.i_s3;
        i_s4 = cache.i_s4;
        i_s5 = cache.i_s5;
        i_s6 = cache.i_s6;
        i_s7 = cache.i_s7;
        i_s8 = cache.i_s8;
        i_s9 = cache.i_s9;
        i_s10 = cache.i_s10;
        i_s11 = cache.i_s11;
        i_s12 = cache.i_s12;
        i_s13 = cache.i_s13;
        i_s14 = cache.i_s14;
        i_s15 = cache.i_s15;
        i_s16 = cache.i_s16;
        i_s17 = cache.i_s17;
        i_s18 = cache.i_s18;
        i_s19 = cache.i_s19;
        i_s20 = cache.i_s20;
        i_s21 = cache.i_s21;
        i_s22 = cache.i_s22;
        i_s23 = cache.i_s23;
        i_s24 = cache.i_s24;
        i_s25 = cache.i_s25;
        i_s26 = cache.i_s26;
        i_s27 = cache.i_s27;
        i_s28 = cache.i_s28;
        i_s29 = cache.i_s29;
        i_s30 = cache.i_s30;
        i_time = cache.i_time;
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
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// layer: layer (1-6)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_layer, row));
    }
    /// component: strips
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_component, row));
    }
    /// order: order: 0 - ADCL , 1 - ADCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_order, row));
    }
    /// timestamp: timestamp
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_timestamp, row));
    }
    /// s1: first sample
    [[nodiscard]] int s1(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s1, row));
    }
    /// s2: 
    [[nodiscard]] int s2(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s2, row));
    }
    /// s3: 
    [[nodiscard]] int s3(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s3, row));
    }
    /// s4: 
    [[nodiscard]] int s4(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s4, row));
    }
    /// s5: 
    [[nodiscard]] int s5(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s5, row));
    }
    /// s6: 
    [[nodiscard]] int s6(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s6, row));
    }
    /// s7: 
    [[nodiscard]] int s7(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s7, row));
    }
    /// s8: 
    [[nodiscard]] int s8(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s8, row));
    }
    /// s9: 
    [[nodiscard]] int s9(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s9, row));
    }
    /// s10: 
    [[nodiscard]] int s10(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s10, row));
    }
    /// s11: 
    [[nodiscard]] int s11(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s11, row));
    }
    /// s12: 
    [[nodiscard]] int s12(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s12, row));
    }
    /// s13: 
    [[nodiscard]] int s13(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s13, row));
    }
    /// s14: 
    [[nodiscard]] int s14(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s14, row));
    }
    /// s15: 
    [[nodiscard]] int s15(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s15, row));
    }
    /// s16: 
    [[nodiscard]] int s16(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s16, row));
    }
    /// s17: 
    [[nodiscard]] int s17(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s17, row));
    }
    /// s18: 
    [[nodiscard]] int s18(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s18, row));
    }
    /// s19: 
    [[nodiscard]] int s19(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s19, row));
    }
    /// s20: 
    [[nodiscard]] int s20(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s20, row));
    }
    /// s21: 
    [[nodiscard]] int s21(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s21, row));
    }
    /// s22: 
    [[nodiscard]] int s22(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s22, row));
    }
    /// s23: 
    [[nodiscard]] int s23(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s23, row));
    }
    /// s24: 
    [[nodiscard]] int s24(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s24, row));
    }
    /// s25: 
    [[nodiscard]] int s25(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s25, row));
    }
    /// s26: 
    [[nodiscard]] int s26(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s26, row));
    }
    /// s27: 
    [[nodiscard]] int s27(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s27, row));
    }
    /// s28: 
    [[nodiscard]] int s28(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s28, row));
    }
    /// s29: 
    [[nodiscard]] int s29(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s29, row));
    }
    /// s30: 
    [[nodiscard]] int s30(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_s30, row));
    }
    /// time: for fine time correction
    [[nodiscard]] int time(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_time, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_sector = -1;
    int i_layer = -1;
    int i_component = -1;
    int i_order = -1;
    int i_timestamp = -1;
    int i_s1 = -1;
    int i_s2 = -1;
    int i_s3 = -1;
    int i_s4 = -1;
    int i_s5 = -1;
    int i_s6 = -1;
    int i_s7 = -1;
    int i_s8 = -1;
    int i_s9 = -1;
    int i_s10 = -1;
    int i_s11 = -1;
    int i_s12 = -1;
    int i_s13 = -1;
    int i_s14 = -1;
    int i_s15 = -1;
    int i_s16 = -1;
    int i_s17 = -1;
    int i_s18 = -1;
    int i_s19 = -1;
    int i_s20 = -1;
    int i_s21 = -1;
    int i_s22 = -1;
    int i_s23 = -1;
    int i_s24 = -1;
    int i_s25 = -1;
    int i_s26 = -1;
    int i_s27 = -1;
    int i_s28 = -1;
    int i_s29 = -1;
    int i_s30 = -1;
    int i_time = -1;

    friend class Row;
};

/// Lightweight row view for AHDC::wf. Stores a pointer + row index only.
class AHDC_wf::Row {
 public:
    Row(const AHDC_wf* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int component() const noexcept { return m_bank->component(m_row); }
    [[nodiscard]] int order() const noexcept { return m_bank->order(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
    [[nodiscard]] int s1() const noexcept { return m_bank->s1(m_row); }
    [[nodiscard]] int s2() const noexcept { return m_bank->s2(m_row); }
    [[nodiscard]] int s3() const noexcept { return m_bank->s3(m_row); }
    [[nodiscard]] int s4() const noexcept { return m_bank->s4(m_row); }
    [[nodiscard]] int s5() const noexcept { return m_bank->s5(m_row); }
    [[nodiscard]] int s6() const noexcept { return m_bank->s6(m_row); }
    [[nodiscard]] int s7() const noexcept { return m_bank->s7(m_row); }
    [[nodiscard]] int s8() const noexcept { return m_bank->s8(m_row); }
    [[nodiscard]] int s9() const noexcept { return m_bank->s9(m_row); }
    [[nodiscard]] int s10() const noexcept { return m_bank->s10(m_row); }
    [[nodiscard]] int s11() const noexcept { return m_bank->s11(m_row); }
    [[nodiscard]] int s12() const noexcept { return m_bank->s12(m_row); }
    [[nodiscard]] int s13() const noexcept { return m_bank->s13(m_row); }
    [[nodiscard]] int s14() const noexcept { return m_bank->s14(m_row); }
    [[nodiscard]] int s15() const noexcept { return m_bank->s15(m_row); }
    [[nodiscard]] int s16() const noexcept { return m_bank->s16(m_row); }
    [[nodiscard]] int s17() const noexcept { return m_bank->s17(m_row); }
    [[nodiscard]] int s18() const noexcept { return m_bank->s18(m_row); }
    [[nodiscard]] int s19() const noexcept { return m_bank->s19(m_row); }
    [[nodiscard]] int s20() const noexcept { return m_bank->s20(m_row); }
    [[nodiscard]] int s21() const noexcept { return m_bank->s21(m_row); }
    [[nodiscard]] int s22() const noexcept { return m_bank->s22(m_row); }
    [[nodiscard]] int s23() const noexcept { return m_bank->s23(m_row); }
    [[nodiscard]] int s24() const noexcept { return m_bank->s24(m_row); }
    [[nodiscard]] int s25() const noexcept { return m_bank->s25(m_row); }
    [[nodiscard]] int s26() const noexcept { return m_bank->s26(m_row); }
    [[nodiscard]] int s27() const noexcept { return m_bank->s27(m_row); }
    [[nodiscard]] int s28() const noexcept { return m_bank->s28(m_row); }
    [[nodiscard]] int s29() const noexcept { return m_bank->s29(m_row); }
    [[nodiscard]] int s30() const noexcept { return m_bank->s30(m_row); }
    [[nodiscard]] int time() const noexcept { return m_bank->time(m_row); }
 private:
    const AHDC_wf* m_bank;
    int m_row;
};

inline AHDC_wf::Row AHDC_wf::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
