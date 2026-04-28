/**
 * @file decays_lambda.cpp
 * @brief Lambda analysis using DECAYS::Particle bank
 *
 * This executable reads pre-reconstructed Lambda candidates from the
 * DECAYS::Particle bank and performs analysis.
 */

#include <memory>
#include <string>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/DecaysLambdaAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "DECAYS Lambda Analysis", "decays_lambda_analysis",
        [](auto& builder, const auto& /*config*/) {
            // Tuning knobs (mass_window, min_momentum, min_decay_length,
            // min/max_vertex_detach) are Property<double> members on
            // DecaysLambdaAlg and load automatically from the
            // [DecaysLambdaAnalysis] TOML section.
            builder
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::DecaysLambdaAlg>("DecaysLambdaAnalysis");
        });
}
