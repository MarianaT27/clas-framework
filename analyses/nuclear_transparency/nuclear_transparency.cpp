/**
 * @file nuclear_transparency.cpp
 * @brief Nuclear transparency analysis
 */

#include <string>
#include <string_view>
#include <vector>

#include <toml++/toml.hpp>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/algorithms/selection/PionSelectionAlg.hpp>
#include <clas12/algorithms/reconstruction/Rho0ReconstructionAlg.hpp>
#include <clas12/framework/services/DecayTreeFitterService.hpp>
#include <clas12/framework/services/ElectronCutsService.hpp>
#include <clas12/framework/services/VzCorrectorService.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/StringUtils.hpp>

namespace {
// Pull a scan vector out of a toml::table at `[scans].<key>` (a TOML array of doubles).
auto read_scan(const toml::table& root, std::string_view key) -> std::vector<double> {
    std::vector<double> out;
    if (auto arr = root["scans"][key].as_array()) {
        out.reserve(arr->size());
        for (auto const& node : *arr) {
            if (auto v = node.value<double>()) out.push_back(*v);
        }
    }
    return out;
}
}  // namespace

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Nuclear Transparency Analysis", "nuclear_transparency_analysis",
        [](auto& builder, const auto& config) {
            // Scan parameters live under [scans] in the TOML, e.g.
            //   [scans]
            //   sigma_scan = [2.0, 2.5, 3.0, 3.5, 4.0]
            //   lc_scan    = [0.5, 0.75, 1.0, 1.25, 1.5, 2.0]
            //   ...
            std::vector<double> sigma_scan, lc_scan, zh_scan, t_scan, dvz_scan;
            if (config.config_table) {
                const auto& root = *config.config_table;
                sigma_scan = read_scan(root, "sigma_scan");
                lc_scan    = read_scan(root, "lc_scan");
                zh_scan    = read_scan(root, "zh_scan");
                t_scan     = read_scan(root, "t_scan");
                dvz_scan   = read_scan(root, "dvz_scan");
            }

            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_service<framework::DecayTreeFitterService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::KinematicRegion{.q2_min = 1.0, .w_min = 2.0}})
                .template add_algorithm<algorithms::PionSelectionAlg>("PionSelection", algorithms::PionSelectionConfig{.target = config.target, .polarity = config.polarity, .sigma_scan = sigma_scan})
                .template add_algorithm<algorithms::Rho0ReconstructionAlg>("Rho0Reconstruction", algorithms::Rho0ReconstructionConfig{.target = config.target, .sigma_scan = sigma_scan, .lc_scan = lc_scan, .zh_scan = zh_scan, .t_scan = t_scan, .dvz_scan = dvz_scan});
        });
}
