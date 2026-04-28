/**
 * @file example_framework.cpp
 * @brief Minimal end-to-end example using the new --config TOML flow.
 *
 * Run with:
 *   ./framework_example --config config/example_framework.toml
 *
 * Demonstrates:
 *   - AnalysisRunner: single-flag CLI; everything else lives in TOML
 *   - PropertyMixin: each [<AlgorithmName>] section auto-applies to the
 *     algorithm's `Property<T>` declarations
 *   - Builder + standard services + a custom service
 */

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/framework/services/SamplingFractionService.hpp>
#include <clas12/framework/services/ConditionsService.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Example Framework Analysis", "framework_example",
        [](auto& builder, const auto& config) {
            // Run number for conditions IOV — pull from
            // [conditions].run_number in the TOML.
            int run_number = 0;
            if (config.config_table) {
                if (auto v = (*config.config_table)["conditions"]["run_number"]
                                .template value<int64_t>())
                    run_number = static_cast<int>(*v);
            }

            builder
                .template add_service<framework::SamplingFractionService>()
                .template configure_service<framework::ConditionsService>(
                    [run_number](framework::ConditionsService& svc) {
                        svc.set_run_number(run_number);
                    })
                .template add_algorithm<algorithms::ElectronSelectionAlg>(
                    "ElectronSelection", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity});
        });
}
