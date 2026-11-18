/**
 * @file photon_analysis.cpp
 * @brief Photon identification check (photon_check/): cutflow and cut distributions.
 *
 * Chain, the same electron and DIS selection as the pi0 analysis (pi0_multiplicity.cpp):
 *   1. ElectronSelectionAlg ("ElectronSelection") - trigger electron, all electron cuts of the
 *                                                   [analysis] target and polarity
 *   2. DISSelectionAlg      ("DISSelection")      - standard DIS region: 1 <= Q2 <= 20 GeV^2,
 *                                                   2 <= W <= 25 GeV, y <= 0.85
 *   3. PhotonAnalysisAlg    ("PhotonAnalysis")    - the 8 photon steps, cutflow and histograms
 * Every cut switch and threshold is set in the [ElectronSelection] / [PhotonAnalysis] sections.
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/PhotonAnalysisAlg.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/services/PhotonCutsService.hpp>
#include <clas12/services/VzCorrectorService.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Photon Analysis", "photon_analysis",
        [](auto& builder, const auto& config) {
            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_service<framework::PhotonCutsService>()
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelection", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::standard_dis_region()})
                .template add_algorithm<algorithms::PhotonAnalysisAlg>("PhotonAnalysis", algorithms::PhotonAnalysisConfig{.target = config.target, .polarity = config.polarity});
        });
}
