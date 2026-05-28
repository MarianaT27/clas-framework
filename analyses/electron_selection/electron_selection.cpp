/**
 * @file electron_selection.cpp
 * @brief Lean framework baseline: runs ONLY ElectronSelectionAlg.
 *
 * Mirrors analyses/electron/electron_analysis.cpp but wires the lean
 * selection algorithm (no EventMonitor, no heavy ElectronAnalysisAlg) so
 * the full-framework cost measured here is the closest possible match to the
 * chain-only tools/benchmark_electron program. The gap between the two is
 * the framework's per-event EDM overhead. See tools/compare_electron_benchmark.sh.
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/services/VzCorrectorService.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Electron Selection", "electron_selection",
        [](auto& builder, const auto& config) {
            // VzCorrectorService is a no-op for LD2 (legacy fixed window);
            // configured here for parity with the analysis wiring so solid
            // targets behave identically if benchmarked later.
            framework::add_svc<framework::VzCorrectorService>(builder,
                framework::VzCorrectorService::Config{.target = config.target,
                                                      .polarity = config.polarity});
            framework::add_svc<framework::ElectronCutsService>(builder);
            framework::add_alg<algorithms::ElectronSelectionAlg>(builder, "ElectronSelection");
        });
}
