/**
 * @file main_lambda.cpp
 * @brief Lambda reconstruction analysis executable
 *
 * This executable chains electron selection with Λ → p + π⁻ reconstruction
 * and writes results to an NTuple.
 */

#include <fmt/format.h>
#include <cstdlib>
#include <cxxopts.hpp>
#include <string>
#include <TROOT.h>
#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/LambdaReconstructionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "CLAS/FileUtils.hpp"
#include "CLAS/Runs.hpp"
#include "CLAS/StringUtils.hpp"
#include "CLAS/Target.hpp"
#include "Framework/Application.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/MetadataService.hpp"
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/ProfilingService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"
#include "Physics/DISAnalysis.hpp"
#include "Physics/Kinematics.hpp"
#include "fmt/base.h"

int main(int argc, char* argv[]) {
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();
    
    cxxopts::Options options("lambda_analysis", "Lambda reconstruction analysis (Λ → p + π⁻)");

    options.add_options() 
            ("target", "Target type (LD2, CxC, CuSn)", cxxopts::value<std::string>()->default_value("LD2"))
            ("polarity", "Magnetic field polarity (Inbending, Outbending)", cxxopts::value<std::string>()->default_value("Outbending"))
            ("percentage", "Percentage of events to process (0-100)", cxxopts::value<double>()->default_value("100"))
            ("num_threads", "Number of threads", cxxopts::value<int>()->default_value("1"))
            ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("lambda_analysis_{target}_{polarity}.root"))
            ("input", "Input HIPO files (comma-separated or single file)", cxxopts::value<std::string>()->default_value(""))
            ("mass_window", "Lambda mass window in GeV", cxxopts::value<double>()->default_value("0.5"))
            ("max_delta_vz", "Max vertex z difference in cm", cxxopts::value<double>()->default_value("5.0"))
            ("coplanarity", "Max coplanarity angle in deg (0=disabled)", cxxopts::value<double>()->default_value("0"))
            ("beam_energy", "Beam energy in GeV", cxxopts::value<double>()->default_value("10.53"))
            ("min_displaced_vz", "Min displaced vertex z in cm (decay_vz - primary_vz)", cxxopts::value<double>()->default_value("-1000"))
            ("max_displaced_vz", "Max displaced vertex z in cm (0=no limit)", cxxopts::value<double>()->default_value("0"))
            ("help", "Show help");

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return EXIT_SUCCESS;
    }

    double percentage = args["percentage"].as<double>();
    int num_threads = args["num_threads"].as<int>();
    std::string output_file = args["output"].as<std::string>();
    std::string target_str = args["target"].as<std::string>();
    std::string polarity_str = args["polarity"].as<std::string>();
    std::string input_str = args["input"].as<std::string>();
    double mass_window = args["mass_window"].as<double>();
    double max_delta_vz = args["max_delta_vz"].as<double>();
    double coplanarity = args["coplanarity"].as<double>();
    double beam_energy = args["beam_energy"].as<double>();
    double min_displaced_vz = args["min_displaced_vz"].as<double>();
    double max_displaced_vz = args["max_displaced_vz"].as<double>();
    
    CLAS::Target target = CLAS::string_to_target(target_str);
    CLAS::Polarity polarity = CLAS::string_to_polarity(polarity_str);

    output_file = CLAS::StringUtils::replace_all(output_file, "{target}", CLAS::to_string(target));
    output_file = CLAS::StringUtils::replace_all(output_file, "{polarity}", CLAS::to_string(polarity));

    // Determine input files
    std::vector<std::string> input_files = {"/work/clas12/skuditha/skims/phys_val/ana_v2/020082_lambda.hipo"};
    // std::vector<std::string> input_files = CLAS::FileUtils::read_recursive_hipo_files("/volatile/clas12/rg-e/production/dst/phys_val/dst/recon/020082/");

    // Create application
    Framework::Application app("Lambda Analysis");

    // Create services
    auto log_svc = std::make_unique<Framework::LoggingService>();
    auto ntuple_svc = std::make_unique<Framework::NTupleService>();
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    auto conditions_svc = std::make_unique<Framework::ConditionsService>();
    auto metadata_svc = std::make_unique<Framework::MetadataService>();
    auto profiling_svc = std::make_unique<Framework::ProfilingService>();
    auto sf_svc = std::make_unique<Framework::SamplingFractionService>();
    auto particle_svc = std::make_unique<Framework::ParticleSelectorService>();

    // Configure output
    ntuple_svc->set_output_file(output_file);
    hist_svc->set_output_file(output_file);
    metadata_svc->set_output_file(output_file);
    ntuple_svc->enable_incremental_writing();

    // Configure metadata
    metadata_svc->set_version("1.0");
    metadata_svc->set_beam_energy(10.53);
    metadata_svc->set_target(CLAS::to_string(target));
    metadata_svc->set_command_line(argc, argv);
    metadata_svc->add_parameter("polarity", CLAS::to_string(polarity));
    metadata_svc->add_parameter("percentage", percentage);
    metadata_svc->add_parameter("num_threads", num_threads);
    metadata_svc->add_parameter("mass_window", mass_window);
    metadata_svc->add_parameter("max_delta_vz", max_delta_vz);
    metadata_svc->add_parameter("coplanarity", coplanarity);
    metadata_svc->add_parameter("beam_energy", beam_energy);
    metadata_svc->add_parameter("min_displaced_vz", min_displaced_vz);
    metadata_svc->add_parameter("max_displaced_vz", max_displaced_vz);

    // Add services
    app.service_manager().add_service(std::move(log_svc));
    app.service_manager().add_service(std::move(conditions_svc));
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(ntuple_svc));
    app.service_manager().add_service(std::move(metadata_svc));
    app.service_manager().add_service(std::move(profiling_svc));
    app.service_manager().add_service(std::move(sf_svc));
    app.service_manager().add_service(std::move(particle_svc));

    // Add algorithms
    auto monitoring_alg = std::make_unique<Algorithms::EventMonitor>("EventMonitor");
    app.add_algorithm(std::move(monitoring_alg));

    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>("ElectronSelection", target, polarity);
    app.add_algorithm(std::move(electron_alg));

    auto dis_alg = std::make_unique<Algorithms::DISSelectionAlg>("DISSelection", Physics::DISAnalysis::standard_dis_region());
    app.add_algorithm(std::move(dis_alg));

    auto lambda_alg = std::make_unique<Algorithms::LambdaReconstructionAlg>("LambdaReconstruction", mass_window);
    lambda_alg->set_max_delta_vz(max_delta_vz);
    lambda_alg->set_coplanarity_cut(coplanarity);
    lambda_alg->set_beam_energy(beam_energy);
    lambda_alg->set_min_displaced_vz(min_displaced_vz);
    lambda_alg->set_max_displaced_vz(max_displaced_vz);
    app.add_algorithm(std::move(lambda_alg));

    // Configure input
    app.set_input_files(input_files);
    app.set_percentage(percentage);
    app.set_num_threads(num_threads);

    // Run analysis
    auto sc = app.initialize();
    if (sc.is_failure()) {
        Framework::Log::error("Application", "Initialization failed");
        return EXIT_FAILURE;
    }

    Framework::Log::info("Configuration", "Lambda Analysis Configuration:");
    Framework::Log::info("Configuration", "  Target: {}", CLAS::to_string(target));
    Framework::Log::info("Configuration", "  Polarity: {}", CLAS::to_string(polarity));
    Framework::Log::info("Configuration", "  Percentage: {}", percentage);
    Framework::Log::info("Configuration", "  Threads: {}", num_threads);
    Framework::Log::info("Configuration", "  Output file: {}", output_file);
    Framework::Log::info("Configuration", "  Input files: {}", input_files.size());
    Framework::Log::info("Configuration", "  Mass window: {} GeV", mass_window);
    Framework::Log::info("Configuration", "  Max delta vz: {} cm", max_delta_vz);
    Framework::Log::info("Configuration", "  Coplanarity cut: {} deg", coplanarity);
    Framework::Log::info("Configuration", "  Beam energy: {} GeV", beam_energy);
    Framework::Log::info("Configuration", "  Min displaced vz: {} cm", min_displaced_vz);
    Framework::Log::info("Configuration", "  Max displaced vz: {} cm", max_displaced_vz);

    sc = app.run();
    if (sc.is_failure()) {
        Framework::Log::error("Application", "Event processing failed");
        return EXIT_FAILURE;
    }

    sc = app.finalize();
    if (sc.is_failure()) {
        Framework::Log::warning("Application", "Finalization had issues");
    }

    Framework::Log::info("Application", "Lambda analysis complete! Output: {}", output_file);

    return EXIT_SUCCESS;
}
