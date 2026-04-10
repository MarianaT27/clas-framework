/**
 * @file lambda.cpp
 * @brief Lambda reconstruction analysis (Lambda -> p + pi-)
 */

#include <memory>
#include <string>

#include <toml++/toml.hpp>

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
            // Create lambda algorithm and configure properties from config.custom,
            // which contains the parsed CLI options. configure_from_toml reads the
            // same key names that ParseExtraFn wrote below.
            auto lambda_alg = std::make_unique<clas12::algorithms::LambdaReconstructionAlg>("LambdaReconstruction");
            lambda_alg->configure_from_toml(config.custom);

            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>(
                    "ElectronSelection", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>(
                    "DISSelection", clas12::physics::DISAnalysis::standard_dis_region())
                .add_algorithm(std::move(lambda_alg));
        },
        // Extra CLI options
        [](cxxopts::Options& opts) {
            opts.add_options()
                ("mass_window", "Lambda mass window [GeV]", cxxopts::value<double>()->default_value("0.5"))
                ("max_delta_vz", "Max vertex z difference [cm]", cxxopts::value<double>()->default_value("5.0"))
                ("coplanarity", "Max coplanarity angle [deg] (0=disabled)", cxxopts::value<double>()->default_value("0"))
                ("beam_energy", "Beam energy [GeV]", cxxopts::value<double>()->default_value("10.53"))
                ("min_displaced_vz", "Min displaced vertex z [cm]", cxxopts::value<double>()->default_value("-1000"))
                ("max_displaced_vz", "Max displaced vertex z [cm] (0=no limit)", cxxopts::value<double>()->default_value("0"));
        },
        // Parse extra options
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& config) {
            config.custom.insert_or_assign("mass_window",      args["mass_window"].as<double>());
            config.custom.insert_or_assign("max_delta_vz",     args["max_delta_vz"].as<double>());
            config.custom.insert_or_assign("max_coplanarity",  args["coplanarity"].as<double>());
            config.custom.insert_or_assign("beam_energy",      args["beam_energy"].as<double>());
            config.custom.insert_or_assign("min_displaced_vz", args["min_displaced_vz"].as<double>());
            config.custom.insert_or_assign("max_displaced_vz", args["max_displaced_vz"].as<double>());
        });
}
