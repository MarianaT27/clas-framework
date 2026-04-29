#pragma once

#include <cmath>
#include <optional>
#include <tuple>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/physics/CompositeParticle.hpp>

namespace algorithms {

/// Configuration for Pi0ReconstructionAlg.
struct Pi0ReconstructionConfig {
    double pi0_mass_window = 0.03;  // GeV (±30 MeV around 135 MeV)
    double min_photon_energy = 0.1;

    static constexpr auto config_fields() {
        using T = Pi0ReconstructionConfig;
        return std::make_tuple(
            framework::field("pi0_mass_window",  &T::pi0_mass_window),
            framework::field("min_photon_energy", &T::min_photon_energy));
    }
};

/**
 * @brief Pure reconstruction algorithm for π⁰ → γγ
 *
 * Reads selected photons from the event store (placed by PhotonSelectionAlg),
 * finds π⁰ candidates from photon pairs within a mass window, and stores
 * results back in the event store for downstream algorithms.
 */
class Pi0ReconstructionAlg : public framework::Algorithm {
   public:
    using Config = Pi0ReconstructionConfig;
    using StatusCode = framework::StatusCode;

    explicit Pi0ReconstructionAlg(std::string name = "Pi0ReconstructionAlg", const Config& config = Config{});

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override;
    auto process(framework::Event& event) -> StatusCode override;
    auto on_finalize() -> StatusCode override;

    // Public helper methods (also useful for unit tests).
    auto select_photons(const std::vector<Particle>& photons) const -> std::vector<Particle>;
    static auto calculate_invariant_mass(const Particle& g1, const Particle& g2) -> double;
    auto find_best_pi0(const std::vector<Particle>& photons) const -> std::optional<std::tuple<Particle, Particle, double>>;
    auto find_all_unique_pi0_candidates(const std::vector<Particle>& photons) const -> std::optional<std::vector<std::tuple<Particle, Particle, double>>>;

    auto pi0_mass_window() const -> double { return m_config.pi0_mass_window; }

    static constexpr double PI0_MASS = Constants::PI0MASS;

   private:
    Config m_config;
    framework::Cutflow m_cutflow;
};

}  // namespace algorithms
