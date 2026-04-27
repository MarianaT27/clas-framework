#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/analysis/PionAnalysisAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Pion Analysis", "pion_analysis",
        [](auto& builder, const auto& config) {
            clas12::algorithms::ElectronAnalysisConfig electron_config;
            electron_config.target = config.target;
            electron_config.polarity = config.polarity;

            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronAnalysisAlg>("ElectronAnalysis", electron_config)
                .template add_algorithm<clas12::algorithms::PionAnalysisAlg>("PionAnalysis", clas12::algorithms::PionAnalysisConfig{.target = config.target, .polarity = config.polarity});
        });
}
