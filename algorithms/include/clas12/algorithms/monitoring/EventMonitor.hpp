#pragma once

#include <atomic>
#include <chrono>
#include <vector>
#include <clas12/PIDs.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

/// Configuration for EventMonitor.
struct EventMonitorConfig {
    size_t progress_interval = 10000;
    std::vector<clas12::PIDs> monitored_pids = {clas12::PIDs::ELECTRON};
};

/**
 * @brief Monitoring algorithm for event and detector statistics
 *
 * Tracks event processing rates, particle multiplicities, and timing.
 */
class EventMonitor : public clas12::framework::Algorithm {
   public:
    using Config = EventMonitorConfig;
    using StatusCode = clas12::framework::StatusCode;

    explicit EventMonitor(std::string name = "EventMonitor", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        required_services(m_hist);
        declare_banks("REC::Particle");
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing");
        book_histograms();
        m_start_time = std::chrono::high_resolution_clock::now();
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        print_statistics();
        return StatusCode::SUCCESS();
    }

    auto process(clas12::framework::Event& event) -> StatusCode override {
        const auto event_start = std::chrono::high_resolution_clock::now();
        m_events_processed.fetch_add(1, std::memory_order_relaxed);
        process_particle_statistics(event);
        record_processing_time(event_start);
        return StatusCode::SUCCESS();
    }

   private:
    static constexpr int PARTICLES_HIST_BINS = 50;
    static constexpr int PARTICLES_HIST_MIN = 0;
    static constexpr int PARTICLES_HIST_MAX = 50;

    static constexpr int ELECTRONS_HIST_BINS = 10;
    static constexpr int ELECTRONS_HIST_MIN = 0;
    static constexpr int ELECTRONS_HIST_MAX = 10;

    static constexpr int TIME_HIST_BINS = 100;
    static constexpr int TIME_HIST_MIN = 0;
    static constexpr int TIME_HIST_MAX = 1000;

    auto book_histograms() -> void {
        h_particles_per_event = m_hist->book_hist1D("/Monitor/particles_per_event", "Particles per Event;N_{particles};Events", PARTICLES_HIST_BINS, PARTICLES_HIST_MIN, PARTICLES_HIST_MAX);
        h_electrons_per_event = m_hist->book_hist1D("/Monitor/electrons_per_event", "Electrons per Event;N_{e};Events", ELECTRONS_HIST_BINS, ELECTRONS_HIST_MIN, ELECTRONS_HIST_MAX);
        h_processing_time = m_hist->book_hist1D("/Monitor/processing_time", "Event Processing Time;Time [#mus];Events", TIME_HIST_BINS, TIME_HIST_MIN, TIME_HIST_MAX);
    }

    auto process_particle_statistics(clas12::framework::Event& event) -> void {
        auto& rec_particle = event.bank<clas12::bankdefs::REC_Particle>();
        const int n_particles = rec_particle.rows();
        m_total_particles.fetch_add(static_cast<size_t>(n_particles), std::memory_order_relaxed);

        if (h_particles_per_event) h_particles_per_event->fill(n_particles);
        if (h_electrons_per_event) h_electrons_per_event->fill(static_cast<int>(event.electrons().size()));
    }

    auto record_processing_time(const std::chrono::time_point<std::chrono::high_resolution_clock>& event_start) -> void {
        const auto event_end = std::chrono::high_resolution_clock::now();
        const auto event_duration = std::chrono::duration_cast<std::chrono::microseconds>(event_end - event_start);
        if (h_processing_time) h_processing_time->fill(event_duration.count());
    }

    [[nodiscard]] auto calculate_event_rate(long duration_seconds) const -> double {
        return duration_seconds > 0 ? static_cast<double>(m_events_processed.load(std::memory_order_relaxed)) / duration_seconds : 0.0;
    }

    [[nodiscard]] auto calculate_avg_particles() const -> double {
        const auto events = m_events_processed.load(std::memory_order_relaxed);
        const auto particles = m_total_particles.load(std::memory_order_relaxed);
        return events > 0 ? static_cast<double>(particles) / static_cast<double>(events) : 0.0;
    }

    auto print_statistics() const -> void {
        const auto end_time = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - m_start_time);

        Log::info(name(), "Event Monitor Statistics:");
        Log::info(name(), "   - Total events processed: {}", m_events_processed.load(std::memory_order_relaxed));
        Log::info(name(), "   - Total runtime: {} seconds", duration.count());
        Log::info(name(), "   - Average rate: {:.1f} events/second", calculate_event_rate(duration.count()));
        Log::info(name(), "   - Average particles/event: {:.2f}", calculate_avg_particles());
    }

    Config m_config;

    clas12::framework::ServiceRef<clas12::framework::HistogramService> m_hist;

    clas12::framework::ThreadLocalHistogram1D* h_particles_per_event = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_electrons_per_event = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_processing_time = nullptr;

    std::atomic<size_t> m_events_processed{0};
    std::atomic<size_t> m_total_particles{0};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};

}  // namespace clas12::algorithms
