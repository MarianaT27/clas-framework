// =============================================================================
// Auto-generated from data.json - do not edit by hand.
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


/// HIPO bank wrapper for `AHDC::wf` (group 22400, item 10).
/// Waveform bank for the AHDC
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
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
            hipo::column_handle h_sector;
            hipo::column_handle h_layer;
            hipo::column_handle h_component;
            hipo::column_handle h_order;
            hipo::column_handle h_timestamp;
            hipo::column_handle h_s1;
            hipo::column_handle h_s2;
            hipo::column_handle h_s3;
            hipo::column_handle h_s4;
            hipo::column_handle h_s5;
            hipo::column_handle h_s6;
            hipo::column_handle h_s7;
            hipo::column_handle h_s8;
            hipo::column_handle h_s9;
            hipo::column_handle h_s10;
            hipo::column_handle h_s11;
            hipo::column_handle h_s12;
            hipo::column_handle h_s13;
            hipo::column_handle h_s14;
            hipo::column_handle h_s15;
            hipo::column_handle h_s16;
            hipo::column_handle h_s17;
            hipo::column_handle h_s18;
            hipo::column_handle h_s19;
            hipo::column_handle h_s20;
            hipo::column_handle h_s21;
            hipo::column_handle h_s22;
            hipo::column_handle h_s23;
            hipo::column_handle h_s24;
            hipo::column_handle h_s25;
            hipo::column_handle h_s26;
            hipo::column_handle h_s27;
            hipo::column_handle h_s28;
            hipo::column_handle h_s29;
            hipo::column_handle h_s30;
            hipo::column_handle h_time;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("component"); h) cache.h_component = *h;
            else cache.h_component = {};
            if (auto h = schema.get_handle("order"); h) cache.h_order = *h;
            else cache.h_order = {};
            if (auto h = schema.get_handle("timestamp"); h) cache.h_timestamp = *h;
            else cache.h_timestamp = {};
            if (auto h = schema.get_handle("s1"); h) cache.h_s1 = *h;
            else cache.h_s1 = {};
            if (auto h = schema.get_handle("s2"); h) cache.h_s2 = *h;
            else cache.h_s2 = {};
            if (auto h = schema.get_handle("s3"); h) cache.h_s3 = *h;
            else cache.h_s3 = {};
            if (auto h = schema.get_handle("s4"); h) cache.h_s4 = *h;
            else cache.h_s4 = {};
            if (auto h = schema.get_handle("s5"); h) cache.h_s5 = *h;
            else cache.h_s5 = {};
            if (auto h = schema.get_handle("s6"); h) cache.h_s6 = *h;
            else cache.h_s6 = {};
            if (auto h = schema.get_handle("s7"); h) cache.h_s7 = *h;
            else cache.h_s7 = {};
            if (auto h = schema.get_handle("s8"); h) cache.h_s8 = *h;
            else cache.h_s8 = {};
            if (auto h = schema.get_handle("s9"); h) cache.h_s9 = *h;
            else cache.h_s9 = {};
            if (auto h = schema.get_handle("s10"); h) cache.h_s10 = *h;
            else cache.h_s10 = {};
            if (auto h = schema.get_handle("s11"); h) cache.h_s11 = *h;
            else cache.h_s11 = {};
            if (auto h = schema.get_handle("s12"); h) cache.h_s12 = *h;
            else cache.h_s12 = {};
            if (auto h = schema.get_handle("s13"); h) cache.h_s13 = *h;
            else cache.h_s13 = {};
            if (auto h = schema.get_handle("s14"); h) cache.h_s14 = *h;
            else cache.h_s14 = {};
            if (auto h = schema.get_handle("s15"); h) cache.h_s15 = *h;
            else cache.h_s15 = {};
            if (auto h = schema.get_handle("s16"); h) cache.h_s16 = *h;
            else cache.h_s16 = {};
            if (auto h = schema.get_handle("s17"); h) cache.h_s17 = *h;
            else cache.h_s17 = {};
            if (auto h = schema.get_handle("s18"); h) cache.h_s18 = *h;
            else cache.h_s18 = {};
            if (auto h = schema.get_handle("s19"); h) cache.h_s19 = *h;
            else cache.h_s19 = {};
            if (auto h = schema.get_handle("s20"); h) cache.h_s20 = *h;
            else cache.h_s20 = {};
            if (auto h = schema.get_handle("s21"); h) cache.h_s21 = *h;
            else cache.h_s21 = {};
            if (auto h = schema.get_handle("s22"); h) cache.h_s22 = *h;
            else cache.h_s22 = {};
            if (auto h = schema.get_handle("s23"); h) cache.h_s23 = *h;
            else cache.h_s23 = {};
            if (auto h = schema.get_handle("s24"); h) cache.h_s24 = *h;
            else cache.h_s24 = {};
            if (auto h = schema.get_handle("s25"); h) cache.h_s25 = *h;
            else cache.h_s25 = {};
            if (auto h = schema.get_handle("s26"); h) cache.h_s26 = *h;
            else cache.h_s26 = {};
            if (auto h = schema.get_handle("s27"); h) cache.h_s27 = *h;
            else cache.h_s27 = {};
            if (auto h = schema.get_handle("s28"); h) cache.h_s28 = *h;
            else cache.h_s28 = {};
            if (auto h = schema.get_handle("s29"); h) cache.h_s29 = *h;
            else cache.h_s29 = {};
            if (auto h = schema.get_handle("s30"); h) cache.h_s30 = *h;
            else cache.h_s30 = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
        }
        h_sector = cache.h_sector;
        h_layer = cache.h_layer;
        h_component = cache.h_component;
        h_order = cache.h_order;
        h_timestamp = cache.h_timestamp;
        h_s1 = cache.h_s1;
        h_s2 = cache.h_s2;
        h_s3 = cache.h_s3;
        h_s4 = cache.h_s4;
        h_s5 = cache.h_s5;
        h_s6 = cache.h_s6;
        h_s7 = cache.h_s7;
        h_s8 = cache.h_s8;
        h_s9 = cache.h_s9;
        h_s10 = cache.h_s10;
        h_s11 = cache.h_s11;
        h_s12 = cache.h_s12;
        h_s13 = cache.h_s13;
        h_s14 = cache.h_s14;
        h_s15 = cache.h_s15;
        h_s16 = cache.h_s16;
        h_s17 = cache.h_s17;
        h_s18 = cache.h_s18;
        h_s19 = cache.h_s19;
        h_s20 = cache.h_s20;
        h_s21 = cache.h_s21;
        h_s22 = cache.h_s22;
        h_s23 = cache.h_s23;
        h_s24 = cache.h_s24;
        h_s25 = cache.h_s25;
        h_s26 = cache.h_s26;
        h_s27 = cache.h_s27;
        h_s28 = cache.h_s28;
        h_s29 = cache.h_s29;
        h_s30 = cache.h_s30;
        h_time = cache.h_time;
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
    /// sector: sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// layer: layer (1-6)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// component: strips
    [[nodiscard]] int component(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_component, row));
    }
    /// order: order: 0 - ADCL , 1 - ADCR
    [[nodiscard]] int order(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_order, row));
    }
    /// timestamp: timestamp
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_timestamp, row));
    }
    /// s1: first sample
    [[nodiscard]] int s1(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s1, row));
    }
    /// s2: 
    [[nodiscard]] int s2(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s2, row));
    }
    /// s3: 
    [[nodiscard]] int s3(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s3, row));
    }
    /// s4: 
    [[nodiscard]] int s4(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s4, row));
    }
    /// s5: 
    [[nodiscard]] int s5(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s5, row));
    }
    /// s6: 
    [[nodiscard]] int s6(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s6, row));
    }
    /// s7: 
    [[nodiscard]] int s7(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s7, row));
    }
    /// s8: 
    [[nodiscard]] int s8(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s8, row));
    }
    /// s9: 
    [[nodiscard]] int s9(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s9, row));
    }
    /// s10: 
    [[nodiscard]] int s10(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s10, row));
    }
    /// s11: 
    [[nodiscard]] int s11(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s11, row));
    }
    /// s12: 
    [[nodiscard]] int s12(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s12, row));
    }
    /// s13: 
    [[nodiscard]] int s13(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s13, row));
    }
    /// s14: 
    [[nodiscard]] int s14(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s14, row));
    }
    /// s15: 
    [[nodiscard]] int s15(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s15, row));
    }
    /// s16: 
    [[nodiscard]] int s16(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s16, row));
    }
    /// s17: 
    [[nodiscard]] int s17(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s17, row));
    }
    /// s18: 
    [[nodiscard]] int s18(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s18, row));
    }
    /// s19: 
    [[nodiscard]] int s19(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s19, row));
    }
    /// s20: 
    [[nodiscard]] int s20(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s20, row));
    }
    /// s21: 
    [[nodiscard]] int s21(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s21, row));
    }
    /// s22: 
    [[nodiscard]] int s22(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s22, row));
    }
    /// s23: 
    [[nodiscard]] int s23(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s23, row));
    }
    /// s24: 
    [[nodiscard]] int s24(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s24, row));
    }
    /// s25: 
    [[nodiscard]] int s25(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s25, row));
    }
    /// s26: 
    [[nodiscard]] int s26(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s26, row));
    }
    /// s27: 
    [[nodiscard]] int s27(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s27, row));
    }
    /// s28: 
    [[nodiscard]] int s28(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s28, row));
    }
    /// s29: 
    [[nodiscard]] int s29(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s29, row));
    }
    /// s30: 
    [[nodiscard]] int s30(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_s30, row));
    }
    /// time: for fine time correction
    [[nodiscard]] int time(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_time, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
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
    /// order column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> order_col() const noexcept {
        return m_view.column<int8_t>(h_order);
    }
    /// timestamp column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> timestamp_col() const noexcept {
        return m_view.column<int64_t>(h_timestamp);
    }
    /// s1 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s1_col() const noexcept {
        return m_view.column<int16_t>(h_s1);
    }
    /// s2 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s2_col() const noexcept {
        return m_view.column<int16_t>(h_s2);
    }
    /// s3 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s3_col() const noexcept {
        return m_view.column<int16_t>(h_s3);
    }
    /// s4 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s4_col() const noexcept {
        return m_view.column<int16_t>(h_s4);
    }
    /// s5 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s5_col() const noexcept {
        return m_view.column<int16_t>(h_s5);
    }
    /// s6 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s6_col() const noexcept {
        return m_view.column<int16_t>(h_s6);
    }
    /// s7 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s7_col() const noexcept {
        return m_view.column<int16_t>(h_s7);
    }
    /// s8 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s8_col() const noexcept {
        return m_view.column<int16_t>(h_s8);
    }
    /// s9 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s9_col() const noexcept {
        return m_view.column<int16_t>(h_s9);
    }
    /// s10 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s10_col() const noexcept {
        return m_view.column<int16_t>(h_s10);
    }
    /// s11 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s11_col() const noexcept {
        return m_view.column<int16_t>(h_s11);
    }
    /// s12 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s12_col() const noexcept {
        return m_view.column<int16_t>(h_s12);
    }
    /// s13 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s13_col() const noexcept {
        return m_view.column<int16_t>(h_s13);
    }
    /// s14 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s14_col() const noexcept {
        return m_view.column<int16_t>(h_s14);
    }
    /// s15 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s15_col() const noexcept {
        return m_view.column<int16_t>(h_s15);
    }
    /// s16 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s16_col() const noexcept {
        return m_view.column<int16_t>(h_s16);
    }
    /// s17 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s17_col() const noexcept {
        return m_view.column<int16_t>(h_s17);
    }
    /// s18 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s18_col() const noexcept {
        return m_view.column<int16_t>(h_s18);
    }
    /// s19 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s19_col() const noexcept {
        return m_view.column<int16_t>(h_s19);
    }
    /// s20 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s20_col() const noexcept {
        return m_view.column<int16_t>(h_s20);
    }
    /// s21 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s21_col() const noexcept {
        return m_view.column<int16_t>(h_s21);
    }
    /// s22 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s22_col() const noexcept {
        return m_view.column<int16_t>(h_s22);
    }
    /// s23 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s23_col() const noexcept {
        return m_view.column<int16_t>(h_s23);
    }
    /// s24 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s24_col() const noexcept {
        return m_view.column<int16_t>(h_s24);
    }
    /// s25 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s25_col() const noexcept {
        return m_view.column<int16_t>(h_s25);
    }
    /// s26 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s26_col() const noexcept {
        return m_view.column<int16_t>(h_s26);
    }
    /// s27 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s27_col() const noexcept {
        return m_view.column<int16_t>(h_s27);
    }
    /// s28 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s28_col() const noexcept {
        return m_view.column<int16_t>(h_s28);
    }
    /// s29 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s29_col() const noexcept {
        return m_view.column<int16_t>(h_s29);
    }
    /// s30 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> s30_col() const noexcept {
        return m_view.column<int16_t>(h_s30);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> time_col() const noexcept {
        return m_view.column<int32_t>(h_time);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_component{};
    hipo::column_handle h_order{};
    hipo::column_handle h_timestamp{};
    hipo::column_handle h_s1{};
    hipo::column_handle h_s2{};
    hipo::column_handle h_s3{};
    hipo::column_handle h_s4{};
    hipo::column_handle h_s5{};
    hipo::column_handle h_s6{};
    hipo::column_handle h_s7{};
    hipo::column_handle h_s8{};
    hipo::column_handle h_s9{};
    hipo::column_handle h_s10{};
    hipo::column_handle h_s11{};
    hipo::column_handle h_s12{};
    hipo::column_handle h_s13{};
    hipo::column_handle h_s14{};
    hipo::column_handle h_s15{};
    hipo::column_handle h_s16{};
    hipo::column_handle h_s17{};
    hipo::column_handle h_s18{};
    hipo::column_handle h_s19{};
    hipo::column_handle h_s20{};
    hipo::column_handle h_s21{};
    hipo::column_handle h_s22{};
    hipo::column_handle h_s23{};
    hipo::column_handle h_s24{};
    hipo::column_handle h_s25{};
    hipo::column_handle h_s26{};
    hipo::column_handle h_s27{};
    hipo::column_handle h_s28{};
    hipo::column_handle h_s29{};
    hipo::column_handle h_s30{};
    hipo::column_handle h_time{};

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

}  // namespace clas12::bankdefs
