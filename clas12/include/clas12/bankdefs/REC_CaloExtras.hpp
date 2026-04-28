// =============================================================================
// Auto-generated from event.json - do not edit by hand.
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


/// HIPO bank wrapper for `REC::CaloExtras` (group 300, item 46).
/// Calorimeter Cluster Parameters
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// REC_CaloExtras bank;
/// if (auto sc = bank.bind(event.get_bank("REC::CaloExtras")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto dbstU = row.dbstU();
/// }
/// @endcode
class REC_CaloExtras {
 public:
    static constexpr std::string_view bank_name = "REC::CaloExtras";
    static constexpr int bank_group = 300;
    static constexpr int bank_item  = 46;

    class Row;

    REC_CaloExtras() = default;

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
            hipo::column_handle h_dbstU;
            hipo::column_handle h_dbstV;
            hipo::column_handle h_dbstW;
            hipo::column_handle h_rawEU;
            hipo::column_handle h_rawEV;
            hipo::column_handle h_rawEW;
            hipo::column_handle h_recEU;
            hipo::column_handle h_recEV;
            hipo::column_handle h_recEW;
            hipo::column_handle h_recDTU;
            hipo::column_handle h_recDTV;
            hipo::column_handle h_recDTW;
            hipo::column_handle h_recFTU;
            hipo::column_handle h_recFTV;
            hipo::column_handle h_recFTW;
            hipo::column_handle h_size;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("dbstU"); h) cache.h_dbstU = *h;
            else cache.h_dbstU = {};
            if (auto h = schema.get_handle("dbstV"); h) cache.h_dbstV = *h;
            else cache.h_dbstV = {};
            if (auto h = schema.get_handle("dbstW"); h) cache.h_dbstW = *h;
            else cache.h_dbstW = {};
            if (auto h = schema.get_handle("rawEU"); h) cache.h_rawEU = *h;
            else cache.h_rawEU = {};
            if (auto h = schema.get_handle("rawEV"); h) cache.h_rawEV = *h;
            else cache.h_rawEV = {};
            if (auto h = schema.get_handle("rawEW"); h) cache.h_rawEW = *h;
            else cache.h_rawEW = {};
            if (auto h = schema.get_handle("recEU"); h) cache.h_recEU = *h;
            else cache.h_recEU = {};
            if (auto h = schema.get_handle("recEV"); h) cache.h_recEV = *h;
            else cache.h_recEV = {};
            if (auto h = schema.get_handle("recEW"); h) cache.h_recEW = *h;
            else cache.h_recEW = {};
            if (auto h = schema.get_handle("recDTU"); h) cache.h_recDTU = *h;
            else cache.h_recDTU = {};
            if (auto h = schema.get_handle("recDTV"); h) cache.h_recDTV = *h;
            else cache.h_recDTV = {};
            if (auto h = schema.get_handle("recDTW"); h) cache.h_recDTW = *h;
            else cache.h_recDTW = {};
            if (auto h = schema.get_handle("recFTU"); h) cache.h_recFTU = *h;
            else cache.h_recFTU = {};
            if (auto h = schema.get_handle("recFTV"); h) cache.h_recFTV = *h;
            else cache.h_recFTV = {};
            if (auto h = schema.get_handle("recFTW"); h) cache.h_recFTW = *h;
            else cache.h_recFTW = {};
            if (auto h = schema.get_handle("size"); h) cache.h_size = *h;
            else cache.h_size = {};
        }
        h_dbstU = cache.h_dbstU;
        h_dbstV = cache.h_dbstV;
        h_dbstW = cache.h_dbstW;
        h_rawEU = cache.h_rawEU;
        h_rawEV = cache.h_rawEV;
        h_rawEW = cache.h_rawEW;
        h_recEU = cache.h_recEU;
        h_recEV = cache.h_recEV;
        h_recEW = cache.h_recEW;
        h_recDTU = cache.h_recDTU;
        h_recDTV = cache.h_recDTV;
        h_recDTW = cache.h_recDTW;
        h_recFTU = cache.h_recFTU;
        h_recFTV = cache.h_recFTV;
        h_recFTW = cache.h_recFTW;
        h_size = cache.h_size;
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
    /// dbstU: DB status + U strip*10
    [[nodiscard]] int dbstU(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_dbstU, row));
    }
    /// dbstV: DB status + V strip*10
    [[nodiscard]] int dbstV(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_dbstV, row));
    }
    /// dbstW: DB status + W strip*10
    [[nodiscard]] int dbstW(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_dbstW, row));
    }
    /// rawEU: raw U peak energy
    [[nodiscard]] float rawEU(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_rawEU, row));
    }
    /// rawEV: raw V peak energy
    [[nodiscard]] float rawEV(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_rawEV, row));
    }
    /// rawEW: raw W peak energy
    [[nodiscard]] float rawEW(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_rawEW, row));
    }
    /// recEU: recon U peak energy (GeV)
    [[nodiscard]] float recEU(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recEU, row));
    }
    /// recEV: recon V peak energy (GeV)
    [[nodiscard]] float recEV(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recEV, row));
    }
    /// recEW: recon W peak energy (GeV)
    [[nodiscard]] float recEW(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recEW, row));
    }
    /// recDTU: recon U peak TDC time (ns)
    [[nodiscard]] float recDTU(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recDTU, row));
    }
    /// recDTV: recon V peak TDC time (ns)
    [[nodiscard]] float recDTV(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recDTV, row));
    }
    /// recDTW: recon W peak TDC time (ns)
    [[nodiscard]] float recDTW(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recDTW, row));
    }
    /// recFTU: recon U peak FADC time (ns)
    [[nodiscard]] float recFTU(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recFTU, row));
    }
    /// recFTV: recon V peak FADC time (ns)
    [[nodiscard]] float recFTV(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recFTV, row));
    }
    /// recFTW: recon W peak FADC time (ns)
    [[nodiscard]] float recFTW(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_recFTW, row));
    }
    /// size: cluster size
    [[nodiscard]] float size(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_size, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// dbstU column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> dbstU_col() const noexcept {
        return m_view.column<int16_t>(h_dbstU);
    }
    /// dbstV column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> dbstV_col() const noexcept {
        return m_view.column<int16_t>(h_dbstV);
    }
    /// dbstW column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> dbstW_col() const noexcept {
        return m_view.column<int16_t>(h_dbstW);
    }
    /// rawEU column as a contiguous span.
    [[nodiscard]] std::span<const float> rawEU_col() const noexcept {
        return m_view.column<float>(h_rawEU);
    }
    /// rawEV column as a contiguous span.
    [[nodiscard]] std::span<const float> rawEV_col() const noexcept {
        return m_view.column<float>(h_rawEV);
    }
    /// rawEW column as a contiguous span.
    [[nodiscard]] std::span<const float> rawEW_col() const noexcept {
        return m_view.column<float>(h_rawEW);
    }
    /// recEU column as a contiguous span.
    [[nodiscard]] std::span<const float> recEU_col() const noexcept {
        return m_view.column<float>(h_recEU);
    }
    /// recEV column as a contiguous span.
    [[nodiscard]] std::span<const float> recEV_col() const noexcept {
        return m_view.column<float>(h_recEV);
    }
    /// recEW column as a contiguous span.
    [[nodiscard]] std::span<const float> recEW_col() const noexcept {
        return m_view.column<float>(h_recEW);
    }
    /// recDTU column as a contiguous span.
    [[nodiscard]] std::span<const float> recDTU_col() const noexcept {
        return m_view.column<float>(h_recDTU);
    }
    /// recDTV column as a contiguous span.
    [[nodiscard]] std::span<const float> recDTV_col() const noexcept {
        return m_view.column<float>(h_recDTV);
    }
    /// recDTW column as a contiguous span.
    [[nodiscard]] std::span<const float> recDTW_col() const noexcept {
        return m_view.column<float>(h_recDTW);
    }
    /// recFTU column as a contiguous span.
    [[nodiscard]] std::span<const float> recFTU_col() const noexcept {
        return m_view.column<float>(h_recFTU);
    }
    /// recFTV column as a contiguous span.
    [[nodiscard]] std::span<const float> recFTV_col() const noexcept {
        return m_view.column<float>(h_recFTV);
    }
    /// recFTW column as a contiguous span.
    [[nodiscard]] std::span<const float> recFTW_col() const noexcept {
        return m_view.column<float>(h_recFTW);
    }
    /// size column as a contiguous span.
    [[nodiscard]] std::span<const float> size_col() const noexcept {
        return m_view.column<float>(h_size);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_dbstU{};
    hipo::column_handle h_dbstV{};
    hipo::column_handle h_dbstW{};
    hipo::column_handle h_rawEU{};
    hipo::column_handle h_rawEV{};
    hipo::column_handle h_rawEW{};
    hipo::column_handle h_recEU{};
    hipo::column_handle h_recEV{};
    hipo::column_handle h_recEW{};
    hipo::column_handle h_recDTU{};
    hipo::column_handle h_recDTV{};
    hipo::column_handle h_recDTW{};
    hipo::column_handle h_recFTU{};
    hipo::column_handle h_recFTV{};
    hipo::column_handle h_recFTW{};
    hipo::column_handle h_size{};

    friend class Row;
};

