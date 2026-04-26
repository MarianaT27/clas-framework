// =============================================================================
// Auto-generated from tof.json - do not edit by hand.
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


/// HIPO bank wrapper for `FTOF::matchedclusters` (group 21200, item 33).
/// matched clusters from FTOF
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// FTOF_matchedclusters bank;
/// if (auto sc = bank.bind(event.get_bank("FTOF::matchedclusters")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto sector = row.sector();
/// }
/// @endcode
class FTOF_matchedclusters {
 public:
    static constexpr std::string_view bank_name = "FTOF::matchedclusters";
    static constexpr int bank_group = 21200;
    static constexpr int bank_item  = 33;

    class Row;

    FTOF_matchedclusters() = default;

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
            hipo::column_handle h_paddle_id1A;
            hipo::column_handle h_paddle_id1B;
            hipo::column_handle h_clus_1Aid;
            hipo::column_handle h_clus_1Bid;
            hipo::column_handle h_clusSize_1A;
            hipo::column_handle h_clusSize_1B;
            hipo::column_handle h_tminAlgo_1B_tCorr;
            hipo::column_handle h_midbarAlgo_1B_tCorr;
            hipo::column_handle h_EmaxAlgo_1B_tCorr;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("paddle_id1A"); h) cache.h_paddle_id1A = *h;
            else cache.h_paddle_id1A = {};
            if (auto h = schema.get_handle("paddle_id1B"); h) cache.h_paddle_id1B = *h;
            else cache.h_paddle_id1B = {};
            if (auto h = schema.get_handle("clus_1Aid"); h) cache.h_clus_1Aid = *h;
            else cache.h_clus_1Aid = {};
            if (auto h = schema.get_handle("clus_1Bid"); h) cache.h_clus_1Bid = *h;
            else cache.h_clus_1Bid = {};
            if (auto h = schema.get_handle("clusSize_1A"); h) cache.h_clusSize_1A = *h;
            else cache.h_clusSize_1A = {};
            if (auto h = schema.get_handle("clusSize_1B"); h) cache.h_clusSize_1B = *h;
            else cache.h_clusSize_1B = {};
            if (auto h = schema.get_handle("tminAlgo_1B_tCorr"); h) cache.h_tminAlgo_1B_tCorr = *h;
            else cache.h_tminAlgo_1B_tCorr = {};
            if (auto h = schema.get_handle("midbarAlgo_1B_tCorr"); h) cache.h_midbarAlgo_1B_tCorr = *h;
            else cache.h_midbarAlgo_1B_tCorr = {};
            if (auto h = schema.get_handle("EmaxAlgo_1B_tCorr"); h) cache.h_EmaxAlgo_1B_tCorr = *h;
            else cache.h_EmaxAlgo_1B_tCorr = {};
        }
        h_sector = cache.h_sector;
        h_paddle_id1A = cache.h_paddle_id1A;
        h_paddle_id1B = cache.h_paddle_id1B;
        h_clus_1Aid = cache.h_clus_1Aid;
        h_clus_1Bid = cache.h_clus_1Bid;
        h_clusSize_1A = cache.h_clusSize_1A;
        h_clusSize_1B = cache.h_clusSize_1B;
        h_tminAlgo_1B_tCorr = cache.h_tminAlgo_1B_tCorr;
        h_midbarAlgo_1B_tCorr = cache.h_midbarAlgo_1B_tCorr;
        h_EmaxAlgo_1B_tCorr = cache.h_EmaxAlgo_1B_tCorr;
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
    /// sector: sector of FTOF
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// paddle_id1A: paddle id of hit with lowest paddle id in cluster for panel 1A
    [[nodiscard]] int paddle_id1A(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_paddle_id1A, row));
    }
    /// paddle_id1B: paddle id of hit with lowest paddle id in cluster for panel 1B
    [[nodiscard]] int paddle_id1B(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_paddle_id1B, row));
    }
    /// clus_1Aid: id of cluster in 1A
    [[nodiscard]] int clus_1Aid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clus_1Aid, row));
    }
    /// clus_1Bid: id of cluster in 1B
    [[nodiscard]] int clus_1Bid(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clus_1Bid, row));
    }
    /// clusSize_1A: size of cluster in 1A
    [[nodiscard]] int clusSize_1A(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusSize_1A, row));
    }
    /// clusSize_1B: size of cluster in 1B
    [[nodiscard]] int clusSize_1B(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusSize_1B, row));
    }
    /// tminAlgo_1B_tCorr: corrected hit time using tmin algorithm to compute the path length between 1A and 1B
    [[nodiscard]] float tminAlgo_1B_tCorr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_tminAlgo_1B_tCorr, row));
    }
    /// midbarAlgo_1B_tCorr: corrected hit time using middle of bar algorithm to compute the path length between 1A and 1B
    [[nodiscard]] float midbarAlgo_1B_tCorr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_midbarAlgo_1B_tCorr, row));
    }
    /// EmaxAlgo_1B_tCorr: corrected hit time using Emax algorithm to compute the path length between 1A and 1B
    [[nodiscard]] float EmaxAlgo_1B_tCorr(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_EmaxAlgo_1B_tCorr, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// paddle_id1A column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> paddle_id1A_col() const noexcept {
        return m_view.column<int16_t>(h_paddle_id1A);
    }
    /// paddle_id1B column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> paddle_id1B_col() const noexcept {
        return m_view.column<int16_t>(h_paddle_id1B);
    }
    /// clus_1Aid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clus_1Aid_col() const noexcept {
        return m_view.column<int16_t>(h_clus_1Aid);
    }
    /// clus_1Bid column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clus_1Bid_col() const noexcept {
        return m_view.column<int16_t>(h_clus_1Bid);
    }
    /// clusSize_1A column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusSize_1A_col() const noexcept {
        return m_view.column<int16_t>(h_clusSize_1A);
    }
    /// clusSize_1B column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusSize_1B_col() const noexcept {
        return m_view.column<int16_t>(h_clusSize_1B);
    }
    /// tminAlgo_1B_tCorr column as a contiguous span.
    [[nodiscard]] std::span<const float> tminAlgo_1B_tCorr_col() const noexcept {
        return m_view.column<float>(h_tminAlgo_1B_tCorr);
    }
    /// midbarAlgo_1B_tCorr column as a contiguous span.
    [[nodiscard]] std::span<const float> midbarAlgo_1B_tCorr_col() const noexcept {
        return m_view.column<float>(h_midbarAlgo_1B_tCorr);
    }
    /// EmaxAlgo_1B_tCorr column as a contiguous span.
    [[nodiscard]] std::span<const float> EmaxAlgo_1B_tCorr_col() const noexcept {
        return m_view.column<float>(h_EmaxAlgo_1B_tCorr);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_paddle_id1A{};
    hipo::column_handle h_paddle_id1B{};
    hipo::column_handle h_clus_1Aid{};
    hipo::column_handle h_clus_1Bid{};
    hipo::column_handle h_clusSize_1A{};
    hipo::column_handle h_clusSize_1B{};
    hipo::column_handle h_tminAlgo_1B_tCorr{};
    hipo::column_handle h_midbarAlgo_1B_tCorr{};
    hipo::column_handle h_EmaxAlgo_1B_tCorr{};

    friend class Row;
};

