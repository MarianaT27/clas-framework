#pragma once

#include <cmath>
#include <optional>
#include <tuple>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/physics/CompositeParticle.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>

namespace clas12::algorithms {

/**
 * @brief Pure reconstruction algorithm for π⁰ → γγ
 *
 * Reads selected photons from the event store (placed by PhotonSelectionAlg),
 * finds π⁰ candidates from photon pairs within a mass window, and stores
 * results back in the event store for downstream algorithms.
 *
 * Inputs from event store:
 * - Keys::SelectedPhotons_Pi0: selected photons from PhotonSelectionAlg
 *
 * Outputs stored in event store:
 * - Keys::Pi0Candidates: vector of CompositeParticle (π⁰ candidates)
 * - Decisions::HasPi0: true if at least one candidate found
 */
class Pi0ReconstructionAlg : public clas12::framework::SimpleAlgorithm {
   public:
    explicit Pi0ReconstructionAlg(
        std::string name = "Pi0ReconstructionAlg",
        double pi0_mass_window = 0.03    // GeV (±30 MeV around 135 MeV)
    );

    // SimpleAlgorithm interface
    auto configure() -> void override;
    auto on_initialize() -> clas12::framework::StatusCode override;
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override;
    auto on_finalize() -> clas12::framework::StatusCode override;

    // Public helper methods for testing
    auto select_photons(const std::vector<clas12::Particle>& photons) const -> std::vector<clas12::Particle>;

    static auto calculate_invariant_mass(const clas12::Particle& g1, const clas12::Particle& g2) -> double;

    auto find_best_pi0(const std::vector<clas12::Particle>& photons) const -> std::optional<std::tuple<clas12::Particle, clas12::Particle, double>>;

    auto find_all_unique_pi0_candidates(const std::vector<clas12::Particle>& photons) const -> std::optional<std::vector<std::tuple<clas12::Particle, clas12::Particle, double>>>;

    auto pi0_mass_window() const -> double { return m_pi0_mass_window.get(); }

    static constexpr double PI0_MASS = clas12::Constants::PI0MASS;  // GeV/c²

   private:
    // Properties
    clas12::framework::Property<double> m_min_photon_energy{"min_photon_energy", 0.1, "Minimum photon energy [GeV]"};
    clas12::framework::Property<double> m_pi0_mass_window{"pi0_mass_window", 0.03, "Pi0 mass window [GeV]"};

    // Cutflow
    clas12::framework::Cutflow m_cutflow;
};

}  // namespace clas12::algorithms
