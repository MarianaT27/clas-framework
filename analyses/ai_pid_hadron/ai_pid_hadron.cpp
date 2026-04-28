/**
 * @file ai_pid_hadron.cpp
 * @brief AI PID Hadron analysis for CLAS12 data
 */

#include <string>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/physics/DISAnalysis.hpp>

#include "AiPidHadronAlg.hpp"

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "AI PID Hadron Analysis", "ai_pid_hadron",
        [](auto& builder, const auto& config) {
            // truth_mode is a constructor arg; pull it from the
            // [AiPidHadronAlg].truth_mode TOML key. Default "rich" if
            // missing or no config file.
            std::string truth_mode = "rich";
            if (config.config_table) {
                if (auto v = (*config.config_table)["AiPidHadronAlg"]["truth_mode"].template value<std::string>())
                    truth_mode = *v;
            }
            builder
                .template add_service<framework::ElectronCutsService>()
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::high_q2_region()})
                .template add_algorithm<ai_pid_hadron::AiPidHadronAlg>("AiPidHadronAlg", ai_pid_hadron::AiPidHadronConfig{.truth_mode = truth_mode});
        });
}
