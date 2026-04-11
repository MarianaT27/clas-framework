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


/// HIPO bank wrapper for `ALERT::ai:pid` (group 23000, item 34).
/// AI-assisted PID for ALERT
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// ALERT_ai_pid bank;
/// if (auto sc = bank.bind(event.get_bank("ALERT::ai:pid")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto trackid = row.trackid();
/// }
/// @endcode
class ALERT_ai_pid {
 public:
    static constexpr std::string_view bank_name = "ALERT::ai:pid";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 34;

    class Row;

    ALERT_ai_pid() = default;

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
        if (auto h = schema.get_handle("pid"); h) h_pid = *h;
        else h_pid = {};
        if (auto h = schema.get_handle("prob_2212"); h) h_prob_2212 = *h;
        else h_prob_2212 = {};
        if (auto h = schema.get_handle("prob_45"); h) h_prob_45 = *h;
        else h_prob_45 = {};
        if (auto h = schema.get_handle("prob_46"); h) h_prob_46 = *h;
        else h_prob_46 = {};
        if (auto h = schema.get_handle("prob_47"); h) h_prob_47 = *h;
        else h_prob_47 = {};
        if (auto h = schema.get_handle("prob_49"); h) h_prob_49 = *h;
        else h_prob_49 = {};
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
    /// pid: argmax PID
    [[nodiscard]] int pid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_pid, row));
    }
    /// prob_2212: P(pid=2212)
    [[nodiscard]] float prob_2212(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob_2212, row));
    }
    /// prob_45: P(pid=45)
    [[nodiscard]] float prob_45(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob_45, row));
    }
    /// prob_46: P(pid=46)
    [[nodiscard]] float prob_46(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob_46, row));
    }
    /// prob_47: P(pid=47)
    [[nodiscard]] float prob_47(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob_47, row));
    }
    /// prob_49: P(pid=49)
    [[nodiscard]] float prob_49(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob_49, row));
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
    /// pid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> pid_col() const noexcept {
        return m_view.column<int32_t>(h_pid);
    }
    /// prob_2212 column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_2212_col() const noexcept {
        return m_view.column<float>(h_prob_2212);
    }
    /// prob_45 column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_45_col() const noexcept {
        return m_view.column<float>(h_prob_45);
    }
    /// prob_46 column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_46_col() const noexcept {
        return m_view.column<float>(h_prob_46);
    }
    /// prob_47 column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_47_col() const noexcept {
        return m_view.column<float>(h_prob_47);
    }
    /// prob_49 column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_49_col() const noexcept {
        return m_view.column<float>(h_prob_49);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_trackid{};
    hipo::column_handle h_clusterid{};
    hipo::column_handle h_pid{};
    hipo::column_handle h_prob_2212{};
    hipo::column_handle h_prob_45{};
    hipo::column_handle h_prob_46{};
    hipo::column_handle h_prob_47{};
    hipo::column_handle h_prob_49{};

    friend class Row;
};

/// Lightweight row view for ALERT::ai:pid. Stores a pointer + row index only.
class ALERT_ai_pid::Row {
 public:
    Row(const ALERT_ai_pid* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
    [[nodiscard]] int clusterid() const noexcept { return m_bank->clusterid(m_row); }
    [[nodiscard]] int pid() const noexcept { return m_bank->pid(m_row); }
    [[nodiscard]] float prob_2212() const noexcept { return m_bank->prob_2212(m_row); }
    [[nodiscard]] float prob_45() const noexcept { return m_bank->prob_45(m_row); }
    [[nodiscard]] float prob_46() const noexcept { return m_bank->prob_46(m_row); }
    [[nodiscard]] float prob_47() const noexcept { return m_bank->prob_47(m_row); }
    [[nodiscard]] float prob_49() const noexcept { return m_bank->prob_49(m_row); }
 private:
    const ALERT_ai_pid* m_bank;
    int m_row;
};

inline ALERT_ai_pid::Row ALERT_ai_pid::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
