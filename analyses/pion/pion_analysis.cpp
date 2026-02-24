/**
 * @file pion_analysis.cpp
 * @brief Pion analysis with calorimeter SF and DC quality histograms
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/analysis/PionAnalysisAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/FiducialService.hpp>

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Pion Analysis", "pion_analysis",
        [](auto& builder, const auto& config) {
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>("ElectronSelection", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::PionAnalysisAlg>("PionAnalysis", config.target, config.polarity);
        });
}
