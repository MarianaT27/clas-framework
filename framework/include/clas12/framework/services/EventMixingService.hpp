#pragma once

#include <algorithm>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

/**
 * @brief Binning specification for event mixing
 */
struct MixingBin {
    double z_vertex_min = -100.0;
    double z_vertex_max = 100.0;
    double Q2_min = 0.0;
    double Q2_max = 100.0;
    int sector = -1;  ///< -1 means all sectors

    /// Check if values fall within this bin
    [[nodiscard]] auto contains(double vz, double Q2, int sec = -1) const -> bool {
        if (vz < z_vertex_min || vz >= z_vertex_max) return false;
        if (Q2 < Q2_min || Q2 >= Q2_max) return false;
        if (sector >= 0 && sec >= 0 && sector != sec) return false;
        return true;
    }

    /// Generate a unique key for this bin
    [[nodiscard]] auto key() const -> std::string {
        return std::to_string(static_cast<int>(z_vertex_min)) + "_" +
               std::to_string(static_cast<int>(z_vertex_max)) + "_" +
               std::to_string(static_cast<int>(Q2_min * 10)) + "_" +
               std::to_string(static_cast<int>(Q2_max * 10)) + "_" +
               std::to_string(sector);
    }

    /// Comparison for map ordering
    auto operator<(const MixingBin& other) const -> bool {
        return key() < other.key();
    }
};

/**
 * @brief Pool of particles from previous events for mixing
 */
template <typename ParticleType = clas12::Particle>
class EventMixingPool {
   public:
    explicit EventMixingPool(std::size_t max_events = 10)
        : m_max_events(max_events) {}

    /**
     * @brief Add particles from current event to pool
     *
     * @param bin Mixing bin (determined from event kinematics)
     * @param particles Particles to store
     */
    auto add(const MixingBin& bin, const std::vector<ParticleType>& particles) -> void {
        auto& pool = m_pools[bin.key()];

        // Add new event's particles
        pool.push_back(particles);

        // Remove oldest if over limit
        while (pool.size() > m_max_events) {
            pool.pop_front();
        }
    }

    /**
     * @brief Get particles from pool for mixing
     *
     * @param bin Mixing bin to get particles from
     * @return Vector of particle vectors from different events
     */
    [[nodiscard]] auto get(const MixingBin& bin) const
        -> const std::deque<std::vector<ParticleType>>& {
        static const std::deque<std::vector<ParticleType>> empty;
        auto it = m_pools.find(bin.key());
        if (it != m_pools.end()) {
            return it->second;
        }
        return empty;
    }

    /**
     * @brief Get random event from pool
     */
    [[nodiscard]] auto get_random(const MixingBin& bin, std::mt19937& rng) const
        -> std::optional<std::vector<ParticleType>> {
        const auto& pool = get(bin);
        if (pool.empty()) return std::nullopt;

        std::uniform_int_distribution<size_t> dist(0, pool.size() - 1);
        return pool[dist(rng)];
    }

    /**
     * @brief Get number of events in a bin
     */
    [[nodiscard]] auto count(const MixingBin& bin) const -> std::size_t {
        auto it = m_pools.find(bin.key());
        if (it != m_pools.end()) {
            return it->second.size();
        }
        return 0;
    }

    /**
     * @brief Clear all stored particles
     */
    auto clear() -> void {
        m_pools.clear();
    }

    /**
     * @brief Set maximum events per bin
     */
    auto set_max_events(std::size_t n) -> void {
        m_max_events = n;
    }

   private:
    std::size_t m_max_events;
    std::map<std::string, std::deque<std::vector<ParticleType>>> m_pools;
};

