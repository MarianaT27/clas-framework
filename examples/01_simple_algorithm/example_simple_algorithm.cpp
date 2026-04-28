/**
 * @file example_simple_algorithm.cpp
 * @brief Example demonstrating Algorithm usage
 *
 * Shows the standard CLAS12 algorithm shape:
 *  - Inheriting from framework::Algorithm
 *  - Declaring properties + services + banks in the constructor
 *  - Owning mh::Histogram instances as value members and registering them
 *    with HistogramService for ROOT writeout at finalize
 */

#include <fmt/format.h>

#include <clas12/PIDs.hpp>
#include <clas12/FileUtils.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Application.hpp>
#include <clas12/framework/core/ApplicationBuilder.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/OutputFileService.hpp>

using namespace framework;

// ============================================================================
// Example 1: Algorithm with service injection and properties
// ============================================================================
class ElectronCounterAlg : public Algorithm {
public:
    ElectronCounterAlg() : Algorithm("ElectronCounter") {
        required_services(m_hist);
        declare_banks("REC::Particle");
        declare_properties(m_min_momentum, m_max_momentum, m_verbose);
    }

    auto on_initialize() -> StatusCode override {
        fmt::print("  [{}] Initializing with momentum range [{}, {}] GeV\n",
                   name(), m_min_momentum.get(), m_max_momentum.get());
        m_hist->register_hist("electron_momentum",
                              "Electron Momentum;p [GeV];Counts", h_electron_momentum);
        m_hist->register_hist("electron_count",
                              "Electrons per Event;N_{e};Events", h_electron_count);
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        m_events_processed++;

        int electron_count = 0;
        const double pmin = m_min_momentum.get();
        const double pmax = m_max_momentum.get();
        for (const auto& electron : event.electrons()) {
            const double pmag = electron.p();
            if (pmag < pmin || pmag > pmax) continue;
            electron_count++;
            h_electron_momentum.fill(pmag);
        }

        h_electron_count.fill(static_cast<double>(electron_count));
        m_total_electrons += electron_count;

        if (m_verbose.get() && electron_count > 0) {
            fmt::print("  [{}] Event {}: Found {} electrons\n",
                       name(), m_events_processed, electron_count);
        }

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        fmt::print("  [{}] Summary:\n", name());
        fmt::print("    Events processed: {}\n", m_events_processed);
        fmt::print("    Total electrons: {}\n", m_total_electrons);
        if (m_events_processed > 0) {
            fmt::print("    Average electrons/event: {:.2f}\n",
                       static_cast<double>(m_total_electrons) / m_events_processed);
        }
        return StatusCode::SUCCESS();
    }

private:
    Property<double> m_min_momentum{"min_momentum", 0.5, "Minimum electron momentum [GeV]"};
    Property<double> m_max_momentum{"max_momentum", 10.0, "Maximum electron momentum [GeV]"};
    Property<bool>   m_verbose{"verbose", false, "Print per-event information"};

    ServiceRef<HistogramService> m_hist;

    H1<> h_electron_momentum{mh::regular(0.0, 10.0, 100)};
    H1<> h_electron_count{mh::regular(0.0, 10.0, 10)};

    int m_events_processed = 0;
    int m_total_electrons = 0;
};

// ============================================================================
// Example 2: Another Algorithm demonstrating multiple services
// ============================================================================
class PhotonAnalysisAlg : public Algorithm {
public:
    PhotonAnalysisAlg() : Algorithm("PhotonAnalysis") {
        required_services(m_hist);
        declare_banks("REC::Particle");
        declare_properties(m_energy_threshold);
    }

    auto on_initialize() -> StatusCode override {
        fmt::print("  [{}] Initializing with energy threshold {} GeV\n",
                   name(), m_energy_threshold.get());
        m_hist->register_hist("photon_energy",
                              "Photon Energy;E [GeV];Counts", h_photon_energy);
        return StatusCode::SUCCESS();
    }

    auto process(Event& event) -> StatusCode override {
        auto photons = event.particles(PIDs::PHOTON);
        for (const auto& photon : photons) {
            if (photon.E() > m_energy_threshold.get()) {
                h_photon_energy.fill(photon.E());
                m_photon_count++;
            }
        }
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        fmt::print("  [{}] Total photons above threshold: {}\n",
                   name(), m_photon_count);
        return StatusCode::SUCCESS();
    }

private:
    Property<double> m_energy_threshold{"energy_threshold", 0.1, "Minimum photon energy [GeV]"};
    ServiceRef<HistogramService> m_hist;
    H1<> h_photon_energy{mh::regular(0.0, 5.0, 100)};
    int m_photon_count = 0;
};

auto main(int argc, char* argv[]) -> int {
    if (argc < 2) {
        fmt::print(stderr, "Usage: {} <input.hipo> [output.root]\n", argv[0]);
        return 1;
    }
    const std::string input  = argv[1];
    const std::string output = argc >= 3 ? argv[2] : "example_simple_algorithm.root";

    auto app = make_application("Simple Algorithm Example")
        .with_input_files({input})
        .with_output(output)
        .with_standard_services()
        .add_algorithm<ElectronCounterAlg>()
        .add_algorithm<PhotonAnalysisAlg>()
        .build();

    if (auto sc = app.initialize(); sc.is_failure()) return 1;
    if (auto sc = app.run();        sc.is_failure()) return 1;
    app.finalize();
    return 0;
}
