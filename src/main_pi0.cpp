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
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include "Algorithms/EventMonitor.hpp"
#include "Core/Helpers.hpp"
#include "Core/Target.hpp"
#include "Framework/Application.hpp"
#include "Framework/Services/ConditionsService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "fmt/base.h"

int main(int argc, char* argv[]) {
    ROOT::EnableThreadSafety();
    ROOT::EnableImplicitMT();
    
    cxxopts::Options options("pi0_analysis", "Pi0 reconstruction analysis executable");

    options.add_options() 
            ("target", "Target type (LD2, CxC, CuSn, Cu, Sn)", cxxopts::value<std::string>()->default_value("LD2"))
            ("polarity", "Magnetic field polarity (In, Out)", cxxopts::value<std::string>()->default_value("Outbending"))
            ("percentage", "Percentage of events to process (0-100)", cxxopts::value<long>()->default_value("100"))
            ("num_threads", "Number of threads", cxxopts::value<int>()->default_value("1"))
            ("output", "Output ROOT file", cxxopts::value<std::string>()->default_value("pi0_analysis_{target}_{polarity}.root"))
            ("help", "Show help");

    // Parse command-line arguments
    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return EXIT_SUCCESS;
    }

    long percentage = args["percentage"].as<long>();
    int num_threads = args["num_threads"].as<int>();
    std::string output_file = args["output"].as<std::string>();
    std::string target_str = args["target"].as<std::string>();
    std::string polarity_str = args["polarity"].as<std::string>();
    Core::Target target = Core::string_to_target(target_str);
    Core::Polarity polarity = Core::string_to_polarity(polarity_str);

    output_file = Core::replace_all(output_file, "{target}", Core::to_string(target));
    output_file = Core::replace_all(output_file, "{polarity}", Core::to_string(polarity));

    std::array sources = (polarity == Core::Polarity::Outbending)
                             ? std::array{std::tuple{Core::Target::CuSn, "/cache/clas12/rg-d/production/pass1/recon/CuSn/dst/train/SIDIS/", &Core::outbending_runs_CuSn},
                                          std::tuple{Core::Target::CxC, "/cache/clas12/rg-d/production/pass1/recon/CxC/dst/train/SIDIS/", &Core::outbending_runs_CxC},
                                          std::tuple{Core::Target::LD2, "/cache/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/", &Core::outbending_runs_LD2}}
                             : std::array{std::tuple{Core::Target::CuSn, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &Core::inbending_runs_CuSn},
                                          std::tuple{Core::Target::CxC, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &Core::inbending_runs_CxC},
                                          std::tuple{Core::Target::LD2, "/cache/hallb/scratch/rg-d/production/skim_pass0v11/", &Core::inbending_runs_LD2}};
    
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
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    auto ntuple_svc = std::make_unique<Framework::NTupleService>();
    auto conditions_svc = std::make_unique<Framework::ConditionsService>();
    auto log_svc = std::make_unique<Framework::LoggingService>();

    // Configure output files for services
    hist_svc->set_output_file(output_file);
    ntuple_svc->set_output_file(output_file);

    app.service_manager().add_service(std::move(conditions_svc));
    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(ntuple_svc));
    app.service_manager().add_service(std::move(log_svc));

    auto monitoring_alg = std::make_unique<Algorithms::EventMonitor>("EventMonitor");
    app.add_algorithm(std::move(monitoring_alg));

    // Configure algorithm sequence
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>("ElectronSelection", target, polarity);
    app.add_algorithm(std::move(electron_alg));

    auto pi0_alg = std::make_unique<Algorithms::Pi0ReconstructionAlg>("Pi0Reconstruction", 0.2, 0.3);
    app.add_algorithm(std::move(pi0_alg));

    // Configure input files
    std::vector<std::string> input_files = Core::read_recursive_file_in_directory(data_dir);
    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No input files found in {}\n", data_dir);
        return 1;
    }
    app.set_input_files(input_files);
    app.set_percentage(percentage);
    app.set_num_threads(num_threads);

    // Print configuration
    fmt::print("Pi0 Analysis Configuration:\n");
    fmt::print("  Data directory: {}\n", data_dir);
    fmt::print("  Percentage: {}\n", percentage);
    fmt::print("  Threads: {}\n", num_threads);
    fmt::print("  Output file: {}\n", output_file);
    fmt::print("  Input files: {}\n", input_files.size());
    fmt::print("\n");

    // Run the analysis
    auto sc = app.initialize();
    if (sc.is_failure()) {
        fmt::print(stderr, "ERROR: Initialization failed\n");
        return 1;
    }

    sc = app.run();
    if (sc.is_failure()) {
        fmt::print(stderr, "ERROR: Event processing failed\n");
        return 1;
    }

    sc = app.finalize();
    if (sc.is_failure()) {
        fmt::print(stderr, "WARNING: Finalization had issues\n");
    }

    fmt::print("\n✓ Pi0 analysis complete!\n");
    fmt::print("  Output: {}\n\n", output_file);

    return 0;
}
