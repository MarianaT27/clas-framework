#pragma once

/**
 * @file EventHelpers.hpp
 * @brief Convenience functions for common event store retrieval patterns
 *
 * Eliminates the repetitive 4-6 line pattern for electron and DIS kinematics
 * retrieval that every downstream algorithm repeats.
 *
 * Example:
 * @code
 * // BEFORE: 4 lines repeated in every algorithm
 * if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return FILTERED();
 * auto electrons_opt = event.retrieve_typed(Keys::SelectedElectrons);
 * if (!electrons_opt || electrons_opt->empty()) return FILTERED();
 * const auto& electron = electrons_opt->front();
 *
 * // AFTER: 2 lines
 * auto electron_opt = clas12::framework::EventHelpers::get_dis_electron(event);
 * if (!electron_opt) return FILTERED();
 * const auto& electron = *electron_opt;
 * @endcode
 */

#include <optional>

#include <clas12/Constants.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/physics/Kinematics.hpp>

namespace clas12::framework::EventHelpers {

/**
 * @brief Retrieve the selected electron (checks HasGoodElectron decision)
 * @return Copy of the first selected electron, or nullopt
 */
inline auto get_selected_electron(Event& event)
    -> std::optional<clas12::Particle> {
    if (!event.passes_decision_typed(Decisions::HasGoodElectron)) return std::nullopt;
    auto electrons_opt = event.retrieve_typed(Keys::SelectedElectrons);
    if (!electrons_opt || electrons_opt->empty()) return std::nullopt;
    return electrons_opt->front();
}

/**
 * @brief Retrieve the DIS electron (checks HasGoodDISElectron decision)
 * @return Copy of the first selected electron, or nullopt
 */
inline auto get_dis_electron(Event& event)
    -> std::optional<clas12::Particle> {
    if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return std::nullopt;
    auto electrons_opt = event.retrieve_typed(Keys::SelectedElectrons);
    if (!electrons_opt || electrons_opt->empty()) return std::nullopt;
    return electrons_opt->front();
}

/**
 * @brief DIS kinematics variables retrieved from the event store
 */
struct DISVars {
    double Q2;
    double W;
    double xB;
    double y;
    double nu;
};

/**
 * @brief Retrieve all DIS kinematics from the event store
 * @return DISVars if all variables are available, nullopt otherwise
 */
inline auto get_dis_kinematics(Event& event) -> std::optional<DISVars> {
    auto Q2 = event.retrieve_typed(Keys::DIS_Q2);
    auto W = event.retrieve_typed(Keys::DIS_W);
    auto xB = event.retrieve_typed(Keys::DIS_xB);
    auto y = event.retrieve_typed(Keys::DIS_y);
    auto nu = event.retrieve_typed(Keys::DIS_nu);
    if (!Q2 || !W || !xB || !y || !nu) return std::nullopt;
    return DISVars{*Q2, *W, *xB, *y, *nu};
}

/**
 * @brief Compute (and cache on the event store) DIS kinematics from
 *        the trigger electron and beam energy.
 *
 * Reads `Keys::TriggerElectron`. If absent, returns nullopt. On the
 * first call for an event, computes Q2/W/xB/y/nu via
 * `clas12::physics::KinematicCalculator::calculate_DIS` and stores
 * each component under `Keys::DIS_*` so subsequent calls for the same
 * event hit the cache. Target is treated as a stationary proton —
 * algorithms that need a different target should compute their own
 * kinematics inline rather than use this helper.
 *
 * @param event Event to read the electron from / cache into.
 * @param beam_energy Beam energy in GeV (electron beam, +z direction).
 * @return DISVars on success, nullopt if no trigger electron or any
 *         derived quantity is non-finite.
 */
inline auto dis_kinematics(Event& event, double beam_energy)
    -> std::optional<DISVars> {
    if (auto cached = get_dis_kinematics(event)) return cached;

    auto electron_opt = event.retrieve_typed(Keys::TriggerElectron);
    if (!electron_opt) return std::nullopt;
    const clas12::Particle& scattered = *electron_opt;

    // Construct beam (along +z, electron mass) and stationary proton target.
    clas12::Particle beam(11, 0, -1, -1, clas12::Constants::ELECTRONMASS,
                          0.0, 0.0, beam_energy, beam_energy,
                          0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    clas12::Particle target(2212, 0, -1, +1, clas12::Constants::PROTONMASS,
                            0.0, 0.0, 0.0, clas12::Constants::PROTONMASS,
                            0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    auto kin = clas12::physics::KinematicCalculator::calculate_DIS(beam, scattered, target);

    DISVars vars{kin.Q2, kin.W, kin.x_bj, kin.y, kin.nu};
    event.store_typed(Keys::DIS_Q2, vars.Q2);
    event.store_typed(Keys::DIS_W,  vars.W);
    event.store_typed(Keys::DIS_xB, vars.xB);
    event.store_typed(Keys::DIS_y,  vars.y);
    event.store_typed(Keys::DIS_nu, vars.nu);
    return vars;
}

}  // namespace clas12::framework::EventHelpers
