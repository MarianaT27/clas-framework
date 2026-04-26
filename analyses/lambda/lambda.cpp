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
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Lambda Analysis", "lambda_analysis",
        [](auto& builder, const auto& config) {
            // LambdaReconstructionAlg's tuning knobs (mass_window,
            // max_delta_vz, max_coplanarity, beam_energy,
            // min/max_displaced_vz) are declared as Property<double>
            // members on the algorithm, so the framework will load them
            // from the [LambdaReconstruction] section of the TOML
            // automatically — no manual configure_from_toml() needed.
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>(
                    "ElectronSelection", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>(
                    "DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
                .template add_algorithm<clas12::algorithms::LambdaReconstructionAlg>(
                    "LambdaReconstruction");
        });
}
