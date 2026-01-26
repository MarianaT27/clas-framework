#pragma once

#include <cmath>
#include <optional>
#include <tuple>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/Particle.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/framework/core/AtomicHistogram.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/ServiceHelpers.hpp>
#include <clas12/framework/core/SimpleAlgorithm.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/NTupleService.hpp>
#include <clas12/framework/services/PhotonCutsService.hpp>
#include "bank.h"

namespace clas12::algorithms {

/**
 * @brief Algorithm for π⁰ reconstruction from photon pairs
 *
 * Modernized using SimpleAlgorithm CRTP pattern with:
 * - Automatic service injection via require_service<T>()
 * - Property system for configuration
 * - Reduced boilerplate through CRTP
 */
class Pi0ReconstructionAlg : public clas12::framework::SimpleAlgorithm<Pi0ReconstructionAlg> {
   public:
    explicit Pi0ReconstructionAlg(
        std::string name = "Pi0ReconstructionAlg",
        double min_photon_energy = 0.1,  // GeV
        double pi0_mass_window = 0.03    // GeV (±30 MeV around 135 MeV)
    );

    // SimpleAlgorithm interface
    auto configure() -> void override;
    auto on_initialize() -> clas12::framework::StatusCode override;
    auto process(clas12::framework::Event& event) -> clas12::framework::StatusCode override;
    auto on_finalize() -> clas12::framework::StatusCode override;

    // Public helper methods for testing
    auto select_photons(const std::vector<clas12::Particle>& photons) const -> std::vector<clas12::Particle>;
    auto select_photons_with_AI(hipo::bank& RUN_config, hipo::bank& REC_Particle, hipo::bank& REC_Calorimeter) const -> std::vector<clas12::Particle>;

    static auto calculate_invariant_mass(const clas12::Particle& g1, const clas12::Particle& g2) -> double;

    auto find_best_pi0(const std::vector<clas12::Particle>& photons) const -> std::optional<std::tuple<clas12::Particle, clas12::Particle, double>>;

    auto find_all_unique_pi0_candidates(const std::vector<clas12::Particle>& photons) const -> std::optional<std::vector<std::tuple<clas12::Particle, clas12::Particle, double>>>;

    auto min_photon_energy() const -> double { return m_min_photon_energy.get(); }

    auto pi0_mass_window() const -> double { return m_pi0_mass_window.get(); }

    static constexpr double PI0_MASS = clas12::Constants::PI0MASS;  // GeV/c²

   private:
    // Properties
    clas12::framework::Property<double> m_min_photon_energy{"min_photon_energy", 0.1, "Minimum photon energy [GeV]"};
    clas12::framework::Property<double> m_pi0_mass_window{"pi0_mass_window", 0.03, "Pi0 mass window [GeV]"};
    clas12::framework::Property<bool> m_enable_ntuple{"enable_ntuple", true, "Enable N-tuple output"};

    // Service handles (auto-injected via require_service)
    clas12::framework::ServiceHandle<clas12::framework::HistogramService>* m_hist_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::NTupleService>* m_ntuple_svc = nullptr;
    clas12::framework::ServiceHandle<clas12::framework::PhotonCutsService>* m_photon_cuts_svc = nullptr;

    // Cutflow
    clas12::framework::Cutflow m_cutflow;

    // Add mutex for thread safety
    mutable std::mutex m_ntuple_mutex;

    // Lock-free atomic histograms (managed by HistogramService)
    clas12::framework::AtomicHistogram1D* h_photon_energy_raw = nullptr;
    clas12::framework::AtomicHistogram1D* h_photon_theta_raw = nullptr;
    clas12::framework::AtomicHistogram1D* h_photon_phi_raw = nullptr;
    clas12::framework::AtomicHistogram1D* h_photon_energy = nullptr;
    clas12::framework::AtomicHistogram1D* h_photon_theta = nullptr;
    clas12::framework::AtomicHistogram1D* h_n_photons = nullptr;
    clas12::framework::AtomicHistogram1D* h_gg_mass_all = nullptr;
    clas12::framework::AtomicHistogram1D* h_gg_mass_traditional = nullptr;
    clas12::framework::AtomicHistogram1D* h_gg_mass_selected = nullptr;
    clas12::framework::AtomicHistogram1D* h_gg_mass_AI = nullptr;
    clas12::framework::AtomicHistogram1D* h_pi0_momentum = nullptr;
    clas12::framework::AtomicHistogram1D* h_pi0_theta = nullptr;
    clas12::framework::AtomicHistogram1D* h_opening_angle = nullptr;

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

}  // namespace clas12::algorithms
