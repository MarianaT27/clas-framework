#pragma once

/**
 * @file ReadBank.hpp
 * @brief Physics-view bank readers for CLAS12 detector data.
 *
 * Every read_*_bank function takes a clas12::framework::Event& and goes
 * through the per-event generated-wrapper cache:
 *
 *   event.bank<clas12::bankdefs::REC_Calorimeter>()     // bind + cache
 *   event.rows_for_pindex<clas12::bankdefs::REC_Calorimeter>(pindex)
 *
 * The first call per bank per event binds the generated wrapper and
 * builds a PindexLinker; subsequent calls on the same bank type reuse
 * both. For a selection pipeline that reads the same calorimeter bank
 * N times per event, N-1 of those reads cost nothing beyond a hash
 * lookup.
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
#include <span>

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
    auto& cher = event.bank<clas12::bankdefs::REC_Cherenkov>();
    if (!cher.valid()) return result;

    for (int row : event.rows_for_pindex<clas12::bankdefs::REC_Cherenkov>(pindex)) {
        if (cher.detector(row) != HTCC) continue;

        result.sector = cher.sector(row);
        result.nphe   = cher.nphe(row);
        result.time   = cher.time(row);
        result.path   = cher.path(row);
        result.chi2   = cher.chi2(row);
        result.x      = cher.x(row);
        result.y      = cher.y(row);
        result.z      = cher.z(row);
        result.dtheta = cher.dtheta(row);
        result.dphi   = cher.dphi(row);
        result.status = cher.status(row);
        break;  // Take first HTCC hit.
    }

    return result;
}

// =============================================================================
// Calorimeter Bank Reader
// =============================================================================

namespace detail {

inline auto fill_calo_base(CalorimeterBank::CalorimeterStruct& out,
                           const clas12::bankdefs::REC_Calorimeter& calo, int row) -> void {
    out.sector = calo.sector(row);
    out.layer  = calo.layer(row);
    out.energy = calo.energy(row);
    out.time   = calo.time(row);
    out.path   = calo.path(row);
    out.chi2   = calo.chi2(row);
    out.x      = calo.x(row);
    out.y      = calo.y(row);
    out.z      = calo.z(row);
    out.lu     = calo.lu(row);
    out.lv     = calo.lv(row);
    out.lw     = calo.lw(row);
}

inline auto fill_calo_extended(CalorimeterBank::CalorimeterStruct& out,
                               const clas12::bankdefs::REC_Calorimeter& calo, int row) -> void {
    out.du  = calo.du(row);
    out.dv  = calo.dv(row);
    out.dw  = calo.dw(row);
    out.m2u = calo.m2u(row);
    out.m2v = calo.m2v(row);
    out.m2w = calo.m2w(row);
    out.m3u = calo.m3u(row);
    out.m3v = calo.m3v(row);
    out.m3w = calo.m3w(row);
}

inline auto fill_calo_extras(CalorimeterBank::CalorimeterStruct& out,
                             const clas12::bankdefs::REC_CaloExtras& extras, int row) -> void {
    out.dbstU  = static_cast<int16_t>(extras.dbstU(row));
    out.dbstV  = static_cast<int16_t>(extras.dbstV(row));
    out.dbstW  = static_cast<int16_t>(extras.dbstW(row));
    out.rawEU  = extras.rawEU(row);
    out.rawEV  = extras.rawEV(row);
    out.rawEW  = extras.rawEW(row);
    out.recEU  = extras.recEU(row);
    out.recEV  = extras.recEV(row);
    out.recEW  = extras.recEW(row);
    out.recDTU = extras.recDTU(row);
    out.recDTV = extras.recDTV(row);
    out.recDTW = extras.recDTW(row);
    out.recFTU = extras.recFTU(row);
    out.recFTV = extras.recFTV(row);
    out.recFTW = extras.recFTW(row);
    out.size   = extras.size(row);
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
    auto& calo = event.bank<clas12::bankdefs::REC_Calorimeter>();
    if (!calo.valid()) return result;

    for (int row : event.rows_for_pindex<clas12::bankdefs::REC_Calorimeter>(pindex)) {
        if (calo.detector(row) != ECAL) continue;

        auto* target = detail::calo_target(result, calo.layer(row));
        if (!target) continue;
        detail::fill_calo_base(*target, calo, row);
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
    auto& calo   = event.bank<clas12::bankdefs::REC_Calorimeter>();
    auto& extras = event.bank<clas12::bankdefs::REC_CaloExtras>();
    if (!calo.valid()) return result;
    const bool has_extras = extras.valid();

    for (int row : event.rows_for_pindex<clas12::bankdefs::REC_Calorimeter>(pindex)) {
        if (calo.detector(row) != ECAL) continue;

        auto* target = detail::calo_target(result, calo.layer(row));
        if (!target) continue;
        detail::fill_calo_base(*target, calo, row);
        detail::fill_calo_extended(*target, calo, row);
        if (has_extras && row < extras.rows()) {
            detail::fill_calo_extras(*target, extras, row);
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
    auto& track = event.bank<clas12::bankdefs::REC_Track>();
    if (!track.valid()) return result;

    for (int row : event.rows_for_pindex<clas12::bankdefs::REC_Track>(pindex)) {
        if (track.detector(row) != DC) continue;

        result.index    = track.index(row);
        result.pindex   = track.pindex(row);
        result.detector = track.detector(row);
        result.sector   = track.sector(row);
        result.status   = track.status(row);
        result.q        = track.q(row);
        result.chi2     = track.chi2(row);
        result.ndf      = track.NDF(row);
        result.hbindex  = track.hbindex(row);
        break;  // Take first DC track.
    }

    return result;
}

// =============================================================================
// Trajectory Bank Reader
// =============================================================================

namespace detail {

inline auto fill_traj(TrajBank& out, const clas12::bankdefs::REC_Traj& traj, int row) -> void {
    out.pindex   = traj.pindex(row);
    out.index    = traj.index(row);
    out.detector = traj.detector(row);
    out.layer    = traj.layer(row);
    out.x  = traj.x(row);
    out.y  = traj.y(row);
    out.z  = traj.z(row);
    out.cx = traj.cx(row);
    out.cy = traj.cy(row);
    out.cz = traj.cz(row);
    out.path = traj.path(row);
    out.edge = traj.edge(row);
}

}  // namespace detail

/**
 * @brief Extract DC trajectory data for a particle (region1/region2/region3).
 */
