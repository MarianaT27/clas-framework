#pragma once

/**
 * @file ReadBank.hpp
 * @brief Physics-view bank readers for CLAS12 detector data.
 *
 * Every read_*_bank function takes a clas12::framework::Event& and
 * iterates row proxies from the framework's pindex-linked row view:
 *
 *   for (auto hit : event.rows_for_pindex<REC_Calorimeter>(pindex)) {
 *       if (hit.detector() != ECAL) continue;
 *       ... hit.energy(), hit.layer(), ...
 *   }
 *
 * The first call per bank per event binds the generated wrapper and
 * builds a PindexLinker; subsequent calls reuse both via the cache in
 * Event. The Row proxies yielded by the view carry a pointer + row
 * index only, so iteration allocates nothing.
 *
 * The denormalized physics-view struct types (CalorimeterBank with
 * pcal/inner/outer, DCTrajBank with region1/region2/region3, etc.) are
 * preserved unchanged as return values - consumer algorithms depend on
 * that layout.
 */

#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>

#include <clas12/bankdefs/REC_Calorimeter.hpp>
#include <clas12/bankdefs/REC_CaloExtras.hpp>
#include <clas12/bankdefs/REC_Cherenkov.hpp>
#include <clas12/bankdefs/REC_Scintillator.hpp>
#include <clas12/bankdefs/REC_ScintExtras.hpp>
#include <clas12/bankdefs/REC_Track.hpp>
#include <clas12/bankdefs/REC_Traj.hpp>
#include <clas12/bankdefs/RICH_Particle.hpp>
#include <clas12/bankdefs/MC_GenMatch.hpp>
#include <clas12/bankdefs/MC_RecMatch.hpp>

#include <clas12/framework/core/Event.hpp>

#include <cstdint>
#include <optional>

