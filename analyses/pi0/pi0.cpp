/**
 * @file pi0.cpp
 * @brief Pi0 reconstruction analysis using ApplicationBuilder pattern
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/selection/PhotonSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/services/PhotonCutsService.hpp>
#include <clas12/services/VzCorrectorService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Pi0 Analysis", "pi0_analysis",
        [](auto& builder, const auto& config) {
            // mass_window is a constructor arg (not a Property) so it
            // can't be picked up by the auto per-algorithm config
            // path. Pull it from the [Pi0Reconstruction] section
            // directly. Default 0.2 if missing or no config file.
            double mass_window = 0.2;
            if (config.config_table) {
                if (auto v = (*config.config_table)["Pi0Reconstruction"]["mass_window"].template value<double>())
                    mass_window = *v;
            }
            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_service<framework::PhotonCutsService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronSelectionAlg>()
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::standard_dis_region()})
                .template add_algorithm<algorithms::PhotonSelectionAlg>("PhotonSelection")
                .template add_algorithm<algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", algorithms::Pi0ReconstructionConfig{.pi0_mass_window = mass_window});
        });
}
