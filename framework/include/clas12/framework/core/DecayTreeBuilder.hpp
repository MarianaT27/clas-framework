#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

#include <clas12/MCParticle.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/physics/CompositeParticle.hpp>
#include <clas12/framework/core/DecayDescriptor.hpp>
#include <clas12/framework/core/Event.hpp>

namespace clas12::framework {

// Forward declaration
class MCTruthService;

/**
 * @brief A candidate for a decay reconstruction
 */
struct DecayCandidate {
    /// Composite particle (mother)
    clas12::physics::CompositeParticle composite;

    /// Daughter particles (pointers to event particles)
    std::vector<const clas12::Particle*> daughters;

    /// Invariant mass of the candidate [GeV/c^2]
    double mass = 0.0;

    /// Chi-squared from kinematic fit (if fitted)
    double chi2 = -1.0;

    /// Number of degrees of freedom (if fitted)
    int ndf = 0;

    /// Fit probability (if fitted)
    double probability = -1.0;

    /// MC truth match (if MC available)
    const clas12::MCParticle* mc_match = nullptr;

    /// Whether this is a true decay (from MC matching)
    bool is_true_decay = false;

    /// Opening angle between daughters [degrees]
    double opening_angle = 0.0;

    /// Vertex z of the candidate (average of daughters) [cm]
    double vertex_z = 0.0;

    /// Maximum vertex separation between daughters [cm]
    double max_vertex_separation = 0.0;

    // Derived quantities

    /// Total momentum [GeV/c]
    [[nodiscard]] auto p() const -> double { return composite.p(); }

    /// Transverse momentum [GeV/c]
    [[nodiscard]] auto pt() const -> double { return composite.pt(); }

    /// Polar angle [degrees]
    [[nodiscard]] auto theta() const -> double { return composite.theta(); }

    /// Azimuthal angle [degrees]
    [[nodiscard]] auto phi() const -> double { return composite.phi(); }

    /// Check if candidate is valid
    [[nodiscard]] auto is_valid() const -> bool {
        return !daughters.empty() && mass > 0.0;
    }

    /// Check if candidate was fitted
    [[nodiscard]] auto is_fitted() const -> bool { return chi2 >= 0.0; }

    /// Check if mass is within window of target
    [[nodiscard]] auto in_mass_window(double target_mass, double half_width) const -> bool {
        return std::abs(mass - target_mass) < half_width;
    }
};

/**
 * @brief Builder for decay candidates from event particles
 *
 * DecayTreeBuilder uses a DecayDescriptor to construct decay candidates
 * from reconstructed particles in an event. It handles combinatorics,
 * applies pre-selection cuts, and optionally performs MC truth matching.
 *
 * Example usage:
 * @code
 * // Create builder from descriptor
 * auto desc = DecayDescriptor::parse("Lambda0 -> p+ pi-");
 * DecayTreeBuilder builder(desc);
 *
 * // Configure cuts
 * builder.set_mass_window(1.115683, 0.030)  // Lambda mass +/- 30 MeV
 *        .set_max_vertex_separation(3.0);   // 3 cm vertex cut
 *
 * // Build candidates from event
 * auto candidates = builder.build(event);
 *
 * // Get best candidate
 * auto* best = builder.best_candidate(candidates);
 * if (best) {
 *     h_mass->Fill(best->mass);
 * }
 * @endcode
 */
class DecayTreeBuilder {
   public:
    /**
     * @brief Construct builder from decay descriptor
     *
     * @param descriptor Parsed decay descriptor
     */
    explicit DecayTreeBuilder(const DecayDescriptor& descriptor)
        : m_descriptor(descriptor) {
        // Set default mass window based on mother particle
        m_mass_center = descriptor.mother_mass();
        m_mass_half_width = 0.1;  // 100 MeV default
    }

