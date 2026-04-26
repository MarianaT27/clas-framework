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


/// HIPO bank wrapper for `AHDC::hits` (group 23000, item 23).
/// Raw Hits
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// AHDC_hits bank;
/// if (auto sc = bank.bind(event.get_bank("AHDC::hits")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class AHDC_hits {
 public:
    static constexpr std::string_view bank_name = "AHDC::hits";
    static constexpr int bank_group = 23000;
    static constexpr int bank_item  = 23;

    class Row;

    AHDC_hits() = default;

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
            hipo::column_handle h_layer;
            hipo::column_handle h_superlayer;
            hipo::column_handle h_wire;
            hipo::column_handle h_doca;
            hipo::column_handle h_residual;
            hipo::column_handle h_time;
            hipo::column_handle h_adc;
            hipo::column_handle h_timeOverThreshold;
            hipo::column_handle h_trackid;
        };
        static thread_local HandleCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            if (auto h = schema.get_handle("id"); h) cache.h_id = *h;
            else cache.h_id = {};
            if (auto h = schema.get_handle("layer"); h) cache.h_layer = *h;
            else cache.h_layer = {};
            if (auto h = schema.get_handle("superlayer"); h) cache.h_superlayer = *h;
            else cache.h_superlayer = {};
            if (auto h = schema.get_handle("wire"); h) cache.h_wire = *h;
            else cache.h_wire = {};
            if (auto h = schema.get_handle("doca"); h) cache.h_doca = *h;
            else cache.h_doca = {};
            if (auto h = schema.get_handle("residual"); h) cache.h_residual = *h;
            else cache.h_residual = {};
            if (auto h = schema.get_handle("time"); h) cache.h_time = *h;
            else cache.h_time = {};
            if (auto h = schema.get_handle("adc"); h) cache.h_adc = *h;
            else cache.h_adc = {};
            if (auto h = schema.get_handle("timeOverThreshold"); h) cache.h_timeOverThreshold = *h;
            else cache.h_timeOverThreshold = {};
            if (auto h = schema.get_handle("trackid"); h) cache.h_trackid = *h;
            else cache.h_trackid = {};
        }
        h_id = cache.h_id;
        h_layer = cache.h_layer;
        h_superlayer = cache.h_superlayer;
        h_wire = cache.h_wire;
        h_doca = cache.h_doca;
        h_residual = cache.h_residual;
        h_time = cache.h_time;
        h_adc = cache.h_adc;
        h_timeOverThreshold = cache.h_timeOverThreshold;
        h_trackid = cache.h_trackid;
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
    /// id: hit id
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_id, row));
    }
    /// layer: layer number
    [[nodiscard]] int layer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_layer, row));
    }
    /// superlayer: superlayer number
    [[nodiscard]] int superlayer(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_superlayer, row));
    }
    /// wire: wire number
    [[nodiscard]] int wire(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_wire, row));
    }
    /// doca: distance of closest approch (mm) is actually the distance from time2distance calibration
    [[nodiscard]] double doca(int row) const noexcept {
        return static_cast<double>(m_view.get<double>(h_doca, row));
    }
    /// residual: residual (mm) calculated by the Kalman Filter
    [[nodiscard]] double residual(int row) const noexcept {
        return static_cast<double>(m_view.get<double>(h_residual, row));
    }
    /// time: calibrated time (ns)
    [[nodiscard]] double time(int row) const noexcept {
        return static_cast<double>(m_view.get<double>(h_time, row));
    }
    /// adc: calibrated ADC
    [[nodiscard]] int adc(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_adc, row));
    }
    /// timeOverThreshold: calibrated time over threshold (ns)
    [[nodiscard]] double timeOverThreshold(int row) const noexcept {
        return static_cast<double>(m_view.get<double>(h_timeOverThreshold, row));
    }
    /// trackid: track id, is -1 if not associated to a Track
    [[nodiscard]] int trackid(int row) const noexcept {
        return static_cast<int>(m_view.get<int32_t>(h_trackid, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> id_col() const noexcept {
        return m_view.column<int16_t>(h_id);
    }
    /// layer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> layer_col() const noexcept {
        return m_view.column<int8_t>(h_layer);
    }
    /// superlayer column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> superlayer_col() const noexcept {
        return m_view.column<int8_t>(h_superlayer);
    }
    /// wire column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> wire_col() const noexcept {
        return m_view.column<int32_t>(h_wire);
    }
    /// doca column as a contiguous span.
    [[nodiscard]] std::span<const double> doca_col() const noexcept {
        return m_view.column<double>(h_doca);
    }
    /// residual column as a contiguous span.
    [[nodiscard]] std::span<const double> residual_col() const noexcept {
        return m_view.column<double>(h_residual);
    }
    /// time column as a contiguous span.
    [[nodiscard]] std::span<const double> time_col() const noexcept {
        return m_view.column<double>(h_time);
    }
    /// adc column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> adc_col() const noexcept {
        return m_view.column<int32_t>(h_adc);
    }
    /// timeOverThreshold column as a contiguous span.
    [[nodiscard]] std::span<const double> timeOverThreshold_col() const noexcept {
        return m_view.column<double>(h_timeOverThreshold);
    }
    /// trackid column as a contiguous span.
    [[nodiscard]] std::span<const int32_t> trackid_col() const noexcept {
        return m_view.column<int32_t>(h_trackid);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_layer{};
    hipo::column_handle h_superlayer{};
    hipo::column_handle h_wire{};
    hipo::column_handle h_doca{};
    hipo::column_handle h_residual{};
    hipo::column_handle h_time{};
    hipo::column_handle h_adc{};
    hipo::column_handle h_timeOverThreshold{};
    hipo::column_handle h_trackid{};

    friend class Row;
};

/// Lightweight row view for AHDC::hits. Stores a pointer + row index only.
class AHDC_hits::Row {
 public:
    Row(const AHDC_hits* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int layer() const noexcept { return m_bank->layer(m_row); }
    [[nodiscard]] int superlayer() const noexcept { return m_bank->superlayer(m_row); }
    [[nodiscard]] int wire() const noexcept { return m_bank->wire(m_row); }
    [[nodiscard]] double doca() const noexcept { return m_bank->doca(m_row); }
    [[nodiscard]] double residual() const noexcept { return m_bank->residual(m_row); }
    [[nodiscard]] double time() const noexcept { return m_bank->time(m_row); }
    [[nodiscard]] int adc() const noexcept { return m_bank->adc(m_row); }
    [[nodiscard]] double timeOverThreshold() const noexcept { return m_bank->timeOverThreshold(m_row); }
    [[nodiscard]] int trackid() const noexcept { return m_bank->trackid(m_row); }
 private:
    const AHDC_hits* m_bank;
    int m_row;
};

inline AHDC_hits::Row AHDC_hits::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
