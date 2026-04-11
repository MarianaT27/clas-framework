// =============================================================================
// Auto-generated from neuralnetwork.json - do not edit by hand.
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


/// HIPO bank wrapper for `aidn::tracks` (group 4200, item 4).
/// tracks identified with denoised AI
///
/// Bind the wrapper to a `hipo::bank_view` once, then access columns
/// via zero-cost handle-based getters.
///
/// Example:
/// @code
/// aidn_tracks bank;
/// if (auto sc = bank.bind(event.get_bank("aidn::tracks")); !sc) return FAILURE();
/// for (int i = 0; i < bank.rows(); ++i) {
///     auto row = bank.row(i);
///     auto id = row.id();
/// }
/// @endcode
class aidn_tracks {
 public:
    static constexpr std::string_view bank_name = "aidn::tracks";
    static constexpr int bank_group = 4200;
    static constexpr int bank_item  = 4;

    class Row;

    aidn_tracks() = default;

    /// Bind to a live bank_view. Resolves every column handle exactly once.
    /// Returns false if any required column is missing in the schema.
    bool bind(const hipo::bank_view& view) noexcept {
        m_view = view;
        if (!view.valid()) return false;
        const auto& schema = view.get_schema();
        if (auto h = schema.get_handle("id"); h) h_id = *h;
        else h_id = {};
        if (auto h = schema.get_handle("sector"); h) h_sector = *h;
        else h_sector = {};
        if (auto h = schema.get_handle("charge"); h) h_charge = *h;
        else h_charge = {};
        if (auto h = schema.get_handle("prob"); h) h_prob = *h;
        else h_prob = {};
        if (auto h = schema.get_handle("c1"); h) h_c1 = *h;
        else h_c1 = {};
        if (auto h = schema.get_handle("c2"); h) h_c2 = *h;
        else h_c2 = {};
        if (auto h = schema.get_handle("c3"); h) h_c3 = *h;
        else h_c3 = {};
        if (auto h = schema.get_handle("c4"); h) h_c4 = *h;
        else h_c4 = {};
        if (auto h = schema.get_handle("c5"); h) h_c5 = *h;
        else h_c5 = {};
        if (auto h = schema.get_handle("c6"); h) h_c6 = *h;
        else h_c6 = {};
        if (auto h = schema.get_handle("p"); h) h_p = *h;
        else h_p = {};
        if (auto h = schema.get_handle("theta"); h) h_theta = *h;
        else h_theta = {};
        if (auto h = schema.get_handle("phi"); h) h_phi = *h;
        else h_phi = {};
        if (auto h = schema.get_handle("vz"); h) h_vz = *h;
        else h_vz = {};
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
        return static_cast<int>(m_view.get<int8_t>(h_id, row));
    }
    /// sector: sector of the track
    [[nodiscard]] int sector(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_sector, row));
    }
    /// charge: charge of the track
    [[nodiscard]] int charge(int row) const noexcept {
        return static_cast<int>(m_view.get<int8_t>(h_charge, row));
    }
    /// prob: probability of inference
    [[nodiscard]] float prob(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_prob, row));
    }
    /// c1: cluster ID in superlayer 1
    [[nodiscard]] int c1(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_c1, row));
    }
    /// c2: cluster ID in superlayer 2
    [[nodiscard]] int c2(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_c2, row));
    }
    /// c3: cluster ID in superlayer 3
    [[nodiscard]] int c3(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_c3, row));
    }
    /// c4: cluster ID in superlayer 4
    [[nodiscard]] int c4(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_c4, row));
    }
    /// c5: cluster ID in superlayer 5
    [[nodiscard]] int c5(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_c5, row));
    }
    /// c6: cluster ID in superlayer 6
    [[nodiscard]] int c6(int row) const noexcept {
        return static_cast<int>(m_view.get<int16_t>(h_c6, row));
    }
    /// p: momentum of the track
    [[nodiscard]] float p(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_p, row));
    }
    /// theta: theta angle of the track
    [[nodiscard]] float theta(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_theta, row));
    }
    /// phi: phi angle of the track
    [[nodiscard]] float phi(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_phi, row));
    }
    /// vz: vertex Z component for predicted track
    [[nodiscard]] float vz(int row) const noexcept {
        return static_cast<float>(m_view.get<float>(h_vz, row));
    }

    // -------------------------------------------------------------------
    // Whole-column span accessors (SIMD-friendly, zero-copy)
    // -------------------------------------------------------------------
    /// id column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> id_col() const noexcept {
        return m_view.column<int8_t>(h_id);
    }
    /// sector column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> sector_col() const noexcept {
        return m_view.column<int8_t>(h_sector);
    }
    /// charge column as a contiguous span.
    [[nodiscard]] std::span<const int8_t> charge_col() const noexcept {
        return m_view.column<int8_t>(h_charge);
    }
    /// prob column as a contiguous span.
    [[nodiscard]] std::span<const float> prob_col() const noexcept {
        return m_view.column<float>(h_prob);
    }
    /// c1 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> c1_col() const noexcept {
        return m_view.column<int16_t>(h_c1);
    }
    /// c2 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> c2_col() const noexcept {
        return m_view.column<int16_t>(h_c2);
    }
    /// c3 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> c3_col() const noexcept {
        return m_view.column<int16_t>(h_c3);
    }
    /// c4 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> c4_col() const noexcept {
        return m_view.column<int16_t>(h_c4);
    }
    /// c5 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> c5_col() const noexcept {
        return m_view.column<int16_t>(h_c5);
    }
    /// c6 column as a contiguous span.
    [[nodiscard]] std::span<const int16_t> c6_col() const noexcept {
        return m_view.column<int16_t>(h_c6);
    }
    /// p column as a contiguous span.
    [[nodiscard]] std::span<const float> p_col() const noexcept {
        return m_view.column<float>(h_p);
    }
    /// theta column as a contiguous span.
    [[nodiscard]] std::span<const float> theta_col() const noexcept {
        return m_view.column<float>(h_theta);
    }
    /// phi column as a contiguous span.
    [[nodiscard]] std::span<const float> phi_col() const noexcept {
        return m_view.column<float>(h_phi);
    }
    /// vz column as a contiguous span.
    [[nodiscard]] std::span<const float> vz_col() const noexcept {
        return m_view.column<float>(h_vz);
    }

    /// Construct a lightweight Row view for the given index.
    [[nodiscard]] Row row(int index) const noexcept;

 private:
    hipo::bank_view m_view{};
    hipo::column_handle h_id{};
    hipo::column_handle h_sector{};
    hipo::column_handle h_charge{};
    hipo::column_handle h_prob{};
    hipo::column_handle h_c1{};
    hipo::column_handle h_c2{};
    hipo::column_handle h_c3{};
    hipo::column_handle h_c4{};
    hipo::column_handle h_c5{};
    hipo::column_handle h_c6{};
    hipo::column_handle h_p{};
    hipo::column_handle h_theta{};
    hipo::column_handle h_phi{};
    hipo::column_handle h_vz{};

    friend class Row;
};

/// Lightweight row view for aidn::tracks. Stores a pointer + row index only.
class aidn_tracks::Row {
 public:
    Row(const aidn_tracks* bank, int row) noexcept : m_bank(bank), m_row(row) {}
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
    const aidn_tracks* m_bank;
    int m_row;
};

inline aidn_tracks::Row aidn_tracks::row(int index) const noexcept {
    return Row{this, index};
}

}  // namespace clas12::bankdefs
