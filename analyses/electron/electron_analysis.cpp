/**
 * @file electron_analysis.cpp
 * @brief Electron analysis using the ApplicationBuilder pattern
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/ElectronAnalysisAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/services/VzCorrectorService.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Electron Analysis", "electron_analysis",
        [](auto& builder, const auto& config) {
            // target/polarity flow from `[analysis]` in the TOML file directly
            // into ElectronAnalysisConfig — see config_fields() in
            // ElectronAnalysisAlg.hpp. No manual plumbing needed here.
            framework::add_svc<framework::VzCorrectorService>(builder,
                framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity});
            framework::add_svc<framework::ElectronCutsService>(builder);
            framework::add_alg<algorithms::EventMonitor>(builder, "EventMonitor");
            framework::add_alg<algorithms::ElectronAnalysisAlg>(builder, "ElectronAnalysis");
        });
}
