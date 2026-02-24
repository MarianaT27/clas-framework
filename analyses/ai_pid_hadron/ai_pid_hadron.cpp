/**
 * @file ai_pid_hadron.cpp
 * @brief AI PID Hadron analysis for CLAS12 data
 */

#include <string>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

#include "AiPidHadronAlg.hpp"

// AI PID-specific parameter
static std::string s_truth_mode = "rich";

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "AI PID Hadron Analysis", "ai_pid_hadron",
        [](auto& builder, const auto& config) {
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::high_q2_region())
                .template add_algorithm<ai_pid_hadron::AiPidHadronAlg>("AiPidHadronAlg", s_truth_mode);
        },
        // Extra CLI options
        [](cxxopts::Options& opts) {
            opts.add_options()
                ("truth-mode", "Truth PID mode: 'rich' or 'exclusive'",
                    cxxopts::value<std::string>()->default_value("rich"));
        },
        // Parse extra options
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& /*config*/) {
            s_truth_mode = args["truth-mode"].as<std::string>();
        });
}