inline auto read_Traj_bank(clas12::framework::Event& event, int pindex) -> DCTrajBank {
    DCTrajBank result;
    auto& traj = event.bank<clas12::bankdefs::REC_Traj>();
    if (!traj.valid()) return result;

    for (int row : event.rows_for_pindex<clas12::bankdefs::REC_Traj>(pindex)) {
        if (traj.detector(row) != DC) continue;

        switch (traj.layer(row)) {
            case DC1: detail::fill_traj(result.region1, traj, row); break;
            case DC3: detail::fill_traj(result.region2, traj, row); break;
            case DC6: detail::fill_traj(result.region3, traj, row); break;
        }
    }

    return result;
}

// =============================================================================
// Scintillator Bank Reader
// =============================================================================

namespace detail {

inline auto fill_scint_base(ScintillatorBank& out,
                            const clas12::bankdefs::REC_Scintillator& scint, int row) -> void {
    out.index     = scint.index(row);
    out.pindex    = scint.pindex(row);
    out.detector  = scint.detector(row);
    out.sector    = scint.sector(row);
    out.layer     = scint.layer(row);
    out.component = scint.component(row);
    out.energy    = scint.energy(row);
    out.time      = scint.time(row);
    out.path      = scint.path(row);
    out.chi2      = scint.chi2(row);
    out.x         = scint.x(row);
    out.y         = scint.y(row);
    out.z         = scint.z(row);
    out.hx        = scint.hx(row);
    out.hy        = scint.hy(row);
    out.hz        = scint.hz(row);
    out.status    = scint.status(row);
}

/// Select the best FTOF row from the linked rows (priority: 1B > 1A > 2).
inline auto select_best_ftof(const clas12::bankdefs::REC_Scintillator& scint,
                             std::span<const int> rows) -> int {
    int best_row = -1;
    int best_layer = -1;
    for (int row : rows) {
        if (scint.detector(row) != FTOF) continue;

        const int layer = scint.layer(row);
        const bool is_better =
            (layer == FTOF1B && best_layer != FTOF1B) ||
            (layer == FTOF1A && best_layer != FTOF1B && best_layer != FTOF1A) ||
            (layer == FTOF2  && best_row == -1);
        if (is_better) {
            best_row = row;
            best_layer = layer;
        }
    }
    return best_row;
}

}  // namespace detail

