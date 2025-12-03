/**
 * @file example_auto_histograms.cpp
 * @brief Demonstration of automatic histogram writing to ROOT file
 *
 * This example shows how the HistogramService automatically writes
 * all booked histograms to a ROOT file when the application finalizes.
 * No manual intervention required!
 */

#include "Framework/Application.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Histogram.hpp"
#include "Core/Particle.hpp"
#include "Core/ParticleSelector.hpp"
#include "Core/Helpers.hpp"
#include "TMath.h"
#include <fmt/format.h>
#include <chrono>

/**
 * @brief Simple algorithm that books and fills histograms
 *
 * This algorithm demonstrates:
 * 1. Booking histograms using type-safe wrappers
 * 2. Using HistogramGroup for organization
 * 3. Using CutflowHistogram for automatic per-cut histograms
 * 4. All histograms automatically written to ROOT file on finalize
 */
class HistogramDemo : public Framework::Algorithm {
public:
    explicit HistogramDemo(std::string name = "HistogramDemo")
        : Algorithm(std::move(name)) {}

    auto initialize() -> Framework::StatusCode override {
        fmt::print("  {} - Initializing\n", name());

        // Get histogram service
        m_hist_svc = service<Framework::HistogramService>("HistogramService");
        if (!m_hist_svc) {
            fmt::print(stderr, "ERROR: Failed to get HistogramService\n");
            return Framework::StatusCode::FAILURE();
        }

        // Method 1: Book individual histograms with type-safe wrappers
        fmt::print("    Booking individual histograms...\n");
        TH1D* h_n_part = m_hist_svc->book_hist1D(
            "/Event/n_particles",
            "Particles per Event;N_{particles};Events",
            50, 0, 50
        );
        h_n_particles = Framework::Hist1D(h_n_part);

        // Method 2: Use HistogramGroup for organized collections
        fmt::print("    Creating histogram group for particles...\n");
        m_particle_hists = std::make_unique<Framework::HistogramGroup>(
            m_hist_svc, "/Particles"
        );
        m_particle_hists->add_hist1D("momentum", "Momentum;p [GeV/c];Particles", 100, 0, 10);
        m_particle_hists->add_hist1D("theta", "Theta;#theta [deg];Particles", 100, 0, 50);
        m_particle_hists->add_hist1D("phi", "Phi;#phi [deg];Particles", 100, -180, 180);
        m_particle_hists->add_hist2D("p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
                                      100, 0, 50, 100, 0, 10);

        // Method 3: Use CutflowHistogram for automatic per-cut histograms
        fmt::print("    Creating cutflow histogram for momentum...\n");
        m_p_cutflow = std::make_unique<Framework::CutflowHistogram>(
            m_hist_svc, "/Particles/p_cutflow",
            "Momentum Cutflow;p [GeV/c];Particles",
            100, 0, 10
        );
        m_p_cutflow->register_cut("All");
        m_p_cutflow->register_cut("p>1GeV");
        m_p_cutflow->register_cut("p>2GeV");
        m_p_cutflow->register_cut("Forward");

        // Method 4: Use TProfile for correlation studies
        fmt::print("    Booking TProfile...\n");
        p_theta_vs_p = m_hist_svc->book_profile(
            "/Particles/theta_vs_p",
            "Mean Theta vs Momentum;p [GeV/c];<#theta> [deg]",
            100, 0, 10
        );

        fmt::print("    Total histograms booked: {} 1D + {} 2D + {} profiles\n",
                   m_hist_svc->num_hist1D(),
                   m_hist_svc->num_hist2D(),
                   m_hist_svc->num_profiles());

        return Framework::StatusCode::SUCCESS();
    }

