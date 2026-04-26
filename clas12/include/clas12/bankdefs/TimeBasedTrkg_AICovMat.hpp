// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
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


/// HIPO bank wrapper for `TimeBasedTrkg::AICovMat` (group 20600, item 77).
/// reconstructed track covariance matrix in lab frame
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// TimeBasedTrkg_AICovMat bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::AICovMat")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_AICovMat {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::AICovMat";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 77;

    class Row;

    TimeBasedTrkg_AICovMat() = default;

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
            hipo::column_handle h_id;
            hipo::column_handle h_C11;
            hipo::column_handle h_C12;
            hipo::column_handle h_C13;
            hipo::column_handle h_C14;
            hipo::column_handle h_C15;
            hipo::column_handle h_C16;
            hipo::column_handle h_C21;
            hipo::column_handle h_C22;
            hipo::column_handle h_C23;
            hipo::column_handle h_C24;
            hipo::column_handle h_C25;
            hipo::column_handle h_C26;
            hipo::column_handle h_C31;
            hipo::column_handle h_C32;
            hipo::column_handle h_C33;
            hipo::column_handle h_C34;
            hipo::column_handle h_C35;
            hipo::column_handle h_C36;
            hipo::column_handle h_C41;
            hipo::column_handle h_C42;
            hipo::column_handle h_C43;
            hipo::column_handle h_C44;
            hipo::column_handle h_C45;
            hipo::column_handle h_C46;
            hipo::column_handle h_C51;
            hipo::column_handle h_C52;
            hipo::column_handle h_C53;
            hipo::column_handle h_C54;
            hipo::column_handle h_C55;
            hipo::column_handle h_C56;
            hipo::column_handle h_C61;
            hipo::column_handle h_C62;
            hipo::column_handle h_C63;
            hipo::column_handle h_C64;
            hipo::column_handle h_C65;
            hipo::column_handle h_C66;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("C11"); h) cache.h_C11 = *h;
            else cache.h_C11 = {};
            if (auto h = schema.get_handle("C12"); h) cache.h_C12 = *h;
            else cache.h_C12 = {};
            if (auto h = schema.get_handle("C13"); h) cache.h_C13 = *h;
            else cache.h_C13 = {};
            if (auto h = schema.get_handle("C14"); h) cache.h_C14 = *h;
            else cache.h_C14 = {};
            if (auto h = schema.get_handle("C15"); h) cache.h_C15 = *h;
            else cache.h_C15 = {};
            if (auto h = schema.get_handle("C16"); h) cache.h_C16 = *h;
            else cache.h_C16 = {};
            if (auto h = schema.get_handle("C21"); h) cache.h_C21 = *h;
            else cache.h_C21 = {};
            if (auto h = schema.get_handle("C22"); h) cache.h_C22 = *h;
            else cache.h_C22 = {};
            if (auto h = schema.get_handle("C23"); h) cache.h_C23 = *h;
            else cache.h_C23 = {};
            if (auto h = schema.get_handle("C24"); h) cache.h_C24 = *h;
            else cache.h_C24 = {};
            if (auto h = schema.get_handle("C25"); h) cache.h_C25 = *h;
            else cache.h_C25 = {};
            if (auto h = schema.get_handle("C26"); h) cache.h_C26 = *h;
            else cache.h_C26 = {};
            if (auto h = schema.get_handle("C31"); h) cache.h_C31 = *h;
            else cache.h_C31 = {};
            if (auto h = schema.get_handle("C32"); h) cache.h_C32 = *h;
            else cache.h_C32 = {};
            if (auto h = schema.get_handle("C33"); h) cache.h_C33 = *h;
            else cache.h_C33 = {};
            if (auto h = schema.get_handle("C34"); h) cache.h_C34 = *h;
            else cache.h_C34 = {};
            if (auto h = schema.get_handle("C35"); h) cache.h_C35 = *h;
            else cache.h_C35 = {};
            if (auto h = schema.get_handle("C36"); h) cache.h_C36 = *h;
            else cache.h_C36 = {};
            if (auto h = schema.get_handle("C41"); h) cache.h_C41 = *h;
            else cache.h_C41 = {};
            if (auto h = schema.get_handle("C42"); h) cache.h_C42 = *h;
            else cache.h_C42 = {};
            if (auto h = schema.get_handle("C43"); h) cache.h_C43 = *h;
            else cache.h_C43 = {};
            if (auto h = schema.get_handle("C44"); h) cache.h_C44 = *h;
            else cache.h_C44 = {};
            if (auto h = schema.get_handle("C45"); h) cache.h_C45 = *h;
            else cache.h_C45 = {};
            if (auto h = schema.get_handle("C46"); h) cache.h_C46 = *h;
            else cache.h_C46 = {};
            if (auto h = schema.get_handle("C51"); h) cache.h_C51 = *h;
            else cache.h_C51 = {};
            if (auto h = schema.get_handle("C52"); h) cache.h_C52 = *h;
            else cache.h_C52 = {};
            if (auto h = schema.get_handle("C53"); h) cache.h_C53 = *h;
            else cache.h_C53 = {};
            if (auto h = schema.get_handle("C54"); h) cache.h_C54 = *h;
            else cache.h_C54 = {};
            if (auto h = schema.get_handle("C55"); h) cache.h_C55 = *h;
            else cache.h_C55 = {};
            if (auto h = schema.get_handle("C56"); h) cache.h_C56 = *h;
            else cache.h_C56 = {};
            if (auto h = schema.get_handle("C61"); h) cache.h_C61 = *h;
            else cache.h_C61 = {};
            if (auto h = schema.get_handle("C62"); h) cache.h_C62 = *h;
            else cache.h_C62 = {};
            if (auto h = schema.get_handle("C63"); h) cache.h_C63 = *h;
            else cache.h_C63 = {};
            if (auto h = schema.get_handle("C64"); h) cache.h_C64 = *h;
            else cache.h_C64 = {};
            if (auto h = schema.get_handle("C65"); h) cache.h_C65 = *h;
            else cache.h_C65 = {};
            if (auto h = schema.get_handle("C66"); h) cache.h_C66 = *h;
            else cache.h_C66 = {};
        }
        h_id = cache.h_id;
        h_C11 = cache.h_C11;
        h_C12 = cache.h_C12;
        h_C13 = cache.h_C13;
        h_C14 = cache.h_C14;
        h_C15 = cache.h_C15;
        h_C16 = cache.h_C16;
        h_C21 = cache.h_C21;
        h_C22 = cache.h_C22;
        h_C23 = cache.h_C23;
        h_C24 = cache.h_C24;
        h_C25 = cache.h_C25;
        h_C26 = cache.h_C26;
        h_C31 = cache.h_C31;
        h_C32 = cache.h_C32;
        h_C33 = cache.h_C33;
        h_C34 = cache.h_C34;
        h_C35 = cache.h_C35;
        h_C36 = cache.h_C36;
        h_C41 = cache.h_C41;
        h_C42 = cache.h_C42;
        h_C43 = cache.h_C43;
        h_C44 = cache.h_C44;
        h_C45 = cache.h_C45;
        h_C46 = cache.h_C46;
        h_C51 = cache.h_C51;
        h_C52 = cache.h_C52;
        h_C53 = cache.h_C53;
        h_C54 = cache.h_C54;
        h_C55 = cache.h_C55;
        h_C56 = cache.h_C56;
        h_C61 = cache.h_C61;
        h_C62 = cache.h_C62;
        h_C63 = cache.h_C63;
        h_C64 = cache.h_C64;
        h_C65 = cache.h_C65;
        h_C66 = cache.h_C66;
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
    /// id: id of the track
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// C11: var(x) at vertex in lab frame
    [[nodiscard]] float C11(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C11, row));
    }
    /// C12: cov(x, y) at vertex in lab frame
    [[nodiscard]] float C12(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C12, row));
    }
    /// C13: cov(x, z) at vertex in lab frame
    [[nodiscard]] float C13(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C13, row));
    }
    /// C14: cov(x, p_theta) at vertex in lab frame
    [[nodiscard]] float C14(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C14, row));
    }
    /// C15: cov(x, p_phi) at vertex in lab frame
    [[nodiscard]] float C15(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C15, row));
    }
    /// C16: cov(x, p) at vertex in lab frame
    [[nodiscard]] float C16(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C16, row));
    }
    /// C21: cov(y, x) at vertex in lab frame
    [[nodiscard]] float C21(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C21, row));
    }
    /// C22: var(y) at vertex in lab frame
    [[nodiscard]] float C22(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C22, row));
    }
    /// C23: cov(y, z) at vertex in lab frame
    [[nodiscard]] float C23(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C23, row));
    }
    /// C24: cov(y, p_theta) at vertex in lab frame
    [[nodiscard]] float C24(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C24, row));
    }
    /// C25: cov(y, p_phi) at vertex in lab frame
    [[nodiscard]] float C25(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C25, row));
    }
    /// C26: cov(y, p) at vertex in lab frame
    [[nodiscard]] float C26(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C26, row));
    }
    /// C31: cov(z, x) at vertex in lab frame
    [[nodiscard]] float C31(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C31, row));
    }
    /// C32: cov(z, y) at vertex in lab frame
    [[nodiscard]] float C32(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C32, row));
    }
    /// C33: var(z) at vertex in lab frame
    [[nodiscard]] float C33(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C33, row));
    }
    /// C34: cov(z, p_theta) at vertex in lab frame
    [[nodiscard]] float C34(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C34, row));
    }
    /// C35: cov(z, p_phi) at vertex in lab frame
    [[nodiscard]] float C35(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C35, row));
    }
    /// C36: cov(z, p) at vertex in lab frame
    [[nodiscard]] float C36(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C36, row));
    }
    /// C41: cov(p_theta, x) at vertex in lab frame
    [[nodiscard]] float C41(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C41, row));
    }
    /// C42: cov(p_theta, y) at vertex in lab frame
    [[nodiscard]] float C42(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C42, row));
    }
    /// C43: cov(p_theta, z) at vertex in lab frame
    [[nodiscard]] float C43(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C43, row));
    }
    /// C44: var(p_theta) at vertex in lab frame
    [[nodiscard]] float C44(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C44, row));
    }
    /// C45: cov(p_theta, p_phi) at vertex in lab frame
    [[nodiscard]] float C45(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C45, row));
    }
    /// C46: cov(p_theta, p) at vertex in lab frame
    [[nodiscard]] float C46(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C46, row));
    }
    /// C51: cov(p_phi, x) at vertex in lab frame
    [[nodiscard]] float C51(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C51, row));
    }
    /// C52: cov(p_phi, y) at vertex in lab frame
    [[nodiscard]] float C52(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C52, row));
    }
    /// C53: cov(p_phi, z) at vertex in lab frame
    [[nodiscard]] float C53(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C53, row));
    }
    /// C54: cov(p_phi, p_theta) at vertex in lab frame
    [[nodiscard]] float C54(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C54, row));
    }
    /// C55: var(p_phi) at vertex in lab frame
    [[nodiscard]] float C55(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C55, row));
    }
    /// C56: cov(p_phi, p) at vertex in lab frame
    [[nodiscard]] float C56(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C56, row));
    }
    /// C61: cov(p, x) at vertex in lab frame
    [[nodiscard]] float C61(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C61, row));
    }
    /// C62: cov(p, y) at vertex in lab frame
    [[nodiscard]] float C62(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C62, row));
    }
    /// C63: cov(p, z) at vertex in lab frame
    [[nodiscard]] float C63(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C63, row));
    }
    /// C64: cov(p, p_theta) at vertex in lab frame
    [[nodiscard]] float C64(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C64, row));
    }
    /// C65: cov(p, p_phi) at vertex in lab frame
    [[nodiscard]] float C65(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C65, row));
    }
    /// C66: var(p) at vertex in lab frame
    [[nodiscard]] float C66(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_C66, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// C11 column as a contiguous span.
    [[nodiscard]] std::span<const float> C11_col() const noexcept {
        return m_view.column<float>(h_C11);
    }
    /// C12 column as a contiguous span.
    [[nodiscard]] std::span<const float> C12_col() const noexcept {
        return m_view.column<float>(h_C12);
    }
    /// C13 column as a contiguous span.
    [[nodiscard]] std::span<const float> C13_col() const noexcept {
        return m_view.column<float>(h_C13);
    }
    /// C14 column as a contiguous span.
    [[nodiscard]] std::span<const float> C14_col() const noexcept {
        return m_view.column<float>(h_C14);
    }
    /// C15 column as a contiguous span.
    [[nodiscard]] std::span<const float> C15_col() const noexcept {
        return m_view.column<float>(h_C15);
    }
    /// C16 column as a contiguous span.
    [[nodiscard]] std::span<const float> C16_col() const noexcept {
        return m_view.column<float>(h_C16);
    }
    /// C21 column as a contiguous span.
    [[nodiscard]] std::span<const float> C21_col() const noexcept {
        return m_view.column<float>(h_C21);
    }
    /// C22 column as a contiguous span.
    [[nodiscard]] std::span<const float> C22_col() const noexcept {
        return m_view.column<float>(h_C22);
    }
    /// C23 column as a contiguous span.
    [[nodiscard]] std::span<const float> C23_col() const noexcept {
        return m_view.column<float>(h_C23);
    }
    /// C24 column as a contiguous span.
    [[nodiscard]] std::span<const float> C24_col() const noexcept {
        return m_view.column<float>(h_C24);
    }
    /// C25 column as a contiguous span.
    [[nodiscard]] std::span<const float> C25_col() const noexcept {
        return m_view.column<float>(h_C25);
    }
    /// C26 column as a contiguous span.
    [[nodiscard]] std::span<const float> C26_col() const noexcept {
        return m_view.column<float>(h_C26);
    }
    /// C31 column as a contiguous span.
    [[nodiscard]] std::span<const float> C31_col() const noexcept {
        return m_view.column<float>(h_C31);
    }
    /// C32 column as a contiguous span.
    [[nodiscard]] std::span<const float> C32_col() const noexcept {
        return m_view.column<float>(h_C32);
    }
    /// C33 column as a contiguous span.
    [[nodiscard]] std::span<const float> C33_col() const noexcept {
        return m_view.column<float>(h_C33);
    }
    /// C34 column as a contiguous span.
    [[nodiscard]] std::span<const float> C34_col() const noexcept {
        return m_view.column<float>(h_C34);
    }
    /// C35 column as a contiguous span.
    [[nodiscard]] std::span<const float> C35_col() const noexcept {
        return m_view.column<float>(h_C35);
    }
    /// C36 column as a contiguous span.
    [[nodiscard]] std::span<const float> C36_col() const noexcept {
        return m_view.column<float>(h_C36);
    }
    /// C41 column as a contiguous span.
    [[nodiscard]] std::span<const float> C41_col() const noexcept {
        return m_view.column<float>(h_C41);
    }
    /// C42 column as a contiguous span.
    [[nodiscard]] std::span<const float> C42_col() const noexcept {
        return m_view.column<float>(h_C42);
    }
    /// C43 column as a contiguous span.
    [[nodiscard]] std::span<const float> C43_col() const noexcept {
        return m_view.column<float>(h_C43);
    }
    /// C44 column as a contiguous span.
    [[nodiscard]] std::span<const float> C44_col() const noexcept {
        return m_view.column<float>(h_C44);
    }
    /// C45 column as a contiguous span.
    [[nodiscard]] std::span<const float> C45_col() const noexcept {
        return m_view.column<float>(h_C45);
    }
    /// C46 column as a contiguous span.
    [[nodiscard]] std::span<const float> C46_col() const noexcept {
        return m_view.column<float>(h_C46);
    }
    /// C51 column as a contiguous span.
    [[nodiscard]] std::span<const float> C51_col() const noexcept {
        return m_view.column<float>(h_C51);
    }
    /// C52 column as a contiguous span.
    [[nodiscard]] std::span<const float> C52_col() const noexcept {
        return m_view.column<float>(h_C52);
    }
    /// C53 column as a contiguous span.
    [[nodiscard]] std::span<const float> C53_col() const noexcept {
        return m_view.column<float>(h_C53);
    }
    /// C54 column as a contiguous span.
    [[nodiscard]] std::span<const float> C54_col() const noexcept {
        return m_view.column<float>(h_C54);
    }
    /// C55 column as a contiguous span.
    [[nodiscard]] std::span<const float> C55_col() const noexcept {
        return m_view.column<float>(h_C55);
    }
    /// C56 column as a contiguous span.
    [[nodiscard]] std::span<const float> C56_col() const noexcept {
        return m_view.column<float>(h_C56);
    }
    /// C61 column as a contiguous span.
    [[nodiscard]] std::span<const float> C61_col() const noexcept {
        return m_view.column<float>(h_C61);
    }
    /// C62 column as a contiguous span.
    [[nodiscard]] std::span<const float> C62_col() const noexcept {
        return m_view.column<float>(h_C62);
    }
    /// C63 column as a contiguous span.
    [[nodiscard]] std::span<const float> C63_col() const noexcept {
        return m_view.column<float>(h_C63);
    }
    /// C64 column as a contiguous span.
    [[nodiscard]] std::span<const float> C64_col() const noexcept {
        return m_view.column<float>(h_C64);
    }
    /// C65 column as a contiguous span.
    [[nodiscard]] std::span<const float> C65_col() const noexcept {
        return m_view.column<float>(h_C65);
    }
    /// C66 column as a contiguous span.
    [[nodiscard]] std::span<const float> C66_col() const noexcept {
        return m_view.column<float>(h_C66);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_C11{};
    hipo::column_handle h_C12{};
    hipo::column_handle h_C13{};
    hipo::column_handle h_C14{};
    hipo::column_handle h_C15{};
    hipo::column_handle h_C16{};
    hipo::column_handle h_C21{};
    hipo::column_handle h_C22{};
    hipo::column_handle h_C23{};
    hipo::column_handle h_C24{};
    hipo::column_handle h_C25{};
    hipo::column_handle h_C26{};
    hipo::column_handle h_C31{};
    hipo::column_handle h_C32{};
    hipo::column_handle h_C33{};
    hipo::column_handle h_C34{};
    hipo::column_handle h_C35{};
    hipo::column_handle h_C36{};
    hipo::column_handle h_C41{};
    hipo::column_handle h_C42{};
    hipo::column_handle h_C43{};
    hipo::column_handle h_C44{};
    hipo::column_handle h_C45{};
    hipo::column_handle h_C46{};
    hipo::column_handle h_C51{};
    hipo::column_handle h_C52{};
    hipo::column_handle h_C53{};
    hipo::column_handle h_C54{};
    hipo::column_handle h_C55{};
    hipo::column_handle h_C56{};
    hipo::column_handle h_C61{};
    hipo::column_handle h_C62{};
    hipo::column_handle h_C63{};
    hipo::column_handle h_C64{};
    hipo::column_handle h_C65{};
    hipo::column_handle h_C66{};

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::AICovMat. Stores a pointer + row index only.
class TimeBasedTrkg_AICovMat::Row {
 public:
    Row(const TimeBasedTrkg_AICovMat* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
    const TimeBasedTrkg_AICovMat* m_bank;
    int m_row;
};

inline TimeBasedTrkg_AICovMat::Row TimeBasedTrkg_AICovMat::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