/**
 * @brief Extract FTOF scintillator data for a particle.
 *
 * Prioritizes FTOF layers in order: FTOF1B > FTOF1A > FTOF2.
 */
inline auto read_Scintillator_bank(clas12::framework::Event& event, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    auto& scint = event.bank<clas12::bankdefs::REC_Scintillator>();
    if (!scint.valid()) return result;

    const int best_row = detail::select_best_ftof(
        scint, event.rows_for_pindex<clas12::bankdefs::REC_Scintillator>(pindex));
    if (best_row < 0) return result;

    detail::fill_scint_base(result, scint, best_row);
    return result;
}

/**
 * @brief Extract FTOF scintillator data with extended fields from REC::ScintExtras.
 */
inline auto read_Scintillator_bank_with_extras(clas12::framework::Event& event, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    auto& scint  = event.bank<clas12::bankdefs::REC_Scintillator>();
    auto& extras = event.bank<clas12::bankdefs::REC_ScintExtras>();
    if (!scint.valid()) return result;
    const bool has_extras = extras.valid();

    const int best_row = detail::select_best_ftof(
        scint, event.rows_for_pindex<clas12::bankdefs::REC_Scintillator>(pindex));
    if (best_row < 0) return result;

    detail::fill_scint_base(result, scint, best_row);
    if (has_extras && best_row < extras.rows()) {
        result.dedx      = extras.dedx(best_row);
        result.size      = static_cast<int16_t>(extras.size(best_row));
        result.layermult = static_cast<char>(extras.layermult(best_row));
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
    auto& rich = event.bank<clas12::bankdefs::RICH_Particle>();
    if (!rich.valid()) return result;

    // RICH::Particle exposes a pindex handle; use the cached linker to
    // skip straight to the matched rows instead of a linear scan.
    for (int row : event.rows_for_pindex<clas12::bankdefs::RICH_Particle>(pindex)) {
        result.id       = rich.id(row);
        result.hindex   = rich.hindex(row);
        result.pindex   = pindex;

        result.emilay   = rich.emilay(row);
        result.emico    = rich.emico(row);
        result.enico    = rich.enico(row);
        result.emqua    = rich.emqua(row);
        result.mchi2    = rich.mchi2(row);
        result.mass     = rich.mass(row);

        result.best_PID = rich.best_PID(row);
        result.RQ       = rich.RQ(row);
        result.ReQ      = rich.ReQ(row);
        result.el_logl  = rich.el_logl(row);
        result.pi_logl  = rich.pi_logl(row);
        result.k_logl   = rich.k_logl(row);
        result.pr_logl  = rich.pr_logl(row);

        result.best_ch   = rich.best_ch(row);
        result.best_c2   = rich.best_c2(row);
        result.best_RL   = rich.best_RL(row);
        result.best_ntot = rich.best_ntot(row);
        result.best_mass = rich.best_mass(row);
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
    auto& match = event.bank<clas12::bankdefs::MC_RecMatch>();
    if (!match.valid()) return -1;

    for (int row : event.rows_for_pindex<clas12::bankdefs::MC_RecMatch>(rec_pindex)) {
        const int mc_index = match.mcindex(row);
        const float quality = match.quality(row);
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