/// Lightweight row view for REC::CaloExtras. Stores a pointer + row index only.
class REC_CaloExtras::Row {
 public:
    Row(const REC_CaloExtras* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int dbstU() const noexcept { return m_bank->dbstU(m_row); }
    [[nodiscard]] int dbstV() const noexcept { return m_bank->dbstV(m_row); }
    [[nodiscard]] int dbstW() const noexcept { return m_bank->dbstW(m_row); }
    [[nodiscard]] float rawEU() const noexcept { return m_bank->rawEU(m_row); }
    [[nodiscard]] float rawEV() const noexcept { return m_bank->rawEV(m_row); }
    [[nodiscard]] float rawEW() const noexcept { return m_bank->rawEW(m_row); }
    [[nodiscard]] float recEU() const noexcept { return m_bank->recEU(m_row); }
    [[nodiscard]] float recEV() const noexcept { return m_bank->recEV(m_row); }
    [[nodiscard]] float recEW() const noexcept { return m_bank->recEW(m_row); }
    [[nodiscard]] float recDTU() const noexcept { return m_bank->recDTU(m_row); }
    [[nodiscard]] float recDTV() const noexcept { return m_bank->recDTV(m_row); }
    [[nodiscard]] float recDTW() const noexcept { return m_bank->recDTW(m_row); }
    [[nodiscard]] float recFTU() const noexcept { return m_bank->recFTU(m_row); }
    [[nodiscard]] float recFTV() const noexcept { return m_bank->recFTV(m_row); }
    [[nodiscard]] float recFTW() const noexcept { return m_bank->recFTW(m_row); }
    [[nodiscard]] float size() const noexcept { return m_bank->size(m_row); }
 private:
    const REC_CaloExtras* m_bank;
    int m_row;
};

inline REC_CaloExtras::Row REC_CaloExtras::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
