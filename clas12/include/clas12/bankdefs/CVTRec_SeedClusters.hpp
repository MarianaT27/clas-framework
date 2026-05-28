// =============================================================================
// Auto-generated from cvt.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `CVTRec::SeedClusters` (group 20500, item 44).
/// Ids of clusters on track for 12 layers of CVT
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// CVTRec_SeedClusters bank;
/// if (auto sc = bank.bind(event.get_bank("CVTRec::SeedClusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CVTRec_SeedClusters {
 public:
    static constexpr std::string_view bank_name = "CVTRec::SeedClusters";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 44;

    class Row;

    CVTRec_SeedClusters() = default;

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
            int i_id = -1;
            int i_Clus1_ID = -1;
            int i_Clus2_ID = -1;
            int i_Clus3_ID = -1;
            int i_Clus4_ID = -1;
            int i_Clus5_ID = -1;
            int i_Clus6_ID = -1;
            int i_Clus7_ID = -1;
            int i_Clus8_ID = -1;
            int i_Clus9_ID = -1;
            int i_Clus10_ID = -1;
            int i_Clus11_ID = -1;
            int i_Clus12_ID = -1;
            int i_prob = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_Clus1_ID = schema.exists("Clus1_ID") ? schema.getEntryOrder("Clus1_ID") : -1;
            cache.i_Clus2_ID = schema.exists("Clus2_ID") ? schema.getEntryOrder("Clus2_ID") : -1;
            cache.i_Clus3_ID = schema.exists("Clus3_ID") ? schema.getEntryOrder("Clus3_ID") : -1;
            cache.i_Clus4_ID = schema.exists("Clus4_ID") ? schema.getEntryOrder("Clus4_ID") : -1;
            cache.i_Clus5_ID = schema.exists("Clus5_ID") ? schema.getEntryOrder("Clus5_ID") : -1;
            cache.i_Clus6_ID = schema.exists("Clus6_ID") ? schema.getEntryOrder("Clus6_ID") : -1;
            cache.i_Clus7_ID = schema.exists("Clus7_ID") ? schema.getEntryOrder("Clus7_ID") : -1;
            cache.i_Clus8_ID = schema.exists("Clus8_ID") ? schema.getEntryOrder("Clus8_ID") : -1;
            cache.i_Clus9_ID = schema.exists("Clus9_ID") ? schema.getEntryOrder("Clus9_ID") : -1;
            cache.i_Clus10_ID = schema.exists("Clus10_ID") ? schema.getEntryOrder("Clus10_ID") : -1;
            cache.i_Clus11_ID = schema.exists("Clus11_ID") ? schema.getEntryOrder("Clus11_ID") : -1;
            cache.i_Clus12_ID = schema.exists("Clus12_ID") ? schema.getEntryOrder("Clus12_ID") : -1;
            cache.i_prob = schema.exists("prob") ? schema.getEntryOrder("prob") : -1;
        }
        i_id = cache.i_id;
        i_Clus1_ID = cache.i_Clus1_ID;
        i_Clus2_ID = cache.i_Clus2_ID;
        i_Clus3_ID = cache.i_Clus3_ID;
        i_Clus4_ID = cache.i_Clus4_ID;
        i_Clus5_ID = cache.i_Clus5_ID;
        i_Clus6_ID = cache.i_Clus6_ID;
        i_Clus7_ID = cache.i_Clus7_ID;
        i_Clus8_ID = cache.i_Clus8_ID;
        i_Clus9_ID = cache.i_Clus9_ID;
        i_Clus10_ID = cache.i_Clus10_ID;
        i_Clus11_ID = cache.i_Clus11_ID;
        i_Clus12_ID = cache.i_Clus12_ID;
        i_prob = cache.i_prob;
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
    /// Clus1_ID: ID of cluster on track in layer 1
    [[nodiscard]] int Clus1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus1_ID, row));
    }
    /// Clus2_ID: ID of cluster on track in layer 2
    [[nodiscard]] int Clus2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus2_ID, row));
    }
    /// Clus3_ID: ID of cluster on track in layer 3
    [[nodiscard]] int Clus3_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus3_ID, row));
    }
    /// Clus4_ID: ID of cluster on track in layer 4
    [[nodiscard]] int Clus4_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus4_ID, row));
    }
    /// Clus5_ID: ID of cluster on track in layer 5
    [[nodiscard]] int Clus5_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus5_ID, row));
    }
    /// Clus6_ID: ID of cluster on track in layer 6
    [[nodiscard]] int Clus6_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus6_ID, row));
    }
    /// Clus7_ID: ID of cluster on track in layer 7
    [[nodiscard]] int Clus7_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus7_ID, row));
    }
    /// Clus8_ID: ID of cluster on track in layer 8
    [[nodiscard]] int Clus8_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus8_ID, row));
    }
    /// Clus9_ID: ID of cluster on track in layer 9
    [[nodiscard]] int Clus9_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus9_ID, row));
    }
    /// Clus10_ID: ID of cluster on track in layer 10
    [[nodiscard]] int Clus10_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus10_ID, row));
    }
    /// Clus11_ID: ID of cluster on track in layer 11
    [[nodiscard]] int Clus11_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus11_ID, row));
    }
    /// Clus12_ID: ID of cluster on track in layer 12
    [[nodiscard]] int Clus12_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Clus12_ID, row));
    }
    /// prob: AI seed probability
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_Clus1_ID = -1;
    int i_Clus2_ID = -1;
    int i_Clus3_ID = -1;
    int i_Clus4_ID = -1;
    int i_Clus5_ID = -1;
    int i_Clus6_ID = -1;
    int i_Clus7_ID = -1;
    int i_Clus8_ID = -1;
    int i_Clus9_ID = -1;
    int i_Clus10_ID = -1;
    int i_Clus11_ID = -1;
    int i_Clus12_ID = -1;
    int i_prob = -1;

    friend class Row;
};

/// Lightweight row view for CVTRec::SeedClusters. Stores a pointer + row index only.
class CVTRec_SeedClusters::Row {
 public:
    Row(const CVTRec_SeedClusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int Clus1_ID() const noexcept { return m_bank->Clus1_ID(m_row); }
    [[nodiscard]] int Clus2_ID() const noexcept { return m_bank->Clus2_ID(m_row); }
    [[nodiscard]] int Clus3_ID() const noexcept { return m_bank->Clus3_ID(m_row); }
    [[nodiscard]] int Clus4_ID() const noexcept { return m_bank->Clus4_ID(m_row); }
    [[nodiscard]] int Clus5_ID() const noexcept { return m_bank->Clus5_ID(m_row); }
    [[nodiscard]] int Clus6_ID() const noexcept { return m_bank->Clus6_ID(m_row); }
    [[nodiscard]] int Clus7_ID() const noexcept { return m_bank->Clus7_ID(m_row); }
    [[nodiscard]] int Clus8_ID() const noexcept { return m_bank->Clus8_ID(m_row); }
    [[nodiscard]] int Clus9_ID() const noexcept { return m_bank->Clus9_ID(m_row); }
    [[nodiscard]] int Clus10_ID() const noexcept { return m_bank->Clus10_ID(m_row); }
    [[nodiscard]] int Clus11_ID() const noexcept { return m_bank->Clus11_ID(m_row); }
    [[nodiscard]] int Clus12_ID() const noexcept { return m_bank->Clus12_ID(m_row); }
    [[nodiscard]] float prob() const noexcept { return m_bank->prob(m_row); }
 private:
    const CVTRec_SeedClusters* m_bank;
    int m_row;
};

inline CVTRec_SeedClusters::Row CVTRec_SeedClusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
