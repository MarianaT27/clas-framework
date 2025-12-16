#pragma once

#include <memory>
#include <string>
#include "Framework/Histogram.hpp"
#include "Framework/Services/HistogramService.hpp"

namespace Framework {

/**
 * @brief Predefined histogram templates for common analyses
 *
 * Provides standard histogram configurations for:
 * - Particle kinematics
 * - Quality assurance
 * - Detector response
 * - Trigger studies
 */
namespace HistogramTemplates {

/**
 * @brief Standard particle kinematics histograms
 */
struct ParticleKinematics {
    static auto create(HistogramService* service, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(service, base_path);

        // Momentum
        group->add_hist1D("p", "Momentum;p [GeV/c];Particles", 200, 0, 10);
        group->add_hist1D("pt", "Transverse Momentum;p_{T} [GeV/c];Particles", 100, 0, 5);
        group->add_hist1D("pz", "Longitudinal Momentum;p_{z} [GeV/c];Particles", 200, -2, 10);

        // Angles
        group->add_hist1D("theta", "Polar Angle;#theta [deg];Particles", 180, 0, 180);
        group->add_hist1D("phi", "Azimuthal Angle;#phi [deg];Particles", 360, -180, 180);
        group->add_hist1D("eta", "Pseudorapidity;#eta;Particles", 100, -5, 5);

        // 2D correlations
        group->add_hist2D("p_vs_theta", "p vs #theta;#theta [deg];p [GeV/c]",
                          180, 0, 180, 100, 0, 10);
        group->add_hist2D("phi_vs_theta", "#phi vs #theta;#theta [deg];#phi [deg]",
                          180, 0, 180, 180, -180, 180);

        // Energy
        group->add_hist1D("E", "Energy;E [GeV];Particles", 200, 0, 12);

        return group;
    }
};

/**
 * @brief Standard electron ID histograms
 */
struct ElectronID {
    static auto create(HistogramService* service, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(service, base_path);

        // PID
        group->add_hist1D("chi2pid", "#chi^{2} PID;#chi^{2};Electrons", 100, -5, 5);
        group->add_hist1D("status", "Status;Status;Electrons", 100, -4500, -2000);

        // Calorimeter
        group->add_hist1D("ecal_energy", "ECAL Energy;E_{ECAL} [GeV];Electrons", 200, 0, 12);
        group->add_hist1D("sampling_fraction", "Sampling Fraction;E/p;Electrons", 100, 0, 0.5);
        group->add_hist2D("sf_vs_p", "SF vs p;p [GeV/c];E/p",
                          100, 0, 10, 100, 0, 0.5);

        // Tracking
        group->add_hist1D("vz", "Vertex Z;v_{z} [cm];Electrons", 100, -15, 10);
        group->add_hist1D("chi2", "Track #chi^{2};#chi^{2}/ndf;Electrons", 100, 0, 10);

        // Fiducial
        group->add_hist2D("dc_xy", "DC Position;x [cm];y [cm]",
                          200, -200, 200, 200, -200, 200);

        return group;
    }
};

/**
 * @brief Quality assurance histograms
 */
struct QualityAssurance {
    static auto create(HistogramService* service, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(service, base_path);

        // Event-level
        group->add_hist1D("n_particles", "Particles per Event;N_{particles};Events",
                          50, 0, 50);
        group->add_hist1D("n_electrons", "Electrons per Event;N_{e};Events",
                          10, 0, 10);
        group->add_hist1D("n_photons", "Photons per Event;N_{#gamma};Events",
                          20, 0, 20);
        group->add_hist1D("n_pions", "Pions per Event;N_{#pi};Events",
                          15, 0, 15);

        // Run conditions
        group->add_hist1D("event_number", "Event Number;Event;Events",
                          100, 0, 1000000);
        group->add_hist1D("helicity", "Helicity;Helicity;Events", 3, -1.5, 1.5);

        // Timing
        group->add_hist1D("start_time", "Event Start Time;t_{start} [ns];Events",
                          200, -50, 50);
        group->add_hist1D("rf_time", "RF Time;t_{RF} [ns];Events",
                          200, 0, 4);

        return group;
    }
};

/**
 * @brief Missing mass/momentum histograms
 */
struct MissingMassHistograms {
    static auto create(HistogramService* service, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(service, base_path);

        // Missing mass
        group->add_hist1D("mm", "Missing Mass;M_{X} [GeV/c^{2}];Events",
                          200, -1, 3);
        group->add_hist1D("mm2", "Missing Mass Squared;M_{X}^{2} [GeV^{2}/c^{4}];Events",
                          200, -1, 5);

        // Missing momentum
        group->add_hist1D("pmiss", "Missing Momentum;p_{miss} [GeV/c];Events",
                          100, 0, 5);
        group->add_hist1D("pmiss_perp", "Missing p_{#perp};p_{#perp}^{miss} [GeV/c];Events",
                          100, 0, 2);
        group->add_hist1D("pmiss_parallel", "Missing p_{||};p_{||}^{miss} [GeV/c];Events",
                          100, -2, 5);

        // Missing angles
        group->add_hist1D("theta_miss", "Missing #theta;#theta_{miss} [deg];Events",
                          180, 0, 180);
        group->add_hist1D("phi_miss", "Missing #phi;#phi_{miss} [deg];Events",
                          360, -180, 180);

        // Correlations
        group->add_hist2D("mm_vs_pmiss", "M_{X} vs p_{miss};p_{miss} [GeV/c];M_{X} [GeV/c^{2}]",
                          100, 0, 5, 100, -1, 3);

        return group;
    }
};

/**
 * @brief Invariant mass histograms for composite particles
 */
struct InvariantMassHistograms {
    static auto create(HistogramService* service, const std::string& base_path,
                       const std::string& title, double min, double max, int nbins = 100)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(service, base_path);

