#pragma once

#include <cmath>
#include <optional>
#include <tuple>
#include <vector>
#include "CLAS/Constants.hpp"
#include "CLAS/PIDs.hpp"
#include "CLAS/Particle.hpp"
#include "CLAS/ReadBank.hpp"
#include "Framework/Cutflow.hpp"
#include "Framework/Event.hpp"
#include "Framework/Property.hpp"
#include "Framework/SimpleAlgorithm.hpp"
#include "Framework/ThreadSafeHistogram.hpp"
#include "Framework/Services/HistogramService.hpp"
#include "Framework/Services/NTupleService.hpp"
#include "Framework/Services/PhotonCutsService.hpp"
#include "bank.h"

namespace Algorithms {

/**
 * @brief Algorithm for π⁰ reconstruction from photon pairs
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>()
 * - Property system for configuration
 * - Reduced boilerplate through CRTP
 */
class Pi0ReconstructionAlg : public Framework::SimpleAlgorithm<Pi0ReconstructionAlg> {
   public:
    explicit Pi0ReconstructionAlg(
        std::string name = "Pi0ReconstructionAlg",
        double min_photon_energy = 0.1,  // GeV
        double pi0_mass_window = 0.03    // GeV (±30 MeV around 135 MeV)
    );

    // SimpleAlgorithm interface
    auto configure() -> void override;
    auto on_initialize() -> Framework::StatusCode override;
    auto process(Framework::Event& event) -> Framework::StatusCode override;
    auto on_finalize() -> Framework::StatusCode override;

    // Public helper methods for testing
    auto select_photons(const std::vector<CLAS::Particle>& photons) const -> std::vector<CLAS::Particle>;
    auto select_photons_with_AI(hipo::bank& RUN_config, hipo::bank& REC_Particle, hipo::bank& REC_Calorimeter) const -> std::vector<CLAS::Particle>;

    static auto calculate_invariant_mass(const CLAS::Particle& g1, const CLAS::Particle& g2) -> double;

    auto find_best_pi0(const std::vector<CLAS::Particle>& photons) const -> std::optional<std::tuple<CLAS::Particle, CLAS::Particle, double>>;

    auto find_all_unique_pi0_candidates(const std::vector<CLAS::Particle>& photons) const -> std::optional<std::vector<std::tuple<CLAS::Particle, CLAS::Particle, double>>>;

    auto min_photon_energy() const -> double { return m_min_photon_energy.get(); }

    auto pi0_mass_window() const -> double { return m_pi0_mass_window.get(); }

    static constexpr double PI0_MASS = CLAS::Constants::PI0MASS;  // GeV/c²

   private:
    // Properties
    Framework::Property<double> m_min_photon_energy{"min_photon_energy", 0.1, "Minimum photon energy [GeV]"};
    Framework::Property<double> m_pi0_mass_window{"pi0_mass_window", 0.03, "Pi0 mass window [GeV]"};
    Framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles (auto-injected via require_service)
    Framework::ServiceHandle<Framework::HistogramService>* m_hist_svc = nullptr;
    Framework::ServiceHandle<Framework::NTupleService>* m_ntuple_svc = nullptr;
    Framework::ServiceHandle<Framework::PhotonCutsService>* m_photon_cuts_svc = nullptr;

    // Cutflow
    Framework::Cutflow m_cutflow;

    // Add mutex for thread safety
    mutable std::mutex m_ntuple_mutex;

    // Thread-safe histograms (wrappers managed by HistogramService)
    Framework::ThreadSafeHist1D* h_photon_energy_raw = nullptr;
    Framework::ThreadSafeHist1D* h_photon_theta_raw = nullptr;
    Framework::ThreadSafeHist1D* h_photon_phi_raw = nullptr;
    Framework::ThreadSafeHist1D* h_photon_energy = nullptr;
    Framework::ThreadSafeHist1D* h_photon_theta = nullptr;
    Framework::ThreadSafeHist1D* h_n_photons = nullptr;
    Framework::ThreadSafeHist1D* h_gg_mass_all = nullptr;
    Framework::ThreadSafeHist1D* h_gg_mass_traditional = nullptr;
    Framework::ThreadSafeHist1D* h_gg_mass_selected = nullptr;
    Framework::ThreadSafeHist1D* h_gg_mass_AI = nullptr;
    Framework::ThreadSafeHist1D* h_pi0_momentum = nullptr;
    Framework::ThreadSafeHist1D* h_pi0_theta = nullptr;
    Framework::ThreadSafeHist1D* h_opening_angle = nullptr;

    // NTuple branch variables
    double m_e_p = 0.0;
    double m_e_theta = 0.0;
    double m_e_phi = 0.0;
    double m_e_vz = 0.0;
    double m_g1_E = 0.0;
    std::vector<double> mgs_E;
    std::vector<double> mgs_theta;
    std::vector<double> mgs_phi;
    std::vector<double> mpi0_masses;
    std::vector<double> mpi0_ps;
    std::vector<double> mpi0_thetas;
    std::vector<double> mpi0_phis;

    // Private helper methods
    auto book_histograms() -> void;
    auto setup_ntuple() -> void;
};

}  // namespace Algorithms
