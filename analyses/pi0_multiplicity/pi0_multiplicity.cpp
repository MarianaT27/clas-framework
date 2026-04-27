/**
 * @file pi0_multiplicity.cpp
 * @brief Pi0 multiplicity ratio analysis with event-mixing background estimation
 *
 * Produces histograms binned in DIS kinematics (Q2, xB, z, pT2)
 * for computing the multiplicity ratio R_A = N_A^pi0 / N_D^pi0.
 * Also produces same-event and mixed-event invariant mass distributions
 * in 4D kinematic bins for background subtraction and signal extraction.
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/selection/PhotonSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/reconstruction/Pi0ReconstructionAlg.hpp>
#include <clas12/algorithms/analysis/Pi0MultiplicityRatioAlg.hpp>
#include <clas12/algorithms/analysis/Pi0EventMixingAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Pi0 Multiplicity Ratio", "pi0_multiplicity",
        [](auto& builder, const auto& config) {
            // Constructor args: pull from each algorithm's TOML section.
            double mass_window = 0.2;
            int    max_pool_events = 50;
            if (config.config_table) {
                if (auto v = (*config.config_table)["Pi0Reconstruction"]["mass_window"].template value<double>())
                    mass_window = *v;
                if (auto v = (*config.config_table)["Pi0EventMixing"]["max_pool_events"].template value<int64_t>())
                    max_pool_events = static_cast<int>(*v);
            }
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::PhotonCutsService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>()
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
                .template add_algorithm<clas12::algorithms::PhotonSelectionAlg>("PhotonSelection")
                .template add_algorithm<clas12::algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", mass_window)
                .template add_algorithm<clas12::algorithms::Pi0MultiplicityRatioAlg>("Pi0Multiplicity")
                .template add_algorithm<clas12::algorithms::Pi0EventMixingAlg>("Pi0EventMixing", max_pool_events);
        });
}
