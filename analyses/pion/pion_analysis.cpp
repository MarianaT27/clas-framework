#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/analysis/PionAnalysisAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/VzCorrectorService.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Pion Analysis", "pion_analysis",
        [](auto& builder, const auto& config) {
            algorithms::ElectronAnalysisConfig electron_config;
            electron_config.target = config.target;
            electron_config.polarity = config.polarity;

            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronAnalysisAlg>("ElectronAnalysis", electron_config)
                .template add_algorithm<algorithms::PionAnalysisAlg>("PionAnalysis", algorithms::PionAnalysisConfig{.target = config.target, .polarity = config.polarity});
        });
}
