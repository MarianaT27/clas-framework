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
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

// Pi0-specific cut parameters stored alongside the standard AnalysisConfig
static double s_mass_window = 0.2;

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Pi0 Analysis", "pi0_analysis",
        [](auto& builder, [[maybe_unused]] const auto& config) {
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_service<clas12::framework::PhotonCutsService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>()
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
                .template add_algorithm<clas12::algorithms::PhotonSelectionAlg>("PhotonSelection")
                .template add_algorithm<clas12::algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", s_mass_window);
        },
        // Extra CLI options
        [](cxxopts::Options& opts) {
            opts.add_options()
                ("mass_window", "Pi0 mass window [GeV]", cxxopts::value<double>()->default_value("0.2"));
        },
        // Parse extra options
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& /*config*/) {
            s_mass_window = args["mass_window"].as<double>();
        });
}