    /**
     * @brief Construct builder from descriptor string
     *
     * @param descriptor_str Decay string (e.g., "Lambda0 -> p+ pi-")
     */
    explicit DecayTreeBuilder(const std::string& descriptor_str)
        : DecayTreeBuilder(DecayDescriptor::parse(descriptor_str)) {}

    // ========================================================================
    // Configuration (Fluent API)
    // ========================================================================

    /**
     * @brief Set mass window for candidate selection
     *
     * @param center Center of mass window [GeV/c^2]
     * @param half_width Half-width of window [GeV/c^2]
     * @return Reference to this builder
     */
    auto set_mass_window(double center, double half_width) -> DecayTreeBuilder& {
        m_mass_center = center;
        m_mass_half_width = half_width;
        return *this;
    }

    /**
     * @brief Set minimum daughter momentum
     *
     * @param p_min Minimum momentum [GeV/c]
     * @return Reference to this builder
     */
    auto set_min_daughter_momentum(double p_min) -> DecayTreeBuilder& {
        m_min_daughter_p = p_min;
        return *this;
    }

    /**
     * @brief Set maximum vertex separation between daughters
     *
     * @param dz_max Maximum vertex z separation [cm]
     * @return Reference to this builder
     */
    auto set_max_vertex_separation(double dz_max) -> DecayTreeBuilder& {
        m_max_vertex_separation = dz_max;
        return *this;
    }

    /**
     * @brief Set minimum opening angle between daughters
     *
     * @param angle_min Minimum opening angle [degrees]
     * @return Reference to this builder
     */
    auto set_min_opening_angle(double angle_min) -> DecayTreeBuilder& {
        m_min_opening_angle = angle_min;
        return *this;
    }

    /**
     * @brief Enable/disable mass window cut
     */
    auto enable_mass_cut(bool enable) -> DecayTreeBuilder& {
        m_apply_mass_cut = enable;
        return *this;
    }

    /**
     * @brief Enable/disable vertex separation cut
     */
    auto enable_vertex_cut(bool enable) -> DecayTreeBuilder& {
        m_apply_vertex_cut = enable;
        return *this;
    }

    // ========================================================================
    // Building API
    // ========================================================================

    /**
     * @brief Build all decay candidates from event
     *
     * Performs combinatorics over all possible daughter combinations
     * and applies pre-selection cuts.
     *
     * @param event Event to build candidates from
     * @return Vector of decay candidates
     */
    auto build(Event& event) -> std::vector<DecayCandidate> {
        std::vector<DecayCandidate> candidates;

        if (!m_descriptor.is_valid()) {
            return candidates;
        }

        // Get final state particle specs
        auto final_states = m_descriptor.final_state_particles();

        if (final_states.size() == 2) {
            // Two-body decay
            build_two_body(event, final_states[0], final_states[1], candidates);
        } else if (final_states.size() == 3) {
            // Three-body decay
            build_three_body(event, final_states[0], final_states[1], final_states[2], candidates);
        }

        return candidates;
    }

    /**
     * @brief Build candidates with MC truth matching
     *
     * @param event Event to build from
     * @param mc_svc MC truth service for matching
     * @return Vector of candidates with truth info
     */
    auto build_with_truth(Event& event, MCTruthService* mc_svc) -> std::vector<DecayCandidate>;

    /**
     * @brief Get best candidate (closest to PDG mass)
     *
     * @param candidates Vector of candidates to search
     * @return Pointer to best candidate, or nullptr if empty
     */
    auto best_candidate(const std::vector<DecayCandidate>& candidates) const
        -> const DecayCandidate* {
        if (candidates.empty()) return nullptr;

        auto it = std::min_element(candidates.begin(), candidates.end(),
                                   [this](const DecayCandidate& a, const DecayCandidate& b) {
                                       return std::abs(a.mass - m_mass_center) <
                                              std::abs(b.mass - m_mass_center);
                                   });

        return &(*it);
    }

