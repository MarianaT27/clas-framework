#pragma once

/**
 * @file ReadBank.hpp
 * @brief Utilities for extracting detector data from HIPO banks
 *
 * This header provides functions to read and parse CLAS12 detector banks
 * (Calorimeter, Cherenkov, Track, Trajectory, Scintillator) into structured
 * C++ objects for convenient analysis.
 */

#include <bank_view.h>
#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>

#include <map>
#include <string>
#include <vector>

namespace clas12 {

// =============================================================================
// Index Mapping Utilities
// =============================================================================

/**
 * @brief Build a map from particle index to bank row indices
 *
 * Creates a lookup table mapping each unique value in the specified index column
 * to the list of rows containing that value. This enables efficient O(1) lookup
 * of all bank rows associated with a given particle.
 *
 * @param bank The HIPO bank to index
 * @param index_column Name of the index column (typically "pindex")
 * @return Map from index value to vector of row numbers
 */
inline auto build_index_map(const hipo::bank_view& bank, const char* index_column)
    -> std::map<int, std::vector<int>> {
    std::map<int, std::vector<int>> result;

    const int n_rows = bank.rows();
    if (n_rows <= 0) return result;

    // Resolve column once, then use type-safe get_numeric to read the correct
    // number of bytes regardless of the column's storage type (int16, int32, etc.).
    auto col_idx = bank.get_schema().column_index(index_column);
    if (!col_idx) return result;

    constexpr int BATCH_SIZE = 16;
    for (int i = 0; i < n_rows; i += BATCH_SIZE) {
        const int end = std::min(i + BATCH_SIZE, n_rows);
        for (int row = i; row < end; ++row) {
            const int idx = static_cast<int>(bank.get_numeric(*col_idx, row));
            auto& vec = result[idx];
            if (vec.empty()) vec.reserve(8);
            vec.push_back(row);
        }
    }

    return result;
}



// =============================================================================
// Cherenkov Bank Reader
// =============================================================================

/**
 * @brief Extract Cherenkov detector data for a particle
 *
 * @param bank REC::Cherenkov bank (hipo::bank or BankView)
 * @param pindex Particle index to look up
 * @return CherenkovBank structure with HTCC data (empty if not found)
 */
inline auto read_Cherenkov_bank(const hipo::bank_view& bank, int pindex) -> CherenkovBank {
    CherenkovBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    for (int row : index_map.at(pindex)) {
        if (bank.get<int8_t>("detector", row) != HTCC) continue;

        result.sector = bank.get<int8_t>("sector", row);
        result.nphe = bank.get<float>("nphe", row);
        result.time = bank.get<float>("time", row);
        result.path = bank.get<float>("path", row);
        result.chi2 = bank.get<float>("chi2", row);
        result.x = bank.get<float>("x", row);
        result.y = bank.get<float>("y", row);
        result.z = bank.get<float>("z", row);
        result.dtheta = bank.get<float>("dtheta", row);
        result.dphi = bank.get<float>("dphi", row);
        result.status = bank.get<int16_t>("status", row);
        break;  // Take first HTCC hit
    }

    return result;
}



// =============================================================================
// Calorimeter Bank Reader
// =============================================================================

/**
 * @brief Extract calorimeter data for a particle
 *
 * Reads PCAL, ECin (inner), and ECout (outer) calorimeter information
 * for the specified particle index.
 *
 * @param bank REC::Calorimeter bank
 * @param pindex Particle index to look up
 * @return CalorimeterBank with pcal, inner, and outer sub-structures
 */
inline auto read_Calorimeter_bank(const hipo::bank_view& bank, int pindex) -> CalorimeterBank {
    CalorimeterBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    // Helper to populate a calorimeter sub-structure
    auto fill_calo = [&bank](CalorimeterBank::CalorimeterStruct& calo, int row) {
        calo.sector = bank.get<int8_t>("sector", row);
        calo.layer = bank.get<int8_t>("layer", row);
        calo.energy = bank.get<float>("energy", row);
        calo.time = bank.get<float>("time", row);
        calo.path = bank.get<float>("path", row);
        calo.chi2 = bank.get<float>("chi2", row);
        calo.x = bank.get<float>("x", row);
        calo.y = bank.get<float>("y", row);
        calo.z = bank.get<float>("z", row);
        calo.lu = bank.get<float>("lu", row);
        calo.lv = bank.get<float>("lv", row);
        calo.lw = bank.get<float>("lw", row);
    };

    for (int row : index_map.at(pindex)) {
        if (bank.get<int8_t>("detector", row) != ECAL) continue;

        switch (bank.get<int8_t>("layer", row)) {
            case PCAL:
                fill_calo(result.pcal, row);
                break;
            case ECIN:
                fill_calo(result.inner, row);
                break;
            case ECOUT:
                fill_calo(result.outer, row);
                break;
        }
    }

    return result;
}

/**
 * @brief Extract calorimeter data with extended fields from REC::CaloExtras
 *
 * Overloaded version that reads both the main REC::Calorimeter bank and
 * the companion REC::CaloExtras bank (1:1 row mapping).
 *
 * @param bank REC::Calorimeter bank
 * @param extra_bank REC::CaloExtras bank
 * @param pindex Particle index to look up
 * @return CalorimeterBank with all fields including extended data
 */
inline auto read_Calorimeter_bank(const hipo::bank_view& bank, const hipo::bank_view& extra_bank, int pindex) -> CalorimeterBank {
    CalorimeterBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    auto fill_calo = [&bank, &extra_bank](CalorimeterBank::CalorimeterStruct& calo, int row) {
        // Basic fields from REC::Calorimeter
        calo.sector = bank.get<int8_t>("sector", row);
        calo.layer  = bank.get<int8_t>("layer", row);
        calo.energy = bank.get<float>("energy", row);
        calo.time   = bank.get<float>("time", row);
        calo.path   = bank.get<float>("path", row);
        calo.chi2   = bank.get<float>("chi2", row);
        calo.x      = bank.get<float>("x", row);
        calo.y      = bank.get<float>("y", row);
        calo.z      = bank.get<float>("z", row);
        calo.lu     = bank.get<float>("lu", row);
        calo.lv     = bank.get<float>("lv", row);
        calo.lw     = bank.get<float>("lw", row);

        // Extended fields from REC::Calorimeter
        calo.du  = bank.get<float>("du", row);
        calo.dv  = bank.get<float>("dv", row);
        calo.dw  = bank.get<float>("dw", row);
        calo.m2u = bank.get<float>("m2u", row);
        calo.m2v = bank.get<float>("m2v", row);
        calo.m2w = bank.get<float>("m2w", row);
        calo.m3u = bank.get<float>("m3u", row);
        calo.m3v = bank.get<float>("m3v", row);
        calo.m3w = bank.get<float>("m3w", row);

        // Extra fields from REC::CaloExtras (1:1 row mapping)
        if (row < extra_bank.rows()) {
            calo.dbstU  = extra_bank.get<int16_t>("dbstU", row);
            calo.dbstV  = extra_bank.get<int16_t>("dbstV", row);
            calo.dbstW  = extra_bank.get<int16_t>("dbstW", row);
            calo.rawEU  = extra_bank.get<float>("rawEU", row);
            calo.rawEV  = extra_bank.get<float>("rawEV", row);
            calo.rawEW  = extra_bank.get<float>("rawEW", row);
            calo.recEU  = extra_bank.get<float>("recEU", row);
            calo.recEV  = extra_bank.get<float>("recEV", row);
            calo.recEW  = extra_bank.get<float>("recEW", row);
            calo.recDTU = extra_bank.get<float>("recDTU", row);
            calo.recDTV = extra_bank.get<float>("recDTV", row);
            calo.recDTW = extra_bank.get<float>("recDTW", row);
            calo.recFTU = extra_bank.get<float>("recFTU", row);
            calo.recFTV = extra_bank.get<float>("recFTV", row);
            calo.recFTW = extra_bank.get<float>("recFTW", row);
            calo.size   = extra_bank.get<float>("size", row);
        }
    };

    for (int row : index_map.at(pindex)) {
        if (bank.get<int8_t>("detector", row) != ECAL) continue;

        switch (bank.get<int8_t>("layer", row)) {
            case PCAL:
                fill_calo(result.pcal, row);
                break;
            case ECIN:
                fill_calo(result.inner, row);
                break;
            case ECOUT:
                fill_calo(result.outer, row);
                break;
        }
    }

    return result;
}



// =============================================================================
// Track Bank Reader
// =============================================================================

/**
 * @brief Extract drift chamber track data for a particle
 *
 * @param bank REC::Track bank
 * @param pindex Particle index to look up
 * @return TrackBank structure with DC track info
 */
inline auto read_Track_bank(const hipo::bank_view& bank, int pindex) -> TrackBank {
    TrackBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    for (int row : index_map.at(pindex)) {
        if (bank.get<int8_t>("detector", row) != DC) continue;

        result.index = bank.get<int16_t>("index", row);
        result.pindex = bank.get<int16_t>("pindex", row);
        result.detector = bank.get<int8_t>("detector", row);
        result.sector = bank.get<int8_t>("sector", row);
        result.status = bank.get<int16_t>("status", row);
        result.q = bank.get<int8_t>("q", row);
        result.chi2 = bank.get<float>("chi2", row);
        result.ndf = bank.get<int16_t>("NDF", row);
        result.hbindex = bank.get<int16_t>("hbindex", row);
        break;  // Take first DC track
    }

    return result;
}



// =============================================================================
// Trajectory Bank Reader
// =============================================================================

/**
 * @brief Extract DC trajectory data for a particle
 *
 * Reads trajectory information for all three DC regions (layers 6, 18, 36).
 *
 * @param bank REC::Traj bank
 * @param pindex Particle index to look up
 * @return DCTrajBank with region1, region2, region3 sub-structures
 */
inline auto read_Traj_bank(const hipo::bank_view& bank, int pindex) -> DCTrajBank {
    DCTrajBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    // Helper to populate a trajectory region
    auto fill_traj = [&bank](TrajBank& traj, int row) {
        traj.pindex = bank.get<int16_t>("pindex", row);
        traj.index = bank.get<int16_t>("index", row);
        traj.detector = bank.get<int8_t>("detector", row);
        traj.layer = bank.get<int8_t>("layer", row);
        traj.x = bank.get<float>("x", row);
        traj.y = bank.get<float>("y", row);
        traj.z = bank.get<float>("z", row);
        traj.cx = bank.get<float>("cx", row);
        traj.cy = bank.get<float>("cy", row);
        traj.cz = bank.get<float>("cz", row);
        traj.path = bank.get<float>("path", row);
        traj.edge = bank.get<float>("edge", row);
    };

    for (int row : index_map.at(pindex)) {
        if (bank.get<int8_t>("detector", row) != DC) continue;

        switch (bank.get<int8_t>("layer", row)) {
            case DC1:
                fill_traj(result.region1, row);
                break;
            case DC3:
                fill_traj(result.region2, row);
                break;
            case DC6:
                fill_traj(result.region3, row);
                break;
        }
    }

    return result;
}



// =============================================================================
// Scintillator Bank Reader
// =============================================================================

/**
 * @brief Extract FTOF scintillator data for a particle
 *
 * Prioritizes FTOF layers in order: FTOF1B > FTOF1A > FTOF2
 *
 * @param bank REC::Scintillator bank
 * @param pindex Particle index to look up
 * @return ScintillatorBank structure with FTOF data
 */
inline auto read_Scintillator_bank(const hipo::bank_view& bank, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    // Find best FTOF layer (priority: 1B > 1A > 2)
    int best_row = -1;
    int best_layer = -1;

    for (int row : index_map.at(pindex)) {
        if (bank.get<int8_t>("detector", row) != FTOF) continue;

        int layer = bank.get<int8_t>("layer", row);

        // Priority: FTOF1B (2) > FTOF1A (1) > FTOF2 (3)
        bool is_better = (layer == FTOF1B && best_layer != FTOF1B) ||
                         (layer == FTOF1A && best_layer != FTOF1B && best_layer != FTOF1A) ||
                         (layer == FTOF2 && best_row == -1);

        if (is_better) {
            best_row = row;
            best_layer = layer;
        }
    }

    if (best_row < 0) return result;

    // Fill result from best row
    result.index = bank.get<int16_t>("index", best_row);
    result.pindex = bank.get<int16_t>("pindex", best_row);
    result.detector = bank.get<int8_t>("detector", best_row);
    result.sector = bank.get<int8_t>("sector", best_row);
    result.layer = bank.get<int8_t>("layer", best_row);
    result.component = bank.get<int16_t>("component", best_row);
    result.energy = bank.get<float>("energy", best_row);
    result.time = bank.get<float>("time", best_row);
    result.path = bank.get<float>("path", best_row);
    result.chi2 = bank.get<float>("chi2", best_row);
    result.x = bank.get<float>("x", best_row);
    result.y = bank.get<float>("y", best_row);
    result.z = bank.get<float>("z", best_row);
    result.hx = bank.get<float>("hx", best_row);
    result.hy = bank.get<float>("hy", best_row);
    result.hz = bank.get<float>("hz", best_row);
    result.status = bank.get<int16_t>("status", best_row);

    return result;
}

/**
 * @brief Extract FTOF scintillator data with extended fields from REC::ScintExtras
 *
 * Overloaded version that reads both the main REC::Scintillator bank and
 * the companion REC::ScintExtras bank (1:1 row mapping).
 *
 * @param bank REC::Scintillator bank
 * @param extra_bank REC::ScintExtras bank
 * @param pindex Particle index to look up
 * @return ScintillatorBank structure with all fields including extended data
 */
inline auto read_Scintillator_bank(const hipo::bank_view& bank, const hipo::bank_view& extra_bank, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    // Find best FTOF layer (priority: 1B > 1A > 2)
    int best_row = -1;
    int best_layer = -1;

    for (int row : index_map.at(pindex)) {
        if (bank.get<int8_t>("detector", row) != FTOF) continue;

        int layer = bank.get<int8_t>("layer", row);

        bool is_better = (layer == FTOF1B && best_layer != FTOF1B) ||
                         (layer == FTOF1A && best_layer != FTOF1B && best_layer != FTOF1A) ||
                         (layer == FTOF2 && best_row == -1);

        if (is_better) {
            best_row = row;
            best_layer = layer;
        }
    }

    if (best_row < 0) return result;

    // Fill result from best row - basic fields
    result.index     = bank.get<int16_t>("index", best_row);
    result.pindex    = bank.get<int16_t>("pindex", best_row);
    result.detector  = bank.get<int8_t>("detector", best_row);
    result.sector    = bank.get<int8_t>("sector", best_row);
    result.layer     = bank.get<int8_t>("layer", best_row);
    result.component = bank.get<int16_t>("component", best_row);
    result.energy    = bank.get<float>("energy", best_row);
    result.time      = bank.get<float>("time", best_row);
    result.path      = bank.get<float>("path", best_row);
    result.chi2      = bank.get<float>("chi2", best_row);
    result.x         = bank.get<float>("x", best_row);
    result.y         = bank.get<float>("y", best_row);
    result.z         = bank.get<float>("z", best_row);
    result.hx        = bank.get<float>("hx", best_row);
    result.hy        = bank.get<float>("hy", best_row);
    result.hz        = bank.get<float>("hz", best_row);
    result.status    = bank.get<int16_t>("status", best_row);

    // Extra fields from REC::ScintExtras (1:1 row mapping)
    if (best_row < extra_bank.rows()) {
        result.dedx      = extra_bank.get<float>("dedx", best_row);
        result.size      = extra_bank.get<int16_t>("size", best_row);
        result.layermult = extra_bank.get<int8_t>("layermult", best_row);
    }

    return result;
}



// =============================================================================
// RICH Particle Bank Reader
// =============================================================================

/**
 * @brief Extract RICH particle data for a given REC::Particle index
 *
 * Reads the RICH::Particle bank and returns the entry matching the given pindex.
 *
 * @param bank RICH::Particle bank
 * @param pindex Particle index (row in REC::Particle)
 * @return RICHParticleBank structure (empty/NaN if not found)
 */
inline auto read_RICHParticle_bank(const hipo::bank_view& bank, int pindex) -> RICHParticleBank {
    RICHParticleBank result;
    const int n_rows = bank.rows();

    for (int row = 0; row < n_rows; ++row) {
        if (static_cast<int>(bank.get<int8_t>("pindex", row)) != pindex) continue;

        result.id       = static_cast<int>(bank.get<int8_t>("id", row));
        result.hindex   = static_cast<int>(bank.get<int16_t>("hindex", row));
        result.pindex   = pindex;

        result.emilay   = static_cast<int>(bank.get<int8_t>("emilay", row));
        result.emico    = static_cast<int>(bank.get<int8_t>("emico", row));
        result.enico    = static_cast<int>(bank.get<int8_t>("enico", row));
        result.emqua    = static_cast<int>(bank.get<int16_t>("emqua", row));
        result.mchi2    = bank.get<float>("mchi2", row);
        result.mass     = bank.get<float>("mass", row);

        result.best_PID = static_cast<int>(bank.get<int16_t>("best_PID", row));
        result.RQ       = bank.get<float>("RQ", row);
        result.ReQ      = bank.get<float>("ReQ", row);
        result.el_logl  = bank.get<float>("el_logl", row);
        result.pi_logl  = bank.get<float>("pi_logl", row);
        result.k_logl   = bank.get<float>("k_logl", row);
        result.pr_logl  = bank.get<float>("pr_logl", row);

        result.best_ch   = bank.get<float>("best_ch", row);
        result.best_c2   = bank.get<float>("best_c2", row);
        result.best_RL   = bank.get<float>("best_RL", row);
        result.best_ntot = bank.get<float>("best_ntot", row);
        result.best_mass = bank.get<float>("best_mass", row);
        break;  // Take first match
    }

    return result;
}

// =============================================================================
// MC Matching Utilities
// =============================================================================

/**
 * @brief Find MC particle index matching a reconstructed particle
 *
 * @param bank MC::RecMatch bank
 * @param rec_pindex Reconstructed particle index
 * @param min_quality Minimum matching quality threshold (default: 0.92)
 * @return MC particle index, or -1 if no match found
 */
inline auto get_mc_particle_index(const hipo::bank_view& bank, int rec_pindex,
                                  float min_quality = 0.92f) -> int {
    const int n_rows = bank.rows();

    for (int row = 0; row < n_rows; ++row) {
        if (bank.get<int16_t>("pindex", row) != rec_pindex) continue;

        int mc_index = bank.get<int16_t>("mcindex", row);
        float quality = bank.get<float>("quality", row);

        if (quality >= min_quality && mc_index >= 0) {
            return mc_index;
        }
        break;  // pindex is unique, no need to continue
    }

    return -1;
}

/**
 * @brief Find reconstructed particle index matching an MC particle
 *
 * @param bank MC::GenMatch bank
 * @param mc_index MC particle index
 * @param min_quality Minimum matching quality threshold (default: 0.92)
 * @return Reconstructed particle index, or -1 if no match found
 */
inline auto get_rec_particle_index(const hipo::bank_view& bank, int mc_index,
                                   float min_quality = 0.92f) -> int {
    const int n_rows = bank.rows();

    for (int row = 0; row < n_rows; ++row) {
        if (bank.get<int16_t>("mcindex", row) != mc_index) continue;

        int pindex = bank.get<int16_t>("pindex", row);
        float quality = bank.get<float>("quality", row);

        if (quality >= min_quality && pindex >= 0) {
            return pindex;
        }
        break;  // mcindex is unique, no need to continue
    }

    return -1;
}

// =============================================================================
// Legacy Aliases
// =============================================================================

/// @deprecated Use build_index_map instead
inline auto load_bank_by_index(const hipo::bank_view& bank, const std::string& idx)
    -> std::map<int, std::vector<int>> {
    return build_index_map(bank, idx.c_str());
}

}  // namespace clas12
