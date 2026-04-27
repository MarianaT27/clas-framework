/**
 * @file CuSn_contamination.cpp
 * @brief CuSn Contamination analysis for CLAS12 data
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>

#include "CusnContaminationAlg.hpp"

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "CuSn Contamination Analysis", "CuSn_contamination",
        [](auto& builder, const auto& config) {
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", clas12::algorithms::ElectronSelectionConfig{.target = clas12::Target::Custom, .polarity = config.polarity})
                .template add_algorithm<CuSnContamination::CuSnContaminationAlg>("CuSnContaminationAlg");
        });
}
