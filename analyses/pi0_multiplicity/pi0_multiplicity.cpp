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
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

// Pi0-specific cut parameters
static double s_mass_window = 0.2;
static int s_max_pool_events = 10;

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Pi0 Multiplicity Ratio", "pi0_multiplicity",
        [](auto& builder, [[maybe_unused]] const auto& config) {
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_service<clas12::framework::PhotonCutsService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>()
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
                .template add_algorithm<clas12::algorithms::PhotonSelectionAlg>("PhotonSelection")
                .template add_algorithm<clas12::algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", s_mass_window)
                .template add_algorithm<clas12::algorithms::Pi0MultiplicityRatioAlg>("Pi0Multiplicity")
                .template add_algorithm<clas12::algorithms::Pi0EventMixingAlg>("Pi0EventMixing", s_max_pool_events);
        },
        // Extra CLI options
        [](cxxopts::Options& opts) {
            opts.add_options()
                ("mass_window", "Pi0 mass window [GeV]", cxxopts::value<double>()->default_value("0.2"))
                ("max_pool_events", "Max events per DIS bin in mixing pool", cxxopts::value<int>()->default_value("10"));
        },
        // Parse extra options
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& /*config*/) {
            s_mass_window = args["mass_window"].as<double>();
            s_max_pool_events = args["max_pool_events"].as<int>();
        });
}