/**
 * @brief Service for event mixing background estimation
 *
 * EventMixingService provides functionality for combinatorial background
 * estimation by mixing particles from different events. This is a standard
 * technique in HEP for understanding background shapes in invariant mass
 * distributions.
 *
 * The service maintains pools of particles binned by kinematic variables
 * (vertex z, Q2, sector) to ensure mixed events come from similar
 * kinematic regions.
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_mixing_svc = &require_service<EventMixingService>("EventMixingService");
 * m_mixing_svc->add_vz_bins({-40, -20, 0, 20, 40});
 * m_mixing_svc->add_Q2_bins({1, 2, 3, 5, 10});
 *
 * // In algorithm process():
 * // Get current event particles
 * auto pions_plus = event.particles(PIDs::PIPLUS);
 * auto pions_minus = event.particles(PIDs::PIMINUS);
 *
 * // Fill same-event histogram
 * for (const auto& pip : pions_plus) {
 *     for (const auto& pim : pions_minus) {
 *         h_mass_same->Fill(invariant_mass(pip, pim));
 *     }
 * }
 *
 * // Add to mixing pool
 * m_mixing_svc->add_to_pool("piplus", event, pions_plus);
 * m_mixing_svc->add_to_pool("piminus", event, pions_minus);
 *
 * // Get mixed-event pairs
 * auto mixed_pairs = m_mixing_svc->get_mixed_pairs<Particle, Particle>(
 *     "piplus", "piminus", event);
 *
 * double weight = m_mixing_svc->mixed_event_weight();
 * for (const auto& [pip, pim] : mixed_pairs) {
 *     h_mass_mixed->Fill(invariant_mass(*pip, *pim), weight);
 * }
 * @endcode
 */
