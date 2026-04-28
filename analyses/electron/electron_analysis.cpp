/**
 * @file electron_analysis.cpp
 * @brief Electron analysis using the ApplicationBuilder pattern
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Electron Analysis", "electron_analysis",
        [](auto& builder, const auto& config) {
            algorithms::ElectronAnalysisConfig alg_config;
            alg_config.target = config.target;
            alg_config.polarity = config.polarity;

            builder
                .template add_service<framework::ElectronCutsService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronAnalysisAlg>("ElectronAnalysis", alg_config);
        });
}
