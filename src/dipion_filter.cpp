/**
 * @file dipion_filter.cpp
 * @brief Filter HIPO events with DIS electron + pi+ pi-
 *
 * Event selection criteria:
 * - Electron with Q2 > 0.7 GeV^2 and W > 1.7 GeV
 * - At least one pi+ with p > 0.3 GeV and |chi2pid| < 10
 * - At least one pi- with p > 0.3 GeV and |chi2pid| < 10
 *
 * Creates one output file per input file with prefix "dipion_filter_"
 */

#include <cstdlib>
#include <filesystem>

#include <TROOT.h>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include "Algorithms/DiPionFilterAlg.hpp"
#include "CLAS/FileUtils.hpp"
#include "Framework/ApplicationBuilder.hpp"

namespace fs = std::filesystem;

/**
 * @brief Generate output filename from input filename
 * @param input_file Full path to input file
 * @param output_dir Output directory (empty = current directory)
 * @return Output filename with prefix "dipion_filter_"
 */
auto make_output_filename(const std::string& input_file, const std::string& output_dir) -> std::string {
    fs::path input_path(input_file);
    std::string output_name = "dipion_filter_" + input_path.filename().string();

    if (output_dir.empty()) {
        return output_name;
    }
    return (fs::path(output_dir) / output_name).string();
}

auto main(int argc, char** argv) -> int {
    ROOT::EnableThreadSafety();

    // ========================================================================
    // Parse command line arguments
    // ========================================================================
    cxxopts::Options options("dipion_filter",
        "Filter HIPO events with DIS electron + pi+ pi-");

    options.add_options()
        ("h,help", "Print usage")
        ("i,input", "Input HIPO file or directory", cxxopts::value<std::string>()->default_value("data/"))
        ("O,output-dir", "Output directory for filtered HIPO files", cxxopts::value<std::string>()->default_value(""))
        ("r,root", "Output ROOT file for histograms", cxxopts::value<std::string>()->default_value("dipion_analysis.root"))
        ("q2-min", "Minimum Q2 [GeV^2]", cxxopts::value<double>()->default_value("0.7"))
        ("w-min", "Minimum W [GeV]", cxxopts::value<double>()->default_value("1.7"))
        ("p-min", "Minimum pion momentum [GeV]", cxxopts::value<double>()->default_value("0.3"))
        ("chi2-max", "Maximum |chi2pid| for pions", cxxopts::value<double>()->default_value("10.0"))
        ("t,threads", "Number of threads", cxxopts::value<int>()->default_value("4"));

    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        fmt::print("{}\n", options.help());
        return EXIT_SUCCESS;
    }

    std::string input_path = args["input"].as<std::string>();
    std::string output_dir = args["output-dir"].as<std::string>();
    std::string output_root = args["root"].as<std::string>();
    double q2_min = args["q2-min"].as<double>();
    double w_min = args["w-min"].as<double>();
    double p_min = args["p-min"].as<double>();
    double chi2_max = args["chi2-max"].as<double>();
    int num_threads = args["threads"].as<int>();

    // ========================================================================
    // Find input files
    // ========================================================================
    auto input_files = CLAS::FileUtils::read_recursive_hipo_files(input_path);
    if (input_files.empty()) {
        Framework::Log::error("Main", "No HIPO files found in {}", input_path);
        return EXIT_FAILURE;
    }

    // Create output directory if specified and doesn't exist
    if (!output_dir.empty()) {
        fs::create_directories(output_dir);
    }

    fmt::print("=== Di-Pion Event Filter ===\n\n");
    fmt::print("Configuration:\n");
    fmt::print("  Input: {} file(s) from {}\n", input_files.size(), input_path);
    fmt::print("  Output dir: {}\n", output_dir.empty() ? "." : output_dir);
    fmt::print("  Output ROOT: {}\n", output_root);
    fmt::print("  DIS cuts: Q2 > {} GeV^2, W > {} GeV\n", q2_min, w_min);
    fmt::print("  Pion cuts: p > {} GeV, |chi2pid| < {}\n", p_min, chi2_max);
    fmt::print("  Threads: {}\n\n", num_threads);

    // ========================================================================
    // Process each input file separately (1-to-1 output)
    // ========================================================================
    std::vector<std::string> output_files;
    size_t total_files = input_files.size();

    for (size_t i = 0; i < total_files; ++i) {
        const auto& input_file = input_files[i];
        std::string output_hipo = make_output_filename(input_file, output_dir);
        output_files.push_back(output_hipo);

        fmt::print("[{}/{}] Processing: {}\n", i + 1, total_files, fs::path(input_file).filename().string());
        fmt::print("       Output: {}\n", output_hipo);

        // Create algorithm with cuts configured
        auto dipion_alg = std::make_unique<Algorithms::DiPionFilterAlg>();
        dipion_alg->set_q2_min(q2_min)
                  .set_w_min(w_min)
                  .set_pion_p_min(p_min)
                  .set_pion_chi2pid_max(chi2_max);

        // Build application for this single file
        auto app = Framework::make_application("DiPion Filter")
            .with_input_file(input_file)
            .with_output(output_root)
            .with_threads(num_threads)
            .with_standard_services()
            .add_algorithm(std::move(dipion_alg))
            .build();

        // Configure HIPO output file
        if (auto* hipo_svc = app.service_manager().get_service<Framework::HipoFileService>("HipoFileService")) {
            hipo_svc->set_output_file(output_hipo);
        }

        // Run analysis
        if (auto sc = app.initialize(); sc.is_failure()) {
            Framework::Log::error("Main", "Initialization failed: {}", sc.message());
            continue;
        }

        if (auto sc = app.run(); sc.is_failure()) {
            Framework::Log::error("Main", "Processing failed: {}", sc.message());
        }

        (void)app.finalize();
        fmt::print("\n");
    }

    // ========================================================================
    // Summary
    // ========================================================================
    fmt::print("=== Analysis Complete ===\n");
    fmt::print("Processed {} files:\n", total_files);
    for (const auto& out : output_files) {
        fmt::print("  - {}\n", out);
    }
    fmt::print("ROOT histograms: {}\n", output_root);

    return EXIT_SUCCESS;
}
