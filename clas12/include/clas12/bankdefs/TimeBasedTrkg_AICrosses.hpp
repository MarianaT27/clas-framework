// =============================================================================
// Auto-generated from dcnn.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `TimeBasedTrkg::AICrosses` (group 20600, item 75).
/// reconstructed segments using DC timing information
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// TimeBasedTrkg_AICrosses bank;
/// if (auto sc = bank.bind(event.get_bank("TimeBasedTrkg::AICrosses")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class TimeBasedTrkg_AICrosses {
 public:
    static constexpr std::string_view bank_name = "TimeBasedTrkg::AICrosses";
    static constexpr int bank_group = 20600;
    static constexpr int bank_item  = 75;

    class Row;

    TimeBasedTrkg_AICrosses() = default;

    /// Bind to a live hipo::bank. Per-thread cache keyed by schema pointer:
    /// the first bind() in a thread resolves every column name to its item
    /// index via schema.getEntryOrder(); subsequent bind()s for the same
    /// schema hit the cache. Returns true on success.
    bool bind(const hipo::bank& view) noexcept {
        m_view = &view;
        // const_cast is OK: hipo::bank::getSchema() lacks a const overload
        // but only reads schemaEntriesMap; we never mutate the bank here.
        auto& schema = const_cast<hipo::bank&>(view).getSchema();
        struct ItemCache {
            const hipo::schema* schema_ptr = nullptr;
            int i_id = -1;
            int i_status = -1;
            int i_sector = -1;
            int i_region = -1;
            int i_x = -1;
            int i_y = -1;
            int i_z = -1;
            int i_err_x = -1;
            int i_err_y = -1;
            int i_err_z = -1;
            int i_ux = -1;
            int i_uy = -1;
            int i_uz = -1;
            int i_err_ux = -1;
            int i_err_uy = -1;
            int i_err_uz = -1;
            int i_Segment1_ID = -1;
            int i_Segment2_ID = -1;
        };
        static thread_local ItemCache cache;
        if (cache.schema_ptr != &schema) {
            cache.schema_ptr = &schema;
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_status = schema.exists("status") ? schema.getEntryOrder("status") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_region = schema.exists("region") ? schema.getEntryOrder("region") : -1;
            cache.i_x = schema.exists("x") ? schema.getEntryOrder("x") : -1;
            cache.i_y = schema.exists("y") ? schema.getEntryOrder("y") : -1;
            cache.i_z = schema.exists("z") ? schema.getEntryOrder("z") : -1;
            cache.i_err_x = schema.exists("err_x") ? schema.getEntryOrder("err_x") : -1;
            cache.i_err_y = schema.exists("err_y") ? schema.getEntryOrder("err_y") : -1;
            cache.i_err_z = schema.exists("err_z") ? schema.getEntryOrder("err_z") : -1;
            cache.i_ux = schema.exists("ux") ? schema.getEntryOrder("ux") : -1;
            cache.i_uy = schema.exists("uy") ? schema.getEntryOrder("uy") : -1;
            cache.i_uz = schema.exists("uz") ? schema.getEntryOrder("uz") : -1;
            cache.i_err_ux = schema.exists("err_ux") ? schema.getEntryOrder("err_ux") : -1;
            cache.i_err_uy = schema.exists("err_uy") ? schema.getEntryOrder("err_uy") : -1;
            cache.i_err_uz = schema.exists("err_uz") ? schema.getEntryOrder("err_uz") : -1;
            cache.i_Segment1_ID = schema.exists("Segment1_ID") ? schema.getEntryOrder("Segment1_ID") : -1;
            cache.i_Segment2_ID = schema.exists("Segment2_ID") ? schema.getEntryOrder("Segment2_ID") : -1;
        }
        i_id = cache.i_id;
        i_status = cache.i_status;
        i_sector = cache.i_sector;
        i_region = cache.i_region;
        i_x = cache.i_x;
        i_y = cache.i_y;
        i_z = cache.i_z;
        i_err_x = cache.i_err_x;
        i_err_y = cache.i_err_y;
        i_err_z = cache.i_err_z;
        i_ux = cache.i_ux;
        i_uy = cache.i_uy;
        i_uz = cache.i_uz;
        i_err_ux = cache.i_err_ux;
        i_err_uy = cache.i_err_uy;
        i_err_uz = cache.i_err_uz;
        i_Segment1_ID = cache.i_Segment1_ID;
        i_Segment2_ID = cache.i_Segment2_ID;
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
    /// id: id of the cross
    [[nodiscard]] int id(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_id, row));
    }
    /// status: status of the cross
    [[nodiscard]] int status(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_status, row));
    }
    /// sector: sector of the cross
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// region: region of the cross
    [[nodiscard]] int region(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_region, row));
    }
    /// x: DC track cross x-coordinate (in the DC tilted sector coordinate system)
    [[nodiscard]] float x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_x, row));
    }
    /// y: DC track cross y-coordinate (in the DC tilted sector coordinate system)
    [[nodiscard]] float y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_y, row));
    }
    /// z: DC track cross z-coordinate (in the DC tilted sector coordinate system)
    [[nodiscard]] float z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_z, row));
    }
    /// err_x: DC track cross x-coordinate uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_x(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_x, row));
    }
    /// err_y: DC track cross y-coordinate uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_y(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_y, row));
    }
    /// err_z: DC track cross z-coordinate uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_z(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_z, row));
    }
    /// ux: DC track cross x-direction (in the DC tilted sector coordinate system)
    [[nodiscard]] float ux(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_ux, row));
    }
    /// uy: DC track cross y-direction (in the DC tilted sector coordinate system)
    [[nodiscard]] float uy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_uy, row));
    }
    /// uz: DC track cross z-direction (in the DC tilted sector coordinate system)
    [[nodiscard]] float uz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_uz, row));
    }
    /// err_ux: DC track cross x-direction uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_ux(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_ux, row));
    }
    /// err_uy: DC track cross y-direction uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_uy(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_uy, row));
    }
    /// err_uz: DC track cross z-direction uncertainty (in the DC tilted sector coordinate system)
    [[nodiscard]] float err_uz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_err_uz, row));
    }
    /// Segment1_ID: id of first superlayer used in segment
    [[nodiscard]] int Segment1_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Segment1_ID, row));
    }
    /// Segment2_ID: id of second superlayer used in segment
    [[nodiscard]] int Segment2_ID(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_Segment2_ID, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_status = -1;
    int i_sector = -1;
    int i_region = -1;
    int i_x = -1;
    int i_y = -1;
    int i_z = -1;
    int i_err_x = -1;
    int i_err_y = -1;
    int i_err_z = -1;
    int i_ux = -1;
    int i_uy = -1;
    int i_uz = -1;
    int i_err_ux = -1;
    int i_err_uy = -1;
    int i_err_uz = -1;
    int i_Segment1_ID = -1;
    int i_Segment2_ID = -1;

    friend class Row;
};

