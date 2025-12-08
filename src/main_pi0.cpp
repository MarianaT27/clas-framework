/**
 * @file main_pi0.cpp
 * @brief Pi0 reconstruction analysis executable
 *
 * This executable chains electron selection with π⁰ reconstruction
 * and writes results to an NTuple.
 *
 * Requirements: 3.1, 3.2, 4.4
 */

#include <fmt/format.h>
#include <cstdlib>
#include <cxxopts.hpp>
#include <string>
#include <TROOT.h>
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/DISSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
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
#include "Framework/Services/ParticleSelectorService.hpp"
#include "Framework/Services/PhotonCutsService.hpp"
#include "Framework/Services/MetadataService.hpp"
#include "Framework/Services/ProfilingService.hpp"
#include "Framework/Services/SamplingFractionService.hpp"
#include "fmt/base.h"

int main(int argc, char* argv[]) {
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();
    
    cxxopts::Options options("pi0_analysis", "Pi0 reconstruction analysis executable");

    options.add_options() 
            ("target", "Target type (LD2, CxC, CuSn, Cu, Sn)", cxxopts::value<std::string>()->default_value("LD2"))
            ("polarity", "Magnetic field polarity (In, Out)", cxxopts::value<std::string>()->default_value("Outbending"))
            ("percentage", "Percentage of events to process (0-100)", cxxopts::value<double>()->default_value("100"))
            ("num_threads", "Number of threads", cxxopts::value<int>()->default_value("1"))
            ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("pi0_analysis_{target}_{polarity}.root"))
            ("help", "Show help");

    // Parse command-line arguments
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
    CLAS::Target target = CLAS::string_to_target(target_str);
    CLAS::Polarity polarity = CLAS::string_to_polarity(polarity_str);

    output_file = CLAS::StringUtils::replace_all(output_file, "{target}", CLAS::to_string(target));
    output_file = CLAS::StringUtils::replace_all(output_file, "{polarity}", CLAS::to_string(polarity));

    std::array sources = (polarity == CLAS::Polarity::Outbending)
                             ? std::array{std::tuple{CLAS::Target::CuSn, "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/", &CLAS::outbending_runs_CuSn},
                                          std::tuple{CLAS::Target::CxC, "/cache/clas12/rg-d/production/pass1/recon/CxC/dst/train/SIDIS/", &CLAS::outbending_runs_CxC},
                                          std::tuple{CLAS::Target::LD2, "/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/", &CLAS::outbending_runs_LD2}}
                             : std::array{std::tuple{CLAS::Target::CuSn, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &CLAS::inbending_runs_CuSn},
                                          std::tuple{CLAS::Target::CxC, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &CLAS::inbending_runs_CxC},
                                          std::tuple{CLAS::Target::LD2, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &CLAS::inbending_runs_LD2}};
    
    std::string data_dir;
    if (target_str != "" && polarity_str != "") {
        bool valid = false;
        for (const auto& [src_target, src_dir, _] : sources) {
            if (target == src_target) {
                data_dir = src_dir;
                valid = true;
                break;
            }
        }
        if (!valid) {
            fmt::println(stderr, "ERROR: Invalid target '{}' for polarity '{}'", target_str, polarity_str);
            fmt::println(stderr, "Valid targets are: LD2, CxC, CuSn, Cu, or Sn");
            return EXIT_FAILURE;
        }
    }

    // Create application
    Framework::Application app("Pi0 Analysis");

    // Create services
    auto ntuple_svc = std::make_unique<Framework::NTupleService>();
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    auto conditions_svc = std::make_unique<Framework::ConditionsService>();
    auto log_svc = std::make_unique<Framework::LoggingService>();
    auto particle_svc = std::make_unique<Framework::ParticleSelectorService>();
    auto photon_cuts_svc = std::make_unique<Framework::PhotonCutsService>();
    auto metadata_svc = std::make_unique<Framework::MetadataService>();
    auto profiling_svc = std::make_unique<Framework::ProfilingService>();
    auto sf_svc = std::make_unique<Framework::SamplingFractionService>();

    // Configure output files for services
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

    // Keep pointers before moving
    auto* log_ptr = log_svc.get();
    auto* hist_ptr = hist_svc.get();
    auto* ntuple_ptr = ntuple_svc.get();
    auto* conditions_ptr = conditions_svc.get();

    app.service_manager().add_service(std::move(conditions_svc));
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(log_svc));
    app.service_manager().add_service(std::move(ntuple_svc));
    app.service_manager().add_service(std::move(particle_svc));
    app.service_manager().add_service(std::move(photon_cuts_svc));
    app.service_manager().add_service(std::move(metadata_svc));
    app.service_manager().add_service(std::move(profiling_svc));
    app.service_manager().add_service(std::move(sf_svc));

    // Inject logger into all services, ServiceManager, and Application
    app.set_logger(log_ptr);
    app.service_manager().set_logger(log_ptr);
    ntuple_ptr->set_logger(log_ptr);
    hist_ptr->set_logger(log_ptr);
    conditions_ptr->set_logger(log_ptr);

    auto monitoring_alg = std::make_unique<Algorithms::EventMonitor>("EventMonitor");
    app.add_algorithm(std::move(monitoring_alg));

    // Configure algorithm sequence
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>("ElectronSelection", target, polarity);
    app.add_algorithm(std::move(electron_alg));

    auto dis_alg = std::make_unique<Algorithms::DISSelectionAlg>("DISSelection", Physics::DISAnalysis::standard_dis_region());
    app.add_algorithm(std::move(dis_alg));

    auto pi0_alg = std::make_unique<Algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", 0.2, 0.3);
    app.add_algorithm(std::move(pi0_alg));

    // Configure input files
    std::vector<std::string> input_files = CLAS::FileUtils::read_recursive_hipo_files(data_dir);
    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No input files found in {}\n", data_dir);
        return 1;
    }
    app.set_input_files(input_files);
    app.set_percentage(percentage);
    app.set_num_threads(num_threads);

    // Print configuration
    log_ptr->info("Configuration", "Pi0 Analysis Configuration:");
    log_ptr->info("Configuration", "  Data directory: {}", data_dir);
    log_ptr->info("Configuration", "  Percentage: {}", percentage);
    log_ptr->info("Configuration", "  Threads: {}", num_threads);
    log_ptr->info("Configuration", "  Output file: {}", output_file);
    log_ptr->info("Configuration", "  Input files: {}", input_files.size());

    // Run the analysis
    auto sc = app.initialize();
    if (sc.is_failure()) {
        log_ptr->error("Application", "Initialization failed");
        return 1;
    }

    sc = app.run();
    if (sc.is_failure()) {
        log_ptr->error("Application", "Event processing failed");
        return 1;
    }

    sc = app.finalize();
    if (sc.is_failure()) {
        log_ptr->warning("Application", "Finalization had issues");
    }

    log_ptr->info("Application", "Pi0 analysis complete! Output: {}", output_file);

    return 0;
}
