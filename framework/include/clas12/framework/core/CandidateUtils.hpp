#pragma once

/**
 * @file CandidateUtils.hpp
 * @brief Shared utilities for particle candidate construction
 *
 * Centralizes timing calculations (delta-t, TOF mass², beta) and detector
 * region classification that are duplicated across pion/hadron algorithms.
 */

#include <cmath>
#include <clas12/Constants.hpp>

namespace clas12::framework::CandidateUtils {

/**
 * @brief Result of time-of-flight calculations
 */
struct TimingResult {
    double beta_expected = 0.0;  ///< Expected beta from mass hypothesis
    double delta_t = 0.0;       ///< Measured - expected time
    double tof_mass2 = 0.0;     ///< TOF mass squared: p²(1/β² - 1)
    double beta_diff = 0.0;     ///< Measured beta - expected beta
};

/**
 * @brief Compute timing quantities from TOF information
 *
 * @param p Particle momentum [GeV/c]
 * @param mass Mass hypothesis [GeV/c²]
 * @param beta_measured Measured beta from tracking
 * @param tof_time Time-of-flight measured time [ns]
 * @param tof_path TOF path length [cm]
 * @param start_time Event start time [ns]
 * @return TimingResult with all derived timing quantities
 */
inline auto compute_timing(double p, double mass, double beta_measured,
                           double tof_time, double tof_path, double start_time) -> TimingResult {
    TimingResult result;
    result.beta_expected = p / std::sqrt(p * p + mass * mass);

    if (tof_path > 0) {
        double t_expected = tof_path / (result.beta_expected * clas12::Constants::SPEED_OF_LIGHT);
        double measured_time = tof_time - start_time;
        result.delta_t = measured_time - t_expected;
    }

    if (beta_measured > 0) {
        result.tof_mass2 = p * p * (1.0 / (beta_measured * beta_measured) - 1.0);
    }

    result.beta_diff = beta_measured - result.beta_expected;
    return result;
}

/**
 * @brief Check if particle is in the forward detector (CLAS12 FD)
 * @param status Particle status code from REC::Particle
 */
inline auto is_forward_detector(int status) -> bool {
    return std::abs(status) >= 2000 && std::abs(status) < 4000;
}

/**
 * @brief Check if particle is in the central detector (CLAS12 CD)
 * @param status Particle status code from REC::Particle
 */
inline auto is_central_detector(int status) -> bool {
    return std::abs(status) >= 4000 && std::abs(status) < 6000;
}

}  // namespace clas12::framework::CandidateUtils
