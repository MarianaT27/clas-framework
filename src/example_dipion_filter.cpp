/**
 * @file example_dipion_filter.cpp
 * @brief Example filtering HIPO events with DIS electron + pi+ pi-
 *
 * This example demonstrates:
 * - Reading HIPO files
 * - Filtering events with specific particle content and kinematics
 * - Writing filtered events to a new HIPO file
 *
 * Event selection criteria:
 * - Electron with Q2 > 0.7 GeV^2 and W > 1.7 GeV
 * - At least one pi+ with p > 0.3 GeV and |chi2pid| < 10
 * - At least one pi- with p > 0.3 GeV and |chi2pid| < 10
 *
 * Usage:
 *   ./example_dipion_filter <input_dir_or_file> [output.hipo]
 */

#include <fmt/format.h>
#include <cxxopts.hpp>

#include "Algorithms/DiPionFilterAlg.hpp"
#include "CLAS/FileUtils.hpp"
#include "Framework/Application.hpp"
#include "Framework/Services/HipoFileService.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/OutputFileService.hpp"

using namespace Framework;

int main(int argc, char* argv[]) {
    // ========================================================================
    // Parse command line arguments
    // ========================================================================
    cxxopts::Options options("example_dipion_filter",
        "Filter HIPO events with DIS electron + pi+ pi-");

    options.add_options()
        ("h,help", "Print usage")
        ("i,input", "Input HIPO file or directory", cxxopts::value<std::string>()->default_value("data/"))
        ("o,output", "Output HIPO file", cxxopts::value<std::string>()->default_value("dipion_filtered.hipo"))
        ("r,root", "Output ROOT file for histograms", cxxopts::value<std::string>()->default_value("dipion_analysis.root"))
        ("q2-min", "Minimum Q2 [GeV^2]", cxxopts::value<double>()->default_value("0.7"))
        ("w-min", "Minimum W [GeV]", cxxopts::value<double>()->default_value("1.7"))
        ("p-min", "Minimum pion momentum [GeV]", cxxopts::value<double>()->default_value("0.3"))
        ("chi2-max", "Maximum |chi2pid| for pions", cxxopts::value<double>()->default_value("10.0"))
        ("n,nevents", "Maximum events to process (0 = all)", cxxopts::value<int>()->default_value("0"))
        ("t,threads", "Number of threads", cxxopts::value<int>()->default_value("1"));

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return 0;
    }

    std::string input_path = args["input"].as<std::string>();
    std::string output_hipo = args["output"].as<std::string>();
    std::string output_root = args["root"].as<std::string>();
    double q2_min = args["q2-min"].as<double>();
    double w_min = args["w-min"].as<double>();
    double p_min = args["p-min"].as<double>();
    double chi2_max = args["chi2-max"].as<double>();
    int max_events = args["nevents"].as<int>();
    int num_threads = args["threads"].as<int>();

    fmt::print("=== Di-Pion Event Filter ===\n\n");
    fmt::print("Configuration:\n");
    fmt::print("  Input: {}\n", input_path);
    fmt::print("  Output HIPO: {}\n", output_hipo);
    fmt::print("  Output ROOT: {}\n", output_root);
    fmt::print("  DIS cuts: Q2 > {} GeV^2, W > {} GeV\n", q2_min, w_min);
    fmt::print("  Pion cuts: p > {} GeV, |chi2pid| < {}\n", p_min, chi2_max);
    if (max_events > 0) {
        fmt::print("  Max events: {}\n", max_events);
    }
    fmt::print("\n");

    // ========================================================================
    // Find input files
    // ========================================================================
    std::vector<std::string> input_files = CLAS::FileUtils::read_recursive_hipo_files(input_path);

    if (input_files.empty()) {
        fmt::print("ERROR: No HIPO files found in {}\n", input_path);
        return 1;
    }

    fmt::print("Found {} input file(s)\n", input_files.size());
    for (const auto& f : input_files) {
        fmt::print("  - {}\n", f);
    }
    fmt::print("\n");

    // ========================================================================
    // Create application and services
    // ========================================================================
    Application app("DiPionFilter");

    // Output ROOT file service (for histograms)
    auto output_svc = std::make_unique<OutputFileService>();
    output_svc->set_output_file(output_root);
    OutputFileService* output_svc_ptr = output_svc.get();
    app.service_manager().add_service(std::move(output_svc));

    // Histogram service
    auto hist_svc = std::make_unique<HistogramService>();
    hist_svc->set_output_file_service(output_svc_ptr);
    app.service_manager().add_service(std::move(hist_svc));

    // HIPO file service (for reading input and writing filtered output)
    auto hipo_svc = std::make_unique<HipoFileService>();
    hipo_svc->set_input_files(input_files);
    hipo_svc->set_output_file(output_hipo);
    app.service_manager().add_service(std::move(hipo_svc));

    // ========================================================================
    // Create and configure the filter algorithm
    // ========================================================================
    auto filter_alg = std::make_unique<Algorithms::DiPionFilterAlg>();

    // Set cut parameters via setter methods (fluent interface)
    filter_alg->set_q2_min(q2_min)
              .set_w_min(w_min)
              .set_pion_p_min(p_min)
              .set_pion_chi2pid_max(chi2_max);

    app.add_algorithm(std::move(filter_alg));

    // ========================================================================
    // Configure and run
    // ========================================================================
    app.set_input_files(input_files);
    app.set_num_threads(num_threads);

    // Note: max_events not directly supported; percentage-based processing available
    (void)max_events;

    // Initialize
    fmt::print("--- Initialization ---\n");
    auto sc = app.initialize();
    if (sc.is_failure()) {
        fmt::print("ERROR: Initialization failed: {}\n", sc.message());
        return 1;
    }

    // Run event loop
    fmt::print("\n--- Processing Events ---\n");
    sc = app.run();
    if (sc.is_failure()) {
        fmt::print("ERROR: Event processing failed\n");
        return 1;
    }

    // Finalize
    fmt::print("\n--- Finalization ---\n");
    sc = app.finalize();
    if (sc.is_failure()) {
        fmt::print("WARNING: Finalization had issues\n");
    }

    // ========================================================================
    // Print summary
    // ========================================================================
    fmt::print("\n=== Summary ===\n");
    fmt::print("  Output HIPO file: {}\n", output_hipo);
    fmt::print("  Output ROOT file: {}\n", output_root);
    fmt::print("\nDone!\n");

    return 0;
}
