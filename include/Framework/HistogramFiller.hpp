#pragma once

/**
 * @file HistogramFiller.hpp
 * @brief Utility functions for filling standard particle histograms with minimal boilerplate
 *
 * This file provides helper functions that fill all standard histograms for common
 * particle types (electrons, hadrons, photons) in a single function call, reducing
 * repetitive histogram filling code by ~90%.
 *
 * Example usage:
 * @code
 * // BEFORE: 28 lines of repetitive filling
 * kinematics_groups["before_cuts"]->hist1d("p").fill(p);
 * kinematics_groups["before_cuts"]->hist1d("theta").fill(electron.theta());
 * // ... 26 more lines
 *
 * // AFTER: 1 line
 * HistogramFiller::fill_electron(kinematics_groups["before_cuts"].get(), electron, calo, trk_r1, trk_r2, trk_r3, SF);
 * @endcode
 */

#include "CLAS/Banks.hpp"
#include "CLAS/Particle.hpp"
#include "Framework/Histogram.hpp"

namespace Framework {

/**
 * @brief Static utility class for filling standard particle histograms
 *
 * Provides one-call filling of all histograms for common particle types,
 * matching the histogram names defined in HistogramTemplates.hpp.
 */
class HistogramFiller {
public:
    /**
     * @brief Fill basic kinematics histograms (p, theta, phi, vz, vx, vy)
     *
     * Fills: p, theta, phi, vz, vx, vy
     *
     * @param group Histogram group to fill (must not be nullptr)
     * @param particle The particle to get kinematics from
     */
    static auto fill_kinematics(HistogramGroup* group, const CLAS::Particle& particle) -> void {
        if (!group) return;

        group->hist1d("p").fill(particle.p());
        group->hist1d("theta").fill(particle.theta());
        group->hist1d("phi").fill(particle.phi());
        group->hist1d("vz").fill(particle.vz());
        group->hist1d("vx").fill(particle.vx());
        group->hist1d("vy").fill(particle.vy());
    }

    /**
     * @brief Fill basic kinematics + PID histograms
     *
     * Fills: p, theta, phi, vz, vx, vy, chi2pid, beta
     *
     * @param group Histogram group to fill
     * @param particle The particle to get kinematics from
     */
    static auto fill_kinematics_with_pid(HistogramGroup* group, const CLAS::Particle& particle) -> void {
        if (!group) return;

        fill_kinematics(group, particle);
        group->hist1d("chi2pid").fill(particle.chi2pid());
        group->hist1d("beta").fill(particle.beta());
    }

