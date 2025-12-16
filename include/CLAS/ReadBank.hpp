#pragma once

/**
 * @file ReadBank.hpp
 * @brief Utilities for extracting detector data from HIPO banks
 *
 * This header provides functions to read and parse CLAS12 detector banks
 * (Calorimeter, Cherenkov, Track, Trajectory, Scintillator) into structured
 * C++ objects for convenient analysis.
 *
 * All functions support both hipo::bank and hipo::BankView as input.
 */

#include <hipo4/bank.h>
#include <hipo4/hipoeventiterator.h>
#include <CLAS/Banks.hpp>
#include <CLAS/Detectors.hpp>

#include <map>
#include <string>
#include <vector>

namespace CLAS {

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
inline auto build_index_map(const hipo::bank& bank, const char* index_column)
    -> std::map<int, std::vector<int>> {
    std::map<int, std::vector<int>> result;

    const int n_rows = bank.getRows();
    if (n_rows <= 0) return result;

    // Process in batches for better cache performance
    constexpr int BATCH_SIZE = 16;
    for (int i = 0; i < n_rows; i += BATCH_SIZE) {
        const int end = std::min(i + BATCH_SIZE, n_rows);
        for (int row = i; row < end; ++row) {
            const int idx = static_cast<int>(bank.get<float>(index_column, row));
            auto& vec = result[idx];
            if (vec.empty()) vec.reserve(8);
            vec.push_back(row);
        }
    }

    return result;
}

/// Overload for BankView
inline auto build_index_map(const hipo::BankView& view, const char* index_column)
    -> std::map<int, std::vector<int>> {
    return build_index_map(view.data(), index_column);
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
inline auto read_Cherenkov_bank(const hipo::bank& bank, int pindex) -> CherenkovBank {
    CherenkovBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    for (int row : index_map.at(pindex)) {
        if (bank.get<int>("detector", row) != HTCC) continue;

        result.sector = bank.get<int>("sector", row);
        result.nphe = bank.get<float>("nphe", row);
        result.time = bank.get<float>("time", row);
        result.path = bank.get<float>("path", row);
        result.chi2 = bank.get<float>("chi2", row);
        result.x = bank.get<float>("x", row);
        result.y = bank.get<float>("y", row);
        result.z = bank.get<float>("z", row);
        result.dtheta = bank.get<float>("dtheta", row);
        result.dphi = bank.get<float>("dphi", row);
        result.status = bank.get<int>("status", row);
        break;  // Take first HTCC hit
    }

    return result;
}

inline auto read_Cherenkov_bank(hipo::BankView& view, int pindex) -> CherenkovBank {
    return read_Cherenkov_bank(view.data(), pindex);
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
inline auto read_Calorimeter_bank(const hipo::bank& bank, int pindex) -> CalorimeterBank {
    CalorimeterBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    // Helper to populate a calorimeter sub-structure
    auto fill_calo = [&bank](CalorimeterBank::CalorimeterStruct& calo, int row) {
        calo.sector = bank.get<int>("sector", row);
        calo.layer = bank.get<int>("layer", row);
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
        if (bank.get<int>("detector", row) != ECAL) continue;

        switch (bank.get<int>("layer", row)) {
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

inline auto read_Calorimeter_bank(hipo::BankView& view, int pindex) -> CalorimeterBank {
    return read_Calorimeter_bank(view.data(), pindex);
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
inline auto read_Track_bank(const hipo::bank& bank, int pindex) -> TrackBank {
    TrackBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    for (int row : index_map.at(pindex)) {
        if (bank.get<int>("detector", row) != DC) continue;

        result.index = bank.get<int>("index", row);
        result.pindex = bank.get<int>("pindex", row);
        result.detector = bank.get<int>("detector", row);
        result.sector = bank.get<int>("sector", row);
        result.status = bank.get<int>("status", row);
        result.q = bank.get<int>("q", row);
        result.chi2 = bank.get<float>("chi2", row);
        result.ndf = bank.get<int>("NDF", row);
        result.hbindex = bank.get<int>("hbindex", row);
        break;  // Take first DC track
    }

    return result;
}

inline auto read_Track_bank(hipo::BankView& view, int pindex) -> TrackBank {
    return read_Track_bank(view.data(), pindex);
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
inline auto read_Traj_bank(const hipo::bank& bank, int pindex) -> DCTrajBank {
    DCTrajBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    // Helper to populate a trajectory region
    auto fill_traj = [&bank](TrajBank& traj, int row) {
        traj.pindex = bank.get<int>("pindex", row);
        traj.index = bank.get<int>("index", row);
        traj.detector = bank.get<int>("detector", row);
        traj.layer = bank.get<int>("layer", row);
        traj.x = bank.get<double>("x", row);
        traj.y = bank.get<double>("y", row);
        traj.z = bank.get<double>("z", row);
        traj.cx = bank.get<double>("cx", row);
        traj.cy = bank.get<double>("cy", row);
        traj.cz = bank.get<double>("cz", row);
        traj.path = bank.get<double>("path", row);
        traj.edge = bank.get<double>("edge", row);
    };

    for (int row : index_map.at(pindex)) {
        if (bank.get<int>("detector", row) != DC) continue;

        switch (bank.get<int>("layer", row)) {
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

inline auto read_Traj_bank(hipo::BankView& view, int pindex) -> DCTrajBank {
    return read_Traj_bank(view.data(), pindex);
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
inline auto read_Scintillator_bank(const hipo::bank& bank, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    auto index_map = build_index_map(bank, "pindex");

    if (!index_map.contains(pindex)) return result;

    // Find best FTOF layer (priority: 1B > 1A > 2)
    int best_row = -1;
    int best_layer = -1;

    for (int row : index_map.at(pindex)) {
        if (bank.get<int>("detector", row) != FTOF) continue;

        int layer = bank.get<int>("layer", row);

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
    result.index = bank.get<int>("index", best_row);
    result.pindex = bank.get<int>("pindex", best_row);
    result.detector = bank.get<int>("detector", best_row);
    result.sector = bank.get<int>("sector", best_row);
    result.layer = bank.get<int>("layer", best_row);
    result.component = bank.get<int>("component", best_row);
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
    result.status = bank.get<int>("status", best_row);

    return result;
}

inline auto read_Scintillator_bank(hipo::BankView& view, int pindex) -> ScintillatorBank {
    return read_Scintillator_bank(view.data(), pindex);
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
inline auto get_mc_particle_index(const hipo::bank& bank, int rec_pindex,
                                  float min_quality = 0.92f) -> int {
    const int n_rows = bank.getRows();

    for (int row = 0; row < n_rows; ++row) {
        if (bank.get<int>("pindex", row) != rec_pindex) continue;

        int mc_index = bank.get<int>("mcindex", row);
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
inline auto get_rec_particle_index(const hipo::bank& bank, int mc_index,
                                   float min_quality = 0.92f) -> int {
    const int n_rows = bank.getRows();

    for (int row = 0; row < n_rows; ++row) {
        if (bank.get<int>("mcindex", row) != mc_index) continue;

        int pindex = bank.get<int>("pindex", row);
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
inline auto load_bank_by_index(const hipo::bank& bank, const std::string& idx)
    -> std::map<int, std::vector<int>> {
    return build_index_map(bank, idx.c_str());
}

}  // namespace CLAS
