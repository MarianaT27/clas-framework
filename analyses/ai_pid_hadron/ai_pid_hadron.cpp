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

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
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
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::high_q2_region())
                .template add_algorithm<ai_pid_hadron::AiPidHadronAlg>("AiPidHadronAlg", truth_mode);
        });
}