    /**
     * @brief Fill all standard electron histograms
     *
     * Fills all histograms defined in HistogramTemplates::ElectronHistograms:
     * - Basic kinematics: p, theta, phi, vz, vx, vy, chi2pid, beta
     * - Sector
     * - Calorimeter: pcal_E, ecin_E, ecout_E, ecal_E, sampling_fraction
     * - DC chi2: dc_chi2_R1, dc_chi2_R2, dc_chi2_R3
     * - Fiducial: pcal_lu, pcal_lv, pcal_lw
     * - 2D correlations: sf_vs_p, chi2pid_vs_p, vz_vs_phi, vz_vs_theta, theta_vs_phi, p_vs_theta, pcal_lv_vs_lu, ecin_vs_pcal
     *
     * @param group Histogram group to fill
     * @param electron The electron particle
     * @param calo Calorimeter bank data
     * @param trk_r1 Track bank for DC region 1
     * @param trk_r2 Track bank for DC region 2
     * @param trk_r3 Track bank for DC region 3
     * @param SF Sampling fraction (pre-calculated)
     */
    static auto fill_electron(HistogramGroup* group,
                              const CLAS::Particle& electron,
                              const CLAS::CalorimeterBank& calo,
                              const CLAS::TrackBank& trk_r1,
                              const CLAS::TrackBank& trk_r2,
                              const CLAS::TrackBank& trk_r3,
                              double SF) -> void {
        if (!group) return;

        const double p = electron.p();
        const double E_tot = calo.pcal.energy + calo.inner.energy + calo.outer.energy;

        // Basic kinematics
        fill_kinematics_with_pid(group, electron);

        // Sector
        group->hist1d("sector").fill(static_cast<double>(calo.pcal.sector));

        // Calorimeter energies
        group->hist1d("pcal_E").fill(calo.pcal.energy);
        group->hist1d("ecin_E").fill(calo.inner.energy);
        group->hist1d("ecout_E").fill(calo.outer.energy);
        group->hist1d("ecal_E").fill(E_tot);
        group->hist1d("sampling_fraction").fill(SF);

        // DC chi2/NDF
        group->hist1d("dc_chi2_R1").fill(trk_r1.chi2 / trk_r1.ndf);
        group->hist1d("dc_chi2_R2").fill(trk_r2.chi2 / trk_r2.ndf);
        group->hist1d("dc_chi2_R3").fill(trk_r3.chi2 / trk_r3.ndf);

        // PCAL fiducial
        group->hist1d("pcal_lu").fill(calo.pcal.lu);
        group->hist1d("pcal_lv").fill(calo.pcal.lv);
        group->hist1d("pcal_lw").fill(calo.pcal.lw);

        // 2D correlations
        group->hist2d("sf_vs_p").fill(p, SF);
        group->hist2d("chi2pid_vs_p").fill(p, electron.chi2pid());
        group->hist2d("vz_vs_phi").fill(electron.phi(), electron.vz());
        group->hist2d("vz_vs_theta").fill(electron.theta(), electron.vz());
        group->hist2d("theta_vs_phi").fill(electron.phi(), electron.theta());
        group->hist2d("p_vs_theta").fill(electron.theta(), p);
        group->hist2d("pcal_lv_vs_lu").fill(calo.pcal.lu, calo.pcal.lv);
        group->hist2d("ecin_vs_pcal").fill(calo.pcal.energy, calo.inner.energy);
    }

    /**
     * @brief Fill all standard hadron histograms
     *
     * Fills all histograms defined in HistogramTemplates::HadronHistograms:
     * - Basic kinematics: p, theta, phi, vz, vx, vy, chi2pid, beta
     * - Delta quantities: delta_beta (calculated from expected beta)
     * - 2D correlations: beta_vs_p, chi2pid_vs_p, delta_beta_vs_p, theta_vs_phi, p_vs_theta, vz_vs_phi
     *
     * @param group Histogram group to fill
     * @param hadron The hadron particle
     * @param expected_beta Expected beta from particle hypothesis (beta = p/E)
     */
    static auto fill_hadron(HistogramGroup* group,
                            const CLAS::Particle& hadron,
                            double expected_beta) -> void {
        if (!group) return;

        const double p = hadron.p();
        const double delta_beta = hadron.beta() - expected_beta;

        // Basic kinematics
        fill_kinematics_with_pid(group, hadron);

        // Delta beta
        group->hist1d("delta_beta").fill(delta_beta);

        // 2D correlations
        group->hist2d("beta_vs_p").fill(p, hadron.beta());
        group->hist2d("chi2pid_vs_p").fill(p, hadron.chi2pid());
        group->hist2d("delta_beta_vs_p").fill(p, delta_beta);
        group->hist2d("theta_vs_phi").fill(hadron.phi(), hadron.theta());
        group->hist2d("p_vs_theta").fill(hadron.theta(), p);
        group->hist2d("vz_vs_phi").fill(hadron.phi(), hadron.vz());
    }

    /**
     * @brief Fill all standard hadron histograms with electron vertex comparison
     *
     * @param group Histogram group to fill
     * @param hadron The hadron particle
     * @param expected_beta Expected beta from particle hypothesis
     * @param electron_vz Electron vertex z for delta_vz calculation
     */
    static auto fill_hadron(HistogramGroup* group,
                            const CLAS::Particle& hadron,
                            double expected_beta,
                            double electron_vz) -> void {
        fill_hadron(group, hadron, expected_beta);
        if (!group) return;

        const double dvz = hadron.vz() - electron_vz;
        group->hist1d("dvz").fill(dvz);
    }

