// =============================================================================
// Auto-generated from neuralnetwork.json - do not edit by hand.
// Regenerate via: python3 scripts/gen_bankdefs.py
// Source of truth:
//   https://github.com/JeffersonLab/coatjava/tree/development/etc/bankdefs/hipo4
// =============================================================================

#pragma once

#include <cstdint>
#include <string_view>

#include <bank.h>

namespace bankdefs {


/// HIPO bank wrapper for `ai::tracks` (group 4200, item 3).
/// tracks identified with AI
///
/// Bind the wrapper to a `hipo::bank` once, then access columns
/// via cached item-index getters (no per-call name lookup).
///
/// Example:
/// @code
/// ai_tracks bank;
/// if (auto sc = bank.bind(event.get_bank("ai::tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class ai_tracks {
 public:
    static constexpr std::string_view bank_name = "ai::tracks";
    static constexpr int bank_group = 4200;
    static constexpr int bank_item  = 3;

    class Row;

    ai_tracks() = default;

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
            int i_sector = -1;
            int i_charge = -1;
            int i_prob = -1;
            int i_c1 = -1;
            int i_c2 = -1;
            int i_c3 = -1;
            int i_c4 = -1;
            int i_c5 = -1;
            int i_c6 = -1;
            int i_p = -1;
            int i_theta = -1;
            int i_phi = -1;
            int i_vz = -1;
        };
        static thread_local ItemCache cache;
        if (cache.gi_group != schema.getGroup() || cache.gi_item != schema.getItem()) {
            cache.gi_group = schema.getGroup();
            cache.gi_item  = schema.getItem();
            cache.i_id = schema.exists("id") ? schema.getEntryOrder("id") : -1;
            cache.i_sector = schema.exists("sector") ? schema.getEntryOrder("sector") : -1;
            cache.i_charge = schema.exists("charge") ? schema.getEntryOrder("charge") : -1;
            cache.i_prob = schema.exists("prob") ? schema.getEntryOrder("prob") : -1;
            cache.i_c1 = schema.exists("c1") ? schema.getEntryOrder("c1") : -1;
            cache.i_c2 = schema.exists("c2") ? schema.getEntryOrder("c2") : -1;
            cache.i_c3 = schema.exists("c3") ? schema.getEntryOrder("c3") : -1;
            cache.i_c4 = schema.exists("c4") ? schema.getEntryOrder("c4") : -1;
            cache.i_c5 = schema.exists("c5") ? schema.getEntryOrder("c5") : -1;
            cache.i_c6 = schema.exists("c6") ? schema.getEntryOrder("c6") : -1;
            cache.i_p = schema.exists("p") ? schema.getEntryOrder("p") : -1;
            cache.i_theta = schema.exists("theta") ? schema.getEntryOrder("theta") : -1;
            cache.i_phi = schema.exists("phi") ? schema.getEntryOrder("phi") : -1;
            cache.i_vz = schema.exists("vz") ? schema.getEntryOrder("vz") : -1;
        }
        i_id = cache.i_id;
        i_sector = cache.i_sector;
        i_charge = cache.i_charge;
        i_prob = cache.i_prob;
        i_c1 = cache.i_c1;
        i_c2 = cache.i_c2;
        i_c3 = cache.i_c3;
        i_c4 = cache.i_c4;
        i_c5 = cache.i_c5;
        i_c6 = cache.i_c6;
        i_p = cache.i_p;
        i_theta = cache.i_theta;
        i_phi = cache.i_phi;
        i_vz = cache.i_vz;
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
        return static_cast<int>(m_view->getByte(i_id, row));
    }
    /// sector: sector of the track
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_sector, row));
    }
    /// charge: charge of the track
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view->getByte(i_charge, row));
    }
    /// prob: probability of inference
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_prob, row));
    }
    /// c1: cluster ID in superlayer 1
    [[nodiscard]] int c1(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_c1, row));
    }
    /// c2: cluster ID in superlayer 2
    [[nodiscard]] int c2(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_c2, row));
    }
    /// c3: cluster ID in superlayer 3
    [[nodiscard]] int c3(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_c3, row));
    }
    /// c4: cluster ID in superlayer 4
    [[nodiscard]] int c4(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_c4, row));
    }
    /// c5: cluster ID in superlayer 5
    [[nodiscard]] int c5(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_c5, row));
    }
    /// c6: cluster ID in superlayer 6
    [[nodiscard]] int c6(int row) const noexcept {
        return static_cast<int>(m_view->getShort(i_c6, row));
    }
    /// p: momentum of the track
    [[nodiscard]] float p(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_p, row));
    }
    /// theta: theta angle of the track
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_theta, row));
    }
    /// phi: phi angle of the track
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_phi, row));
    }
    /// vz: vertex Z component for predicted track
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view->getFloat(i_vz, row));
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    const hipo::bank* m_view = nullptr;
    int i_id = -1;
    int i_sector = -1;
    int i_charge = -1;
    int i_prob = -1;
    int i_c1 = -1;
    int i_c2 = -1;
    int i_c3 = -1;
    int i_c4 = -1;
    int i_c5 = -1;
    int i_c6 = -1;
    int i_p = -1;
    int i_theta = -1;
    int i_phi = -1;
    int i_vz = -1;

    friend class Row;
};

/// Lightweight row view for ai::tracks. Stores a pointer + row index only.
class ai_tracks::Row {
 public:
    Row(const ai_tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
    [[nodiscard]] int row_index() const noexcept { return m_row; }
    [[nodiscard]] int id() const noexcept { return m_bank->id(m_row); }
    [[nodiscard]] int sector() const noexcept { return m_bank->sector(m_row); }
    [[nodiscard]] int charge() const noexcept { return m_bank->charge(m_row); }
    [[nodiscard]] float prob() const noexcept { return m_bank->prob(m_row); }
    [[nodiscard]] int c1() const noexcept { return m_bank->c1(m_row); }
    [[nodiscard]] int c2() const noexcept { return m_bank->c2(m_row); }
    [[nodiscard]] int c3() const noexcept { return m_bank->c3(m_row); }
    [[nodiscard]] int c4() const noexcept { return m_bank->c4(m_row); }
    [[nodiscard]] int c5() const noexcept { return m_bank->c5(m_row); }
    [[nodiscard]] int c6() const noexcept { return m_bank->c6(m_row); }
    [[nodiscard]] float p() const noexcept { return m_bank->p(m_row); }
    [[nodiscard]] float theta() const noexcept { return m_bank->theta(m_row); }
    [[nodiscard]] float phi() const noexcept { return m_bank->phi(m_row); }
    [[nodiscard]] float vz() const noexcept { return m_bank->vz(m_row); }
 private:
    const ai_tracks* m_bank;
    int m_row;
};

inline ai_tracks::Row ai_tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace bankdefs
