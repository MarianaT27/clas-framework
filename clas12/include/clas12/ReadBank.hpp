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

namespace clas12 {

/**
 * @brief Find the MC particle index matching a reconstructed particle.
 *
 * Walks MC::RecMatch rows linked to `rec_pindex` and returns the first row
 * whose quality is at least `min_quality`. Returns -1 if no match is found.
 */
inline auto get_mc_particle_index(clas12::framework::Event& event, int rec_pindex,
                                  float min_quality = 0.92f) -> int {
    for (auto match : event.rows_for_pindex<clas12::bankdefs::MC_RecMatch>(rec_pindex)) {
        const int mc_index = match.mcindex();
        const float quality = match.quality();
        if (quality >= min_quality && mc_index >= 0) return mc_index;
        break;  // pindex is unique.
    }
    return -1;
}

/**
 * @brief Find the reconstructed particle index matching an MC particle.
 *
 * MC::GenMatch is keyed by mcindex (not pindex), so this is a linear scan.
 * The bank is small (one row per matched particle).
 */
inline auto get_rec_particle_index(clas12::framework::Event& event, int mc_index,
                                   float min_quality = 0.92f) -> int {
    auto& match = event.bank<clas12::bankdefs::MC_GenMatch>();
    if (!match.valid()) return -1;

    const int n_rows = match.rows();
    for (int row = 0; row < n_rows; ++row) {
        if (match.mcindex(row) != mc_index) continue;

        const int pindex = match.pindex(row);
        const float quality = match.quality(row);
        if (quality >= min_quality && pindex >= 0) return pindex;
        break;  // mcindex is unique.
    }
    return -1;
}

}  // namespace clas12
