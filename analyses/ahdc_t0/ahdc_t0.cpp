/**
 * @file ahdc_t0.cpp
 * @brief AHDC t0 (timing pedestal) extraction from the uncalibrated AHDC::adc
 *        bank, per wire, for all hits and for hits on tracks only.
 */

#include <clas12/framework/core/AnalysisRunner.hpp>

#include "AhdcT0Alg.hpp"

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "AHDC t0 Analysis", "ahdc_t0",
        []([[maybe_unused]] auto& builder, [[maybe_unused]] const auto& config) {
            builder.template add_algorithm<ahdc_t0::AhdcT0Alg>("AhdcT0Alg");
        });
}
