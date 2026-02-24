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

// Lambda-specific cut parameters stored alongside the standard AnalysisConfig
static double s_mass_window = 0.2;
static double s_min_momentum = 0.0;
static double s_min_decay_length = 0.0;
static double s_min_vertex_detach = 1.0;
static double s_max_vertex_detach = 100.0;

auto main(int argc, char* argv[]) -> int {
    return clas12::framework::AnalysisRunner::run(
        argc, argv, "DECAYS Lambda Analysis", "decays_lambda_analysis",
        [](auto& builder, [[maybe_unused]] const auto& config) {
            // Create algorithm with cuts configured via Property system
            auto decays_lambda_alg = std::make_unique<clas12::algorithms::DecaysLambdaAlg>("DecaysLambdaAnalysis");

            // Configure properties via TOML table
            toml::table decays_config;
            decays_config.insert("mass_window", s_mass_window);
            decays_config.insert("min_momentum", s_min_momentum);
            decays_config.insert("min_decay_length", s_min_decay_length);
            decays_config.insert("min_vertex_detach", s_min_vertex_detach);
            decays_config.insert("max_vertex_detach", s_max_vertex_detach);
            decays_lambda_alg->configure_from_toml(decays_config);

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
        [](const cxxopts::ParseResult& args, clas12::framework::AnalysisConfig& /*config*/) {
            s_mass_window = args["mass_window"].as<double>();
            s_min_momentum = args["min_momentum"].as<double>();
            s_min_decay_length = args["min_decay_length"].as<double>();
            s_min_vertex_detach = args["min_vertex_detach"].as<double>();
            s_max_vertex_detach = args["max_vertex_detach"].as<double>();
        });
}
