/**
 * @file lambda.cpp
 * @brief Lambda reconstruction analysis (Lambda -> p + pi-)
 */

#include <memory>
#include <string>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/reconstruction/LambdaReconstructionAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/VzCorrectorService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Lambda Analysis", "lambda_analysis",
        [](auto& builder, const auto& config) {
            // LambdaReconstructionAlg's tuning knobs (mass_window,
            // max_delta_vz, max_coplanarity, beam_energy,
            // min/max_displaced_vz) are declared as Property<double>
            // members on the algorithm, so the framework will load them
            // from the [LambdaReconstruction] section of the TOML
            // automatically — no manual configure_from_toml() needed.
            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronSelectionAlg>(
                    "ElectronSelection", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>(
                    "DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::standard_dis_region()})
                .template add_algorithm<algorithms::LambdaReconstructionAlg>(
                    "LambdaReconstruction");
        });
}
