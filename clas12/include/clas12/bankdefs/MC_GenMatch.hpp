// =============================================================================
// Auto-generated from mc.json - do not edit by hand.
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


/// HIPO bank wrapper for `MC::GenMatch` (group 40, item 6).
/// MC::Particle -> REC::Particle matching
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// MC_GenMatch bank;
/// if (auto sc = bank.bind(event.get_bank("MC::GenMatch")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto mcindex = row.mcindex();
/// }
/// @endcode
class MC_GenMatch {
 public:
    static constexpr std::string_view bank_name = "MC::GenMatch";
    static constexpr int bank_group = 40;
    static constexpr int bank_item  = 6;

    class Row;

    MC_GenMatch() = default;

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
            hipo::column_handle h_mcindex;
            hipo::column_handle h_pindex;
            hipo::column_handle h_mclayer1;
            hipo::column_handle h_mclayer2;
            hipo::column_handle h_player1;
            hipo::column_handle h_player2;
            hipo::column_handle h_quality;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("mcindex"); h) cache.h_mcindex = *h;
            else cache.h_mcindex = {};
            if (auto h = schema.get_handle("pindex"); h) cache.h_pindex = *h;
            else cache.h_pindex = {};
            if (auto h = schema.get_handle("mclayer1"); h) cache.h_mclayer1 = *h;
            else cache.h_mclayer1 = {};
            if (auto h = schema.get_handle("mclayer2"); h) cache.h_mclayer2 = *h;
            else cache.h_mclayer2 = {};
            if (auto h = schema.get_handle("player1"); h) cache.h_player1 = *h;
            else cache.h_player1 = {};
            if (auto h = schema.get_handle("player2"); h) cache.h_player2 = *h;
            else cache.h_player2 = {};
            if (auto h = schema.get_handle("quality"); h) cache.h_quality = *h;
            else cache.h_quality = {};
        }
        h_mcindex = cache.h_mcindex;
        h_pindex = cache.h_pindex;
        h_mclayer1 = cache.h_mclayer1;
        h_mclayer2 = cache.h_mclayer2;
        h_player1 = cache.h_player1;
        h_player2 = cache.h_player2;
        h_quality = cache.h_quality;
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
    /// mcindex: MC::Particle index
    [[nodiscard]] int mcindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_mcindex, row));
    }
    /// pindex: REC::Particle index
    [[nodiscard]] int pindex(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_pindex, row));
    }
    /// mclayer1: layers from the 1st set of detectors hit by MC particle
    [[nodiscard]] int64_t mclayer1(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_mclayer1, row));
    }
    /// mclayer2: layers from the 2nd set of detectors hit by MC particle
    [[nodiscard]] int64_t mclayer2(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_mclayer2, row));
    }
    /// player1: layers from the 1st set of detectors hit by Recon particle
    [[nodiscard]] int64_t player1(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_player1, row));
    }
    /// player2: layers from the 2nd set of detectors hit by Recon particle
    [[nodiscard]] int64_t player2(int row) const noexcept {
        return static_cast<int64_t>(m_view.get<int64_t>(h_player2, row));
    }
    /// quality: matching quality parameter
    [[nodiscard]] float quality(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_quality, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// mcindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> mcindex_col() const noexcept {
        return m_view.column<int16_t>(h_mcindex);
    }
    /// pindex column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> pindex_col() const noexcept {
        return m_view.column<int16_t>(h_pindex);
    }
    /// mclayer1 column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> mclayer1_col() const noexcept {
        return m_view.column<int64_t>(h_mclayer1);
    }
    /// mclayer2 column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> mclayer2_col() const noexcept {
        return m_view.column<int64_t>(h_mclayer2);
    }
    /// player1 column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> player1_col() const noexcept {
        return m_view.column<int64_t>(h_player1);
    }
    /// player2 column as a contiguous span.
    [[nodiscard]] std::span<const int64_t> player2_col() const noexcept {
        return m_view.column<int64_t>(h_player2);
    }
    /// quality column as a contiguous span.
    [[nodiscard]] std::span<const float> quality_col() const noexcept {
        return m_view.column<float>(h_quality);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

    // -------------------------------------------------------------------
    // Link-column handles (used by PindexLinker-style helpers to
    // read the whole index column as a typed span in one shot)
    // -------------------------------------------------------------------
    [[nodiscard]] hipo::column_handle mcindex_handle() const noexcept { return h_mcindex; }
    [[nodiscard]] hipo::column_handle pindex_handle() const noexcept { return h_pindex; }

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_mcindex{};
    hipo::column_handle h_pindex{};
    hipo::column_handle h_mclayer1{};
    hipo::column_handle h_mclayer2{};
    hipo::column_handle h_player1{};
    hipo::column_handle h_player2{};
    hipo::column_handle h_quality{};

    friend class Row;
};

/// Lightweight row view for MC::GenMatch. Stores a pointer + row index only.
class MC_GenMatch::Row {
 public:
    Row(const MC_GenMatch* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int mcindex() const noexcept { return m_bank->mcindex(m_row); }
    [[nodiscard]] int pindex() const noexcept { return m_bank->pindex(m_row); }
    [[nodiscard]] int64_t mclayer1() const noexcept { return m_bank->mclayer1(m_row); }
    [[nodiscard]] int64_t mclayer2() const noexcept { return m_bank->mclayer2(m_row); }
    [[nodiscard]] int64_t player1() const noexcept { return m_bank->player1(m_row); }
    [[nodiscard]] int64_t player2() const noexcept { return m_bank->player2(m_row); }
    [[nodiscard]] float quality() const noexcept { return m_bank->quality(m_row); }
 private:
    const MC_GenMatch* m_bank;
    int m_row;
};

inline MC_GenMatch::Row MC_GenMatch::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