    auto execute(Framework::Event& event) -> Framework::StatusCode override {
        m_events_processed++;

        auto& hipo_event = event.hipo_event();
        hipo::bank_map REC_Particle = hipo_event.get_bank_map("REC::Particle");

        int n_particles = REC_Particle.get_rows();

        // Fill individual histogram
        h_n_particles.fill(n_particles);

        // Extract particles
        Core::ParticleMap particles = Core::ParticleSelector::extract_particles(
            REC_Particle,
            {Core::PIDs::ELECTRON, Core::PIDs::POSITRON,
             Core::PIDs::PIPLUS, Core::PIDs::PIMINUS,
             Core::PIDs::PHOTON}
        );

        // Fill histogram group
        for (const auto& [pid, particle_list] : particles) {
            for (const auto& p : particle_list) {
                double momentum = p.p();
                double theta = p.theta() * TMath::RadToDeg();
                double phi = p.phi() * TMath::RadToDeg();

                (*m_particle_hists)["momentum"].fill(momentum);
                (*m_particle_hists)["theta"].fill(theta);
                (*m_particle_hists)["phi"].fill(phi);
                m_particle_hists->hist2d("p_vs_theta").fill(theta, momentum);

                // Fill cutflow histogram
                m_p_cutflow->fill("All", momentum);
                if (momentum > 1.0) {
                    m_p_cutflow->fill("p>1GeV", momentum);
                    if (momentum > 2.0) {
                        m_p_cutflow->fill("p>2GeV", momentum);
                        if (theta < 35) {
                            m_p_cutflow->fill("Forward", momentum);
                        }
                    }
                }

                // Fill profile
                if (p_theta_vs_p) {
                    p_theta_vs_p->Fill(momentum, theta);
                }
            }
        }

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        fmt::print("\n{} Statistics:\n", name());
        fmt::print("  Events processed: {}\n", m_events_processed);
        fmt::print("  Histograms created: {} 1D, {} 2D, {} profiles\n",
                   m_hist_svc->num_hist1D(),
                   m_hist_svc->num_hist2D(),
                   m_hist_svc->num_profiles());
        fmt::print("  Output file: {}\n", m_hist_svc->get_output_file());
        fmt::print("  → All histograms will be automatically written!\n");

        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::HistogramService* m_hist_svc = nullptr;
    size_t m_events_processed = 0;

    // Individual histogram
    Framework::Hist1D h_n_particles;

    // Histogram group
    std::unique_ptr<Framework::HistogramGroup> m_particle_hists;

    // Cutflow histogram
    std::unique_ptr<Framework::CutflowHistogram> m_p_cutflow;

    // Profile
    TProfile* p_theta_vs_p = nullptr;
};

int main(int argc, char** argv) {
    std::string data_dir = (argc > 1) ? argv[1] : "data/";
    long max_events = (argc > 2) ? std::stol(argv[2]) : 1000;

    fmt::print("\n");
    fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
    fmt::print("║          Automatic Histogram Writing Demonstration            ║\n");
    fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
    fmt::print("\n");

    // Create application
    Framework::Application app("Histogram Demo");

    // Configure HistogramService
    fmt::print("Configuring Services:\n");
    auto hist_svc = std::make_unique<Framework::HistogramService>();
    hist_svc->set_output_file("demo_histograms.root");
    hist_svc->set_auto_write(true);  // Enable automatic writing (default)
    fmt::print("  ✓ HistogramService → auto-write enabled\n");
    fmt::print("  ✓ Output file: demo_histograms.root\n");
    app.service_manager().add_service(std::unique_ptr<Framework::IService>(std::move(hist_svc)));

    // Add algorithm
    fmt::print("\nConfiguring Algorithm:\n");
    auto demo = std::make_unique<HistogramDemo>("HistogramDemo");
    fmt::print("  ✓ HistogramDemo algorithm\n");
    app.add_algorithm(std::move(demo));

    // Configure input
    fmt::print("\nConfiguring Input:\n");
    std::vector<std::string> input_files = Core::read_recursive_file_in_directory(data_dir);
    if (input_files.empty()) {
        fmt::print(stderr, "ERROR: No .hipo files found in {}\n", data_dir);
        fmt::print("\nUsage: {} [data_dir] [max_events]\n", argv[0]);
        return 1;
    }
    fmt::print("  Data directory: {}\n", data_dir);
    fmt::print("  Input files: {}\n", input_files.size());
    fmt::print("  Max events: {}\n", max_events);

    app.set_input_files(input_files);
    // app.set_max_events(max_events);

    // Initialize
    fmt::print("\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Initialization\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("\n");

    auto sc = app.initialize();
    if (sc.is_failure()) {
        fmt::print(stderr, "\nERROR: Initialization failed\n");
        return 1;
    }

    // Run
    fmt::print("\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Processing Events\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("\n");

    auto start_time = std::chrono::high_resolution_clock::now();
    sc = app.run();
    auto end_time = std::chrono::high_resolution_clock::now();

    if (sc.is_failure()) {
        fmt::print(stderr, "\nERROR: Event processing failed\n");
        return 1;
    }

    // Finalize
    fmt::print("\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("  Finalization\n");
    fmt::print("═══════════════════════════════════════════════════════════════\n");
    fmt::print("\n");

    sc = app.finalize();
    if (sc.is_failure()) {
        fmt::print(stderr, "\nWARNING: Finalization had issues\n");
    }

    // Summary
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    fmt::print("\n");
    fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
    fmt::print("║                    Processing Complete!                       ║\n");
    fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
    fmt::print("\n");
    fmt::print("Performance:\n");
    fmt::print("  Processing time: {:.2f} seconds\n", duration.count() / 1000.0);
    fmt::print("\n");
    fmt::print("Output:\n");
    fmt::print("  ROOT file: demo_histograms.root\n");
    fmt::print("    /Event/           - Event-level histograms\n");
    fmt::print("    /Particles/       - Particle kinematics\n");
    fmt::print("    /Particles/p_cutflow_* - Automatic cutflow histograms\n");
    fmt::print("\n");
    fmt::print("Features Demonstrated:\n");
    fmt::print("  ✓ Type-safe histogram wrappers (Hist1D, Hist2D)\n");
    fmt::print("  ✓ HistogramGroup for organized collections\n");
    fmt::print("  ✓ CutflowHistogram for automatic per-cut histograms\n");
    fmt::print("  ✓ TProfile for correlation studies\n");
    fmt::print("  ✓ AUTOMATIC writing to ROOT file on finalize\n");
    fmt::print("\n");
    fmt::print("Verify:\n");
    fmt::print("  $ root -l demo_histograms.root\n");
    fmt::print("  root [0] .ls\n");
    fmt::print("  root [1] Event->ls()\n");
    fmt::print("  root [2] Particles->ls()\n");
    fmt::print("  root [3] Particles->cd()\n");
    fmt::print("  root [4] p_cutflow_All->Draw()\n");
    fmt::print("\n");

    return 0;
}