    /**
     * @brief Fill all standard photon histograms
     *
     * Fills all histograms defined in HistogramTemplates::PhotonID:
     * - Basic: beta, energy, theta, phi
     * - 2D: beta_vs_p, E_vs_theta
     *
     * @param group Histogram group to fill
     * @param photon The photon particle
     */
    static auto fill_photon(HistogramGroup* group, const CLAS::Particle& photon) -> void {
        if (!group) return;

        group->hist1d("beta").fill(photon.beta());
        group->hist1d("energy").fill(photon.E());
        group->hist1d("theta").fill(photon.theta());
        group->hist1d("phi").fill(photon.phi());

        // 2D correlations
        group->hist2d("beta_vs_p").fill(photon.p(), photon.beta());
        group->hist2d("E_vs_theta").fill(photon.theta(), photon.E());
    }

    /**
     * @brief Fill DIS kinematics histograms
     *
     * Fills all histograms defined in HistogramTemplates::DISKinematics:
     * - 1D: Q2, W, xB, y, nu
     * - 2D: Q2_vs_W, Q2_vs_xB, y_vs_xB, W_vs_xB
     *
     * @param group Histogram group to fill
     * @param Q2 Four-momentum transfer squared
     * @param W Invariant mass of hadronic system
     * @param xB Bjorken x
     * @param y Inelasticity
     * @param nu Energy transfer
     */
    static auto fill_dis_kinematics(HistogramGroup* group,
                                    double Q2, double W, double xB, double y, double nu) -> void {
        if (!group) return;

        // 1D
        group->hist1d("Q2").fill(Q2);
        group->hist1d("W").fill(W);
        group->hist1d("xB").fill(xB);
        group->hist1d("y").fill(y);
        group->hist1d("nu").fill(nu);

        // 2D
        group->hist2d("Q2_vs_W").fill(W, Q2);
        group->hist2d("Q2_vs_xB").fill(xB, Q2);
        group->hist2d("y_vs_xB").fill(xB, y);
        group->hist2d("W_vs_xB").fill(xB, W);
    }

    /**
     * @brief Fill missing kinematics histograms
     *
     * @param group Histogram group to fill
     * @param Mx Missing mass
     * @param Mx2 Missing mass squared
     * @param Pmiss Missing momentum magnitude
     * @param Emiss Missing energy
     * @param Q2 For 2D correlation (optional, pass 0 to skip)
     * @param W For 2D correlation (optional, pass 0 to skip)
     */
    static auto fill_missing_kinematics(HistogramGroup* group,
                                        double Mx, double Mx2, double Pmiss, double Emiss,
                                        double Q2 = 0.0, double W = 0.0) -> void {
        if (!group) return;

        group->hist1d("Mx").fill(Mx);
        group->hist1d("Mx2").fill(Mx2);
        group->hist1d("Pmiss").fill(Pmiss);
        group->hist1d("Emiss").fill(Emiss);

        if (Q2 > 0) group->hist2d("Mx_vs_Q2").fill(Q2, Mx);
        if (W > 0) group->hist2d("Mx_vs_W").fill(W, Mx);
    }
};

/**
 * @brief Staged histogram filler for before/after cuts pattern
 *
 * Provides convenient access to fill histograms at different cut stages.
 *
 * Example:
 * @code
 * std::map<std::string, std::unique_ptr<HistogramGroup>> groups;
 * StagedFiller filler(groups);
 *
 * // Fill before cuts
 * HistogramFiller::fill_electron(filler.at("before_cuts"), electron, calo, trk1, trk2, trk3, SF);
 *
 * // Apply cuts...
 *
 * // Fill after cuts
 * HistogramFiller::fill_electron(filler.at("after_cuts"), electron, calo, trk1, trk2, trk3, SF);
 * @endcode
 */
class StagedFiller {
public:
    explicit StagedFiller(std::map<std::string, std::unique_ptr<HistogramGroup>>& groups)
        : m_groups(groups) {}

    /**
     * @brief Get histogram group for a specific stage
     * @param stage Stage name (e.g., "before_cuts", "after_cuts")
     * @return HistogramGroup pointer, or nullptr if stage not found
     */
    auto at(const std::string& stage) -> HistogramGroup* {
        auto it = m_groups.find(stage);
        return it != m_groups.end() ? it->second.get() : nullptr;
    }

private:
    std::map<std::string, std::unique_ptr<HistogramGroup>>& m_groups;
};

}  // namespace Framework
