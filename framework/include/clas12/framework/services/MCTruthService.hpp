#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <clas12/MCParticle.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

#include <hipo4/bank.h>

namespace clas12::framework {

/**
 * @brief Service for Monte Carlo truth matching and analysis
 *
 * MCTruthService provides functionality for:
 * - Loading MC truth particles from MC::Particle bank
 * - Matching reconstructed particles to their MC truth counterparts
 * - Computing resolution and efficiency measurements
 * - Following decay chains in generated events
 *
 * This service follows patterns from LHC experiments (ATLAS, CMS, LHCb)
 * for truth-level analysis and efficiency studies.
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_mc_svc = &require_service<MCTruthService>("MCTruthService");
 *
 * // In algorithm process():
 * if (event.has_bank("MC::Particle")) {
 *     auto& mc_bank = event.get_bank("MC::Particle");
 *     auto& rec_match_bank = event.get_bank("MC::RecMatch");
 *
 *     m_mc_svc->load_event(mc_bank, rec_match_bank);
 *
 *     // Get all MC electrons
 *     auto mc_electrons = m_mc_svc->mc_particles(clas12::PIDs::ELECTRON);
 *
 *     // Match reconstructed electron to truth
 *     for (const auto& rec_electron : event.particles(clas12::PIDs::ELECTRON)) {
 *         auto* mc_match = m_mc_svc->match(rec_electron);
 *         if (mc_match) {
 *             double p_res = m_mc_svc->momentum_resolution(rec_electron);
 *             h_p_resolution->Fill(p_res.value());
 *         }
 *     }
 * }
 * @endcode
 *
 * @see Requirements for MC truth matching in physics analysis
 */
class MCTruthService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "MCTruthService"; }

    explicit MCTruthService(std::string name = "MCTruthService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "MCTruthService initialized with min_match_quality={}",
                  m_min_match_quality.get());
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "MCTruthService finalized. Events processed: {}, "
                                "Particles loaded: {}, Matches found: {}",
                  m_events_processed, m_particles_loaded, m_matches_found);
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Minimum match quality threshold (0-1 scale)
    Property<double> m_min_match_quality{"min_match_quality", 0.92,
                                         "Minimum match quality threshold for MC matching"};

    /// Whether to automatically load MC particles on event access
    Property<bool> m_auto_load{"auto_load", false,
                               "Automatically load MC particles when accessing event"};

    // ========================================================================
    // Event Loading API
    // ========================================================================

    /**
     * @brief Load MC particles and matching info for current event
     *
     * This must be called at the start of each event before using
     * matching functions. Clears previous event's data.
     *
     * @param mc_particle_bank The MC::Particle bank
     * @param rec_match_bank Optional MC::RecMatch bank for matching info
     * @param gen_match_bank Optional MC::GenMatch bank for reverse matching
     */
    auto load_event(hipo::bank& mc_particle_bank,
                    hipo::bank* rec_match_bank = nullptr,
                    hipo::bank* gen_match_bank = nullptr) -> void {
        clear_event();
        load_mc_particles(mc_particle_bank);

        if (rec_match_bank) {
            load_rec_match(*rec_match_bank);
        }

        if (gen_match_bank) {
            load_gen_match(*gen_match_bank);
        }

        m_events_processed++;
    }

    /**
     * @brief Clear all MC data from current event
     */
    auto clear_event() -> void {
        m_mc_particles.clear();
        m_mc_particles_by_pdg.clear();
        m_rec_to_mc_index.clear();
        m_mc_to_rec_index.clear();
        m_match_qualities.clear();
    }

    /**
     * @brief Check if MC data is loaded for current event
     */
    [[nodiscard]] auto is_loaded() const -> bool {
        return !m_mc_particles.empty();
    }

    // ========================================================================
    // MC Particle Access
    // ========================================================================

    /**
     * @brief Get all MC particles in current event
     * @return Reference to vector of all MC particles
     */
    [[nodiscard]] auto mc_particles() const -> const std::vector<clas12::MCParticle>& {
        return m_mc_particles;
    }

    /**
     * @brief Get MC particles of a specific type
     * @param pid Particle ID (from PIDs enum)
     * @return Vector of pointers to matching MC particles
     */
    [[nodiscard]] auto mc_particles(clas12::PIDs pid) const
        -> std::vector<const clas12::MCParticle*> {
        return mc_particles(static_cast<int>(pid));
    }

    /**
     * @brief Get MC particles by PDG code
     * @param pdg PDG particle ID
     * @return Vector of pointers to matching MC particles
     */
    [[nodiscard]] auto mc_particles(int pdg) const
        -> std::vector<const clas12::MCParticle*> {
        std::vector<const clas12::MCParticle*> result;

        auto it = m_mc_particles_by_pdg.find(pdg);
        if (it != m_mc_particles_by_pdg.end()) {
            result.reserve(it->second.size());
            for (int idx : it->second) {
                if (idx >= 0 && static_cast<size_t>(idx) < m_mc_particles.size()) {
                    result.push_back(&m_mc_particles[idx]);
                }
            }
        }

        return result;
    }

    /**
     * @brief Get MC particle by index
     * @param index Index in MC::Particle bank
     * @return Pointer to MC particle, or nullptr if not found
     */
    [[nodiscard]] auto mc_particle(int index) const -> const clas12::MCParticle* {
        if (index >= 0 && static_cast<size_t>(index) < m_mc_particles.size()) {
            return &m_mc_particles[index];
        }
        return nullptr;
    }

    /**
     * @brief Get number of MC particles
     */
    [[nodiscard]] auto num_mc_particles() const -> size_t {
        return m_mc_particles.size();
    }

    /**
     * @brief Get primary MC particles only (from initial interaction)
     */
    [[nodiscard]] auto primary_particles() const -> std::vector<const clas12::MCParticle*> {
        std::vector<const clas12::MCParticle*> result;
        for (const auto& mc : m_mc_particles) {
            if (mc.is_primary()) {
                result.push_back(&mc);
            }
        }
        return result;
    }

    /**
     * @brief Get primary MC particles of specific type
     */
    [[nodiscard]] auto primary_particles(clas12::PIDs pid) const
        -> std::vector<const clas12::MCParticle*> {
        std::vector<const clas12::MCParticle*> result;
        for (const auto* mc : mc_particles(pid)) {
            if (mc->is_primary()) {
                result.push_back(mc);
            }
        }
        return result;
    }

    // ========================================================================
    // Matching API (Reconstructed -> MC Truth)
    // ========================================================================

    /**
     * @brief Match a reconstructed particle to its MC truth
     *
     * Uses the MC::RecMatch bank to find the best matching MC particle.
     * Returns nullptr if no match above quality threshold is found.
     *
     * @param rec Reconstructed particle to match
     * @return Pointer to matching MC particle, or nullptr
     */
    [[nodiscard]] auto match(const clas12::Particle& rec) const
        -> const clas12::MCParticle* {
        auto it = m_rec_to_mc_index.find(rec.index());
        if (it != m_rec_to_mc_index.end() && it->second >= 0) {
            return mc_particle(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Get match quality for a reconstructed particle
     * @param rec Reconstructed particle
     * @return Match quality (0-1), or -1 if not matched
     */
    [[nodiscard]] auto match_quality(const clas12::Particle& rec) const -> float {
        auto it = m_match_qualities.find(rec.index());
        if (it != m_match_qualities.end()) {
            return it->second;
        }
        return -1.0f;
    }

    /**
     * @brief Check if reconstructed particle is matched to MC truth
     */
    [[nodiscard]] auto is_matched(const clas12::Particle& rec) const -> bool {
        return match(rec) != nullptr;
    }

    // ========================================================================
    // Reverse Matching API (MC Truth -> Reconstructed)
    // ========================================================================

    /**
     * @brief Check if an MC particle was reconstructed
     * @param mc MC particle to check
     * @return True if a reconstructed particle matches this MC particle
     */
    [[nodiscard]] auto is_reconstructed(const clas12::MCParticle& mc) const -> bool {
        auto it = m_mc_to_rec_index.find(mc.index());
        return it != m_mc_to_rec_index.end() && it->second >= 0;
    }

    /**
     * @brief Get reconstructed particle index for an MC particle
     * @param mc MC particle
     * @return Reconstructed particle index, or -1 if not reconstructed
     */
    [[nodiscard]] auto reconstructed_index(const clas12::MCParticle& mc) const -> int {
        auto it = m_mc_to_rec_index.find(mc.index());
        if (it != m_mc_to_rec_index.end()) {
            return it->second;
        }
        return -1;
    }

    // ========================================================================
    // Resolution and Efficiency Helpers
    // ========================================================================

    /**
     * @brief Calculate momentum resolution for a matched particle
     *
     * Returns (p_rec - p_true) / p_true
     *
     * @param rec Reconstructed particle (must be matched)
     * @return Resolution value, or nullopt if not matched
     */
    [[nodiscard]] auto momentum_resolution(const clas12::Particle& rec) const
        -> std::optional<double> {
        const auto* mc = match(rec);
        if (!mc || mc->p() < 1e-6) return std::nullopt;
        return (rec.p() - mc->p()) / mc->p();
    }

    /**
     * @brief Calculate theta resolution for a matched particle
     *
     * Returns (theta_rec - theta_true) in degrees
     *
     * @param rec Reconstructed particle (must be matched)
     * @return Resolution value, or nullopt if not matched
     */
    [[nodiscard]] auto theta_resolution(const clas12::Particle& rec) const
        -> std::optional<double> {
        const auto* mc = match(rec);
        if (!mc) return std::nullopt;
        return rec.theta() - mc->theta();
    }

    /**
     * @brief Calculate phi resolution for a matched particle
     *
     * Returns (phi_rec - phi_true) in degrees, handling wrap-around
     *
     * @param rec Reconstructed particle (must be matched)
     * @return Resolution value, or nullopt if not matched
     */
    [[nodiscard]] auto phi_resolution(const clas12::Particle& rec) const
        -> std::optional<double> {
        const auto* mc = match(rec);
        if (!mc) return std::nullopt;

        double dphi = rec.phi() - mc->phi();
        // Handle wrap-around
        while (dphi > 180.0) dphi -= 360.0;
        while (dphi < -180.0) dphi += 360.0;
        return dphi;
    }

    /**
     * @brief Calculate vertex z resolution for a matched particle
     *
     * Returns (vz_rec - vz_true) in cm
     *
     * @param rec Reconstructed particle (must be matched)
     * @return Resolution value, or nullopt if not matched
     */
    [[nodiscard]] auto vz_resolution(const clas12::Particle& rec) const
        -> std::optional<double> {
        const auto* mc = match(rec);
        if (!mc) return std::nullopt;
        return rec.vz() - mc->vz();
    }

    /**
     * @brief Calculate reconstruction efficiency for a particle type
     *
     * Counts MC particles of given type and how many were reconstructed.
     *
     * @param pid Particle type
     * @return Efficiency (0-1), or -1 if no MC particles of this type
     */
    [[nodiscard]] auto reconstruction_efficiency(clas12::PIDs pid) const -> double {
        auto mc_list = mc_particles(pid);
        if (mc_list.empty()) return -1.0;

        int n_reconstructed = 0;
        for (const auto* mc : mc_list) {
            if (is_reconstructed(*mc)) {
                n_reconstructed++;
            }
        }

        return static_cast<double>(n_reconstructed) / static_cast<double>(mc_list.size());
    }

    /**
     * @brief Calculate primary reconstruction efficiency
     *
     * Only considers primary particles (from initial vertex).
     */
    [[nodiscard]] auto primary_efficiency(clas12::PIDs pid) const -> double {
        auto mc_list = primary_particles(pid);
        if (mc_list.empty()) return -1.0;

        int n_reconstructed = 0;
        for (const auto* mc : mc_list) {
            if (is_reconstructed(*mc)) {
                n_reconstructed++;
            }
        }

        return static_cast<double>(n_reconstructed) / static_cast<double>(mc_list.size());
    }

    // ========================================================================
    // Decay Chain Analysis
    // ========================================================================

    /**
     * @brief Get parent of an MC particle
     * @param mc MC particle
     * @return Pointer to parent particle, or nullptr if primary
     */
    [[nodiscard]] auto parent(const clas12::MCParticle& mc) const
        -> const clas12::MCParticle* {
        if (mc.is_primary()) return nullptr;
        return mc_particle(mc.parent_index());
    }

    /**
     * @brief Get all daughters of an MC particle
     * @param mc MC particle
     * @return Vector of pointers to daughter particles
     */
    [[nodiscard]] auto daughters(const clas12::MCParticle& mc) const
        -> std::vector<const clas12::MCParticle*> {
        std::vector<const clas12::MCParticle*> result;
        for (const auto& other : m_mc_particles) {
            if (other.parent_index() == mc.index()) {
                result.push_back(&other);
            }
        }
        return result;
    }

    /**
     * @brief Check if MC particle is from a specific decay
     *
     * Walks up the parent chain looking for a particle with the given PDG.
     *
     * @param mc MC particle to check
     * @param parent_pdg PDG code of potential ancestor
     * @return True if descended from particle with given PDG
     */
    [[nodiscard]] auto is_from_decay(const clas12::MCParticle& mc, int parent_pdg) const -> bool {
        const clas12::MCParticle* current = &mc;
        while (current && !current->is_primary()) {
            const auto* p = parent(*current);
            if (p && p->pdg() == parent_pdg) {
                return true;
            }
            current = p;
        }
        return false;
    }

    // ========================================================================
    // Statistics
    // ========================================================================

    /// Get total events processed
    [[nodiscard]] auto events_processed() const -> size_t { return m_events_processed; }

    /// Get total particles loaded
    [[nodiscard]] auto particles_loaded() const -> size_t { return m_particles_loaded; }

    /// Get total matches found
    [[nodiscard]] auto matches_found() const -> size_t { return m_matches_found; }

   private:
    // Event data (cleared each event)
    std::vector<clas12::MCParticle> m_mc_particles;
    std::map<int, std::vector<int>> m_mc_particles_by_pdg;  // PDG -> indices
    std::map<int, int> m_rec_to_mc_index;                   // rec_index -> mc_index
    std::map<int, int> m_mc_to_rec_index;                   // mc_index -> rec_index
    std::map<int, float> m_match_qualities;                 // rec_index -> quality

    // Statistics
    size_t m_events_processed = 0;
    size_t m_particles_loaded = 0;
    size_t m_matches_found = 0;

    /**
     * @brief Load MC particles from MC::Particle bank
     */
    auto load_mc_particles(hipo::bank& bank) -> void {
        const int n_rows = bank.getRows();
        m_mc_particles.reserve(n_rows);

        for (int row = 0; row < n_rows; ++row) {
            int pdg = bank.getInt("pid", row);
            double px = bank.getFloat("px", row);
            double py = bank.getFloat("py", row);
            double pz = bank.getFloat("pz", row);
            double vx = bank.getFloat("vx", row);
            double vy = bank.getFloat("vy", row);
            double vz = bank.getFloat("vz", row);
            double vt = bank.getFloat("vt", row);

            // Calculate mass and energy from PDG mass
            double mass = get_pdg_mass(pdg);
            double p_mag = std::sqrt(px * px + py * py + pz * pz);
            double E = std::sqrt(p_mag * p_mag + mass * mass);

            // Parent index (typically stored in "parent" column, may vary by bank version)
            int parent_index = -1;
            if (bank.getSchema().hasEntry("parent")) {
                parent_index = bank.getInt("parent", row);
            }

            m_mc_particles.emplace_back(pdg, row, parent_index,
                                        px, py, pz, E, mass,
                                        vx, vy, vz, vt);

            // Index by PDG
            m_mc_particles_by_pdg[pdg].push_back(row);
        }

        m_particles_loaded += n_rows;
    }

    /**
     * @brief Load reconstructed-to-MC matching from MC::RecMatch bank
     */
    auto load_rec_match(hipo::bank& bank) -> void {
        const int n_rows = bank.getRows();
        const float min_quality = static_cast<float>(m_min_match_quality.get());

        for (int row = 0; row < n_rows; ++row) {
            int rec_index = bank.getInt("pindex", row);
            int mc_index = bank.getInt("mcindex", row);
            float quality = bank.getFloat("quality", row);

            if (quality >= min_quality && mc_index >= 0) {
                // Check if this rec particle already has a better match
                auto it = m_match_qualities.find(rec_index);
                if (it == m_match_qualities.end() || quality > it->second) {
                    m_rec_to_mc_index[rec_index] = mc_index;
                    m_match_qualities[rec_index] = quality;

                    // Update MC particle's matched status
                    if (mc_index >= 0 && static_cast<size_t>(mc_index) < m_mc_particles.size()) {
                        m_mc_particles[mc_index].set_matched_rec_index(rec_index);
                        m_mc_particles[mc_index].set_match_quality(quality);
                    }

                    m_matches_found++;
                }
            }
        }
    }

    /**
     * @brief Load MC-to-reconstructed matching from MC::GenMatch bank
     */
    auto load_gen_match(hipo::bank& bank) -> void {
        const int n_rows = bank.getRows();
        const float min_quality = static_cast<float>(m_min_match_quality.get());

        for (int row = 0; row < n_rows; ++row) {
            int mc_index = bank.getInt("mcindex", row);
            int rec_index = bank.getInt("pindex", row);
            float quality = bank.getFloat("quality", row);

            if (quality >= min_quality && rec_index >= 0) {
                m_mc_to_rec_index[mc_index] = rec_index;
            }
        }
    }

    /**
     * @brief Get PDG mass for common particles
     */
    static auto get_pdg_mass(int pdg) -> double {
        switch (std::abs(pdg)) {
            case 11:
                return 0.000511;  // electron
            case 13:
                return 0.10566;  // muon
            case 22:
                return 0.0;  // photon
            case 111:
                return 0.13498;  // pi0
            case 211:
                return 0.13957;  // pi+-
            case 321:
                return 0.49368;  // K+-
            case 2212:
                return 0.93827;  // proton
            case 2112:
                return 0.93957;  // neutron
            case 3122:
                return 1.11568;  // Lambda
            case 113:
                return 0.77526;  // rho0
            case 443:
                return 3.09690;  // J/psi
            default:
                return 0.0;
        }
    }
};

}  // namespace clas12::framework
