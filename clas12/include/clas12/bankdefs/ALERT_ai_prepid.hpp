// =============================================================================
// Auto-generated from alert.json - do not edit by hand.
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


/// HIPO bank wrapper for `ALERT::ai:prepid` (group 23000, item 33).
/// ALERT AI-assisted PrePID to be used for Kalman Filter
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ALERT_ai_prepid bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::ai:prepid")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trackid = row.trackid();
/// }
/// @endcode
class ALERT_ai_prepid {
 public:
    static constexpr std::string_view bank_name = "ALERT::ai:prepid";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 33;

    class Row;

    ALERT_ai_prepid() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("trackid"); h) h_trackid = *h;
        else h_trackid = {};
        if (auto h = schema.get_handle("clusterid"); h) h_clusterid = *h;
        else h_clusterid = {};
        if (auto h = schema.get_handle("prepid"); h) h_prepid = *h;
        else h_prepid = {};
        if (auto h = schema.get_handle("p2212"); h) h_p2212 = *h;
        else h_p2212 = {};
        if (auto h = schema.get_handle("p45"); h) h_p45 = *h;
        else h_p45 = {};
        if (auto h = schema.get_handle("p46"); h) h_p46 = *h;
        else h_p46 = {};
        if (auto h = schema.get_handle("p47"); h) h_p47 = *h;
        else h_p47 = {};
        if (auto h = schema.get_handle("p49"); h) h_p49 = *h;
        else h_p49 = {};
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
    /// trackid: AHDC trackid
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trackid, row));
    }
    /// clusterid: ATOF cluster id
    [[nodiscard]] int clusterid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_clusterid, row));
    }
    /// prepid: argmax PID
    [[nodiscard]] int prepid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_prepid, row));
    }
    /// p2212: P(pid=2212)
    [[nodiscard]] float p2212(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p2212, row));
    }
    /// p45: P(pid=45)
    [[nodiscard]] float p45(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p45, row));
    }
    /// p46: P(pid=46)
    [[nodiscard]] float p46(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p46, row));
    }
    /// p47: P(pid=47)
    [[nodiscard]] float p47(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p47, row));
    }
    /// p49: P(pid=49)
    [[nodiscard]] float p49(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p49, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// trackid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trackid_col() const noexcept {
        return m_view.column<int32_t>(h_trackid);
    }
    /// clusterid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> clusterid_col() const noexcept {
        return m_view.column<int32_t>(h_clusterid);
    }
    /// prepid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> prepid_col() const noexcept {
        return m_view.column<int32_t>(h_prepid);
    }
    /// p2212 column as a contiguous span.
    [[nodiscard]] std::span<const float> p2212_col() const noexcept {
        return m_view.column<float>(h_p2212);
    }
    /// p45 column as a contiguous span.
    [[nodiscard]] std::span<const float> p45_col() const noexcept {
        return m_view.column<float>(h_p45);
    }
    /// p46 column as a contiguous span.
    [[nodiscard]] std::span<const float> p46_col() const noexcept {
        return m_view.column<float>(h_p46);
    }
    /// p47 column as a contiguous span.
    [[nodiscard]] std::span<const float> p47_col() const noexcept {
        return m_view.column<float>(h_p47);
    }
    /// p49 column as a contiguous span.
    [[nodiscard]] std::span<const float> p49_col() const noexcept {
        return m_view.column<float>(h_p49);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_trackid{};
    hipo::column_handle h_clusterid{};
    hipo::column_handle h_prepid{};
    hipo::column_handle h_p2212{};
    hipo::column_handle h_p45{};
    hipo::column_handle h_p46{};
    hipo::column_handle h_p47{};
    hipo::column_handle h_p49{};

    friend class Row;
};

/// Lightweight row view for ALERT::ai:prepid. Stores a pointer + row index only.
class ALERT_ai_prepid::Row {
 public:
    Row(const ALERT_ai_prepid* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
    [[nodiscard]] int prepid() const noexcept { return m_bank->prepid(m_row); }
    [[nodiscard]] float p2212() const noexcept { return m_bank->p2212(m_row); }
    [[nodiscard]] float p45() const noexcept { return m_bank->p45(m_row); }
    [[nodiscard]] float p46() const noexcept { return m_bank->p46(m_row); }
    [[nodiscard]] float p47() const noexcept { return m_bank->p47(m_row); }
    [[nodiscard]] float p49() const noexcept { return m_bank->p49(m_row); }
 private:
    const ALERT_ai_prepid* m_bank;
    int m_row;
};

inline ALERT_ai_prepid::Row ALERT_ai_prepid::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