namespace clas12 {

// =============================================================================
// Cherenkov Bank Reader
// =============================================================================

/**
 * @brief Extract Cherenkov detector data for a particle.
 *
 * Prioritizes the HTCC detector response.
 */
inline auto read_Cherenkov_bank(clas12::framework::Event& event, int pindex) -> CherenkovBank {
    CherenkovBank result;
    for (auto hit : event.rows_for_pindex<clas12::bankdefs::REC_Cherenkov>(pindex)) {
        if (hit.detector() != HTCC) continue;

        result.sector = hit.sector();
        result.nphe   = hit.nphe();
        result.time   = hit.time();
        result.path   = hit.path();
        result.chi2   = hit.chi2();
        result.x      = hit.x();
        result.y      = hit.y();
        result.z      = hit.z();
        result.dtheta = hit.dtheta();
        result.dphi   = hit.dphi();
        result.status = hit.status();
        break;  // Take first HTCC hit.
    }

    return result;
}

// =============================================================================
// Calorimeter Bank Reader
// =============================================================================

namespace detail {

inline auto fill_calo_base(CalorimeterBank::CalorimeterStruct& out,
                           const clas12::bankdefs::REC_Calorimeter::Row& calo) -> void {
    out.sector = calo.sector();
    out.layer  = calo.layer();
    out.energy = calo.energy();
    out.time   = calo.time();
    out.path   = calo.path();
    out.chi2   = calo.chi2();
    out.x      = calo.x();
    out.y      = calo.y();
    out.z      = calo.z();
    out.lu     = calo.lu();
    out.lv     = calo.lv();
    out.lw     = calo.lw();
}

inline auto fill_calo_extended(CalorimeterBank::CalorimeterStruct& out,
                               const clas12::bankdefs::REC_Calorimeter::Row& calo) -> void {
    out.du  = calo.du();
    out.dv  = calo.dv();
    out.dw  = calo.dw();
    out.m2u = calo.m2u();
    out.m2v = calo.m2v();
    out.m2w = calo.m2w();
    out.m3u = calo.m3u();
    out.m3v = calo.m3v();
    out.m3w = calo.m3w();
}

inline auto fill_calo_extras(CalorimeterBank::CalorimeterStruct& out,
                             const clas12::bankdefs::REC_CaloExtras::Row& extras) -> void {
    out.dbstU  = static_cast<int16_t>(extras.dbstU());
    out.dbstV  = static_cast<int16_t>(extras.dbstV());
    out.dbstW  = static_cast<int16_t>(extras.dbstW());
    out.rawEU  = extras.rawEU();
    out.rawEV  = extras.rawEV();
    out.rawEW  = extras.rawEW();
    out.recEU  = extras.recEU();
    out.recEV  = extras.recEV();
    out.recEW  = extras.recEW();
    out.recDTU = extras.recDTU();
    out.recDTV = extras.recDTV();
    out.recDTW = extras.recDTW();
    out.recFTU = extras.recFTU();
    out.recFTV = extras.recFTV();
    out.recFTW = extras.recFTW();
    out.size   = extras.size();
}

inline auto calo_target(CalorimeterBank& result, int layer)
    -> CalorimeterBank::CalorimeterStruct* {
    switch (layer) {
        case PCAL:  return &result.pcal;
        case ECIN:  return &result.inner;
        case ECOUT: return &result.outer;
        default:    return nullptr;
    }
}

}  // namespace detail

/**
 * @brief Extract calorimeter data for a particle (PCAL / ECin / ECout).
 */
inline auto read_Calorimeter_bank(clas12::framework::Event& event, int pindex) -> CalorimeterBank {
    CalorimeterBank result;
    for (auto calo : event.rows_for_pindex<clas12::bankdefs::REC_Calorimeter>(pindex)) {
        if (calo.detector() != ECAL) continue;

        auto* target = detail::calo_target(result, calo.layer());
        if (!target) continue;
        detail::fill_calo_base(*target, calo);
    }

    return result;
}

/**
 * @brief Extract calorimeter data with extended fields from REC::CaloExtras.
 *
 * The REC::CaloExtras bank has a 1:1 row mapping with REC::Calorimeter so
 * the same row index from the linker is used for both.
 */
inline auto read_Calorimeter_bank_with_extras(clas12::framework::Event& event, int pindex) -> CalorimeterBank {
    CalorimeterBank result;
    auto& extras = event.bank<clas12::bankdefs::REC_CaloExtras>();
    const bool has_extras = extras.valid();
    const int extras_rows = extras.rows();

    for (auto calo : event.rows_for_pindex<clas12::bankdefs::REC_Calorimeter>(pindex)) {
        if (calo.detector() != ECAL) continue;

        auto* target = detail::calo_target(result, calo.layer());
        if (!target) continue;
        detail::fill_calo_base(*target, calo);
        detail::fill_calo_extended(*target, calo);
        const int row = calo.row_index();
        if (has_extras && row < extras_rows) {
            detail::fill_calo_extras(*target, extras.row(row));
        }
    }

    return result;
}

// =============================================================================
// Track Bank Reader
// =============================================================================

/**
 * @brief Extract drift chamber track data for a particle (first DC match).
 */
inline auto read_Track_bank(clas12::framework::Event& event, int pindex) -> TrackBank {
    TrackBank result;
    for (auto track : event.rows_for_pindex<clas12::bankdefs::REC_Track>(pindex)) {
        if (track.detector() != DC) continue;

        result.index    = track.index();
        result.pindex   = track.pindex();
        result.detector = track.detector();
        result.sector   = track.sector();
        result.status   = track.status();
        result.q        = track.q();
        result.chi2     = track.chi2();
        result.ndf      = track.NDF();
        result.hbindex  = track.hbindex();
        break;  // Take first DC track.
    }

    return result;
}

// =============================================================================
// Trajectory Bank Reader
// =============================================================================

namespace detail {

inline auto fill_traj(TrajBank& out, const clas12::bankdefs::REC_Traj::Row& traj) -> void {
    out.pindex   = traj.pindex();
    out.index    = traj.index();
    out.detector = traj.detector();
    out.layer    = traj.layer();
    out.x  = traj.x();
    out.y  = traj.y();
    out.z  = traj.z();
    out.cx = traj.cx();
    out.cy = traj.cy();
    out.cz = traj.cz();
    out.path = traj.path();
    out.edge = traj.edge();
}

}  // namespace detail

/**
 * @brief Extract DC trajectory data for a particle (region1/region2/region3).
 */
inline auto read_Traj_bank(clas12::framework::Event& event, int pindex) -> DCTrajBank {
    DCTrajBank result;
    for (auto traj : event.rows_for_pindex<clas12::bankdefs::REC_Traj>(pindex)) {
        if (traj.detector() != DC) continue;

        switch (traj.layer()) {
            case DC1: detail::fill_traj(result.region1, traj); break;
            case DC3: detail::fill_traj(result.region2, traj); break;
            case DC6: detail::fill_traj(result.region3, traj); break;
        }
    }

    return result;
}

// =============================================================================
// Scintillator Bank Reader
// =============================================================================

namespace detail {

inline auto fill_scint_base(ScintillatorBank& out,
                            const clas12::bankdefs::REC_Scintillator::Row& scint) -> void {
    out.index     = scint.index();
    out.pindex    = scint.pindex();
    out.detector  = scint.detector();
    out.sector    = scint.sector();
    out.layer     = scint.layer();
    out.component = scint.component();
    out.energy    = scint.energy();
    out.time      = scint.time();
    out.path      = scint.path();
    out.chi2      = scint.chi2();
    out.x         = scint.x();
    out.y         = scint.y();
    out.z         = scint.z();
    out.hx        = scint.hx();
    out.hy        = scint.hy();
    out.hz        = scint.hz();
    out.status    = scint.status();
}

/// Select the best FTOF Row from the linked rows (priority: 1B > 1A > 2).
inline auto select_best_ftof(
    clas12::framework::PindexRowView<clas12::bankdefs::REC_Scintillator> rows)
    -> std::optional<clas12::bankdefs::REC_Scintillator::Row> {
    std::optional<clas12::bankdefs::REC_Scintillator::Row> best;
    int best_layer = -1;
    for (auto scint : rows) {
        if (scint.detector() != FTOF) continue;

        const int layer = scint.layer();
        const bool is_better =
            (layer == FTOF1B && best_layer != FTOF1B) ||
            (layer == FTOF1A && best_layer != FTOF1B && best_layer != FTOF1A) ||
            (layer == FTOF2  && !best.has_value());
        if (is_better) {
            best = scint;
            best_layer = layer;
        }
    }
    return best;
}

}  // namespace detail

/**
 * @brief Extract FTOF scintillator data for a particle.
 *
 * Prioritizes FTOF layers in order: FTOF1B > FTOF1A > FTOF2.
 */
inline auto read_Scintillator_bank(clas12::framework::Event& event, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    auto best = detail::select_best_ftof(
        event.rows_for_pindex<clas12::bankdefs::REC_Scintillator>(pindex));
    if (!best.has_value()) return result;

    detail::fill_scint_base(result, *best);
    return result;
}

/**
 * @brief Extract FTOF scintillator data with extended fields from REC::ScintExtras.
 */
inline auto read_Scintillator_bank_with_extras(clas12::framework::Event& event, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    auto best = detail::select_best_ftof(
        event.rows_for_pindex<clas12::bankdefs::REC_Scintillator>(pindex));
    if (!best.has_value()) return result;

    detail::fill_scint_base(result, *best);

    auto& extras = event.bank<clas12::bankdefs::REC_ScintExtras>();
    const int best_row = best->row_index();
    if (extras.valid() && best_row < extras.rows()) {
        auto ext = extras.row(best_row);
        result.dedx      = ext.dedx();
        result.size      = static_cast<int16_t>(ext.size());
        result.layermult = static_cast<char>(ext.layermult());
    }

    return result;
}

// =============================================================================
// RICH Particle Bank Reader
// =============================================================================

/**
 * @brief Extract RICH particle data for a given REC::Particle index.
 */
inline auto read_RICHParticle_bank(clas12::framework::Event& event, int pindex) -> RICHParticleBank {
    RICHParticleBank result;
    // RICH::Particle exposes a pindex handle; use the cached linker to
    // skip straight to the matched rows instead of a linear scan.
    for (auto rich : event.rows_for_pindex<clas12::bankdefs::RICH_Particle>(pindex)) {
        result.id       = rich.id();
        result.hindex   = rich.hindex();
        result.pindex   = pindex;

        result.emilay   = rich.emilay();
        result.emico    = rich.emico();
        result.enico    = rich.enico();
        result.emqua    = rich.emqua();
        result.mchi2    = rich.mchi2();
        result.mass     = rich.mass();

        result.best_PID = rich.best_PID();
        result.RQ       = rich.RQ();
        result.ReQ      = rich.ReQ();
        result.el_logl  = rich.el_logl();
        result.pi_logl  = rich.pi_logl();
        result.k_logl   = rich.k_logl();
        result.pr_logl  = rich.pr_logl();

        result.best_ch   = rich.best_ch();
        result.best_c2   = rich.best_c2();
        result.best_RL   = rich.best_RL();
        result.best_ntot = rich.best_ntot();
        result.best_mass = rich.best_mass();
        break;  // Take first match.
    }

    return result;
}

// =============================================================================
// MC Matching Utilities
// =============================================================================

/**
 * @brief Find MC particle index matching a reconstructed particle.
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
 * @brief Find reconstructed particle index matching an MC particle.
 *
 * Note: MC::GenMatch is keyed by mcindex rather than pindex, so we cannot
 * use the Event::rows_for_pindex<T> cache here. A linear scan is used
 * instead; the bank is small (one row per matched particle).
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
