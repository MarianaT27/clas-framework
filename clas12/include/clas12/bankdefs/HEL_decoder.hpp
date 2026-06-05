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


/// HIPO bank wrapper for `HEL::decoder` (group 22000, item 14).
/// Helicity decoder board data
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// HEL_decoder bank;
/// if (auto sc = bank.bind(event.get_bank("HEL::decoder")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto helicity = row.helicity();
/// }
/// @endcode
class HEL_decoder {
 public:
    static constexpr std::string_view bank_name = "HEL::decoder";
    static constexpr int bank_group = 22000;
    static constexpr int bank_item  = 14;
    static constexpr std::string_view bank_schema = "helicity/B,pair/B,pattern/B,tSettle/B,helicityPattern/B,polarity/B,phase/B,timestamp/L,helicitySeed/I,nTStableRE/I,nTStableFE/I,nPattern/I,nPair/I,tStableStart/I,tStableEnd/I,tStableTime/I,tSettleTime/I,patternArray/I,pairArray/I,helicityArray/I,helicityPArray/I";

    class Row;

    HEL_decoder() = default;

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
            int i_helicity = -1;
            int i_pair = -1;
            int i_pattern = -1;
            int i_tSettle = -1;
            int i_helicityPattern = -1;
            int i_polarity = -1;
            int i_phase = -1;
            int i_timestamp = -1;
            int i_helicitySeed = -1;
            int i_nTStableRE = -1;
            int i_nTStableFE = -1;
            int i_nPattern = -1;
            int i_nPair = -1;
            int i_tStableStart = -1;
            int i_tStableEnd = -1;
            int i_tStableTime = -1;
            int i_tSettleTime = -1;
            int i_patternArray = -1;
            int i_pairArray = -1;
            int i_helicityArray = -1;
            int i_helicityPArray = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_helicity = schema.exists("helicity") ? schema.getEntryOrder("helicity") : -1;
            cache.i_pair = schema.exists("pair") ? schema.getEntryOrder("pair") : -1;
            cache.i_pattern = schema.exists("pattern") ? schema.getEntryOrder("pattern") : -1;
            cache.i_tSettle = schema.exists("tSettle") ? schema.getEntryOrder("tSettle") : -1;
            cache.i_helicityPattern = schema.exists("helicityPattern") ? schema.getEntryOrder("helicityPattern") : -1;
            cache.i_polarity = schema.exists("polarity") ? schema.getEntryOrder("polarity") : -1;
            cache.i_phase = schema.exists("phase") ? schema.getEntryOrder("phase") : -1;
            cache.i_timestamp = schema.exists("timestamp") ? schema.getEntryOrder("timestamp") : -1;
            cache.i_helicitySeed = schema.exists("helicitySeed") ? schema.getEntryOrder("helicitySeed") : -1;
            cache.i_nTStableRE = schema.exists("nTStableRE") ? schema.getEntryOrder("nTStableRE") : -1;
            cache.i_nTStableFE = schema.exists("nTStableFE") ? schema.getEntryOrder("nTStableFE") : -1;
            cache.i_nPattern = schema.exists("nPattern") ? schema.getEntryOrder("nPattern") : -1;
            cache.i_nPair = schema.exists("nPair") ? schema.getEntryOrder("nPair") : -1;
            cache.i_tStableStart = schema.exists("tStableStart") ? schema.getEntryOrder("tStableStart") : -1;
            cache.i_tStableEnd = schema.exists("tStableEnd") ? schema.getEntryOrder("tStableEnd") : -1;
            cache.i_tStableTime = schema.exists("tStableTime") ? schema.getEntryOrder("tStableTime") : -1;
            cache.i_tSettleTime = schema.exists("tSettleTime") ? schema.getEntryOrder("tSettleTime") : -1;
            cache.i_patternArray = schema.exists("patternArray") ? schema.getEntryOrder("patternArray") : -1;
            cache.i_pairArray = schema.exists("pairArray") ? schema.getEntryOrder("pairArray") : -1;
            cache.i_helicityArray = schema.exists("helicityArray") ? schema.getEntryOrder("helicityArray") : -1;
            cache.i_helicityPArray = schema.exists("helicityPArray") ? schema.getEntryOrder("helicityPArray") : -1;
        }
        i_helicity = cache.i_helicity;
        i_pair = cache.i_pair;
        i_pattern = cache.i_pattern;
        i_tSettle = cache.i_tSettle;
        i_helicityPattern = cache.i_helicityPattern;
        i_polarity = cache.i_polarity;
        i_phase = cache.i_phase;
        i_timestamp = cache.i_timestamp;
        i_helicitySeed = cache.i_helicitySeed;
        i_nTStableRE = cache.i_nTStableRE;
        i_nTStableFE = cache.i_nTStableFE;
        i_nPattern = cache.i_nPattern;
        i_nPair = cache.i_nPair;
        i_tStableStart = cache.i_tStableStart;
        i_tStableEnd = cache.i_tStableEnd;
        i_tStableTime = cache.i_tStableTime;
        i_tSettleTime = cache.i_tSettleTime;
        i_patternArray = cache.i_patternArray;
        i_pairArray = cache.i_pairArray;
        i_helicityArray = cache.i_helicityArray;
        i_helicityPArray = cache.i_helicityPArray;
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
    /// helicity: HELICITY
    [[nodiscard]] int helicity(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicity, row));
    }
    /// pair: PAIR_SYNC
    [[nodiscard]] int pair(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pair, row));
    }
    /// pattern: PATTERN_SYNC
    [[nodiscard]] int pattern(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_pattern, row));
    }
    /// tSettle: T_SETTLE
    [[nodiscard]] int tSettle(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_tSettle, row));
    }
    /// helicityPattern: HELICITY at pattern start
    [[nodiscard]] int helicityPattern(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_helicityPattern, row));
    }
    /// polarity: event polarity (XOR of 3,4)
    [[nodiscard]] int polarity(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_polarity, row));
    }
    /// phase: pattern phase count
    [[nodiscard]] int phase(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_phase, row));
    }
    /// timestamp: timestamp (4ns)
    [[nodiscard]] int64_t timestamp(int row) const noexcept {
        return static_cast<int64_t>(m_view->getLong(i_timestamp, row));
    }
    /// helicitySeed: helicity seed
    [[nodiscard]] int helicitySeed(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_helicitySeed, row));
    }
    /// nTStableRE: count of T_STABLE rising edge
    [[nodiscard]] int nTStableRE(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_nTStableRE, row));
    }
    /// nTStableFE: count of T_STABLE falling edge
    [[nodiscard]] int nTStableFE(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_nTStableFE, row));
    }
    /// nPattern: count of PATTERN_SYNC
    [[nodiscard]] int nPattern(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_nPattern, row));
    }
    /// nPair: count of PAIR_SYNC
    [[nodiscard]] int nPair(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_nPair, row));
    }
    /// tStableStart: time of trigger since start of T_STABLE (8 ns
    [[nodiscard]] int tStableStart(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tStableStart, row));
    }
    /// tStableEnd: time of trigger since end of T_STABLE (8 ns
    [[nodiscard]] int tStableEnd(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tStableEnd, row));
    }
    /// tStableTime: time duration of last complete T_STABLE period (8 ns
    [[nodiscard]] int tStableTime(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tStableTime, row));
    }
    /// tSettleTime: time duration of last complete T_SETTLE period (8 ns
    [[nodiscard]] int tSettleTime(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_tSettleTime, row));
    }
    /// patternArray: last 32 windows of PATTERN_SYNC
    [[nodiscard]] int patternArray(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_patternArray, row));
    }
    /// pairArray: last 32 windows of PAIR_SYNC
    [[nodiscard]] int pairArray(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_pairArray, row));
    }
    /// helicityArray: last 32 windows of HELICITY_STATE
    [[nodiscard]] int helicityArray(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_helicityArray, row));
    }
    /// helicityPArray: last 32 windows of HELICITY_STATE at start of pattern
    [[nodiscard]] int helicityPArray(int row) const noexcept {
        return static_cast<int>(m_view->getInt(i_helicityPArray, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_helicity = -1;
    int i_pair = -1;
    int i_pattern = -1;
    int i_tSettle = -1;
    int i_helicityPattern = -1;
    int i_polarity = -1;
    int i_phase = -1;
    int i_timestamp = -1;
    int i_helicitySeed = -1;
    int i_nTStableRE = -1;
    int i_nTStableFE = -1;
    int i_nPattern = -1;
    int i_nPair = -1;
    int i_tStableStart = -1;
    int i_tStableEnd = -1;
    int i_tStableTime = -1;
    int i_tSettleTime = -1;
    int i_patternArray = -1;
    int i_pairArray = -1;
    int i_helicityArray = -1;
    int i_helicityPArray = -1;

    friend class Row;
};

/// Lightweight row view for HEL::decoder. Stores a pointer + row index only.
class HEL_decoder::Row {
 public:
    Row(const HEL_decoder* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int helicity() const noexcept { return m_bank->helicity(m_row); }
    [[nodiscard]] int pair() const noexcept { return m_bank->pair(m_row); }
    [[nodiscard]] int pattern() const noexcept { return m_bank->pattern(m_row); }
    [[nodiscard]] int tSettle() const noexcept { return m_bank->tSettle(m_row); }
    [[nodiscard]] int helicityPattern() const noexcept { return m_bank->helicityPattern(m_row); }
    [[nodiscard]] int polarity() const noexcept { return m_bank->polarity(m_row); }
    [[nodiscard]] int phase() const noexcept { return m_bank->phase(m_row); }
    [[nodiscard]] int64_t timestamp() const noexcept { return m_bank->timestamp(m_row); }
    [[nodiscard]] int helicitySeed() const noexcept { return m_bank->helicitySeed(m_row); }
    [[nodiscard]] int nTStableRE() const noexcept { return m_bank->nTStableRE(m_row); }
    [[nodiscard]] int nTStableFE() const noexcept { return m_bank->nTStableFE(m_row); }
    [[nodiscard]] int nPattern() const noexcept { return m_bank->nPattern(m_row); }
    [[nodiscard]] int nPair() const noexcept { return m_bank->nPair(m_row); }
    [[nodiscard]] int tStableStart() const noexcept { return m_bank->tStableStart(m_row); }
    [[nodiscard]] int tStableEnd() const noexcept { return m_bank->tStableEnd(m_row); }
    [[nodiscard]] int tStableTime() const noexcept { return m_bank->tStableTime(m_row); }
    [[nodiscard]] int tSettleTime() const noexcept { return m_bank->tSettleTime(m_row); }
    [[nodiscard]] int patternArray() const noexcept { return m_bank->patternArray(m_row); }
    [[nodiscard]] int pairArray() const noexcept { return m_bank->pairArray(m_row); }
    [[nodiscard]] int helicityArray() const noexcept { return m_bank->helicityArray(m_row); }
    [[nodiscard]] int helicityPArray() const noexcept { return m_bank->helicityPArray(m_row); }
 private:
    const HEL_decoder* m_bank;
    int m_row;
};

inline HEL_decoder::Row HEL_decoder::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