        // Basic mass
        std::string mass_title = "Invariant Mass " + title + ";M [GeV/c^{2}];Events";
        group->add_hist1D("mass", mass_title, nbins, min, max);

        // Mass vs kinematics
        group->add_hist2D("mass_vs_p", "M vs p;p [GeV/c];M [GeV/c^{2}]",
                          100, 0, 10, nbins, min, max);
        group->add_hist2D("mass_vs_theta", "M vs #theta;#theta [deg];M [GeV/c^{2}]",
                          180, 0, 180, nbins, min, max);

        // Kinematics
        group->add_hist1D("p", "Momentum;p [GeV/c];Events", 100, 0, 10);
        group->add_hist1D("theta", "Theta;#theta [deg];Events", 180, 0, 180);
        group->add_hist1D("phi", "Phi;#phi [deg];Events", 360, -180, 180);

        return group;
    }
};

/**
 * @brief Time-of-flight histograms
 */
struct TimeOfFlightHistograms {
    static auto create(HistogramService* service, const std::string& base_path)
        -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(service, base_path);

        // Beta
        group->add_hist1D("beta", "Beta;#beta;Particles", 100, 0, 1.2);
        group->add_hist2D("beta_vs_p", "#beta vs p;p [GeV/c];#beta",
                          100, 0, 10, 100, 0, 1.2);

        // Delta-t
        group->add_hist1D("delta_t", "#Delta t;#Deltat [ns];Particles",
                          200, -10, 10);
        group->add_hist2D("delta_t_vs_p", "#Delta t vs p;p [GeV/c];#Delta t [ns]",
                          100, 0, 10, 200, -10, 10);

        // Timing
        group->add_hist1D("tof", "Time of Flight;t [ns];Particles",
                          200, 0, 50);
        group->add_hist1D("path", "Path Length;L [cm];Particles",
                          200, 0, 1000);

        return group;
    }
};

/**
 * @brief DIS (Deep Inelastic Scattering) kinematics
 * 
 * 1D histograms for Q2, W, nu xB and y.
 * 2D histograms for Q2 vs W, Q2 vs xB and xB vs y. 
 */
struct DISKinematics {
    static auto create(HistogramService* service, const std::string& base_path) -> std::unique_ptr<HistogramGroup> {
        auto group = std::make_unique<HistogramGroup>(service, base_path);

        // DIS variables
        group->add_hist1D("Q2", "Q^{2};Q^{2} [GeV^{2}];Events", 100, 0, 10);
        group->add_hist1D("W", "W;W [GeV];Events", 100, 0, 5);
        group->add_hist1D("nu", "#nu;#nu [GeV];Events", 100, 0, 10);
        group->add_hist1D("xB", "Bjorken x;x;Events", 100, 0, 1);
        group->add_hist1D("y", "Inelasticity;y;Events", 100, 0, 1);

        // 2D correlations
        group->add_hist2D("W_vs_Q2", "W vs Q^{2};Q^{2} [GeV^{2}];W [GeV]", 100, 0, 10, 100, 0, 5);
        group->add_hist2D("xB_vs_Q2", "xB vs Q^{2};Q^{2} [GeV^{2}];xB", 100, 0, 10, 100, 0, 1);
        group->add_hist2D("y_vs_xB", "y vs xB;xB;y", 100, 0, 1, 100, 0, 1);

        return group;
    }
};

/**
 * @brief Helper function to create standard particle histograms
 */
inline auto create_particle_histograms(HistogramService* service,
                                       const std::string& particle_name)
    -> std::unique_ptr<HistogramGroup> {
    std::string base = "/" + particle_name;
    return ParticleKinematics::create(service, base);
}

/**
 * @brief Helper function to create Pi0 invariant mass histograms
 */
inline auto create_pi0_histograms(HistogramService* service,
                                  const std::string& base_path = "/Pi0")
    -> std::unique_ptr<HistogramGroup> {
    return InvariantMassHistograms::create(service, base_path,
                                           "(#gamma#gamma)", 0.0, 0.5, 200);
}

/**
 * @brief Helper function to create Eta invariant mass histograms
 */
inline auto create_eta_histograms(HistogramService* service,
                                  const std::string& base_path = "/Eta")
    -> std::unique_ptr<HistogramGroup> {
    return InvariantMassHistograms::create(service, base_path,
                                           "(#gamma#gamma)", 0.3, 0.8, 200);
}

}  // namespace HistogramTemplates

}  // namespace Framework