    /**
     * @brief Get best candidate (non-const version for modification)
     */
    auto best_candidate(std::vector<DecayCandidate>& candidates) -> DecayCandidate* {
        if (candidates.empty()) return nullptr;

        auto it = std::min_element(candidates.begin(), candidates.end(),
                                   [this](const DecayCandidate& a, const DecayCandidate& b) {
                                       return std::abs(a.mass - m_mass_center) <
                                              std::abs(b.mass - m_mass_center);
                                   });

        return &(*it);
    }

    /**
     * @brief Filter candidates by mass window
     */
    auto filter_by_mass(std::vector<DecayCandidate>& candidates) const -> void {
        candidates.erase(
            std::remove_if(candidates.begin(), candidates.end(),
                           [this](const DecayCandidate& c) {
                               return !c.in_mass_window(m_mass_center, m_mass_half_width);
                           }),
            candidates.end());
    }

    /**
     * @brief Sort candidates by mass (closest to center first)
     */
    auto sort_by_mass(std::vector<DecayCandidate>& candidates) const -> void {
        std::sort(candidates.begin(), candidates.end(),
                  [this](const DecayCandidate& a, const DecayCandidate& b) {
                      return std::abs(a.mass - m_mass_center) < std::abs(b.mass - m_mass_center);
                  });
    }

    // ========================================================================
    // Accessors
    // ========================================================================

    /// Get the decay descriptor
    [[nodiscard]] auto descriptor() const -> const DecayDescriptor& { return m_descriptor; }

    /// Get mass window center
    [[nodiscard]] auto mass_center() const -> double { return m_mass_center; }

    /// Get mass window half-width
    [[nodiscard]] auto mass_half_width() const -> double { return m_mass_half_width; }

   private:
    DecayDescriptor m_descriptor;

    // Cut parameters
    double m_mass_center = 0.0;
    double m_mass_half_width = 0.1;
    double m_min_daughter_p = 0.0;
    double m_max_vertex_separation = 1000.0;  // Effectively no cut
    double m_min_opening_angle = 0.0;

    // Cut enables
    bool m_apply_mass_cut = true;
    bool m_apply_vertex_cut = false;

    /**
     * @brief Build two-body decay candidates
     */
    auto build_two_body(Event& event,
                        const ParticleSpec& spec1,
                        const ParticleSpec& spec2,
                        std::vector<DecayCandidate>& candidates) -> void {

        // Get particles of each type
        auto particles1 = get_particles(event, spec1.pdg);
        auto particles2 = get_particles(event, spec2.pdg);

        // Handle same-particle case (e.g., gamma gamma)
        bool same_type = (spec1.pdg == spec2.pdg);

        for (size_t i = 0; i < particles1.size(); ++i) {
            const auto& p1 = particles1[i];

            // Apply daughter momentum cut
            if (p1.p() < m_min_daughter_p) continue;

            size_t j_start = same_type ? i + 1 : 0;
            for (size_t j = j_start; j < particles2.size(); ++j) {
                const auto& p2 = particles2[j];

                // Skip same particle
                if (&p1 == &p2) continue;

                // Apply daughter momentum cut
                if (p2.p() < m_min_daughter_p) continue;

                // Check vertex separation
                double dz = std::abs(p1.vz() - p2.vz());
                if (m_apply_vertex_cut && dz > m_max_vertex_separation) continue;

                // Build candidate
                auto candidate = make_candidate({&p1, &p2});

                // Apply mass cut
                if (m_apply_mass_cut &&
                    !candidate.in_mass_window(m_mass_center, m_mass_half_width)) {
                    continue;
                }

                candidates.push_back(std::move(candidate));
            }
        }
    }

