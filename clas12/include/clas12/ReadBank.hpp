#pragma once

/**
 * @file ReadBank.hpp
 * @brief Utilities for extracting detector data from HIPO banks
 *
 * This header provides functions to read and parse CLAS12 detector banks
 * (Calorimeter, Cherenkov, Track, Trajectory, Scintillator) into structured
 * C++ objects for convenient analysis.
 *
 * Hot-path note: every per-row accessor below goes through pre-resolved
 * hipo::column_handle values, never through name-based lookups. String
 * resolution happens exactly once per function call, not per row, not per
 * field. This matters because these readers are called O(particles) times
 * per event in selection and analysis algorithms; the old implementation
 * was doing tens of string -> index binary searches per call.
 */

#include <bank_view.h>
#include <clas12/Banks.hpp>
#include <clas12/Detectors.hpp>
#include <schema.h>

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace clas12 {

namespace detail {

/// Resolve a column_handle or return an invalid handle if the column is
/// missing from the schema. Keeps the call sites below readable without
/// requiring C++23 `.value_or()` on `result<column_handle>`.
inline auto try_handle(const hipo::schema& s, std::string_view col) -> hipo::column_handle {
    auto h = s.get_handle(col);
    return h ? *h : hipo::column_handle{};
}

}  // namespace detail

// =============================================================================
// Index Mapping Utilities
// =============================================================================

/**
 * @brief Build a map from link-column value to bank row indices.
 *
 * Reads the entire link column as a typed contiguous span via
 * hipo::bank_view::visit_column, then bucketizes rows by value into the
 * returned std::map. The read pass is a single typed loop, not a chain of
 * per-row `get_numeric` dispatches through the polymorphic type switch.
 *
 * The return type is kept as std::map<int, std::vector<int>> for source
 * compatibility with the existing read_*_bank callers below. For code
 * written today, prefer clas12::framework::PindexLinker which uses a flat
 * prefix-sum layout with zero per-lookup allocation.
 *
 * @param bank The HIPO bank to index.
 * @param index_column Name of the link column (typically "pindex").
 * @return Map from column value to vector of row numbers.
 */
inline auto build_index_map(const hipo::bank_view& bank, const char* index_column)
    -> std::map<int, std::vector<int>> {
    std::map<int, std::vector<int>> result;
    const int n_rows = bank.rows();
    if (n_rows <= 0) return result;

    auto h = detail::try_handle(bank.get_schema(), index_column);
    if (!h.valid()) return result;

    // visit_column dispatches on the column's storage type once, then we
    // walk a typed span with no further dispatch. Replaces the old per-row
    // get_numeric() calls that went through the runtime type switch for
    // every single row.
    bank.visit_column(h, [&](auto col) {
        for (int row = 0; row < n_rows; ++row) {
            const int key = static_cast<int>(col[row]);
            auto& vec = result[key];
            if (vec.empty()) vec.reserve(8);
            vec.push_back(row);
        }
    });

    return result;
}

// =============================================================================
// Cherenkov Bank Reader
// =============================================================================

/**
 * @brief Extract Cherenkov detector data for a particle.
 *
 * @param bank REC::Cherenkov bank
 * @param pindex Particle index to look up
 * @return CherenkovBank structure with HTCC data (empty if not found)
 */
inline auto read_Cherenkov_bank(const hipo::bank_view& bank, int pindex) -> CherenkovBank {
    CherenkovBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const auto& s = bank.get_schema();
    const auto h_detector = detail::try_handle(s, "detector");
    const auto h_sector   = detail::try_handle(s, "sector");
    const auto h_nphe     = detail::try_handle(s, "nphe");
    const auto h_time     = detail::try_handle(s, "time");
    const auto h_path     = detail::try_handle(s, "path");
    const auto h_chi2     = detail::try_handle(s, "chi2");
    const auto h_x        = detail::try_handle(s, "x");
    const auto h_y        = detail::try_handle(s, "y");
    const auto h_z        = detail::try_handle(s, "z");
    const auto h_dtheta   = detail::try_handle(s, "dtheta");
    const auto h_dphi     = detail::try_handle(s, "dphi");
    const auto h_status   = detail::try_handle(s, "status");

    auto index_map = build_index_map(bank, "pindex");
    auto it = index_map.find(pindex);
    if (it == index_map.end()) return result;

    for (int row : it->second) {
        if (bank.get<int8_t>(h_detector, row) != HTCC) continue;

        result.sector = bank.get<int8_t>(h_sector, row);
        result.nphe   = bank.get<float>(h_nphe, row);
        result.time   = bank.get<float>(h_time, row);
        result.path   = bank.get<float>(h_path, row);
        result.chi2   = bank.get<float>(h_chi2, row);
        result.x      = bank.get<float>(h_x, row);
        result.y      = bank.get<float>(h_y, row);
        result.z      = bank.get<float>(h_z, row);
        result.dtheta = bank.get<float>(h_dtheta, row);
        result.dphi   = bank.get<float>(h_dphi, row);
        result.status = bank.get<int16_t>(h_status, row);
        break;  // Take first HTCC hit
    }

    return result;
}

// =============================================================================
// Calorimeter Bank Reader
// =============================================================================

namespace detail {

/// Resolved column handles for REC::Calorimeter's base fields. Used by both
/// overloads of read_Calorimeter_bank below so the handle-resolution code
/// is not duplicated.
struct CaloHandles {
    hipo::column_handle detector, layer, sector;
    hipo::column_handle energy, time, path, chi2;
    hipo::column_handle x, y, z;
    hipo::column_handle lu, lv, lw;
    hipo::column_handle du, dv, dw;
    hipo::column_handle m2u, m2v, m2w;
    hipo::column_handle m3u, m3v, m3w;

    explicit CaloHandles(const hipo::schema& s)
        : detector(try_handle(s, "detector")), layer(try_handle(s, "layer")),
          sector  (try_handle(s, "sector")),   energy(try_handle(s, "energy")),
          time    (try_handle(s, "time")),     path  (try_handle(s, "path")),
          chi2    (try_handle(s, "chi2")),     x     (try_handle(s, "x")),
          y       (try_handle(s, "y")),        z     (try_handle(s, "z")),
          lu      (try_handle(s, "lu")),       lv    (try_handle(s, "lv")),
          lw      (try_handle(s, "lw")),       du    (try_handle(s, "du")),
          dv      (try_handle(s, "dv")),       dw    (try_handle(s, "dw")),
          m2u     (try_handle(s, "m2u")),      m2v   (try_handle(s, "m2v")),
          m2w     (try_handle(s, "m2w")),      m3u   (try_handle(s, "m3u")),
          m3v     (try_handle(s, "m3v")),      m3w   (try_handle(s, "m3w")) {}
};

/// Resolved column handles for REC::CaloExtras.
struct CaloExtraHandles {
    hipo::column_handle dbstU, dbstV, dbstW;
    hipo::column_handle rawEU, rawEV, rawEW;
    hipo::column_handle recEU, recEV, recEW;
    hipo::column_handle recDTU, recDTV, recDTW;
    hipo::column_handle recFTU, recFTV, recFTW;
    hipo::column_handle size;

    explicit CaloExtraHandles(const hipo::schema& s)
        : dbstU(try_handle(s, "dbstU")),  dbstV(try_handle(s, "dbstV")),
          dbstW(try_handle(s, "dbstW")),  rawEU(try_handle(s, "rawEU")),
          rawEV(try_handle(s, "rawEV")),  rawEW(try_handle(s, "rawEW")),
          recEU(try_handle(s, "recEU")),  recEV(try_handle(s, "recEV")),
          recEW(try_handle(s, "recEW")),  recDTU(try_handle(s, "recDTU")),
          recDTV(try_handle(s, "recDTV")), recDTW(try_handle(s, "recDTW")),
          recFTU(try_handle(s, "recFTU")), recFTV(try_handle(s, "recFTV")),
          recFTW(try_handle(s, "recFTW")), size  (try_handle(s, "size")) {}
};

inline auto fill_calo_base(CalorimeterBank::CalorimeterStruct& out,
                           const hipo::bank_view& bank,
                           const CaloHandles& H, int row) -> void {
    out.sector = bank.get<int8_t>(H.sector, row);
    out.layer  = bank.get<int8_t>(H.layer, row);
    out.energy = bank.get<float>(H.energy, row);
    out.time   = bank.get<float>(H.time, row);
    out.path   = bank.get<float>(H.path, row);
    out.chi2   = bank.get<float>(H.chi2, row);
    out.x      = bank.get<float>(H.x, row);
    out.y      = bank.get<float>(H.y, row);
    out.z      = bank.get<float>(H.z, row);
    out.lu     = bank.get<float>(H.lu, row);
    out.lv     = bank.get<float>(H.lv, row);
    out.lw     = bank.get<float>(H.lw, row);
}

inline auto fill_calo_extended(CalorimeterBank::CalorimeterStruct& out,
                               const hipo::bank_view& bank,
                               const CaloHandles& H, int row) -> void {
    out.du  = bank.get<float>(H.du, row);
    out.dv  = bank.get<float>(H.dv, row);
    out.dw  = bank.get<float>(H.dw, row);
    out.m2u = bank.get<float>(H.m2u, row);
    out.m2v = bank.get<float>(H.m2v, row);
    out.m2w = bank.get<float>(H.m2w, row);
    out.m3u = bank.get<float>(H.m3u, row);
    out.m3v = bank.get<float>(H.m3v, row);
    out.m3w = bank.get<float>(H.m3w, row);
}

inline auto fill_calo_extras(CalorimeterBank::CalorimeterStruct& out,
                             const hipo::bank_view& extra_bank,
                             const CaloExtraHandles& H, int row) -> void {
    out.dbstU  = extra_bank.get<int16_t>(H.dbstU, row);
    out.dbstV  = extra_bank.get<int16_t>(H.dbstV, row);
    out.dbstW  = extra_bank.get<int16_t>(H.dbstW, row);
    out.rawEU  = extra_bank.get<float>(H.rawEU, row);
    out.rawEV  = extra_bank.get<float>(H.rawEV, row);
    out.rawEW  = extra_bank.get<float>(H.rawEW, row);
    out.recEU  = extra_bank.get<float>(H.recEU, row);
    out.recEV  = extra_bank.get<float>(H.recEV, row);
    out.recEW  = extra_bank.get<float>(H.recEW, row);
    out.recDTU = extra_bank.get<float>(H.recDTU, row);
    out.recDTV = extra_bank.get<float>(H.recDTV, row);
    out.recDTW = extra_bank.get<float>(H.recDTW, row);
    out.recFTU = extra_bank.get<float>(H.recFTU, row);
    out.recFTV = extra_bank.get<float>(H.recFTV, row);
    out.recFTW = extra_bank.get<float>(H.recFTW, row);
    out.size   = extra_bank.get<float>(H.size, row);
}

}  // namespace detail

/**
 * @brief Extract calorimeter data for a particle.
 *
 * Reads PCAL, ECin (inner), and ECout (outer) calorimeter information
 * for the specified particle index.
 */
inline auto read_Calorimeter_bank(const hipo::bank_view& bank, int pindex) -> CalorimeterBank {
    CalorimeterBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const detail::CaloHandles H{bank.get_schema()};

    auto index_map = build_index_map(bank, "pindex");
    auto it = index_map.find(pindex);
    if (it == index_map.end()) return result;

    for (int row : it->second) {
        if (bank.get<int8_t>(H.detector, row) != ECAL) continue;

        switch (bank.get<int8_t>(H.layer, row)) {
            case PCAL:  detail::fill_calo_base(result.pcal,  bank, H, row); break;
            case ECIN:  detail::fill_calo_base(result.inner, bank, H, row); break;
            case ECOUT: detail::fill_calo_base(result.outer, bank, H, row); break;
        }
    }

    return result;
}

/**
 * @brief Extract calorimeter data with extended fields from REC::CaloExtras.
 */
inline auto read_Calorimeter_bank(const hipo::bank_view& bank,
                                  const hipo::bank_view& extra_bank,
                                  int pindex) -> CalorimeterBank {
    CalorimeterBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const detail::CaloHandles      H{bank.get_schema()};
    const detail::CaloExtraHandles E{extra_bank.valid() ? extra_bank.get_schema() : hipo::schema{}};
    const bool has_extras = extra_bank.valid();

    auto index_map = build_index_map(bank, "pindex");
    auto it = index_map.find(pindex);
    if (it == index_map.end()) return result;

    for (int row : it->second) {
        if (bank.get<int8_t>(H.detector, row) != ECAL) continue;

        CalorimeterBank::CalorimeterStruct* target = nullptr;
        switch (bank.get<int8_t>(H.layer, row)) {
            case PCAL:  target = &result.pcal;  break;
            case ECIN:  target = &result.inner; break;
            case ECOUT: target = &result.outer; break;
            default: continue;
        }
        detail::fill_calo_base(*target, bank, H, row);
        detail::fill_calo_extended(*target, bank, H, row);
        if (has_extras && row < extra_bank.rows()) {
            detail::fill_calo_extras(*target, extra_bank, E, row);
        }
    }

    return result;
}

// =============================================================================
// Track Bank Reader
// =============================================================================

/**
 * @brief Extract drift chamber track data for a particle.
 */
inline auto read_Track_bank(const hipo::bank_view& bank, int pindex) -> TrackBank {
    TrackBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const auto& s = bank.get_schema();
    const auto h_index    = detail::try_handle(s, "index");
    const auto h_pindex   = detail::try_handle(s, "pindex");
    const auto h_detector = detail::try_handle(s, "detector");
    const auto h_sector   = detail::try_handle(s, "sector");
    const auto h_status   = detail::try_handle(s, "status");
    const auto h_q        = detail::try_handle(s, "q");
    const auto h_chi2     = detail::try_handle(s, "chi2");
    const auto h_ndf      = detail::try_handle(s, "NDF");
    const auto h_hbindex  = detail::try_handle(s, "hbindex");

    auto index_map = build_index_map(bank, "pindex");
    auto it = index_map.find(pindex);
    if (it == index_map.end()) return result;

    for (int row : it->second) {
        if (bank.get<int8_t>(h_detector, row) != DC) continue;

        result.index    = bank.get<int16_t>(h_index, row);
        result.pindex   = bank.get<int16_t>(h_pindex, row);
        result.detector = bank.get<int8_t>(h_detector, row);
        result.sector   = bank.get<int8_t>(h_sector, row);
        result.status   = bank.get<int16_t>(h_status, row);
        result.q        = bank.get<int8_t>(h_q, row);
        result.chi2     = bank.get<float>(h_chi2, row);
        result.ndf      = bank.get<int16_t>(h_ndf, row);
        result.hbindex  = bank.get<int16_t>(h_hbindex, row);
        break;  // Take first DC track
    }

    return result;
}

// =============================================================================
// Trajectory Bank Reader
// =============================================================================

/**
 * @brief Extract DC trajectory data for a particle.
 */
inline auto read_Traj_bank(const hipo::bank_view& bank, int pindex) -> DCTrajBank {
    DCTrajBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const auto& s = bank.get_schema();
    const auto h_pindex   = detail::try_handle(s, "pindex");
    const auto h_index    = detail::try_handle(s, "index");
    const auto h_detector = detail::try_handle(s, "detector");
    const auto h_layer    = detail::try_handle(s, "layer");
    const auto h_x        = detail::try_handle(s, "x");
    const auto h_y        = detail::try_handle(s, "y");
    const auto h_z        = detail::try_handle(s, "z");
    const auto h_cx       = detail::try_handle(s, "cx");
    const auto h_cy       = detail::try_handle(s, "cy");
    const auto h_cz       = detail::try_handle(s, "cz");
    const auto h_path     = detail::try_handle(s, "path");
    const auto h_edge     = detail::try_handle(s, "edge");

    auto fill_traj = [&](TrajBank& traj, int row) {
        traj.pindex   = bank.get<int16_t>(h_pindex, row);
        traj.index    = bank.get<int16_t>(h_index, row);
        traj.detector = bank.get<int8_t>(h_detector, row);
        traj.layer    = bank.get<int8_t>(h_layer, row);
        traj.x  = bank.get<float>(h_x, row);
        traj.y  = bank.get<float>(h_y, row);
        traj.z  = bank.get<float>(h_z, row);
        traj.cx = bank.get<float>(h_cx, row);
        traj.cy = bank.get<float>(h_cy, row);
        traj.cz = bank.get<float>(h_cz, row);
        traj.path = bank.get<float>(h_path, row);
        traj.edge = bank.get<float>(h_edge, row);
    };

    auto index_map = build_index_map(bank, "pindex");
    auto it = index_map.find(pindex);
    if (it == index_map.end()) return result;

    for (int row : it->second) {
        if (bank.get<int8_t>(h_detector, row) != DC) continue;

        switch (bank.get<int8_t>(h_layer, row)) {
            case DC1: fill_traj(result.region1, row); break;
            case DC3: fill_traj(result.region2, row); break;
            case DC6: fill_traj(result.region3, row); break;
        }
    }

    return result;
}

// =============================================================================
// Scintillator Bank Reader
// =============================================================================

namespace detail {

struct ScintHandles {
    hipo::column_handle index, pindex, detector, sector, layer, component;
    hipo::column_handle energy, time, path, chi2;
    hipo::column_handle x, y, z, hx, hy, hz;
    hipo::column_handle status;

    explicit ScintHandles(const hipo::schema& s)
        : index    (try_handle(s, "index")),
          pindex   (try_handle(s, "pindex")),
          detector (try_handle(s, "detector")),
          sector   (try_handle(s, "sector")),
          layer    (try_handle(s, "layer")),
          component(try_handle(s, "component")),
          energy   (try_handle(s, "energy")),
          time     (try_handle(s, "time")),
          path     (try_handle(s, "path")),
          chi2     (try_handle(s, "chi2")),
          x        (try_handle(s, "x")),
          y        (try_handle(s, "y")),
          z        (try_handle(s, "z")),
          hx       (try_handle(s, "hx")),
          hy       (try_handle(s, "hy")),
          hz       (try_handle(s, "hz")),
          status   (try_handle(s, "status")) {}
};

struct ScintExtraHandles {
    hipo::column_handle dedx, size, layermult;

    explicit ScintExtraHandles(const hipo::schema& s)
        : dedx     (try_handle(s, "dedx")),
          size     (try_handle(s, "size")),
          layermult(try_handle(s, "layermult")) {}
};

inline auto fill_scint_base(ScintillatorBank& out, const hipo::bank_view& bank,
                            const ScintHandles& H, int row) -> void {
    out.index     = bank.get<int16_t>(H.index, row);
    out.pindex    = bank.get<int16_t>(H.pindex, row);
    out.detector  = bank.get<int8_t>(H.detector, row);
    out.sector    = bank.get<int8_t>(H.sector, row);
    out.layer     = bank.get<int8_t>(H.layer, row);
    out.component = bank.get<int16_t>(H.component, row);
    out.energy    = bank.get<float>(H.energy, row);
    out.time      = bank.get<float>(H.time, row);
    out.path      = bank.get<float>(H.path, row);
    out.chi2      = bank.get<float>(H.chi2, row);
    out.x         = bank.get<float>(H.x, row);
    out.y         = bank.get<float>(H.y, row);
    out.z         = bank.get<float>(H.z, row);
    out.hx        = bank.get<float>(H.hx, row);
    out.hy        = bank.get<float>(H.hy, row);
    out.hz        = bank.get<float>(H.hz, row);
    out.status    = bank.get<int16_t>(H.status, row);
}

/// Select the best FTOF row from an index-map bucket. Returns -1 if no FTOF
/// row is present. Priority: FTOF1B > FTOF1A > FTOF2.
inline auto select_best_ftof(const hipo::bank_view& bank, const ScintHandles& H,
                             const std::vector<int>& rows) -> int {
    int best_row = -1;
    int best_layer = -1;
    for (int row : rows) {
        if (bank.get<int8_t>(H.detector, row) != FTOF) continue;

        const int layer = bank.get<int8_t>(H.layer, row);
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
inline auto read_Scintillator_bank(const hipo::bank_view& bank, int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const detail::ScintHandles H{bank.get_schema()};

    auto index_map = build_index_map(bank, "pindex");
    auto it = index_map.find(pindex);
    if (it == index_map.end()) return result;

    const int best_row = detail::select_best_ftof(bank, H, it->second);
    if (best_row < 0) return result;

    detail::fill_scint_base(result, bank, H, best_row);
    return result;
}

/**
 * @brief Extract FTOF scintillator data with extended fields from REC::ScintExtras.
 */
inline auto read_Scintillator_bank(const hipo::bank_view& bank,
                                   const hipo::bank_view& extra_bank,
                                   int pindex) -> ScintillatorBank {
    ScintillatorBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const detail::ScintHandles      H{bank.get_schema()};
    const detail::ScintExtraHandles E{extra_bank.valid() ? extra_bank.get_schema() : hipo::schema{}};
    const bool has_extras = extra_bank.valid();

    auto index_map = build_index_map(bank, "pindex");
    auto it = index_map.find(pindex);
    if (it == index_map.end()) return result;

    const int best_row = detail::select_best_ftof(bank, H, it->second);
    if (best_row < 0) return result;

    detail::fill_scint_base(result, bank, H, best_row);
    if (has_extras && best_row < extra_bank.rows()) {
        result.dedx      = extra_bank.get<float>(E.dedx, best_row);
        result.size      = extra_bank.get<int16_t>(E.size, best_row);
        result.layermult = extra_bank.get<int8_t>(E.layermult, best_row);
    }

    return result;
}

// =============================================================================
// RICH Particle Bank Reader
// =============================================================================

/**
 * @brief Extract RICH particle data for a given REC::Particle index.
 */
inline auto read_RICHParticle_bank(const hipo::bank_view& bank, int pindex) -> RICHParticleBank {
    RICHParticleBank result;
    if (!bank.valid() || bank.rows() == 0) return result;

    const auto& s = bank.get_schema();
    const auto h_pindex   = detail::try_handle(s, "pindex");
    const auto h_id       = detail::try_handle(s, "id");
    const auto h_hindex   = detail::try_handle(s, "hindex");
    const auto h_emilay   = detail::try_handle(s, "emilay");
    const auto h_emico    = detail::try_handle(s, "emico");
    const auto h_enico    = detail::try_handle(s, "enico");
    const auto h_emqua    = detail::try_handle(s, "emqua");
    const auto h_mchi2    = detail::try_handle(s, "mchi2");
    const auto h_mass     = detail::try_handle(s, "mass");
    const auto h_best_PID = detail::try_handle(s, "best_PID");
    const auto h_RQ       = detail::try_handle(s, "RQ");
    const auto h_ReQ      = detail::try_handle(s, "ReQ");
    const auto h_el_logl  = detail::try_handle(s, "el_logl");
    const auto h_pi_logl  = detail::try_handle(s, "pi_logl");
    const auto h_k_logl   = detail::try_handle(s, "k_logl");
    const auto h_pr_logl  = detail::try_handle(s, "pr_logl");
    const auto h_best_ch   = detail::try_handle(s, "best_ch");
    const auto h_best_c2   = detail::try_handle(s, "best_c2");
    const auto h_best_RL   = detail::try_handle(s, "best_RL");
    const auto h_best_ntot = detail::try_handle(s, "best_ntot");
    const auto h_best_mass = detail::try_handle(s, "best_mass");

    const int n_rows = bank.rows();
    for (int row = 0; row < n_rows; ++row) {
        if (static_cast<int>(bank.get<int8_t>(h_pindex, row)) != pindex) continue;

        result.id     = static_cast<int>(bank.get<int8_t>(h_id, row));
        result.hindex = static_cast<int>(bank.get<int16_t>(h_hindex, row));
        result.pindex = pindex;

        result.emilay = static_cast<int>(bank.get<int8_t>(h_emilay, row));
        result.emico  = static_cast<int>(bank.get<int8_t>(h_emico, row));
        result.enico  = static_cast<int>(bank.get<int8_t>(h_enico, row));
        result.emqua  = static_cast<int>(bank.get<int16_t>(h_emqua, row));
        result.mchi2  = bank.get<float>(h_mchi2, row);
        result.mass   = bank.get<float>(h_mass, row);

        result.best_PID = static_cast<int>(bank.get<int16_t>(h_best_PID, row));
        result.RQ       = bank.get<float>(h_RQ, row);
        result.ReQ      = bank.get<float>(h_ReQ, row);
        result.el_logl  = bank.get<float>(h_el_logl, row);
        result.pi_logl  = bank.get<float>(h_pi_logl, row);
        result.k_logl   = bank.get<float>(h_k_logl, row);
        result.pr_logl  = bank.get<float>(h_pr_logl, row);

        result.best_ch   = bank.get<float>(h_best_ch, row);
        result.best_c2   = bank.get<float>(h_best_c2, row);
        result.best_RL   = bank.get<float>(h_best_RL, row);
        result.best_ntot = bank.get<float>(h_best_ntot, row);
        result.best_mass = bank.get<float>(h_best_mass, row);
        break;
    }

    return result;
}

// =============================================================================
// MC Matching Utilities
// =============================================================================

/**
 * @brief Find MC particle index matching a reconstructed particle.
 */
inline auto get_mc_particle_index(const hipo::bank_view& bank, int rec_pindex,
                                  float min_quality = 0.92f) -> int {
    if (!bank.valid() || bank.rows() == 0) return -1;

    const auto& s = bank.get_schema();
    const auto h_pindex  = detail::try_handle(s, "pindex");
    const auto h_mcindex = detail::try_handle(s, "mcindex");
    const auto h_quality = detail::try_handle(s, "quality");

    const int n_rows = bank.rows();
    for (int row = 0; row < n_rows; ++row) {
        if (bank.get<int16_t>(h_pindex, row) != rec_pindex) continue;

        const int mc_index = bank.get<int16_t>(h_mcindex, row);
        const float quality = bank.get<float>(h_quality, row);

        if (quality >= min_quality && mc_index >= 0) return mc_index;
        break;  // pindex is unique
    }

    return -1;
}

/**
 * @brief Find reconstructed particle index matching an MC particle.
 */
inline auto get_rec_particle_index(const hipo::bank_view& bank, int mc_index,
                                   float min_quality = 0.92f) -> int {
    if (!bank.valid() || bank.rows() == 0) return -1;

    const auto& s = bank.get_schema();
    const auto h_mcindex = detail::try_handle(s, "mcindex");
    const auto h_pindex  = detail::try_handle(s, "pindex");
    const auto h_quality = detail::try_handle(s, "quality");

    const int n_rows = bank.rows();
    for (int row = 0; row < n_rows; ++row) {
        if (bank.get<int16_t>(h_mcindex, row) != mc_index) continue;

        const int pindex = bank.get<int16_t>(h_pindex, row);
        const float quality = bank.get<float>(h_quality, row);

        if (quality >= min_quality && pindex >= 0) return pindex;
        break;  // mcindex is unique
    }

    return -1;
}

// =============================================================================
// Legacy Aliases
// =============================================================================

/// @deprecated Use build_index_map instead.
inline auto load_bank_by_index(const hipo::bank_view& bank, const std::string& idx)
    -> std::map<int, std::vector<int>> {
    return build_index_map(bank, idx.c_str());
}

}  // namespace clas12
