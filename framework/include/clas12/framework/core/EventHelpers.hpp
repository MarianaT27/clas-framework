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
 * auto electrons_opt = event.retrieve_typed(Keys::SelectedElectronsCapital);
 * if (!electrons_opt || electrons_opt->empty()) return FILTERED();
 * const auto& electron = electrons_opt->front();
 *
 * // AFTER: 2 lines
 * auto electron_ref = clas12::framework::EventHelpers::get_dis_electron(event);
 * if (!electron_ref) return FILTERED();
 * const auto& electron = electron_ref->get();
 * @endcode
 */

#include <functional>
#include <optional>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>

namespace clas12::framework::EventHelpers {

/**
 * @brief Retrieve the selected electron (checks HasGoodElectron decision)
 * @return Reference to the first selected electron, or nullopt
 */
inline auto get_selected_electron(Event& event)
    -> std::optional<std::reference_wrapper<const clas12::Particle>> {
    if (!event.passes_decision_typed(Decisions::HasGoodElectron)) return std::nullopt;
    auto electrons_opt = event.retrieve_typed(Keys::SelectedElectronsCapital);
    if (!electrons_opt || electrons_opt->empty()) return std::nullopt;
    return std::cref(electrons_opt->front());
}

/**
 * @brief Retrieve the DIS electron (checks HasGoodDISElectron decision)
 * @return Reference to the first selected electron, or nullopt
 */
inline auto get_dis_electron(Event& event)
    -> std::optional<std::reference_wrapper<const clas12::Particle>> {
    if (!event.passes_decision_typed(Decisions::HasGoodDISElectron)) return std::nullopt;
    auto electrons_opt = event.retrieve_typed(Keys::SelectedElectronsCapital);
    if (!electrons_opt || electrons_opt->empty()) return std::nullopt;
    return std::cref(electrons_opt->front());
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

}  // namespace clas12::framework::EventHelpers
