#pragma once

#include <atomic>
#include <chrono>
#include <vector>
#include <clas12/PIDs.hpp>
#include <clas12/bankdefs/REC_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace algorithms {

using framework::Log;

/// Configuration for EventMonitor.
struct EventMonitorConfig {
    size_t progress_interval = 10000;
    std::vector<PIDs> monitored_pids = {PIDs::ELECTRON};
};

/**
 * @brief Monitoring algorithm for event and detector statistics
 *
 * Tracks event processing rates, particle multiplicities, and timing.
 */
class EventMonitor : public framework::Algorithm {
   public:
    using Config = EventMonitorConfig;
    using StatusCode = framework::StatusCode;

    explicit EventMonitor(std::string name = "EventMonitor", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        required_services(m_hist);
        declare_banks("REC::Particle");
        declare_histograms(h_particles_per_event, h_electrons_per_event, h_processing_time);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing");
        m_start_time = std::chrono::high_resolution_clock::now();
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        print_statistics();
        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        const auto event_start = std::chrono::high_resolution_clock::now();
        m_events_processed.fetch_add(1, std::memory_order_relaxed);
        process_particle_statistics(event);
        record_processing_time(event_start);
        return StatusCode::SUCCESS();
    }

   private:
    auto process_particle_statistics(framework::Event& event) -> void {
        auto& rec_particle = event.bank<bankdefs::REC_Particle>();
        const int n_particles = rec_particle.rows();
        m_total_particles.fetch_add(static_cast<size_t>(n_particles), std::memory_order_relaxed);

        h_particles_per_event.fill(static_cast<double>(n_particles));
        h_electrons_per_event.fill(static_cast<double>(event.electrons().size()));
    }

    auto record_processing_time(const std::chrono::time_point<std::chrono::high_resolution_clock>& event_start) -> void {
        const auto event_end = std::chrono::high_resolution_clock::now();
        const auto event_duration = std::chrono::duration_cast<std::chrono::microseconds>(event_end - event_start);
        h_processing_time.fill(static_cast<double>(event_duration.count()));
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

    framework::ServiceRef<framework::HistogramService> m_hist;

    framework::Hist1<> h_particles_per_event{"/Monitor/particles_per_event",
        "Particles per Event;N_{particles};Events", 0.0, 50.0, 50};
    framework::Hist1<> h_electrons_per_event{"/Monitor/electrons_per_event",
        "Electrons per Event;N_{e};Events", 0.0, 10.0, 10};
    framework::Hist1<> h_processing_time{"/Monitor/processing_time",
        "Event Processing Time;Time [#mus];Events", 0.0, 1000.0, 100};

    std::atomic<size_t> m_events_processed{0};
    std::atomic<size_t> m_total_particles{0};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time;
};

}  // namespace algorithms