class EventMixingService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "EventMixingService"; }

    explicit EventMixingService(std::string name = "EventMixingService")
        : Service(std::move(name))
        , m_rng(std::random_device{}()) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        // Initialize default bins if none configured
        if (m_vz_bin_edges.empty()) {
            m_vz_bin_edges = {-40, -20, 0, 20, 40};
        }
        if (m_Q2_bin_edges.empty()) {
            m_Q2_bin_edges = {0, 1, 2, 3, 5, 10, 100};
        }

        build_bins();

        Log::info(this->name(), "EventMixingService initialized");
        Log::info(this->name(), "  Vz bins: {}, Q2 bins: {}, Sector bins: {}",
                  m_vz_bin_edges.size() - 1, m_Q2_bin_edges.size() - 1,
                  m_separate_sectors.get() ? 6 : 1);
        Log::info(this->name(), "  Events per bin: {}", m_events_per_bin.get());

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "EventMixingService finalized. "
                                "Events mixed: {}, Pairs generated: {}",
                  m_events_mixed, m_pairs_generated);
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Number of events to store per bin
    Property<int> m_events_per_bin{"events_per_bin", 10, "Events to pool per bin"};

    /// Whether to separate by sector
    Property<bool> m_separate_sectors{"separate_sectors", true,
                                      "Separate mixing by detector sector"};

    /// Random seed (0 = random)
    Property<int> m_seed{"seed", 0, "Random seed (0 = random)"};

    // ========================================================================
    // Binning Configuration
    // ========================================================================

    /**
     * @brief Set vertex z bin edges
     *
     * @param edges Bin edges (e.g., {-40, -20, 0, 20, 40})
     */
    auto add_vz_bins(const std::vector<double>& edges) -> void {
        m_vz_bin_edges = edges;
        std::sort(m_vz_bin_edges.begin(), m_vz_bin_edges.end());
    }

    /**
     * @brief Set Q2 bin edges
     *
     * @param edges Bin edges (e.g., {0, 1, 2, 3, 5, 10})
     */
    auto add_Q2_bins(const std::vector<double>& edges) -> void {
        m_Q2_bin_edges = edges;
        std::sort(m_Q2_bin_edges.begin(), m_Q2_bin_edges.end());
    }

    /**
     * @brief Set whether to separate by sector
     */
    auto add_sector_bins(bool separate = true) -> void {
        m_separate_sectors.set(separate);
    }

    // ========================================================================
    // Pool Management
    // ========================================================================

    /**
     * @brief Add particles to mixing pool
     *
     * @tparam T Particle type
     * @param pool_name Name of the pool (e.g., "piplus", "piminus")
     * @param event Current event (for determining bin)
     * @param particles Particles to add
     * @param vz Vertex z (if not provided, uses first particle)
     * @param Q2 Q2 value (if not provided, must be stored in event)
     * @param sector Sector (if not provided, uses first particle)
     */
    template <typename T = clas12::Particle>
    auto add_to_pool(const std::string& pool_name,
                     Event& event,
                     const std::vector<T>& particles,
                     double vz = std::numeric_limits<double>::quiet_NaN(),
                     double Q2 = std::numeric_limits<double>::quiet_NaN(),
                     int sector = -1) -> void {

        if (particles.empty()) return;

        // Determine bin values
        if (std::isnan(vz)) {
            vz = particles[0].vz();
        }

        if (std::isnan(Q2)) {
            // Try to get Q2 from event store
            auto q2_opt = event.event_store().retrieve(Keys::Q2);
            Q2 = q2_opt.value_or(0.0);
        }

        // Find the matching bin
        auto bin = find_bin(vz, Q2, sector);

        // Get or create pool
        auto& pool = get_or_create_pool<T>(pool_name);
        pool.add(bin, particles);

        m_events_mixed++;
    }

    /**
     * @brief Get particles from pool matching current event's kinematics
     *
     * @tparam T Particle type
     * @param pool_name Pool name
     * @param event Current event
     * @return Vector of pointers to mixed particles
     */
    template <typename T = clas12::Particle>
    auto get_mixed_particles(const std::string& pool_name,
                             Event& event,
                             double vz = std::numeric_limits<double>::quiet_NaN(),
                             double Q2 = std::numeric_limits<double>::quiet_NaN(),
                             int sector = -1) -> std::vector<const T*> {

        std::vector<const T*> result;

        // Determine bin
        if (std::isnan(vz)) {
            // Use electron vertex if available
            auto electrons = event.particles(clas12::PIDs::ELECTRON);
            if (!electrons.empty()) {
                vz = electrons[0].vz();
            } else {
                vz = 0.0;
            }
        }

        if (std::isnan(Q2)) {
            auto q2_opt = event.event_store().retrieve(Keys::Q2);
            Q2 = q2_opt.value_or(0.0);
        }

        auto bin = find_bin(vz, Q2, sector);

        // Get pool
        auto* pool = get_pool<T>(pool_name);
        if (!pool) return result;

        // Get random event from pool
        auto mixed_event = pool->get_random(bin, m_rng);
        if (mixed_event) {
            for (const auto& p : *mixed_event) {
                result.push_back(&p);
            }
        }

        return result;
    }

    /**
     * @brief Get mixed-event pairs from two pools
     *
     * @tparam T1 First particle type
     * @tparam T2 Second particle type
     * @param pool1 First pool name
     * @param pool2 Second pool name
     * @param event Current event
     * @return Vector of particle pairs
     */
    template <typename T1 = clas12::Particle, typename T2 = clas12::Particle>
    auto get_mixed_pairs(const std::string& pool1,
                         const std::string& pool2,
                         Event& event,
                         double vz = std::numeric_limits<double>::quiet_NaN(),
                         double Q2 = std::numeric_limits<double>::quiet_NaN(),
                         int sector = -1) -> std::vector<std::pair<T1, T2>> {

        std::vector<std::pair<T1, T2>> result;

        // Determine bin
        if (std::isnan(vz)) {
            auto electrons = event.particles(clas12::PIDs::ELECTRON);
            if (!electrons.empty()) {
                vz = electrons[0].vz();
            } else {
                vz = 0.0;
            }
        }

        if (std::isnan(Q2)) {
            auto q2_opt = event.event_store().retrieve(Keys::Q2);
            Q2 = q2_opt.value_or(0.0);
        }

        auto bin = find_bin(vz, Q2, sector);

        // Get pools
        auto* p1 = get_pool<T1>(pool1);
        auto* p2 = get_pool<T2>(pool2);

        if (!p1 || !p2) return result;

        // Get particles from different events in each pool
        const auto& events1 = p1->get(bin);
        const auto& events2 = p2->get(bin);

        if (events1.size() < 2 || events2.size() < 2) return result;

        // Mix: take particles from event i in pool1 with particles from event j in pool2 (i != j)
        for (size_t i = 0; i < events1.size(); ++i) {
            for (size_t j = 0; j < events2.size(); ++j) {
                if (i == j) continue;  // Skip same event

                for (const auto& part1 : events1[i]) {
                    for (const auto& part2 : events2[j]) {
                        result.emplace_back(part1, part2);
                        m_pairs_generated++;
                    }
                }
            }
        }

        return result;
    }

    // ========================================================================
    // Normalization
    // ========================================================================

    /**
     * @brief Get weight for mixed-event normalization
     *
     * The normalization depends on the mixing scheme. For N events in pool,
     * there are N*(N-1)/2 mixed pairs vs N same-event pairs.
     *
     * @return Normalization weight
     */
    [[nodiscard]] auto mixed_event_weight() const -> double {
        int n = m_events_per_bin.get();
        if (n <= 1) return 1.0;

        // Normalization: same-event has N pairs per N events
        // Mixed-event has N*(N-1)/2 pairs
        // Weight = 2 / (N-1) to normalize
        return 2.0 / (n - 1);
    }

    /**
     * @brief Get number of events stored per bin
     */
    [[nodiscard]] auto events_per_bin() const -> int {
        return m_events_per_bin.get();
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    [[nodiscard]] auto events_mixed() const -> size_t { return m_events_mixed; }
    [[nodiscard]] auto pairs_generated() const -> size_t { return m_pairs_generated; }

    /**
     * @brief Clear all pools
     */
    auto clear_pools() -> void {
        m_particle_pools.clear();
    }

   private:
    // Bin configuration
    std::vector<double> m_vz_bin_edges;
    std::vector<double> m_Q2_bin_edges;
    std::vector<MixingBin> m_bins;

    // Particle pools (type-erased)
    std::map<std::string, std::shared_ptr<void>> m_particle_pools;

    // Random number generator
    mutable std::mt19937 m_rng;

    // Statistics
    mutable size_t m_events_mixed = 0;
    mutable size_t m_pairs_generated = 0;

    /**
     * @brief Build mixing bins from edge vectors
     */
    auto build_bins() -> void {
        m_bins.clear();

        int n_vz = static_cast<int>(m_vz_bin_edges.size()) - 1;
        int n_Q2 = static_cast<int>(m_Q2_bin_edges.size()) - 1;
        int n_sector = m_separate_sectors.get() ? 6 : 1;

        for (int iv = 0; iv < n_vz; ++iv) {
            for (int iq = 0; iq < n_Q2; ++iq) {
                for (int is = 0; is < n_sector; ++is) {
                    MixingBin bin;
                    bin.z_vertex_min = m_vz_bin_edges[iv];
                    bin.z_vertex_max = m_vz_bin_edges[iv + 1];
                    bin.Q2_min = m_Q2_bin_edges[iq];
                    bin.Q2_max = m_Q2_bin_edges[iq + 1];
                    bin.sector = m_separate_sectors.get() ? (is + 1) : -1;
                    m_bins.push_back(bin);
                }
            }
        }
    }

    /**
     * @brief Find bin matching the given values
     */
    auto find_bin(double vz, double Q2, int sector) const -> MixingBin {
        for (const auto& bin : m_bins) {
            if (bin.contains(vz, Q2, sector)) {
                return bin;
            }
        }

        // Return default bin if none match
        MixingBin default_bin;
        return default_bin;
    }

    /**
     * @brief Get or create particle pool
     */
    template <typename T>
    auto get_or_create_pool(const std::string& name) -> EventMixingPool<T>& {
        auto it = m_particle_pools.find(name);
        if (it == m_particle_pools.end()) {
            auto pool = std::make_shared<EventMixingPool<T>>(m_events_per_bin.get());
            m_particle_pools[name] = pool;
            return *pool;
        }
        return *std::static_pointer_cast<EventMixingPool<T>>(it->second);
    }

    /**
     * @brief Get existing particle pool
     */
    template <typename T>
    auto get_pool(const std::string& name) -> EventMixingPool<T>* {
        auto it = m_particle_pools.find(name);
        if (it != m_particle_pools.end()) {
            return std::static_pointer_cast<EventMixingPool<T>>(it->second).get();
        }
        return nullptr;
    }

    template <typename T>
    auto get_pool(const std::string& name) const -> const EventMixingPool<T>* {
        auto it = m_particle_pools.find(name);
        if (it != m_particle_pools.end()) {
            return std::static_pointer_cast<EventMixingPool<T>>(it->second).get();
        }
        return nullptr;
    }
};

}  // namespace clas12::framework
