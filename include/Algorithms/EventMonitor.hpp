#pragma once

#include "Core/PIDs.hpp"
#include "Framework/AlgorithmTypes.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Core/ParticleSelector.hpp"
#include <chrono>
#include <vector>

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
 * monitor->set_progress_interval(5000);  // Update every 5000 events
 * app.add_algorithm(std::move(monitor));
 * @endcode
 */
class EventMonitor : public Framework::MonitoringAlgorithm {
public:
    explicit EventMonitor(std::string name = "EventMonitor")
        : MonitoringAlgorithm(std::move(name))
        , m_monitored_pids(initialize_default_pids()) {}

    // Configuration methods
    void set_progress_interval(size_t interval) { m_progress_interval = interval; }
    void set_particle_types(std::vector<Core::PIDs> pids) { m_monitored_pids = std::move(pids); }
    auto initialize() -> Framework::StatusCode override {
        fmt::print("  {} - Initializing\n", name());

        // Initialize service handle (optional - histogram service may not be available)
        if (m_svc_mgr) {
            m_hist_svc.initialize(*m_svc_mgr);
        }

        if (m_hist_svc) {
            book_histograms();
        }

        m_start_time = std::chrono::high_resolution_clock::now();

        return Framework::StatusCode::SUCCESS();
    }

    auto finalize() -> Framework::StatusCode override {
        print_statistics();
        return Framework::StatusCode::SUCCESS();
    }

protected:
    /**
     * @brief Monitor event characteristics
     */
    auto monitor(Framework::Event& event) -> Framework::StatusCode override {
        const auto event_start = std::chrono::high_resolution_clock::now();
        m_events_processed++;
        process_particle_statistics(event);
        record_processing_time(event_start);

        return Framework::StatusCode::SUCCESS();
    }

private:
    // Configuration constants
    static constexpr int PARTICLES_HIST_BINS = 50;
    static constexpr int PARTICLES_HIST_MIN = 0;
    static constexpr int PARTICLES_HIST_MAX = 50;

    static constexpr int ELECTRONS_HIST_BINS = 10;
    static constexpr int ELECTRONS_HIST_MIN = 0;
    static constexpr int ELECTRONS_HIST_MAX = 10;

    static constexpr int TIME_HIST_BINS = 100;
    static constexpr int TIME_HIST_MIN = 0;
    static constexpr int TIME_HIST_MAX = 1000;

    static constexpr size_t DEFAULT_PROGRESS_INTERVAL = 10000;

    /**
     * @brief Book all monitoring histograms
     */
    void book_histograms() {
        if (!m_hist_svc.is_valid()) return;

        h_particles_per_event = m_hist_svc->book_hist1D(
            "/Monitor/particles_per_event",
            "Particles per Event;N_{particles};Events",
            PARTICLES_HIST_BINS, PARTICLES_HIST_MIN, PARTICLES_HIST_MAX);

        h_electrons_per_event = m_hist_svc->book_hist1D(
            "/Monitor/electrons_per_event",
            "Electrons per Event;N_{e};Events",
            ELECTRONS_HIST_BINS, ELECTRONS_HIST_MIN, ELECTRONS_HIST_MAX);

        h_processing_time = m_hist_svc->book_hist1D(
            "/Monitor/processing_time",
            "Event Processing Time;Time [#mus];Events",
            TIME_HIST_BINS, TIME_HIST_MIN, TIME_HIST_MAX);
    }

    /**
     * @brief Process and record particle statistics for the event
     */
    void process_particle_statistics(Framework::Event& event) {
        auto& hipo_event = event.hipo_event();
        hipo::bank_map REC_Particle = hipo_event.get_bank_map("REC::Particle");

        const int n_particles = REC_Particle.get_rows();
        m_total_particles += n_particles;

        if (h_particles_per_event) {
            h_particles_per_event->Fill(n_particles);
        }

        // Extract particles for detailed monitoring
        const Core::ParticleMap particles = Core::ParticleSelector::extract_particles(
            REC_Particle, m_monitored_pids);

        const int n_electrons = particles.at(Core::PIDs::ELECTRON).size();

        if (h_electrons_per_event) {
            h_electrons_per_event->Fill(n_electrons);
        }
    }

    /**
     * @brief Record the event processing time
     */
    void record_processing_time(
        const std::chrono::time_point<std::chrono::high_resolution_clock>& event_start) {

        const auto event_end = std::chrono::high_resolution_clock::now();
        const auto event_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            event_end - event_start);

        if (h_processing_time) {
            h_processing_time->Fill(event_duration.count());
        }
    }

    /**
     * @brief Calculate and return average event rate
     */
    [[nodiscard]] double calculate_event_rate(long duration_seconds) const {
        return duration_seconds > 0 ? static_cast<double>(m_events_processed) / duration_seconds : 0.0;
    }

    /**
     * @brief Calculate and return average particles per event
     */
    [[nodiscard]] double calculate_avg_particles() const {
        return m_events_processed > 0
            ? static_cast<double>(m_total_particles) / m_events_processed
            : 0.0;
    }

    /**
     * @brief Print final statistics summary
     */
    void print_statistics() const {
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - m_start_time);

        fmt::print("\nEvent Monitor Statistics:\n");
        fmt::print("  Total events processed: {}\n", m_events_processed);
        fmt::print("  Total runtime: {} seconds\n", duration.count());
        fmt::print("  Average rate: {:.1f} events/second\n", calculate_event_rate(duration.count()));
        fmt::print("  Average particles/event: {:.2f}\n", calculate_avg_particles());
    }

    auto initialize_default_pids() const -> std::vector<Core::PIDs> {
        return {Core::PIDs::ELECTRON};
    }

    // Services
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};

    // Monitoring histograms
    TH1D* h_particles_per_event = nullptr;
    TH1D* h_electrons_per_event = nullptr;
    TH1D* h_processing_time = nullptr;

    // Statistics counters
    size_t m_events_processed = 0;
    size_t m_total_particles = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;

    // Configuration
    size_t m_progress_interval = DEFAULT_PROGRESS_INTERVAL;
    std::vector<Core::PIDs> m_monitored_pids;
};

}  // namespace Algorithms