    /**
     * @brief Build three-body decay candidates
     */
    auto build_three_body(Event& event,
                          const ParticleSpec& spec1,
                          const ParticleSpec& spec2,
                          const ParticleSpec& spec3,
                          std::vector<DecayCandidate>& candidates) -> void {

        auto particles1 = get_particles(event, spec1.pdg);
        auto particles2 = get_particles(event, spec2.pdg);
        auto particles3 = get_particles(event, spec3.pdg);

        for (const auto& p1 : particles1) {
            if (p1.p() < m_min_daughter_p) continue;

            for (const auto& p2 : particles2) {
                if (&p1 == &p2) continue;
                if (p2.p() < m_min_daughter_p) continue;

                for (const auto& p3 : particles3) {
                    if (&p3 == &p1 || &p3 == &p2) continue;
                    if (p3.p() < m_min_daughter_p) continue;

                    // Check max vertex separation
                    double max_dz = std::max({
                        std::abs(p1.vz() - p2.vz()),
                        std::abs(p1.vz() - p3.vz()),
                        std::abs(p2.vz() - p3.vz())});

                    if (m_apply_vertex_cut && max_dz > m_max_vertex_separation) continue;

                    auto candidate = make_candidate({&p1, &p2, &p3});

                    if (m_apply_mass_cut &&
                        !candidate.in_mass_window(m_mass_center, m_mass_half_width)) {
                        continue;
                    }

                    candidates.push_back(std::move(candidate));
                }
            }
        }
    }

    /**
     * @brief Get particles from event by PDG code
     */
    auto get_particles(Event& event, int pdg) -> std::vector<clas12::Particle> {
        // Convert PDG to PIDs enum
        clas12::PIDs pid = pdg_to_pids(pdg);
        if (pid == clas12::PIDs::UNKNOWN) {
            return {};
        }
        return event.particles(pid);
    }

    /**
     * @brief Convert PDG code to PIDs enum
     */
    static auto pdg_to_pids(int pdg) -> clas12::PIDs {
        switch (pdg) {
            case 11: return clas12::PIDs::ELECTRON;
            case -11: return clas12::PIDs::POSITRON;
            case 22: return clas12::PIDs::PHOTON;
            case 211: return clas12::PIDs::PIPLUS;
            case -211: return clas12::PIDs::PIMINUS;
            case 321: return clas12::PIDs::KPLUS;
            case -321: return clas12::PIDs::KMINUS;
            case 2212: return clas12::PIDs::PROTON;
            case 2112: return clas12::PIDs::NEUTRON;
            case 1000010020: return clas12::PIDs::DEUTERON;
            default: return clas12::PIDs::UNKNOWN;
        }
    }

    /**
     * @brief Create a decay candidate from daughters
     */
    auto make_candidate(const std::vector<const clas12::Particle*>& daughters) -> DecayCandidate {
        DecayCandidate candidate;
        candidate.daughters = daughters;

        // Build composite particle
        std::vector<clas12::Particle> daughter_particles;
        double vz_sum = 0.0;
        double max_dz = 0.0;

        for (const auto* p : daughters) {
            daughter_particles.push_back(*p);
            vz_sum += p->vz();
        }

        // Calculate max vertex separation
        for (size_t i = 0; i < daughters.size(); ++i) {
            for (size_t j = i + 1; j < daughters.size(); ++j) {
                double dz = std::abs(daughters[i]->vz() - daughters[j]->vz());
                max_dz = std::max(max_dz, dz);
            }
        }

        candidate.composite = clas12::physics::CompositeParticle(
            daughter_particles, m_descriptor.mother_pdg());
        candidate.mass = candidate.composite.mass();
        candidate.vertex_z = vz_sum / daughters.size();
        candidate.max_vertex_separation = max_dz;

        // Calculate opening angle (for two-body)
        if (daughters.size() == 2) {
            auto p1 = daughters[0]->PxPyPzVector();
            auto p2 = daughters[1]->PxPyPzVector();
            double cos_angle = p1.Dot(p2) / (p1.R() * p2.R());
            candidate.opening_angle = std::acos(std::clamp(cos_angle, -1.0, 1.0)) * 180.0 / M_PI;
        }

        return candidate;
    }
};

}  // namespace clas12::framework
