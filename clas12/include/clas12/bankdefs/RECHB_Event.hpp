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


/// HIPO bank wrapper for `RECHB::Event` (group 300, item 10).
/// Event Header Bank
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// RECHB_Event bank;
/// if (auto sc = bank.bind(event.get_bank("RECHB::Event")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto category = row.category();
/// }
/// @endcode
class RECHB_Event {
 public:
    static constexpr std::string_view bank_name = "RECHB::Event";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 10;
    static constexpr std::string_view bank_schema = "category/L,topology/L,beamCharge/F,liveTime/D,startTime/F,RFTime/F,helicity/B,helicityRaw/B,procTime/F";

    class Row;

    RECHB_Event() = default;

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
            int i_category = -1;
            int i_topology = -1;
            int i_beamCharge = -1;
            int i_liveTime = -1;
            int i_startTime = -1;
            int i_RFTime = -1;
            int i_helicity = -1;
            int i_helicityRaw = -1;
            int i_procTime = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_category = schema.exists("category") ? schema.getEntryOrder("category") : -1;
            cache.i_topology = schema.exists("topology") ? schema.getEntryOrder("topology") : -1;
            cache.i_beamCharge = schema.exists("beamCharge") ? schema.getEntryOrder("beamCharge") : -1;
            cache.i_liveTime = schema.exists("liveTime") ? schema.getEntryOrder("liveTime") : -1;
            cache.i_startTime = schema.exists("startTime") ? schema.getEntryOrder("startTime") : -1;
            cache.i_RFTime = schema.exists("RFTime") ? schema.getEntryOrder("RFTime") : -1;
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
            cache.i_helicityRaw = schema.exists("helicityRaw") ? schema.getEntryOrder("helicityRaw") : -1;
            cache.i_procTime = schema.exists("procTime") ? schema.getEntryOrder("procTime") : -1;
        }
        i_category = cache.i_category;
        i_topology = cache.i_topology;
        i_beamCharge = cache.i_beamCharge;
        i_liveTime = cache.i_liveTime;
        i_startTime = cache.i_startTime;
        i_RFTime = cache.i_RFTime;
        i_helicity = cache.i_helicity;
        i_helicityRaw = cache.i_helicityRaw;
        i_procTime = cache.i_procTime;
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
    /// category: Undefined
    [[nodiscard]] int64_t category(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_category, row));
    }
    /// topology: Undefined
    [[nodiscard]] int64_t topology(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_topology, row));
    }
    /// beamCharge: Beam charge, gated (nano-Coulomb)
    [[nodiscard]] float beamCharge(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_beamCharge, row));
    }
    /// liveTime: Lifetime
    [[nodiscard]] double liveTime(int row) const noexcept {
        return static_cast<double>(m_view->getDouble(i_liveTime, row));
    }
    /// startTime: Event Start Time (ns)
    [[nodiscard]] float startTime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_startTime, row));
    }
    /// RFTime: RF Time (ns)
    [[nodiscard]] float RFTime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_RFTime, row));
    }
    /// helicity: Helicity of Event (+/-1, else undefined), with HWP-correction
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicity, row));
    }
    /// helicityRaw: Helicity of Event (+/-1, else undefined)
    [[nodiscard]] int helicityRaw(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicityRaw, row));
    }
    /// procTime: Event Processing Time (UNIX Time = seconds)
    [[nodiscard]] float procTime(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_procTime, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_category = -1;
    int i_topology = -1;
    int i_beamCharge = -1;
    int i_liveTime = -1;
    int i_startTime = -1;
    int i_RFTime = -1;
    int i_helicity = -1;
    int i_helicityRaw = -1;
    int i_procTime = -1;

    friend class Row;
};

/// Lightweight row view for RECHB::Event. Stores a pointer + row index only.
class RECHB_Event::Row {
 public:
    Row(const RECHB_Event* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
    const RECHB_Event* m_bank;
    int m_row;
};

inline RECHB_Event::Row RECHB_Event::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
