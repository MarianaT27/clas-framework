/**
 * @file nuclear_transparency.cpp
 * @brief Nuclear transparency analysis
 */

#include <string>
#include <vector>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/selection/PionSelectionAlg.hpp>
#include <clas12/algorithms/reconstruction/Rho0ReconstructionAlg.hpp>
#include <clas12/framework/services/DecayTreeFitterService.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/FiducialService.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/StringUtils.hpp>

// Scan values stored alongside the standard AnalysisConfig
static std::vector<double> s_sigma_scan_values;
static std::vector<double> s_lc_scan_values;
static std::vector<double> s_zh_scan_values;
static std::vector<double> s_t_scan_values;
static std::vector<double> s_dvz_scan_values;

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "Nuclear Transparency Analysis", "nuclear_transparency_analysis",
        [](auto& builder, const auto& config) {
            builder
                .template add_service<clas12::framework::ElectronCutsService>()
                .template add_service<clas12::framework::FiducialService>()
                .template add_service<clas12::framework::DecayTreeFitterService>()
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<clas12::algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", config.target, config.polarity)
                .template add_algorithm<clas12::algorithms::DISSelectionAlg>("DISSelection", clas12::physics::DISAnalysis::KinematicRegion{.q2_min = 1.0, .w_min = 2.0})
                .template add_algorithm<clas12::algorithms::PionSelectionAlg>("PionSelection", config.target, true, s_sigma_scan_values)
                .template add_algorithm<clas12::algorithms::Rho0ReconstructionAlg>("Rho0Reconstruction", config.target, s_sigma_scan_values,
                    s_lc_scan_values, s_zh_scan_values, s_t_scan_values, s_dvz_scan_values);
        },
        // Extra CLI options
        [](cxxopts::Options& opts) {
            opts.add_options()
                ("sigma-scan", "Sigma values for delta_t scan (e.g., '2.0,2.5,3.0,3.5,4.0')",
                    cxxopts::value<std::string>()->default_value(""))
                ("lc-scan", "lc_max values for coherence length scan (e.g., '0.5,0.75,1.0,1.25,1.5,2.0')",
                    cxxopts::value<std::string>()->default_value(""))
                ("zh-scan", "zh_min values for energy fraction scan (e.g., '0.7,0.8,0.85,0.9,0.95')",
                    cxxopts::value<std::string>()->default_value(""))
                ("t-scan", "t_max values for momentum transfer scan (e.g., '0.3,0.4,0.5,0.6,0.7')",
                    cxxopts::value<std::string>()->default_value(""))
                ("dvz-scan", "N-sigma for delta_vz scan (e.g., '2.0,2.5,3.0,3.5,4.0')",
                    cxxopts::value<std::string>()->default_value(""));
        },
        // Parse extra options
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& /*config*/) {
            auto parse_scan = [&args](const std::string& key, std::vector<double>& out) {
                std::string str = args[key].as<std::string>();
                if (!str.empty()) {
                    auto parts = clas12::StringUtils::split(str, ',');
                    for (const auto& part : parts) {
                        out.push_back(std::stod(part));
                    }
                }
            };

            parse_scan("sigma-scan", s_sigma_scan_values);
            parse_scan("lc-scan", s_lc_scan_values);
            parse_scan("zh-scan", s_zh_scan_values);
            parse_scan("t-scan", s_t_scan_values);
            parse_scan("dvz-scan", s_dvz_scan_values);
        });
}
