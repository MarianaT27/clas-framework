/**
 * @file pion_selection.cpp
 * @brief Pion selection with cutflow: trigger electron -> DIS -> PionSelectionAlg.
 *
 * Counterpart of electron_selection for charged pions. Same algorithm chain as the
 * dipion / nuclear-transparency analyses (without the rho0 reconstruction):
 *   1. ElectronSelectionAlg ("ElectronSelection") - trigger electron, all electron cuts
 *   2. DISSelectionAlg      ("DISSelection")      - Q2 >= 1.0 GeV^2, W >= 2.0 GeV
 *   3. PionSelectionAlg     ("PionSelection")     - DC fiducial, chi2pid, momentum, Vz,
 *                                                   Delta-Vz, Delta-t
 * The pi+ and pi- cutflows are printed at the end of the run (the pion version of
 * Table 5 of the IB mini note). Target and polarity come from [analysis] in the TOML;
 * every cut switch and threshold is set in the [ElectronSelection] / [PionSelection]
 * sections.
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/selection/PionSelectionAlg.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/services/VzCorrectorService.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Pion Selection", "pion_selection",
        [](auto& builder, const auto& config) {
            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelection", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::KinematicRegion{.q2_min = 1.0, .w_min = 2.0}})
                .template add_algorithm<algorithms::PionSelectionAlg>("PionSelection", algorithms::PionSelectionConfig{.target = config.target, .polarity = config.polarity});
        });
}
