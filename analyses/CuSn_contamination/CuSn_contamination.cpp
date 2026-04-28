/**
 * @file CuSn_contamination.cpp
 * @brief CuSn Contamination analysis for CLAS12 data
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>

#include "CusnContaminationAlg.hpp"

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "CuSn Contamination Analysis", "CuSn_contamination",
        [](auto& builder, const auto& config) {
            builder
                .template add_service<framework::ElectronCutsService>()
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", algorithms::ElectronSelectionConfig{.target = Target::Custom, .polarity = config.polarity})
                .template add_algorithm<CuSnContamination::CuSnContaminationAlg>("CuSnContaminationAlg");
        });
}
