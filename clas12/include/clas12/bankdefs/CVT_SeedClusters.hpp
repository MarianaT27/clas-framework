// =============================================================================
// Auto-generated from cvt.json - do not edit by hand.
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


/// HIPO bank wrapper for `CVT::SeedClusters` (group 20500, item 43).
/// Ids of clusters on track for 12 layers of CVT
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// CVT_SeedClusters bank;
/// if (auto sc = bank.bind(event.get_bank("CVT::SeedClusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class CVT_SeedClusters {
 public:
    static constexpr std::string_view bank_name = "CVT::SeedClusters";
    static constexpr int bank_group = 20500;
    static constexpr int bank_item  = 43;

    class Row;

    CVT_SeedClusters() = default;

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
            hipo::column_handle h_Clus1_ID;
            hipo::column_handle h_Clus2_ID;
            hipo::column_handle h_Clus3_ID;
            hipo::column_handle h_Clus4_ID;
            hipo::column_handle h_Clus5_ID;
            hipo::column_handle h_Clus6_ID;
            hipo::column_handle h_Clus7_ID;
            hipo::column_handle h_Clus8_ID;
            hipo::column_handle h_Clus9_ID;
            hipo::column_handle h_Clus10_ID;
            hipo::column_handle h_Clus11_ID;
            hipo::column_handle h_Clus12_ID;
            hipo::column_handle h_prob;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("Clus1_ID"); h) cache.h_Clus1_ID = *h;
            else cache.h_Clus1_ID = {};
            if (auto h = schema.get_handle("Clus2_ID"); h) cache.h_Clus2_ID = *h;
            else cache.h_Clus2_ID = {};
            if (auto h = schema.get_handle("Clus3_ID"); h) cache.h_Clus3_ID = *h;
            else cache.h_Clus3_ID = {};
            if (auto h = schema.get_handle("Clus4_ID"); h) cache.h_Clus4_ID = *h;
            else cache.h_Clus4_ID = {};
            if (auto h = schema.get_handle("Clus5_ID"); h) cache.h_Clus5_ID = *h;
            else cache.h_Clus5_ID = {};
            if (auto h = schema.get_handle("Clus6_ID"); h) cache.h_Clus6_ID = *h;
            else cache.h_Clus6_ID = {};
            if (auto h = schema.get_handle("Clus7_ID"); h) cache.h_Clus7_ID = *h;
            else cache.h_Clus7_ID = {};
            if (auto h = schema.get_handle("Clus8_ID"); h) cache.h_Clus8_ID = *h;
            else cache.h_Clus8_ID = {};
            if (auto h = schema.get_handle("Clus9_ID"); h) cache.h_Clus9_ID = *h;
            else cache.h_Clus9_ID = {};
            if (auto h = schema.get_handle("Clus10_ID"); h) cache.h_Clus10_ID = *h;
            else cache.h_Clus10_ID = {};
            if (auto h = schema.get_handle("Clus11_ID"); h) cache.h_Clus11_ID = *h;
            else cache.h_Clus11_ID = {};
            if (auto h = schema.get_handle("Clus12_ID"); h) cache.h_Clus12_ID = *h;
            else cache.h_Clus12_ID = {};
            if (auto h = schema.get_handle("prob"); h) cache.h_prob = *h;
            else cache.h_prob = {};
        }
        h_id = cache.h_id;
        h_Clus1_ID = cache.h_Clus1_ID;
        h_Clus2_ID = cache.h_Clus2_ID;
        h_Clus3_ID = cache.h_Clus3_ID;
        h_Clus4_ID = cache.h_Clus4_ID;
        h_Clus5_ID = cache.h_Clus5_ID;
        h_Clus6_ID = cache.h_Clus6_ID;
        h_Clus7_ID = cache.h_Clus7_ID;
        h_Clus8_ID = cache.h_Clus8_ID;
        h_Clus9_ID = cache.h_Clus9_ID;
        h_Clus10_ID = cache.h_Clus10_ID;
        h_Clus11_ID = cache.h_Clus11_ID;
        h_Clus12_ID = cache.h_Clus12_ID;
        h_prob = cache.h_prob;
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
    /// Clus1_ID: ID of cluster on track in layer 1
    [[nodiscard]] int Clus1_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus1_ID, row));
    }
    /// Clus2_ID: ID of cluster on track in layer 2
    [[nodiscard]] int Clus2_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus2_ID, row));
    }
    /// Clus3_ID: ID of cluster on track in layer 3
    [[nodiscard]] int Clus3_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus3_ID, row));
    }
    /// Clus4_ID: ID of cluster on track in layer 4
    [[nodiscard]] int Clus4_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus4_ID, row));
    }
    /// Clus5_ID: ID of cluster on track in layer 5
    [[nodiscard]] int Clus5_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus5_ID, row));
    }
    /// Clus6_ID: ID of cluster on track in layer 6
    [[nodiscard]] int Clus6_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus6_ID, row));
    }
    /// Clus7_ID: ID of cluster on track in layer 7
    [[nodiscard]] int Clus7_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus7_ID, row));
    }
    /// Clus8_ID: ID of cluster on track in layer 8
    [[nodiscard]] int Clus8_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus8_ID, row));
    }
    /// Clus9_ID: ID of cluster on track in layer 9
    [[nodiscard]] int Clus9_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus9_ID, row));
    }
    /// Clus10_ID: ID of cluster on track in layer 10
    [[nodiscard]] int Clus10_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus10_ID, row));
    }
    /// Clus11_ID: ID of cluster on track in layer 11
    [[nodiscard]] int Clus11_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus11_ID, row));
    }
    /// Clus12_ID: ID of cluster on track in layer 12
    [[nodiscard]] int Clus12_ID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_Clus12_ID, row));
    }
    /// prob: AI seed probability
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// Clus1_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus1_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus1_ID);
    }
    /// Clus2_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus2_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus2_ID);
    }
    /// Clus3_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus3_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus3_ID);
    }
    /// Clus4_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus4_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus4_ID);
    }
    /// Clus5_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus5_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus5_ID);
    }
    /// Clus6_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus6_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus6_ID);
    }
    /// Clus7_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus7_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus7_ID);
    }
    /// Clus8_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus8_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus8_ID);
    }
    /// Clus9_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus9_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus9_ID);
    }
    /// Clus10_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus10_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus10_ID);
    }
    /// Clus11_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus11_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus11_ID);
    }
    /// Clus12_ID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> Clus12_ID_col() const noexcept {
        return m_view.column<int16_t>(h_Clus12_ID);
    }
    /// prob column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_col() const noexcept {
        return m_view.column<float>(h_prob);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_Clus1_ID{};
    hipo::column_handle h_Clus2_ID{};
    hipo::column_handle h_Clus3_ID{};
    hipo::column_handle h_Clus4_ID{};
    hipo::column_handle h_Clus5_ID{};
    hipo::column_handle h_Clus6_ID{};
    hipo::column_handle h_Clus7_ID{};
    hipo::column_handle h_Clus8_ID{};
    hipo::column_handle h_Clus9_ID{};
    hipo::column_handle h_Clus10_ID{};
    hipo::column_handle h_Clus11_ID{};
    hipo::column_handle h_Clus12_ID{};
    hipo::column_handle h_prob{};

    friend class Row;
};

/// Lightweight row view for CVT::SeedClusters. Stores a pointer + row index only.
class CVT_SeedClusters::Row {
 public:
    Row(const CVT_SeedClusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
    const CVT_SeedClusters* m_bank;
    int m_row;
};

inline CVT_SeedClusters::Row CVT_SeedClusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
