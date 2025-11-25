#pragma once

#include "Framework/AlgorithmTypes.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "TH1D.h"
#include <chrono>

namespace Algorithms {

/**
 * @brief Monitoring algorithm for event and detector statistics
 *
 * This is a MonitoringAlgorithm that tracks event processing rates,
 * particle multiplicities, and detector occupancies.
 *
 * Usage:
 * @code
 * auto monitor = std::make_unique<EventMonitor>("EventMonitor");
 * app.add_algorithm(std::move(monitor));
 * @endcode
 */
class EventMonitor : public Framework::MonitoringAlgorithm {
public:
    explicit EventMonitor(std::string name = "EventMonitor")
        : MonitoringAlgorithm(std::move(name)) {}

    auto initialize() -> Framework::StatusCode override {
        fmt::print("  {} - Initializing\n", name());

        // Get histogram service
        m_hist_svc = service<Framework::HistogramService>("HistogramService");

        if (m_hist_svc) {
            // Book monitoring histograms
            h_particles_per_event = m_hist_svc->book_hist1D(
                "/Monitor/particles_per_event",
                "Particles per Event;N_{particles};Events",
                50, 0, 50);

            h_electrons_per_event = m_hist_svc->book_hist1D(
                "/Monitor/electrons_per_event",
                "Electrons per Event;N_{e};Events",
                10, 0, 10);

            h_processing_time = m_hist_svc->book_hist1D(
                "/Monitor/processing_time",
                "Event Processing Time;Time [#mus];Events",
                100, 0, 1000);
        }

        m_start_time = std::chrono::high_resolution_clock::now();

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            end_time - m_start_time);

        fmt::print("\nEvent Monitor Statistics:\n");
        fmt::print("  Total events processed: {}\n", m_events_processed);
        fmt::print("  Total runtime: {} seconds\n", duration.count());
        fmt::print("  Average rate: {:.1f} events/second\n",
                   duration.count() > 0 ? double(m_events_processed) / duration.count() : 0.0);
        fmt::print("  Average particles/event: {:.2f}\n",
                   m_events_processed > 0 ? double(m_total_particles) / m_events_processed : 0.0);

        return Framework::StatusCode::SUCCESS();
    }

protected:
    /**
     * @brief Monitor event characteristics
     */
    auto monitor(Framework::Event& event) -> Framework::StatusCode override {
        auto event_start = std::chrono::high_resolution_clock::now();

        m_events_processed++;

        auto& hipo_event = event.hipo_event();
        hipo::bank_map REC_Particle = hipo_event.get_bank_map("REC::Particle");

        int n_particles = REC_Particle.get_rows();
        m_total_particles += n_particles;

        // Extract particles for detailed monitoring
        Core::ParticleMap particles = Core::ParticleSelector::extract_particles(
            REC_Particle,
            {Core::PIDs::ELECTRON, Core::PIDs::POSITRON,
             Core::PIDs::PIPLUS, Core::PIDs::PIMINUS,
             Core::PIDs::PHOTON});

        int n_electrons = particles[Core::PIDs::ELECTRON].size();

        // Fill monitoring histograms
        if (h_particles_per_event) {
            h_particles_per_event->Fill(n_particles);
        }

        if (h_electrons_per_event) {
            h_electrons_per_event->Fill(n_electrons);
        }

        // Measure processing time
        auto event_end = std::chrono::high_resolution_clock::now();
        auto event_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            event_end - event_start);

        if (h_processing_time) {
            h_processing_time->Fill(event_duration.count());
        }

        // Print periodic status updates
        if (m_events_processed % 10000 == 0) {
            fmt::print("  Processed {} events...\n", m_events_processed);
        }

        return Framework::StatusCode::SUCCESS();
    }

private:
    Framework::HistogramService* m_hist_svc = nullptr;

    // Monitoring histograms
    TH1D* h_particles_per_event = nullptr;
    TH1D* h_electrons_per_event = nullptr;
    TH1D* h_processing_time = nullptr;

    // Statistics
    size_t m_events_processed = 0;
    size_t m_total_particles = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};

}  // namespace Algorithms
