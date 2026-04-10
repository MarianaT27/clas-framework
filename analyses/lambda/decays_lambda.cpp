/**
 * @file decays_lambda.cpp
 * @brief Lambda analysis using DECAYS::Particle bank
 *
 * This executable reads pre-reconstructed Lambda candidates from the
 * DECAYS::Particle bank and performs analysis.
 */

#include <memory>
#include <string>

#include <toml++/toml.hpp>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/algorithms/analysis/DecaysLambdaAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "DECAYS Lambda Analysis", "decays_lambda_analysis",
        [](auto& builder, const auto& config) {
            // Create algorithm and configure properties from the parsed CLI options
            // stored in config.custom (same keys the ParseExtraFn writes below).
            auto decays_lambda_alg = std::make_unique<clas12::algorithms::DecaysLambdaAlg>("DecaysLambdaAnalysis");
            decays_lambda_alg->configure_from_toml(config.custom);

            builder
                .template add_algorithm<clas12::algorithms::EventMonitor>("EventMonitor")
                .add_algorithm(std::move(decays_lambda_alg));
        },
        // Extra CLI options
        [](cxxopts::Options& opts) {
            opts.add_options()
                ("mass_window", "Lambda mass window [GeV]", cxxopts::value<double>()->default_value("0.2"))
                ("min_momentum", "Minimum Lambda momentum [GeV]", cxxopts::value<double>()->default_value("0.0"))
                ("min_decay_length", "Minimum decay length [cm] (0=disabled)", cxxopts::value<double>()->default_value("0.0"))
                ("min_vertex_detach", "Min vertex detachment z [cm]", cxxopts::value<double>()->default_value("1.0"))
                ("max_vertex_detach", "Max vertex detachment z [cm] (0=no limit)", cxxopts::value<double>()->default_value("100"));
        },
        // Parse extra options
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& config) {
            config.custom.insert_or_assign("mass_window",       args["mass_window"].as<double>());
            config.custom.insert_or_assign("min_momentum",      args["min_momentum"].as<double>());
            config.custom.insert_or_assign("min_decay_length",  args["min_decay_length"].as<double>());
            config.custom.insert_or_assign("min_vertex_detach", args["min_vertex_detach"].as<double>());
            config.custom.insert_or_assign("max_vertex_detach", args["max_vertex_detach"].as<double>());
        });
}
