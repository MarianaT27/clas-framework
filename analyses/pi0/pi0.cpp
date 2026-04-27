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
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
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
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::PhotonCutsService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>()
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
                .template add_algorithm<clas12::algorithms::PhotonSelectionAlg>("PhotonSelection")
                .template add_algorithm<clas12::algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", mass_window);
        });
}