/// Lightweight row view for FTOF::matchedclusters. Stores a pointer + row index only.
class FTOF_matchedclusters::Row {
 public:
    Row(const FTOF_matchedclusters* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int paddle_id1A() const noexcept { return m_bank->paddle_id1A(m_row); }
    [[nodiscard]] int paddle_id1B() const noexcept { return m_bank->paddle_id1B(m_row); }
    [[nodiscard]] int clus_1Aid() const noexcept { return m_bank->clus_1Aid(m_row); }
    [[nodiscard]] int clus_1Bid() const noexcept { return m_bank->clus_1Bid(m_row); }
    [[nodiscard]] int clusSize_1A() const noexcept { return m_bank->clusSize_1A(m_row); }
    [[nodiscard]] int clusSize_1B() const noexcept { return m_bank->clusSize_1B(m_row); }
    [[nodiscard]] float tminAlgo_1B_tCorr() const noexcept { return m_bank->tminAlgo_1B_tCorr(m_row); }
    [[nodiscard]] float midbarAlgo_1B_tCorr() const noexcept { return m_bank->midbarAlgo_1B_tCorr(m_row); }
    [[nodiscard]] float EmaxAlgo_1B_tCorr() const noexcept { return m_bank->EmaxAlgo_1B_tCorr(m_row); }
 private:
    const FTOF_matchedclusters* m_bank;
    int m_row;
};

inline FTOF_matchedclusters::Row FTOF_matchedclusters::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
