// =============================================================================
// Auto-generated from event-ai.json - do not edit by hand.
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


/// HIPO bank wrapper for `RECHBAI::Event` (group 400, item 10).
/// Event Header Bank
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// RECHBAI_Event bank;
/// if (auto sc = bank.bind(event.get_bank("RECHBAI::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto category = row.category();
/// }
/// @endcode
class RECHBAI_Event {
 public:
    static constexpr std::string_view bank_name = "RECHBAI::Event";
    static constexpr int bank_group = 400;
    static constexpr int bank_item  = 10;

    class Row;

    RECHBAI_Event() = default;

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
            hipo::column_handle h_category;
            hipo::column_handle h_topology;
            hipo::column_handle h_beamCharge;
            hipo::column_handle h_liveTime;
            hipo::column_handle h_startTime;
            hipo::column_handle h_RFTime;
            hipo::column_handle h_helicity;
            hipo::column_handle h_helicityRaw;
            hipo::column_handle h_procTime;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("category"); h) cache.h_category = *h;
            else cache.h_category = {};
            if (auto h = schema.get_handle("topology"); h) cache.h_topology = *h;
            else cache.h_topology = {};
            if (auto h = schema.get_handle("beamCharge"); h) cache.h_beamCharge = *h;
            else cache.h_beamCharge = {};
            if (auto h = schema.get_handle("liveTime"); h) cache.h_liveTime = *h;
            else cache.h_liveTime = {};
            if (auto h = schema.get_handle("startTime"); h) cache.h_startTime = *h;
            else cache.h_startTime = {};
            if (auto h = schema.get_handle("RFTime"); h) cache.h_RFTime = *h;
            else cache.h_RFTime = {};
            if (auto h = schema.get_handle("helicity"); h) cache.h_helicity = *h;
            else cache.h_helicity = {};
            if (auto h = schema.get_handle("helicityRaw"); h) cache.h_helicityRaw = *h;
            else cache.h_helicityRaw = {};
            if (auto h = schema.get_handle("procTime"); h) cache.h_procTime = *h;
            else cache.h_procTime = {};
        }
        h_category = cache.h_category;
        h_topology = cache.h_topology;
        h_beamCharge = cache.h_beamCharge;
        h_liveTime = cache.h_liveTime;
        h_startTime = cache.h_startTime;
        h_RFTime = cache.h_RFTime;
        h_helicity = cache.h_helicity;
        h_helicityRaw = cache.h_helicityRaw;
        h_procTime = cache.h_procTime;
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
    /// category: Undefined
    [[nodiscard]] int64_t category(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_category, row));
    }
    /// topology: Undefined
    [[nodiscard]] int64_t topology(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_topology, row));
    }
    /// beamCharge: Beam charge, gated (nano-Coulomb)
    [[nodiscard]] float beamCharge(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_beamCharge, row));
    }
    /// liveTime: Lifetime
    [[nodiscard]] double liveTime(int row) const noexcept {
        return static_cast<double>(m_view.get<double>(h_liveTime, row));
    }
    /// startTime: Event Start Time (ns)
    [[nodiscard]] float startTime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_startTime, row));
    }
    /// RFTime: RF Time (ns)
    [[nodiscard]] float RFTime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_RFTime, row));
    }
    /// helicity: Helicity of Event (+/-1, else undefined), with HWP-correction
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicity, row));
    }
    /// helicityRaw: Helicity of Event (+/-1, else undefined)
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_helicityRaw, row));
    }
    /// procTime: Event Processing Time (UNIX Time = seconds)
    [[nodiscard]] float procTime(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_procTime, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// category column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> category_col() const noexcept {
        return m_view.column<int64_t>(h_category);
    }
    /// topology column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> topology_col() const noexcept {
        return m_view.column<int64_t>(h_topology);
    }
    /// beamCharge column as a contiguous span.
    [[nodiscard]] std::span<const float> beamCharge_col() const noexcept {
        return m_view.column<float>(h_beamCharge);
    }
    /// liveTime column as a contiguous span.
    [[nodiscard]] std::span<const double> liveTime_col() const noexcept {
        return m_view.column<double>(h_liveTime);
    }
    /// startTime column as a contiguous span.
    [[nodiscard]] std::span<const float> startTime_col() const noexcept {
        return m_view.column<float>(h_startTime);
    }
    /// RFTime column as a contiguous span.
    [[nodiscard]] std::span<const float> RFTime_col() const noexcept {
        return m_view.column<float>(h_RFTime);
    }
    /// helicity column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicity_col() const noexcept {
        return m_view.column<int8_t>(h_helicity);
    }
    /// helicityRaw column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> helicityRaw_col() const noexcept {
        return m_view.column<int8_t>(h_helicityRaw);
    }
    /// procTime column as a contiguous span.
    [[nodiscard]] std::span<const float> procTime_col() const noexcept {
        return m_view.column<float>(h_procTime);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_category{};
    hipo::column_handle h_topology{};
    hipo::column_handle h_beamCharge{};
    hipo::column_handle h_liveTime{};
    hipo::column_handle h_startTime{};
    hipo::column_handle h_RFTime{};
    hipo::column_handle h_helicity{};
    hipo::column_handle h_helicityRaw{};
    hipo::column_handle h_procTime{};

    friend class Row;
};

/// Lightweight row view for RECHBAI::Event. Stores a pointer + row index only.
class RECHBAI_Event::Row {
 public:
    Row(const RECHBAI_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int64_t category() const noexcept { return m_bank->category(m_row); }
    [[nodiscard]] int64_t topology() const noexcept { return m_bank->topology(m_row); }
    [[nodiscard]] float beamCharge() const noexcept { return m_bank->beamCharge(m_row); }
    [[nodiscard]] double liveTime() const noexcept { return m_bank->liveTime(m_row); }
    [[nodiscard]] float startTime() const noexcept { return m_bank->startTime(m_row); }
    [[nodiscard]] float RFTime() const noexcept { return m_bank->RFTime(m_row); }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int helicityRaw() const noexcept { return m_bank->helicityRaw(m_row); }
    [[nodiscard]] float procTime() const noexcept { return m_bank->procTime(m_row); }
 private:
    const RECHBAI_Event* m_bank;
    int m_row;
};

inline RECHBAI_Event::Row RECHBAI_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
