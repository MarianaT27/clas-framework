// =============================================================================
// Auto-generated from dc.json - do not edit by hand.
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


/// HIPO bank wrapper for `HitBasedTrkg::DNHits` (group 20600, item 48).
/// reconstructed hits using DC wire positions
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// HitBasedTrkg_DNHits bank;
/// if (auto sc = bank.bind(event.get_bank("HitBasedTrkg::DNHits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class HitBasedTrkg_DNHits {
 public:
    static constexpr std::string_view bank_name = "HitBasedTrkg::DNHits";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 48;

    class Row;

    HitBasedTrkg_DNHits() = default;

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
            hipo::column_handle h_indexTDC;
            hipo::column_handle h_status;
            hipo::column_handle h_sector;
            hipo::column_handle h_superlayer;
            hipo::column_handle h_layer;
            hipo::column_handle h_wire;
            hipo::column_handle h_TDC;
            hipo::column_handle h_jitter;
            hipo::column_handle h_trkDoca;
            hipo::column_handle h_docaError;
            hipo::column_handle h_LR;
            hipo::column_handle h_LocX;
            hipo::column_handle h_LocY;
            hipo::column_handle h_X;
            hipo::column_handle h_Z;
            hipo::column_handle h_clusterID;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("indexTDC"); h) cache.h_indexTDC = *h;
            else cache.h_indexTDC = {};
            if (auto h = schema.get_handle("status"); h) cache.h_status = *h;
            else cache.h_status = {};
            if (auto h = schema.get_handle("sector"); h) cache.h_sector = *h;
            else cache.h_sector = {};
            if (auto h = schema.get_handle("superlayer"); h) cache.h_superlayer = *h;
            else cache.h_superlayer = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("wire"); h) cache.h_wire = *h;
            else cache.h_wire = {};
            if (auto h = schema.get_handle("TDC"); h) cache.h_TDC = *h;
            else cache.h_TDC = {};
            if (auto h = schema.get_handle("jitter"); h) cache.h_jitter = *h;
            else cache.h_jitter = {};
            if (auto h = schema.get_handle("trkDoca"); h) cache.h_trkDoca = *h;
            else cache.h_trkDoca = {};
            if (auto h = schema.get_handle("docaError"); h) cache.h_docaError = *h;
            else cache.h_docaError = {};
            if (auto h = schema.get_handle("LR"); h) cache.h_LR = *h;
            else cache.h_LR = {};
            if (auto h = schema.get_handle("LocX"); h) cache.h_LocX = *h;
            else cache.h_LocX = {};
            if (auto h = schema.get_handle("LocY"); h) cache.h_LocY = *h;
            else cache.h_LocY = {};
            if (auto h = schema.get_handle("X"); h) cache.h_X = *h;
            else cache.h_X = {};
            if (auto h = schema.get_handle("Z"); h) cache.h_Z = *h;
            else cache.h_Z = {};
            if (auto h = schema.get_handle("clusterID"); h) cache.h_clusterID = *h;
            else cache.h_clusterID = {};
        }
        h_id = cache.h_id;
        h_indexTDC = cache.h_indexTDC;
        h_status = cache.h_status;
        h_sector = cache.h_sector;
        h_superlayer = cache.h_superlayer;
        h_layer = cache.h_layer;
        h_wire = cache.h_wire;
        h_TDC = cache.h_TDC;
        h_jitter = cache.h_jitter;
        h_trkDoca = cache.h_trkDoca;
        h_docaError = cache.h_docaError;
        h_LR = cache.h_LR;
        h_LocX = cache.h_LocX;
        h_LocY = cache.h_LocY;
        h_X = cache.h_X;
        h_Z = cache.h_Z;
        h_clusterID = cache.h_clusterID;
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
    /// id: id of the hit
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// indexTDC: index in the bank DC::TDC
    [[nodiscard]] int indexTDC(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_indexTDC, row));
    }
    /// status: id of the hit
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_status, row));
    }
    /// sector: DC sector
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// superlayer: DC superlayer (1...6)
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_superlayer, row));
    }
    /// layer: DC layer in superlayer (1...6)
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// wire: wire id of DC
    [[nodiscard]] int wire(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_wire, row));
    }
    /// TDC: raw time of the hit
    [[nodiscard]] int TDC(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_TDC, row));
    }
    /// jitter: time jitter (ns)
    [[nodiscard]] int jitter(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_jitter, row));
    }
    /// trkDoca: track doca of the hit (in cm)
    [[nodiscard]] float trkDoca(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_trkDoca, row));
    }
    /// docaError: error on track doca of the hit
    [[nodiscard]] float docaError(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_docaError, row));
    }
    /// LR: Left/Right ambiguity of the hit
    [[nodiscard]] int LR(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_LR, row));
    }
    /// LocX: x in planar local coordinate system
    [[nodiscard]] float LocX(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_LocX, row));
    }
    /// LocY: y in planar local coordinate system
    [[nodiscard]] float LocY(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_LocY, row));
    }
    /// X: wire x-coordinate  in tilted-sector
    [[nodiscard]] float X(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_X, row));
    }
    /// Z: wire z-coordinate  in tilted-sector
    [[nodiscard]] float Z(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_Z, row));
    }
    /// clusterID: ID of associated cluster
    [[nodiscard]] int clusterID(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_clusterID, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// indexTDC column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> indexTDC_col() const noexcept {
        return m_view.column<int16_t>(h_indexTDC);
    }
    /// status column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> status_col() const noexcept {
        return m_view.column<int16_t>(h_status);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// superlayer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> superlayer_col() const noexcept {
        return m_view.column<int8_t>(h_superlayer);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// wire column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> wire_col() const noexcept {
        return m_view.column<int16_t>(h_wire);
    }
    /// TDC column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> TDC_col() const noexcept {
        return m_view.column<int32_t>(h_TDC);
    }
    /// jitter column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> jitter_col() const noexcept {
        return m_view.column<int8_t>(h_jitter);
    }
    /// trkDoca column as a contiguous span.
    [[nodiscard]] std::span<const float> trkDoca_col() const noexcept {
        return m_view.column<float>(h_trkDoca);
    }
    /// docaError column as a contiguous span.
    [[nodiscard]] std::span<const float> docaError_col() const noexcept {
        return m_view.column<float>(h_docaError);
    }
    /// LR column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> LR_col() const noexcept {
        return m_view.column<int8_t>(h_LR);
    }
    /// LocX column as a contiguous span.
    [[nodiscard]] std::span<const float> LocX_col() const noexcept {
        return m_view.column<float>(h_LocX);
    }
    /// LocY column as a contiguous span.
    [[nodiscard]] std::span<const float> LocY_col() const noexcept {
        return m_view.column<float>(h_LocY);
    }
    /// X column as a contiguous span.
    [[nodiscard]] std::span<const float> X_col() const noexcept {
        return m_view.column<float>(h_X);
    }
    /// Z column as a contiguous span.
    [[nodiscard]] std::span<const float> Z_col() const noexcept {
        return m_view.column<float>(h_Z);
    }
    /// clusterID column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> clusterID_col() const noexcept {
        return m_view.column<int16_t>(h_clusterID);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_indexTDC{};
    hipo::column_handle h_status{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_superlayer{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_wire{};
    hipo::column_handle h_TDC{};
    hipo::column_handle h_jitter{};
    hipo::column_handle h_trkDoca{};
    hipo::column_handle h_docaError{};
    hipo::column_handle h_LR{};
    hipo::column_handle h_LocX{};
    hipo::column_handle h_LocY{};
    hipo::column_handle h_X{};
    hipo::column_handle h_Z{};
    hipo::column_handle h_clusterID{};

    friend class Row;
};

/// Lightweight row view for HitBasedTrkg::DNHits. Stores a pointer + row index only.
class HitBasedTrkg_DNHits::Row {
 public:
    Row(const HitBasedTrkg_DNHits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int indexTDC() const noexcept { return m_bank->indexTDC(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int wire() const noexcept { return m_bank->wire(m_row); }
    [[nodiscard]] int TDC() const noexcept { return m_bank->TDC(m_row); }
    [[nodiscard]] int jitter() const noexcept { return m_bank->jitter(m_row); }
    [[nodiscard]] float trkDoca() const noexcept { return m_bank->trkDoca(m_row); }
    [[nodiscard]] float docaError() const noexcept { return m_bank->docaError(m_row); }
    [[nodiscard]] int LR() const noexcept { return m_bank->LR(m_row); }
    [[nodiscard]] float LocX() const noexcept { return m_bank->LocX(m_row); }
    [[nodiscard]] float LocY() const noexcept { return m_bank->LocY(m_row); }
    [[nodiscard]] float X() const noexcept { return m_bank->X(m_row); }
    [[nodiscard]] float Z() const noexcept { return m_bank->Z(m_row); }
    [[nodiscard]] int clusterID() const noexcept { return m_bank->clusterID(m_row); }
 private:
    const HitBasedTrkg_DNHits* m_bank;
    int m_row;
};

inline HitBasedTrkg_DNHits::Row HitBasedTrkg_DNHits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
