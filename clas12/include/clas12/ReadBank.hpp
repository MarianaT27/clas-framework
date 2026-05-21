#pragma once

/**
 * @file ReadBank.hpp
 * @brief Physics-view bank readers for CLAS12 detector data.
 *
 * The per-bank physics-view structs (CalorimeterBank, CherenkovBank, ...) and
 * their `read_*_bank(Event&, int pindex)` entry points are generated from the
 * declarative PHYSICS_VIEWS config in `scripts/gen_bankdefs.py`. This header
 * forwards to the generated umbrella and layers on two small MC-matching
 * helpers that don't fit the row -> struct mold (they filter by a quality
 * threshold and return a single int).
 */

#include <clas12/bankdefs/generated_readers.hpp>

#include <clas12/framework/core/Event.hpp>


/// Outcome of a single MC::RecMatch / MC::GenMatch lookup. Returned by the
/// `_diag` overloads so callers can distinguish "bank empty", "no row for
/// this pindex", "row found but failed quality cut", and "row found,
/// matched-to mcindex=-1" instead of collapsing every failure into "-1".
struct McMatchResult {
    int   mc_index = -1;     ///< MC::Particle (or REC::Particle) index, -1 if no match.
    float quality  = 0.0f;   ///< quality of the row that was found, 0 if none.
    bool  row_found = false; ///< true if MC::RecMatch had a row with this pindex.
    int   bank_rows = 0;     ///< total rows in the lookup bank for this event.
};

/**
 * @brief Find the MC particle index matching a reconstructed particle.
 *
 * Walks MC::RecMatch rows linked to `rec_pindex` and returns the first row
 * whose quality is at least `min_quality`. Returns -1 if no match is found.
 *
 * `min_quality` defaults to 0.5 — the historical 0.92 default rejected
 * everything but near-perfect matches and dropped the rate of accepted
 * MC matches on simulations whose layer overlap is in the 0.5-0.9 range
 * to effectively zero. Tighten via the parameter when the analysis
 * needs only the cleanest matches.
 */
inline auto get_mc_particle_index_diag(framework::Event& event, int rec_pindex,
                                       float min_quality = 0.5f) -> McMatchResult {
    McMatchResult out;
    auto& bank = event.bank<bankdefs::MC_RecMatch>();
    out.bank_rows = bank.rows();
    for (auto match : event.rows_for_pindex<bankdefs::MC_RecMatch>(rec_pindex)) {
        out.row_found = true;
        out.mc_index  = match.mcindex();
        out.quality   = match.quality();
        if (out.quality >= min_quality && out.mc_index >= 0) {
            return out;  // pass; mc_index is the matched value
        }
        // Row exists but failed quality or has mc_index < 0; report it
        // through the diag fields and clear mc_index for the caller.
        out.mc_index = -1;
        break;  // pindex is unique.
    }
    return out;
}

inline auto get_mc_particle_index(framework::Event& event, int rec_pindex,
                                  float min_quality = 0.5f) -> int {
    return get_mc_particle_index_diag(event, rec_pindex, min_quality).mc_index;
}

/**
 * @brief Find the reconstructed particle index matching an MC particle.
 *
 * MC::GenMatch is keyed by mcindex (not pindex), so this is a linear scan.
 * The bank is small (one row per matched particle).
 */
inline auto get_rec_particle_index_diag(framework::Event& event, int mc_index,
                                        float min_quality = 0.5f) -> McMatchResult {
    McMatchResult out;
    auto& match = event.bank<bankdefs::MC_GenMatch>();
    if (!match.valid()) return out;
    out.bank_rows = match.rows();
    for (int row = 0; row < out.bank_rows; ++row) {
        if (match.mcindex(row) != mc_index) continue;
        out.row_found = true;
        out.quality   = match.quality(row);
        const int pindex = match.pindex(row);
        if (out.quality >= min_quality && pindex >= 0) {
            out.mc_index = pindex;  // re-use mc_index slot for the rec pindex
            return out;
        }
        break;  // mcindex is unique.
    }
    return out;
}

inline auto get_rec_particle_index(framework::Event& event, int mc_index,
                                   float min_quality = 0.5f) -> int {
    return get_rec_particle_index_diag(event, mc_index, min_quality).mc_index;
}

