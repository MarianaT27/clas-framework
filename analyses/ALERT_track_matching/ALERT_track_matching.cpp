/**
 * @file ALERT_track_matching.cpp
 * @brief ALERT Track Matching analysis for CLAS12 data
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/ElasticEPAlg.hpp>

#include "ALERT_track_matchingAlg.hpp"

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "ALERT Track Matching Analysis", "ALERT_track_matching",
        []([[maybe_unused]] auto& builder, [[maybe_unused]] const auto& config) {
            constexpr double beam_energy = 2.2;  // GeV for ALERT data
            builder
                .template add_algorithm<clas12::algorithms::ElasticEPAlg>("ElasticEPAlg", clas12::algorithms::NuclearTarget::Deuteron, beam_energy)
                .template add_algorithm<ALERT_track_matching::ALERT_track_matchingAlg>("ALERT_track_matchingAlg");
        });
}