/// Lightweight row view for TimeBasedTrkg::AICrosses. Stores a pointer + row index only.
class TimeBasedTrkg_AICrosses::Row {
 public:
    Row(const TimeBasedTrkg_AICrosses* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int status() const noexcept { return m_bank->status(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int region() const noexcept { return m_bank->region(m_row); }
    [[nodiscard]] float x() const noexcept { return m_bank->x(m_row); }
    [[nodiscard]] float y() const noexcept { return m_bank->y(m_row); }
    [[nodiscard]] float z() const noexcept { return m_bank->z(m_row); }
    [[nodiscard]] float err_x() const noexcept { return m_bank->err_x(m_row); }
    [[nodiscard]] float err_y() const noexcept { return m_bank->err_y(m_row); }
    [[nodiscard]] float err_z() const noexcept { return m_bank->err_z(m_row); }
    [[nodiscard]] float ux() const noexcept { return m_bank->ux(m_row); }
    [[nodiscard]] float uy() const noexcept { return m_bank->uy(m_row); }
    [[nodiscard]] float uz() const noexcept { return m_bank->uz(m_row); }
    [[nodiscard]] float err_ux() const noexcept { return m_bank->err_ux(m_row); }
    [[nodiscard]] float err_uy() const noexcept { return m_bank->err_uy(m_row); }
    [[nodiscard]] float err_uz() const noexcept { return m_bank->err_uz(m_row); }
    [[nodiscard]] int Segment1_ID() const noexcept { return m_bank->Segment1_ID(m_row); }
    [[nodiscard]] int Segment2_ID() const noexcept { return m_bank->Segment2_ID(m_row); }
 private:
    const TimeBasedTrkg_AICrosses* m_bank;
    int m_row;
};

inline TimeBasedTrkg_AICrosses::Row TimeBasedTrkg_AICrosses::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
