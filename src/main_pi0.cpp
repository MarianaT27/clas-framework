/**
 * @file main_pi0.cpp
 * @brief Pi0 reconstruction analysis executable
 *
 * This executable chains electron selection with π⁰ reconstruction
 * and writes results to an NTuple.
 *
 * Requirements: 3.1, 3.2, 4.4
 */

#include "Framework/Application.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Algorithms/ElectronSelectionAlg.hpp"
#include "Algorithms/Pi0ReconstructionAlg.hpp"
#include "Core/Helpers.hpp"
#include "Core/Target.hpp"
#include <fmt/format.h>
#include <string>
#include <cstdlib>

void print_usage(const char* program_name) {
    fmt::print("Usage: {} [options]\n", program_name);
    fmt::print("\nOptions:\n");
    fmt::print("  --data_dir <path>     Input data directory (default: data/)\n");
    fmt::print("  --max_events <n>      Maximum events to process (-1 for all, default: -1)\n");
    fmt::print("  --num_threads <n>     Number of threads (default: 1)\n");
    fmt::print("  --output <file>       Output ROOT file (default: pi0_analysis.root)\n");
    fmt::print("  --help                Show this help message\n");
}

int main(int argc, char* argv[]) {
    // Default configuration
    std::string data_dir = "data/";
    long max_events = -1;
    int num_threads = 1;
    std::string output_file = "pi0_analysis.root";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--data_dir" && i + 1 < argc) {
            data_dir = argv[++i];
        } else if (arg == "--max_events" && i + 1 < argc) {
            max_events = std::stol(argv[++i]);
        } else if (arg == "--num_threads" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            output_file = argv[++i];
        } else {
            fmt::print(stderr, "ERROR: Unknown argument: {}\n", arg);
            print_usage(argv[0]);
            return 1;
        }
    }

    // Create application
    Framework::Application app("Pi0 Analysis");

    // Configure services
    // Requirements: 4.4 - Add HistogramService with output file
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file(output_file);

    // Requirements: 4.4 - Add NTupleService with same output file
    auto ntuple_svc = std::make_unique<Framework::NTupleService>();
    ntuple_svc->set_output_file(output_file);

    // Add LoggingService (optional)
    auto log_svc = std::make_unique<Framework::LoggingService>();

    app.service_manager().add_service(std::move(hist_svc));
    app.service_manager().add_service(std::move(ntuple_svc));
    app.service_manager().add_service(std::move(log_svc));

    // Configure algorithm sequence
    // Requirements: 3.1, 3.2 - Add ElectronSelectionAlg as first algorithm
    auto electron_alg = std::make_unique<Algorithms::ElectronSelectionAlg>(
        "ElectronSelection",
        Core::Target::LD2,
        Core::Polarity::Outbending
    );
    app.add_algorithm(std::move(electron_alg));

    // Requirements: 3.1, 3.2 - Add Pi0ReconstructionAlg as second algorithm
    auto pi0_alg = std::make_unique<Algorithms::Pi0ReconstructionAlg>(
        "Pi0Reconstruction",
        0.1,   // min_photon_energy (GeV)
        0.03   // pi0_mass_window (GeV)
    );
    app.add_algorithm(std::move(pi0_alg));

    // Configure input files
    std::vector<std::string> input_files = Core::read_recursive_file_in_directory(data_dir);
    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No input files found in {}\n", data_dir);
        return 1;
    }
    app.set_input_files(input_files);
    app.set_max_events(max_events);
    app.set_num_threads(num_threads);

    // Print configuration
    fmt::print("Pi0 Analysis Configuration:\n");
    fmt::print("  Data directory: {}\n", data_dir);
    fmt::print("  Max events: {}\n", max_events);
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
