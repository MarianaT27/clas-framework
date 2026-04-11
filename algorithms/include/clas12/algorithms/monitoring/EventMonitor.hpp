#pragma once

#include <atomic>
#include <chrono>
#include <vector>
#include <clas12/PIDs.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Pipeline.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/ParticleSelectorService.hpp>

namespace clas12::algorithms {

/**
 * @brief Monitoring algorithm for event and detector statistics
 *
 * This algorithm tracks event processing rates, particle multiplicities,
 * and detector occupancies.
 *
 * Usage:
 * @code
 * auto monitor = std::make_unique<EventMonitor>("EventMonitor");
 * monitor->set_progress_interval(5000);  // Update every 5000 events
 * app.add_algorithm(std::move(monitor));
 * @endcode
 */
class EventMonitor : public clas12::framework::SimpleAlgorithm {
   public:
    explicit EventMonitor(std::string name = "EventMonitor")
        : SimpleAlgorithm(std::move(name)), m_monitored_pids(initialize_default_pids()) {
        declare_bank("REC::Particle");
    }

    // Configuration methods
    void set_progress_interval(size_t interval) { m_progress_interval = interval; }

    void set_particle_types(std::vector<clas12::PIDs> pids) { m_monitored_pids = std::move(pids); }

    auto on_initialize() -> clas12::framework::StatusCode override {
        m_hist_svc = svc<clas12::framework::HistogramService>();
        m_particle_svc = svc<clas12::framework::ParticleSelectorService>();

        clas12::framework::Log::info("Event Monitor", "  {} - Initializing", name());

        if (m_hist_svc) {
            book_histograms();
        }

        m_start_time = std::chrono::high_resolution_clock::now();

        return clas12::framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> clas12::framework::StatusCode override {
        print_statistics();
        return clas12::framework::StatusCode::SUCCESS();
    }

    /**
     * @brief Monitor event characteristics.
     *
     * No filtering - this is a pure monitoring pipeline that always
     * passes through. The event start timestamp is captured before
     * the pipeline runs so the processing-time histogram sees the
     * full body duration.
     */
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override {
        const auto event_start = std::chrono::high_resolution_clock::now();
        return clas12::framework::pipeline(event)
            .tap([&] {
                m_events_processed.fetch_add(1, std::memory_order_relaxed);
                process_particle_statistics(event);
                record_processing_time(event_start);
            })
            .status();
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
        if (!m_hist_svc) return;

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
    void process_particle_statistics(clas12::framework::Event& event) {
        const auto& REC_Particle = event.get_bank("REC::Particle");

        const int n_particles = REC_Particle.rows();
        m_total_particles.fetch_add(static_cast<size_t>(n_particles), std::memory_order_relaxed);

        if (h_particles_per_event) {
            h_particles_per_event->fill(n_particles);
        }

        // Extract particles for detailed monitoring using ParticleSelectorService
        const clas12::framework::ParticleMap particles = m_particle_svc->extract_particles(
            REC_Particle, m_monitored_pids);

        const int n_electrons = m_particle_svc->get_electrons(particles).size();

        if (h_electrons_per_event) {
            h_electrons_per_event->fill(n_electrons);
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
            h_processing_time->fill(event_duration.count());
        }
    }

    /**
     * @brief Calculate and return average event rate
     */
    [[nodiscard]] double calculate_event_rate(long duration_seconds) const {
        return duration_seconds > 0
                   ? static_cast<double>(m_events_processed.load(std::memory_order_relaxed)) / duration_seconds
                   : 0.0;
    }

    /**
     * @brief Calculate and return average particles per event
     */
    [[nodiscard]] double calculate_avg_particles() const {
        const auto events = m_events_processed.load(std::memory_order_relaxed);
        const auto particles = m_total_particles.load(std::memory_order_relaxed);
        return events > 0 ? static_cast<double>(particles) / static_cast<double>(events) : 0.0;
    }

    /**
     * @brief Print final statistics summary
     */
    void print_statistics() const {
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - m_start_time);

        clas12::framework::Log::info("EventMonitor", "Event Monitor Statistics:");
        clas12::framework::Log::info("EventMonitor", "   - Total events processed: {}", m_events_processed.load(std::memory_order_relaxed));
        clas12::framework::Log::info("EventMonitor", "   - Total runtime: {} seconds", duration.count());
        clas12::framework::Log::info("EventMonitor", "   - Average rate: {:.1f} events/second", calculate_event_rate(duration.count()));
        clas12::framework::Log::info("EventMonitor", "   - Average particles/event: {:.2f}", calculate_avg_particles());
    }

    auto initialize_default_pids() const -> std::vector<clas12::PIDs> {
        return {clas12::PIDs::ELECTRON};
    }

    // Services
    clas12::framework::HistogramService* m_hist_svc = nullptr;
    clas12::framework::ParticleSelectorService* m_particle_svc = nullptr;

    // Monitoring histograms (atomic for thread-safety)
    clas12::framework::ThreadLocalHistogram1D* h_particles_per_event = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_electrons_per_event = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_processing_time = nullptr;

    // Statistics counters. Atomic because process() is invoked concurrently
    // from many worker threads on the same algorithm instance.
    std::atomic<size_t> m_events_processed{0};
    std::atomic<size_t> m_total_particles{0};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;

    // Configuration
    size_t m_progress_interval = DEFAULT_PROGRESS_INTERVAL;
    std::vector<clas12::PIDs> m_monitored_pids;
};

}  // namespace clas12::algorithms
