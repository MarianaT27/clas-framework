#pragma once

#include <cmath>
#include <optional>
#include <tuple>
#include <vector>
#include "Core/PIDs.hpp"
#include "Core/Constants.hpp"
#include "Core/Particle.hpp"
#include "Core/Cuts.hpp"
#include "Core/ReadBank.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/IAlgorithm.hpp"
#include "Framework/ServiceHandle.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "bank.h"

namespace Algorithms {

/**
 * @brief Algorithm for π⁰ reconstruction from photon pairs
 *
 * This algorithm reconstructs π⁰ candidates by:
 * 1. Selecting photons above a minimum energy threshold
 * 2. Forming all possible photon pair combinations
 * 3. Calculating invariant mass for each pair
 * 4. Selecting the best candidate within a mass window around the π⁰ mass
 *
 * Requirements: 1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 2.4
 */
class Pi0ReconstructionAlg : public Framework::Algorithm {
   public:
    explicit Pi0ReconstructionAlg(
        std::string name = "Pi0ReconstructionAlg",
        double min_photon_energy = 0.1,  // GeV
        double pi0_mass_window = 0.03    // GeV (±30 MeV around 135 MeV)
    );

    auto initialize() -> Framework::StatusCode override;
    auto execute(Framework::Event& event) -> Framework::StatusCode override;
    auto finalize() -> Framework::StatusCode override;

    // Public helper methods for testing
    auto select_photons(const std::vector<Core::Particle>& photons) const -> std::vector<Core::Particle>;
    auto select_photons_with_AI(hipo::bank& RUN_config, hipo::bank&  REC_Particle, hipo::bank& REC_Calorimeter) const -> std::vector<Core::Particle>;

    static auto calculate_invariant_mass(const Core::Particle& g1, const Core::Particle& g2) -> double;

    auto find_best_pi0(const std::vector<Core::Particle>& photons) const -> std::optional<std::tuple<Core::Particle, Core::Particle, double>>;

    // Accessors for configuration (useful for testing)
    auto min_photon_energy() const -> double { return m_min_photon_energy; }

    auto pi0_mass_window() const -> double { return m_pi0_mass_window; }

    static constexpr double PI0_MASS = Core::Constants::PI0MASS;  // GeV/c²

   private:
    // Configuration
    double m_min_photon_energy;
    double m_pi0_mass_window;

    // Services
    Framework::ServiceHandle<Framework::HistogramService> m_hist_svc{"HistogramService"};
    Framework::ServiceHandle<Framework::NTupleService> m_ntuple_svc{"NTupleService"};
    Framework::ServiceHandle<Framework::LoggingService> m_log_svc{"LoggingService"};

    // Cutflow
    Framework::Cutflow m_cutflow;

    // Histograms (non-owning pointers managed by HistogramService)
    TH1D* h_photon_energy = nullptr;
    TH1D* h_photon_theta = nullptr;
    TH1D* h_n_photons = nullptr;
    TH1D* h_gg_mass_all = nullptr;
    TH1D* h_gg_mass_selected = nullptr;
    TH1D* h_pi0_momentum = nullptr;
    TH1D* h_pi0_theta = nullptr;
    TH1D* h_opening_angle = nullptr;

    // NTuple branch variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_g1_E = 0.0;
    double m_g1_theta = 0.0;
    double m_g1_phi = 0.0;
    double m_g2_E = 0.0;
    double m_g2_theta = 0.0;
    double m_g2_phi = 0.0;
    double m_pi0_mass = 0.0;
    double m_pi0_p = 0.0;
    double m_pi0_theta = 0.0;
    double m_pi0_phi = 0.0;

    // Private helper methods
    auto book_histograms() -> void;
    auto setup_ntuple() -> void;
};

}  // namespace Algorithms
